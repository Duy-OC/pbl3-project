#include "network.h"
#include "mavlink_codec.h"
#include "time_utils.h"
#include "config.h"
#include "qos.h"

#include <iostream>

int main()
{
    Network::Runtime winsock;

    if (winsock.error() != 0)
    {
        std::cerr
            << "WSAStartup loi: "
            << winsock.error() << '\n';

        return 1;
    }

    SOCKET socket = ::socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (socket == INVALID_SOCKET)
    {
        return 1;
    }

    sockaddr_in server{};

    server.sin_family = AF_INET;
    server.sin_port = htons(Config::TCP_PORT);
    server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(
            socket,
            reinterpret_cast<const sockaddr *>(&server),
            sizeof(server)) == SOCKET_ERROR)
    {

        std::cerr
            << "connect loi: "
            << WSAGetLastError() << '\n';

        Network::closeSocket(socket);
        return 1;
    }

    if (!Network::setNonblocking(socket))
    {
        Network::closeSocket(socket);
        return 1;
    }

    std::cout
        << "Da ket noi Gateway 127.0.0.1:"
        << Config::TCP_PORT << '\n';

    QosRecorder qos;

    if (!qos.open("client_rx"))
    {
        Network::closeSocket(socket);
        return 1;
    }

    MavlinkParser parser;

    unsigned long long frames = 0;
    unsigned long long bytes = 0;
    unsigned long long crc = 0;
    unsigned long long previousFrames = 0;

    TimePoint lastReport = Clock::now();

    char buffer[4096];
    int exitCode = 0;

    while (true)
    {
        fd_set reads;

        FD_ZERO(&reads);
        FD_SET(socket, &reads);

        timeval timeout{};
        timeout.tv_usec = Config::SELECT_TIMEOUT_US;

        int ready = select(
            0,
            &reads,
            nullptr,
            nullptr,
            &timeout);

        if (ready == SOCKET_ERROR)
        {
            exitCode = 1;
            break;
        }

        if (FD_ISSET(socket, &reads))
        {
            int n = recv(
                socket,
                buffer,
                sizeof(buffer),
                0);

            if (n == 0)
            {
                std::cout << "Gateway da dong ket noi\n";
                break;
            }

            if (n == SOCKET_ERROR)
            {
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    exitCode = 1;
                    break;
                }
            }
            else
            {
                bytes += n;

                for (int i = 0; i < n; ++i)
                {
                    uint8_t status = parser.feed(
                        static_cast<uint8_t>(buffer[i]));

                    if (status == MAVLINK_FRAMING_BAD_CRC)
                    {
                        ++crc;
                    }

                    if (status != MAVLINK_FRAMING_OK)
                    {
                        continue;
                    }

                    ++frames;

                    qos.onFrame(
                        parser.message,
                        serialize(parser.message).size(),
                        server);

                    if (parser.message.msgid ==
                        MAVLINK_MSG_ID_HEARTBEAT)
                    {

                        std::cout
                            << "HEARTBEAT SYSID="
                            << static_cast<int>(
                                   parser.message.sysid)
                            << " COMPID="
                            << static_cast<int>(
                                   parser.message.compid)
                            << '\n';
                    }
                }
            }
        }

        qos.tick();

        TimePoint now = Clock::now();
        double seconds = elapsedSeconds(now, lastReport);

        if (seconds >= Config::STATS_INTERVAL_SECONDS)
        {
            std::cout
                << "CLIENT STATS: frames=" << frames
                << " | bytes=" << bytes
                << " | CRC=" << crc
                << " | rate="
                << (frames - previousFrames) / seconds
                << " frames/s\n";

            previousFrames = frames;
            lastReport = now;
        }
    }

    Network::closeSocket(socket);

    return exitCode;
}