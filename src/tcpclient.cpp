#include <winsock2.h>
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>

#include "../third_party/mavlink/common/mavlink.h"

// Moi nguon du lieu co mot parser rieng
struct MavlinkParser {
    mavlink_message_t receiving{};
    mavlink_status_t state{};

    mavlink_message_t message{};
    mavlink_status_t report{};

    bool feed(uint8_t byte) {
        return mavlink_frame_char_buffer(
            &receiving,
            &state,
            byte,
            &message,
            &report
        ) == MAVLINK_FRAMING_OK;
    }
};

// Trang thai cua mot TCP client
struct Client {
    SOCKET socket = INVALID_SOCKET;
    unsigned int id = 0;

    MavlinkParser parser;
    std::string pendingData;

    unsigned long long sentBytes = 0;
};

// Mot frame cho gui ve UAV
struct UdpFrame {
    SOCKET owner;
    std::string data;
};

const size_t MAX_CLIENTS = 10;
const size_t MAX_TCP_PENDING = 256 * 1024;
const size_t MAX_UDP_FRAMES = 256;

// Tao socket va bind
SOCKET createBoundSocket(int type, unsigned short port) {
    SOCKET s = socket(
        AF_INET,
        type,
        type == SOCK_DGRAM ? IPPROTO_UDP : IPPROTO_TCP
    );

    if (s == INVALID_SOCKET) {
        std::cerr << "socket loi: "
                  << WSAGetLastError() << '\n';
        return INVALID_SOCKET;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    // UDP nhan tu WSL; TCP nhan client tren cung Windows
    address.sin_addr.s_addr = htonl(
        type == SOCK_DGRAM ? INADDR_ANY : INADDR_LOOPBACK
    );

    if (bind(
            s,
            reinterpret_cast<const sockaddr*>(&address),
            sizeof(address)) == SOCKET_ERROR) {

        std::cerr << "bind cong " << port
                  << " loi: " << WSAGetLastError() << '\n';

        closesocket(s);
        return INVALID_SOCKET;
    }

    u_long mode = 1;

    if (ioctlsocket(s, FIONBIO, &mode) == SOCKET_ERROR) {
        std::cerr << "non-blocking loi: "
                  << WSAGetLastError() << '\n';

        closesocket(s);
        return INVALID_SOCKET;
    }

    return s;
}

// Chuyen thong diep thanh byte MAVLink
std::string serializeMessage(const mavlink_message_t& message) {
    uint8_t packet[MAVLINK_MAX_PACKET_LEN];

    uint16_t length = mavlink_msg_to_send_buffer(
        packet,
        &message
    );

    return std::string(
        reinterpret_cast<const char*>(packet),
        length
    );
}

int main() {
    // 1. Khoi tao Winsock
    WSADATA wsaData{};

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0) {
        std::cerr << "WSAStartup loi: " << result << '\n';
        return 1;
    }

    // 2. Tao cac socket
    SOCKET udpSocket = createBoundSocket(SOCK_DGRAM, 14550);
    SOCKET listenSocket = createBoundSocket(SOCK_STREAM, 5760);

    if (udpSocket == INVALID_SOCKET ||
        listenSocket == INVALID_SOCKET) {

        if (udpSocket != INVALID_SOCKET)
            closesocket(udpSocket);

        if (listenSocket != INVALID_SOCKET)
            closesocket(listenSocket);

        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen loi: "
                  << WSAGetLastError() << '\n';

        closesocket(udpSocket);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // 3. Trang thai Gateway
    std::vector<Client> clients;
    std::deque<UdpFrame> pendingUdp;

    unsigned int nextClientId = 1;

    sockaddr_in uavAddress{};
    bool uavKnown = false;
    uint8_t uavSystemId = 0;

    MavlinkParser uavParser;

    unsigned long long receivedFrames = 0;
    unsigned long long udpSentFrames = 0;

    char buffer[65536];

    // Ngat client va huy cac frame cua client con cho gui UAV
    auto disconnectClient = [&](Client& client) {
        SOCKET oldSocket = client.socket;

        std::cout
            << "Client #" << client.id
            << " ngat ket noi"
            << " | Da gui " << client.sentBytes
            << " bytes\n";

        closesocket(oldSocket);
        client.socket = INVALID_SOCKET;
        client.pendingData.clear();

        pendingUdp.erase(
            std::remove_if(
                pendingUdp.begin(),
                pendingUdp.end(),
                [oldSocket](const UdpFrame& frame) {
                    return frame.owner == oldSocket;
                }
            ),
            pendingUdp.end()
        );
    };

    std::cout << "=== PBL3 MAVLink Gateway: FAN-OUT ===\n";
    std::cout << "UDP: 0.0.0.0:14550\n";
    std::cout << "TCP: 127.0.0.1:5760\n";
    std::cout << "Toi da " << MAX_CLIENTS << " TCP clients\n";
    std::cout << "Ban nay ho tro mot UAV\n\n";

    // 4. Vong lap mang
    while (true) {
        fd_set readSet;
        fd_set writeSet;

        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);

        FD_SET(udpSocket, &readSet);
        FD_SET(listenSocket, &readSet);

        if (uavKnown && !pendingUdp.empty())
            FD_SET(udpSocket, &writeSet);

        for (const Client& client : clients) {
            FD_SET(client.socket, &readSet);

            if (!client.pendingData.empty())
                FD_SET(client.socket, &writeSet);
        }

        timeval timeout{};
        timeout.tv_sec = 1;

        int ready = select(
            0,
            &readSet,
            &writeSet,
            nullptr,
            &timeout
        );

        if (ready == SOCKET_ERROR) {
            std::cerr << "select loi: "
                      << WSAGetLastError() << '\n';
            break;
        }

        // A. Xu ly cac TCP client hien tai
        for (Client& client : clients) {
            // Nhan du lieu tu client
            if (FD_ISSET(client.socket, &readSet)) {
                int n = recv(
                    client.socket,
                    buffer,
                    sizeof(buffer),
                    0
                );

                if (n == 0) {
                    disconnectClient(client);
                } else if (n == SOCKET_ERROR) {
                    int error = WSAGetLastError();

                    if (error != WSAEWOULDBLOCK)
                        disconnectClient(client);
                } else {
                    // Parser rieng cho tung client
                    for (int i = 0; i < n; ++i) {
                        if (!client.parser.feed(
                                static_cast<uint8_t>(buffer[i]))) {
                            continue;
                        }

                        if (!uavKnown)
                            continue;

                        if (pendingUdp.size() >= MAX_UDP_FRAMES) {
                            std::cout
                                << "Hang doi UDP day, ngat client #"
                                << client.id << '\n';

                            disconnectClient(client);
                            break;
                        }

                        UdpFrame frame;
                        frame.owner = client.socket;
                        frame.data = serializeMessage(
                            client.parser.message
                        );

                        pendingUdp.push_back(frame);

                        uint32_t msgid =
                            client.parser.message.msgid;

                        if (msgid == MAVLINK_MSG_ID_COMMAND_LONG ||
                            msgid == MAVLINK_MSG_ID_COMMAND_INT ||
                            msgid == MAVLINK_MSG_ID_PARAM_REQUEST_LIST) {

                            std::cout
                                << "Client #" << client.id
                                << " -> UAV"
                                << " | MSGID=" << msgid
                                << '\n';
                        }
                    }
                }
            }

            // Gui du lieu dang cho den client nay
            if (client.socket != INVALID_SOCKET &&
                FD_ISSET(client.socket, &writeSet) &&
                !client.pendingData.empty()) {

                int sent = send(
                    client.socket,
                    client.pendingData.data(),
                    static_cast<int>(client.pendingData.size()),
                    0
                );

                if (sent > 0) {
                    client.pendingData.erase(
                        0,
                        static_cast<size_t>(sent)
                    );

                    client.sentBytes += sent;
                } else if (sent == 0) {
                    disconnectClient(client);
                } else {
                    int error = WSAGetLastError();

                    if (error != WSAEWOULDBLOCK)
                        disconnectClient(client);
                }
            }
        }

        // Xoa cac client da ngat
        clients.erase(
            std::remove_if(
                clients.begin(),
                clients.end(),
                [](const Client& client) {
                    return client.socket == INVALID_SOCKET;
                }
            ),
            clients.end()
        );

        // B. Gui mot frame trong hang doi ve UAV
        if (FD_ISSET(udpSocket, &writeSet) &&
            uavKnown &&
            !pendingUdp.empty()) {

            const std::string& packet = pendingUdp.front().data;

            int sent = sendto(
                udpSocket,
                packet.data(),
                static_cast<int>(packet.size()),
                0,
                reinterpret_cast<const sockaddr*>(&uavAddress),
                sizeof(uavAddress)
            );

            if (sent == SOCKET_ERROR) {
                int error = WSAGetLastError();

                if (error != WSAEWOULDBLOCK) {
                    std::cerr << "sendto UAV loi: "
                              << error << '\n';

                    pendingUdp.pop_front();
                }
            } else {
                if (sent == static_cast<int>(packet.size()))
                    ++udpSentFrames;
                else
                    std::cerr << "UDP gui khong du frame\n";

                pendingUdp.pop_front();
            }
        }

        // C. Chap nhan client moi
        if (FD_ISSET(listenSocket, &readSet)) {
            SOCKET newSocket = accept(
                listenSocket,
                nullptr,
                nullptr
            );

            if (newSocket != INVALID_SOCKET) {
                // readSet can chua UDP + listen + cac client
                if (clients.size() >= MAX_CLIENTS ||
                    clients.size() + 2 >= FD_SETSIZE) {

                    std::cout << "Da du so client\n";
                    closesocket(newSocket);
                } else {
                    u_long mode = 1;

                    if (ioctlsocket(
                            newSocket,
                            FIONBIO,
                            &mode) == SOCKET_ERROR) {

                        closesocket(newSocket);
                    } else {
                        Client client;
                        client.socket = newSocket;
                        client.id = nextClientId++;

                        clients.push_back(client);

                        std::cout
                            << "Client #" << client.id
                            << " da ket noi"
                            << " | Tong client=" << clients.size()
                            << '\n';
                    }
                }
            } else {
                int error = WSAGetLastError();

                if (error != WSAEWOULDBLOCK)
                    std::cerr << "accept loi: "
                              << error << '\n';
            }
        }

        // D. Nhan du lieu tu UAV
        if (FD_ISSET(udpSocket, &readSet)) {
            sockaddr_in sender{};
            int senderSize = sizeof(sender);

            int n = recvfrom(
                udpSocket,
                buffer,
                sizeof(buffer),
                0,
                reinterpret_cast<sockaddr*>(&sender),
                &senderSize
            );

            if (n == SOCKET_ERROR) {
                int error = WSAGetLastError();

                if (error != WSAEWOULDBLOCK)
                    std::cerr << "recvfrom loi: "
                              << error << '\n';

                continue;
            }

            // Ban thu nay chi nhan mot endpoint UAV
            if (uavKnown &&
                (sender.sin_addr.s_addr !=
                     uavAddress.sin_addr.s_addr ||
                 sender.sin_port != uavAddress.sin_port)) {
                continue;
            }

            for (int i = 0; i < n; ++i) {
                if (!uavParser.feed(
                        static_cast<uint8_t>(buffer[i]))) {
                    continue;
                }

                const mavlink_message_t& message =
                    uavParser.message;

                ++receivedFrames;

                if (message.msgid == MAVLINK_MSG_ID_HEARTBEAT) {
                    mavlink_heartbeat_t heartbeat{};

                    mavlink_msg_heartbeat_decode(
                        &message,
                        &heartbeat
                    );

                    if (!uavKnown &&
                        heartbeat.autopilot != MAV_AUTOPILOT_INVALID &&
                        heartbeat.type != MAV_TYPE_GCS) {

                        uavKnown = true;
                        uavAddress = sender;
                        uavSystemId = message.sysid;

                        std::cout
                            << "Da hoc UAV SYSID="
                            << static_cast<int>(uavSystemId)
                            << " tai "
                            << inet_ntoa(sender.sin_addr)
                            << ":" << ntohs(sender.sin_port)
                            << '\n';
                    }

                    std::cout
                        << "HEARTBEAT"
                        << " | Clients=" << clients.size()
                        << " | RX frames=" << receivedFrames
                        << " | UDP TX=" << udpSentFrames
                        << '\n';
                }

                if (!uavKnown || message.sysid != uavSystemId)
                    continue;

                std::string packet = serializeMessage(message);

                // FAN-OUT: them cung frame vao hang doi tung client
                for (Client& client : clients) {
                    if (client.socket == INVALID_SOCKET)
                        continue;

                    if (client.pendingData.size() + packet.size() >
                        MAX_TCP_PENDING) {

                        std::cout
                            << "Client #" << client.id
                            << " nhan qua cham\n";

                        disconnectClient(client);
                    } else {
                        client.pendingData.append(packet);
                    }
                }

                if (message.msgid == MAVLINK_MSG_ID_COMMAND_ACK) {
                    mavlink_command_ack_t ack{};
                    mavlink_msg_command_ack_decode(&message, &ack);

                    std::cout
                        << "COMMAND_ACK"
                        << " command=" << ack.command
                        << " result=" << static_cast<int>(ack.result)
                        << '\n';
                }
            }
        }

        // Don client bi ngat khi fan-out
        clients.erase(
            std::remove_if(
                clients.begin(),
                clients.end(),
                [](const Client& client) {
                    return client.socket == INVALID_SOCKET;
                }
            ),
            clients.end()
        );
    }

    // 5. Don dep
    for (const Client& client : clients)
        closesocket(client.socket);

    closesocket(udpSocket);
    closesocket(listenSocket);
    WSACleanup();

    return 0;
}