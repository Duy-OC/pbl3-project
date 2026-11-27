#pragma once

#include "../common/envelope.h"
#include "../common/network.h"

#include <chrono>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

class GatewayManager
{
public:
    using Clock = std::chrono::steady_clock;

    struct GatewayInfo
    {
        std::string id;
        sockaddr_in endpoint{};
        std::uint32_t sessionId = 0;
        Clock::time_point lastSeen{};
        bool registered = false;
    };

    struct LinkMetrics
    {
        std::string gatewayId;
        EnvelopeCodec::LinkReportPayload report{};
        Clock::time_point updatedAt{};
    };

    explicit GatewayManager(std::set<std::string> allowedIds = {"A", "B"}, int timeoutMs = 3000)
        : allowedIds_(std::move(allowedIds)), timeoutMs_(timeoutMs) {}

    bool isAllowed(const std::string &id) const
    {
        return allowedIds_.empty() || allowedIds_.count(id) > 0;
    }

    bool handleRegisterOrHeartbeat(const std::string &gatewayId,
                                   const sockaddr_in &sender,
                                   std::uint32_t sessionId,
                                   Clock::time_point now,
                                   bool &restartedOut)
    {
        restartedOut = false;
        if (!isAllowed(gatewayId))
        {
            return false;
        }
        auto &info = gateways_[gatewayId];
        if (info.registered && info.sessionId != 0 && info.sessionId != sessionId)
        {
            restartedOut = true;
        }
        info.id = gatewayId;
        info.endpoint = sender;
        info.sessionId = sessionId;
        info.lastSeen = now;
        info.registered = true;
        return true;
    }

    void updateLinkReport(std::uint8_t sysid,
                          const std::string &gatewayId,
                          const EnvelopeCodec::LinkReportPayload &report,
                          Clock::time_point now)
    {
        if (!isAllowed(gatewayId))
        {
            return;
        }
        auto &entry = linkReports_[sysid][gatewayId];
        entry.gatewayId = gatewayId;
        entry.report = report;
        entry.updatedAt = now;
    }

    bool isAlive(const std::string &gatewayId, Clock::time_point now) const
    {
        const auto it = gateways_.find(gatewayId);
        if (it == gateways_.end() || !it->second.registered)
        {
            return false;
        }
        const auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second.lastSeen);
        return age.count() <= timeoutMs_;
    }

    std::optional<sockaddr_in> endpointOf(const std::string &gatewayId, Clock::time_point now) const
    {
        if (!isAlive(gatewayId, now))
        {
            return std::nullopt;
        }
        return gateways_.at(gatewayId).endpoint;
    }

    std::vector<LinkMetrics> freshCandidates(std::uint8_t sysid,
                                             Clock::time_point now,
                                             int metricsStaleTimeoutMs) const
    {
        std::vector<LinkMetrics> result;
        const auto uavIt = linkReports_.find(sysid);
        if (uavIt == linkReports_.end())
        {
            return result;
        }
        for (const auto &[gwId, metrics] : uavIt->second)
        {
            if (!isAlive(gwId, now))
            {
                continue;
            }
            const auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - metrics.updatedAt);
            if (age.count() <= metricsStaleTimeoutMs)
            {
                result.push_back(metrics);
            }
        }
        return result;
    }

    std::optional<LinkMetrics> metricsFor(std::uint8_t sysid,
                                          const std::string &gatewayId,
                                          Clock::time_point now,
                                          int metricsStaleTimeoutMs) const
    {
        const auto uavIt = linkReports_.find(sysid);
        if (uavIt == linkReports_.end())
        {
            return std::nullopt;
        }
        const auto gwIt = uavIt->second.find(gatewayId);
        if (gwIt == uavIt->second.end())
        {
            return std::nullopt;
        }
        const auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - gwIt->second.updatedAt);
        if (age.count() > metricsStaleTimeoutMs)
        {
            return std::nullopt;
        }
        return gwIt->second;
    }

    const std::map<std::string, GatewayInfo> &gateways() const { return gateways_; }

private:
    std::set<std::string> allowedIds_;
    int timeoutMs_ = 3000;
    std::map<std::string, GatewayInfo> gateways_;
    std::map<std::uint8_t, std::map<std::string, LinkMetrics>> linkReports_;
};
