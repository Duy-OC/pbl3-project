#include "network.h"
#include "config.h"
#include "mavlink_codec.h"
#include "uav_manager.h"
#include "client_manager.h"
#include "statistics.h"
#include "qos.h"
#include "emulator.h"
#include <windows.h>
#include <winerror.h>

#include <deque>
#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <cerrno>
#include <cctype>
#include <stdexcept>
#include <set>

struct RunOptions
{
    Markov::Config markov;
    std::string scenario = "I19_OFF";
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

static bool fileExists(const std::string &path)
{
    std::ifstream file(path.c_str());
    return file.good();
}

static RunOptions parseOptions(int argc, char **argv)
{
    RunOptions options;
    options.markov.urb = 0.0;
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
        else
        {
            char *end = nullptr;
            errno = 0;
            const double number = std::strtod(value.c_str(), &end);
            if (errno || end == value.c_str() || *end != '\0')
                throw std::invalid_argument("Gia tri so khong hop le: " + value);
            if (key == "--urb") options.markov.urb = number;
            else if (key == "--dur-s") options.markov.dur_s = number;
            else if (key == "--lossgood") options.markov.lossgood = number;
            else if (key == "--lossbad") options.markov.lossbad = number;
            else if (key == "--test-sendto-invalid-addrlen-once")
            {
                if (number != 0.0 && number != 1.0)
                    throw std::invalid_argument("Gia tri phai la 0 hoac 1: " + value);
                options.testSendtoInvalidAddrlenOnce = number == 1.0;
            }
            else if (key == "--slot-ms" || key == "--seed" ||
                     key == "--test-hold-udp-ms")
            {
                if (number < 0 || !std::isfinite(number) ||
                    number != std::floor(number) ||
                    number > (key == "--seed" ? 4294967295.0 :
                              key == "--test-hold-udp-ms" ? 300000.0 :
                              9007199254740991.0))
                    throw std::invalid_argument("So nguyen khong hop le: " + value);
                if (key == "--seed") options.markov.seed = static_cast<std::uint32_t>(number);
                else if (key == "--test-hold-udp-ms")
                    options.testHoldUdpMs = static_cast<std::uint64_t>(number);
                else options.markov.slot_ms = static_cast<std::uint64_t>(number);
            }
            else throw std::invalid_argument("Tham so khong biet: " + key);
        }
    }
    options.markov.validate();
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
        : options_(options), emulator_(options.markov) {}
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
    bool udpSendReady(TimePoint now) const;

    void purgeUavQueue(int sysid);
    void purgeClientQueue(SOCKET owner);

    SOCKET udp_ = INVALID_SOCKET;
    SOCKET listener_ = INVALID_SOCKET;

    ClientManager clients_;
    UavManager uavs_;

    RunOptions options_;
    Markov::Emulator emulator_;
    QosRecorder ingressQos_;
    QosRecorder egressQos_;
    bool windowSet_ = false;
    bool summaryWritten_ = false;
    TimePoint emulatorStart_{};
    TimePoint measurementEnd_{};
    struct FrameCounts { std::uint64_t in = 0, kept = 0, dropped = 0; };
    std::map<int, FrameCounts> counts_;
    std::set<int> loggedStates_;
    std::ofstream stateFile_;
    std::string filePrefix_;
    std::uint64_t emulatorMs(TimePoint now) const
    {
        return static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now - emulatorStart_).count());
    }
    void finishMeasurement();

    Statistics stats_;
    StatisticsReporter reporter_;

    using SourceKey =
        std::pair<unsigned long, unsigned short>;

    std::map<SourceKey, UdpSource> sources_;
    std::deque<UdpFrame> outgoingUdp_;
    bool testSendtoInjected_ = false;
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
        return 1;

    if (!CreateDirectoryA("docs/ketqua/issue19", nullptr) &&
        GetLastError() != ERROR_ALREADY_EXISTS)
    {
        std::cerr << "Khong tao duoc docs/ketqua/issue19\n";
        return 1;
    }
    filePrefix_ = "docs/ketqua/issue19/" + options_.scenario + "_" + options_.runId;
    if (fileExists(filePrefix_ + "_config.txt") ||
        fileExists(filePrefix_ + "_states.csv") ||
        fileExists(filePrefix_ + "_markov.csv") ||
        fileExists(filePrefix_ + "_gateway_ingress.csv") ||
        fileExists(filePrefix_ + "_gateway_ingress_msgid.csv") ||
        fileExists(filePrefix_ + "_gateway_egress.csv") ||
        fileExists(filePrefix_ + "_gateway_egress_msgid.csv"))
    {
        std::cerr << "run_id da ton tai; hay chon run_id khac\n";
        return 1;
    }
    stateFile_.open((filePrefix_ + "_states.csv").c_str());
    if (!stateFile_)
        return 1;
    {
        std::ofstream configFile((filePrefix_ + "_config.txt").c_str());
        if (!configFile)
            return 1;
        const auto &m = options_.markov;
        configFile << "scenario=" << options_.scenario << "\nrun_id=" << options_.runId
                   << "\nurb=" << m.urb << "\ndur_s=" << m.dur_s
                   << "\nslot_ms=" << m.slot_ms << "\nlossgood=" << m.lossgood
                   << "\nlossbad=" << m.lossbad << "\nseed=" << m.seed
                   << "\ntest_hold_udp_ms=" << options_.testHoldUdpMs
                   << "\ntest_sendto_invalid_addrlen_once="
                   << (options_.testSendtoInvalidAddrlenOnce ? 1 : 0)
                   << "\nwarmup_s=10\nmeasure_s=600\nroute_timeout_s=5\n"
                   << "ingress_timeout_s=5\negress_timeout_s=3\n";
        configFile.close();
        if (!configFile)
            return 1;
    }
    stateFile_ << "run_id,sysid,t_ms,event,state\n";
    emulator_.setStateCallback([this](int sysid, std::uint64_t at, bool bad)
    {
        if (summaryWritten_)
            return;
        const bool initial = loggedStates_.insert(sysid).second;
        stateFile_ << options_.runId << ',' << sysid << ',' << at << ','
                   << (initial ? "INITIAL" : "TRANSITION") << ','
                   << (bad ? 'B' : 'G') << '\n';
        stateFile_.flush();
    });
    if (!ingressQos_.open(
            "gateway_ingress",
            Config::HEARTBEAT_TIMEOUT_SECONDS,
            options_.scenario, options_.runId, "docs/ketqua/issue19") ||
        !egressQos_.open(
            "gateway_egress", 3.0,
            options_.scenario, options_.runId, "docs/ketqua/issue19"))
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

        if (udpSendReady(Clock::now()))
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

