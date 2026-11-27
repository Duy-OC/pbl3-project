#include "network.h"
#include "config.h"
#include "mavlink_codec.h"
#include "uav_manager.h"
#include "client_manager.h"
#include "statistics.h"
#include "central_link.h"
#include "probe_handler.h"
#include <cmath>
#include <windows.h>
#include <winerror.h>

#include <deque>
#include <map>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <cerrno>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <set>

struct RunOptions
{
#if defined(GATEWAY_PROFILE_A)
    unsigned short udpPort = 14560;
    unsigned short tcpPort = 5760;
    std::string scenario = "gateway_a";
    bool centralMode = true;
    std::string gatewayId = "A";
#elif defined(GATEWAY_PROFILE_B)
    unsigned short udpPort = 14561;
    unsigned short tcpPort = 5761;
    std::string scenario = "gateway_b";
    bool centralMode = true;
    std::string gatewayId = "B";
#else
    unsigned short udpPort = Config::UDP_PORT;
    unsigned short tcpPort = Config::TCP_PORT;
    std::string scenario = "DEFAULT";
    bool centralMode = false;
    std::string gatewayId = "A";
#endif
    std::string centralIp = "127.0.0.1";
    unsigned short centralPort = 14700;
    std::string runId;
    std::uint64_t testHoldUdpMs = 0;
    bool testSendtoInvalidAddrlenOnce = false;
};

static std::string safeName(const std::string &value)
{
    if (value.empty())
        throw std::invalid_argument("Ten khong duoc rong");
    for (char c : value)
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-')
            throw std::invalid_argument("Ten chi duoc chua a-z, 0-9, _ hoac -");
    return value;
}

