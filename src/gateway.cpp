#include <winsock2.h>
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <algorithm>
#include <chrono>

#include "../third_party/mavlink/common/mavlink.h"

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

const int HEARTBEAT_TIMEOUT = 5;
const int STATS_INTERVAL = 5;

const size_t MAX_CLIENTS = 10;
const size_t MAX_TCP_PENDING = 256 * 1024;
const size_t MAX_UDP_PENDING = 512;
const size_t MAX_UDP_SOURCES = 32;

// Parser rieng cho moi luong du lieu
struct Parser {
    mavlink_message_t receiving{};
    mavlink_status_t state{};
    mavlink_message_t message{};
    mavlink_status_t report{};

    uint8_t feed(uint8_t byte) {
        return mavlink_frame_char_buffer(
            &receiving, &state, byte, &message, &report
        );
    }
};

struct Client {
    SOCKET socket = INVALID_SOCKET;
    unsigned int id = 0;

    Parser parser;
    std::string pending;
    unsigned long long sentBytes = 0;
};

struct Uav {
    sockaddr_in address{};
    bool online = false;
    TimePoint lastHeartbeat{};

    // Tang khi route duoc hoc lai
    unsigned int generation = 0;

    unsigned long long receivedFrames = 0;
};

struct UdpSource {
    Parser parser;
    TimePoint lastSeen{};
};

struct UdpFrame {
    SOCKET owner;
    int target;
    unsigned int generation;
    sockaddr_in destination;
    std::string data;
};

struct Statistics {
    unsigned long long udpDatagrams = 0;
    unsigned long long udpRxBytes = 0;
    unsigned long long uavFrames = 0;
    unsigned long long clientFrames = 0;

    unsigned long long badCrc = 0;
    unsigned long long badSignature = 0;

    unsigned long long tcpTxBytes = 0;
    unsigned long long udpTxFrames = 0;

    unsigned long long droppedUdpFrames = 0;
    unsigned long long blockedMessages = 0;
    unsigned long long slowClients = 0;
};

bool sameAddress(const sockaddr_in& a, const sockaddr_in& b) {
    return a.sin_addr.s_addr == b.sin_addr.s_addr &&
           a.sin_port == b.sin_port;
}

double elapsedSeconds(TimePoint now, TimePoint before) {
    return std::chrono::duration<double>(now - before).count();
}

bool isOnline(const Uav& uav, TimePoint now) {
    return uav.online &&
           elapsedSeconds(now, uav.lastHeartbeat) <
               HEARTBEAT_TIMEOUT;
}

std::string serialize(const mavlink_message_t& message) {
    uint8_t bytes[MAVLINK_MAX_PACKET_LEN];

    uint16_t length = mavlink_msg_to_send_buffer(
        bytes, &message
    );

    return std::string(
        reinterpret_cast<const char*>(bytes), length
    );
}

// Doc target_system theo metadata cua thu vien.
// Khong co truong target_system hoac gia tri 0: broadcast.
// -1: khong co metadata.
int getTargetSystem(const mavlink_message_t& message) {
    const mavlink_msg_entry_t* entry =
        mavlink_get_msg_entry(message.msgid);

    if (!entry)
        return -1;

    if (!(entry->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_SYSTEM))
        return 0;

    unsigned int offset = entry->target_system_ofs;

    // MAVLink 2 co the cat byte 0 o cuoi payload
    if (offset >= message.len)
        return 0;

    return static_cast<uint8_t>(_MAV_PAYLOAD(&message)[offset]);
}

bool isCommand(const mavlink_message_t& message) {
    return message.msgid == MAVLINK_MSG_ID_COMMAND_LONG ||
           message.msgid == MAVLINK_MSG_ID_COMMAND_INT;
}

