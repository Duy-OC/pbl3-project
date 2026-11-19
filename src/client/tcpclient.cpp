#include "network.h"
#include "config.h"
#include "mavlink_codec.h"
#include "time_utils.h"
#include "uav_manager.h"

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <sstream>

// Ham ve thanh pin truc quan [||||||||  ]
std::string batteryBar(int8_t pct)
{
    if (pct < 0) return "[UNKNOWN]";
    int bars = pct / 10;
    std::string s = "[";
    for (int i = 0; i < 10; ++i)
    {
        s += (i < bars) ? "|" : " ";
    }
    s += "] " + std::to_string(pct) + "%";
    return s;
}

// Ham hien thi Live Dashboard tren Console
void renderDashboard(
    const UavManager &uavManager,
    unsigned long long totalFrames,
    unsigned long long totalBytes,
    double frameRate,
    double byteRate,
    unsigned long long crcErrors,
    TimePoint now)
{
    const auto &uavs = uavManager.all();

    std::cout << "\n========================================================================================\n"
              << "                    PBL3 MAVLINK TCP CLIENT - UAV LIVE DASHBOARD                        \n"
              << "========================================================================================\n";

    if (uavs.empty())
    {
        std::cout << "  [DANG CHO DU LIEU TU GATEWAY...] Chua nhan duoc goi tin tu UAV nao qua TCP.\n"
                  << "  Hay dam bao: PX4 SITL -> Gateway 14550 dang hoat dong.\n";
    }
    else
    {
        for (const auto &item : uavs)
        {
            int sysid = item.first;
            const Uav &uav = item.second;
            const UavTelemetry &tel = uav.telemetry;
            bool online = isOnline(uav, now);
            double age = elapsedSeconds(now, tel.lastUpdate);

            std::cout << "  UAV [SYSID: " << sysid << "]  "
                      << "Trang thai: " << (online ? "[ONLINE]" : "[OFFLINE/TIMEOUT]")
                      << "  |  Che do bay: [" << tel.flightMode << "]\n"
                      << "  Do tuoi du lieu: " << std::fixed << std::setprecision(1) << age << "s truoc"
                      << "  |  TCP RX Rate: " << std::setprecision(1) << frameRate << " frames/s ("
                      << (byteRate / 1024.0) << " kB/s)\n"
                      << "----------------------------------------------------------------------------------------\n"
                      << "  [PIN & NGUON]      Dien ap: " << std::setprecision(2) << tel.voltageBattery << " V"
                      << "      | Dong dien: " << std::setprecision(2) << tel.currentBattery << " A"
                      << "     | Pin: " << batteryBar(tel.batteryRemaining) << "\n"
                      << "  [GPS & DO CAO]     Vi do: " << std::setprecision(6) << tel.latitude << " deg"
                      << "    | Kinh do: " << std::setprecision(6) << tel.longitude << " deg"
                      << "   | GPS: " << tel.gpsFixString() << " (" << static_cast<int>(tel.satellitesVisible) << " ve tinh)\n"
                      << "                     Do cao tuong doi: " << std::setprecision(1) << tel.relativeAlt << " m"
                      << "   | Do cao MSL: " << std::setprecision(1) << tel.altitudeMSL << " m"
                      << "    | Heading: " << std::setprecision(1) << tel.heading << " deg\n"
                      << "  [GOC NGHIENG]      Roll: " << std::setprecision(1) << tel.roll << " deg"
                      << "         | Pitch: " << std::setprecision(1) << tel.pitch << " deg"
                      << "           | Yaw: " << std::setprecision(1) << tel.yaw << " deg\n"
                      << "  [TOC DO (HUD)]     Mat dat: " << std::setprecision(1) << tel.groundspeed << " m/s"
                      << "      | Gio: " << std::setprecision(1) << tel.airspeed << " m/s"
                      << "          | Leo cao: " << std::setprecision(1) << tel.climbRate << " m/s"
                      << " | Tay ga: " << static_cast<int>(tel.throttle) << "%\n";
        }
    }

    std::cout << "----------------------------------------------------------------------------------------\n"
              << "  Thong ke TCP Client: Tong frame: " << totalFrames
              << "  |  Tong bytes: " << (totalBytes / 1024.0) << " kB"
              << "  |  Loi CRC: " << crcErrors << "\n"
              << "========================================================================================\n\n";
}