void Gateway::finishMeasurement()
{
    summaryWritten_ = true;
    const std::uint64_t endMs = emulatorMs(measurementEnd_);
    for (const auto &entry : emulator_.channels())
        stateFile_ << options_.runId << ',' << entry.first << ',' << endMs
                   << ",END," << (entry.second.isBad() ? 'B' : 'G') << '\n';
    stateFile_.flush();

    std::ofstream out((filePrefix_ + "_markov.csv").c_str());
    out << "run_id,sysid,frames_in,frames_kept,frames_dropped_emu,"
           "ingress_qos_frames,egress_qos_frames,run_slots,run_bad_slots,"
           "run_bad_fraction,measurement_drop_fraction,run_complete_bad_runs,"
           "run_mean_bad_s,run_initial_partial_bad_runs,run_final_open_bad_run\n";
    for (const auto &entry : emulator_.channels())
    {
        const int sysid = entry.first;
        const FrameCounts &c = counts_[sysid];
        const Markov::Channel &channel = entry.second;
        const Markov::Stats &s = channel.stats();
        const auto ingress = ingressQos_.totalFrames(sysid);
        const auto egress = egressQos_.totalFrames(sysid);
        out << options_.runId << ',' << sysid << ',' << c.in << ',' << c.kept
            << ',' << c.dropped << ',' << ingress << ',' << egress << ','
            << s.slots << ',' << s.badSlots << ',' << s.badFraction() << ','
            << (c.in ? static_cast<double>(c.dropped) / c.in : 0.0) << ','
            << s.completeBadRuns << ',' << s.meanBadSeconds() << ','
            << s.initialPartialBadRuns << ',' << channel.hasOpenBadRun() << '\n';
        if (c.in != c.kept + c.dropped || ingress != c.in || egress != c.kept)
            std::cerr << "QOS doi soat loi: UAV " << sysid << '\n';
    }
    std::cout << "Issue19: da ghi " << filePrefix_ << "_markov.csv\n";
}

void Gateway::maintenance()
{
    TimePoint now = Clock::now();

    ingressQos_.tick(now);
    egressQos_.tick(now);
    if (windowSet_)
    {
        emulator_.tick(emulatorMs(
            summaryWritten_ ? now : std::min(now, measurementEnd_)));
        if (ingressQos_.finished() && !summaryWritten_)
            finishMeasurement();
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
        std::cout << "QUEUE FULL: client=" << client.id
                  << " queued=" << outgoingUdp_.size()
                  << " incoming=" << destinations.size()
                  << " limit=" << Config::MAX_UDP_PENDING << '\n';

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
        frame.queuedAt = Clock::now();

        frame.destination = uav.address;
        frame.data = packet;

        outgoingUdp_.push_back(frame);

        if (options_.testHoldUdpMs != 0 && isCommand(message))
            std::cout << "QUEUE: target_system=" << sysid
                      << " hold_ms=" << options_.testHoldUdpMs
                      << " queued=" << outgoingUdp_.size() << '\n';

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

        if (!windowSet_)
        {
            windowSet_ = true;
            emulatorStart_ = now;
            const TimePoint measurementStart = now + std::chrono::seconds(10);
            measurementEnd_ = measurementStart + std::chrono::seconds(600);
            ingressQos_.setSharedWindow(measurementStart);
            egressQos_.setSharedWindow(measurementStart);
            std::cout << "Issue19: warm-up 10 s; do 600 s, run_id="
                      << options_.runId << '\n';
        }

        ingressQos_.onFrame(
            message,
            packet.size(),
            sender,
            now);

        const bool keep = emulator_.keepFrame(message.sysid, emulatorMs(now));
        if (ingressQos_.inMeasurementWindow(now))
        {
            FrameCounts &count = counts_[message.sysid];
            ++count.in;
            if (keep) ++count.kept;
            else ++count.dropped;
        }

        if (keep)
        {
            egressQos_.onFrame(message, packet.size(), sender, now);
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

int main(int argc, char **argv)
{
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    RunOptions options;
    try { options = parseOptions(argc, argv); }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << "\nUsage: gateway.exe [--scenario NAME] "
                  << "[--run-id ID] [--urb N] [--dur-s N] [--slot-ms N] "
                  << "[--lossgood N] [--lossbad N] [--seed N] "
                  << "[--test-hold-udp-ms N] "
                  << "[--test-sendto-invalid-addrlen-once 0|1]\n";
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
