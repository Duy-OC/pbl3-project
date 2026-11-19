@echo off
cd /d "%~dp0"
if not exist bin mkdir bin
g++ -std=c++17 -O2 -Isrc/common -Isrc/gateway -Isrc/client -Isrc/proxy src/gateway/gateway.cpp -o bin/gateway.exe -lws2_32
if errorlevel 1 exit /b 1
g++ -std=c++17 -O2 -Isrc/common -Isrc/gateway -Isrc/client -Isrc/proxy src/client/tcpclient.cpp -o bin/tcpclient.exe -lws2_32
if errorlevel 1 exit /b 1
if /I not "%~1"=="tests" exit /b 0
g++ -std=c++17 -O2 -Isrc/common -Isrc/gateway -Isrc/client -Isrc/proxy tests/test_stage1_framing.cpp -o bin/test_stage1_framing.exe -lws2_32
if errorlevel 1 exit /b 1
g++ -std=c++17 -O2 -Isrc/common -Isrc/gateway -Isrc/client -Isrc/proxy tests/test_stage2_telemetry.cpp -o bin/test_stage2_telemetry.exe -lws2_32
if errorlevel 1 exit /b 1
g++ -std=c++17 -O2 -Isrc/common -Isrc/gateway -Isrc/client -Isrc/proxy tests/test_sequence_cases.cpp -o bin/test_sequence_cases.exe -lws2_32
if errorlevel 1 exit /b 1
g++ -std=c++17 -O2 -Isrc/common -Isrc/gateway -Isrc/client -Isrc/proxy tools/markov_model_check.cpp -o bin/markov_model_check.exe -lws2_32
if errorlevel 1 exit /b 1
g++ -std=c++17 -O2 -Isrc/common -Isrc/gateway -Isrc/client -Isrc/proxy tools/mavlink_parser_check.cpp -o bin/mavlink_parser_check.exe -lws2_32
if errorlevel 1 exit /b 1
g++ -std=c++17 -O2 -Isrc/common -Isrc/gateway -Isrc/client -Isrc/proxy tools/mavlink_integrity_check.cpp -o bin/mavlink_integrity_check.exe -lws2_32
if errorlevel 1 exit /b 1
exit /b 0
