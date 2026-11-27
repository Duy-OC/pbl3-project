#pragma once

#include <algorithm>
#include <cmath>
#include <random>

class RssiModel
{
public:
    struct Config
    {
        bool enabled = false;
        double referenceDistanceM = 1.0;
        double referenceRssiDbm = -40.0;
        double pathLossExponent = 2.0;
        double shadowingStddevDb = 0.0;
        unsigned int randomSeed = 1;
    };

    RssiModel() : rng_(config_.randomSeed) {}
    explicit RssiModel(const Config &config)
        : config_(config), rng_(config.randomSeed) {}

    void configure(const Config &config)
    {
        config_ = config;
        rng_.seed(config_.randomSeed);
    }

    bool enabled() const { return config_.enabled; }

    double estimateDbm(double distance3dM)
    {
        const double d0 = std::max(0.01, config_.referenceDistanceM);
        const double d = std::max(d0, distance3dM);
        double rssi = config_.referenceRssiDbm -
                      10.0 * config_.pathLossExponent * std::log10(d / d0);
        if (config_.shadowingStddevDb > 0.0)
        {
            std::normal_distribution<double> noise(0.0, config_.shadowingStddevDb);
            rssi += noise(rng_);
        }
        return rssi;
    }

private:
    Config config_{};
    std::mt19937 rng_;
};
