#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <winsock2.h>
#include <iostream>

namespace Network
{

    class Runtime
    {
    public:
        Runtime()
        {
            WSADATA data{};
            error_ = WSAStartup(MAKEWORD(2, 2), &data);
        }

        ~Runtime()
        {
            if (error_ == 0)
            {
                WSACleanup();
            }
        }

        int error() const
        {
            return error_;
        }

        Runtime(const Runtime &) = delete;
        Runtime &operator=(const Runtime &) = delete;

    private:
        int error_;
    };

    inline bool setNonblocking(SOCKET socket)
    {
        u_long mode = 1;

        return ioctlsocket(
                   socket,
                   FIONBIO,
                   &mode) != SOCKET_ERROR;
    }

    inline void closeSocket(SOCKET &socket)
    {
        if (socket != INVALID_SOCKET)
        {
            closesocket(socket);
        }

        socket = INVALID_SOCKET;
    }

    inline bool sameAddress(
        const sockaddr_in &a,
        const sockaddr_in &b)
    {
        return a.sin_addr.s_addr == b.sin_addr.s_addr &&
               a.sin_port == b.sin_port;
    }

    inline SOCKET createBoundSocket(
        int type,
        unsigned short port)
    {
        SOCKET socket = ::socket(
            AF_INET,
            type,
            type == SOCK_DGRAM ? IPPROTO_UDP : IPPROTO_TCP);

        if (socket == INVALID_SOCKET)
        {
            std::cerr << "socket loi: "
                      << WSAGetLastError() << '\n';

            return INVALID_SOCKET;
        }

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_port = htons(port);

        // UDP nhan tu WSL.
        // TCP chi nhan client tren cung Windows.
        address.sin_addr.s_addr = htonl(
            type == SOCK_DGRAM ? INADDR_ANY : INADDR_LOOPBACK);

        if (bind(
                socket,
                reinterpret_cast<const sockaddr *>(&address),
                sizeof(address)) == SOCKET_ERROR)
        {

            std::cerr << "bind cong " << port
                      << " loi: " << WSAGetLastError() << '\n';

            closeSocket(socket);
            return INVALID_SOCKET;
        }

        if (!setNonblocking(socket))
        {
            std::cerr << "non-blocking loi: "
                      << WSAGetLastError() << '\n';

            closeSocket(socket);
        }

        return socket;
    }

}