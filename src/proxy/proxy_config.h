#pragma once

#include "coordinate_transform.h"
#include "coverage_gate.h"
#include "coverage_model.h"
#include "link_monitor.h"
#include "rssi_model.h"
#include <fstream>
#include <filesystem>
#include <iterator>
#include <limits>
#include <locale>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace proxy_config {

// Read the JSON types used by proxy.json without a runtime dependency.
struct Json {
    using Object = std::map<std::string, Json>;
    using Array = std::vector<Json>;
    std::variant<std::monostate, bool, double, std::string, Object, Array> value;

    const Json& get(const std::string& key) const {
        const auto& object = std::get<Object>(value);
        const auto found = object.find(key);
        static const Json missing{};
        return found == object.end() ? missing : found->second;
    }
    bool empty() const { return std::holds_alternative<std::monostate>(value); }
};

class JsonReader {
public:
    explicit JsonReader(std::string input) : input_(std::move(input)) {}

    Json read() {
        Json result = value(0);
        space();
        if (offset_ != input_.size()) fail();
        return result;
    }

private:
    [[noreturn]] void fail() const {
        throw std::runtime_error("Invalid JSON at byte " + std::to_string(offset_));
    }

    void space() {
        offset_ = input_.find_first_not_of(" \t\r\n", offset_);
        if (offset_ == std::string::npos) offset_ = input_.size();
    }

    bool take(char token) {
        space();
        if (offset_ == input_.size() || input_[offset_] != token) return false;
        ++offset_;
        return true;
    }

    void expect(char token) {
        if (!take(token)) fail();
    }

    std::string string() {
        expect('"');
        std::string result;
        while (offset_ < input_.size()) {
            const auto ch = static_cast<unsigned char>(input_[offset_++]);
            if (ch == '"') return result;
            if (ch < 32) fail();
            if (ch != '\\') {
                result += static_cast<char>(ch);
                continue;
            }
            if (offset_ == input_.size()) fail();
            const char escaped = input_[offset_++];
            const std::string escapes = "\"\\/bfnrt";
            const auto index = escapes.find(escaped);
            if (index != std::string::npos) {
                result += "\"\\/\b\f\n\r\t"[index];
            } else if (escaped == 'u') {
                const auto hex = input_.substr(offset_, 4);
                if (hex.size() != 4 || hex.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos)
                    fail();
                const auto code = std::stoul(hex, nullptr, 16);
                offset_ += 4;
                if (code < 0x80) result += static_cast<char>(code);
                else if (code < 0x800) {
                    result += static_cast<char>(0xc0 | (code >> 6));
                    result += static_cast<char>(0x80 | (code & 63));
                } else {
                    result += static_cast<char>(0xe0 | (code >> 12));
                    result += static_cast<char>(0x80 | ((code >> 6) & 63));
                    result += static_cast<char>(0x80 | (code & 63));
                }
            } else fail();
        }
        fail();
    }

    Json value(unsigned depth) {
        space();
        if (depth >= 32 || offset_ == input_.size()) fail();
        if (take('{')) {
            Json::Object object;
            if (take('}')) return {std::move(object)};
            do {
                auto key = string();
                expect(':');
                if (!object.emplace(std::move(key), value(depth + 1)).second) fail();
            } while (take(','));
            expect('}');
            return {std::move(object)};
        }
        if (take('[')) {
            Json::Array array;
            if (take(']')) return {std::move(array)};
            do {
                array.push_back(value(depth + 1));
            } while (take(','));
            expect(']');
            return {std::move(array)};
        }
        if (input_[offset_] == '"') return {string()};
        if (input_.compare(offset_, 4, "null") == 0) { offset_ += 4; return {}; }
        if (input_.compare(offset_, 4, "true") == 0) { offset_ += 4; return {true}; }
        if (input_.compare(offset_, 5, "false") == 0) { offset_ += 5; return {false}; }

        static const std::regex number(R"(-?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][+-]?[0-9]+)?)");
        std::smatch match;
        if (!std::regex_search(input_.cbegin() + offset_, input_.cend(), match, number,
                               std::regex_constants::match_continuous)) fail();
        offset_ += match.length();
        std::istringstream stream(match.str());
        stream.imbue(std::locale::classic());
        double result;
        if (!(stream >> result)) result = std::numeric_limits<double>::quiet_NaN();
        return {result};
    }

