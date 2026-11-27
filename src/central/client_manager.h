#pragma once

#include "../common/mavlink_codec.h"
#include "../common/network.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

class CentralClientManager
{
public:
    struct Client
    {
        SOCKET socket = INVALID_SOCKET;
        unsigned int id = 0;
        MavlinkParser parser{};
        std::string pending;
        std::uint64_t sentBytes = 0;
    };

    using MavlinkHandler = std::function<void(unsigned int clientId,
                                              const mavlink_message_t &msg,
                                              const std::vector<std::uint8_t> &rawBytes)>;
    using DisconnectHandler = std::function<void(unsigned int clientId)>;

    explicit CentralClientManager(std::size_t maxClients = 10,
                                  std::size_t maxQueueBytes = 1048576)
        : maxClients_(maxClients), maxQueueBytes_(maxQueueBytes) {}

    ~CentralClientManager()
    {
        for (auto &client : clients_)
        {
            Network::closeSocket(client.socket);
        }
    }

    std::size_t size() const { return clients_.size(); }

    void prepareSets(fd_set &reads, fd_set &writes) const
    {
        for (const auto &client : clients_)
        {
            FD_SET(client.socket, &reads);
            if (!client.pending.empty())
            {
                FD_SET(client.socket, &writes);
            }
        }
    }

    void acceptOne(SOCKET listener)
    {
        SOCKET s = ::accept(listener, nullptr, nullptr);
        if (s == INVALID_SOCKET)
        {
            const int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK)
            {
                std::cerr << "[Central] accept failed: " << err << '\n';
            }
            return;
        }
        if (clients_.size() >= maxClients_ || clients_.size() + 3 >= FD_SETSIZE)
        {
            Network::closeSocket(s);
            return;
        }
        if (!Network::setNonblocking(s))
        {
            Network::closeSocket(s);
            return;
        }
        Client client;
        client.socket = s;
        client.id = nextId_++;
        std::cout << "[Central] Client #" << client.id << " connected\n";
        clients_.push_back(std::move(client));
    }

    void broadcastTelemetry(const std::uint8_t *data,
                            std::size_t size,
                            const DisconnectHandler &onDisconnect)
    {
        if (!data || size == 0)
        {
            return;
        }
        for (auto it = clients_.begin(); it != clients_.end();)
        {
            if (it->pending.size() + size > maxQueueBytes_)
            {
                std::cerr << "[Central] Slow client #" << it->id << " disconnected (queue overflow)\n";
                const unsigned int id = it->id;
                Network::closeSocket(it->socket);
                it = clients_.erase(it);
                if (onDisconnect)
                {
                    onDisconnect(id);
                }
                continue;
            }
            it->pending.append(reinterpret_cast<const char *>(data), size);
            ++it;
        }
    }

    void serviceClients(const fd_set &reads,
                        const fd_set &writes,
                        const MavlinkHandler &onMavlink,
                        const DisconnectHandler &onDisconnect)
    {
        std::array<char, 8192> buffer{};
        for (auto it = clients_.begin(); it != clients_.end();)
        {
            bool drop = false;

            if (FD_ISSET(it->socket, &writes) && !it->pending.empty())
            {
                const int sent = ::send(it->socket,
                                        it->pending.data(),
                                        static_cast<int>(it->pending.size()),
                                        0);
                if (sent > 0)
                {
                    it->sentBytes += static_cast<std::uint64_t>(sent);
                    it->pending.erase(0, static_cast<std::size_t>(sent));
                }
                else if (sent == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    drop = true;
                }
            }

            if (!drop && FD_ISSET(it->socket, &reads))
            {
                const int received = ::recv(it->socket, buffer.data(), static_cast<int>(buffer.size()), 0);
                if (received > 0)
                {
                    for (int i = 0; i < received; ++i)
                    {
                        if (it->parser.feed(static_cast<std::uint8_t>(buffer[i])) == MAVLINK_FRAMING_OK)
                        {
                            if (onMavlink)
                            {
                                const std::string serialized = serialize(it->parser.message);
                                std::vector<std::uint8_t> raw(serialized.begin(), serialized.end());
                                onMavlink(it->id, it->parser.message, raw);
                            }
                        }
                    }
                }
                else if (received == 0 || (received == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK))
                {
                    drop = true;
                }
            }

            if (drop)
            {
                std::cout << "[Central] Client #" << it->id << " disconnected\n";
                const unsigned int id = it->id;
                Network::closeSocket(it->socket);
                it = clients_.erase(it);
                if (onDisconnect)
                {
                    onDisconnect(id);
                }
            }
            else
            {
                ++it;
            }
        }
    }

private:
    std::size_t maxClients_ = 10;
    std::size_t maxQueueBytes_ = 1048576;
    unsigned int nextId_ = 1;
    std::vector<Client> clients_;
};
