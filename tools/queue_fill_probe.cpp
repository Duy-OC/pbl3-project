#include "../src/network.h"
#include "../src/mavlink_codec.h"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>

// Gui 513 lenh chi yeu cau thong tin. Gateway giu UDP trong che do test,
// nen lenh thu 513 phai gap gioi han queue 512 va client bi ngat.
int main()
{
    std::cout << std::unitbuf;
    Network::Runtime winsock;
    if (winsock.error() != 0)
    {
        std::cerr << "WSAStartup error=" << winsock.error() << '\n';
        return 2;
    }

    SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket == INVALID_SOCKET)
    {
        std::cerr << "socket error=" << WSAGetLastError() << '\n';
        return 2;
    }

    sockaddr_in gateway{};
    gateway.sin_family = AF_INET;
    gateway.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    gateway.sin_port = htons(5760);
    if (connect(socket, reinterpret_cast<const sockaddr *>(&gateway),
                sizeof(gateway)) == SOCKET_ERROR)
    {
        std::cerr << "connect error=" << WSAGetLastError() << '\n';
        Network::closeSocket(socket);
        return 2;
    }

    constexpr int frameCount = 513;
    std::string wire;
    for (int i = 0; i < frameCount; ++i)
    {
        mavlink_message_t message{};
        mavlink_msg_command_long_pack(
            255, 191, &message, 1, 1,
            MAV_CMD_REQUEST_MESSAGE, 0,
            static_cast<float>(MAVLINK_MSG_ID_AUTOPILOT_VERSION),
            0, 0, 0, 0, 0, 0);
        const std::string frame = serialize(message);
        if (frame.empty())
        {
            std::cerr << "serialize failed at frame=" << i + 1 << '\n';
            Network::closeSocket(socket);
            return 2;
        }
        wire += frame;
    }

    std::size_t sent = 0;
    while (sent < wire.size())
    {
        const int n = send(socket, wire.data() + sent,
                           static_cast<int>(wire.size() - sent), 0);
        if (n <= 0)
        {
            std::cerr << "send error=" << WSAGetLastError()
                      << " bytes_sent=" << sent << '\n';
            Network::closeSocket(socket);
            return 2;
        }
        sent += static_cast<std::size_t>(n);
    }
    std::cout << "QUEUE FILL: target_sysid=1 command=512 frames_sent="
              << frameCount << " bytes_sent=" << sent << '\n';

    std::uint64_t bytesReceived = 0;
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::seconds(30);
    char buffer[4096];
    while (std::chrono::steady_clock::now() < deadline)
    {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket, &reads);
        timeval timeout{};
        timeout.tv_usec = 250000;
        const int ready = select(0, &reads, nullptr, nullptr, &timeout);
        if (ready == SOCKET_ERROR)
        {
            std::cerr << "select error=" << WSAGetLastError() << '\n';
            Network::closeSocket(socket);
            return 2;
        }
        if (ready == 0)
            continue;

        const int n = recv(socket, buffer, sizeof(buffer), 0);
        if (n == 0)
        {
            std::cout << "QUEUE FILL RESULT: frames_sent=" << frameCount
                      << " bytes_sent=" << sent
                      << " tcp_bytes_received=" << bytesReceived
                      << " peer_closed=1\n";
            Network::closeSocket(socket);
            return 0;
        }
        if (n == SOCKET_ERROR)
        {
            std::cerr << "recv error=" << WSAGetLastError()
                      << " tcp_bytes_received=" << bytesReceived << '\n';
            Network::closeSocket(socket);
            return 2;
        }
        bytesReceived += static_cast<std::uint64_t>(n);
    }

    std::cout << "QUEUE FILL RESULT: frames_sent=" << frameCount
              << " bytes_sent=" << sent
              << " tcp_bytes_received=" << bytesReceived
              << " peer_closed=0 timeout_s=30\n";
    Network::closeSocket(socket);
    return 1;
}
