#pragma once

#include "network.h"
#include "mavlink_codec.h"
#include "time_utils.h"
#include "config.h"

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ==============================================================================
// Cau truc du lieu luu tru toan bo trang thai telemetry chi tiet cua 1 UAV
// ==============================================================================
struct UavTelemetry
{
    // 1. HEARTBEAT (msgid 0)
    uint8_t type = 0;
    uint8_t autopilot = 0;
    uint8_t baseMode = 0;
    uint32_t customMode = 0;
    uint8_t systemStatus = 0;
    bool armed = false;
    std::string flightMode = "DISARMED - MANUAL";

    // 2. SYS_STATUS (msgid 1)
    float voltageBattery = 0.0f;  // Volt (mV / 1000)
    float currentBattery = 0.0f;  // Ampere (cA / 100)
    int8_t batteryRemaining = -1; // 0 - 100%

    // 3. GLOBAL_POSITION_INT (msgid 33)
    double latitude = 0.0;        // do (-90..90)
    double longitude = 0.0;       // do (-180..180)
    float altitudeMSL = 0.0f;     // m
    float relativeAlt = 0.0f;     // m (do cao so voi mat dat cat canh)
    float vx = 0.0f;              // m/s
    float vy = 0.0f;              // m/s
    float vz = 0.0f;              // m/s
    float heading = 0.0f;         // do (0..360)

    // 4. ATTITUDE (msgid 30)
    float roll = 0.0f;            // do (-180..180)
    float pitch = 0.0f;           // do (-90..90)
    float yaw = 0.0f;             // do (-180..180)
    float rollSpeed = 0.0f;       // rad/s
    float pitchSpeed = 0.0f;      // rad/s
    float yawSpeed = 0.0f;        // rad/s

    // 5. GPS_RAW_INT (msgid 24)
    uint8_t gpsFixType = 0;       // 0: No fix, 2: 2D, 3: 3D, 4: DGPS, 5: RTK
    uint8_t satellitesVisible = 0;// So luong ve tinh

    // 6. VFR_HUD (msgid 74)
    float airspeed = 0.0f;        // m/s
    float groundspeed = 0.0f;     // m/s
    float climbRate = 0.0f;       // m/s
    float throttle = 0.0f;        // % (0..100)

    // Thoi diem va bo dem cap nhat tung message
    TimePoint lastUpdate{};
    unsigned long long heartbeatCount = 0;
    unsigned long long sysStatusCount = 0;
    unsigned long long positionCount = 0;
    unsigned long long attitudeCount = 0;
    unsigned long long gpsCount = 0;
    unsigned long long vfrHudCount = 0;

    // Giai ma che do bay PX4 tu base_mode va custom_mode
    static std::string decodePx4FlightMode(uint8_t baseMode, uint32_t customMode)
    {
        bool isArmed = (baseMode & MAV_MODE_FLAG_SAFETY_ARMED) != 0;
        std::string modeStr = isArmed ? "ARMED" : "DISARMED";

        uint8_t mainMode = static_cast<uint8_t>((customMode >> 16) & 0xFF);
        uint8_t subMode  = static_cast<uint8_t>((customMode >> 24) & 0xFF);

        std::string subStr = "UNKNOWN";
        switch (mainMode)
        {
            case 1: subStr = "MANUAL"; break;
            case 2: subStr = "ALTCTL"; break;
            case 3: subStr = "POSCTL"; break;
            case 4: // AUTO
                switch (subMode)
                {
                    case 1: subStr = "AUTO_READY"; break;
                    case 2: subStr = "AUTO_TAKEOFF"; break;
                    case 3: subStr = "AUTO_LOITER"; break;
                    case 4: subStr = "AUTO_MISSION"; break;
                    case 5: subStr = "AUTO_RTL"; break;
                    case 6: subStr = "AUTO_LAND"; break;
                    case 7: subStr = "AUTO_RTGS"; break;
                    case 8: subStr = "AUTO_FOLLOW"; break;
                    default: subStr = "AUTO"; break;
                }
                break;
            case 5: subStr = "ACRO"; break;
            case 6: subStr = "OFFBOARD"; break;
            case 7: subStr = "STABILIZED"; break;
            case 8: subStr = "RATTITUDE"; break;
            default:
                if (baseMode & MAV_MODE_FLAG_MANUAL_INPUT_ENABLED) subStr = "MANUAL";
                else if (baseMode & MAV_MODE_FLAG_GUIDED_ENABLED) subStr = "GUIDED";
                else if (baseMode & MAV_MODE_FLAG_STABILIZE_ENABLED) subStr = "STABILIZE";
                break;
        }

        return modeStr + " | " + subStr;
    }

