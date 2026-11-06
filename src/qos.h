#pragma once

#include "network.h"
#include "mavlink_codec.h"
#include "time_utils.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <tuple>

class QosRecorder
{
public:
    bool open(const std::string &point)
    {
        point_ = point;

        auto stamp =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count();

        const std::string prefix =
            "docs/ketqua/E0_" + point + "_" +
            std::to_string(stamp);

        std::string base = prefix;
        unsigned int suffix = 0;

        while (exists(base + ".csv") ||
               exists(base + "_msgid.csv"))
        {

            base = prefix + "_" + std::to_string(++suffix);
        }

        streamFile_.open((base + ".csv").c_str());
        messageFile_.open((base + "_msgid.csv").c_str());

        if (!streamFile_ || !messageFile_)
        {
            std::cerr
                << "QOS: khong mo duoc CSV. "
                << "Tao docs/ketqua truoc.\n";

            return false;
        }

        streamFile_
            << "t_s,point,source_ip,source_port,sysid,compid,"
            << "frames,frame_bytes,frames_per_s,frame_Bps,"
            << "hb_count,hb_intervals,hb_mean_ms,"
            << "hb_jitter_ms,max_silence_ms\n";

        messageFile_
            << "t_s,point,source_ip,source_port,sysid,compid,"
            << "msgid,frames,frame_bytes,frames_per_s,frame_Bps\n";

        streamFile_ << std::fixed << std::setprecision(3);
        messageFile_ << std::fixed << std::setprecision(3);

        streamFile_.flush();
        messageFile_.flush();

        std::cout
            << "QOS CSV: " << base << ".csv\n"
            << "QOS MSGID: " << base << "_msgid.csv\n";

        return true;
    }

    void onFrame(
        const mavlink_message_t &message,
        std::size_t frameBytes,
        const sockaddr_in &source,
        TimePoint now = Clock::now())
    {
        if (finished_ || !streamFile_ || !messageFile_)
        {
            return;
        }

        // Bat dau warm-up tu frame hop le dau tien.
        if (!warmup_)
        {
            warmup_ = true;
            warmupStart_ = now;

            std::cout
                << "QOS: warm-up 10 giay "
                << "tu frame hop le dau tien\n";
        }

        if (!measuring_)
        {
            if (elapsedSeconds(now, warmupStart_) <
                WARMUP_SECONDS)
            {
                return;
            }

            measuring_ = true;
            start_ = lastWrite_ = now;

            std::cout << "QOS: bat dau do 600 giay\n";
        }

        // Het khoang do thi khong dem them frame.
        if (elapsedSeconds(now, start_) >= MEASURE_SECONDS)
        {
            tick(now);
            return;
        }

        // Phan biet endpoint, sysid va compid.
        Key key(
            source.sin_addr.s_addr,
            source.sin_port,
            message.sysid,
            message.compid);

        Stream &stream = streams_[key];

        stream.ip = inet_ntoa(source.sin_addr);
        stream.port = ntohs(source.sin_port);
        stream.sysid = message.sysid;
        stream.compid = message.compid;

        // Khoang lang giua hai frame lien tiep.
        if (stream.frames > 0)
        {
            stream.maxSilenceMs = std::max(
                stream.maxSilenceMs,
                elapsedSeconds(now, stream.lastFrame) * 1000.0);
        }

        stream.lastFrame = now;
        ++stream.frames;
        stream.bytes += frameBytes;

        // Thong ke rieng theo MSGID.
        Count &count = stream.messages[message.msgid];

        ++count.frames;
        count.bytes += frameBytes;

        // Chi do heartbeat cua autopilot.
        if (message.msgid == MAVLINK_MSG_ID_HEARTBEAT)
        {
            mavlink_heartbeat_t heartbeat{};

            mavlink_msg_heartbeat_decode(
                &message,
                &heartbeat);

            if (heartbeat.autopilot == MAV_AUTOPILOT_INVALID ||
                heartbeat.type == MAV_TYPE_GCS)
            {
                return;
            }

            if (stream.heartbeats > 0)
            {
                double interval =
                    elapsedSeconds(
                        now,
                        stream.lastHeartbeat) *
                    1000.0;

                // Welford: cap nhat trung binh va phuong sai.
                ++stream.intervals;

                double difference =
                    interval - stream.meanMs;

                stream.meanMs +=
                    difference / stream.intervals;

                stream.m2 +=
                    difference * (interval - stream.meanMs);
            }

            ++stream.heartbeats;
            stream.lastHeartbeat = now;
        }
    }

