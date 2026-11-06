#pragma once

#include "time_utils.h"
#include "mavlink_codec.h"
#include "uav_manager.h"
#include "config.h"

#include <cstddef>
#include <iostream>

struct Statistics
{
    unsigned long long udpDatagrams = 0;
    unsigned long long udpRxBytes = 0;

    unsigned long long uavFrames = 0;
    unsigned long long clientFrames = 0;

    unsigned long long badCrc = 0;
    unsigned long long badSignature = 0;

    unsigned long long udpBadCrc = 0;
    unsigned long long tcpBadCrc = 0;

    unsigned long long tcpTxBytes = 0;
    unsigned long long udpTxFrames = 0;

    unsigned long long droppedUdpFrames = 0;
    unsigned long long blockedMessages = 0;
    unsigned long long slowClients = 0;

    unsigned long long blockedUnknownMetadata = 0;
    unsigned long long blockedNoRoute = 0;

    bool acceptParse(uint8_t status, bool fromUdp)
    {
        if (status == MAVLINK_FRAMING_BAD_CRC)
        {
            ++badCrc;

            if (fromUdp)
            {
                ++udpBadCrc;
            }
            else
            {
                ++tcpBadCrc;
            }
        }
        else if (
            status == MAVLINK_FRAMING_BAD_SIGNATURE)
        {
            ++badSignature;
        }

        return status == MAVLINK_FRAMING_OK;
    }
};

class StatisticsReporter
{
public:
    void printIfDue(
        const Statistics &stats,
        const std::map<int, Uav> &uavs,
        std::size_t clients,
        std::size_t queued,
        TimePoint now)
    {
        double seconds = elapsedSeconds(now, last_);

        if (seconds < Config::STATS_INTERVAL_SECONDS)
        {
            return;
        }

        std::size_t online = 0;

        for (const auto &item : uavs)
        {
            if (isOnline(item.second, now))
            {
                ++online;
            }
        }

        std::cout
            << "\n=== STATS ==="
            << "\nUAV online=" << online
            << " | UAV da hoc=" << uavs.size()
            << " | Clients=" << clients

            << "\nUDP RX datagrams=" << stats.udpDatagrams
            << " | UDP RX bytes=" << stats.udpRxBytes

            << "\nMAVLink UAV RX=" << stats.uavFrames
            << " | Client RX=" << stats.clientFrames

            << "\nCRC errors=" << stats.badCrc
            << " (UDP=" << stats.udpBadCrc
            << ", TCP=" << stats.tcpBadCrc << ")"
            << " | Signature errors=" << stats.badSignature

            << "\nTCP TX bytes=" << stats.tcpTxBytes
            << " | UDP TX frames=" << stats.udpTxFrames

            << "\nUDP queued=" << queued
            << " | UDP dropped/cancelled="
            << stats.droppedUdpFrames

            << "\nBlocked messages=" << stats.blockedMessages
            << " (no route=" << stats.blockedNoRoute
            << ", unknown metadata="
            << stats.blockedUnknownMetadata << ")"
            << " | Slow clients=" << stats.slowClients

            << "\nUDP RX rate="
            << (stats.udpRxBytes - previousBytes_) / seconds
            << " B/s"

            << " | MAVLink UAV RX rate="
            << (stats.uavFrames - previousFrames_) / seconds
            << " frames/s\n";

        for (const auto &item : uavs)
        {
            std::cout
                << "  UAV " << item.first
                << ": "
                << (isOnline(item.second, now)
                        ? "ONLINE"
                        : "OFFLINE")
                << " | RX frames="
                << item.second.receivedFrames
                << '\n';
        }

        std::cout << "=============\n\n";

        previousBytes_ = stats.udpRxBytes;
        previousFrames_ = stats.uavFrames;
        last_ = now;
    }

private:
    TimePoint last_ = Clock::now();

    unsigned long long previousBytes_ = 0;
    unsigned long long previousFrames_ = 0;
};