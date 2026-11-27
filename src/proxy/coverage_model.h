#pragma once

#include "coordinate_transform.h"

#include <cmath>
#include <optional>
#include <string>
#include <vector>

struct GatewayZone
{
    std::string id;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double radius = 40.0;

    double horizontalDistance(const WorldPosition &pos) const
    {
        return std::hypot(pos.x - x, pos.y - y);
    }

    double distance3D(const WorldPosition &pos) const
    {
        const double dx = pos.x - x;
        const double dy = pos.y - y;
        const double dz = pos.z - z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
};

class CoverageModel
{
public:
    struct GatewayStatus
    {
        std::string gatewayId;
        double horizontalDistanceM = 0.0;
        double distance3dM = 0.0;
        bool inRange = false;
    };

    static GatewayStatus evaluateOne(const GatewayZone &zone, const std::optional<WorldPosition> &world)
    {
        GatewayStatus status;
        status.gatewayId = zone.id;
        if (!world)
        {
            return status;
        }
        status.horizontalDistanceM = zone.horizontalDistance(*world);
        status.distance3dM = zone.distance3D(*world);
        status.inRange = status.horizontalDistanceM <= zone.radius;
        return status;
    }

    static std::vector<GatewayStatus> evaluateAll(const std::vector<GatewayZone> &zones,
                                                  const std::optional<WorldPosition> &world)
    {
        std::vector<GatewayStatus> result;
        result.reserve(zones.size());
        for (const auto &zone : zones)
        {
            result.push_back(evaluateOne(zone, world));
        }
        return result;
    }
};