    // Format mo ta GPS Fix
    std::string gpsFixString() const
    {
        switch (gpsFixType)
        {
            case 0:
            case 1: return "No Fix";
            case 2: return "2D Fix";
            case 3: return "3D Fix";
            case 4: return "DGPS Fix";
            case 5: return "RTK Float";
            case 6: return "RTK Fixed";
            default: return "Fix " + std::to_string(gpsFixType);
        }
    }

    // Xuat toan bo thong tin chi tiet thanh van ban ro rang cho Client xem (GET_DETAIL)
    std::string toDetailString(int sysid, TimePoint now) const
    {
        double age = elapsedSeconds(now, lastUpdate);
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "========================================================\n"
           << "          THONG TIN CHI TIET UAV [SYSID: " << sysid << "]\n"
           << "========================================================\n"
           << "  Trang thai:        " << flightMode << "\n"
           << "  Do tuoi du lieu:   " << std::setprecision(1) << age << "s truoc\n"
           << "--------------------------------------------------------\n"
           << "  PIN & NGUON:\n"
           << "    Dien ap:         " << std::setprecision(2) << voltageBattery << " V\n"
           << "    Dong dien:       " << std::setprecision(2) << currentBattery << " A\n"
           << "    Dung luong con:  " << static_cast<int>(batteryRemaining) << " %\n"
           << "--------------------------------------------------------\n"
           << "  VI TRI & DO CAO (GPS):\n"
           << "    Vi do (Lat):     " << std::setprecision(6) << latitude << " deg\n"
           << "    Kinh do (Lon):   " << std::setprecision(6) << longitude << " deg\n"
           << "    Do cao tuong doi:" << std::setprecision(1) << relativeAlt << " m\n"
           << "    Do cao MSL:      " << std::setprecision(1) << altitudeMSL << " m\n"
           << "    Heading:         " << std::setprecision(1) << heading << " deg\n"
           << "    GPS Fix:         " << gpsFixString() << " (" << static_cast<int>(satellitesVisible) << " ve tinh)\n"
           << "--------------------------------------------------------\n"
           << "  GOC NGHIENG (ATTITUDE):\n"
           << "    Roll:            " << std::setprecision(1) << roll << " deg\n"
           << "    Pitch:           " << std::setprecision(1) << pitch << " deg\n"
           << "    Yaw:             " << std::setprecision(1) << yaw << " deg\n"
           << "--------------------------------------------------------\n"
           << "  VAN TOC (HUD):\n"
           << "    Toc do mat dat:  " << std::setprecision(2) << groundspeed << " m/s\n"
           << "    Toc do gio:      " << std::setprecision(2) << airspeed << " m/s\n"
           << "    Toc do leo cao:  " << std::setprecision(2) << climbRate << " m/s\n"
           << "    Tay ga Throttle: " << std::setprecision(0) << throttle << " %\n"
           << "========================================================\n";
        return ss.str();
    }

    // Xuat dinh dang JSON de Client de dang parse bang may
    static std::string jsonNumber(double value, int precision)
    {
        if (!std::isfinite(value)) return "null";
        std::ostringstream number;
        number << std::fixed << std::setprecision(precision) << value;
        return number.str();
    }

    std::string toJsonString(int sysid, TimePoint now) const
    {
        double age = elapsedSeconds(now, lastUpdate);
        std::ostringstream ss;
        ss << std::fixed;
        ss << "{"
           << "\"sysid\":" << sysid << ","
           << "\"age_s\":" << jsonNumber(age, 2) << ","
           << "\"armed\":" << (armed ? "true" : "false") << ","
           << "\"flight_mode\":\"" << flightMode << "\","
           << "\"battery\":{"
           << "\"voltage_v\":" << jsonNumber(voltageBattery, 2) << ","
           << "\"current_a\":" << jsonNumber(currentBattery, 2) << ","
           << "\"remaining_pct\":" << static_cast<int>(batteryRemaining)
           << "},"
           << "\"position\":{"
           << "\"lat\":" << jsonNumber(latitude, 7) << ","
           << "\"lon\":" << jsonNumber(longitude, 7) << ","
           << "\"alt_rel_m\":" << jsonNumber(relativeAlt, 2) << ","
           << "\"alt_msl_m\":" << jsonNumber(altitudeMSL, 2) << ","
           << "\"heading_deg\":" << jsonNumber(heading, 1)
           << "},"
           << "\"attitude\":{"
           << "\"roll_deg\":" << jsonNumber(roll, 2) << ","
           << "\"pitch_deg\":" << jsonNumber(pitch, 2) << ","
           << "\"yaw_deg\":" << jsonNumber(yaw, 2)
           << "},"
           << "\"gps\":{"
           << "\"fix_type\":" << static_cast<int>(gpsFixType) << ","
           << "\"fix_str\":\"" << gpsFixString() << "\","
           << "\"satellites\":" << static_cast<int>(satellitesVisible)
           << "},"
           << "\"hud\":{"
           << "\"groundspeed_ms\":" << jsonNumber(groundspeed, 2) << ","
           << "\"airspeed_ms\":" << jsonNumber(airspeed, 2) << ","
           << "\"climb_ms\":" << jsonNumber(climbRate, 2) << ","
           << "\"throttle_pct\":" << jsonNumber(throttle, 0)
           << "}"
           << "}";
        return ss.str();
    }
};

