#pragma once

#include "network.h"
#include "mavlink_codec.h"
#include "time_utils.h"
#include "config.h"

#include <map>
#include <vector>
#include <iostream>

struct Uav
{
    sockaddr_in address{};

    bool online = false;
    TimePoint lastHeartbeat{};

    unsigned int generation = 0;
    unsigned long long receivedFrames = 0;
};

inline bool isOnline(
    const Uav &uav,
    TimePoint now)
{
    return uav.online &&
           elapsedSeconds(now, uav.lastHeartbeat) <
               Config::HEARTBEAT_TIMEOUT_SECONDS;
}

class UavManager
{
public:
    // True khi hoc route moi hoac UAV ket noi lai.
    bool observeHeartbeat(
        const mavlink_message_t &message,
        const sockaddr_in &sender,
        TimePoint now)
    {
        if (message.msgid != MAVLINK_MSG_ID_HEARTBEAT ||
            message.sysid == 0)
        {
            return false;
        }

        mavlink_heartbeat_t heartbeat{};

        mavlink_msg_heartbeat_decode(
            &message,
            &heartbeat);

        if (heartbeat.autopilot == MAV_AUTOPILOT_INVALID ||
            heartbeat.type == MAV_TYPE_GCS)
        {
            return false;
        }

        Uav &uav = uavs_[message.sysid];

        bool alive = isOnline(uav, now);

        if (alive &&
            !Network::sameAddress(uav.address, sender))
        {

            std::cout
                << "CONFLICT: SYSID="
                << static_cast<int>(message.sysid)
                << " tu endpoint khac, bo qua\n";

            return false;
        }

        if (!alive)
        {
            ++uav.generation;
            uav.address = sender;

            std::cout
                << "UAV "
                << static_cast<int>(message.sysid)
                << " ONLINE tai "
                << inet_ntoa(sender.sin_addr)
                << ":" << ntohs(sender.sin_port)
                << '\n';
        }

        uav.online = true;
        uav.lastHeartbeat = now;

        return !alive;
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

                std::cout
                    << "UAV " << item.first
                    << " OFFLINE\n";
            }
        }

        return expired;
    }

    std::vector<int> destinations(
        int target,
        TimePoint now) const
    {
        std::vector<int> result;

        for (const auto &item : uavs_)
        {
            if ((target == 0 || target == item.first) &&
                isOnline(item.second, now))
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
            !isOnline(found->second, now) ||
            !Network::sameAddress(
                found->second.address,
                sender))
        {

            return false;
        }

        ++found->second.receivedFrames;
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
               Network::sameAddress(
                   found->second.address,
                   address);
    }

    const std::map<int, Uav> &all() const
    {
        return uavs_;
    }

private:
    std::map<int, Uav> uavs_;
};