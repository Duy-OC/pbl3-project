#include "../src/emulator.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>

namespace
{
constexpr std::uint64_t kSlots = 180000;
constexpr std::uint64_t kHashStart = 1469598103934665603ull;
constexpr std::uint64_t kHashPrime = 1099511628211ull;

void mix(std::uint64_t &hash, std::uint64_t value)
{
    for (int i = 0; i < 8; ++i)
    {
        hash ^= static_cast<std::uint8_t>(value >> (8 * i));
        hash *= kHashPrime;
    }
}

struct Result
{
    Markov::Stats stats;
    std::uint64_t stateHash = kHashStart;
    std::uint64_t dropHash = kHashStart;
    bool finalBad = false;
    bool openBadRun = false;
};

Result simulate(const Markov::Config &config, int sysid)
{
    Result result;
    Markov::Channel channel(config, sysid, 0,
        [&result](std::uint64_t at, bool bad)
        {
            mix(result.stateHash, at);
            mix(result.stateHash, bad ? 1 : 0);
        });
    for (std::uint64_t i = 0; i < kSlots; ++i)
    {
        const bool kept = channel.keepFrame(i * config.slot_ms);
        mix(result.dropHash, kept ? 1 : 0);
    }
    channel.tick(kSlots * config.slot_ms);
    result.stats = channel.stats();
    result.finalBad = channel.isBad();
    result.openBadRun = channel.hasOpenBadRun();
    return result;
}

Result jumpTick(const Markov::Config &config, int sysid)
{
    Result result;
    Markov::Channel channel(config, sysid, 0,
        [&result](std::uint64_t at, bool bad)
        {
            mix(result.stateHash, at);
            mix(result.stateHash, bad ? 1 : 0);
        });
    channel.tick(kSlots * config.slot_ms);
    result.stats = channel.stats();
    result.finalBad = channel.isBad();
    result.openBadRun = channel.hasOpenBadRun();
    return result;
}

bool sameRun(const Result &a, const Result &b)
{
    return a.stateHash == b.stateHash && a.dropHash == b.dropHash &&
           a.finalBad == b.finalBad && a.openBadRun == b.openBadRun &&
           a.stats.seen == b.stats.seen &&
           a.stats.kept == b.stats.kept &&
           a.stats.dropped == b.stats.dropped &&
           a.stats.slots == b.stats.slots &&
           a.stats.badSlots == b.stats.badSlots &&
           a.stats.completeBadRuns == b.stats.completeBadRuns &&
           a.stats.completeBadMs == b.stats.completeBadMs &&
           a.stats.initialPartialBadRuns == b.stats.initialPartialBadRuns;
}

bool sameStateSchedule(const Result &a, const Result &b)
{
    return a.stateHash == b.stateHash && a.finalBad == b.finalBad &&
           a.stats.slots == b.stats.slots &&
           a.stats.badSlots == b.stats.badSlots &&
           a.stats.completeBadRuns == b.stats.completeBadRuns &&
           a.stats.completeBadMs == b.stats.completeBadMs &&
           a.stats.initialPartialBadRuns == b.stats.initialPartialBadRuns;
}

bool invalidConfigRejected(Markov::Config config)
{
    try
    {
        config.validate();
    }
    catch (const std::invalid_argument &)
    {
        return true;
    }
    return false;
}

void printRun(std::uint32_t seed, int sysid, const Result &result)
{
    const auto &s = result.stats;
    std::cout << "seed=" << seed << " sysid=" << sysid
              << " slots=" << s.slots << " bad_slots=" << s.badSlots
              << " bad_fraction=" << s.badFraction()
              << " complete_bad_runs=" << s.completeBadRuns
              << " mean_bad_s=" << s.meanBadSeconds()
              << " initial_partial_bad_runs=" << s.initialPartialBadRuns
              << " final_open_bad_run=" << result.openBadRun
              << " frames_seen=" << s.seen << " frames_kept=" << s.kept
              << " frames_dropped=" << s.dropped
              << " drop_fraction=" << s.dropFraction()
              << " state_hash=" << result.stateHash
              << " drop_hash=" << result.dropHash << '\n';
}
}

