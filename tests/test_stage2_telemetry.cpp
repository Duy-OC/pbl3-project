// ==============================================================================
// Test Suite Giai đoạn 2: Giải mã trạng thái UAV chi tiết (T2.1 - T2.5)
// ==============================================================================
#include "uav_manager.h"
#include "mavlink_codec.h"
#include "time_utils.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cassert>
#include <cmath>

struct TestResult
{
    std::string id;
    std::string name;
    bool passed = false;
    std::string detail;
};

int main()
{
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::cout << "==============================================================\n"
              << "     KIEM THU GIAI DOAN 2: GIAI MA TRANG THAI UAV CHI TIET    \n"
              << "==============================================================\n";

    UavManager manager;
    TimePoint now = Clock::now();

    sockaddr_in sender{};
    sender.sin_family = AF_INET;
    sender.sin_port = htons(14550);
    sender.sin_addr.s_addr = inet_addr("127.0.0.1");

    std::vector<TestResult> results;

    // --------------------------------------------------------------------------
    // Test 1: Giai ma HEARTBEAT (Armed, Autopilot, Che do bay PX4)
    // --------------------------------------------------------------------------
    {
        TestResult res;
        res.id = "T2.1";
        res.name = "Giai ma HEARTBEAT (PX4 POSCTL, Armed)";

        mavlink_message_t msg;
        // PX4 POSCTL: main_mode = 3 (POSCTL), custom_mode = (3 << 16)
        uint32_t custom_mode = (3 << 16);
        uint8_t base_mode = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED | MAV_MODE_FLAG_SAFETY_ARMED;

        mavlink_msg_heartbeat_pack(
            1, 1, &msg,
            MAV_TYPE_QUADROTOR,
            MAV_AUTOPILOT_PX4,
            base_mode,
            custom_mode,
            MAV_STATE_ACTIVE);

        bool newRoute = manager.observeHeartbeat(msg, sender, now);
        const Uav *uav = manager.get(1);

        if (newRoute && uav && uav->telemetry.armed &&
            uav->telemetry.flightMode == "ARMED | POSCTL")
        {
            res.passed = true;
            res.detail = "Armed: TRUE, Flight mode: " + uav->telemetry.flightMode;
        }
        else
        {
            res.passed = false;
            res.detail = "Khong giai ma dung armed hoac flight mode!";
        }
        results.push_back(res);
    }

    // --------------------------------------------------------------------------
    // Test 2: Giai ma SYS_STATUS (Dien ap, Dong dien, Dung luong pin)
    // --------------------------------------------------------------------------
    {
        TestResult res;
        res.id = "T2.2";
        res.name = "Giai ma SYS_STATUS (Pin & Nguon)";

        mavlink_message_t msg;
        mavlink_msg_sys_status_pack(
            1, 1, &msg,
            0, 0, 0, 500, // sensors, load
            15450,        // 15.45 V (15450 mV)
            230,          // 2.30 A (230 cA)
            82,           // 82%
            0, 0, 0, 0, 0, 0, 0, 0, 0);

        manager.acceptTelemetry(msg, sender, now);
        const Uav *uav = manager.get(1);

        if (uav &&
            std::abs(uav->telemetry.voltageBattery - 15.45f) < 0.01f &&
            std::abs(uav->telemetry.currentBattery - 2.30f) < 0.01f &&
            uav->telemetry.batteryRemaining == 82)
        {
            res.passed = true;
            res.detail = "Voltage: 15.45V, Current: 2.30A, Remaining: 82%";
        }
        else
        {
            res.passed = false;
            res.detail = "Sai lech gia tri pin!";
        }
        results.push_back(res);
    }

    // --------------------------------------------------------------------------
    // Test 3: Giai ma GLOBAL_POSITION_INT (Vi tri, Do cao, Heading)
    // --------------------------------------------------------------------------
    {
        TestResult res;
        res.id = "T2.3";
        res.name = "Giai ma GLOBAL_POSITION_INT (GPS & Do cao)";

        mavlink_message_t msg;
        mavlink_msg_global_position_int_pack(
            1, 1, &msg,
            10000,          // time_boot_ms
            160734560,      // 16.073456 deg (160734560 degE7)
            1081523410,     // 108.152341 deg
            25400,          // 25.4 m MSL (25400 mm)
            12800,          // 12.8 m relative alt (12800 mm)
            250,            // vx = 2.5 m/s (250 cm/s)
            -150,           // vy = -1.5 m/s (-150 cm/s)
            50,             // vz = 0.5 m/s (50 cm/s)
            18520);         // hdg = 185.2 deg (18520 cdeg)

        manager.acceptTelemetry(msg, sender, now);
        const Uav *uav = manager.get(1);

        if (uav &&
            std::abs(uav->telemetry.latitude - 16.073456) < 1e-5 &&
            std::abs(uav->telemetry.longitude - 108.152341) < 1e-5 &&
            std::abs(uav->telemetry.relativeAlt - 12.8f) < 0.05f &&
            std::abs(uav->telemetry.heading - 185.2f) < 0.1f)
        {
            res.passed = true;
            res.detail = "Lat: 16.073456, Lon: 108.152341, RelAlt: 12.8m, Hdg: 185.2 deg";
        }
        else
        {
            res.passed = false;
            res.detail = "Sai lech gia tri toa do / do cao!";
        }
        results.push_back(res);
    }

    // --------------------------------------------------------------------------
    // Test 4: Giai ma ATTITUDE (Roll, Pitch, Yaw)
    // --------------------------------------------------------------------------
    {
        TestResult res;
        res.id = "T2.4";
        res.name = "Giai ma ATTITUDE (Goc nghieng)";

        mavlink_message_t msg;
        // 0.087266 rad = 5.0 deg; -0.052360 rad = -3.0 deg; 1.570796 rad = 90.0 deg
        mavlink_msg_attitude_pack(
            1, 1, &msg,
            10000,
            0.0872665f,
            -0.0523599f,
            1.5707963f,
            0.01f, 0.02f, 0.03f);

        manager.acceptTelemetry(msg, sender, now);
        const Uav *uav = manager.get(1);

        if (uav &&
            std::abs(uav->telemetry.roll - 5.0f) < 0.1f &&
            std::abs(uav->telemetry.pitch - (-3.0f)) < 0.1f &&
            std::abs(uav->telemetry.yaw - 90.0f) < 0.1f)
        {
            res.passed = true;
            res.detail = "Roll: 5.0 deg, Pitch: -3.0 deg, Yaw: 90.0 deg";
        }
        else
        {
            res.passed = false;
            res.detail = "Sai lech goc nghieng roll/pitch/yaw!";
        }
        results.push_back(res);
    }

    // --------------------------------------------------------------------------
    // Test 5: Giai ma GPS_RAW_INT & VFR_HUD (Ve tinh, Toc do gio, Throttle)
    // --------------------------------------------------------------------------
    {
        TestResult res;
        res.id = "T2.5";
        res.name = "Giai ma GPS_RAW_INT & VFR_HUD";

        mavlink_message_t gpsMsg;
        mavlink_msg_gps_raw_int_pack(
            1, 1, &gpsMsg,
            10000, 3, // 3D Fix
            160734560, 1081523410, 25400,
            100, 100, 300, 18500,
            16, // 16 ve tinh
            0, 0, 0, 0, 0, 0);

        mavlink_message_t hudMsg;
        mavlink_msg_vfr_hud_pack(
            1, 1, &hudMsg,
            5.2f, // airspeed
            4.8f, // groundspeed
            185,  // heading
            65,   // throttle 65%
            25.4f,// alt
            0.8f);// climb rate 0.8 m/s

        manager.acceptTelemetry(gpsMsg, sender, now);
        manager.acceptTelemetry(hudMsg, sender, now);
        const Uav *uav = manager.get(1);

        if (uav &&
            uav->telemetry.gpsFixType == 3 &&
            uav->telemetry.satellitesVisible == 16 &&
            std::abs(uav->telemetry.airspeed - 5.2f) < 0.05f &&
            std::abs(uav->telemetry.groundspeed - 4.8f) < 0.05f &&
            std::abs(uav->telemetry.climbRate - 0.8f) < 0.05f &&
            std::abs(uav->telemetry.throttle - 65.0f) < 0.1f)
        {
            res.passed = true;
            res.detail = "GPS 3D Fix (16 ve tinh), Groundspeed: 4.8 m/s, Throttle: 65%";
        }
        else
        {
            res.passed = false;
            res.detail = "Sai lech gia tri GPS_RAW_INT hoac VFR_HUD!";
        }
        results.push_back(res);
    }

    // --------------------------------------------------------------------------
    // Test 6: Kiem tra xuat chuoi chi tiet (toDetailString & toJsonString)
    // --------------------------------------------------------------------------
    {
        TestResult res;
        res.id = "T2.6";
        res.name = "Xuat chuoi GET_DETAIL va JSON";

        const Uav *uav = manager.get(1);
        std::string detail = uav->telemetry.toDetailString(1, now);
        std::string json = uav->telemetry.toJsonString(1, now);

        if (!detail.empty() && !json.empty() &&
            detail.find("15.45 V") != std::string::npos &&
            json.find("\"remaining_pct\":82") != std::string::npos)
        {
            res.passed = true;
            res.detail = "Sinh day du text report va JSON hop le cho Client.";
        }
        else
        {
            res.passed = false;
            res.detail = "Loi dinh dang chuoi xuat detail/json!";
        }
        results.push_back(res);
    }

    // In bang ket qua
    std::cout << "\n===================================================================================\n"
              << "                              BANG KET QUA KIEM THU                                \n"
              << "===================================================================================\n";
    std::cout << std::left << std::setw(8)  << "Ma"
              << std::setw(42) << "Phep thu"
              << std::setw(10) << "Ket qua"
              << "Chi tiet\n";
    std::cout << "-----------------------------------------------------------------------------------\n";

    bool allPassed = true;
    for (const auto &r : results)
    {
        if (!r.passed) allPassed = false;
        std::cout << std::left << std::setw(8)  << r.id
                  << std::setw(42) << r.name
                  << std::setw(10) << (r.passed ? "PASS" : "FAIL")
                  << r.detail << "\n";
    }

    // Xuat ra file CSV
    std::string csvPath = "docs/ketqua/tich_hop_projectltm/stage2_telemetry_results.csv";
    std::ofstream csvFile(csvPath.c_str());
    if (csvFile)
    {
        csvFile << "test_id,test_name,status,detail\n";
        for (const auto &r : results)
        {
            csvFile << r.id << ",\""
                    << r.name << "\","
                    << (r.passed ? "PASS" : "FAIL") << ",\""
                    << r.detail << "\"\n";
        }
        csvFile.close();
        std::cout << "\n[OK] Da xuat file ket qua CSV: " << csvPath << "\n";
    }

    // In mau chuoi chi tiet de kiem tra truc quan
    std::cout << "\n[DEMO GET_DETAIL TEXT CHO CLIENT]:\n"
              << manager.get(1)->telemetry.toDetailString(1, now) << "\n";

    return allPassed ? 0 : 1;
}
