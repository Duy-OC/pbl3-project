#include "sequence_tracker.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

static bool record(std::ofstream &out, const std::string &name,
                   unsigned long long expected, unsigned long long actual)
{
    const bool pass = expected == actual;
    out << name << ',' << expected << ',' << actual << ','
        << (pass ? "PASS" : "FAIL") << '\n';
    return pass;
}

int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    std::ofstream out(argv[1]);
    if (!out) return 2;
    out << "scenario,expected,actual,result\n";
    const TimePoint now = Clock::now();
    bool okay = true;
    {
        SequenceTracker tracker;
        tracker.observe(10, now);
        tracker.observe(11, now + std::chrono::milliseconds(20));
        okay &= record(out, "continuous", 0, tracker.gapEstimate());
    }
    {
        SequenceTracker tracker;
        tracker.observe(10, now);
        tracker.observe(12, now + std::chrono::milliseconds(20));
        okay &= record(out, "one_missing", 1, tracker.gapEstimate());
    }
    {
        SequenceTracker tracker;
        tracker.observe(10, now);
        tracker.observe(16, now + std::chrono::milliseconds(20));
        okay &= record(out, "five_missing", 5, tracker.gapEstimate());
    }
    {
        SequenceTracker tracker;
        tracker.observe(255, now);
        tracker.observe(0, now + std::chrono::milliseconds(20));
        okay &= record(out, "wrap_255_0", 0, tracker.gapEstimate());
    }
    {
        SequenceTracker tracker;
        tracker.observe(10, now);
        tracker.observe(12, now + std::chrono::milliseconds(20));
        tracker.observe(11, now + std::chrono::milliseconds(40));
        okay &= record(out, "late_reorder_clears_gap", 0, tracker.gapEstimate());
        okay &= record(out, "late_reorder_counter", 1, tracker.reordered);
    }
    {
        SequenceTracker tracker;
        tracker.observe(10, now);
        tracker.observe(10, now + std::chrono::milliseconds(20));
        okay &= record(out, "duplicate_counter", 1, tracker.duplicates);
    }
    {
        SequenceTracker tracker;
        tracker.observe(10, now);
        tracker.observe(40, now + std::chrono::milliseconds(20));
        tracker.restart();
        tracker.observe(2, now + std::chrono::milliseconds(40));
        okay &= record(out, "restart_resync", 1, tracker.resyncs);
        okay &= record(out, "restart_uncertain", 29, tracker.uncertainGaps);
    }
    {
        SequenceTracker tracker;
        tracker.observe(10, now);
        tracker.observe(100, now + std::chrono::seconds(3));
        okay &= record(out, "silence_resync", 1, tracker.resyncs);
        okay &= record(out, "silence_no_false_gap", 0, tracker.gapEstimate());
    }
    std::cout << "Sequence logic: " << (okay ? "PASS" : "FAIL") << '\n';
    return okay ? 0 : 1;
}