static RunOptions parseOptions(int argc, char **argv)
{
    RunOptions options;
    options.runId = std::to_string(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    for (int i = 1; i < argc; i += 2)
    {
        if (i + 1 >= argc)
            throw std::invalid_argument("Thieu gia tri cho tham so");
        const std::string key(argv[i]);
        const std::string value(argv[i + 1]);
        if (key == "--scenario") options.scenario = safeName(value);
        else if (key == "--run-id") options.runId = safeName(value);
        else if (key == "--gateway-id") options.gatewayId = safeName(value);
        else if (key == "--central-ip")
        {
            options.centralIp = value;
            options.centralMode = true;
        }
        else
        {
            char *end = nullptr;
            errno = 0;
            const double number = std::strtod(value.c_str(), &end);
            if (errno || end == value.c_str() || *end != '\0')
                throw std::invalid_argument("Gia tri so khong hop le: " + value);
            if (key == "--udp-port" || key == "--tcp-port" || key == "--central-port")
            {
                if (!std::isfinite(number) || number < 1 || number > 65535 || number != std::floor(number)) throw std::invalid_argument("Port khong hop le: " + key);
                if (key == "--udp-port") options.udpPort = static_cast<unsigned short>(number);
                else if (key == "--tcp-port") options.tcpPort = static_cast<unsigned short>(number);
                else
                {
                    options.centralPort = static_cast<unsigned short>(number);
                    options.centralMode = true;
                }
            }
            else if (key == "--central")
            {
                if (number != 0.0 && number != 1.0)
                    throw std::invalid_argument("Gia tri phai la 0 hoac 1: " + value);
                options.centralMode = number == 1.0;
            }
            else if (key == "--test-sendto-invalid-addrlen-once")
            {
                if (number != 0.0 && number != 1.0)
                    throw std::invalid_argument("Gia tri phai la 0 hoac 1: " + value);
                options.testSendtoInvalidAddrlenOnce = number == 1.0;
            }
            else if (key == "--test-hold-udp-ms")
            {
                if (number < 0 || !std::isfinite(number) ||
                    number != std::floor(number) || number > 300000.0)
                    throw std::invalid_argument("So nguyen khong hop le: " + value);
                options.testHoldUdpMs = static_cast<std::uint64_t>(number);
            }
            else throw std::invalid_argument("Tham so khong biet: " + key);
        }
    }
    return options;
}

struct UdpFrame
{
    SOCKET owner;
    int target;
    unsigned int generation;
    TimePoint queuedAt{};

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
    explicit Gateway(const RunOptions &options)
        : options_(options) {}
    ~Gateway();

    Gateway(const Gateway &) = delete;
    Gateway &operator=(const Gateway &) = delete;

    int run();

private:
    bool openSockets();
    void handleText(Client &client, const std::string &line);
    void sendCommand(int sysid, unsigned int owner, uint16_t command, float param1, float param7, unsigned int attempt);
    std::string detail(TimePoint now) const;
    void maintenance();

    bool routeClientFrame(
        const Client &client,
        const mavlink_message_t &message);

    void receiveUavData();
    void receiveCentralData();
    void sendPendingUdp();
    bool udpSendReady(TimePoint now) const;

    void purgeUavQueue(int sysid);
    void purgeClientQueue(SOCKET owner);

    SOCKET udp_ = INVALID_SOCKET;
    SOCKET listener_ = INVALID_SOCKET;
    SOCKET controller_ = INVALID_SOCKET;
    TimePoint lastGcsHeartbeat_{};
    TimePoint lastManualNoRouteLog_{};
    struct PendingCommand
    {
        SOCKET owner = INVALID_SOCKET;
        int sysid = 0;
        uint16_t command = 0;
        float param1 = 0;
        float param7 = 0;
        unsigned int attempts = 0;
        TimePoint sentAt{};
        unsigned int generation = 0;
    };
    std::map<int, PendingCommand> commands_;

    ClientManager clients_;
    UavManager uavs_;

    RunOptions options_;
    Statistics stats_;
    StatisticsReporter reporter_;

    using SourceKey =
        std::pair<unsigned long, unsigned short>;

    std::map<SourceKey, UdpSource> sources_;
    std::deque<UdpFrame> outgoingUdp_;
    bool testSendtoInjected_ = false;
    CentralLink centralLink_;
    ProbeHandler probeHandler_;
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
        options_.udpPort);

    if (udp_ == INVALID_SOCKET)
    {
        return false;
    }

    probeHandler_.setGatewayId(options_.gatewayId);

    if (options_.centralMode)
    {
        std::string err;
        CentralLink::Config cfg;
        cfg.gatewayId = options_.gatewayId;
        cfg.centralIp = options_.centralIp;
        cfg.centralPort = options_.centralPort;
        if (!centralLink_.open(cfg, err))
        {
            std::cerr << "CentralLink loi: " << err << '\n';
            return false;
        }
        return true;
    }

    listener_ = Network::createBoundSocket(
        SOCK_STREAM,
        options_.tcpPort);

    if (listener_ == INVALID_SOCKET)
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
        return 1;

    std::cout
        << "=== PBL3 GATEWAY: HEADER MODULES ===\n"
        << "Gateway ID: " << options_.gatewayId << '\n'
        << "UDP: 0.0.0.0:" << options_.udpPort << '\n';
    if (options_.centralMode)
    {
        std::cout << "Central UDP: " << options_.centralIp << ":" << options_.centralPort << '\n';
    }
    else
    {
        std::cout << "TCP: 127.0.0.1:" << options_.tcpPort << '\n';
    }
    std::cout
        << "Dialect: development\n"
        << "Heartbeat timeout: "
        << Config::HEARTBEAT_TIMEOUT_SECONDS << " giay\n\n";

    while (true)
    {
        maintenance();
        if (options_.centralMode)
        {
            centralLink_.tick();
        }

        fd_set reads;
        fd_set writes;

        FD_ZERO(&reads);
        FD_ZERO(&writes);

        FD_SET(udp_, &reads);
        if (options_.centralMode && centralLink_.isOpen())
        {
            FD_SET(centralLink_.socket(), &reads);
        }
        else if (listener_ != INVALID_SOCKET)
        {
            FD_SET(listener_, &reads);
        }

        if (udpSendReady(Clock::now()))
        {
            FD_SET(udp_, &writes);
        }

        if (!options_.centralMode)
        {
            clients_.prepareSets(reads, writes);
        }

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

        if (!options_.centralMode)
        {
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
                [this](Client &client, const std::string &line)
                { handleText(client, line); },
                [this](SOCKET owner)
                {
                    purgeClientQueue(owner);
                });
        }

        if (FD_ISSET(udp_, &writes))
        {
            sendPendingUdp();
        }

        if (options_.centralMode)
        {
            if (centralLink_.isOpen() && FD_ISSET(centralLink_.socket(), &reads))
            {
                receiveCentralData();
            }
        }
        else if (listener_ != INVALID_SOCKET && FD_ISSET(listener_, &reads))
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

    if (lastGcsHeartbeat_ == TimePoint{} || elapsedSeconds(now, lastGcsHeartbeat_) >= 1.0)
    {
        lastGcsHeartbeat_ = now;
        mavlink_message_t heartbeat{};
        mavlink_msg_heartbeat_pack(255, MAV_COMP_ID_MISSIONPLANNER, &heartbeat,
                                   MAV_TYPE_GCS, MAV_AUTOPILOT_INVALID, 0, 0, MAV_STATE_ACTIVE);
        for (const auto &item : uavs_.all())
            if (isOnline(item.second, now) && outgoingUdp_.size() < Config::MAX_UDP_PENDING)
            {
                UdpFrame frame{};
                frame.owner = INVALID_SOCKET;
                frame.target = item.first;
                frame.generation = item.second.generation;
                frame.destination = item.second.address;
                frame.data = serialize(heartbeat);
                outgoingUdp_.push_back(frame);
            }
    }

    clients_.forEach([this, now](Client &client)
    {
        if (client.textMode && client.subscribeHz > 0 && now >= client.nextDetail)
        {
            clients_.queue(client.socket, detail(now));
            client.nextDetail = now + std::chrono::milliseconds(1000 / client.subscribeHz);
        }
    });

    for (auto it = commands_.begin(); it != commands_.end();)
    {
        PendingCommand command = it->second;
        const Uav *uav = uavs_.get(command.sysid);
        if (!uav || !isOnline(*uav, now) || uav->generation != command.generation)
        {
            clients_.queue(command.owner, "RESULT " + std::to_string(command.sysid) + " OFFLINE\n");
            it = commands_.erase(it);
        }
        else if (elapsedSeconds(now, command.sentAt) >= 1.0)
        {
            if (command.attempts >= 3)
            {
                clients_.queue(command.owner, "RESULT " + std::to_string(command.sysid) + " TIMEOUT\n");
                it = commands_.erase(it);
            }
            else
            {
                sendCommand(command.sysid, 0, command.command, command.param1, command.param7, command.attempts + 1);
                ++it;
            }
        }
        else ++it;
    }

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

    const std::size_t cancelled = before - outgoingUdp_.size();
    stats_.droppedUdpFrames += cancelled;
    if (cancelled != 0)
        std::cout << "QUEUE CANCEL: reason=route_offline sysid=" << sysid
                  << " count=" << cancelled << '\n';
}

