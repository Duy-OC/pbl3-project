#pragma once

#include <cmath>
#include <optional>

struct WorldPosition {
    double x, y, z;
};

class CoordinateTransform {
public:
    CoordinateTransform() = default;
    CoordinateTransform(WorldPosition origin, double northYawRad, double altitudeOffset)
        : origin_(origin), northX_(std::cos(northYawRad)), northY_(std::sin(northYawRad)),
          altitudeOffset_(altitudeOffset), configured_(true) {}

    bool valid() const { return configured_; }

    // Hook for reboot detection and future estimator-origin reset notifications.
    void invalidate() { configured_ = false; }

    std::optional<WorldPosition> apply(double north, double east, double down) const {
        if (!valid()) return std::nullopt;
        // World is Z-up. East is 90 degrees clockwise from North in X-Y.
        const WorldPosition world{
            origin_.x + northX_ * north + northY_ * east,
            origin_.y + northY_ * north - northX_ * east,
            (origin_.z + altitudeOffset_) - down
        };
        if (!std::isfinite(world.x) || !std::isfinite(world.y) || !std::isfinite(world.z))
            return std::nullopt;
        return world;
    }

private:
    WorldPosition origin_{};
    double northX_ = 0, northY_ = 0, altitudeOffset_ = 0;
    bool configured_ = false;
};
