#pragma once

#include "time_utils.h"

#include <cstdint>
#include <map>

class SequenceTracker
{
public:
    unsigned long long duplicates = 0;
    unsigned long long reordered = 0;
    unsigned long long resyncs = 0;
    unsigned long long uncertainGaps = 0;

    // Tao moc theo doi moi, giu lai cac bo dem da tich luy.
    void restart()
    {
        if (initialized_)
        {
            ++resyncs;

            // Khoang thieu dang cho khong con du chac chan
            // de tiep tuc tinh vao gapEstimate().
            uncertainGaps += pending_;
        }

        recent_.clear();
        pending_ = 0;
        initialized_ = false;
    }

    // Goi mot lan cho moi frame MAVLink hop le.
    void observe(uint8_t seq, TimePoint now)
    {
        // Sau khoang im lang dai, khong suy dien so frame mat
        // chi tu sequence 8 bit. Day la nguong heuristic.
        if (initialized_ &&
            elapsedSeconds(now, lastArrival_) > 2.0)
        {
            restart();
        }

        lastArrival_ = now;

        // Frame dau tien chi dung lam moc.
        if (!initialized_)
        {
            initialized_ = true;
            lastSeq_ = seq;
            highest_ = 0;
            recent_[0] = true;
            return;
        }

        // Tinh khoang cach, co xu ly quay vong 255 -> 0.
        const int delta =
            (static_cast<int>(seq) - lastSeq_ + 256) % 256;

        if (delta == 0)
        {
            ++duplicates;
            return;
        }

        // Buoc tien trong cua so co the phan biet.
        if (delta < 128)
        {
            // Danh dau cac vi tri chua nhan duoc.
            for (int i = 1; i < delta; ++i)
            {
                recent_[highest_ + i] = false;
                ++pending_;
            }

            highest_ += delta;
            lastSeq_ = seq;
            recent_[highest_] = true;

            // Giu cua so gom toi da 128 vi tri sequence.
            while (!recent_.empty() &&
                   recent_.begin()->first < highest_ - 127)
            {
                if (!recent_.begin()->second)
                {
                    --pending_;
                    ++confirmed_;
                }

                recent_.erase(recent_.begin());
            }

            return;
        }

        // Co the la frame cu hoac frame den muon.
        const long long position =
            highest_ - (256 - delta);

        auto old = recent_.find(position);

        if (delta != 128 && old != recent_.end())
        {
            if (old->second)
            {
                ++duplicates;
            }
            else
            {
                // Frame den muon lap vao vi tri dang thieu.
                old->second = true;
                --pending_;
                ++reordered;
            }

            return;
        }

        // Khong phan biet duoc nhay xa, reset hay frame qua cu.
        restart();

        initialized_ = true;
        lastSeq_ = seq;
        highest_ = 0;
        recent_[0] = true;
    }

    // Uoc luong so vi tri sequence con thieu.
    unsigned long long gapEstimate() const
    {
        return confirmed_ + pending_;
    }

private:
    bool initialized_ = false;

    uint8_t lastSeq_ = 0;
    long long highest_ = 0;

    TimePoint lastArrival_{};

    // true: da nhan; false: dang thieu.
    std::map<long long, bool> recent_;

    unsigned long long pending_ = 0;
    unsigned long long confirmed_ = 0;
};