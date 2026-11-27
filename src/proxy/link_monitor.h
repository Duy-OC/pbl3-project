#pragma once

#include "../common/envelope.h"

#include <chrono>
#include <cstdint>
#include <deque>
#include <map>
#include <string>
#include <vector>

class LinkMonitor
{
public:
    using Clock = std::chrono::steady_clock;

    struct Config
    {
        bool enabled = false;
        int intervalMs = 100;
        int timeoutMs = 500;
        int statisticsWindowMs = 2000;
    };

    struct LinkQuality
    {
        double lastRttMs = 0.0;
        double probeLossRatio = 0.0;
        std::uint32_t consecutiveTimeouts = 0;
        std::uint64_t probesSent = 0;
        std::uint64_t pongsReceived = 0;
        bool hasSample = false;
    };

    LinkMonitor() = default;
    explicit LinkMonitor(const Config &config) : config_(config) {}

    void configure(const Config &config) { config_ = config; }
    bool enabled() const { return config_.enabled; }

    std::vector<EnvelopeCodec::Envelope> tickGateway(const std::string &gatewayId,
                                                     std::uint8_t sysid,
                                                     bool allowProbe,
                                                     Clock::time_point now)
    {
        std::vector<EnvelopeCodec::Envelope> outgoing;
        auto &state = gateways_[gatewayId];

        // Check timeouts on pending probes.
        for (auto it = state.pending.begin(); it != state.pending.end();)
        {
            const auto ageMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count();
            if (ageMs >= config_.timeoutMs)
            {
                state.history.push_back({now, false});
                ++state.quality.consecutiveTimeouts;
                it = state.pending.erase(it);
            }
            else
            {
                ++it;
            }
        }

        pruneWindow(state, now);
        recomputeLoss(state);

        if (!config_.enabled || !allowProbe)
        {
            return outgoing;
        }

        const auto sinceLastMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - state.lastSent).count();
        if (state.lastSent == Clock::time_point{} || sinceLastMs >= config_.intervalMs)
        {
            state.lastSent = now;
            const std::uint32_t seq = ++state.nextSeq;
            const std::uint64_t nowUs = static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count());

            state.pending[seq] = now;
            ++state.quality.probesSent;

            EnvelopeCodec::Envelope ping;
            ping.type = EnvelopeCodec::MessageType::ProbePing;
            ping.sysid = sysid;
            ping.gatewayId = gatewayId;
            ping.sequence = seq;
            ping.payload = EnvelopeCodec::encodeProbe({nowUs, seq});
            outgoing.push_back(std::move(ping));
        }

        return outgoing;
    }

    bool onPong(const EnvelopeCodec::Envelope &pong, Clock::time_point now)
    {
        if (pong.type != EnvelopeCodec::MessageType::ProbePong)
        {
            return false;
        }
        EnvelopeCodec::ProbePayload payload;
        if (!EnvelopeCodec::decodeProbe(pong.payload, payload))
        {
            return false;
        }
        auto it = gateways_.find(pong.gatewayId);
        if (it == gateways_.end())
        {
            return false;
        }
        auto &state = it->second;
        const auto pendingIt = state.pending.find(payload.probeSeq);
        if (pendingIt != state.pending.end())
        {
            state.pending.erase(pendingIt);
        }

        const std::uint64_t nowUs = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count());
        const double rttMs = nowUs >= payload.timestampUs
                                 ? static_cast<double>(nowUs - payload.timestampUs) / 1000.0
                                 : 0.0;

        state.quality.lastRttMs = rttMs;
        state.quality.consecutiveTimeouts = 0;
        ++state.quality.pongsReceived;
        state.quality.hasSample = true;
        state.history.push_back({now, true});

        pruneWindow(state, now);
        recomputeLoss(state);
        return true;
    }

    LinkQuality qualityFor(const std::string &gatewayId) const
    {
        const auto it = gateways_.find(gatewayId);
        return it == gateways_.end() ? LinkQuality{} : it->second.quality;
    }

private:
    struct Sample
    {
        Clock::time_point time;
        bool ok;
    };

    struct GatewayState
    {
        std::uint32_t nextSeq = 0;
        Clock::time_point lastSent{};
        std::map<std::uint32_t, Clock::time_point> pending;
        std::deque<Sample> history;
        LinkQuality quality{};
    };

    void pruneWindow(GatewayState &state, Clock::time_point now) const
    {
        while (!state.history.empty())
        {
            const auto ageMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                   now - state.history.front().time)
                                   .count();
            if (ageMs > config_.statisticsWindowMs)
            {
                state.history.pop_front();
            }
            else
            {
                break;
            }
        }
    }

    static void recomputeLoss(GatewayState &state)
    {
        if (state.history.empty())
        {
            state.quality.probeLossRatio = 0.0;
            return;
        }
        std::size_t lost = 0;
        for (const auto &sample : state.history)
        {
            if (!sample.ok)
            {
                ++lost;
            }
        }
        state.quality.probeLossRatio = static_cast<double>(lost) / static_cast<double>(state.history.size());
    }

    Config config_{};
    std::map<std::string, GatewayState> gateways_;
};
