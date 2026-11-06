#include "network.h"
#include "config.h"
#include "mavlink_codec.h"
#include "uav_manager.h"
#include "client_manager.h"
#include "statistics.h"
#include "qos.h"

#include <deque>
#include <map>
#include <algorithm>
#include <iostream>

struct UdpFrame
{
    SOCKET owner;
    int target;
    unsigned int generation;

    sockaddr_in destination;
    std::string data;
};

struct UdpSource
{
    MavlinkParser parser;
    TimePoint lastSeen{};
};

class Gateway
{
public:
    Gateway() = default;
    ~Gateway();

    Gateway(const Gateway &) = delete;
    Gateway &operator=(const Gateway &) = delete;

    int run();

private:
    bool openSockets();
    void maintenance();

    bool routeClientFrame(
        const Client &client,
        const mavlink_message_t &message);

    void receiveUavData();
    void sendPendingUdp();

    void purgeUavQueue(int sysid);
    void purgeClientQueue(SOCKET owner);

    SOCKET udp_ = INVALID_SOCKET;
    SOCKET listener_ = INVALID_SOCKET;

    ClientManager clients_;
    UavManager uavs_;

    QosRecorder qos_;

    Statistics stats_;
    StatisticsReporter reporter_;

    using SourceKey =
        std::pair<unsigned long, unsigned short>;

    std::map<SourceKey, UdpSource> sources_;
    std::deque<UdpFrame> outgoingUdp_;
};

Gateway::~Gateway()
{
    Network::closeSocket(udp_);
    Network::closeSocket(listener_);
}