    void tick(TimePoint now = Clock::now())
    {
        if (!measuring_ || finished_)
        {
            return;
        }

        const TimePoint end =
            start_ + std::chrono::seconds(MEASURE_SECONDS);

        bool complete = now >= end;
        TimePoint sampleTime = complete ? end : now;

        double window =
            elapsedSeconds(sampleTime, lastWrite_);

        if (!complete && window < REPORT_SECONDS)
        {
            return;
        }

        if (window > 0)
        {
            writeRows(sampleTime, window);
        }

        if (finished_)
        {
            return;
        }

        if (complete)
        {
            finished_ = true;

            std::cout
                << "QOS: da ket thuc 600 giay; "
                << "Gateway/client van tiep tuc chay\n";
        }
    }

private:
    enum
    {
        WARMUP_SECONDS = 10,
        MEASURE_SECONDS = 600,
        REPORT_SECONDS = 5
    };

    using Key = std::tuple<
        unsigned long,
        unsigned short,
        uint32_t,
        uint8_t>;

    struct Count
    {
        unsigned long long frames = 0;
        unsigned long long bytes = 0;

        unsigned long long previousFrames = 0;
        unsigned long long previousBytes = 0;
    };

    struct Stream : Count
    {
        std::string ip;

        unsigned short port = 0;
        uint32_t sysid = 0;
        uint8_t compid = 0;

        TimePoint lastFrame{};
        TimePoint lastHeartbeat{};

        double maxSilenceMs = 0;

        unsigned long long heartbeats = 0;
        unsigned long long intervals = 0;

        double meanMs = 0;
        double m2 = 0;

        std::map<uint32_t, Count> messages;
    };

    static bool exists(const std::string &path)
    {
        std::ifstream file(path.c_str());
        return file.good();
    }

    void writePrefix(
        std::ostream &out,
        double seconds,
        const Stream &stream)
    {
        out
            << seconds << ','
            << point_ << ','
            << stream.ip << ','
            << stream.port << ','
            << stream.sysid << ','
            << static_cast<int>(stream.compid);
    }

    void writeRows(TimePoint now, double window)
    {
        double seconds = elapsedSeconds(now, start_);

        unsigned long long totalFrames = 0;

        for (auto &item : streams_)
        {
            Stream &stream = item.second;

            totalFrames += stream.frames;

            // Tinh ca khoang im lang dang dien ra.
            stream.maxSilenceMs = std::max(
                stream.maxSilenceMs,
                elapsedSeconds(now, stream.lastFrame) * 1000.0);

            writePrefix(streamFile_, seconds, stream);

            streamFile_
                << ',' << stream.frames
                << ',' << stream.bytes
                << ','
                << (stream.frames - stream.previousFrames) / window
                << ','
                << (stream.bytes - stream.previousBytes) / window
                << ',' << stream.heartbeats
                << ',' << stream.intervals
                << ',';

            if (stream.intervals > 0)
            {
                streamFile_ << stream.meanMs;
            }

            streamFile_ << ',';

            if (stream.intervals > 1)
            {
                double variance =
                    stream.m2 / (stream.intervals - 1);

                streamFile_
                    << std::sqrt(std::max(0.0, variance));
            }

            streamFile_
                << ',' << stream.maxSilenceMs
                << '\n';

            stream.previousFrames = stream.frames;
            stream.previousBytes = stream.bytes;

            for (auto &entry : stream.messages)
            {
                Count &count = entry.second;

                writePrefix(messageFile_, seconds, stream);

                messageFile_
                    << ',' << entry.first
                    << ',' << count.frames
                    << ',' << count.bytes
                    << ','
                    << (count.frames - count.previousFrames) / window
                    << ','
                    << (count.bytes - count.previousBytes) / window
                    << '\n';

                count.previousFrames = count.frames;
                count.previousBytes = count.bytes;
            }
        }

        streamFile_.flush();
        messageFile_.flush();

        lastWrite_ = now;

        if (!streamFile_ || !messageFile_)
        {
            finished_ = true;

            std::cerr
                << "QOS: ghi CSV that bai; "
                << "khong dung run nay lam ket qua\n";

            return;
        }

        std::cout
            << "QOS: t=" << seconds
            << " s | frames=" << totalFrames
            << '\n';
    }

    std::ofstream streamFile_;
    std::ofstream messageFile_;

    std::string point_;
    std::map<Key, Stream> streams_;

    bool warmup_ = false;
    bool measuring_ = false;
    bool finished_ = false;

    TimePoint warmupStart_{};
    TimePoint start_{};
    TimePoint lastWrite_{};
};