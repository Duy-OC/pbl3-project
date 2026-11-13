#include "../src/network.h"
#include "../src/mavlink_codec.h"

#include <chrono>
#include <cstdlib>
#include <iostream>

// Gui MAV_CMD_REQUEST_MESSAGE (AUTOPILOT_VERSION) toi mot SYSID qua TCP Gateway.
// Khong gui lenh bay. Dung gateway.log de xac nhan ROUTE/BLOCK.
int main(int argc, char **argv)
{
    std::cout << std::unitbuf;
    if (argc != 2 && argc != 3)
    {
        std::cerr << "Usage: route_probe.exe <target_sysid_1_to_255> [timeout_s_1_to_180]\n";
        return 2;
    }

    char *end = nullptr;
    long parsed = std::strtol(argv[1], &end, 10);
    if (*argv[1] == '\0' || *end != '\0' || parsed < 1 || parsed > 255)
    {
        std::cerr << "SYSID phai la so 1..255\n";
        return 2;
    }
    const uint8_t target = static_cast<uint8_t>(parsed);
    long timeoutSeconds = 5;
    if (argc == 3)
    {
        end = nullptr;
        timeoutSeconds = std::strtol(argv[2], &end, 10);
        if (*argv[2] == '\0' || *end != '\0' || timeoutSeconds < 1 || timeoutSeconds > 180)
        {
            std::cerr << "timeout_s phai la so 1..180\n";
            return 2;
        }
    }

    Network::Runtime winsock;
    if (winsock.error() != 0)
    {
        std::cerr << "WSAStartup loi=" << winsock.error() << '\n';
        return 2;
    }

    SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket == INVALID_SOCKET)
    {
        std::cerr << "socket loi=" << WSAGetLastError() << '\n';
        return 2;
    }

    sockaddr_in gateway{};
    gateway.sin_family = AF_INET;
    gateway.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    gateway.sin_port = htons(5760);
    if (connect(socket, reinterpret_cast<const sockaddr *>(&gateway), sizeof(gateway)) == SOCKET_ERROR)
    {
        std::cerr << "connect loi=" << WSAGetLastError() << '\n';
        Network::closeSocket(socket);
        return 2;
    }

    mavlink_message_t command{};
    mavlink_msg_command_long_pack(
        255, 191, &command, target, 1,
        MAV_CMD_REQUEST_MESSAGE, 0,
        static_cast<float>(MAVLINK_MSG_ID_AUTOPILOT_VERSION),
        0, 0, 0, 0, 0, 0);
    const std::string wire = serialize(command);
    if (wire.empty())
    {
        std::cerr << "Khong ma hoa duoc MAVLink command\n";
        Network::closeSocket(socket);
        return 2;
    }

    std::size_t sent = 0;
    while (sent < wire.size())
    {
        int n = send(socket, wire.data() + sent, static_cast<int>(wire.size() - sent), 0);
        if (n <= 0)
        {
            std::cerr << "send loi=" << WSAGetLastError() << '\n';
            Network::closeSocket(socket);
            return 2;
        }
        sent += static_cast<std::size_t>(n);
    }

    std::cout << "ROUTE PROBE: target_sysid=" << static_cast<int>(target)
              << " command=512 request_msgid=" << MAVLINK_MSG_ID_AUTOPILOT_VERSION
              << " bytes_sent=" << sent << '\n';

    MavlinkParser parser;
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::seconds(timeoutSeconds);
    char buffer[4096];
    while (std::chrono::steady_clock::now() < deadline)
    {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket, &reads);
        timeval timeout{};
        timeout.tv_usec = 250000;
        int ready = select(0, &reads, nullptr, nullptr, &timeout);
        if (ready == SOCKET_ERROR)
        {
            std::cerr << "select loi=" << WSAGetLastError() << '\n';
            Network::closeSocket(socket);
            return 2;
        }
        if (ready == 0)
            continue;

        int n = recv(socket, buffer, sizeof(buffer), 0);
        if (n <= 0)
        {
            std::cerr << "recv dong/loi=" << (n < 0 ? WSAGetLastError() : 0) << '\n';
            Network::closeSocket(socket);
            return 2;
        }
        for (int i = 0; i < n; ++i)
        {
            if (parser.feed(static_cast<uint8_t>(buffer[i])) != MAVLINK_FRAMING_OK)
                continue;
            const mavlink_message_t &message = parser.message;
            if (message.sysid != target || message.msgid != MAVLINK_MSG_ID_COMMAND_ACK)
                continue;
            mavlink_command_ack_t ack{};
            mavlink_msg_command_ack_decode(&message, &ack);
            if (ack.command != MAV_CMD_REQUEST_MESSAGE)
                continue;
            std::cout << "ACK: sysid=" << static_cast<int>(message.sysid)
                      << " command=" << ack.command
                      << " result=" << static_cast<int>(ack.result) << '\n';
            Network::closeSocket(socket);
            return ack.result == MAV_RESULT_ACCEPTED ? 0 : 1;
        }
    }

    std::cout << "TIMEOUT: no matching ACK from sysid="
              << static_cast<int>(target) << " within "
              << timeoutSeconds << " s\n";
    Network::closeSocket(socket);
    return 1;
}
