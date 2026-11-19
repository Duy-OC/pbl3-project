// PBL3-04: kiem parser MAVLink voi frame chia/gop, chi in ket qua ra console.
#include "../src/common/mavlink_codec.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

static std::string heartbeat(uint8_t sysid, uint8_t compid)
{
    mavlink_message_t message{};
    mavlink_msg_heartbeat_pack(sysid, compid, &message,
                               MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_PX4,
                               0, 0, MAV_STATE_ACTIVE);
    return serialize(message);
}

int main()
{
    const std::string first = heartbeat(1, 1);
    const std::string second = heartbeat(2, 1);
    if (first.empty() || second.empty())
    {
        std::cerr << "MAVLINK PARSER FAIL: khong tao duoc frame\n";
        return 1;
    }

    MavlinkParser splitParser;
    const std::size_t cut = first.size() / 2;
    int splitCount = 0;
    for (std::size_t i = 0; i < cut; ++i)
        if (splitParser.feed(static_cast<uint8_t>(first[i])) == MAVLINK_FRAMING_OK)
            ++splitCount;
    if (splitCount != 0)
    {
        std::cerr << "MAVLINK PARSER FAIL: frame chua du byte da duoc nhan\n";
        return 1;
    }
    for (std::size_t i = cut; i < first.size(); ++i)
        if (splitParser.feed(static_cast<uint8_t>(first[i])) == MAVLINK_FRAMING_OK)
            ++splitCount;
    if (splitCount != 1 || splitParser.message.sysid != 1 ||
        splitParser.message.compid != 1 ||
        splitParser.message.msgid != MAVLINK_MSG_ID_HEARTBEAT)
    {
        std::cerr << "MAVLINK PARSER FAIL: frame chia bi doc sai\n";
        return 1;
    }

    MavlinkParser joinedParser;
    const std::string joined = first + second;
    std::vector<int> sysids;
    for (unsigned char byte : joined)
        if (joinedParser.feed(byte) == MAVLINK_FRAMING_OK)
        {
            if (joinedParser.message.msgid != MAVLINK_MSG_ID_HEARTBEAT ||
                joinedParser.message.compid != 1)
            {
                std::cerr << "MAVLINK PARSER FAIL: sai msgid/compid\n";
                return 1;
            }
            sysids.push_back(joinedParser.message.sysid);
        }
    if (sysids.size() != 2 || sysids[0] != 1 || sysids[1] != 2)
    {
        std::cerr << "MAVLINK PARSER FAIL: frame gop bi doc sai\n";
        return 1;
    }

    std::cout << "MAVLINK PARSER METRICS: first_wire_bytes=" << first.size()
              << " second_wire_bytes=" << second.size()
              << " split_at_byte=" << cut
              << " frames_after_first_half=0"
              << " frames_after_full=" << splitCount
              << " joined_wire_bytes=" << joined.size()
              << " joined_frames=" << sysids.size()
              << " sysids=" << sysids[0] << ',' << sysids[1]
              << " compid=1 msgid=" << MAVLINK_MSG_ID_HEARTBEAT << '\n';
    std::cout << "MAVLINK PARSER PASS: frame chia/gop, sysid, compid, msgid\n";
    return 0;
}
