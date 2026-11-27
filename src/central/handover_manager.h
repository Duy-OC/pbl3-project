#pragma once

#include "../common/envelope.h"
#include "gateway_manager.h"
#include "route_table.h"

#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

class HandoverManager
{
public:
    using Clock = std::chrono::steady_clock;

    struct Config
    {
        bool automatic = false;
        double rssiHysteresisDb = 5.0;
        double maxCandidateRttMs = 150.0;
        double maxCandidateProbeLossRatio = 0.2;
        int timeToTriggerMs = 1000;
        int minimumHoldMs = 3000;
        std::uint32_t consecutiveProbeTimeouts = 3;
        int metricsStaleTimeoutMs = 1500;
        int ackTimeoutMs = 1000;
        int maxAttempts = 3;
    };

    struct PendingSwitch
    {
        std::uint8_t sysid = 0;
        std::string targetGatewayId;
        std::uint32_t handoverId = 0;
        int attempts = 0;
        Clock::time_point lastSentAt{};
    };

    HandoverManager() = default;
    explicit HandoverManager(const Config &config) : config_(config) {}

    void setAutomatic(bool automatic) { config_.automatic = automatic; }
    bool automatic() const { return config_.automatic; }

    std::optional<PendingSwitch> requestManualSwitch(std::uint8_t sysid,
                                                     const std::string &targetGatewayId,
                                                     Clock::time_point now)
    {
        if (pending_.count(sysid))
        {
            return std::nullopt;
        }
        PendingSwitch sw;
        sw.sysid = sysid;
        sw.targetGatewayId = targetGatewayId;
        sw.handoverId = ++nextHandoverId_;
        sw.attempts = 1;
        sw.lastSentAt = now;
        pending_[sysid] = sw;
        return sw;
    }

    bool onAck(std::uint8_t sysid,
               const std::string &targetGatewayId,
               const EnvelopeCodec::HandoverPayload &ack,
               RouteTable &routes,
               Clock::time_point now)
    {
        const auto it = pending_.find(sysid);
        if (it == pending_.end())
        {
            return false;
        }
        if (it->second.handoverId != ack.handoverId || it->second.targetGatewayId != targetGatewayId)
        {
            return false;
        }
        pending_.erase(it);
        if (ack.status == EnvelopeCodec::HandoverStatus::Accepted)
        {
            routes.confirmRoute(sysid, targetGatewayId, now);
            triggerCandidate_.erase(sysid);
            return true;
        }
        return false;
    }

    std::vector<PendingSwitch> evaluate(const GatewayManager &gateways,
                                        const RouteTable &routes,
                                        Clock::time_point now)
    {
        std::vector<PendingSwitch> toSend;

        // Retry or expire active switch transactions.
        for (auto it = pending_.begin(); it != pending_.end();)
        {
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second.lastSentAt);
            if (elapsed.count() >= config_.ackTimeoutMs)
            {
                if (it->second.attempts >= config_.maxAttempts)
                {
                    it = pending_.erase(it);
                    continue;
                }
                ++it->second.attempts;
                it->second.lastSentAt = now;
                toSend.push_back(it->second);
            }
            ++it;
        }

        if (!config_.automatic)
        {
            return toSend;
        }

        for (const auto &[sysid, route] : routes.entries())
        {
            if (pending_.count(sysid))
            {
                continue;
            }
            const auto currentMetrics = gateways.metricsFor(
                sysid, route.gatewayId, now, config_.metricsStaleTimeoutMs);
            const bool currentHealthy =
                currentMetrics &&
                currentMetrics->report.inRange != 0 &&
                currentMetrics->report.linkUp != 0 &&
                currentMetrics->report.consecutiveTimeouts < config_.consecutiveProbeTimeouts;

            const auto holdElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - route.lastSwitchedAt);
            if (currentHealthy && holdElapsed.count() < config_.minimumHoldMs)
            {
                triggerCandidate_.erase(sysid);
                continue;
            }

            const auto candidates = gateways.freshCandidates(sysid, now, config_.metricsStaleTimeoutMs);
            const GatewayManager::LinkMetrics *best = nullptr;
            for (const auto &candidate : candidates)
            {
                if (candidate.gatewayId == route.gatewayId)
                {
                    continue;
                }
                if (candidate.report.inRange == 0 ||
                    candidate.report.rttMs > config_.maxCandidateRttMs ||
                    candidate.report.probeLossRatio > config_.maxCandidateProbeLossRatio)
                {
                    continue;
                }
                if (currentHealthy)
                {
                    if (candidate.report.rssiDbm < currentMetrics->report.rssiDbm + config_.rssiHysteresisDb)
                    {
                        continue;
                    }
                }
                if (!best || candidate.report.rssiDbm > best->report.rssiDbm)
                {
                    best = &candidate;
                }
            }

            if (!best)
            {
                triggerCandidate_.erase(sysid);
                continue;
            }

            if (!currentHealthy)
            {
                // Immediate failover when current link is lost/unhealthy.
                if (auto sw = requestManualSwitch(sysid, best->gatewayId, now))
                {
                    toSend.push_back(*sw);
                }
                continue;
            }

            auto &trig = triggerCandidate_[sysid];
            if (trig.gatewayId != best->gatewayId)
            {
                trig.gatewayId = best->gatewayId;
                trig.since = now;
            }
            else
            {
                const auto sustained = std::chrono::duration_cast<std::chrono::milliseconds>(now - trig.since);
                if (sustained.count() >= config_.timeToTriggerMs)
                {
                    if (auto sw = requestManualSwitch(sysid, best->gatewayId, now))
                    {
                        toSend.push_back(*sw);
                    }
                }
            }
        }
        return toSend;
    }

private:
    struct TriggerState
    {
        std::string gatewayId;
        Clock::time_point since{};
    };

    Config config_{};
    std::uint32_t nextHandoverId_ = 0;
    std::map<std::uint8_t, PendingSwitch> pending_;
    std::map<std::uint8_t, TriggerState> triggerCandidate_;
};
