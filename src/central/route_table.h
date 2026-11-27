#pragma once

#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <string>

class RouteTable
{
public:
    using Clock = std::chrono::steady_clock;

    struct RouteEntry
    {
        std::string gatewayId;
        bool confirmed = false;
        Clock::time_point lastTelemetryAt{};
        Clock::time_point lastSwitchedAt{};
        std::uint32_t epoch = 0;
    };

    explicit RouteTable(std::string initialGateway = "A",
                        int uavTimeoutMs = 3000,
                        bool requireConfirmed = true)
        : initialGateway_(std::move(initialGateway)),
          uavTimeoutMs_(uavTimeoutMs),
          requireConfirmed_(requireConfirmed) {}

    void recordTelemetry(std::uint8_t sysid, const std::string &gatewayId, Clock::time_point now)
    {
        auto it = routes_.find(sysid);
        if (it == routes_.end())
        {
            RouteEntry entry;
            entry.gatewayId = gatewayId;
            entry.confirmed = true;
            entry.lastTelemetryAt = now;
            entry.lastSwitchedAt = now;
            entry.epoch = 1;
            routes_.emplace(sysid, std::move(entry));
            return;
        }
        if (it->second.gatewayId == gatewayId)
        {
            it->second.lastTelemetryAt = now;
        }
    }

    bool shouldAcceptTelemetry(std::uint8_t sysid, const std::string &gatewayId) const
    {
        const auto it = routes_.find(sysid);
        if (it == routes_.end())
        {
            return gatewayId == initialGateway_ || routes_.empty();
        }
        return it->second.gatewayId == gatewayId;
    }

    void confirmRoute(std::uint8_t sysid, const std::string &gatewayId, Clock::time_point now)
    {
        auto &entry = routes_[sysid];
        entry.gatewayId = gatewayId;
        entry.confirmed = true;
        entry.lastSwitchedAt = now;
        if (entry.lastTelemetryAt == Clock::time_point{})
        {
            entry.lastTelemetryAt = now;
        }
        ++entry.epoch;
    }

    std::optional<std::string> resolveForCommand(std::uint8_t sysid, Clock::time_point now) const
    {
        const auto it = routes_.find(sysid);
        if (it == routes_.end())
        {
            return std::nullopt;
        }
        if (requireConfirmed_ && !it->second.confirmed)
        {
            return std::nullopt;
        }
        const auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second.lastTelemetryAt);
        if (age.count() > uavTimeoutMs_)
        {
            return std::nullopt;
        }
        return it->second.gatewayId;
    }

    std::optional<RouteEntry> get(std::uint8_t sysid) const
    {
        const auto it = routes_.find(sysid);
        if (it == routes_.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    const std::map<std::uint8_t, RouteEntry> &entries() const { return routes_; }

private:
    std::string initialGateway_;
    int uavTimeoutMs_ = 3000;
    bool requireConfirmed_ = true;
    std::map<std::uint8_t, RouteEntry> routes_;
};