// ==============================================================================
// Quan ly 1 UAV: Dia chi mang, trang thai online va telemetry
// ==============================================================================
struct Uav
{
    sockaddr_in address{};

    bool online = false;
    // Thoi diem nhan frame MAVLink hop le gan nhat tu UAV (heartbeat hoac telemetry).
    TimePoint lastSeen{};

    unsigned int generation = 0;
    unsigned long long receivedFrames = 0;

    UavTelemetry telemetry;
};

inline bool isOnline(const Uav &uav, TimePoint now)
{
    return uav.online &&
           elapsedSeconds(now, uav.lastSeen) <
               Config::HEARTBEAT_TIMEOUT_SECONDS;
}

// ==============================================================================
// Bo quan ly danh sach UAV va giai ma thong tin chi tiet
// ==============================================================================
class UavManager
{
public:
    // Theo doi heartbeat va route dia chi
    bool observeHeartbeat(
        const mavlink_message_t &message,
        const sockaddr_in &sender,
        TimePoint now)
    {
        if (message.msgid != MAVLINK_MSG_ID_HEARTBEAT || message.sysid == 0)
        {
            return false;
        }

        mavlink_heartbeat_t heartbeat{};
        mavlink_msg_heartbeat_decode(&message, &heartbeat);

        if (heartbeat.autopilot == MAV_AUTOPILOT_INVALID ||
            heartbeat.type == MAV_TYPE_GCS)
        {
            return false;
        }

        Uav &uav = uavs_[message.sysid];
        bool alive = isOnline(uav, now);

        if (alive && !Network::sameAddress(uav.address, sender))
        {
            std::cout << "CONFLICT: SYSID=" << static_cast<int>(message.sysid)
                      << " tu endpoint khac, bo qua\n";
            return false;
        }

        if (!alive)
        {
            ++uav.generation;
            uav.address = sender;

            std::cout << "UAV " << static_cast<int>(message.sysid)
                      << " ONLINE tai " << inet_ntoa(sender.sin_addr)
                      << ":" << ntohs(sender.sin_port) << '\n';
        }

        uav.online = true;
        uav.lastSeen = now;

        // Giai ma heartbeat vao telemetry
        decodeHeartbeat(uav.telemetry, heartbeat, now);

        return !alive;
    }

