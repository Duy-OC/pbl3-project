#include "../src/common/network.h"
#include "../src/common/mavlink_codec.h"

#include <iostream>
#include <string>

// Gui cung mot frame MAVLink co CRC sai qua UDP va TCP toi Gateway local.
// Chi kiem viec gui; nguoi do doi chieu counter trong gateway.log.
int main()
{
    std::cout << std::unitbuf;
    Network::Runtime winsock;
    if (winsock.error() != 0)
    {
        std::cerr << "WSAStartup loi=" << winsock.error() << '\n';
        return 2;
    }

    mavlink_message_t heartbeat{};
    mavlink_msg_heartbeat_pack(
        42, 1, &heartbeat,
        MAV_TYPE_QUADROTOR,
        MAV_AUTOPILOT_PX4,
        0, 0, MAV_STATE_STANDBY);
    std::string wire = serialize(heartbeat);
    if (wire.size() < 3)
    {
        std::cerr << "Khong ma hoa duoc heartbeat\n";
        return 2;
    }
    // Byte cuoi la checksum high khi frame khong signed.
    wire[wire.size() - 1] ^= 0x01;
    MavlinkParser localCheck;
    bool badCrcSeen = false;
    for (unsigned char byte : wire)
    {
        if (localCheck.feed(byte) == MAVLINK_FRAMING_BAD_CRC)
            badCrcSeen = true;
    }
    if (!badCrcSeen)
    {
        std::cerr << "Frame thu khong bi parser xac dinh la CRC sai\n";
        return 2;
    }

    sockaddr_in udpAddress{};
    udpAddress.sin_family = AF_INET;
    udpAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    udpAddress.sin_port = htons(14550);
    SOCKET udp = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp == INVALID_SOCKET)
    {
        std::cerr << "UDP socket loi=" << WSAGetLastError() << '\n';
        return 2;
    }
    int udpSent = sendto(
        udp, wire.data(), static_cast<int>(wire.size()), 0,
        reinterpret_cast<const sockaddr *>(&udpAddress), sizeof(udpAddress));
    int udpError = udpSent < 0 ? WSAGetLastError() : 0;
    Network::closeSocket(udp);
    if (udpSent != static_cast<int>(wire.size()))
    {
        std::cerr << "UDP sendto loi=" << udpError << '\n';
        return 2;
    }

    sockaddr_in tcpAddress{};
    tcpAddress.sin_family = AF_INET;
    tcpAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    tcpAddress.sin_port = htons(5760);
    SOCKET tcp = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp == INVALID_SOCKET)
    {
        std::cerr << "TCP socket loi=" << WSAGetLastError() << '\n';
        return 2;
    }
    if (connect(tcp, reinterpret_cast<const sockaddr *>(&tcpAddress), sizeof(tcpAddress)) == SOCKET_ERROR)
    {
        std::cerr << "TCP connect loi=" << WSAGetLastError() << '\n';
        Network::closeSocket(tcp);
        return 2;
    }
    std::size_t tcpSent = 0;
    while (tcpSent < wire.size())
    {
        int n = send(tcp, wire.data() + tcpSent,
                     static_cast<int>(wire.size() - tcpSent), 0);
        if (n <= 0)
        {
            std::cerr << "TCP send loi=" << WSAGetLastError() << '\n';
            Network::closeSocket(tcp);
            return 2;
        }
        tcpSent += static_cast<std::size_t>(n);
    }
    Sleep(500);
    Network::closeSocket(tcp);

    std::cout << "STATS CRC PROBE: msgid=0 sysid=42 frame_bytes=" << wire.size()
              << " udp_bytes_sent=" << udpSent
              << " tcp_bytes_sent=" << tcpSent
              << " crc_bytes_corrupted=1\n";
    std::cout << "SEND PASS: doc gateway.log, doi CRC errors=2 (UDP=1, TCP=1)\n";
    return 0;
}
