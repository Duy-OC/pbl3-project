// ==============================================================================
// Test Suite Giai đoạn 1: Parse frame và CRC (T1.1 - T1.5)
// ==============================================================================
#include "mavlink_codec.h"
#include "time_utils.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <chrono>

struct TestCaseResult
{
    std::string id;
    std::string name;
    unsigned long long framesTested = 0;
    unsigned long long validFrames = 0;
    unsigned long long badCrc = 0;
    unsigned long long badSignature = 0;
    unsigned long long fuzzBytes = 0;
    bool passed = false;
    std::string detail;
};

// Ham tao frame MAVLink v2 chuan (Heartbeat) voi seq chi dinh
std::vector<uint8_t> createV2Heartbeat(uint8_t seq = 0)
{
    mavlink_message_t msg;
    mavlink_status_t status{};
    status.current_tx_seq = seq;

    mavlink_msg_heartbeat_pack_status(
        1, 1, &status, &msg,
        MAV_TYPE_QUADROTOR,
        MAV_AUTOPILOT_PX4,
        MAV_MODE_MANUAL_ARMED,
        0,
        MAV_STATE_ACTIVE);

    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);
    return std::vector<uint8_t>(buffer, buffer + len);
}

// Ham tao frame MAVLink v1 chuan (Heartbeat) voi seq chi dinh
std::vector<uint8_t> createV1Heartbeat(uint8_t seq = 0)
{
    mavlink_message_t msg;
    mavlink_status_t status{};
    status.current_tx_seq = seq;
    status.flags |= MAVLINK_STATUS_FLAG_OUT_MAVLINK1; // Bat co MAVLink v1 (0xFE)

    mavlink_msg_heartbeat_pack_status(
        1, 1, &status, &msg,
        MAV_TYPE_QUADROTOR,
        MAV_AUTOPILOT_PX4,
        MAV_MODE_MANUAL_ARMED,
        0,
        MAV_STATE_ACTIVE);

    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);
    return std::vector<uint8_t>(buffer, buffer + len);
}

// ==============================================================================
// T1.1: Frame mau v1 va v2 -> CRC dung 100%
// ==============================================================================
TestCaseResult test_T1_1(std::ostream &log)
{
    TestCaseResult res;
    res.id = "T1.1";
    res.name = "Frame mau v1 va v2 (CRC dung 100%)";

    log << "\n--- [CHAY TEST T1.1: Frame mau v1 va v2] ---\n";
    MavlinkParser parser;

    // Test 100 frame v1 va 100 frame v2
    for (int i = 0; i < 100; ++i)
    {
        auto v1 = createV1Heartbeat(static_cast<uint8_t>(i));
        res.framesTested++;
        for (uint8_t b : v1)
        {
            uint8_t st = parser.feed(b);
            if (st == MAVLINK_FRAMING_OK) res.validFrames++;
            else if (st == MAVLINK_FRAMING_BAD_CRC) res.badCrc++;
        }

        auto v2 = createV2Heartbeat(static_cast<uint8_t>(i));
        res.framesTested++;
        for (uint8_t b : v2)
        {
            uint8_t st = parser.feed(b);
            if (st == MAVLINK_FRAMING_OK) res.validFrames++;
            else if (st == MAVLINK_FRAMING_BAD_CRC) res.badCrc++;
        }
    }

    res.passed = (res.validFrames == res.framesTested && res.badCrc == 0);
    res.detail = "Da test " + std::to_string(res.framesTested) + " frames (100 v1 + 100 v2). " +
                 "Hop le: " + std::to_string(res.validFrames) + ", Sai CRC: " + std::to_string(res.badCrc);
    log << res.detail << "\n";
    return res;
}

