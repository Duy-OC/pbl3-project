#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include "../common/envelope.h"
#include "../common/network.h"
#include "client_manager.h"
#include "control_policy.h"
#include "gateway_manager.h"
#include "handover_manager.h"
#include "route_table.h"

#include <windows.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    volatile LONG stopping = 0;

    BOOL WINAPI onControl(DWORD event)
    {
        if (event != CTRL_C_EVENT && event != CTRL_BREAK_EVENT)
        {
            return FALSE;
        }
        InterlockedExchange(&stopping, 1);
        return TRUE;
    }

    struct SocketGuard
    {
        SOCKET value = INVALID_SOCKET;
        ~SocketGuard() { Network::closeSocket(value); }
        SocketGuard() = default;
        SocketGuard(const SocketGuard &) = delete;
        SocketGuard &operator=(const SocketGuard &) = delete;
    };

    struct CentralOptions
    {
        unsigned short udpPort = 14700;
        unsigned short tcpPort = 5760;
        std::string initialGateway = "A";
        bool autoHandover = false;
    };

    unsigned short parsePort(const char *text)
    {
        const std::string value(text);
        if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
        {
            throw std::runtime_error("Invalid port: " + value);
        }
        const unsigned long number = std::stoul(value);
        if (number == 0 || number > 65535)
        {
            throw std::runtime_error("Port out of range: " + value);
        }
        return static_cast<unsigned short>(number);
    }

    CentralOptions parseArgs(int argc, char **argv)
    {
        CentralOptions opts;
        for (int i = 1; i < argc; ++i)
        {
            const std::string key(argv[i]);
            if (key == "--help")
            {
                std::cout << "Usage: central.exe [--udp-port 14700] [--tcp-port 5760] "
                             "[--initial-gateway A] [--mode manual|auto]\n";
                std::exit(0);
            }
            if (i + 1 >= argc)
            {
                throw std::runtime_error("Missing value for " + key);
            }
            const std::string val(argv[++i]);
            if (key == "--udp-port")
            {
                opts.udpPort = parsePort(val.c_str());
            }
            else if (key == "--tcp-port")
            {
                opts.tcpPort = parsePort(val.c_str());
            }
            else if (key == "--initial-gateway")
            {
                opts.initialGateway = val;
            }
            else if (key == "--mode")
            {
                if (val == "auto")
                {
                    opts.autoHandover = true;
                }
                else if (val == "manual")
                {
                    opts.autoHandover = false;
                }
                else
                {
                    throw std::runtime_error("Invalid mode (use manual or auto): " + val);
                }
            }
            else
            {
                throw std::runtime_error("Unknown option: " + key);
            }
        }
        return opts;
    }

    bool sendEnvelopeTo(SOCKET udpSocket,
                        const sockaddr_in &target,
                        const EnvelopeCodec::Envelope &env)
    {
        const auto wire = EnvelopeCodec::encode(env);
        const int sent = ::sendto(udpSocket,
                                  reinterpret_cast<const char *>(wire.data()),
                                  static_cast<int>(wire.size()),
                                  0,
                                  reinterpret_cast<const sockaddr *>(&target),
                                  sizeof(target));
        return sent == static_cast<int>(wire.size());
    }
} // namespace