    std::string input_;
    std::size_t offset_ = 0;
};

inline int integer(const Json& object, const char* key, int fallback, int minimum, int maximum) {
    const auto& field = object.get(key);
    if (field.empty()) return fallback;
    const auto* value = std::get_if<double>(&field.value);
    if (!value || !std::isfinite(*value) || *value < minimum || *value > maximum ||
        std::floor(*value) != *value)
        throw std::runtime_error(std::string("Invalid config integer: ") + key);
    return static_cast<int>(*value);
}

template <typename T>
T setting(const Json& object, const char* key, T fallback) {
    const auto& field = object.get(key);
    return field.empty() ? fallback : std::get<T>(field.value);
}

struct Settings {
    struct GatewayEndpoint {
        std::string id = "A";
        std::string ip = "127.0.0.1";
        int port = 14550;
    };

    std::string bindIp = "0.0.0.0", gatewayIp = "127.0.0.1";
    std::string gatewayId = "A";
    int listenPort = 14600, gatewayPort = 14550;
    int sysid = 1, staleMs = 1000;
    CoordinateTransform transform;
    struct Zone {
        double x, y, radius;

        double distance(const WorldPosition& position) const {
            return std::hypot(position.x - x, position.y - y);
        }
    };
    std::optional<Zone> zone;
    std::vector<GatewayEndpoint> gateways;
    std::vector<GatewayZone> zones;
    CoverageGate::Policy coveragePolicy{};
    RssiModel::Config rssiConfig{};
    LinkMonitor::Config probeConfig{};
    std::string handoverMode = "manual";

