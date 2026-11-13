#include "../src/network.h"
#include "../src/mavlink_codec.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

struct LoadClient
{
    SOCKET socket = INVALID_SOCKET;
    MavlinkParser parser;
    std::uint64_t frames = 0;
    std::uint64_t frameBytes = 0;
    std::uint64_t heartbeats = 0;
    std::uint64_t crcErrors = 0;
    std::uint64_t signatureErrors = 0;
    bool disconnected = false;
};

int main(int argc, char **argv)
{
    std::cout << std::unitbuf;
    if (argc != 3)
    {
        std::cerr << "Usage: multi_client_load.exe <clients_1_to_10> <measure_s>\n";
        return 2;
    }
    char *endClients = nullptr;
    char *endSeconds = nullptr;
    long count = std::strtol(argv[1], &endClients, 10);
    long measureSeconds = std::strtol(argv[2], &endSeconds, 10);
    if (*argv[1] == '\0' || *endClients != '\0' || count < 1 || count > 10 ||
        *argv[2] == '\0' || *endSeconds != '\0' || measureSeconds < 1 || measureSeconds > 3600)
    {
        std::cerr << "clients=1..10, measure_s=1..3600\n";
        return 2;
    }

    Network::Runtime winsock;
    if (winsock.error() != 0)
    {
        std::cerr << "WSAStartup loi=" << winsock.error() << '\n';
        return 2;
    }

    std::vector<LoadClient> clients(static_cast<std::size_t>(count));
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(5760);

    for (std::size_t i = 0; i < clients.size(); ++i)
    {
        SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socket == INVALID_SOCKET ||
            connect(socket, reinterpret_cast<const sockaddr *>(&address), sizeof(address)) == SOCKET_ERROR ||
            !Network::setNonblocking(socket))
        {
            std::cerr << "Connect client " << i + 1 << " loi=" << WSAGetLastError() << '\n';
            Network::closeSocket(socket);
            for (auto &client : clients)
                Network::closeSocket(client.socket);
            return 2;
        }
        clients[i].socket = socket;
        std::cout << "CONNECTED client=" << i + 1 << '\n';
    }

    using Clock = std::chrono::steady_clock;
    const auto connectedAt = Clock::now();
    const auto measurementStart = connectedAt + std::chrono::seconds(10);
    const auto measurementEnd = measurementStart + std::chrono::seconds(measureSeconds);
    auto nextReport = measurementStart + std::chrono::seconds(5);
    bool ioError = false;
    std::cout << "CLIENT LOAD START clients=" << count
              << " warmup_s=10 measure_s=" << measureSeconds << '\n';

    char buffer[8192];
    while (Clock::now() < measurementEnd)
    {
        fd_set reads;
        FD_ZERO(&reads);
        for (const auto &client : clients)
            if (client.socket != INVALID_SOCKET)
                FD_SET(client.socket, &reads);
        timeval timeout{};
        timeout.tv_usec = 200000;
        int ready = select(0, &reads, nullptr, nullptr, &timeout);
        if (ready == SOCKET_ERROR)
        {
            std::cerr << "select loi=" << WSAGetLastError() << '\n';
            ioError = true;
            break;
        }
        for (std::size_t i = 0; i < clients.size(); ++i)
        {
            LoadClient &client = clients[i];
            if (client.socket == INVALID_SOCKET || !FD_ISSET(client.socket, &reads))
                continue;
            int n = recv(client.socket, buffer, sizeof(buffer), 0);
            if (n <= 0)
            {
                std::cout << "DISCONNECTED client=" << i + 1
                          << " recv=" << n
                          << " error=" << (n < 0 ? WSAGetLastError() : 0) << '\n';
                client.disconnected = true;
                Network::closeSocket(client.socket);
                continue;
            }
            for (int j = 0; j < n; ++j)
            {
                uint8_t status = client.parser.feed(static_cast<uint8_t>(buffer[j]));
                if (Clock::now() < measurementStart)
                    continue;
                if (status == MAVLINK_FRAMING_BAD_CRC)
                    ++client.crcErrors;
                else if (status == MAVLINK_FRAMING_BAD_SIGNATURE)
                    ++client.signatureErrors;
                else if (status == MAVLINK_FRAMING_OK)
                {
                    ++client.frames;
                    client.frameBytes += serialize(client.parser.message).size();
                    if (client.parser.message.msgid == MAVLINK_MSG_ID_HEARTBEAT)
                        ++client.heartbeats;
                }
            }
        }

        const auto now = Clock::now();
        if (now >= nextReport)
        {
            std::uint64_t minFrames = UINT64_MAX;
            std::uint64_t maxFrames = 0;
            std::size_t active = 0;
            for (const auto &client : clients)
            {
                minFrames = std::min(minFrames, client.frames);
                maxFrames = std::max(maxFrames, client.frames);
                if (client.socket != INVALID_SOCKET)
                    ++active;
            }
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now - measurementStart).count();
            std::cout << "CLIENT LOAD t=" << seconds
                      << "s active=" << active
                      << " min_frames=" << minFrames
                      << " max_frames=" << maxFrames << '\n';
            nextReport += std::chrono::seconds(5);
        }
    }

    std::uint64_t totalFrames = 0;
    std::uint64_t totalBytes = 0;
    std::uint64_t totalHb = 0;
    std::uint64_t totalErrors = 0;
    std::size_t disconnected = 0;
    bool pass = !ioError;
    for (std::size_t i = 0; i < clients.size(); ++i)
    {
        const LoadClient &client = clients[i];
        totalFrames += client.frames;
        totalBytes += client.frameBytes;
        totalHb += client.heartbeats;
        totalErrors += client.crcErrors + client.signatureErrors;
        if (client.disconnected)
            ++disconnected;
        if (client.disconnected || client.frames == 0 || client.heartbeats == 0 ||
            client.crcErrors != 0 || client.signatureErrors != 0)
            pass = false;
        std::cout << "client=" << i + 1
                  << " frames=" << client.frames
                  << " bytes=" << client.frameBytes
                  << " heartbeats=" << client.heartbeats
                  << " crc_errors=" << client.crcErrors
                  << " signature_errors=" << client.signatureErrors
                  << " disconnected=" << client.disconnected << '\n';
    }
    for (auto &client : clients)
        Network::closeSocket(client.socket);
    std::cout << "CLIENT LOAD RESULT clients=" << count
              << " measure_s=" << measureSeconds
              << " total_frames=" << totalFrames
              << " total_bytes=" << totalBytes
              << " total_heartbeats=" << totalHb
              << " errors=" << totalErrors
              << " disconnected=" << disconnected
              << " pass=" << pass << '\n';
    return pass ? 0 : 1;
}
