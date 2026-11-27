#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include "../common/envelope.h"
#include "../common/network.h"
#include <ws2tcpip.h>

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

class CentralLink
{
public:
    using Clock = std::chrono::steady_clock;

    struct Config
    {
        std::string gatewayId = "A";
        std::string centralIp = "127.0.0.1";
        unsigned short centralPort = 14700;
        int heartbeatIntervalMs = 1000;
        int timeoutMs = 3000;
    };

    CentralLink() = default;
    ~CentralLink() { close(); }

    CentralLink(const CentralLink &) = delete;
    CentralLink &operator=(const CentralLink &) = delete;

    bool open(const Config &config, std::string &errorOut)
    {
        close();
        config_ = config;
        if (config_.gatewayId.empty() || config_.gatewayId.size() > 4)
        {
            errorOut = "gatewayId must be 1..4 characters";
            return false;
        }

        sockaddr_in remote{};
        remote.sin_family = AF_INET;
        remote.sin_port = htons(config_.centralPort);
        if (inet_pton(AF_INET, config_.centralIp.c_str(), &remote.sin_addr) != 1)
        {
            errorOut = "Invalid central IPv4 address: " + config_.centralIp;
            return false;
        }

        SOCKET s = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (s == INVALID_SOCKET)
        {
            errorOut = "Central UDP socket failed: " + std::to_string(WSAGetLastError());
            return false;
        }
        if (::connect(s, reinterpret_cast<const sockaddr *>(&remote), sizeof(remote)) == SOCKET_ERROR)
        {
            errorOut = "Central UDP connect failed: " + std::to_string(WSAGetLastError());
            Network::closeSocket(s);
            return false;
        }
        if (!Network::setNonblocking(s))
        {
            errorOut = "Cannot set Central UDP socket nonblocking";
            Network::closeSocket(s);
            return false;
        }

        socket_ = s;
        sessionId_ = static_cast<std::uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count());
        if (sessionId_ == 0)
        {
            sessionId_ = 1;
        }
        sequence_ = 0;
        const auto now = Clock::now();
        lastHeartbeatSent_ = Clock::time_point::min();
        lastCentralRx_ = now;
        hasReceivedFromCentral_ = false;

        sendRegister(now);
        return true;
    }

    void close()
    {
        Network::closeSocket(socket_);
    }

    bool isOpen() const { return socket_ != INVALID_SOCKET; }
    SOCKET socket() const { return socket_; }
    const std::string &gatewayId() const { return config_.gatewayId; }
    std::uint32_t sessionId() const { return sessionId_; }

    bool isCentralAlive(Clock::time_point now = Clock::now()) const
    {
        if (!hasReceivedFromCentral_)
        {
            return false;
        }
        const auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCentralRx_);
        return age.count() <= config_.timeoutMs;
    }

    void tick(Clock::time_point now = Clock::now())
    {
        if (!isOpen())
        {
            return;
        }
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastHeartbeatSent_);
        if (lastHeartbeatSent_ == Clock::time_point::min() || elapsed.count() >= config_.heartbeatIntervalMs)
        {
            EnvelopeCodec::Envelope hb;
            hb.type = EnvelopeCodec::MessageType::GatewayHeartbeat;
            sendEnvelope(hb);
            lastHeartbeatSent_ = now;
        }
    }

    bool sendRegister(Clock::time_point now = Clock::now())
    {
        EnvelopeCodec::Envelope reg;
        reg.type = EnvelopeCodec::MessageType::GatewayRegister;
        const bool ok = sendEnvelope(reg);
        if (ok)
        {
            lastHeartbeatSent_ = now;
        }
        return ok;
    }

    bool sendTelemetry(std::uint8_t sysid, const char *data, std::size_t size)
    {
        if (!data || size == 0 || size > EnvelopeCodec::MAX_PAYLOAD_SIZE)
        {
            return false;
        }
        EnvelopeCodec::Envelope env;
        env.type = EnvelopeCodec::MessageType::TelemetryForward;
        env.sysid = sysid;
        env.payload.assign(reinterpret_cast<const std::uint8_t *>(data),
                           reinterpret_cast<const std::uint8_t *>(data) + size);
        return sendEnvelope(env);
    }

    bool sendLinkReport(std::uint8_t sysid, const EnvelopeCodec::LinkReportPayload &report)
    {
        EnvelopeCodec::Envelope env;
        env.type = EnvelopeCodec::MessageType::LinkReport;
        env.sysid = sysid;
        env.payload = EnvelopeCodec::encodeLinkReport(report);
        return sendEnvelope(env);
    }

    bool sendHandoverAck(std::uint8_t sysid,
                         const std::string &targetGatewayId,
                         const EnvelopeCodec::HandoverPayload &handover)
    {
        EnvelopeCodec::Envelope env;
        env.type = EnvelopeCodec::MessageType::HandoverAck;
        env.sysid = sysid;
        env.targetGatewayId = targetGatewayId;
        env.payload = EnvelopeCodec::encodeHandover(handover);
        return sendEnvelope(env);
    }

    bool sendEnvelope(EnvelopeCodec::Envelope &env)
    {
        if (!isOpen())
        {
            return false;
        }
        env.gatewayId = config_.gatewayId;
        env.sessionId = sessionId_;
        if (env.sequence == 0)
        {
            env.sequence = ++sequence_;
        }
        const auto wire = EnvelopeCodec::encode(env);
        const int sent = ::send(socket_,
                                reinterpret_cast<const char *>(wire.data()),
                                static_cast<int>(wire.size()),
                                0);
        if (sent == SOCKET_ERROR)
        {
            const int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK)
            {
                std::cerr << "[CentralLink:" << config_.gatewayId << "] send failed: " << err << '\n';
            }
            return false;
        }
        return sent == static_cast<int>(wire.size());
    }

    std::size_t poll(std::vector<EnvelopeCodec::Envelope> &out, Clock::time_point now = Clock::now())
    {
        if (!isOpen())
        {
            return 0;
        }
        std::size_t count = 0;
        std::array<char, 65535> buffer{};
        for (;;)
        {
            const int received = ::recv(socket_, buffer.data(), static_cast<int>(buffer.size()), 0);
            if (received == SOCKET_ERROR)
            {
                const int err = WSAGetLastError();
                if (err != WSAEWOULDBLOCK && err != WSAECONNRESET)
                {
                    std::cerr << "[CentralLink:" << config_.gatewayId << "] recv failed: " << err << '\n';
                }
                break;
            }
            if (received <= 0)
            {
                break;
            }
            EnvelopeCodec::Envelope env;
            if (!EnvelopeCodec::decode(buffer.data(), static_cast<std::size_t>(received), env))
            {
                continue;
            }
            lastCentralRx_ = now;
            hasReceivedFromCentral_ = true;
            out.push_back(std::move(env));
            ++count;
        }
        return count;
    }

private:
    Config config_{};
    SOCKET socket_ = INVALID_SOCKET;
    std::uint32_t sessionId_ = 0;
    std::uint32_t sequence_ = 0;
    Clock::time_point lastHeartbeatSent_ = Clock::time_point::min();
    Clock::time_point lastCentralRx_ = Clock::time_point::min();
    bool hasReceivedFromCentral_ = false;
};
