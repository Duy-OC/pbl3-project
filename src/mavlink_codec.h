#pragma once

#include <cstdint>
#include <cstring>
#include <string>

#include "../third_party/mavlink/development/mavlink.h"

struct MavlinkParser
{
    mavlink_message_t receiving{};
    mavlink_status_t state{};

    mavlink_message_t message{};
    mavlink_status_t report{};

    uint8_t feed(uint8_t byte)
    {
        return mavlink_frame_char_buffer(
            &receiving,
            &state,
            byte,
            &message,
            &report);
    }
};

// Giu nguyen payload length, CRC va signature khi forwarding.
inline std::string serialize(
    const mavlink_message_t &message)
{
    uint8_t bytes[MAVLINK_MAX_PACKET_LEN];

    const uint8_t headerLength =
        mavlink_header_to_send_buffer(bytes, &message);

    if (headerLength == 0)
    {
        return {};
    }

    std::size_t length = headerLength;

    std::memcpy(
        bytes + length,
        _MAV_PAYLOAD(&message),
        message.len);

    length += message.len;

    bytes[length++] =
        static_cast<uint8_t>(message.checksum & 0xff);

    bytes[length++] =
        static_cast<uint8_t>(message.checksum >> 8);

    if (message.magic != MAVLINK_STX_MAVLINK1 &&
        (message.incompat_flags & MAVLINK_IFLAG_SIGNED))
    {

        std::memcpy(
            bytes + length,
            message.signature,
            MAVLINK_SIGNATURE_BLOCK_LEN);

        length += MAVLINK_SIGNATURE_BLOCK_LEN;
    }

    return std::string(
        reinterpret_cast<const char *>(bytes),
        length);
}

// 0: broadcast.
// >0: sysid dich.
// -1: khong co metadata.
inline int getTargetSystem(
    const mavlink_message_t &message)
{
    const mavlink_msg_entry_t *entry =
        mavlink_get_msg_entry(message.msgid);

    if (!entry)
    {
        return -1;
    }

    if (!(entry->flags &
          MAV_MSG_ENTRY_FLAG_HAVE_TARGET_SYSTEM))
    {
        return 0;
    }

    // MAVLink 2 co the cat cac byte 0 o cuoi payload.
    if (entry->target_system_ofs >= message.len)
    {
        return 0;
    }

    return static_cast<uint8_t>(
        _MAV_PAYLOAD(&message)[entry->target_system_ofs]);
}

inline bool isCommand(
    const mavlink_message_t &message)
{
    return message.msgid == MAVLINK_MSG_ID_COMMAND_LONG ||
           message.msgid == MAVLINK_MSG_ID_COMMAND_INT;
}