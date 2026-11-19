// A2: nhan MAVLink truc tiep tu PX4 va chi in thong ke ra console.
// Khong tao file ket qua. Chay sau khi dong QGC va Gateway.
#include "../src/common/network.h"
#include "../src/common/mavlink_codec.h"
#include "../src/common/time_utils.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>

struct Source
{
    MavlinkParser parser;
};

struct Heartbeat
{
    TimePoint last{};
    bool seen = false;
    std::uint64_t count = 0;
    std::uint64_t intervals = 0;
    double sumMs = 0;
    double sumSqMs = 0;
};

static bool parseDuration(int argc, char **argv, int &seconds)
{
    if (argc != 2)
        return false;
    char *end = nullptr;
    long value = std::strtol(argv[1], &end, 10);
    if (end == argv[1] || *end != '\0' || (value != 60 && value != 600))
        return false;
    seconds = static_cast<int>(value);
    return true;
}

int main(int argc, char **argv)
{
    int seconds = 0;
    if (!parseDuration(argc, argv, seconds))
    {
        std::cerr << "Usage: measure_udp.exe 60|600\n";
        return 1;
    }

    Network::Runtime winsock;
    if (winsock.error() != 0)
    {
        std::cerr << "WSAStartup loi: " << winsock.error() << '\n';
        return 1;
    }
    SOCKET udp = Network::createBoundSocket(SOCK_DGRAM, 14550);
    if (udp == INVALID_SOCKET)
        return 1;

    std::map<std::pair<unsigned long, unsigned short>, Source> sources;
    std::map<std::uint32_t, std::uint64_t> msgids;
    std::map<std::pair<int, int>, Heartbeat> heartbeats;
    std::set<std::pair<unsigned long, unsigned short>> measuredSources;
    std::set<std::pair<int, int>> identities;
    std::uint64_t frames = 0, bytes = 0, crcErrors = 0, signatureErrors = 0;
    bool started = false;
    TimePoint begin{}, measureStart{}, measureEnd{};

    std::cout << "A2: cho MAVLink tren UDP 14550; bo 10 s tu frame hop le dau tien, "
              << "sau do do " << seconds << " s. Ctrl+C de huy.\n";
    while (true)
    {
        const TimePoint now = Clock::now();
        if (started && now >= measureEnd)
            break;
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(udp, &reads);
        timeval timeout{};
        timeout.tv_usec = 200000;
        const int ready = select(0, &reads, nullptr, nullptr, &timeout);
        if (ready == SOCKET_ERROR)
        {
            std::cerr << "select loi: " << WSAGetLastError() << '\n';
            Network::closeSocket(udp);
            return 1;
        }
        if (ready == 0)
            continue;

        char buffer[65535];
        sockaddr_in peer{};
        int peerLength = sizeof(peer);
        const int received = recvfrom(udp, buffer, sizeof(buffer), 0,
                                      reinterpret_cast<sockaddr *>(&peer), &peerLength);
        if (received == SOCKET_ERROR)
        {
            const int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK)
                continue;
            std::cerr << "recvfrom loi: " << error << '\n';
            Network::closeSocket(udp);
            return 1;
        }

        const TimePoint arrived = Clock::now();
        const auto endpoint = std::make_pair(peer.sin_addr.s_addr, peer.sin_port);
        Source &source = sources[endpoint];
        for (int i = 0; i < received; ++i)
        {
            const uint8_t result = source.parser.feed(static_cast<uint8_t>(buffer[i]));
            const bool inWindow = started && arrived >= measureStart && arrived < measureEnd;
            if (result == MAVLINK_FRAMING_BAD_CRC)
            {
                if (inWindow) ++crcErrors;
                continue;
            }
            if (result == MAVLINK_FRAMING_BAD_SIGNATURE)
            {
                if (inWindow) ++signatureErrors;
                continue;
            }
            if (result != MAVLINK_FRAMING_OK)
                continue;

            if (!started)
            {
                started = true;
                begin = arrived;
                measureStart = begin + std::chrono::seconds(10);
                measureEnd = measureStart + std::chrono::seconds(seconds);
                std::cout << "Da nhan frame hop le. Warm-up 10 s bat dau.\n";
            }
            if (arrived < measureStart || arrived >= measureEnd)
                continue;

            const mavlink_message_t &message = source.parser.message;
            ++frames;
            bytes += serialize(message).size();
            ++msgids[message.msgid];
            measuredSources.insert(endpoint);
            const auto identity = std::make_pair(static_cast<int>(message.sysid),
                                                 static_cast<int>(message.compid));
            identities.insert(identity);
            if (message.msgid == MAVLINK_MSG_ID_HEARTBEAT)
            {
                Heartbeat &hb = heartbeats[identity];
                ++hb.count;
                if (hb.seen)
                {
                    const double gapMs = elapsedSeconds(arrived, hb.last) * 1000.0;
                    ++hb.intervals;
                    hb.sumMs += gapMs;
                    hb.sumSqMs += gapMs * gapMs;
                }
                hb.last = arrived;
                hb.seen = true;
            }
        }
    }
    Network::closeSocket(udp);

    std::cout << std::fixed << std::setprecision(2)
              << "\nA2 RESULT (sao chep toan bo phan nay)\n"
              << "measure_s=" << seconds << " warmup_s=10\n"
              << "frames=" << frames << " bytes=" << bytes
              << " frames_per_s=" << static_cast<double>(frames) / seconds
              << " bytes_per_s=" << static_cast<double>(bytes) / seconds
              << " bytes_per_frame=" << (frames ? static_cast<double>(bytes) / frames : 0.0)
              << "\ncrc_errors=" << crcErrors
              << " signature_errors=" << signatureErrors << '\n';
    for (const auto &entry : measuredSources)
    {
        in_addr address{};
        address.s_addr = entry.first;
        std::cout << "source=" << inet_ntoa(address) << ':' << ntohs(entry.second) << '\n';
    }
    for (const auto &id : identities)
        std::cout << "sysid=" << id.first << " compid=" << id.second << '\n';
    for (const auto &entry : heartbeats)
    {
        const Heartbeat &hb = entry.second;
        const double mean = hb.intervals ? hb.sumMs / hb.intervals : 0.0;
        const double variance = hb.intervals
            ? std::max(0.0, hb.sumSqMs / hb.intervals - mean * mean) : 0.0;
        std::cout << "heartbeat sysid=" << entry.first.first
                  << " compid=" << entry.first.second
                  << " count=" << hb.count << " mean_ms=" << mean
                  << " jitter_ms=" << std::sqrt(variance) << '\n';
    }
    for (const auto &entry : msgids)
        std::cout << "msgid=" << entry.first << " frames=" << entry.second << '\n';
    return frames ? 0 : 2;
}
