#pragma once

#include "network.h"
#include "mavlink_codec.h"
#include "statistics.h"
#include "config.h"

#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>

struct Client
{
    SOCKET socket = INVALID_SOCKET;
    unsigned int id = 0;

    MavlinkParser parser;
    std::string pending;

    unsigned long long sentBytes = 0;
};

class ClientManager
{
public:
    using FrameHandler = std::function<
        bool(const Client &, const mavlink_message_t &)>;

    using CloseHandler = std::function<void(SOCKET)>;

    ~ClientManager()
    {
        for (auto &client : clients_)
        {
            Network::closeSocket(client.socket);
        }
    }

    std::size_t size() const
    {
        return clients_.size();
    }

    void prepareSets(
        fd_set &reads,
        fd_set &writes) const
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
        SOCKET socket = accept(
            listener,
            nullptr,
            nullptr);

        if (socket == INVALID_SOCKET)
        {
            int error = WSAGetLastError();

            if (error != WSAEWOULDBLOCK)
            {
                std::cerr
                    << "accept loi: " << error << '\n';
            }

            return;
        }

        if (clients_.size() >= Config::MAX_CLIENTS ||
            clients_.size() + 2 >= FD_SETSIZE)
        {

            std::cout << "Da du so client\n";
            Network::closeSocket(socket);
            return;
        }

        if (!Network::setNonblocking(socket))
        {
            Network::closeSocket(socket);
            return;
        }

        Client client;
        client.socket = socket;
        client.id = nextId_++;

        clients_.push_back(client);

        std::cout
            << "Client #" << client.id
            << " CONNECTED | Tong="
            << clients_.size() << '\n';
    }

    void processReady(
        fd_set &reads,
        fd_set &writes,
        Statistics &stats,
        const FrameHandler &onFrame,
        const CloseHandler &onClose)
    {
        char buffer[65536];

        for (auto &client : clients_)
        {
            // Nhan MAVLink tu client.
            if (FD_ISSET(client.socket, &reads))
            {
                int n = recv(
                    client.socket,
                    buffer,
                    sizeof(buffer),
                    0);

                if (n == 0)
                {
                    disconnect(client, onClose);
                }
                else if (n == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != WSAEWOULDBLOCK)
                    {
                        disconnect(client, onClose);
                    }
                }
                else
                {
                    for (int i = 0; i < n; ++i)
                    {
                        uint8_t status = client.parser.feed(
                            static_cast<uint8_t>(buffer[i]));

                        if (!stats.acceptParse(status, false))
                        {
                            continue;
                        }

                        ++stats.clientFrames;

                        // False: Gateway yeu cau ngat client.
                        if (!onFrame(
                                client,
                                client.parser.message))
                        {

                            disconnect(client, onClose);
                            break;
                        }
                    }
                }
            }

            // Gui phan du lieu dang cho.
            if (client.socket != INVALID_SOCKET &&
                FD_ISSET(client.socket, &writes) &&
                !client.pending.empty())
            {

                int sent = send(
                    client.socket,
                    client.pending.data(),
                    static_cast<int>(client.pending.size()),
                    0);

                if (sent > 0)
                {
                    client.pending.erase(
                        0,
                        static_cast<std::size_t>(sent));

                    client.sentBytes += sent;
                    stats.tcpTxBytes += sent;
                }
                else if (sent == 0)
                {
                    disconnect(client, onClose);
                }
                else if (
                    WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    disconnect(client, onClose);
                }
            }
        }

        removeDisconnected();
    }

    void broadcast(
        const std::string &packet,
        Statistics &stats,
        const CloseHandler &onClose)
    {
        for (auto &client : clients_)
        {
            if (client.pending.size() + packet.size() >
                Config::MAX_TCP_PENDING)
            {

                ++stats.slowClients;

                std::cout << "Client qua cham\n";
                disconnect(client, onClose);
            }
            else
            {
                client.pending.append(packet);
            }
        }

        removeDisconnected();
    }

private:
    void disconnect(
        Client &client,
        const CloseHandler &onClose)
    {
        SOCKET oldSocket = client.socket;

        std::cout
            << "Client #" << client.id
            << " DISCONNECTED | TCP TX="
            << client.sentBytes << " bytes\n";

        Network::closeSocket(client.socket);
        client.pending.clear();

        onClose(oldSocket);
    }

    void removeDisconnected()
    {
        clients_.erase(
            std::remove_if(
                clients_.begin(),
                clients_.end(),
                [](const Client &client)
                {
                    return client.socket == INVALID_SOCKET;
                }),
            clients_.end());
    }

    std::vector<Client> clients_;
    unsigned int nextId_ = 1;
};