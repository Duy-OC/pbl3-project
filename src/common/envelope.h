#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace EnvelopeCodec
{
    inline constexpr std::array<std::uint8_t, 4> MAGIC = {'P', 'B', 'L', '3'};
    inline constexpr std::uint8_t PROTOCOL_VERSION = 1;
    inline constexpr std::size_t HEADER_SIZE = 28;
    inline constexpr std::size_t MAX_PAYLOAD_SIZE = 65000;

    enum class MessageType : std::uint8_t
    {
        GatewayRegister = 1,
        GatewayHeartbeat = 2,
        TelemetryForward = 3,
        CommandForward = 4,
        ProbePing = 5,
        ProbePong = 6,
        LinkReport = 7,
        HandoverRequest = 8,
        HandoverAck = 9
    };

    enum class HandoverStatus : std::uint8_t
    {
        Accepted = 0,
        OutOfRange = 1,
        StalePosition = 2,
        StaleOrLateRequest = 3,
        UnsupportedTarget = 4
    };

    struct ProbePayload
    {
        std::uint64_t timestampUs = 0;
        std::uint32_t probeSeq = 0;
    };

    struct LinkReportPayload
    {
        float rssiDbm = -120.0f;
        float rttMs = 0.0f;
        float probeLossRatio = 0.0f;
        std::uint32_t consecutiveTimeouts = 0;
        std::uint32_t positionAgeMs = 0;
        std::uint8_t inRange = 0;
        std::uint8_t linkUp = 0;
    };

    struct HandoverPayload
    {
        std::uint32_t handoverId = 0;
        HandoverStatus status = HandoverStatus::Accepted;
    };

    struct Envelope
    {
        MessageType type = MessageType::GatewayHeartbeat;
        std::uint16_t flags = 0;
        std::uint8_t sysid = 0;
        std::string gatewayId;
        std::string targetGatewayId;
        std::uint32_t sequence = 0;
        std::uint32_t sessionId = 0;
        std::vector<std::uint8_t> payload;
    };

    namespace detail
    {
        inline void writeU16(std::uint8_t *dst, std::uint16_t value)
        {
            dst[0] = static_cast<std::uint8_t>((value >> 8) & 0xFFu);
            dst[1] = static_cast<std::uint8_t>(value & 0xFFu);
        }

        inline void writeU32(std::uint8_t *dst, std::uint32_t value)
        {
            dst[0] = static_cast<std::uint8_t>((value >> 24) & 0xFFu);
            dst[1] = static_cast<std::uint8_t>((value >> 16) & 0xFFu);
            dst[2] = static_cast<std::uint8_t>((value >> 8) & 0xFFu);
            dst[3] = static_cast<std::uint8_t>(value & 0xFFu);
        }

        inline void writeU64(std::uint8_t *dst, std::uint64_t value)
        {
            for (int i = 0; i < 8; ++i)
            {
                dst[i] = static_cast<std::uint8_t>((value >> ((7 - i) * 8)) & 0xFFu);
            }
        }

        inline std::uint16_t readU16(const std::uint8_t *src)
        {
            return static_cast<std::uint16_t>((static_cast<std::uint16_t>(src[0]) << 8) |
                                              static_cast<std::uint16_t>(src[1]));
        }

        inline std::uint32_t readU32(const std::uint8_t *src)
        {
            return (static_cast<std::uint32_t>(src[0]) << 24) |
                   (static_cast<std::uint32_t>(src[1]) << 16) |
                   (static_cast<std::uint32_t>(src[2]) << 8) |
                   static_cast<std::uint32_t>(src[3]);
        }

        inline std::uint64_t readU64(const std::uint8_t *src)
        {
            std::uint64_t value = 0;
            for (int i = 0; i < 8; ++i)
            {
                value = (value << 8) | static_cast<std::uint64_t>(src[i]);
            }
            return value;
        }

        inline void writeFloat(std::uint8_t *dst, float value)
        {
            std::uint32_t bits = 0;
            static_assert(sizeof(float) == sizeof(bits), "Unexpected float size");
            std::memcpy(&bits, &value, sizeof(bits));
            writeU32(dst, bits);
        }

        inline float readFloat(const std::uint8_t *src)
        {
            const std::uint32_t bits = readU32(src);
            float value = 0.0f;
            std::memcpy(&value, &bits, sizeof(value));
            return value;
        }

        inline void writeFixedId(std::uint8_t *dst, const std::string &id)
        {
            std::memset(dst, 0, 4);
            const std::size_t n = id.size() < 4 ? id.size() : 4;
            if (n > 0)
            {
                std::memcpy(dst, id.data(), n);
            }
        }

        inline std::string readFixedId(const std::uint8_t *src)
        {
            std::size_t n = 0;
            while (n < 4 && src[n] != '\0')
            {
                ++n;
            }
            return std::string(reinterpret_cast<const char *>(src), n);
        }
    } // namespace detail

    inline bool isEnvelope(const char *data, std::size_t size)
    {
        if (!data || size < HEADER_SIZE)
        {
            return false;
        }
        return std::memcmp(data, MAGIC.data(), MAGIC.size()) == 0;
    }

    inline bool isValidType(std::uint8_t raw)
    {
        return raw >= static_cast<std::uint8_t>(MessageType::GatewayRegister) &&
               raw <= static_cast<std::uint8_t>(MessageType::HandoverAck);
    }

    inline std::vector<std::uint8_t> encode(const Envelope &env)
    {
        const std::size_t payloadLen = env.payload.size() > MAX_PAYLOAD_SIZE
                                           ? MAX_PAYLOAD_SIZE
                                           : env.payload.size();
        std::vector<std::uint8_t> out(HEADER_SIZE + payloadLen, 0);
        std::memcpy(out.data(), MAGIC.data(), MAGIC.size());
        out[4] = PROTOCOL_VERSION;
        out[5] = static_cast<std::uint8_t>(env.type);
        detail::writeU16(out.data() + 6, env.flags);
        out[8] = env.sysid;
        out[9] = 0; // reserved
        detail::writeFixedId(out.data() + 10, env.gatewayId);
        detail::writeFixedId(out.data() + 14, env.targetGatewayId);
        detail::writeU32(out.data() + 18, env.sequence);
        detail::writeU32(out.data() + 22, env.sessionId);
        detail::writeU16(out.data() + 26, static_cast<std::uint16_t>(payloadLen));
        if (payloadLen > 0)
        {
            std::memcpy(out.data() + HEADER_SIZE, env.payload.data(), payloadLen);
        }
        return out;
    }

    inline bool decode(const char *data, std::size_t size, Envelope &out)
    {
        if (!isEnvelope(data, size))
        {
            return false;
        }
        const auto *bytes = reinterpret_cast<const std::uint8_t *>(data);
        if (bytes[4] != PROTOCOL_VERSION || !isValidType(bytes[5]))
        {
            return false;
        }
        const std::uint16_t payloadLen = detail::readU16(bytes + 26);
        if (payloadLen > MAX_PAYLOAD_SIZE || size != HEADER_SIZE + payloadLen)
        {
            return false;
        }

        out.type = static_cast<MessageType>(bytes[5]);
        out.flags = detail::readU16(bytes + 6);
        out.sysid = bytes[8];
        out.gatewayId = detail::readFixedId(bytes + 10);
        out.targetGatewayId = detail::readFixedId(bytes + 14);
        out.sequence = detail::readU32(bytes + 18);
        out.sessionId = detail::readU32(bytes + 22);
        out.payload.assign(bytes + HEADER_SIZE, bytes + HEADER_SIZE + payloadLen);
        return true;
    }

    inline std::vector<std::uint8_t> encodeProbe(const ProbePayload &probe)
    {
        std::vector<std::uint8_t> buf(12, 0);
        detail::writeU64(buf.data(), probe.timestampUs);
        detail::writeU32(buf.data() + 8, probe.probeSeq);
        return buf;
    }

    inline bool decodeProbe(const std::vector<std::uint8_t> &payload, ProbePayload &out)
    {
        if (payload.size() != 12)
        {
            return false;
        }
        out.timestampUs = detail::readU64(payload.data());
        out.probeSeq = detail::readU32(payload.data() + 8);
        return true;
    }

    inline std::vector<std::uint8_t> encodeLinkReport(const LinkReportPayload &report)
    {
        std::vector<std::uint8_t> buf(22, 0);
        detail::writeFloat(buf.data(), report.rssiDbm);
        detail::writeFloat(buf.data() + 4, report.rttMs);
        detail::writeFloat(buf.data() + 8, report.probeLossRatio);
        detail::writeU32(buf.data() + 12, report.consecutiveTimeouts);
        detail::writeU32(buf.data() + 16, report.positionAgeMs);
        buf[20] = report.inRange;
        buf[21] = report.linkUp;
        return buf;
    }

    inline bool decodeLinkReport(const std::vector<std::uint8_t> &payload, LinkReportPayload &out)
    {
        if (payload.size() != 22)
        {
            return false;
        }
        out.rssiDbm = detail::readFloat(payload.data());
        out.rttMs = detail::readFloat(payload.data() + 4);
        out.probeLossRatio = detail::readFloat(payload.data() + 8);
        out.consecutiveTimeouts = detail::readU32(payload.data() + 12);
        out.positionAgeMs = detail::readU32(payload.data() + 16);
        out.inRange = payload[20];
        out.linkUp = payload[21];
        return true;
    }

    inline std::vector<std::uint8_t> encodeHandover(const HandoverPayload &handover)
    {
        std::vector<std::uint8_t> buf(5, 0);
        detail::writeU32(buf.data(), handover.handoverId);
        buf[4] = static_cast<std::uint8_t>(handover.status);
        return buf;
    }

    inline bool decodeHandover(const std::vector<std::uint8_t> &payload, HandoverPayload &out)
    {
        if (payload.size() != 5)
        {
            return false;
        }
        out.handoverId = detail::readU32(payload.data());
        out.status = static_cast<HandoverStatus>(payload[4]);
        return true;
    }
} // namespace EnvelopeCodec
