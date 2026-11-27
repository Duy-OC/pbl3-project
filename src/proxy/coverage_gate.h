#pragma once

#include "coverage_model.h"

class CoverageGate
{
public:
    struct Policy
    {
        bool enabled = true;
        bool blockBothDirections = true;
        bool applyToProbes = true;
    };

    enum class TrafficKind
    {
        Telemetry,
        Command,
        Probe
    };

    CoverageGate() = default;
    explicit CoverageGate(const Policy &policy) : policy_(policy) {}

    void setPolicy(const Policy &policy) { policy_ = policy; }
    const Policy &policy() const { return policy_; }

    bool allow(const CoverageModel::GatewayStatus &status, TrafficKind kind) const
    {
        if (!policy_.enabled)
        {
            return true;
        }
        if (kind == TrafficKind::Probe && !policy_.applyToProbes)
        {
            return true;
        }
        if (kind == TrafficKind::Command && !policy_.blockBothDirections)
        {
            return true;
        }
        return status.inRange;
    }

private:
    Policy policy_{};
};
