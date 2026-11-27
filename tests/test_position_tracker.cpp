#include "../src/proxy/position_tracker.h"
#include <iostream>
#include <limits>
#include <stdexcept>

static void check(bool ok, const char* reason) {
    if (!ok) throw std::runtime_error(reason);
}

static mavlink_message_t sample(std::uint8_t id, float z, float vx, float vz) {
    mavlink_message_t result{};
    mavlink_msg_local_position_ned_pack(id, 1, &result, 1234, 12.5f, -3.0f, z, vx, 0, vz);
    return result;
}

int main() {
    try {
        PositionTracker tracker;
        std::ostringstream output;
        auto stationary = sample(1, 0, 0, 0);
        const auto wire = serialize(stationary);
        tracker.ingest(wire.data(), 7, output);
        check(tracker.positions().empty(), "Partial frame accepted");
        tracker.ingest(wire.data() + 7, wire.size() - 7, output);
        check(tracker.positions().at(1).x == 12.5f && tracker.positions().at(1).y == -3,
              "Position decode failed");
        const auto now = PositionTracker::Clock::now();
        const auto wall = std::chrono::system_clock::now();
        const auto climb = sample(1, -2, 0, -1);
        tracker.update(climb, output, now, wall);
        check(tracker.positions().at(1).motion == PositionTracker::Motion::TakingOff, "Climb not detected");
        check(tracker.positions().at(1).receivedAt == wall && tracker.positions().at(1).receivedSteady == now &&
              tracker.positions().at(1).timeBootMs == 1234, "Receive/boot timestamp missing");
        auto move = sample(2, -5, 2, 0);
        const auto joined = serialize(move) + serialize(sample(3, -5, 0, 0));
        tracker.ingest(joined.data(), joined.size(), output);
        check(tracker.positions().size() == 3, "Multiple SYSIDs not retained");
        check(tracker.positions().at(2).motion == PositionTracker::Motion::Moving, "Movement not detected");
        check(tracker.positions().at(3).motion == PositionTracker::Motion::Stationary, "Hover not stationary");
        tracker.update(sample(1, -2, 0, 0), output, now, wall);
        const auto previousSize = output.str().size();
        tracker.update(sample(1, -2, 0, 0), output, now + std::chrono::milliseconds(100), wall);
        check(output.str().size() == previousSize, "Repeated log was not throttled");
        tracker.update(sample(1, -2, 0, 0), output, now + std::chrono::seconds(1), wall);
        check(output.str().size() > previousSize, "Periodic stationary log missing");
        check(!tracker.update(sample(1, std::numeric_limits<float>::quiet_NaN(), 0, 0), output), "NaN accepted");
        check(tracker.positions().at(1).z == -2, "Invalid sample overwrote valid position");

        auto broken = serialize(sample(4, 0, 0, 0));
        broken[broken.size() - 1] ^= 1;
        tracker.ingest(broken.data(), broken.size(), output);
        check(tracker.positions().count(4) == 0, "Bad CRC accepted");
        // Exercise MAVLink 1 with a complete, zero-extended payload.
        auto v1 = sample(5, -3, 1, 0);
        v1.magic = MAVLINK_STX_MAVLINK1;
        v1.len = MAVLINK_MSG_ID_LOCAL_POSITION_NED_LEN;
        mavlink_status_t status{};
        status.flags = MAVLINK_STATUS_FLAG_OUT_MAVLINK1;
        mavlink_finalize_message_buffer(&v1, 5, 1, &status,
            MAVLINK_MSG_ID_LOCAL_POSITION_NED_MIN_LEN, MAVLINK_MSG_ID_LOCAL_POSITION_NED_LEN,
            MAVLINK_MSG_ID_LOCAL_POSITION_NED_CRC);
        const auto v1wire = serialize(v1);
        tracker.ingest(v1wire.data(), v1wire.size(), output);
        check(tracker.positions().at(5).z == -3, "MAVLink 1 decode failed");
        std::cout << "PASS: MAVLink v1/v2, split/joined frames, CRC, SYSID, timestamps, motion, log interval, NaN\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
