#include <winsock2.h>
#include <iostream>
#include <iomanip>

#include "../third_party/mavlink/common/mavlink.h"

// Tra ten mot so thong diep thuong gap
const char* getMessageName(uint32_t msgid) {
    switch (msgid) {
        case MAVLINK_MSG_ID_HEARTBEAT:
            return "HEARTBEAT";

        case MAVLINK_MSG_ID_SYS_STATUS:
            return "SYS_STATUS";

        case MAVLINK_MSG_ID_GPS_RAW_INT:
            return "GPS_RAW_INT";

        case MAVLINK_MSG_ID_ATTITUDE:
            return "ATTITUDE";

        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
            return "GLOBAL_POSITION_INT";

        default:
            return "OTHER";
    }
}

int main() {
    // 1. Khoi tao Winsock
    WSADATA wsaData{};

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0) {
        std::cerr << "WSAStartup loi: " << result << '\n';
        return 1;
    }

    // 2. Tao socket UDP
    SOCKET serverSocket = socket(
        AF_INET,
        SOCK_DGRAM,
        IPPROTO_UDP
    );

    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "socket loi: "
                  << WSAGetLastError() << '\n';

        WSACleanup();
        return 1;
    }

    // 3. Cau hinh dia chi nhan
    sockaddr_in serverAddr{};

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(14550);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 4. Gan socket voi cong UDP 14550
    result = bind(
        serverSocket,
        reinterpret_cast<const sockaddr*>(&serverAddr),
        sizeof(serverAddr)
    );

    if (result == SOCKET_ERROR) {
        std::cerr << "bind loi: "
                  << WSAGetLastError() << '\n';

        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Dang nghe UDP tren cong 14550...\n";
    std::cout << "Dang cho du lieu MAVLink tu PX4...\n\n";

    // Bo dem nhan UDP
    char buffer[65536];

    // Parser cho mot nguon PX4 trong bai thu hien tai
    mavlink_message_t message{};
    mavlink_status_t status{};

    int exitCode = 0;

    // 5. Nhan va phan tich du lieu lien tuc
    while (true) {
        sockaddr_in senderAddr{};
        int senderAddrSize = sizeof(senderAddr);

        int receivedBytes = recvfrom(
            serverSocket,
            buffer,
            sizeof(buffer),
            0,
            reinterpret_cast<sockaddr*>(&senderAddr),
            &senderAddrSize
        );

        if (receivedBytes == SOCKET_ERROR) {
            std::cerr << "recvfrom loi: "
                      << WSAGetLastError() << '\n';

            exitCode = 1;
            break;
        }

        // Mot datagram co the chua nhieu thong diep MAVLink.
        // Dua tung byte vao parser, giu trang thai qua cac lan nhan.
        for (int i = 0; i < receivedBytes; ++i) {
            uint8_t byte = static_cast<uint8_t>(buffer[i]);

            if (mavlink_parse_char(
                    MAVLINK_COMM_0,
                    byte,
                    &message,
                    &status)) {

                // Chi in khi parser nhan duoc thong diep hop le
                std::cout
                    << inet_ntoa(senderAddr.sin_addr)
                    << ":" << ntohs(senderAddr.sin_port)
                    << " | SYSID="
                    << static_cast<int>(message.sysid)
                    << " COMPID="
                    << static_cast<int>(message.compid)
                    << " MSGID=" << message.msgid
                    << " " << getMessageName(message.msgid)
                    << '\n';
                std::cout << std::fixed << std::setprecision(2);

// Ban tin goc nghieng
if (message.msgid == MAVLINK_MSG_ID_ATTITUDE) {
    mavlink_attitude_t attitude{};
    mavlink_msg_attitude_decode(&message, &attitude);

    constexpr double RAD_TO_DEG = 57.295779513;

    std::cout
        << "  Roll: " << attitude.roll * RAD_TO_DEG
        << " deg | Pitch: " << attitude.pitch * RAD_TO_DEG
        << " deg | Yaw: " << attitude.yaw * RAD_TO_DEG
        << " deg\n";
}

// Ban tin vi tri va do cao
if (message.msgid == MAVLINK_MSG_ID_GLOBAL_POSITION_INT) {
    mavlink_global_position_int_t position{};
    mavlink_msg_global_position_int_decode(&message, &position);

    std::cout
        << std::setprecision(7)
        << "  Lat: " << position.lat / 10000000.0
        << " | Lon: " << position.lon / 10000000.0
        << std::setprecision(2)
        << " | Relative altitude: "
        << position.relative_alt / 1000.0
        << " m\n";
}
                
            }
        }
    }

    // 6. Don dep khi vong lap ket thuc
    closesocket(serverSocket);
    WSACleanup();

    return exitCode;
}