void Gateway::purgeClientQueue(SOCKET owner)
{
    if (controller_ == owner) controller_ = INVALID_SOCKET;
    for (auto it = commands_.begin(); it != commands_.end();)
        if (it->second.owner == owner) it = commands_.erase(it); else ++it;
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

    const std::size_t cancelled = before - outgoingUdp_.size();
    stats_.droppedUdpFrames += cancelled;
    if (cancelled != 0)
        std::cout << "QUEUE CANCEL: reason=client_disconnected count="
                  << cancelled << '\n';
}

std::string Gateway::detail(TimePoint now) const
{
    std::string result;
    for (const auto &item : uavs_.all())
        if (isOnline(item.second, now))
            result += "DETAIL " + item.second.telemetry.toJsonString(item.first, now) + "\n";
    return result.empty() ? "DETAIL NONE\n" : result;
}

void Gateway::handleText(Client &client, const std::string &line)
{
    std::istringstream input(line);
    std::string action, extra;
    input >> action;
    auto reply = [this, &client](const std::string &value)
    {
        clients_.queue(client.socket, value + "\n");
    };

    if (action == "GET_DETAIL")
    {
        int sysid = 0;
        if (input >> sysid)
        {
            const Uav *uav = uavs_.get(sysid);
            if (sysid < 1 || (input >> extra)) reply("ERR USAGE GET_DETAIL [sysid]");
            else if (!uav || !isOnline(*uav, Clock::now())) reply("DETAIL NONE");
            else reply("DETAIL " + uav->telemetry.toJsonString(sysid, Clock::now()));
        }
        else if (input.eof()) clients_.queue(client.socket, detail(Clock::now()));
        else reply("ERR USAGE GET_DETAIL [sysid]");
    }
    else if (action == "SUBSCRIBE")
    {
        int hz = 0;
        if (!(input >> hz) || hz < 1 || hz > 5 || (input >> extra))
            reply("ERR USAGE SUBSCRIBE 1..5");
        else
        {
            client.subscribeHz = hz;
            client.nextDetail = Clock::now();
            reply("OK SUBSCRIBE " + std::to_string(hz));
        }
    }
    else if (action == "UNSUBSCRIBE")
    {
        client.subscribeHz = 0;
        reply("OK UNSUBSCRIBE");
    }
    else if (action == "ACQUIRE")
    {
        if (input >> extra) reply("ERR USAGE ACQUIRE");
        else if (controller_ == INVALID_SOCKET || controller_ == client.socket)
        {
            controller_ = client.socket;
            reply("OK ACQUIRE");
        }
        else reply("ERR LOCKED");
    }
    else if (action == "RELEASE")
    {
        if (controller_ != client.socket) reply("ERR NOT_OWNER");
        else
        {
            controller_ = INVALID_SOCKET;
            for (auto it = commands_.begin(); it != commands_.end();)
                if (it->second.owner == client.socket) it = commands_.erase(it);
                else ++it;
            purgeClientQueue(client.socket);
            reply("OK RELEASE");
        }
    }
    else if (action == "COMMAND")
    {
        int sysid = 0;
        std::string name;
        float altitude = 10.0f;
        if (!(input >> sysid >> name) || sysid < 1 || sysid > 255)
        {
            reply("ERR USAGE COMMAND sysid ARM|DISARM|TAKEOFF|LAND [altitude_m]");
            return;
        }
        if (name == "TAKEOFF")
        {
            input >> std::ws;
            if (!input.eof())
            {
                if (!(input >> altitude) || !std::isfinite(altitude) || altitude < 1 || altitude > 100 || (input >> extra))
                {
                    reply("ERR ALTITUDE 1..100");
                    return;
                }
            }
        }
        else if (input >> extra) { reply("ERR USAGE COMMAND sysid ARM|DISARM|TAKEOFF|LAND"); return; }

        if (controller_ != client.socket) { reply("ERR NOT_OWNER"); return; }
        const Uav *uav = uavs_.get(sysid);
        if (!uav || !isOnline(*uav, Clock::now())) { reply("ERR UAV_OFFLINE"); return; }
        if (commands_.count(sysid)) { reply("ERR COMMAND_BUSY"); return; }

        uint16_t command = 0;
        float param1 = 0;
        if (name == "ARM") { command = MAV_CMD_COMPONENT_ARM_DISARM; param1 = 1; }
        else if (name == "DISARM") { command = MAV_CMD_COMPONENT_ARM_DISARM; param1 = 0; }
        else if (name == "TAKEOFF") command = MAV_CMD_NAV_TAKEOFF;
        else if (name == "LAND") command = MAV_CMD_NAV_LAND;
        else { reply("ERR UNKNOWN_COMMAND"); return; }

        PendingCommand pending{};
        pending.owner = client.socket;
        pending.sysid = sysid;
        pending.command = command;
        pending.param1 = param1;
        pending.param7 = name == "TAKEOFF" ? altitude : 0;
        pending.generation = uav->generation;
        commands_[sysid] = pending;
        sendCommand(sysid, 0, command, param1, pending.param7, 1);
        reply("OK COMMAND_PENDING " + std::to_string(sysid) + " " + std::to_string(command));
    }
    else reply("ERR UNKNOWN_COMMAND");
}

