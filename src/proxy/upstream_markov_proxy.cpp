#include "network.h"
#include "mavlink_codec.h"
#include "qos.h"
#include "emulator.h"
#include "time_utils.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cerrno>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace
{
struct Options
{
    Markov::Config markov;
    std::string runId;
    std::string scenario = "UPSTREAM";
    int sysid = 1;
    unsigned short px4ListenPort = 14551;
    unsigned short gatewaySourcePort = 14610;
};

std::string safeName(const std::string &value)
{
    if (value.empty())
        throw std::invalid_argument("Ten run khong duoc rong");
    for (char c : value)
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-')
            throw std::invalid_argument("Ten run chi duoc chua chu, so, _ hoac -");
    return value;
}

bool exists(const std::string &path)
{
    std::ifstream in(path.c_str());
    return in.good();
}

Options parseOptions(int argc, char **argv)
{
    Options options;
    options.runId = std::to_string(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    for (int i = 1; i < argc; i += 2)
    {
        if (i + 1 >= argc)
            throw std::invalid_argument("Thieu gia tri tham so");
        const std::string key(argv[i]);
        const std::string value(argv[i + 1]);
        if (key == "--run-id") options.runId = safeName(value);
        else if (key == "--scenario") options.scenario = safeName(value);
        else
        {
            char *end = nullptr;
            errno = 0;
            const double number = std::strtod(value.c_str(), &end);
            if (errno || end == value.c_str() || *end != '\0' || !std::isfinite(number))
                throw std::invalid_argument("So khong hop le: " + value);
            if (key == "--urb") options.markov.urb = number;
            else if (key == "--dur-s") options.markov.dur_s = number;
            else if (key == "--lossgood") options.markov.lossgood = number;
            else if (key == "--lossbad") options.markov.lossbad = number;
            else if (key == "--seed" || key == "--slot-ms" ||
                     key == "--sysid" || key == "--listen-port" ||
                     key == "--gateway-source-port")
            {
                const double maximum = key == "--seed" ? 4294967295.0 :
                                       key == "--slot-ms" ? 9007199254740991.0 :
                                       key == "--sysid" ? 255.0 : 65535.0;
                if (number < 1 || number > maximum || number != std::floor(number))
                    throw std::invalid_argument("So nguyen ngoai mien: " + value);
                if (key == "--seed") options.markov.seed = static_cast<std::uint32_t>(number);
                else if (key == "--slot-ms") options.markov.slot_ms = static_cast<std::uint64_t>(number);
                else if (key == "--sysid") options.sysid = static_cast<int>(number);
                else if (key == "--listen-port") options.px4ListenPort = static_cast<unsigned short>(number);
                else options.gatewaySourcePort = static_cast<unsigned short>(number);
            }
            else throw std::invalid_argument("Tham so khong biet: " + key);
        }
    }
    if (options.px4ListenPort == options.gatewaySourcePort ||
        options.px4ListenPort == 14550 || options.gatewaySourcePort == 14550)
        throw std::invalid_argument("Cong proxy trung nhau hoac trung Gateway 14550");
    options.markov.validate();
    return options;
}

class Proxy
{
public:
    explicit Proxy(const Options &options) : options_(options) {}
    ~Proxy()
    {
        Network::closeSocket(px4Socket_);
        Network::closeSocket(gatewaySocket_);
    }

    int run()
    {
        px4Socket_ = Network::createBoundSocket(SOCK_DGRAM, options_.px4ListenPort);
        gatewaySocket_ = Network::createBoundSocket(SOCK_DGRAM, options_.gatewaySourcePort);
        if (px4Socket_ == INVALID_SOCKET || gatewaySocket_ == INVALID_SOCKET)
            return 1;

        gatewayAddress_.sin_family = AF_INET;
        gatewayAddress_.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        gatewayAddress_.sin_port = htons(14550);

        outputDir_ = "docs/ketqua/PBL3-19_GatewayMarkov/upstream/" + options_.runId;
        prefix_ = outputDir_ + "/" + options_.scenario + "_" + options_.runId;
        if (exists(prefix_ + "_config.txt") ||
            exists(prefix_ + "_states.csv") ||
            exists(prefix_ + "_summary.csv") ||
            exists(prefix_ + "_post_trace.csv") ||
            exists(prefix_ + "_proxy_pre.csv") ||
            exists(prefix_ + "_proxy_post.csv"))
        {
            std::cerr << "run_id da co file; dung run_id moi\n";
            return 1;
        }
        stateFile_.open((prefix_ + "_states.csv").c_str());
        if (!stateFile_) return 1;
        stateFile_ << "run_id,sysid,t_ms,event,state\n";
        postTraceFile_.open((prefix_ + "_post_trace.csv").c_str());
        if (!postTraceFile_) return 1;
        postTraceFile_ << "epoch_ms,sysid,compid,seq,msgid,len,checksum,magic\n";
        {
            std::ofstream config((prefix_ + "_config.txt").c_str());
            if (!config) return 1;
            config << "scenario=" << options_.scenario
                   << "\nrun_id=" << options_.runId
                   << "\nsysid=" << options_.sysid
                   << "\npx4_listen_port=" << options_.px4ListenPort
                   << "\ngateway_source_port=" << options_.gatewaySourcePort
                   << "\ngateway_address=127.0.0.1:14550"
                   << "\nurb=" << options_.markov.urb
                   << "\ndur_s=" << options_.markov.dur_s
                   << "\nslot_ms=" << options_.markov.slot_ms
                   << "\nlossgood=" << options_.markov.lossgood
                   << "\nlossbad=" << options_.markov.lossbad
                   << "\nseed=" << options_.markov.seed
                   << "\nwarmup_s=10\nmeasure_s=600\n";
        }
        if (!preQos_.open("proxy_pre", 5.0, options_.scenario, options_.runId, outputDir_) ||
            !postQos_.open("proxy_post", 3.0, options_.scenario, options_.runId, outputDir_))
            return 1;

        std::cout << "UPSTREAM PROXY: PX4 -> Windows UDP " << options_.px4ListenPort
                  << " -> Markov -> Gateway 127.0.0.1:14550"
                  << " from 127.0.0.1:" << options_.gatewaySourcePort
                  << " | sysid=" << options_.sysid
                  << " urb=" << options_.markov.urb
                  << " dur_s=" << options_.markov.dur_s
                  << " slot_ms=" << options_.markov.slot_ms
                  << " seed=" << options_.markov.seed << '\n';
        auto lastReport = Clock::now();
        while (true)
        {
            const TimePoint now = Clock::now();
            if (channel_)
            {
                channel_->tick(msSinceStart(
                    summaryWritten_ ? now : std::min(now, measurementEnd_)));
                preQos_.tick(now);
                postQos_.tick(now);
                if (!summaryWritten_ && preQos_.finished())
                    finishMeasurement();
            }
            if (elapsedSeconds(now, lastReport) >= 5.0)
            {
                std::cout << "PROXY STATS: pre=" << preFrames_
                          << " kept=" << keptFrames_
                          << " dropped_markov=" << droppedFrames_
                          << " post_sent=" << postFrames_
                          << " send_errors=" << sendErrors_
                          << " reverse_rx=" << reverseRx_
                          << " reverse_tx=" << reverseTx_
                          << " crc_errors=" << crcErrors_ << '\n';
                lastReport = now;
            }

            fd_set reads;
            FD_ZERO(&reads);
            FD_SET(px4Socket_, &reads);
            FD_SET(gatewaySocket_, &reads);
            timeval timeout{};
            timeout.tv_usec = 200000;
            const int ready = select(0, &reads, nullptr, nullptr, &timeout);
            if (ready == SOCKET_ERROR)
            {
                std::cerr << "select loi: " << WSAGetLastError() << '\n';
                return 1;
            }
            if (FD_ISSET(px4Socket_, &reads)) receivePx4();
            if (FD_ISSET(gatewaySocket_, &reads)) receiveGateway();
        }
    }

private:
    std::uint64_t msSinceStart(TimePoint now) const
    {
        return static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now - firstFrame_).count());
    }

    void receivePx4()
    {
        char bytes[65536];
        sockaddr_in sender{};
        int senderLength = sizeof(sender);
        const int n = recvfrom(px4Socket_, bytes, sizeof(bytes), 0,
            reinterpret_cast<sockaddr *>(&sender), &senderLength);
        if (n == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                std::cerr << "recvfrom PX4 loi: " << WSAGetLastError() << '\n';
            return;
        }
        if (sourceKnown_ && !Network::sameAddress(sender, px4Address_))
        {
            ++foreignDatagrams_;
            return;
        }
        for (int i = 0; i < n; ++i)
        {
            const uint8_t status = parser_.feed(static_cast<std::uint8_t>(bytes[i]));
            if (status == MAVLINK_FRAMING_BAD_CRC) { ++crcErrors_; continue; }
            if (status == MAVLINK_FRAMING_BAD_SIGNATURE) { ++signatureErrors_; continue; }
            if (status != MAVLINK_FRAMING_OK) continue;
            const mavlink_message_t &message = parser_.message;
            if (message.sysid != options_.sysid) { ++foreignFrames_; continue; }
            const TimePoint now = Clock::now();
            if (!sourceKnown_)
            {
                px4Address_ = sender;
                sourceKnown_ = true;
                std::cout << "PX4 SOURCE: " << inet_ntoa(sender.sin_addr)
                          << ':' << ntohs(sender.sin_port) << '\n';
            }
            if (!channel_)
            {
                firstFrame_ = now;
                measurementEnd_ = now + std::chrono::seconds(610);
                const TimePoint measurementStart = now + std::chrono::seconds(10);
                preQos_.setSharedWindow(measurementStart);
                postQos_.setSharedWindow(measurementStart);
                channel_.reset(new Markov::Channel(options_.markov, options_.sysid, 0,
                    [this](std::uint64_t at, bool bad)
                    {
                        if (!summaryWritten_)
                        {
                            stateFile_ << options_.runId << ',' << options_.sysid
                                       << ',' << at << ','
                                       << (at == 0 ? "INITIAL" : "TRANSITION")
                                       << ',' << (bad ? 'B' : 'G') << '\n';
                            stateFile_.flush();
                        }
                    }));
                std::cout << "PROXY WINDOW: warmup_s=10 measure_s=600 run_id="
                          << options_.runId << '\n';
            }
            if (!summaryWritten_ && now >= measurementEnd_)
            {
                preQos_.tick(now);
                postQos_.tick(now);
                finishMeasurement();
            }

            const std::string frame = serialize(message);
            if (frame.empty()) { ++serializeErrors_; continue; }
            preQos_.onFrame(message, frame.size(), sender, now);
            const bool inWindow = preQos_.inMeasurementWindow(now);
            const bool keep = channel_->keepFrame(msSinceStart(now));
            const bool bad = channel_->isBad();
            if (inWindow)
            {
                ++preFrames_;
                if (bad) ++preBadFrames_;
                else ++preGoodFrames_;
            }
            if (!keep)
            {
                if (inWindow)
                {
                    ++droppedFrames_;
                    if (bad) ++droppedBadFrames_;
                    else ++droppedGoodFrames_;
                }
                continue;
            }
            if (inWindow) ++keptFrames_;
            const int sent = sendto(gatewaySocket_, frame.data(),
                static_cast<int>(frame.size()), 0,
                reinterpret_cast<const sockaddr *>(&gatewayAddress_),
                sizeof(gatewayAddress_));
            if (sent != static_cast<int>(frame.size()))
            {
                if (inWindow) ++sendErrors_;
                std::cerr << "sendto Gateway loi: " << WSAGetLastError() << '\n';
                continue;
            }
            postQos_.onFrame(message, frame.size(), sender, now);
            const auto epochMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            postTraceFile_ << epochMs << ','
                           << static_cast<unsigned int>(message.sysid) << ','
                           << static_cast<unsigned int>(message.compid) << ','
                           << static_cast<unsigned int>(message.seq) << ','
                           << message.msgid << ','
                           << static_cast<unsigned int>(message.len) << ','
                           << message.checksum << ','
                           << static_cast<unsigned int>(message.magic) << '\n';
            postTraceFile_.flush();
            if (inWindow) ++postFrames_;
        }
    }

    void receiveGateway()
    {
        char bytes[65536];
        sockaddr_in sender{};
        int senderLength = sizeof(sender);
        const int n = recvfrom(gatewaySocket_, bytes, sizeof(bytes), 0,
            reinterpret_cast<sockaddr *>(&sender), &senderLength);
        if (n == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                std::cerr << "recvfrom Gateway loi: " << WSAGetLastError() << '\n';
            return;
        }
        if (!Network::sameAddress(sender, gatewayAddress_))
        {
            ++foreignDatagrams_;
            return;
        }
        ++reverseRx_;
        if (!sourceKnown_)
        {
            std::cerr << "REVERSE BLOCK: PX4 source chua hoc\n";
            return;
        }
        const int sent = sendto(px4Socket_, bytes, n, 0,
            reinterpret_cast<const sockaddr *>(&px4Address_), sizeof(px4Address_));
        if (sent == n)
        {
            ++reverseTx_;
            std::cout << "REVERSE: Gateway -> PX4 "
                      << inet_ntoa(px4Address_.sin_addr) << ':'
                      << ntohs(px4Address_.sin_port)
                      << " bytes=" << sent << '\n';
        }
        else std::cerr << "sendto PX4 loi: " << WSAGetLastError() << '\n';
    }

    void finishMeasurement()
    {
        summaryWritten_ = true;
        channel_->tick(msSinceStart(measurementEnd_));
        stateFile_ << options_.runId << ',' << options_.sysid << ','
                   << msSinceStart(measurementEnd_) << ",END,"
                   << (channel_->isBad() ? 'B' : 'G') << '\n';
        stateFile_.flush();
        const Markov::Stats &s = channel_->stats();
        std::ofstream out((prefix_ + "_summary.csv").c_str());
        out << "run_id,sysid,frames_pre,frames_pre_good,frames_pre_bad,"
               "frames_dropped_good,frames_dropped_bad,frames_kept,frames_dropped_markov,"
               "frames_post_sent,send_errors,pre_qos_frames,post_qos_frames,"
               "slots_including_warmup,bad_slots_including_warmup,"
               "bad_fraction_including_warmup,complete_bad_runs,mean_bad_s,"
               "crc_errors,signature_errors,foreign_frames,foreign_datagrams,"
               "serialize_errors,reverse_rx,reverse_tx\n";
        out << options_.runId << ',' << options_.sysid << ','
            << preFrames_ << ',' << preGoodFrames_ << ',' << preBadFrames_ << ','
            << droppedGoodFrames_ << ',' << droppedBadFrames_ << ','
            << keptFrames_ << ',' << droppedFrames_ << ','
            << postFrames_ << ',' << sendErrors_ << ','
            << preQos_.totalFrames(options_.sysid) << ','
            << postQos_.totalFrames(options_.sysid) << ','
            << s.slots << ',' << s.badSlots << ',' << s.badFraction() << ','
            << s.completeBadRuns << ',' << s.meanBadSeconds() << ','
            << crcErrors_ << ',' << signatureErrors_ << ','
            << foreignFrames_ << ',' << foreignDatagrams_ << ','
            << serializeErrors_ << ',' << reverseRx_ << ',' << reverseTx_ << '\n';
        out.close();
        if (preFrames_ != preGoodFrames_ + preBadFrames_ ||
            droppedFrames_ != droppedGoodFrames_ + droppedBadFrames_ ||
            preFrames_ != keptFrames_ + droppedFrames_ ||
            keptFrames_ != postFrames_ + sendErrors_ ||
            preFrames_ != preQos_.totalFrames(options_.sysid) ||
            postFrames_ != postQos_.totalFrames(options_.sysid))
            std::cerr << "PROXY QOS DOI SOAT LOI\n";
        std::cout << "UPSTREAM RESULT: pre=" << preFrames_
                  << " pre_good=" << preGoodFrames_
                  << " pre_bad=" << preBadFrames_
                  << " kept=" << keptFrames_
                  << " dropped_markov=" << droppedFrames_
                  << " dropped_good=" << droppedGoodFrames_
                  << " dropped_bad=" << droppedBadFrames_
                  << " post_sent=" << postFrames_
                  << " send_errors=" << sendErrors_
                  << " slots_including_warmup=" << s.slots
                  << " summary=" << prefix_ << "_summary.csv\n";
    }

    Options options_;
    SOCKET px4Socket_ = INVALID_SOCKET;
    SOCKET gatewaySocket_ = INVALID_SOCKET;
    sockaddr_in gatewayAddress_{};
    sockaddr_in px4Address_{};
    bool sourceKnown_ = false;
    MavlinkParser parser_;
    QosRecorder preQos_;
    QosRecorder postQos_;
    std::unique_ptr<Markov::Channel> channel_;
    TimePoint firstFrame_{};
    TimePoint measurementEnd_{};
    std::string outputDir_;
    std::string prefix_;
    std::ofstream stateFile_;
    std::ofstream postTraceFile_;
    bool summaryWritten_ = false;
    std::uint64_t preFrames_ = 0;
    std::uint64_t preGoodFrames_ = 0;
    std::uint64_t preBadFrames_ = 0;
    std::uint64_t keptFrames_ = 0;
    std::uint64_t droppedFrames_ = 0;
    std::uint64_t droppedGoodFrames_ = 0;
    std::uint64_t droppedBadFrames_ = 0;
    std::uint64_t postFrames_ = 0;
    std::uint64_t sendErrors_ = 0;
    std::uint64_t reverseRx_ = 0;
    std::uint64_t reverseTx_ = 0;
    std::uint64_t crcErrors_ = 0;
    std::uint64_t signatureErrors_ = 0;
    std::uint64_t foreignFrames_ = 0;
    std::uint64_t foreignDatagrams_ = 0;
    std::uint64_t serializeErrors_ = 0;
};
}

int main(int argc, char **argv)
{
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    Options options;
    try { options = parseOptions(argc, argv); }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << "\nUsage: upstream_markov_proxy.exe "
                  << "[--run-id ID] [--scenario NAME] [--sysid 1..255] "
                  << "[--listen-port N] [--gateway-source-port N] "
                  << "[--urb N] [--dur-s N] [--slot-ms N] "
                  << "[--lossgood N] [--lossbad N] [--seed N]\n";
        return 1;
    }
    Network::Runtime winsock;
    if (winsock.error() != 0)
    {
        std::cerr << "WSAStartup loi: " << winsock.error() << '\n';
        return 1;
    }
    Proxy proxy(options);
    return proxy.run();
}
