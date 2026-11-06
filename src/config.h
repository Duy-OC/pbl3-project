#pragma once

#include <cstddef>

namespace Config
{
    constexpr unsigned short UDP_PORT = 14550;
    constexpr unsigned short TCP_PORT = 5760;

    constexpr int HEARTBEAT_TIMEOUT_SECONDS = 5;
    constexpr int STATS_INTERVAL_SECONDS = 5;
    constexpr int SOURCE_IDLE_SECONDS = 30;

    constexpr long SELECT_TIMEOUT_US = 200000;

    constexpr std::size_t MAX_CLIENTS = 10;
    constexpr std::size_t MAX_TCP_PENDING = 256 * 1024;
    constexpr std::size_t MAX_UDP_PENDING = 512;
    constexpr std::size_t MAX_UDP_SOURCES = 32;
}