int main(int argc, char **argv)
{
    try
    {
        const CentralOptions opts = parseArgs(argc, argv);

        Network::Runtime runtime;
        if (runtime.error() != 0)
        {
            throw std::runtime_error("WSAStartup failed: " + std::to_string(runtime.error()));
        }

        SocketGuard udpSocket;
        udpSocket.value = Network::createBoundSocket(SOCK_DGRAM, opts.udpPort);
        if (udpSocket.value == INVALID_SOCKET || !Network::setNonblocking(udpSocket.value))
        {
            throw std::runtime_error("Cannot open Central UDP socket on port " + std::to_string(opts.udpPort));
        }

        SocketGuard tcpListener;
        tcpListener.value = Network::createBoundSocket(SOCK_STREAM, opts.tcpPort);
        if (tcpListener.value == INVALID_SOCKET ||
            ::listen(tcpListener.value, SOMAXCONN) == SOCKET_ERROR ||
            !Network::setNonblocking(tcpListener.value))
        {
            throw std::runtime_error("Cannot open Central TCP listener on port " + std::to_string(opts.tcpPort));
        }

        if (!SetConsoleCtrlHandler(onControl, TRUE))
        {
            throw std::runtime_error("Cannot install Ctrl+C handler");
        }

        GatewayManager gateways({"A", "B"}, 3000);
        RouteTable routes(opts.initialGateway, 3000, true);
        HandoverManager::Config hoConfig;
        hoConfig.automatic = opts.autoHandover;
        HandoverManager handover(hoConfig);
        CentralClientManager clients(10, 1048576);
        ControlPolicy controlPolicy(true);

        std::cout << "[Central] Listening UDP gateways on " << opts.udpPort
                  << ", TCP clients on " << opts.tcpPort
                  << ", mode=" << (opts.autoHandover ? "auto" : "manual")
                  << ", initial_gateway=" << opts.initialGateway << '\n'
                  << std::flush;

        using Clock = std::chrono::steady_clock;
        std::uint32_t commandSeq = 0;
        std::array<char, 65535> udpBuffer{};

        std::uint64_t rxTelemetryPkts = 0;
        std::uint64_t txTelemetryBytes = 0;
        std::uint64_t rxClientCmds = 0;
        std::uint64_t txGatewayCmds = 0;
        std::uint64_t droppedCmdsNoCtrl = 0;
        std::uint64_t droppedCmdsNoRoute = 0;
        std::uint64_t rxLinkReports = 0;
        std::uint64_t handoverAcks = 0;
        auto nextStatsLog = Clock::now() + std::chrono::seconds(3);

        const auto onDisconnect = [&](unsigned int clientId)
        {
            const bool wasCtrl = controlPolicy.controllerId() && *controlPolicy.controllerId() == clientId;
            controlPolicy.onClientDisconnect(clientId);
            if (wasCtrl)
            {
                std::cout << "[Central] Client #" << clientId << " released controller lock (disconnected)\n";
            }
        };

        const auto onClientMavlink = [&](unsigned int clientId,
                                         const mavlink_message_t &msg,
                                         const std::vector<std::uint8_t> &rawBytes)
        {
            ++rxClientCmds;
            const auto prevCtrl = controlPolicy.controllerId();
            if (!controlPolicy.canForward(clientId, msg))
            {
                ++droppedCmdsNoCtrl;
                return;
            }
            if (!prevCtrl && controlPolicy.controllerId() && *controlPolicy.controllerId() == clientId)
            {
                std::cout << "[Central] Client #" << clientId << " acquired controller lock\n";
            }
            const auto now = Clock::now();
            // MAVLink target_system is not universal across all messages; default to SYSID 1 when absent.
            const std::uint8_t targetSysid = 1;
            const auto routeGw = routes.resolveForCommand(targetSysid, now);
            if (!routeGw)
            {
                ++droppedCmdsNoRoute;
                return;
            }
            const auto endpoint = gateways.endpointOf(*routeGw, now);
            if (!endpoint)
            {
                ++droppedCmdsNoRoute;
                return;
            }
            EnvelopeCodec::Envelope cmd;
            cmd.type = EnvelopeCodec::MessageType::CommandForward;
            cmd.sysid = targetSysid;
            cmd.gatewayId = *routeGw;
            cmd.sequence = ++commandSeq;
            cmd.payload = rawBytes;
            if (sendEnvelopeTo(udpSocket.value, *endpoint, cmd))
            {
                ++txGatewayCmds;
            }
        };

        while (InterlockedCompareExchange(&stopping, 0, 0) == 0)
        {
            fd_set reads, writes;
            FD_ZERO(&reads);
            FD_ZERO(&writes);
            FD_SET(udpSocket.value, &reads);
            FD_SET(tcpListener.value, &reads);
            clients.prepareSets(reads, writes);

            timeval timeout{0, 50000};
            if (::select(0, &reads, &writes, nullptr, &timeout) == SOCKET_ERROR)
            {
                throw std::runtime_error("select failed: " + std::to_string(WSAGetLastError()));
            }

            const auto now = Clock::now();

            if (FD_ISSET(tcpListener.value, &reads))
            {
                clients.acceptOne(tcpListener.value);
            }

            clients.serviceClients(reads, writes, onClientMavlink, onDisconnect);

            if (FD_ISSET(udpSocket.value, &reads))
            {
                for (;;)
                {
                    sockaddr_in sender{};
                    int senderLen = sizeof(sender);
                    const int count = ::recvfrom(udpSocket.value,
                                                 udpBuffer.data(),
                                                 static_cast<int>(udpBuffer.size()),
                                                 0,
                                                 reinterpret_cast<sockaddr *>(&sender),
                                                 &senderLen);
                    if (count == SOCKET_ERROR)
                    {
                        break;
                    }
                    if (count <= 0)
                    {
                        continue;
                    }
                    EnvelopeCodec::Envelope env;
                    if (!EnvelopeCodec::decode(udpBuffer.data(), static_cast<std::size_t>(count), env))
                    {
                        continue;
                    }
                    bool restarted = false;
                    if (!gateways.handleRegisterOrHeartbeat(env.gatewayId, sender, env.sessionId, now, restarted))
                    {
                        continue;
                    }
                    if (restarted)
                    {
                        std::cout << "[Central] Gateway " << env.gatewayId << " restarted (new session)\n";
                    }

                    switch (env.type)
                    {
                    case EnvelopeCodec::MessageType::GatewayRegister:
                    case EnvelopeCodec::MessageType::GatewayHeartbeat:
                    {
                        if (env.type == EnvelopeCodec::MessageType::GatewayRegister)
                        {
                            std::cout << "[Central] Gateway " << env.gatewayId << " registered\n";
                        }
                        EnvelopeCodec::Envelope ack;
                        ack.type = EnvelopeCodec::MessageType::GatewayHeartbeat;
                        ack.gatewayId = env.gatewayId;
                        ack.sessionId = env.sessionId;
                        ack.sequence = env.sequence;
                        sendEnvelopeTo(udpSocket.value, sender, ack);
                        break;
                    }
                    case EnvelopeCodec::MessageType::TelemetryForward:
                    {
                        ++rxTelemetryPkts;
                        routes.recordTelemetry(env.sysid, env.gatewayId, now);
                        if (routes.shouldAcceptTelemetry(env.sysid, env.gatewayId) && !env.payload.empty())
                        {
                            txTelemetryBytes += env.payload.size() * clients.size();
                            clients.broadcastTelemetry(env.payload.data(), env.payload.size(), onDisconnect);
                        }
                        break;
                    }
                    case EnvelopeCodec::MessageType::LinkReport:
                    {
                        ++rxLinkReports;
                        EnvelopeCodec::LinkReportPayload report;
                        if (EnvelopeCodec::decodeLinkReport(env.payload, report))
                        {
                            gateways.updateLinkReport(env.sysid, env.gatewayId, report, now);
                        }
                        break;
                    }
                    case EnvelopeCodec::MessageType::HandoverAck:
                    {
                        ++handoverAcks;
                        EnvelopeCodec::HandoverPayload ack;
                        if (EnvelopeCodec::decodeHandover(env.payload, ack))
                        {
                            const bool confirmed =
                                (ack.handoverId == 0 && ack.status == EnvelopeCodec::HandoverStatus::Accepted)
                                    ? (routes.confirmRoute(env.sysid, env.targetGatewayId, now), true)
                                    : handover.onAck(env.sysid, env.targetGatewayId, ack, routes, now);
                            if (confirmed)
                            {
                                std::cout << "[Central] Handover confirmed: SYSID="
                                          << static_cast<unsigned>(env.sysid)
                                          << " -> Gateway " << env.targetGatewayId << '\n';
                            }
                        }
                        break;
                    }
                    default:
                        break;
                    }
                }
            }

            const auto pendingSwitches = handover.evaluate(gateways, routes, now);
            for (const auto &sw : pendingSwitches)
            {
                const auto endpoint = gateways.endpointOf(sw.targetGatewayId, now);
                if (!endpoint)
                {
                    continue;
                }
                EnvelopeCodec::Envelope req;
                req.type = EnvelopeCodec::MessageType::HandoverRequest;
                req.sysid = sw.sysid;
                req.gatewayId = sw.targetGatewayId;
                req.targetGatewayId = sw.targetGatewayId;
                req.payload = EnvelopeCodec::encodeHandover({sw.handoverId, EnvelopeCodec::HandoverStatus::Accepted});
                sendEnvelopeTo(udpSocket.value, *endpoint, req);
            }

            if (now >= nextStatsLog)
            {
                nextStatsLog = now + std::chrono::seconds(3);
                std::cout << "[Central STATS] clients=" << clients.size()
                          << " controller=" << (controlPolicy.controllerId()
                                                    ? ("#" + std::to_string(*controlPolicy.controllerId()))
                                                    : "none")
                          << " gwA=" << (gateways.isAlive("A", now) ? "UP" : "DOWN")
                          << " gwB=" << (gateways.isAlive("B", now) ? "UP" : "DOWN")
                          << " telemetry_rx=" << rxTelemetryPkts
                          << " client_tx_bytes=" << txTelemetryBytes
                          << " cmd_rx=" << rxClientCmds
                          << " cmd_tx_gw=" << txGatewayCmds
                          << " cmd_drop_no_ctrl=" << droppedCmdsNoCtrl
                          << " cmd_drop_no_route=" << droppedCmdsNoRoute
                          << " link_reports=" << rxLinkReports
                          << " ho_acks=" << handoverAcks;
                for (const auto &[sysid, entry] : routes.entries())
                {
                    std::cout << " route[" << static_cast<unsigned>(sysid) << "]=" << entry.gatewayId;
                }
                std::cout << '\n' << std::flush;
            }
        }

        SetConsoleCtrlHandler(onControl, FALSE);
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "[Central] Fatal: " << error.what() << '\n';
        return 1;
    }
}