bool Gateway::openSockets()
{
    udp_ = Network::createBoundSocket(
        SOCK_DGRAM,
        Config::UDP_PORT);

    listener_ = Network::createBoundSocket(
        SOCK_STREAM,
        Config::TCP_PORT);

    if (udp_ == INVALID_SOCKET ||
        listener_ == INVALID_SOCKET)
    {
        return false;
    }

    if (listen(listener_, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr
            << "listen loi: " << WSAGetLastError() << '\n';

        return false;
    }

    return true;
}

int Gateway::run()
{
    if (!openSockets())
    {
        return 1;
    }

    if (!qos_.open("gateway_ingress"))
    {
        return 1;
    }

    std::cout
        << "=== PBL3 GATEWAY: HEADER MODULES ===\n"
        << "UDP: 0.0.0.0:" << Config::UDP_PORT << '\n'
        << "TCP: 127.0.0.1:" << Config::TCP_PORT << '\n'
        << "Dialect: development\n"
        << "Heartbeat timeout: "
        << Config::HEARTBEAT_TIMEOUT_SECONDS << " giay\n\n";

    while (true)
    {
        maintenance();

        fd_set reads;
        fd_set writes;

        FD_ZERO(&reads);
        FD_ZERO(&writes);

        FD_SET(udp_, &reads);
        FD_SET(listener_, &reads);

        if (!outgoingUdp_.empty())
        {
            FD_SET(udp_, &writes);
        }

        clients_.prepareSets(reads, writes);

        timeval timeout{};
        timeout.tv_usec = Config::SELECT_TIMEOUT_US;

        int ready = select(
            0,
            &reads,
            &writes,
            nullptr,
            &timeout);

        if (ready == SOCKET_ERROR)
        {
            std::cerr
                << "select loi: "
                << WSAGetLastError() << '\n';

            return 1;
        }

        maintenance();

        clients_.processReady(
            reads,
            writes,
            stats_,
            [this](
                const Client &client,
                const mavlink_message_t &message)
            {
                return routeClientFrame(client, message);
            },
            [this](SOCKET owner)
            {
                purgeClientQueue(owner);
            });

        if (FD_ISSET(udp_, &writes))
        {
            sendPendingUdp();
        }

        if (FD_ISSET(listener_, &reads))
        {
            clients_.acceptOne(listener_);
        }

        if (FD_ISSET(udp_, &reads))
        {
            receiveUavData();
        }
    }
}

void Gateway::maintenance()
{
    TimePoint now = Clock::now();

    qos_.tick(now);

    for (int sysid : uavs_.expire(now))
    {
        purgeUavQueue(sysid);
    }

    for (auto it = sources_.begin(); it != sources_.end();)
    {
        if (elapsedSeconds(now, it->second.lastSeen) >
            Config::SOURCE_IDLE_SECONDS)
        {

            it = sources_.erase(it);
        }
        else
        {
            ++it;
        }
    }

    reporter_.printIfDue(
        stats_,
        uavs_.all(),
        clients_.size(),
        outgoingUdp_.size(),
        now);
}

void Gateway::purgeUavQueue(int sysid)
{
    std::size_t before = outgoingUdp_.size();

    outgoingUdp_.erase(
        std::remove_if(
            outgoingUdp_.begin(),
            outgoingUdp_.end(),
            [sysid](const UdpFrame &frame)
            {
                return frame.target == sysid;
            }),
        outgoingUdp_.end());

    stats_.droppedUdpFrames +=
        before - outgoingUdp_.size();
}

void Gateway::purgeClientQueue(SOCKET owner)
{
    std::size_t before = outgoingUdp_.size();

    outgoingUdp_.erase(
        std::remove_if(
            outgoingUdp_.begin(),
            outgoingUdp_.end(),
            [owner](const UdpFrame &frame)
            {
                return frame.owner == owner;
            }),
        outgoingUdp_.end());

    stats_.droppedUdpFrames +=
        before - outgoingUdp_.size();
}

bool Gateway::routeClientFrame(
    const Client &client,
    const mavlink_message_t &message)
{
    int target = getTargetSystem(message);

    if (target < 0)
    {
        ++stats_.blockedMessages;
        ++stats_.blockedUnknownMetadata;

        return true;
    }

    std::vector<int> destinations =
        uavs_.destinations(target, Clock::now());

    if (destinations.empty())
    {
        ++stats_.blockedMessages;
        ++stats_.blockedNoRoute;

        if (target != 0)
        {
            std::cout
                << "BLOCK: target_system=" << target
                << " offline/chua hoc"
                << " | MSGID=" << message.msgid
                << '\n';
        }

        return true;
    }

    if (outgoingUdp_.size() + destinations.size() >
        Config::MAX_UDP_PENDING)
    {

        std::cout << "Hang doi UDP day\n";

        // ClientManager se ngat client nay.
        return false;
    }

    std::string packet = serialize(message);

    for (int sysid : destinations)
    {
        const Uav &uav = uavs_.all().at(sysid);

        UdpFrame frame;

        frame.owner = client.socket;
        frame.target = sysid;
        frame.generation = uav.generation;

        frame.destination = uav.address;
        frame.data = packet;

        outgoingUdp_.push_back(frame);

        if (isCommand(message))
        {
            std::cout
                << "ROUTE: Client #" << client.id
                << " -> UAV " << sysid
                << " -> "
                << inet_ntoa(uav.address.sin_addr)
                << ":" << ntohs(uav.address.sin_port)
                << '\n';
        }
    }

    return true;
}

void Gateway::sendPendingUdp()
{
    if (outgoingUdp_.empty())
    {
        return;
    }

    const UdpFrame &frame = outgoingUdp_.front();

    bool valid = uavs_.routeValid(
        frame.target,
        frame.generation,
        frame.destination,
        Clock::now());

    if (!valid)
    {
        ++stats_.droppedUdpFrames;
        outgoingUdp_.pop_front();

        return;
    }

    int sent = sendto(
        udp_,
        frame.data.data(),
        static_cast<int>(frame.data.size()),
        0,
        reinterpret_cast<const sockaddr *>(
            &frame.destination),
        sizeof(frame.destination));

    if (sent == SOCKET_ERROR)
    {
        int error = WSAGetLastError();

        if (error == WSAEWOULDBLOCK)
        {
            return;
        }

        std::cerr << "sendto loi: " << error << '\n';
        ++stats_.droppedUdpFrames;
    }
    else if (
        sent == static_cast<int>(frame.data.size()))
    {
        ++stats_.udpTxFrames;
    }
    else
    {
        ++stats_.droppedUdpFrames;
    }

    outgoingUdp_.pop_front();
}

void Gateway::receiveUavData()
{
    char buffer[65536];

    sockaddr_in sender{};
    int senderSize = sizeof(sender);

    int n = recvfrom(
        udp_,
        buffer,
        sizeof(buffer),
        0,
        reinterpret_cast<sockaddr *>(&sender),
        &senderSize);

    if (n == SOCKET_ERROR)
    {
        int error = WSAGetLastError();

        if (error != WSAEWOULDBLOCK)
        {
            std::cerr << "recvfrom loi: "
                      << error << '\n';
        }

        return;
    }

    ++stats_.udpDatagrams;
    stats_.udpRxBytes += n;

    SourceKey key(
        sender.sin_addr.s_addr,
        sender.sin_port);

    auto source = sources_.find(key);

    if (source == sources_.end())
    {
        if (sources_.size() >= Config::MAX_UDP_SOURCES)
        {
            return;
        }

        source = sources_.emplace(
                             key,
                             UdpSource{})
                     .first;
    }

    source->second.lastSeen = Clock::now();

    MavlinkParser &parser = source->second.parser;

    for (int i = 0; i < n; ++i)
    {
        uint8_t status = parser.feed(
            static_cast<uint8_t>(buffer[i]));

        if (!stats_.acceptParse(status, true))
        {
            continue;
        }

        ++stats_.uavFrames;

        const mavlink_message_t &message = parser.message;
        TimePoint now = Clock::now();

        if (uavs_.observeHeartbeat(message, sender, now))
        {
            purgeUavQueue(message.sysid);
        }

        if (!uavs_.acceptTelemetry(message, sender, now))
        {
            continue;
        }

        std::string packet = serialize(message);

        qos_.onFrame(
            message,
            packet.size(),
            sender,
            now);

        clients_.broadcast(
            serialize(message),
            stats_,
            [this](SOCKET owner)
            {
                purgeClientQueue(owner);
            });

        if (message.msgid == MAVLINK_MSG_ID_COMMAND_ACK)
        {
            mavlink_command_ack_t ack{};

            mavlink_msg_command_ack_decode(
                &message,
                &ack);

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

    // Gateway bi huy truoc Runtime:
    // dong socket truoc khi WSACleanup.
    Gateway gateway;

    return gateway.run();
}