SOCKET createSocket(int type, unsigned short port) {
    SOCKET s = socket(
        AF_INET,
        type,
        type == SOCK_DGRAM ? IPPROTO_UDP : IPPROTO_TCP
    );

    if (s == INVALID_SOCKET) {
        std::cerr << "socket loi: " << WSAGetLastError() << '\n';
        return INVALID_SOCKET;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

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

int main() {
    WSADATA wsa{};
    int result = WSAStartup(MAKEWORD(2, 2), &wsa);

    if (result != 0) {
        std::cerr << "WSAStartup loi: " << result << '\n';
        return 1;
    }

    SOCKET udp = createSocket(SOCK_DGRAM, 14550);
    SOCKET listener = createSocket(SOCK_STREAM, 5760);

    if (udp == INVALID_SOCKET || listener == INVALID_SOCKET) {
        if (udp != INVALID_SOCKET)
            closesocket(udp);

        if (listener != INVALID_SOCKET)
            closesocket(listener);

        WSACleanup();
        return 1;
    }

    if (listen(listener, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen loi: " << WSAGetLastError() << '\n';

        closesocket(udp);
        closesocket(listener);
        WSACleanup();
        return 1;
    }

    std::vector<Client> clients;
    std::map<int, Uav> uavs;

    using SourceKey = std::pair<unsigned long, unsigned short>;
    std::map<SourceKey, UdpSource> sources;

    std::deque<UdpFrame> outgoingUdp;

    Statistics stats;
    unsigned int nextClientId = 1;

    TimePoint lastStats = Clock::now();
    unsigned long long previousRxBytes = 0;
    unsigned long long previousFrames = 0;

    char buffer[65536];

    // Huy frame dang cho gui den mot UAV
    auto purgeUavQueue = [&](int sysid) {
        size_t before = outgoingUdp.size();

        outgoingUdp.erase(
            std::remove_if(
                outgoingUdp.begin(),
                outgoingUdp.end(),
                [sysid](const UdpFrame& frame) {
                    return frame.target == sysid;
                }
            ),
            outgoingUdp.end()
        );

        stats.droppedUdpFrames += before - outgoingUdp.size();
    };

    auto disconnect = [&](Client& client) {
        SOCKET oldSocket = client.socket;

        std::cout
            << "Client #" << client.id << " DISCONNECTED"
            << " | TCP TX=" << client.sentBytes << " bytes\n";

        closesocket(oldSocket);
        client.socket = INVALID_SOCKET;
        client.pending.clear();

        size_t before = outgoingUdp.size();

        outgoingUdp.erase(
            std::remove_if(
                outgoingUdp.begin(),
                outgoingUdp.end(),
                [oldSocket](const UdpFrame& frame) {
                    return frame.owner == oldSocket;
                }
            ),
            outgoingUdp.end()
        );

        stats.droppedUdpFrames += before - outgoingUdp.size();
    };

    auto removeDisconnected = [&]() {
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
    };

    // Kiem tra timeout va bao cao thong ke
    auto maintenance = [&]() {
        TimePoint now = Clock::now();

        for (auto& item : uavs) {
            Uav& uav = item.second;

            if (uav.online && !isOnline(uav, now)) {
                uav.online = false;
                purgeUavQueue(item.first);

                std::cout << "UAV " << item.first
                          << " OFFLINE\n";
            }
        }

        // Don parser cua endpoint khong con gui du lieu
        for (auto it = sources.begin(); it != sources.end();) {
            if (elapsedSeconds(now, it->second.lastSeen) > 30)
                it = sources.erase(it);
            else
                ++it;
        }

        double seconds = elapsedSeconds(now, lastStats);

        if (seconds < STATS_INTERVAL)
            return;

        size_t onlineCount = 0;

        for (const auto& item : uavs) {
            if (isOnline(item.second, now))
                ++onlineCount;
        }

        std::cout
            << "\n=== STATS ==="
            << "\nUAV online=" << onlineCount
            << " | UAV da hoc=" << uavs.size()
            << " | Clients=" << clients.size()
            << "\nUDP RX datagrams=" << stats.udpDatagrams
            << " | UDP RX bytes=" << stats.udpRxBytes
            << "\nMAVLink UAV RX=" << stats.uavFrames
            << " | Client RX=" << stats.clientFrames
            << "\nCRC errors=" << stats.badCrc
            << " | Signature errors=" << stats.badSignature
            << "\nTCP TX bytes=" << stats.tcpTxBytes
            << " | UDP TX frames=" << stats.udpTxFrames
            << "\nUDP queued=" << outgoingUdp.size()
            << " | UDP dropped/cancelled="
            << stats.droppedUdpFrames
            << "\nBlocked messages=" << stats.blockedMessages
            << " | Slow clients=" << stats.slowClients
            << "\nUDP RX rate="
            << (stats.udpRxBytes - previousRxBytes) / seconds
            << " B/s"
            << " | MAVLink UAV RX rate="
            << (stats.uavFrames - previousFrames) / seconds
            << " frames/s\n";

        for (const auto& item : uavs) {
            std::cout
                << "  UAV " << item.first
                << ": "
                << (isOnline(item.second, now)
                    ? "ONLINE" : "OFFLINE")
                << " | RX frames="
                << item.second.receivedFrames
                << '\n';
        }

        std::cout << "=============\n\n";

        previousRxBytes = stats.udpRxBytes;
        previousFrames = stats.uavFrames;
        lastStats = now;
    };

    std::cout << "=== PBL3 GATEWAY: MONITORING ===\n";
    std::cout << "UDP: 0.0.0.0:14550\n";
    std::cout << "TCP: 127.0.0.1:5760\n";
    std::cout << "Heartbeat timeout: 5 giay\n";
    std::cout << "Thong ke: moi 5 giay\n\n";

    while (true) {
        maintenance();

        fd_set reads;
        fd_set writes;

        FD_ZERO(&reads);
        FD_ZERO(&writes);

        FD_SET(udp, &reads);
        FD_SET(listener, &reads);

        if (!outgoingUdp.empty())
            FD_SET(udp, &writes);

        for (const Client& client : clients) {
            FD_SET(client.socket, &reads);

            if (!client.pending.empty())
                FD_SET(client.socket, &writes);
        }

        timeval timeout{};
        timeout.tv_usec = 200000; // 200 ms

        if (select(0, &reads, &writes, nullptr, &timeout)
            == SOCKET_ERROR) {

            std::cerr << "select loi: "
                      << WSAGetLastError() << '\n';
            break;
        }

        // Cap nhat timeout truoc khi xu ly lenh
        maintenance();

        // A. TCP client -> hang doi UDP
        for (Client& client : clients) {
            if (FD_ISSET(client.socket, &reads)) {
                int n = recv(
                    client.socket, buffer, sizeof(buffer), 0
                );

                if (n == 0) {
                    disconnect(client);
                } else if (n == SOCKET_ERROR) {
                    if (WSAGetLastError() != WSAEWOULDBLOCK)
                        disconnect(client);
                } else {
                    for (int i = 0; i < n; ++i) {
                        uint8_t parsed = client.parser.feed(
                            static_cast<uint8_t>(buffer[i])
                        );

                        if (parsed == MAVLINK_FRAMING_BAD_CRC) {
                            ++stats.badCrc;
                            continue;
                        }

                        if (parsed == MAVLINK_FRAMING_BAD_SIGNATURE) {
                            ++stats.badSignature;
                            continue;
                        }

                        if (parsed != MAVLINK_FRAMING_OK)
                            continue;

                        ++stats.clientFrames;

                        const mavlink_message_t& message =
                            client.parser.message;

                        int target = getTargetSystem(message);

                        if (target < 0) {
                            ++stats.blockedMessages;
                            continue;
                        }

                        TimePoint now = Clock::now();
                        size_t destinations = 0;

                        for (const auto& item : uavs) {
                            if ((target == 0 || item.first == target) &&
                                isOnline(item.second, now)) {
                                ++destinations;
                            }
                        }

                        if (destinations == 0) {
                            ++stats.blockedMessages;

                            if (target != 0) {
                                std::cout
                                    << "BLOCK: UAV " << target
                                    << " offline hoac chua duoc hoc"
                                    << " | MSGID=" << message.msgid
                                    << '\n';
                            }

                            continue;
                        }

                        if (outgoingUdp.size() + destinations >
                            MAX_UDP_PENDING) {

                            std::cout << "Hang doi UDP day\n";
                            disconnect(client);
                            break;
                        }

                        std::string packet = serialize(message);

                        for (const auto& item : uavs) {
                            if (target != 0 && item.first != target)
                                continue;

                            if (!isOnline(item.second, now))
                                continue;

                            UdpFrame frame;
                            frame.owner = client.socket;
                            frame.target = item.first;
                            frame.generation = item.second.generation;
                            frame.destination = item.second.address;
                            frame.data = packet;

                            outgoingUdp.push_back(frame);

                            if (isCommand(message)) {
                                std::cout
                                    << "ROUTE: Client #" << client.id
                                    << " -> UAV " << item.first
                                    << " -> "
                                    << inet_ntoa(
                                        item.second.address.sin_addr
                                    )
                                    << ":"
                                    << ntohs(item.second.address.sin_port)
                                    << '\n';
                            }
                        }
                    }
                }
            }

            // Gui telemetry cho client, xu ly partial send
            if (client.socket != INVALID_SOCKET &&
                FD_ISSET(client.socket, &writes) &&
                !client.pending.empty()) {

                int sent = send(
                    client.socket,
                    client.pending.data(),
                    static_cast<int>(client.pending.size()),
                    0
                );

                if (sent > 0) {
                    client.pending.erase(
                        0, static_cast<size_t>(sent)
                    );

                    client.sentBytes += sent;
                    stats.tcpTxBytes += sent;
                } else if (sent == 0) {
                    disconnect(client);
                } else if (WSAGetLastError() != WSAEWOULDBLOCK) {
                    disconnect(client);
                }
            }
        }

        removeDisconnected();

        // B. Gui frame UDP, kiem tra route con hop le
        if (FD_ISSET(udp, &writes) && !outgoingUdp.empty()) {
            const UdpFrame& frame = outgoingUdp.front();
            auto route = uavs.find(frame.target);

            bool valid =
                route != uavs.end() &&
                isOnline(route->second, Clock::now()) &&
                route->second.generation == frame.generation &&
                sameAddress(
                    route->second.address, frame.destination
                );

            if (!valid) {
                ++stats.droppedUdpFrames;
                outgoingUdp.pop_front();
            } else {
                int sent = sendto(
                    udp,
                    frame.data.data(),
                    static_cast<int>(frame.data.size()),
                    0,
                    reinterpret_cast<const sockaddr*>(
                        &frame.destination
                    ),
                    sizeof(frame.destination)
                );

                if (sent == SOCKET_ERROR) {
                    int error = WSAGetLastError();

                    if (error != WSAEWOULDBLOCK) {
                        std::cerr << "sendto loi: "
                                  << error << '\n';

                        ++stats.droppedUdpFrames;
                        outgoingUdp.pop_front();
                    }
                } else {
                    if (sent == static_cast<int>(frame.data.size()))
                        ++stats.udpTxFrames;
                    else
                        ++stats.droppedUdpFrames;

                    outgoingUdp.pop_front();
                }
            }
        }

        // C. Chap nhan client moi
        if (FD_ISSET(listener, &reads)) {
            SOCKET s = accept(listener, nullptr, nullptr);

            if (s != INVALID_SOCKET) {
                if (clients.size() >= MAX_CLIENTS ||
                    clients.size() + 2 >= FD_SETSIZE) {

                    std::cout << "Da du so client\n";
                    closesocket(s);
                } else {
                    u_long mode = 1;

                    if (ioctlsocket(s, FIONBIO, &mode)
                        == SOCKET_ERROR) {

                        closesocket(s);
                    } else {
                        Client client;
                        client.socket = s;
                        client.id = nextClientId++;
                        clients.push_back(client);

                        std::cout
                            << "Client #" << client.id
                            << " CONNECTED"
                            << " | Tong=" << clients.size()
                            << '\n';
                    }
                }
            } else if (WSAGetLastError() != WSAEWOULDBLOCK) {
                std::cerr << "accept loi: "
                          << WSAGetLastError() << '\n';
            }
        }

        // D. Nhan MAVLink tu UAV
        if (FD_ISSET(udp, &reads)) {
            sockaddr_in sender{};
            int senderSize = sizeof(sender);

            int n = recvfrom(
                udp,
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

            ++stats.udpDatagrams;
            stats.udpRxBytes += n;

            SourceKey key(
                sender.sin_addr.s_addr, sender.sin_port
            );

            auto source = sources.find(key);

            if (source == sources.end()) {
                if (sources.size() >= MAX_UDP_SOURCES)
                    continue;

                source = sources.emplace(
                    key, UdpSource{}
                ).first;
            }

            source->second.lastSeen = Clock::now();
            Parser& parser = source->second.parser;

            for (int i = 0; i < n; ++i) {
                uint8_t parsed = parser.feed(
                    static_cast<uint8_t>(buffer[i])
                );

                if (parsed == MAVLINK_FRAMING_BAD_CRC) {
                    ++stats.badCrc;
                    continue;
                }

                if (parsed == MAVLINK_FRAMING_BAD_SIGNATURE) {
                    ++stats.badSignature;
                    continue;
                }

                if (parsed != MAVLINK_FRAMING_OK)
                    continue;

                ++stats.uavFrames;

                const mavlink_message_t& message = parser.message;
                TimePoint now = Clock::now();

                // Chi heartbeat autopilot cap nhat online
                if (message.msgid == MAVLINK_MSG_ID_HEARTBEAT) {
                    mavlink_heartbeat_t heartbeat{};

                    mavlink_msg_heartbeat_decode(
                        &message, &heartbeat
                    );

                    if (message.sysid != 0 &&
                        heartbeat.autopilot != MAV_AUTOPILOT_INVALID &&
                        heartbeat.type != MAV_TYPE_GCS) {

                        Uav& uav = uavs[message.sysid];

                        bool alive = isOnline(uav, now);
                        bool same = sameAddress(uav.address, sender);

                        if (alive && !same) {
                            std::cout
                                << "CONFLICT: SYSID="
                                << static_cast<int>(message.sysid)
                                << " tu endpoint khac, bo qua\n";
                        } else {
                            if (!alive) {
                                purgeUavQueue(message.sysid);
                                ++uav.generation;

                                uav.address = sender;

                                std::cout
                                    << "UAV "
                                    << static_cast<int>(message.sysid)
                                    << " ONLINE tai "
                                    << inet_ntoa(sender.sin_addr)
                                    << ":" << ntohs(sender.sin_port)
                                    << '\n';
                            }

                            uav.online = true;
                            uav.lastHeartbeat = now;
                        }
                    }
                }

                auto route = uavs.find(message.sysid);

                if (route == uavs.end() ||
                    !isOnline(route->second, now) ||
                    !sameAddress(route->second.address, sender)) {
                    continue;
                }

                ++route->second.receivedFrames;

                // Fan-out den tat ca client
                std::string packet = serialize(message);

                for (Client& client : clients) {
                    if (client.socket == INVALID_SOCKET)
                        continue;

                    if (client.pending.size() + packet.size() >
                        MAX_TCP_PENDING) {

                        ++stats.slowClients;
                        std::cout << "Client qua cham\n";
                        disconnect(client);
                    } else {
                        client.pending.append(packet);
                    }
                }

                if (message.msgid == MAVLINK_MSG_ID_COMMAND_ACK) {
                    mavlink_command_ack_t ack{};

                    mavlink_msg_command_ack_decode(
                        &message, &ack
                    );

                    std::cout
                        << "ACK UAV "
                        << static_cast<int>(message.sysid)
                        << " | command=" << ack.command
                        << " | result="
                        << static_cast<int>(ack.result)
                        << '\n';
                }
            }
        }

        removeDisconnected();
    }

    for (const Client& client : clients)
        closesocket(client.socket);

    closesocket(udp);
    closesocket(listener);
    WSACleanup();

    return 0;
}