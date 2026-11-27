@echo off
setlocal
cd /d "%~dp0.."
if not exist bin mkdir bin
set "PROXY_OUTPUT=bin/udp_proxy.exe"
if not "%~1"=="" set "PROXY_OUTPUT=%~1"
set "PROXY_COMPILER="
for /f "delims=" %%G in ('where.exe g++.exe 2^>nul') do (
    if not defined PROXY_COMPILER call :find_compiler "%%G"
)
if not defined PROXY_COMPILER (
    echo Build requires MinGW-w64 GCC 7 or newer. Add its bin directory to PATH.
    exit /b 1
)
"%PROXY_COMPILER%" -std=c++17 -O2 -Wall -Wextra -static src/proxy/udp_proxy.cpp -o "%PROXY_OUTPUT%" -lws2_32
exit /b %errorlevel%

:find_compiler
for /f "tokens=1 delims=." %%V in ('call "%~1" -dumpversion 2^>nul') do (
    if %%V GEQ 7 set "PROXY_COMPILER=%~1"
)
exit /b 0
