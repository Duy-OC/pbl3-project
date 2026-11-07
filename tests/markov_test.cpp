#include "../src/emulator.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

void check(bool condition, const char *message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

bool sameStats(
    const Markov::Stats &a,
    const Markov::Stats &b)
{
    return a.seen == b.seen &&
           a.kept == b.kept &&
           a.dropped == b.dropped &&
           a.slots == b.slots &&
           a.badSlots == b.badSlots &&
           a.completeBadRuns == b.completeBadRuns &&
           a.completeBadMs == b.completeBadMs &&
           a.initialPartialBadRuns ==
               b.initialPartialBadRuns;
}

void expectInvalid(const Markov::Config &config)
{
    bool rejected = false;

    try
    {
        config.validate();
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }

    check(rejected, "Cau hinh sai khong bi tu choi");
}

int main()
{
    try
    {
        Markov::Config config;

        // M01: Tat emulator.
        {
            auto off = config;
            off.urb = 0.0;
            off.lossgood = 1.0;
            off.lossbad = 1.0;

            Markov::Channel channel(off, 1, 0);

            for (std::uint64_t i = 0; i < 10000; ++i)
            {
                check(
                    channel.keepFrame(i * 100),
                    "Emulator tat van drop");
            }

            check(channel.stats().dropped == 0,
                  "Counter drop sai khi tat");
        }

        std::cout << "M01 PASS: disabled\n";

        // M02: Cau hinh sai.
        {
            auto c = config;
            c.urb = 1.0;
            expectInvalid(c);

            c = config;
            c.urb = -0.1;
            expectInvalid(c);

            c = config;
            c.dur_s = 0.0;
            expectInvalid(c);

            c = config;
            c.slot_ms = 0;
            expectInvalid(c);

            c = config;
            c.lossbad = 1.2;
            expectInvalid(c);

            c = config;
            c.urb = 0.9;
            c.dur_s = 0.1;
            expectInvalid(c);

            c = config;
            c.urb =
                std::numeric_limits<double>::quiet_NaN();
            expectInvalid(c);
        }

        std::cout << "M02 PASS: validation\n";

        // M03: Cung seed va lich vao -> cung ket qua.
        {
            Markov::Channel a(config, 1, 0);
            Markov::Channel b(config, 1, 0);

            for (std::uint64_t i = 0; i < 20000; ++i)
            {
                const auto t = i * 37;

                check(
                    a.keepFrame(t) == b.keepFrame(t),
                    "Khong tai lap duoc ket qua");
            }

            check(sameStats(a.stats(), b.stats()),
                  "Counters tai lap khong khop");
        }

        std::cout << "M03 PASS: reproducibility\n";

        // M04: Tick tung khe va tick nhay phai khop.
        {
            Markov::Channel fine(config, 1, 0);
            Markov::Channel jump(config, 1, 0);

            for (std::uint64_t t = 100;
                 t <= 100000;
                 t += 100)
            {
                fine.tick(t);
            }

            jump.tick(100000);

            check(fine.isBad() == jump.isBad(),
                  "Catch-up sai trang thai");

            check(sameStats(fine.stats(), jump.stats()),
                  "Catch-up sai counters");
        }

        std::cout << "M04 PASS: catch-up\n";

        // M05: Khong co frame, kenh van chay.
        {
            Markov::Channel channel(config, 1, 0);
            channel.tick(1000000);

            check(channel.stats().seen == 0,
                  "Tick tu tang so frame");

            check(channel.stats().slots == 10000,
                  "Khong cap nhat khi khong co frame");

            check(channel.stats().badSlots > 0 &&
                      channel.stats().badSlots <
                          channel.stats().slots,
                  "Khong quan sat duoc G va B");

            check(channel.stats().completeBadRuns > 0,
                  "Khong co dot B hoan chinh");
        }

        std::cout << "M05 PASS: no-frame ticking\n";

        // M06: So frame khong lam thay doi chuoi G/B.
        {
            Markov::Channel noFrames(config, 1, 0);
            Markov::Channel manyFrames(config, 1, 0);

            for (std::uint64_t i = 0; i < 10000; ++i)
            {
                const auto t = i * 100;

                noFrames.tick(t);

                manyFrames.keepFrame(t);
                manyFrames.keepFrame(t + 20);
                manyFrames.keepFrame(t + 80);

                noFrames.tick(t + 80);

                check(
                    noFrames.isBad() == manyFrames.isBad(),
                    "Frame lam thay doi chuoi G/B");
            }

            check(
                noFrames.stats().badSlots ==
                        manyFrames.stats().badSlots &&
                    noFrames.stats().completeBadMs ==
                        manyFrames.stats().completeBadMs,
                "Frame lam thay doi thong ke trang thai");
        }

        std::cout << "M06 PASS: separate RNGs\n";

        // M07: UAV doc lap, khong phu thuoc thu tu tao.
        {
            Markov::Emulator a(config);
            Markov::Emulator b(config);

            a.addUav(1, 0);

            b.addUav(2, 0);
            b.addUav(1, 0);

            for (std::uint64_t i = 0; i < 10000; ++i)
            {
                const auto t = i * 100;

                b.keepFrame(2, t);

                check(
                    a.keepFrame(1, t) ==
                        b.keepFrame(1, t),
                    "UAV 2 anh huong UAV 1");
            }

            check(
                sameStats(
                    a.channel(1).stats(),
                    b.channel(1).stats()),
                "Counters UAV 1 bi anh huong");
        }

        std::cout << "M07 PASS: per-UAV independence\n";

        // M08: Doi soat frame, loss=0 va loss=1.
        {
            auto zeroLoss = config;
            zeroLoss.lossgood = 0.0;
            zeroLoss.lossbad = 0.0;

            auto fullLoss = config;
            fullLoss.lossgood = 1.0;
            fullLoss.lossbad = 1.0;

            Markov::Channel keepAll(zeroLoss, 1, 0);
            Markov::Channel dropAll(fullLoss, 1, 0);

            for (std::uint64_t i = 0; i < 10000; ++i)
            {
                check(keepAll.keepFrame(i * 100),
                      "loss=0 van drop");

                check(!dropAll.keepFrame(i * 100),
                      "loss=1 van giu");
            }

            for (const auto *channel : {&keepAll, &dropAll})
            {
                const auto &s = channel->stats();

                check(s.seen == s.kept + s.dropped,
                      "seen != kept + dropped");
            }
        }

        std::cout << "M08 PASS: accounting and loss bounds\n";

        // M09: 180000 khe, moi khe mot frame o giua khe.
        for (std::uint32_t seed = 1; seed <= 3; ++seed)
        {
            auto c = config;
            c.seed = seed;

            Markov::Channel channel(c, 1, 0);
            const std::uint64_t count = 180000;

            for (std::uint64_t i = 0; i < count; ++i)
            {
                channel.keepFrame(
                    i * c.slot_ms + c.slot_ms / 2);
            }

            channel.tick(count * c.slot_ms);

            const auto &s = channel.stats();

            check(s.slots == count,
                  "Sai so khe");

            check(s.seen == s.kept + s.dropped,
                  "Sai doi soat frame");

            check(
                std::abs(s.badFraction() - c.urb) <= 0.03,
                "Ty le B ngoai dung sai");

            check(
                std::abs(s.meanBadSeconds() - c.dur_s) <= 0.2,
                "Do dai B ngoai dung sai");

            // Mot frame giua moi khe, G giu/B bo:
            // so frame drop phai bang so khe B.
            check(s.dropped == s.badSlots,
                  "Drop khong khop khe B");

            std::cout
                << "M09 seed=" << seed
                << " slots=" << s.slots
                << " bad_fraction=" << s.badFraction()
                << " mean_bad_s=" << s.meanBadSeconds()
                << " seen=" << s.seen
                << " kept=" << s.kept
                << " dropped=" << s.dropped
                << " initial_partial="
                << s.initialPartialBadRuns
                << " final_open="
                << channel.hasOpenBadRun()
                << '\n';
        }

        std::cout << "M09 PASS: long-run statistics\n";
        std::cout << "ALL TESTS PASS\n";
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}