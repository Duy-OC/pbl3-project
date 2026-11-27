#pragma once

#include <cmath>
#include <optional>
#include <string>
#include <vector>

class HandoverExecutor
{
public:
    struct Candidate
    {
        std::string gatewayId;
        bool inRange = false;
        double rssiDbm = -120.0;
        bool hasRssi = false;
    };

    struct Decision
    {
        bool switched = false;
        std::string fromGateway;
        std::string toGateway;
        double fromRssiDbm = -120.0;
        double toRssiDbm = -120.0;
    };

    explicit HandoverExecutor(bool enabled = true) : enabled_(enabled) {}

    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool enabled() const { return enabled_; }

    // Rule:
    // - Pick the in-range gateway with the strongest RSSI.
    // - If multiple gateways have equal RSSI (within epsilon), keep the currently serving gateway.
    // - If the current gateway is out of range, switch to the strongest in-range candidate.
    Decision evaluate(const std::string &currentGatewayId,
                      const std::vector<Candidate> &candidates) const
    {
        Decision decision;
        decision.fromGateway = currentGatewayId;
        decision.toGateway = currentGatewayId;
        if (!enabled_ || candidates.empty())
        {
            return decision;
        }

        const Candidate *current = nullptr;
        const Candidate *best = nullptr;
        constexpr double kEqualEpsilonDb = 1e-3;

        for (const auto &c : candidates)
        {
            if (c.gatewayId == currentGatewayId)
            {
                current = &c;
            }
            if (!c.inRange || !c.hasRssi)
            {
                continue;
            }
            if (!best)
            {
                best = &c;
            }
            else if (c.rssiDbm > best->rssiDbm + kEqualEpsilonDb)
            {
                best = &c;
            }
        }

        if (!best)
        {
            return decision;
        }

        if (current && current->inRange && current->hasRssi)
        {
            decision.fromRssiDbm = current->rssiDbm;
            // Keep current gateway when RSSI is equal or current is already strongest.
            if (best->gatewayId == current->gatewayId ||
                best->rssiDbm <= current->rssiDbm + kEqualEpsilonDb)
            {
                decision.toRssiDbm = current->rssiDbm;
                return decision;
            }
        }

        decision.switched = (best->gatewayId != currentGatewayId);
        decision.toGateway = best->gatewayId;
        decision.toRssiDbm = best->rssiDbm;
        return decision;
    }

private:
    bool enabled_ = true;
};
