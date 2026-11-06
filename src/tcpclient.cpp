#include "network.h"
#include "config.h"
#include "mavlink_codec.h"
#include "time_utils.h"
#include "qos.h"

#include <cstdint>
#include <iostream>

int main()
{
    // 1. Khoi tao WinSock.
    Network::Runtime winsock;

    if (winsock.error() != 0)
    {
        std::cerr
            << "WSAStartup loi: "
            << winsock.error()
            << '\n';

        return 1;
    }

    // 2. Tao socket TCP.
    SOCKET clientSocket =
        ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr
            << "Tao socket loi: "
            << WSAGetLastError()
            << '\n';

        return 1;
    }

    // 3. Dia chi Gateway tren cung may Windows.
    sockaddr_in server{};

    server.sin_family = AF_INET;
    server.sin_port = htons(Config::TCP_PORT);
    server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // 4. Ket noi Gateway.
    if (connect(
            clientSocket,
            reinterpret_cast<const sockaddr *>(&server),
            sizeof(server)) == SOCKET_ERROR)
    {

        std::cerr
            << "Ket noi Gateway loi: "
            << WSAGetLastError()
            << '\n';

        Network::closeSocket(clientSocket);
        return 1;
    }

    // 5. Chuyen sang nonblocking sau khi ket noi.
    if (!Network::setNonblocking(clientSocket))
    {
        std::cerr
            << "Dat nonblocking loi: "
            << WSAGetLastError()
            << '\n';

        Network::closeSocket(clientSocket);
        return 1;
    }

    std::cout
        << "=== PBL3 TCP CLIENT + QOS ===\n"
        << "Gateway: 127.0.0.1:"
        << Config::TCP_PORT
        << '\n'
        << "Heartbeat timeout: 3 giay\n"
        << "Warm-up: 10 giay | Do: 600 giay\n\n";

    // 6. Mo hai file CSV cho client.
    QosRecorder qos;

    if (!qos.open("client_rx", 3.0))
    {
        std::cerr
            << "Khong khoi tao duoc QoS.\n"
            << "Hay tao thu muc docs/ketqua "
            << "va chay tu thu muc goc du an.\n";

        Network::closeSocket(clientSocket);
        return 1;
    }

    // Parser phai ton tai xuyen suot ket noi:
    // mot frame co the bi chia qua nhieu lan recv().
    MavlinkParser parser;

    // Thong ke van hanh tu luc client bat dau nhan.
    unsigned long long frames = 0;
    unsigned long long receivedBytes = 0;
    unsigned long long crcErrors = 0;
    unsigned long long signatureErrors = 0;

    unsigned long long previousFrames = 0;
    unsigned long long previousBytes = 0;

    TimePoint lastReport = Clock::now();

    char buffer[4096];
    int exitCode = 0;

    // 7. Vong lap nhan du lieu.
    while (true)
    {
        fd_set reads;

        FD_ZERO(&reads);
        FD_SET(clientSocket, &reads);

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
            std::cerr
                << "select loi: "
                << WSAGetLastError()
                << '\n';

            exitCode = 1;
            break;
        }

        if (FD_ISSET(clientSocket, &reads))
        {
            int received = recv(
                clientSocket,
                buffer,
                static_cast<int>(sizeof(buffer)),
                0);

            if (received == 0)
            {
                std::cout
                    << "Gateway da dong ket noi TCP.\n";

                break;
            }

            if (received == SOCKET_ERROR)
            {
                int error = WSAGetLastError();

                if (error != WSAEWOULDBLOCK)
                {
                    std::cerr
                        << "recv loi: "
                        << error
                        << '\n';

                    exitCode = 1;
                    break;
                }
            }
            else
            {
                receivedBytes +=
                    static_cast<unsigned long long>(received);

                // TCP la luong byte:
                // dua tung byte vao parser MAVLink.
                for (int i = 0; i < received; ++i)
                {
                    uint8_t status = parser.feed(
                        static_cast<uint8_t>(buffer[i]));

                    if (status == MAVLINK_FRAMING_BAD_CRC)
                    {
                        ++crcErrors;
                        continue;
                    }

                    if (status == MAVLINK_FRAMING_BAD_SIGNATURE)
                    {
                        ++signatureErrors;
                        continue;
                    }

                    if (status != MAVLINK_FRAMING_OK)
                    {
                        continue;
                    }

                    ++frames;

                    const mavlink_message_t &message =
                        parser.message;

                    // Chi ghi QoS sau khi ghep du frame hop le.
                    qos.onFrame(
                        message,
                        serialize(message).size(),
                        server,
                        Clock::now());

                    // Chi in heartbeat de tranh qua nhieu log.
                    if (message.msgid ==
                        MAVLINK_MSG_ID_HEARTBEAT)
                    {

                        std::cout
                            << "HEARTBEAT"
                            << " | SYSID="
                            << static_cast<int>(message.sysid)
                            << " | COMPID="
                            << static_cast<int>(message.compid)
                            << " | SEQ="
                            << static_cast<int>(message.seq)
                            << '\n';
                    }
                }
            }
        }

        // Phai chay ca khi khong nhan duoc du lieu:
        // phat hien heartbeat timeout va ghi CSV dinh ky.
        TimePoint now = Clock::now();
        qos.tick(now);

        double seconds =
            elapsedSeconds(now, lastReport);

        if (seconds >= Config::STATS_INTERVAL_SECONDS)
        {
            double frameRate =
                (frames - previousFrames) / seconds;

            double byteRate =
                (receivedBytes - previousBytes) / seconds;

            std::cout
                << "\n=== CLIENT STATS ===\n"
                << "Frames=" << frames
                << " | TCP RX bytes=" << receivedBytes
                << '\n'
                << "CRC errors=" << crcErrors
                << " | Signature errors=" << signatureErrors
                << '\n'
                << "Rate=" << frameRate << " frames/s"
                << " | " << byteRate << " B/s\n"
                << "====================\n\n";

            previousFrames = frames;
            previousBytes = receivedBytes;
            lastReport = now;
        }
    }

    // 8. Dong socket truoc khi WinSock duoc don dep.
    Network::closeSocket(clientSocket);

    return exitCode;
}