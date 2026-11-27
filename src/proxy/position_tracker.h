#pragma once

#include "../common/mavlink_codec.h"
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>

// NED: x = North, y = East, z = Down (meters); upward velocity is negative vz.
class PositionTracker {
public:
    using Clock = std::chrono::steady_clock;
    enum class Motion { Stationary, TakingOff, Moving };
    struct Position {
        float x = 0, y = 0, z = 0;
        float vx = 0, vy = 0, vz = 0;
        std::uint32_t timeBootMs = 0;
        std::chrono::system_clock::time_point receivedAt{};
        Clock::time_point receivedSteady{};
        Motion motion = Motion::Stationary;
    };

    const std::map<std::uint8_t, Position>& positions() const { return positions_; }

    enum class Status { NoPosition, Stale, Fresh };
    struct Snapshot {
        const Position* position = nullptr;
        std::chrono::milliseconds age{};
        Status status = Status::NoPosition;
    };

    explicit PositionTracker(std::optional<std::uint8_t> sysid = std::nullopt) : sysid_(sysid) {}

    Snapshot snapshot(std::uint8_t sysid, Clock::time_point now,
                      std::chrono::milliseconds timeout) const {
        const auto found = positions_.find(sysid);
        if (found == positions_.end()) return {};
        const auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - found->second.receivedSteady);
        return {&found->second, age, age >= timeout ? Status::Stale : Status::Fresh};
    }

    bool sawBootTimeRegression() const { return bootTimeRegression_; }

    // Keep parser state across datagrams for split frames; CRC failures are ignored.
    void ingest(const char* bytes, std::size_t size, std::ostream* output = nullptr) {
        for (std::size_t i = 0; i < size; ++i) {
            if (parser_.feed(static_cast<std::uint8_t>(bytes[i])) == MAVLINK_FRAMING_OK)
                update(parser_.message, output, Clock::now(), std::chrono::system_clock::now());
        }
    }

    void ingest(const char* bytes, std::size_t size, std::ostream& output) {
        ingest(bytes, size, &output);
    }

    bool update(const mavlink_message_t& message, std::ostream& output,
                Clock::time_point now = Clock::now(),
                std::chrono::system_clock::time_point received = std::chrono::system_clock::now()) {
        return update(message, &output, now, received);
    }

    static const char* motionName(Motion motion) {
        switch (motion) {
        case Motion::Stationary: return "DUNG_YEN";
        case Motion::TakingOff: return "CAT_CANH_DANG_LEN";
        default: return "DI_CHUYEN";
        }
    }

private:
    bool update(const mavlink_message_t& message, std::ostream* output,
                Clock::time_point now, std::chrono::system_clock::time_point received) {
        if (message.msgid != MAVLINK_MSG_ID_LOCAL_POSITION_NED ||
            (sysid_ && message.sysid != *sysid_)) return false;
        mavlink_local_position_ned_t ned{};
        mavlink_msg_local_position_ned_decode(&message, &ned);
        if (!std::isfinite(ned.x) || !std::isfinite(ned.y) || !std::isfinite(ned.z) ||
            !std::isfinite(ned.vx) || !std::isfinite(ned.vy) || !std::isfinite(ned.vz)) return false;

        const auto found = positions_.find(message.sysid);
        const bool first = found == positions_.end();
        // A substantial backward jump suggests a reboot. Unsigned subtraction
        // allows normal wraparound. Small out-of-order packets cannot refresh age.
        if (!first) {
            const std::uint32_t elapsed = ned.time_boot_ms - found->second.timeBootMs;
            if (elapsed > 0x80000000u) {
                if (found->second.timeBootMs - ned.time_boot_ms <= 1000) return false;
                bootTimeRegression_ = true;
            }
        }
        const Motion previous = first ? Motion::Stationary : found->second.motion;
        Position position;
        position.x = ned.x; position.y = ned.y; position.z = ned.z;
        position.vx = ned.vx; position.vy = ned.vy; position.vz = ned.vz;
        position.timeBootMs = ned.time_boot_ms;
        position.receivedAt = received;
        position.receivedSteady = now;
        const float speed = std::sqrt(ned.vx * ned.vx + ned.vy * ned.vy + ned.vz * ned.vz);
        // Hysteresis limits state changes caused by small velocity fluctuations.
        const float movingThreshold = previous == Motion::Stationary ? 0.25f : 0.15f;
        const float climbThreshold = previous == Motion::TakingOff ? -0.15f : -0.30f;
        if (ned.vz < climbThreshold) position.motion = Motion::TakingOff;
        else if (speed > movingThreshold) position.motion = Motion::Moving;
        positions_[message.sysid] = position;

        if (output && (first || previous != position.motion ||
                       now - lastPrinted_[message.sysid] >= std::chrono::seconds(1))) {
            lastPrinted_[message.sysid] = now;
            const auto epochMs = std::chrono::duration_cast<std::chrono::milliseconds>(received.time_since_epoch()).count();
            std::ostringstream line;
            line << std::fixed << std::setprecision(2)
                 << "[POSITION] SYSID=" << static_cast<unsigned>(message.sysid)
                 << " state=" << motionName(position.motion)
                 << " N=" << ned.x << " E=" << ned.y << " D=" << ned.z << " m"
                 << " vx=" << ned.vx << " vy=" << ned.vy << " vz=" << ned.vz << " m/s"
                 << " boot_ms=" << ned.time_boot_ms << " rx_unix_ms=" << epochMs << '\n';
            *output << line.str() << std::flush;
        }
        return true;
    }

    std::optional<std::uint8_t> sysid_;
    bool bootTimeRegression_ = false;
    MavlinkParser parser_{};
    std::map<std::uint8_t, Position> positions_;
    std::map<std::uint8_t, Clock::time_point> lastPrinted_;
};