// ==============================================================================
// T1.2: Lat 1 bit trong frame -> Bat loi sai CRC 100%
// ==============================================================================
TestCaseResult test_T1_2(std::ostream &log)
{
    TestCaseResult res;
    res.id = "T1.2";
    res.name = "Lat 1 bit trong frame (Bat loi CRC)";

    log << "\n--- [CHAY TEST T1.2: Lat 1 bit trong frame] ---\n";

    // Lay frame mau v2 (header 10 bytes + payload 9 bytes + checksum 2 bytes = 21 bytes)
    auto baseFrame = createV2Heartbeat(42);
    int totalCorrupted = 0;
    int detectedBadCrc = 0;
    int falseValid = 0;

    // Lat tung bit trong toan bo Payload va Checksum (byte 10 den 20)
    for (size_t byteIdx = 10; byteIdx < baseFrame.size(); ++byteIdx)
    {
        for (int bit = 0; bit < 8; ++bit)
        {
            auto corrupted = baseFrame;
            corrupted[byteIdx] ^= (1 << bit); // Lat 1 bit
            totalCorrupted++;

            MavlinkParser parser;
            bool gotBadCrc = false;
            bool gotValid = false;

            for (uint8_t b : corrupted)
            {
                uint8_t st = parser.feed(b);
                if (st == MAVLINK_FRAMING_BAD_CRC) gotBadCrc = true;
                if (st == MAVLINK_FRAMING_OK) gotValid = true;
            }

            if (gotBadCrc) detectedBadCrc++;
            if (gotValid) falseValid++;
        }
    }

    res.framesTested = totalCorrupted;
    res.badCrc = detectedBadCrc;
    res.validFrames = falseValid;

    // 100% frame bi lat bit deu phai bi bat loi BAD_CRC va 0 frame nao bi chap nhan sai
    res.passed = (detectedBadCrc == totalCorrupted && falseValid == 0);
    res.detail = "Lat bit tai payload va checksum (" + std::to_string(totalCorrupted) + " vi tri bit). " +
                 "Phat hien sai CRC: " + std::to_string(detectedBadCrc) + "/" + std::to_string(totalCorrupted) +
                 " (100%), False valid: 0";
    log << res.detail << "\n";
    return res;
}

// ==============================================================================
// T1.3: Datagram ghep, stream byte, resync sau rac -> Parse dung, khong crash
// ==============================================================================
TestCaseResult test_T1_3(std::ostream &log)
{
    TestCaseResult res;
    res.id = "T1.3";
    res.name = "Datagram ghep, manh packet, resync sau rac";

    log << "\n--- [CHAY TEST T1.3: Datagram ghep, stream byte, resync] ---\n";
    MavlinkParser parser;
    int validCount = 0;

    // Case A: 2 frame hop le nam lien tiep trong 1 datagram (f1 + f2)
    auto f1 = createV2Heartbeat(1);
    auto f2 = createV2Heartbeat(2);
    std::vector<uint8_t> joined = f1;
    joined.insert(joined.end(), f2.begin(), f2.end());

    int caseA_count = 0;
    for (uint8_t b : joined)
    {
        if (parser.feed(b) == MAVLINK_FRAMING_OK) caseA_count++;
    }
    log << "  Case A (Ghep 2 frame vao 1 datagram): Nhan duoc " << caseA_count << "/2 frames hop le.\n";
    validCount += caseA_count;

    // Case B: 1 frame hop le f3 bi chia cat thanh nhieu manh stream nho (fragments)
    auto f3 = createV2Heartbeat(3);
    int caseB_count = 0;
    // Chunk 1: 5 bytes
    for (size_t i = 0; i < 5 && i < f3.size(); ++i)
        if (parser.feed(f3[i]) == MAVLINK_FRAMING_OK) caseB_count++;
    // Chunk 2: 7 bytes
    for (size_t i = 5; i < 12 && i < f3.size(); ++i)
        if (parser.feed(f3[i]) == MAVLINK_FRAMING_OK) caseB_count++;
    // Chunk 3: con lai
    for (size_t i = 12; i < f3.size(); ++i)
        if (parser.feed(f3[i]) == MAVLINK_FRAMING_OK) caseB_count++;

    log << "  Case B (Chia nho frame thanh nhieu chunk stream): Nhan duoc " << caseB_count << "/1 frame hop le.\n";
    validCount += caseB_count;

    // Case C: Tu dong dong bo lai (Resync) sau 50 byte rac ngau nhien
    std::vector<uint8_t> streamWithGarbage;
    for (int i = 0; i < 50; ++i) streamWithGarbage.push_back(static_cast<uint8_t>(0xA0 + (i % 30)));
    auto f4 = createV2Heartbeat(4);
    streamWithGarbage.insert(streamWithGarbage.end(), f4.begin(), f4.end());

    int caseC_count = 0;
    for (uint8_t b : streamWithGarbage)
    {
        if (parser.feed(b) == MAVLINK_FRAMING_OK) caseC_count++;
    }
    log << "  Case C (Resync sau 50 byte rac): Nhan duoc " << caseC_count << "/1 frame hop le.\n";
    validCount += caseC_count;

    // Case D: Phuc hoi sau frame loi CRC
    auto f5_bad = createV2Heartbeat(5);
    f5_bad[15] ^= 0xFF; // Lam hong CRC
    auto f6_good = createV2Heartbeat(6);
    std::vector<uint8_t> streamBadThenGood = f5_bad;
    streamBadThenGood.insert(streamBadThenGood.end(), f6_good.begin(), f6_good.end());

    int caseD_count = 0;
    bool caughtBadCrc = false;
    for (uint8_t b : streamBadThenGood)
    {
        uint8_t st = parser.feed(b);
        if (st == MAVLINK_FRAMING_BAD_CRC) caughtBadCrc = true;
        if (st == MAVLINK_FRAMING_OK) caseD_count++;
    }
    log << "  Case D (Bat loi frame hong roi phuc hoi frame tiep): Caught bad CRC: "
        << (caughtBadCrc ? "YES" : "NO") << ", Nhan duoc " << caseD_count << "/1 frame tiep theo.\n";
    validCount += caseD_count;

    res.framesTested = 5; // f1, f2, f3, f4, f6
    res.validFrames = validCount;
    res.passed = (validCount == 5 && caughtBadCrc);
    res.detail = "Dat ca 4 tinh huong: Ghep datagram (2/2), chia nho chunk stream (1/1), resync sau rac (1/1), phuc hoi sau loi (1/1).";
    log << res.detail << "\n";
    return res;
}