    // Giai ma toan bo telemetry den tu UAV hop le
    void decodeTelemetry(const mavlink_message_t &message, TimePoint now)
    {
        auto found = uavs_.find(message.sysid);
        if (found == uavs_.end())
        {
            return;
        }

        UavTelemetry &tel = found->second.telemetry;
        tel.lastUpdate = now;

        switch (message.msgid)
        {
            case MAVLINK_MSG_ID_HEARTBEAT:
            {
                mavlink_heartbeat_t hb{};
                mavlink_msg_heartbeat_decode(&message, &hb);
                decodeHeartbeat(tel, hb, now);
                break;
            }
            case MAVLINK_MSG_ID_SYS_STATUS:
            {
                mavlink_sys_status_t sysStatus{};
                mavlink_msg_sys_status_decode(&message, &sysStatus);
                tel.voltageBattery = sysStatus.voltage_battery / 1000.0f; // mV -> V
                tel.currentBattery = (sysStatus.current_battery >= 0) ? (sysStatus.current_battery / 100.0f) : 0.0f; // cA -> A
                tel.batteryRemaining = sysStatus.battery_remaining;
                tel.sysStatusCount++;
                break;
            }
            case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
            {
                mavlink_global_position_int_t pos{};
                mavlink_msg_global_position_int_decode(&message, &pos);
                tel.latitude = pos.lat / 1e7;
                tel.longitude = pos.lon / 1e7;
                tel.altitudeMSL = pos.alt / 1000.0f;
                tel.relativeAlt = pos.relative_alt / 1000.0f;
                tel.vx = pos.vx / 100.0f;
                tel.vy = pos.vy / 100.0f;
                tel.vz = pos.vz / 100.0f;
                tel.heading = pos.hdg / 100.0f;
                tel.positionCount++;
                break;
            }
            case MAVLINK_MSG_ID_ATTITUDE:
            {
                mavlink_attitude_t att{};
                mavlink_msg_attitude_decode(&message, &att);
                tel.roll = static_cast<float>(att.roll * 180.0 / M_PI);
                tel.pitch = static_cast<float>(att.pitch * 180.0 / M_PI);
                tel.yaw = static_cast<float>(att.yaw * 180.0 / M_PI);
                tel.rollSpeed = att.rollspeed;
                tel.pitchSpeed = att.pitchspeed;
                tel.yawSpeed = att.yawspeed;
                tel.attitudeCount++;
                break;
            }
            case MAVLINK_MSG_ID_GPS_RAW_INT:
            {
                mavlink_gps_raw_int_t gps{};
                mavlink_msg_gps_raw_int_decode(&message, &gps);
                tel.gpsFixType = gps.fix_type;
                tel.satellitesVisible = gps.satellites_visible;
                tel.gpsCount++;
                break;
            }
            case MAVLINK_MSG_ID_VFR_HUD:
            {
                mavlink_vfr_hud_t hud{};
                mavlink_msg_vfr_hud_decode(&message, &hud);
                tel.airspeed = hud.airspeed;
                tel.groundspeed = hud.groundspeed;
                tel.climbRate = hud.climb;
                tel.throttle = static_cast<float>(hud.throttle);
                tel.vfrHudCount++;
                break;
            }
            default:
                break;
        }
    }

    std::vector<int> expire(TimePoint now)
    {
        std::vector<int> expired;
        for (auto &item : uavs_)
        {
            Uav &uav = item.second;
            if (uav.online && !isOnline(uav, now))
            {
                uav.online = false;
                expired.push_back(item.first);
                std::cout << "UAV " << item.first << " OFFLINE (Timeout)\n";
            }
        }
        return expired;
    }

    std::vector<int> destinations(int target, TimePoint now) const
    {
        std::vector<int> result;
        for (const auto &item : uavs_)
        {
            if ((target == 0 || target == item.first) && isOnline(item.second, now))
            {
                result.push_back(item.first);
            }
        }
        return result;
    }

    bool acceptTelemetry(
        const mavlink_message_t &message,
        const sockaddr_in &sender,
        TimePoint now)
    {
        auto found = uavs_.find(message.sysid);
        if (found == uavs_.end() ||
            !Network::sameAddress(found->second.address, sender))
        {
            return false;
        }

        // Bat ky frame MAVLink hop le nao tu endpoint da hoc cung chung minh
        // route con song. Chi dua vao heartbeat 1 Hz de timeout se lam route
        // chap chon va lam mat MANUAL_CONTROL khi heartbeat tre.
        if (!found->second.online)
        {
            ++found->second.generation;
        }
        found->second.online = true;
        found->second.lastSeen = now;

        ++found->second.receivedFrames;
        decodeTelemetry(message, now);
        return true;
    }

    bool routeValid(
        int sysid,
        unsigned int generation,
        const sockaddr_in &address,
        TimePoint now) const
    {
        auto found = uavs_.find(sysid);
        return found != uavs_.end() &&
               isOnline(found->second, now) &&
               found->second.generation == generation &&
               Network::sameAddress(found->second.address, address);
    }

    const std::map<int, Uav> &all() const
    {
        return uavs_;
    }

    const Uav *get(int sysid) const
    {
        auto it = uavs_.find(sysid);
        return (it != uavs_.end()) ? &it->second : nullptr;
    }

private:
    static void decodeHeartbeat(UavTelemetry &tel, const mavlink_heartbeat_t &hb, TimePoint now)
    {
        tel.type = hb.type;
        tel.autopilot = hb.autopilot;
        tel.baseMode = hb.base_mode;
        tel.customMode = hb.custom_mode;
        tel.systemStatus = hb.system_status;
        tel.armed = (hb.base_mode & MAV_MODE_FLAG_SAFETY_ARMED) != 0;
        tel.flightMode = UavTelemetry::decodePx4FlightMode(hb.base_mode, hb.custom_mode);
        tel.lastUpdate = now;
        tel.heartbeatCount++;
    }

    std::map<int, Uav> uavs_;
};
