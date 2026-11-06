#pragma once

#include "network.h"
#include "mavlink_codec.h"
#include "time_utils.h"
#include "sequence_tracker.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <tuple>

class QosRecorder
{
public:
    bool open(
        const std::string &point,
        double offlineTimeoutSeconds = 3.0)
    {
        if (!std::isfinite(offlineTimeoutSeconds) ||
            offlineTimeoutSeconds <= 0.0)
        {
            std::cerr << "QOS: timeout khong hop le.\n";
            return false;
        }

        offlineTimeoutSeconds_ = offlineTimeoutSeconds;
        point_ = point;

        // Tao ten file rieng cho moi lan chay.
        auto stamp =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count();

        const std::string prefix =
            "docs/ketqua/E0_" +
            point +
            "_" +
            std::to_string(stamp);

        std::string base = prefix;
        unsigned int suffix = 0;

        while (exists(base + ".csv") ||
               exists(base + "_msgid.csv"))
        {
            base =
                prefix +
                "_" +
                std::to_string(++suffix);
        }

        streamFile_.open((base + ".csv").c_str());

        messageFile_.open(
            (base + "_msgid.csv").c_str());

        if (!streamFile_ || !messageFile_)
        {
            std::cerr
                << "QOS: khong mo duoc CSV.\n"
                << "Hay tao docs/ketqua va chay "
                << "tu thu muc goc du an.\n";

            return false;
        }

        // CSV tong hop theo nguon/sysid/compid.
        streamFile_
            << "t_s,point,source_ip,source_port,sysid,compid,"
            << "frames,frame_bytes,frames_per_s,frame_Bps,"
            << "hb_count,hb_intervals,hb_mean_ms,hb_jitter_ms,"
            << "max_silence_ms,"
            << "seq_gap_estimate,seq_duplicates,seq_reordered,"
            << "seq_resyncs,seq_uncertain_gaps,"
            << "hb_online,offline_events,offline_time_s\n";

        // CSV phan loai theo msgid.
        messageFile_
            << "t_s,point,source_ip,source_port,sysid,compid,"
            << "msgid,frames,frame_bytes,"
            << "frames_per_s,frame_Bps\n";

        streamFile_
            << std::fixed
            << std::setprecision(3);

        messageFile_
            << std::fixed
            << std::setprecision(3);

        streamFile_.flush();
        messageFile_.flush();

        std::cout
            << "QOS CSV: " << base << ".csv\n"
            << "QOS MSGID: " << base << "_msgid.csv\n"
            << "QOS timeout: "
            << offlineTimeoutSeconds_
            << " giay\n";

        return true;
    }

    // Goi mot lan cho moi frame MAVLink hop le.
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

        // Warm-up bat dau tu frame hop le dau tien.
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
            start_ = now;
            lastWrite_ = now;

