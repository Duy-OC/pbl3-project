@echo off
cd /d "%~dp0"
if not exist bin mkdir bin
g++ -std=c++17 -O2 -Isrc/common -Isrc/gateway -Isrc/client src/gateway/gateway.cpp -o bin/gateway.exe -lws2_32
if errorlevel 1 exit /b 1
g++ -std=c++17 -O2 -Isrc/common -Isrc/gateway -Isrc/client src/client/tcpclient.cpp -o bin/tcpclient.exe -lws2_32
if errorlevel 1 exit /b 1
if /I not "%~1"=="tests" exit /b 0
g++ -std=c++17 -O2 -Isrc/common -Isrc/gateway -Isrc/client tests/test_stage1_framing.cpp -o bin/test_stage1_framing.exe -lws2_32
if errorlevel 1 exit /b 1
g++ -std=c++17 -O2 -Isrc/common -Isrc/gateway -Isrc/client tests/test_stage2_telemetry.cpp -o bin/test_stage2_telemetry.exe -lws2_32
if errorlevel 1 exit /b 1
g++ -std=c++17 -O2 -Isrc/common -Isrc/gateway -Isrc/client tests/test_sequence_cases.cpp -o bin/test_sequence_cases.exe -lws2_32
if errorlevel 1 exit /b 1
g++ -std=c++17 -O2 -Isrc/common -Isrc/gateway -Isrc/client tools/mavlink_parser_check.cpp -o bin/mavlink_parser_check.exe -lws2_32
if errorlevel 1 exit /b 1
g++ -std=c++17 -O2 -Isrc/common -Isrc/gateway -Isrc/client tools/mavlink_integrity_check.cpp -o bin/mavlink_integrity_check.exe -lws2_32
if errorlevel 1 exit /b 1
exit /b 0
