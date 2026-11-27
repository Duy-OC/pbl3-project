@echo off
setlocal
cd /d "%~dp0"
if not exist bin mkdir bin
set "COMPILER="
for /f "delims=" %%G in ('where.exe g++.exe 2^>nul') do (
    if not defined COMPILER call :find_compiler "%%G"
)
if not defined COMPILER (
    echo Build requires MinGW-w64 GCC 7 or newer. Add its bin directory to PATH.
    exit /b 1
)
set "FLAGS=-std=c++17 -O2 -iquote src/common -iquote src/gateway -iquote src/client"
call :build_gateway gatewayA -DGATEWAY_PROFILE_A
if errorlevel 1 exit /b 1
call :build_gateway gatewayB -DGATEWAY_PROFILE_B
if errorlevel 1 exit /b 1
if /i "%~1"=="gateways" exit /b 0
call :build_gateway gateway
if errorlevel 1 exit /b 1
"%COMPILER%" %FLAGS% -static src/central/central.cpp -o bin/central.exe -lws2_32
if errorlevel 1 exit /b 1
"%COMPILER%" %FLAGS% -static src/client/tcpclient.cpp -o bin/tcpclient.exe -lws2_32
if errorlevel 1 exit /b 1
exit /b 0

:build_gateway
"%COMPILER%" %FLAGS% %~2 -static src/gateway/gateway.cpp -o "bin/%~1.exe" -lws2_32
exit /b %errorlevel%

:find_compiler
for /f "tokens=1 delims=." %%V in ('call "%~1" -dumpversion 2^>nul') do (
    if %%V GEQ 7 set "COMPILER=%~1"
)
exit /b 0