int main()
{
    std::cout << std::fixed << std::setprecision(6);
    Markov::Config config;
    config.urb = 0.5;
    config.dur_s = 2.0;
    config.slot_ms = 100;
    config.lossgood = 0.0;
    config.lossbad = 1.0;
    std::cout << "MARKOV CONFIG: urb=" << config.urb
              << " dur_s=" << config.dur_s
              << " slot_ms=" << config.slot_ms
              << " lossgood=" << config.lossgood
              << " lossbad=" << config.lossbad
              << " slots_per_seed=" << kSlots
              << " pBG=" << config.pBG()
              << " pGB=" << config.pGB() << '\n';

    std::array<Result, 3> runs;
    const std::array<std::uint32_t, 3> seeds{{1, 2, 3}};
    bool pass = true;
    for (std::size_t i = 0; i < seeds.size(); ++i)
    {
        config.seed = seeds[i];
        runs[i] = simulate(config, 1);
        printRun(config.seed, 1, runs[i]);
        const auto &s = runs[i].stats;
        pass = pass && s.slots == kSlots && s.seen == kSlots &&
               s.seen == s.kept + s.dropped &&
               std::abs(s.badFraction() - config.urb) < 0.05 &&
               std::abs(s.dropFraction() - config.urb) < 0.05 &&
               std::abs(s.meanBadSeconds() - config.dur_s) < 0.3 &&
               s.completeBadRuns > 1000;
    }

    config.seed = 1;
    const Result repeat = simulate(config, 1);
    const bool repeatEqual = sameRun(runs[0], repeat);
    const Result otherUav = simulate(config, 2);
    const bool independentUav =
        runs[0].stateHash != otherUav.stateHash &&
        runs[0].dropHash != otherUav.dropHash;
    const Result jumped = jumpTick(config, 1);
    const bool tickEqual = sameStateSchedule(runs[0], jumped);
    const bool distinctSeeds =
        runs[0].stateHash != runs[1].stateHash &&
        runs[0].stateHash != runs[2].stateHash &&
        runs[1].stateHash != runs[2].stateHash;
    printRun(1, 2, otherUav);
    std::cout << "CHECK: repeat_same_seed=" << repeatEqual
              << " independent_sysid=" << independentUav
              << " distinct_seeds=" << distinctSeeds
              << " tick_step_equals_jump=" << tickEqual << '\n';
    pass = pass && repeatEqual && independentUav && distinctSeeds && tickEqual;

    Markov::Config off = config;
    off.urb = 0.0;
    const Result offResult = simulate(off, 1);
    printRun(off.seed, 1, offResult);
    const bool offPass = offResult.stats.slots == kSlots &&
                         offResult.stats.seen == kSlots &&
                         offResult.stats.kept == kSlots &&
                         offResult.stats.dropped == 0 &&
                         offResult.stats.badSlots == 0;
    std::cout << "CHECK: urb_zero_keeps_all=" << offPass << '\n';
    pass = pass && offPass;

    int rejected = 0;
    Markov::Config invalid = config;
    invalid.urb = 1.0;
    rejected += invalidConfigRejected(invalid);
    invalid = config;
    invalid.dur_s = 0.0;
    rejected += invalidConfigRejected(invalid);
    invalid = config;
    invalid.slot_ms = 0;
    rejected += invalidConfigRejected(invalid);
    invalid = config;
    invalid.lossbad = 1.1;
    rejected += invalidConfigRejected(invalid);
    invalid = config;
    invalid.urb = 0.9;
    invalid.dur_s = 0.05;
    rejected += invalidConfigRejected(invalid);
    std::cout << "CHECK: invalid_configs_rejected=" << rejected << "/5\n";
    pass = pass && rejected == 5;

    std::cout << "MARKOV MODEL RESULT: pass=" << pass
              << " seeds=3 slots_per_seed=" << kSlots << '\n';
    return pass ? 0 : 1;
}