int main(int argc, char **argv)
{
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    unsigned short port = Config::TCP_PORT; // 5760
    const char *host = "127.0.0.1";

    if (argc >= 2) host = argv[1];
    if (argc > 3) { std::cerr << "Usage: tcpclient.exe [IPv4] [port]\n"; return 1; }
    if (argc >= 3)
    {
        try {
            std::size_t parsed = 0;
            const std::string value(argv[2]);
            const unsigned long number = std::stoul(value, &parsed);
            if (parsed != value.size() || number < 1 || number > 65535) throw std::invalid_argument("port");
            port = static_cast<unsigned short>(number);
        } catch (...) { std::cerr << "TCP port khong hop le\n"; return 1; }
    }

    // 1. Khoi tao WinSock
    Network::Runtime winsock;
    if (winsock.error() != 0)
    {
        std::cerr << "WSAStartup loi: " << winsock.error() << '\n';
        return 1;
    }

    // 2. Tao socket TCP
    SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Tao socket loi: " << WSAGetLastError() << '\n';
        return 1;
    }

    // 3. Dia chi Gateway
    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(host);

    std::cout << "[TCP Client] Dang ket noi toi Gateway " << host << ":" << port << "...\n";

    // 4. Ket noi Gateway
    if (connect(clientSocket, reinterpret_cast<const sockaddr *>(&server), sizeof(server)) == SOCKET_ERROR)
    {
        std::cerr << "[ERROR] Khong the ket noi toi Gateway (" << host << ":" << port
                  << "). Hay dam bao gateway.exe dang chay!\n"
                  << "Loi WinSock: " << WSAGetLastError() << '\n';
        Network::closeSocket(clientSocket);
        return 1;
    }

    // 5. Chuyen sang non-blocking
    if (!Network::setNonblocking(clientSocket))
    {
        std::cerr << "Dat nonblocking loi: " << WSAGetLastError() << '\n';
        Network::closeSocket(clientSocket);
        return 1;
    }

    std::cout << "[SUCCESS] Da ket noi thanh cong toi Gateway!\n"
              << "Dang hien thi Dashboard...\n";

    // 7. Bo quan ly Telemetry UAV tai Client
    UavManager uavManager;
    MavlinkParser parser;

    unsigned long long frames = 0;
    unsigned long long receivedBytes = 0;
    unsigned long long crcErrors = 0;
    unsigned long long signatureErrors = 0;

    unsigned long long previousFrames = 0;
    unsigned long long previousBytes = 0;

    TimePoint lastDashboard = Clock::now();

    char buffer[4096];
    int exitCode = 0;

    // 8. Vong lap nhan va hien thi du lieu
    while (true)
    {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(clientSocket, &reads);

        timeval timeout{};
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 100ms

        int ready = select(0, &reads, nullptr, nullptr, &timeout);
        if (ready == SOCKET_ERROR)
        {
            std::cerr << "select loi: " << WSAGetLastError() << '\n';
            exitCode = 1;
            break;
        }

        if (FD_ISSET(clientSocket, &reads))
        {
            int received = recv(clientSocket, buffer, static_cast<int>(sizeof(buffer)), 0);

            if (received == 0)
            {
                std::cout << "\n[TCP Client] Gateway da dong ket noi TCP.\n";
                break;
            }

            if (received == SOCKET_ERROR)
            {
                int error = WSAGetLastError();
                if (error != WSAEWOULDBLOCK)
                {
                    std::cerr << "recv loi: " << error << '\n';
                    exitCode = 1;
                    break;
                }
            }
            else
            {
                receivedBytes += static_cast<unsigned long long>(received);
                TimePoint now = Clock::now();

                for (int i = 0; i < received; ++i)
                {
                    uint8_t status = parser.feed(static_cast<uint8_t>(buffer[i]));

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
                    const mavlink_message_t &message = parser.message;

                    // Giai ma trang thai telemetry cua UAV de hien thi dashboard
                    uavManager.observeHeartbeat(message, server, now);
                    uavManager.acceptTelemetry(message, server, now);
                }
            }
        }

        TimePoint now = Clock::now();
        // Kiem tra timeout theo cau hinh chung
        uavManager.expire(now);

        // Cap nhat Live Dashboard dinh ky moi 1 giay
        double dashElapsed = elapsedSeconds(now, lastDashboard);
        if (dashElapsed >= 1.0)
        {
            double frameRate = (frames - previousFrames) / dashElapsed;
            double byteRate = (receivedBytes - previousBytes) / dashElapsed;

            renderDashboard(uavManager, frames, receivedBytes, frameRate, byteRate, crcErrors, now);

            previousFrames = frames;
            previousBytes = receivedBytes;
            lastDashboard = now;
        }
    }

    Network::closeSocket(clientSocket);
    return exitCode;
}