    void load(const std::string& path, const std::string& selectedGateway) {
        gatewayId = selectedGateway;
        gateways.clear();
        zones.clear();
        zone.reset();
        const Json config = readFile(path);
        const auto& px4 = config.get("px4");
        if (!px4.empty()) {
            bindIp = setting(px4, "listen_address", bindIp);
            listenPort = integer(px4, "listen_port", listenPort, 1, 65535);
            sysid = integer(px4, "expected_sysid", sysid, 1, 255);
        }
        const auto& gwList = config.get("gateways");
        bool found = false;
        if (!gwList.empty()) {
            for (const auto& gateway : std::get<Json::Array>(gwList.value)) {
                GatewayEndpoint ep;
                ep.id = setting(gateway, "id", std::string{});
                ep.ip = setting(gateway, "address", std::string("127.0.0.1"));
                ep.port = integer(gateway, "port", 14550, 1, 65535);
                if (!ep.id.empty()) gateways.push_back(ep);
                if (ep.id != gatewayId) continue;
                if (found) throw std::runtime_error("Duplicate gateway ID: " + gatewayId);
                found = true;
                gatewayIp = ep.ip;
                gatewayPort = ep.port;
            }
        }
        const auto& position = config.get("position");
        if (!position.empty()) {
            staleMs = integer(position, "stale_timeout_ms", staleMs, 1, 600000);
            readTransform(position.get("transform"));
        }
        const auto& coverage = config.get("coverage");
        if (!coverage.empty()) {
            coveragePolicy.enabled = setting(coverage, "enabled", false);
            coveragePolicy.blockBothDirections = setting(coverage, "block_both_directions", true);
            coveragePolicy.applyToProbes = setting(coverage, "apply_to_probes", true);
        }
        if (coveragePolicy.enabled) {
            if (!found) throw std::runtime_error("Missing configured endpoint for gateway " + gatewayId);
            const auto zonesPath = std::filesystem::path(path).parent_path() /
                setting(coverage, "zones_file", std::string("zones.json"));
            readZones(zonesPath);
        } else if (!found && (!gwList.empty() || gatewayId != "A")) {
            throw std::runtime_error("Unknown gateway ID: " + gatewayId);
        }

        const auto& rssi = config.get("rssi");
        if (!rssi.empty()) {
            rssiConfig.enabled = setting(rssi, "enabled", false);
            rssiConfig.referenceDistanceM = setting(rssi, "reference_distance_m", 1.0);
            rssiConfig.referenceRssiDbm = setting(rssi, "reference_rssi_dbm", -40.0);
            rssiConfig.pathLossExponent = setting(rssi, "path_loss_exponent", 2.0);
            rssiConfig.shadowingStddevDb = setting(rssi, "shadowing_stddev_db", 0.0);
            rssiConfig.randomSeed = static_cast<unsigned int>(integer(rssi, "random_seed", 1, 0, 2000000000));
        }

        const auto& probe = config.get("probe");
        if (!probe.empty()) {
            probeConfig.enabled = setting(probe, "enabled", false);
            probeConfig.intervalMs = integer(probe, "interval_ms", 100, 10, 60000);
            probeConfig.timeoutMs = integer(probe, "timeout_ms", 500, 10, 60000);
            probeConfig.statisticsWindowMs = integer(probe, "statistics_window_ms", 2000, 100, 600000);
        }

        const auto& handover = config.get("handover");
        if (!handover.empty()) {
            handoverMode = setting(handover, "mode", std::string("manual"));
        }
    }

private:
    static Json readFile(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) throw std::runtime_error("Cannot open config: " + path.string());
        std::string source((std::istreambuf_iterator<char>(file)), {});
        if (source.compare(0, 3, "\xef\xbb\xbf") == 0) source.erase(0, 3);
        return JsonReader(std::move(source)).read();
    }

    void readZones(const std::filesystem::path& path) {
        const Json zonesJson = readFile(path);
        for (const auto& entry : std::get<Json::Array>(zonesJson.get("gateways").value)) {
            const auto id = setting(entry, "id", std::string{});
            const auto& antenna = entry.get("antenna_position_m");
            const auto nan = std::numeric_limits<double>::quiet_NaN();
            const double x = setting(antenna, "x", nan);
            const double y = setting(antenna, "y", nan);
            const double z = setting(antenna, "z", 0.0);
            const double radius = setting(entry, "coverage_radius_m", nan);
            if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ||
                !std::isfinite(radius) || radius <= 0)
                throw std::runtime_error("Invalid coverage geometry for gateway " + id);
            zones.push_back(GatewayZone{id, x, y, z, radius});
            if (id != gatewayId) continue;
            if (zone) throw std::runtime_error("Duplicate coverage zone for gateway " + gatewayId);
            zone = Zone{x, y, radius};
        }
        if (!zone) throw std::runtime_error("Missing coverage zone for gateway " + gatewayId);
    }

    void readTransform(const Json& config) {
        if (!std::holds_alternative<Json::Object>(config.value)) return;
        const auto* type = std::get_if<std::string>(&config.get("type").value);
        const auto* origin = std::get_if<Json::Array>(&config.get("world_origin_of_ned_m").value);
        const auto* yaw = std::get_if<double>(&config.get("world_yaw_of_ned_rad").value);
        const auto& offsetField = config.get("altitude_offset_m");
        const double zero = 0;
        const auto* offset = offsetField.empty() ? &zero : std::get_if<double>(&offsetField.value);
        if (!type || *type != "configured_ned_to_world" || !origin || origin->size() != 3 ||
            !yaw || !offset || !std::isfinite(*yaw) || !std::isfinite(*offset)) return;
        double xyz[3];
        for (std::size_t i = 0; i < origin->size(); ++i) {
            const auto* axis = std::get_if<double>(&(*origin)[i].value);
            if (!axis || !std::isfinite(*axis)) return;
            xyz[i] = *axis;
        }
        if (!std::isfinite(xyz[2] + *offset)) return;
        transform = CoordinateTransform({xyz[0], xyz[1], xyz[2]}, *yaw, *offset);
    }
};

} // namespace proxy_config