            std::cout
                << "QOS: bat dau do 600 giay\n";
        }

        // Khong dem frame ngoai cua so do.
        if (elapsedSeconds(now, start_) >=
            MEASURE_SECONDS)
        {
            tick(now);
            return;
        }

        updateOutages(now);

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

        // Neu boot time quay lui, dat moc sequence moi.
        if (message.msgid == MAVLINK_MSG_ID_SYSTEM_TIME)
        {
            mavlink_system_time_t systemTime{};

            mavlink_msg_system_time_decode(
                &message,
                &systemTime);

            if (stream.hasBootTime &&
                systemTime.time_boot_ms <
                    stream.lastBootMs)
            {
                stream.sequence.restart();
            }

            stream.hasBootTime = true;
            stream.lastBootMs = systemTime.time_boot_ms;
        }

        stream.sequence.observe(message.seq, now);

        // Khoang lang giua cac frame cua cung nguon.
        if (stream.frames > 0)
        {
            stream.maxSilenceMs = std::max(
                stream.maxSilenceMs,
                elapsedSeconds(now, stream.lastFrame) * 1000.0);
        }

        stream.lastFrame = now;

        ++stream.frames;
        stream.bytes += frameBytes;

        // Thong ke rieng tung msgid.
        Count &count = stream.messages[message.msgid];

        ++count.frames;
        count.bytes += frameBytes;

        if (message.msgid != MAVLINK_MSG_ID_HEARTBEAT)
        {
            return;
        }

        mavlink_heartbeat_t heartbeat{};

        mavlink_msg_heartbeat_decode(
            &message,
            &heartbeat);

        // Chi dung heartbeat autopilot de do online/offline.
        if (heartbeat.autopilot == MAV_AUTOPILOT_INVALID ||
            heartbeat.type == MAV_TYPE_GCS)
        {
            return;
        }

        // Heartbeat quay lai sau mot dot offline.
        if (stream.offline)
        {
            stream.offlineSeconds += elapsedSeconds(
                now,
                stream.offlineSince);

            stream.offline = false;

            std::cout
                << "QOS " << point_
                << ": UAV " << stream.sysid
                << " ONLINE tai t="
                << elapsedSeconds(now, start_)
                << " s\n";
        }

        // Welford: tinh mean va phuong sai khoang heartbeat.
        if (stream.heartbeats > 0)
        {
            double intervalMs =
                elapsedSeconds(
                    now,
                    stream.lastHeartbeat) *
                1000.0;

            ++stream.intervals;

            double difference =
                intervalMs - stream.meanMs;

            stream.meanMs +=
                difference / stream.intervals;

            stream.m2 +=
                difference * (intervalMs - stream.meanMs);
        }

        ++stream.heartbeats;
        stream.lastHeartbeat = now;
    }

    // Goi dinh ky, ke ca khi khong co frame den.
    void tick(TimePoint now = Clock::now())
    {
        if (!measuring_ || finished_)
        {
            return;
        }

        const TimePoint end =
            start_ +
            std::chrono::seconds(MEASURE_SECONDS);

        bool complete = now >= end;

        // Gioi han dong cuoi dung tai moc 600 giay.
        TimePoint sampleTime = complete ? end : now;

        updateOutages(sampleTime);

        double window =
            elapsedSeconds(sampleTime, lastWrite_);

        if (!complete && window < REPORT_SECONDS)
        {
            return;
        }

        if (window > 0.0)
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

        double maxSilenceMs = 0.0;

        unsigned long long heartbeats = 0;
        unsigned long long intervals = 0;

        double meanMs = 0.0;
        double m2 = 0.0;

        std::map<uint32_t, Count> messages;

        SequenceTracker sequence;

        bool hasBootTime = false;
        uint32_t lastBootMs = 0;

        bool offline = false;
        TimePoint offlineSince{};

        unsigned long long offlineEvents = 0;
        double offlineSeconds = 0.0;
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

    void updateOutages(TimePoint now)
    {
        for (auto &item : streams_)
        {
            Stream &stream = item.second;

            // Chua tung co heartbeat thi chua bat dau
            // theo doi outage cho nguon nay.
            if (stream.heartbeats == 0 ||
                stream.offline)
            {
                continue;
            }

            double silence = elapsedSeconds(
                now,
                stream.lastHeartbeat);

            if (silence < offlineTimeoutSeconds_)
            {
                continue;
            }

            stream.offline = true;

            // Tinh tu dung nguong timeout,
            // khong tu thoi diem vong lap phat hien.
            stream.offlineSince =
                stream.lastHeartbeat +
                std::chrono::duration_cast<Clock::duration>(
                    std::chrono::duration<double>(
                        offlineTimeoutSeconds_));

            ++stream.offlineEvents;

            std::cout
                << "QOS " << point_
                << ": UAV " << stream.sysid
                << " OFFLINE tai t="
                << elapsedSeconds(
                       stream.offlineSince,
                       start_)
                << " s\n";
        }
    }

    void writeRows(
        TimePoint now,
        double window)
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
                elapsedSeconds(
                    now,
                    stream.lastFrame) *
                    1000.0);

            double frameRate =
                (stream.frames - stream.previousFrames) /
                window;

            double byteRate =
                (stream.bytes - stream.previousBytes) /
                window;

            writePrefix(
                streamFile_,
                seconds,
                stream);

            streamFile_
                << ',' << stream.frames
                << ',' << stream.bytes
                << ',' << frameRate
                << ',' << byteRate
                << ',' << stream.heartbeats
                << ',' << stream.intervals
                << ',';

            // Can it nhat mot khoang de tinh mean.
            if (stream.intervals > 0)
            {
                streamFile_ << stream.meanMs;
            }

            streamFile_ << ',';

            // Can it nhat hai khoang de tinh
            // do lech chuan mau.
            if (stream.intervals > 1)
            {
                double variance =
                    stream.m2 / (stream.intervals - 1);

                double jitter = std::sqrt(
                    std::max(0.0, variance));

                streamFile_ << jitter;
            }

            streamFile_
                << ',' << stream.maxSilenceMs
                << ',' << stream.sequence.gapEstimate()
                << ',' << stream.sequence.duplicates
                << ',' << stream.sequence.reordered
                << ',' << stream.sequence.resyncs
                << ',' << stream.sequence.uncertainGaps
                << ',';

            // De trong neu chua co heartbeat autopilot.
            if (stream.heartbeats > 0)
            {
                streamFile_
                    << (stream.offline ? 0 : 1);
            }

            double offlineTime =
                stream.offlineSeconds;

            // Cong them dot offline chua ket thuc.
            if (stream.offline)
            {
                offlineTime += elapsedSeconds(
                    now,
                    stream.offlineSince);
            }

            streamFile_
                << ',' << stream.offlineEvents
                << ',' << offlineTime
                << '\n';

            stream.previousFrames = stream.frames;
            stream.previousBytes = stream.bytes;

            // Ghi tung loai thong diep.
            for (auto &entry : stream.messages)
            {
                Count &count = entry.second;

                double messageFrameRate =
                    (count.frames - count.previousFrames) /
                    window;

                double messageByteRate =
                    (count.bytes - count.previousBytes) /
                    window;

                writePrefix(
                    messageFile_,
                    seconds,
                    stream);

                messageFile_
                    << ',' << entry.first
                    << ',' << count.frames
                    << ',' << count.bytes
                    << ',' << messageFrameRate
                    << ',' << messageByteRate
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

    double offlineTimeoutSeconds_ = 3.0;

    TimePoint warmupStart_{};
    TimePoint start_{};
    TimePoint lastWrite_{};
};