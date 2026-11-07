#pragma once

#include <cmath>
#include <cstdint>
#include <map>
#include <random>
#include <stdexcept>

namespace Markov
{

    struct Config
    {
        double urb = 0.5;
        double dur_s = 2.0;
        std::uint64_t slot_ms = 100;
        double lossgood = 0.0;
        double lossbad = 1.0;
        std::uint32_t seed = 1;

        double pBG() const
        {
            return (static_cast<double>(slot_ms) / 1000.0) / dur_s;
        }

        double pGB() const
        {
            return urb == 0.0
                       ? 0.0
                       : pBG() * urb / (1.0 - urb);
        }

        void validate() const
        {
            if (!std::isfinite(urb) ||
                !std::isfinite(dur_s) ||
                !std::isfinite(lossgood) ||
                !std::isfinite(lossbad))
            {
                throw std::invalid_argument(
                    "Tham so phai huu han");
            }

            if (urb < 0.0 || urb >= 1.0 ||
                dur_s <= 0.0 || slot_ms == 0 ||
                lossgood < 0.0 || lossgood > 1.0 ||
                lossbad < 0.0 || lossbad > 1.0)
            {
                throw std::invalid_argument(
                    "Tham so ngoai mien hop le");
            }

            if (urb > 0.0 &&
                (!std::isfinite(pBG()) ||
                 !std::isfinite(pGB()) ||
                 pBG() <= 0.0 || pBG() > 1.0 ||
                 pGB() < 0.0 || pGB() > 1.0))
            {
                throw std::invalid_argument(
                    "Xac suat chuyen trang thai khong hop le");
            }
        }
    };

    struct Stats
    {
        std::uint64_t seen = 0;
        std::uint64_t kept = 0;
        std::uint64_t dropped = 0;

        // Chi dem cac khe thoi gian da ket thuc.
        std::uint64_t slots = 0;
        std::uint64_t badSlots = 0;

        // Chi dem dot B quan sat du ca dau va cuoi.
        std::uint64_t completeBadRuns = 0;
        std::uint64_t completeBadMs = 0;

        // Bat dau phep do khi kenh da o B.
        std::uint64_t initialPartialBadRuns = 0;

        double badFraction() const
        {
            return slots
                       ? static_cast<double>(badSlots) / slots
                       : 0.0;
        }

        double dropFraction() const
        {
            return seen
                       ? static_cast<double>(dropped) / seen
                       : 0.0;
        }

        double meanBadSeconds() const
        {
            return completeBadRuns
                       ? static_cast<double>(completeBadMs) / completeBadRuns / 1000.0
                       : 0.0;
        }
    };

    class Channel
    {
    public:
        Channel(
            const Config &config,
            int sysid,
            std::uint64_t start_ms)
            : config_(config),
              last_ms_(start_ms),
              boundary_ms_(start_ms),
              badStart_ms_(start_ms)
        {
            config_.validate();

            if (sysid < 1 || sysid > 255)
            {
                throw std::invalid_argument(
                    "sysid phai nam trong 1..255");
            }

            // Seed rieng theo UAV va muc dich RNG.
            std::seed_seq stateSeed{
                config_.seed,
                static_cast<std::uint32_t>(sysid),
                0x53544154u};

            std::seed_seq dropSeed{
                config_.seed,
                static_cast<std::uint32_t>(sysid),
                0x44524F50u};

            stateRng_.seed(stateSeed);
            dropRng_.seed(dropSeed);

            // Khoi tao theo phan bo dung.
            bad_ = config_.urb > 0.0 &&
                   draw(stateRng_) < config_.urb;

            if (bad_)
            {
                initialPartial_ = true;
                stats_.initialPartialBadRuns = 1;
            }
        }

        void tick(std::uint64_t now_ms)
        {
            if (now_ms < last_ms_)
            {
                throw std::invalid_argument(
                    "Thoi gian khong duoc quay lui");
            }

            last_ms_ = now_ms;

            while (now_ms - boundary_ms_ >= config_.slot_ms)
            {
                // Ghi nhan khe vua ket thuc.
                ++stats_.slots;

                if (bad_)
                {
                    ++stats_.badSlots;
                }

                boundary_ms_ += config_.slot_ms;

                if (config_.urb == 0.0)
                {
                    continue;
                }

                // Moi khe chi mot lan chuyen tu trang thai cu.
                const double random = draw(stateRng_);

                if (bad_)
                {
                    if (random < config_.pBG())
                    {
                        if (!initialPartial_)
                        {
                            ++stats_.completeBadRuns;
                            stats_.completeBadMs +=
                                boundary_ms_ - badStart_ms_;
                        }

                        initialPartial_ = false;
                        bad_ = false;
                    }
                }
                else if (random < config_.pGB())
                {
                    bad_ = true;
                    badStart_ms_ = boundary_ms_;
                }
            }
        }

        bool keepFrame(std::uint64_t now_ms)
        {
            tick(now_ms);
            ++stats_.seen;

            // urb=0: tat emulator, giu moi frame.
            const double loss = config_.urb == 0.0
                                    ? 0.0
                                    : (bad_ ? config_.lossbad : config_.lossgood);

            const bool keep = draw(dropRng_) >= loss;

            if (keep)
            {
                ++stats_.kept;
            }
            else
            {
                ++stats_.dropped;
            }

            return keep;
        }

        bool isBad() const
        {
            return bad_;
        }

        bool hasOpenBadRun() const
        {
            // Dot nay chua ket thuc, khong dua vao mean.
            return bad_;
        }

        const Stats &stats() const
        {
            return stats_;
        }

    private:
        static double draw(std::mt19937 &rng)
        {
            // Anh xa truc tiep ve [0, 1).
            return static_cast<double>(rng()) / 4294967296.0;
        }

        Config config_;
        Stats stats_;

        std::mt19937 stateRng_;
        std::mt19937 dropRng_;

        bool bad_ = false;
        bool initialPartial_ = false;

        std::uint64_t last_ms_;
        std::uint64_t boundary_ms_;
        std::uint64_t badStart_ms_;
    };

    class Emulator
    {
    public:
        explicit Emulator(const Config &config)
            : config_(config)
        {
            config_.validate();
        }

        void addUav(int sysid, std::uint64_t start_ms)
        {
            if (channels_.find(sysid) == channels_.end())
            {
                channels_.emplace(
                    sysid, Channel(config_, sysid, start_ms));
            }
        }

        void tick(std::uint64_t now_ms)
        {
            for (auto &entry : channels_)
            {
                entry.second.tick(now_ms);
            }
        }

        bool keepFrame(int sysid, std::uint64_t now_ms)
        {
            addUav(sysid, now_ms);
            return channels_.at(sysid).keepFrame(now_ms);
        }

        const Channel &channel(int sysid) const
        {
            return channels_.at(sysid);
        }

    private:
        Config config_;
        std::map<int, Channel> channels_;
    };

} // namespace Markov