// PBL3-05: bao toan frame, CRC hong va signature bytes; chi in console.
#include "../src/mavlink_codec.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

static int fail(const char *reason)
{
    std::cerr << "MAVLINK INTEGRITY FAIL: " << reason << '\n';
    return 1;
}

int main()
{
    mavlink_message_t message{};
    mavlink_msg_heartbeat_pack(1, 1, &message,
                               MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_PX4,
                               0, 0, MAV_STATE_ACTIVE);

    // MAVLink 2 pack() tu trim byte 0 cuoi. Dung frame hop le co len=9
    // va tinh lai CRC de kiem tra forwarding cua frame chua duoc trim.
    _MAV_PAYLOAD_NON_CONST(&message)[8] = 0;
    message.len = MAVLINK_MSG_ID_HEARTBEAT_LEN;
    uint8_t header[MAVLINK_MAX_HEADER_LEN];
    const std::size_t headerLength =
        mavlink_header_to_send_buffer(header, &message);
    if (headerLength == 0)
        return fail("khong tao duoc header");
    uint16_t checksum = crc_calculate(&header[1], headerLength - 1);
    crc_accumulate_buffer(&checksum, _MAV_PAYLOAD(&message), message.len);
    crc_accumulate(MAVLINK_MSG_ID_HEARTBEAT_CRC, &checksum);
    message.checksum = checksum;
    if (message.len != 9 || _MAV_PAYLOAD(&message)[8] != 0)
        return fail("byte 0 cuoi payload bi cat");

    const std::string original = serialize(message);
    MavlinkParser validParser;
    int accepted = 0;
    for (unsigned char byte : original)
        if (validParser.feed(byte) == MAVLINK_FRAMING_OK)
            ++accepted;
    if (accepted != 1 || serialize(validParser.message) != original)
        return fail("frame hop le khong round-trip byte dung");

    if (headerLength >= original.size())
        return fail("header length khong hop le");
    std::string damaged = original;
    damaged[headerLength] ^= 0x01;
    MavlinkParser damagedParser;
    int badCrc = 0, acceptedDamaged = 0;
    for (unsigned char byte : damaged)
    {
        const uint8_t status = damagedParser.feed(byte);
        if (status == MAVLINK_FRAMING_BAD_CRC) ++badCrc;
        if (status == MAVLINK_FRAMING_OK) ++acceptedDamaged;
    }
    if (badCrc != 1 || acceptedDamaged != 0)
        return fail("frame sua payload khong bi loai vi CRC");

    // Kiem serialize giu nguyen 13 byte signature block; khong xac thuc khoa.
    message.incompat_flags |= MAVLINK_IFLAG_SIGNED;
    for (std::size_t i = 0; i < MAVLINK_SIGNATURE_BLOCK_LEN; ++i)
        message.signature[i] = static_cast<uint8_t>(i + 1);
    const std::string withSignature = serialize(message);
    if (withSignature.size() != original.size() + MAVLINK_SIGNATURE_BLOCK_LEN ||
        std::memcmp(withSignature.data() + withSignature.size() - MAVLINK_SIGNATURE_BLOCK_LEN,
                    message.signature, MAVLINK_SIGNATURE_BLOCK_LEN) != 0)
        return fail("signature bytes khong duoc giu nguyen");

    std::cout << "MAVLINK INTEGRITY METRICS: payload_bytes="
              << static_cast<unsigned>(message.len)
              << " wire_bytes=" << original.size()
              << " accepted_valid=" << accepted
              << " rejected_bad_crc=" << badCrc
              << " accepted_damaged=" << acceptedDamaged
              << " signature_bytes=" << MAVLINK_SIGNATURE_BLOCK_LEN
              << " signed_wire_bytes=" << withSignature.size() << '\n';
    std::cout << "MAVLINK INTEGRITY PASS: payload byte 0, round-trip, CRC, signature bytes\n";
    return 0;
}
