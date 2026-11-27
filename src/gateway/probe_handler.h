#pragma once

#include "../common/envelope.h"
#include "../common/network.h"

#include <cstddef>
#include <cstdint>
#include <string>

class ProbeHandler
{
public:
    struct Stats
    {
        std::uint64_t pingsReceived = 0;
        std::uint64_t pongsSent = 0;
        std::uint64_t malformedProbes = 0;
        std::uint64_t sendFailures = 0;
    };

    explicit ProbeHandler(std::string gatewayId = "A", bool enabled = true)
        : gatewayId_(std::move(gatewayId)), enabled_(enabled) {}

    void setGatewayId(const std::string &gatewayId) { gatewayId_ = gatewayId; }
    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool enabled() const { return enabled_; }
    const Stats &stats() const { return stats_; }

    // Returns true if the datagram was a probe envelope (consumed and not raw MAVLink).
    bool handleDatagram(const char *data,
                        std::size_t size,
                        const sockaddr_in &sender,
                        SOCKET udpSocket)
    {
        if (!EnvelopeCodec::isEnvelope(data, size))
        {
            return false;
        }

        EnvelopeCodec::Envelope incoming;
        if (!EnvelopeCodec::decode(data, size, incoming))
        {
            return false;
        }
        if (incoming.type != EnvelopeCodec::MessageType::ProbePing)
        {
            return false;
        }

        ++stats_.pingsReceived;
        if (!enabled_)
        {
            return true;
        }

        EnvelopeCodec::ProbePayload probe;
        if (!EnvelopeCodec::decodeProbe(incoming.payload, probe))
        {
            ++stats_.malformedProbes;
            return true;
        }

        EnvelopeCodec::Envelope pong;
        pong.type = EnvelopeCodec::MessageType::ProbePong;
        pong.flags = incoming.flags;
        pong.sysid = incoming.sysid;
        pong.gatewayId = gatewayId_;
        pong.targetGatewayId = incoming.gatewayId;
        pong.sequence = incoming.sequence;
        pong.sessionId = incoming.sessionId;
        pong.payload = EnvelopeCodec::encodeProbe(probe);

        const auto wire = EnvelopeCodec::encode(pong);
        const int sent = ::sendto(udpSocket,
                                  reinterpret_cast<const char *>(wire.data()),
                                  static_cast<int>(wire.size()),
                                  0,
                                  reinterpret_cast<const sockaddr *>(&sender),
                                  sizeof(sender));
        if (sent == static_cast<int>(wire.size()))
        {
            ++stats_.pongsSent;
        }
        else
        {
            ++stats_.sendFailures;
        }
        return true;
    }

private:
    std::string gatewayId_;
    bool enabled_ = true;
    Stats stats_{};
};