void Gateway::sendCommand(int sysid, unsigned int, uint16_t command,
                          float param1, float param7, unsigned int attempt)
{
    auto found = commands_.find(sysid);
    const Uav *uav = uavs_.get(sysid);
    if (found == commands_.end() || !uav) return;
    if (outgoingUdp_.size() >= Config::MAX_UDP_PENDING) return;

    mavlink_message_t message{};
    mavlink_msg_command_long_pack(255, MAV_COMP_ID_MISSIONPLANNER, &message,
        static_cast<uint8_t>(sysid), MAV_COMP_ID_AUTOPILOT1, command,
        static_cast<uint8_t>(attempt - 1), param1, 0, 0, 0, 0, 0, param7);
    UdpFrame frame{};
    frame.owner = found->second.owner;
    frame.target = sysid;
    frame.generation = uav->generation;
    frame.destination = uav->address;
    frame.data = serialize(message);
    outgoingUdp_.push_back(frame);
    found->second.attempts = attempt;
    found->second.sentAt = Clock::now();
}

bool Gateway::routeClientFrame(
    const Client &client,
    const mavlink_message_t &message)
{
    if (message.msgid == MAVLINK_MSG_ID_HEARTBEAT)
    {
        // The gateway injects its own GCS heartbeat toward each UAV. A
        // heartbeat from a TCP client (QGC) is ignored, not a policy block.
        return true;
    }

    // QGC uses COMMAND_LONG/INT for telemetry and capability discovery.
    // A raw MAVLink GCS (QGC) may also control the UAV directly. It takes
    // the same controller lock as the text ACQUIRE/COMMAND protocol and
    // acquires it automatically on the first control message.
    const bool commandFrame = message.msgid == MAVLINK_MSG_ID_COMMAND_LONG ||
                              message.msgid == MAVLINK_MSG_ID_COMMAND_INT;
    uint16_t commandId = 0;
    if (message.msgid == MAVLINK_MSG_ID_COMMAND_LONG)
    {
        mavlink_command_long_t decoded{};
        mavlink_msg_command_long_decode(&message, &decoded);
        commandId = decoded.command;
    }
    else if (message.msgid == MAVLINK_MSG_ID_COMMAND_INT)
    {
        mavlink_command_int_t decoded{};
        mavlink_msg_command_int_decode(&message, &decoded);
        commandId = decoded.command;
    }

    const bool manualControl = message.msgid == MAVLINK_MSG_ID_MANUAL_CONTROL;
    const bool gcsFlightCommand = commandFrame &&
        (commandId == MAV_CMD_NAV_LAND ||
         commandId == MAV_CMD_NAV_TAKEOFF ||
         commandId == MAV_CMD_DO_REPOSITION ||
         commandId == MAV_CMD_DO_SET_MODE ||
         commandId == MAV_CMD_COMPONENT_ARM_DISARM);
    const bool gcsControlCommand = !client.textMode &&
        (message.msgid == MAVLINK_MSG_ID_SET_MODE ||
         manualControl || gcsFlightCommand);

    const bool telemetryCommand = commandFrame &&
        (commandId == MAV_CMD_GET_MESSAGE_INTERVAL ||
         commandId == MAV_CMD_SET_MESSAGE_INTERVAL ||
         commandId == MAV_CMD_REQUEST_MESSAGE ||
         commandId == MAV_CMD_REQUEST_PROTOCOL_VERSION ||
         commandId == MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES ||
         commandId == MAV_CMD_RUN_PREARM_CHECKS ||
         commandId == MAV_CMD_GET_HOME_POSITION);

    int target = getTargetSystem(message);
    if (manualControl)
    {
        mavlink_manual_control_t control{};
        mavlink_msg_manual_control_decode(&message, &control);
        target = control.target;
        if (target == 0)
        {
            ++stats_.blockedMessages;
            ++stats_.blockedByPolicy;
            return true;
        }
    }

    if (gcsControlCommand && target <= 0)
    {
        // Never broadcast a flight command to every UAV. QGC targets one
        // vehicle; a target of 0 is treated as invalid for control.
        ++stats_.blockedMessages;
        ++stats_.blockedByPolicy;
        std::cout << "GCS CONTROL BLOCK: Client #" << client.id
                  << " msgid=" << message.msgid
                  << " command=" << commandId
                  << " reason=target_khong_hop_le"
                  << '\n';
        return true;
    }

    if (manualControl && client.textMode && controller_ != client.socket)
    {
        // Text clients still need ACQUIRE before MANUAL_CONTROL.
        ++stats_.blockedMessages;
        ++stats_.blockedByPolicy;
        return true;
    }

    if (message.msgid != MAVLINK_MSG_ID_PING &&
        message.msgid != MAVLINK_MSG_ID_PARAM_REQUEST_READ &&
        message.msgid != MAVLINK_MSG_ID_PARAM_REQUEST_LIST &&
        message.msgid != MAVLINK_MSG_ID_PARAM_EXT_REQUEST_READ &&
        message.msgid != MAVLINK_MSG_ID_PARAM_EXT_REQUEST_LIST &&
        message.msgid != MAVLINK_MSG_ID_REQUEST_DATA_STREAM &&
        message.msgid != MAVLINK_MSG_ID_REQUEST_EVENT &&
        message.msgid != MAVLINK_MSG_ID_TIMESYNC &&
        message.msgid != MAVLINK_MSG_ID_MISSION_REQUEST_LIST &&
        message.msgid != MAVLINK_MSG_ID_MISSION_REQUEST_INT &&
        message.msgid != MAVLINK_MSG_ID_MISSION_REQUEST &&
        message.msgid != MAVLINK_MSG_ID_MANUAL_CONTROL &&
        message.msgid != MAVLINK_MSG_ID_MISSION_ACK &&
        !telemetryCommand &&
        !gcsControlCommand)
    {
        ++stats_.blockedMessages;
        ++stats_.blockedByPolicy;
        if (message.msgid == MAVLINK_MSG_ID_COMMAND_LONG ||
            message.msgid == MAVLINK_MSG_ID_COMMAND_INT)
        {
            std::cerr << "[POLICY BLOCK] msgid=" << message.msgid
                      << " command=" << commandId
                      << " target=" << target
                      << " from_sysid=" << static_cast<int>(message.sysid)
                      << '\n';
        }
        return true;
    }

    if (target < 0)
    {
        ++stats_.blockedMessages;
        ++stats_.blockedUnknownMetadata;
        return true;
    }
    if (telemetryCommand && commandId == MAV_CMD_SET_MESSAGE_INTERVAL && target == 0)
    {
        ++stats_.blockedMessages;
        ++stats_.blockedByPolicy;
        return true;
    }

    TimePoint now = Clock::now();
    std::vector<int> destinations =
        uavs_.destinations(target, now);

    if (destinations.empty())
    {
        ++stats_.blockedMessages;
        ++stats_.blockedNoRoute;

        const bool throttleManual =
            message.msgid == MAVLINK_MSG_ID_MANUAL_CONTROL &&
            lastManualNoRouteLog_ != TimePoint{} &&
            elapsedSeconds(now, lastManualNoRouteLog_) < 1.0;
        if (target != 0 && !throttleManual)
        {
            if (message.msgid == MAVLINK_MSG_ID_MANUAL_CONTROL)
            {
                lastManualNoRouteLog_ = now;
            }
            std::cout
                << "BLOCK: target_system=" << target
                << " offline/chua hoc"
                << " | MSGID=" << message.msgid
                << '\n';
        }

        return true;
    }

    if (gcsControlCommand)
    {
        // The route exists; only now take the controller lock so a failed
        // command (offline UAV) does not lock out other clients.
        if (controller_ == INVALID_SOCKET)
        {
            controller_ = client.socket;
            std::cout << "GCS CONTROL: Client #" << client.id
                      << " giu quyen dieu khien\n";
        }
        else if (controller_ != client.socket)
        {
            ++stats_.blockedMessages;
            ++stats_.blockedByPolicy;
            if (message.msgid != MAVLINK_MSG_ID_MANUAL_CONTROL)
            {
                std::cout << "GCS CONTROL BLOCK: Client #" << client.id
                          << " msgid=" << message.msgid
                          << " command=" << commandId
                          << " reason=client_khac_dang_giu_quyen"
                          << '\n';
            }
            return true;
        }
    }

    if (outgoingUdp_.size() + destinations.size() >
        Config::MAX_UDP_PENDING)
    {
        std::cout << "QUEUE FULL: client=" << client.id
                  << " queued=" << outgoingUdp_.size()
                  << " incoming=" << destinations.size()
                  << " limit=" << Config::MAX_UDP_PENDING << '\n';

        // ClientManager se ngat client nay
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
        frame.queuedAt = Clock::now();
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

bool Gateway::udpSendReady(TimePoint now) const
{
    if (outgoingUdp_.empty())
        return false;
    if (options_.testHoldUdpMs == 0)
        return true;
    const auto age = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - outgoingUdp_.front().queuedAt).count();
    return age >= static_cast<std::int64_t>(options_.testHoldUdpMs);
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

    const bool injectInvalidAddrlen =
        options_.testSendtoInvalidAddrlenOnce && !testSendtoInjected_;
    if (injectInvalidAddrlen)
    {
        testSendtoInjected_ = true;
        std::cout << "TEST SENDTO: first queued frame uses addrlen=0\n";
    }
    int sent = sendto(
        udp_,
        frame.data.data(),
        static_cast<int>(frame.data.size()),
        0,
        reinterpret_cast<const sockaddr *>(
            &frame.destination),
        injectInvalidAddrlen ? 0 : sizeof(frame.destination));

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

    if (probeHandler_.handleDatagram(buffer, static_cast<std::size_t>(n), sender, udp_))
    {
        return;
    }

    if (options_.centralMode && EnvelopeCodec::isEnvelope(buffer, static_cast<std::size_t>(n)))
    {
        EnvelopeCodec::Envelope env;
        if (EnvelopeCodec::decode(buffer, static_cast<std::size_t>(n), env) &&
            (env.type == EnvelopeCodec::MessageType::LinkReport ||
             env.type == EnvelopeCodec::MessageType::HandoverAck))
        {
            centralLink_.sendEnvelope(env);
        }
        return;
    }

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

        if (options_.centralMode)
        {
            centralLink_.sendTelemetry(message.sysid, packet.data(), packet.size());
        }
        else
        {
            clients_.broadcast(
                packet,
                stats_,
                [this](SOCKET owner)
                {
                    purgeClientQueue(owner);
                });
        }

        if (message.msgid == MAVLINK_MSG_ID_COMMAND_ACK)
        {
            mavlink_command_ack_t ack{};

            mavlink_msg_command_ack_decode(
                &message,
                &ack);

            auto pending = commands_.find(message.sysid);
            if (pending != commands_.end() && pending->second.command == ack.command &&
                message.compid == MAV_COMP_ID_AUTOPILOT1 &&
                (ack.target_system == 0 || ack.target_system == 255) &&
                (ack.target_component == 0 || ack.target_component == MAV_COMP_ID_MISSIONPLANNER) &&
                pending->second.generation == uavs_.get(message.sysid)->generation)
            {
                if (ack.result == MAV_RESULT_IN_PROGRESS)
                    pending->second.sentAt = Clock::now();
                else
                {
                    clients_.queue(pending->second.owner,
                        "RESULT " + std::to_string(message.sysid) + " " +
                        std::to_string(ack.command) + " " + std::to_string(ack.result) + "\n");
                    commands_.erase(pending);
                }
            }

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

void Gateway::receiveCentralData()
{
    std::vector<EnvelopeCodec::Envelope> incoming;
    centralLink_.poll(incoming);
    for (const auto &env : incoming)
    {
        if (env.type == EnvelopeCodec::MessageType::CommandForward && !env.payload.empty())
        {
            const Uav *uav = uavs_.get(env.sysid);
            if (!uav || outgoingUdp_.size() >= Config::MAX_UDP_PENDING)
            {
                continue;
            }
            UdpFrame frame{};
            frame.owner = INVALID_SOCKET;
            frame.target = env.sysid;
            frame.generation = uav->generation;
            frame.destination = uav->address;
            frame.data.assign(reinterpret_cast<const char *>(env.payload.data()), env.payload.size());
            outgoingUdp_.push_back(std::move(frame));
        }
        else if (env.type == EnvelopeCodec::MessageType::HandoverRequest)
        {
            const Uav *uav = uavs_.get(env.sysid);
            if (!uav || outgoingUdp_.size() >= Config::MAX_UDP_PENDING)
            {
                continue;
            }
            const auto wire = EnvelopeCodec::encode(env);
            UdpFrame frame{};
            frame.owner = INVALID_SOCKET;
            frame.target = env.sysid;
            frame.generation = uav->generation;
            frame.destination = uav->address;
            frame.data.assign(reinterpret_cast<const char *>(wire.data()), wire.size());
            outgoingUdp_.push_back(std::move(frame));
        }
    }
}

int main(int argc, char **argv)
{
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    RunOptions options;
    try { options = parseOptions(argc, argv); }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << "\nUsage: gateway.exe [--scenario NAME] "
                  << "[--udp-port PORT] [--tcp-port PORT] "
                  << "[--run-id ID] "
                  << "[--test-hold-udp-ms N] "
                  << "[--test-sendto-invalid-addrlen-once 0|1] "
                  << "\n";
        return 1;
    }
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
    Gateway gateway(options);

    return gateway.run();
}
