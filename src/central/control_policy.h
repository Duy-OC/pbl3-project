#pragma once

#include "../common/mavlink_codec.h"

#include <optional>

class ControlPolicy
{
public:
    explicit ControlPolicy(bool allowObserverReadRequests = true)
        : allowObserverReadRequests_(allowObserverReadRequests) {}

    std::optional<unsigned int> controllerId() const { return controllerClientId_; }

    bool acquire(unsigned int clientId)
    {
        if (controllerClientId_ && *controllerClientId_ != clientId)
        {
            return false;
        }
        controllerClientId_ = clientId;
        return true;
    }

    bool release(unsigned int clientId)
    {
        if (!controllerClientId_ || *controllerClientId_ != clientId)
        {
            return false;
        }
        controllerClientId_.reset();
        return true;
    }

    void onClientDisconnect(unsigned int clientId)
    {
        if (controllerClientId_ && *controllerClientId_ == clientId)
        {
            controllerClientId_.reset();
        }
    }

    static bool isReadOnlyMessage(const mavlink_message_t &msg)
    {
        switch (msg.msgid)
        {
        case MAVLINK_MSG_ID_HEARTBEAT:
        case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
        case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
        case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:
        case MAVLINK_MSG_ID_MISSION_REQUEST_INT:
        case MAVLINK_MSG_ID_MISSION_REQUEST:
        case MAVLINK_MSG_ID_REQUEST_DATA_STREAM:
            return true;
        default:
            return false;
        }
    }

    // First eligible client that sends a control message acquires the single-controller lock.
    bool canForward(unsigned int clientId, const mavlink_message_t &msg)
    {
        if (isReadOnlyMessage(msg))
        {
            return allowObserverReadRequests_ || (controllerClientId_ && *controllerClientId_ == clientId);
        }
        if (!controllerClientId_)
        {
            controllerClientId_ = clientId;
            return true;
        }
        return *controllerClientId_ == clientId;
    }

private:
    bool allowObserverReadRequests_ = true;
    std::optional<unsigned int> controllerClientId_;
};
