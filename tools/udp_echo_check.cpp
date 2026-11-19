// PBL3-02: kiem tra WinSock UDP tren loopback, khong tao file ket qua.
#include "../src/common/network.h"

#include <cstring>
#include <iostream>
#include <string>

static bool waitReadable(SOCKET socket)
{
    fd_set reads;
    FD_ZERO(&reads);
    FD_SET(socket, &reads);
    timeval timeout{};
    timeout.tv_sec = 2;
    return select(0, &reads, nullptr, nullptr, &timeout) == 1;
}

int main()
{
    Network::Runtime winsock;
    if (winsock.error() != 0)
    {
        std::cerr << "WSAStartup loi: " << winsock.error() << '\n';
        return 1;
    }

    SOCKET server = Network::createBoundSocket(SOCK_DGRAM, 0);
    SOCKET client = Network::createBoundSocket(SOCK_DGRAM, 0);
    if (server == INVALID_SOCKET || client == INVALID_SOCKET)
    {
        Network::closeSocket(server);
        Network::closeSocket(client);
        return 1;
    }

    int code = 1;
    int sentBytes = 0;
    int receivedBytes = 0;
    int replyBytes = 0;
    int echoedBytes = 0;
    unsigned short serverPort = 0;
    const std::string probe = "pbl3_udp_probe";
    do
    {
        sockaddr_in bound{};
        int boundSize = sizeof(bound);
        if (getsockname(server, reinterpret_cast<sockaddr *>(&bound),
                        &boundSize) == SOCKET_ERROR)
            break;
        serverPort = ntohs(bound.sin_port);
        sockaddr_in target{};
        target.sin_family = AF_INET;
        target.sin_port = bound.sin_port;
        target.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        sentBytes = sendto(client, probe.data(), static_cast<int>(probe.size()), 0,
                           reinterpret_cast<const sockaddr *>(&target), sizeof(target));
        if (sentBytes != static_cast<int>(probe.size()))
            break;
        if (!waitReadable(server))
            break;

        char buffer[256];
        sockaddr_in sender{};
        int senderSize = sizeof(sender);
        receivedBytes = recvfrom(server, buffer, sizeof(buffer), 0,
                                 reinterpret_cast<sockaddr *>(&sender), &senderSize);
        if (receivedBytes != static_cast<int>(probe.size()) ||
            std::memcmp(buffer, probe.data(), probe.size()) != 0)
            break;
        replyBytes = sendto(server, buffer, receivedBytes, 0,
                            reinterpret_cast<const sockaddr *>(&sender), senderSize);
        if (replyBytes != receivedBytes)
            break;
        if (!waitReadable(client))
            break;

        senderSize = sizeof(sender);
        echoedBytes = recvfrom(client, buffer, sizeof(buffer), 0,
                               reinterpret_cast<sockaddr *>(&sender), &senderSize);
        if (echoedBytes != static_cast<int>(probe.size()) ||
            std::memcmp(buffer, probe.data(), probe.size()) != 0)
            break;

        code = 0;
    } while (false);

    Network::closeSocket(server);
    Network::closeSocket(client);
    std::cout << "UDP ECHO METRICS: transport=127.0.0.1:" << serverPort
              << " payload_bytes=" << probe.size()
              << " sent_bytes=" << sentBytes
              << " server_received_bytes=" << receivedBytes
              << " reply_bytes=" << replyBytes
              << " client_received_bytes=" << echoedBytes << '\n';
    if (code == 0)
        std::cout << "UDP ECHO PASS: sendto, recvfrom, reply dung du lieu\n";
    else
        std::cerr << "UDP ECHO FAIL: loi socket/timeout/du lieu khong khop; WSA="
                  << WSAGetLastError() << '\n';
    return code;
}