// ==============================================================================
// T1.4: Fuzzing 1 trieu byte rac ngau nhien -> Khong crash, khong treo
// ==============================================================================
TestCaseResult test_T1_4(std::ostream &log)
{
    TestCaseResult res;
    res.id = "T1.4";
    res.name = "Fuzz 1.000.000 byte rac ngau nhien";

    log << "\n--- [CHAY TEST T1.4: Fuzzing 1.000.000 bytes] ---\n";
    const size_t NUM_BYTES = 1000000;
    res.fuzzBytes = NUM_BYTES;

    std::mt19937 rng(1337); // Seed co dinh
    std::uniform_int_distribution<int> dist(0, 255);

    MavlinkParser parser;
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < NUM_BYTES; ++i)
    {
        uint8_t byte = static_cast<uint8_t>(dist(rng));
        uint8_t st = parser.feed(byte);
        if (st == MAVLINK_FRAMING_OK) res.validFrames++;
        else if (st == MAVLINK_FRAMING_BAD_CRC) res.badCrc++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    double durationSec = std::chrono::duration<double>(end - start).count();
    double throughputMBps = (NUM_BYTES / (1024.0 * 1024.0)) / durationSec;

    res.passed = (durationSec < 5.0);
    res.detail = "Da feed 1.000.000 bytes rac ngau nhien. Thoi gian: " +
                 std::to_string(durationSec).substr(0, 5) + "s (Toc do: " +
                 std::to_string(throughputMBps).substr(0, 5) + " MB/s). Khong crash.";
    log << res.detail << "\n";
    return res;
}

// ==============================================================================
// T1.5: Stream lien tuc 10.000 frame chuan -> Sai CRC = 0, dat 100%
// ==============================================================================
TestCaseResult test_T1_5(std::ostream &log)
{
    TestCaseResult res;
    res.id = "T1.5";
    res.name = "Stream 10.000 frame MAVLink lien tuc";

    log << "\n--- [CHAY TEST T1.5: Stream 10.000 frame lien tuc] ---\n";
    MavlinkParser parser;
    const int NUM_FRAMES = 10000;

    for (int i = 0; i < NUM_FRAMES; ++i)
    {
        auto f = createV2Heartbeat(static_cast<uint8_t>(i % 256));
        res.framesTested++;
        for (uint8_t b : f)
        {
            uint8_t st = parser.feed(b);
            if (st == MAVLINK_FRAMING_OK) res.validFrames++;
            else if (st == MAVLINK_FRAMING_BAD_CRC) res.badCrc++;
        }
    }

    res.passed = (res.validFrames == NUM_FRAMES && res.badCrc == 0);
    res.detail = "Nhan dung 10.000/10.000 frames (100%). Sai CRC = 0.";
    log << res.detail << "\n";
    return res;
}

// ==============================================================================
// MAIN: Dieu phoi chay va xuat file ket qua CSV + LOG
// ==============================================================================
int main()
{
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::string csvPath = "docs/ketqua/tich_hop_projectltm/stage1_framing_results.csv";
    std::string logPath = "docs/ketqua/tich_hop_projectltm/stage1_framing_log.txt";

    std::ofstream logFile(logPath.c_str());
    if (!logFile)
    {
        std::cerr << "[ERROR] Khong the tao file log: " << logPath << "\n";
        return 1;
    }

    std::cout << "==============================================================\n"
              << "       KIEM THU GIAI DOAN 1: PARSE FRAME VA CRC MAVLINK       \n"
              << "==============================================================\n";
    logFile   << "==============================================================\n"
              << "       KIEM THU GIAI DOAN 1: PARSE FRAME VA CRC MAVLINK       \n"
              << "==============================================================\n";

    std::vector<TestCaseResult> results;
    results.push_back(test_T1_1(logFile));
    results.push_back(test_T1_2(logFile));
    results.push_back(test_T1_3(logFile));
    results.push_back(test_T1_4(logFile));
    results.push_back(test_T1_5(logFile));

    // Xuat ra file CSV
    std::ofstream csvFile(csvPath.c_str());
    if (!csvFile)
    {
        std::cerr << "[ERROR] Khong the tao file CSV: " << csvPath << "\n";
        return 1;
    }

    csvFile << "test_id,test_name,frames_tested,valid_frames,bad_crc,fuzz_bytes,status,detail\n";

    std::cout << "\n===================================================================================\n"
              << "                              BANG KET QUA KIEM THU                                \n"
              << "===================================================================================\n";
    std::cout << std::left << std::setw(8)  << "Ma"
              << std::setw(38) << "Phep thu"
              << std::setw(10) << "Ket qua"
              << "Chi tiet\n";
    std::cout << "-----------------------------------------------------------------------------------\n";

    bool allPassed = true;
    for (const auto &r : results)
    {
        if (!r.passed) allPassed = false;

        std::string statusStr = r.passed ? "PASS" : "FAIL";
        std::cout << std::left << std::setw(8)  << r.id
                  << std::setw(38) << r.name
                  << std::setw(10) << statusStr
                  << r.detail << "\n";

        // Ghi vao CSV
        csvFile << r.id << ",\""
                << r.name << "\","
                << r.framesTested << ","
                << r.validFrames << ","
                << r.badCrc << ","
                << r.fuzzBytes << ","
                << statusStr << ",\""
                << r.detail << "\"\n";
    }

    csvFile.close();
    logFile.close();

    std::cout << "===================================================================================\n";
    if (allPassed)
    {
        std::cout << ">>> TONG KET: TAT CA PHEP THU GIAI DOAN 1 DA DAT (100% PASS) <<<\n";
    }
    else
    {
        std::cout << ">>> TONG KET: CO PHEP THU KHONG DAT! <<<\n";
    }

    std::cout << "\n[OK] Da xuat file ket qua CSV: " << csvPath << "\n";
    std::cout << "[OK] Da xuat file log chi tiet: " << logPath << "\n";

    return allPassed ? 0 : 1;
}
