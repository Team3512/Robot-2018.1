// Copyright (c) 2013-2020 FRC Team 3512. All Rights Reserved.

#include "livegrapher/LiveGrapher.hpp"

#include <algorithm>
#include <cstring>

#ifdef __VXWORKS__
#include <hostLib.h>
#include <pipeDrv.h>
#include <selectLib.h>
#include <sockLib.h>

#include <cstdio>
#define be64toh(x) x
#else
#include <endian.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#endif

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

LiveGrapher::LiveGrapher(int port) {
    m_currentTime =
        duration_cast<milliseconds>(system_clock::now().time_since_epoch())
            .count();

    // Store the port to listen on
    m_port = port;

    // Create a pipe for IPC with the thread
    int pipefd[2];
#ifdef __VXWORKS__
    pipeDevCreate("/pipe/graphhost", 10, 100);
    pipefd[0] = open("/pipe/graphhost", O_RDONLY, 0644);
    pipefd[1] = open("/pipe/graphhost", O_WRONLY, 0644);

    if (pipefd[0] == -1 || pipefd[1] == -1) {
        return;
    }
#else
    if (pipe(pipefd) == -1) {
        return;
    }
#endif

    m_ipcfd_r = pipefd[0];
    m_ipcfd_w = pipefd[1];

    // Launch the thread
    m_thread = std::thread([this] { socket_threadmain(); });
}

LiveGrapher::~LiveGrapher() {
    // Tell the other thread to stop
    write(m_ipcfd_w, "x", 1);

    // Join to the other thread
    m_thread.join();

    // Close file descriptors and clean up
    close(m_ipcfd_r);
    close(m_ipcfd_w);
}

bool LiveGrapher::GraphData(float value, std::string dataset) {
    if (!m_running) {
        return false;
    }

    // This will only work if ints are the same size as floats
    static_assert(sizeof(float) == sizeof(uint32_t),
                  "float isn't 32 bits long");

    m_currentTime =
        duration_cast<milliseconds>(system_clock::now().time_since_epoch())
            .count();

    auto i = m_graphList.find(dataset);

    if (i == m_graphList.end()) {
        m_graphList.emplace(dataset, m_graphList.size());
    }

    ClientDataPacket packet;
    packet.ID = kClientDataPacket | i->second;

    // Change to network byte order
    // Swap bytes in x, and copy into the payload struct
    uint64_t xtmp;
    std::memcpy(&xtmp, &m_currentTime, sizeof(xtmp));
    xtmp = be64toh(xtmp);
    std::memcpy(&packet.x, &xtmp, sizeof(xtmp));

    // Swap bytes in y, and copy into the payload struct
    uint32_t ytmp;
    std::memcpy(&ytmp, &value, sizeof(ytmp));
    ytmp = htonl(ytmp);
    std::memcpy(&packet.y, &ytmp, sizeof(ytmp));

    std::lock_guard<std::mutex> lock(m_mutex);

    // Send the point to connected clients
    for (auto& conn : m_connList) {
        for (const auto& datasetID : conn->dataSets) {
            if (datasetID == i->second) {
                // Send the value off
                conn->queueWrite(packet);
            }
        }
    }

    return true;
}

bool LiveGrapher::HasIntervalPassed() {
    m_currentTime =
        duration_cast<milliseconds>(system_clock::now().time_since_epoch())
            .count();

    return m_currentTime - m_lastTime > m_sendInterval;
}

void LiveGrapher::ResetInterval() { m_lastTime = m_currentTime; }

uint8_t LiveGrapher::packetID(uint8_t id) {
    // Masks two high-order bits
    return id & 0xC0;
}

uint8_t LiveGrapher::graphID(uint8_t id) {
    // Masks six low-order bits
    return id & 0x2F;
}

void LiveGrapher::socket_threadmain() {
    int listenfd;
    int maxfd;
    uint8_t ipccmd = 0;

    fd_set readfds;
    fd_set writefds;
    fd_set errorfds;

    // Listen on a socket
    listenfd = socket_listen(m_port, 0);
    if (listenfd == -1) {
        return;
    }

    // Set the running flag after we've finished initializing everything
    m_running = true;

    while (ipccmd != 'x') {
        // Clear the fdsets
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&errorfds);

        // Reset the maxfd
        maxfd = listenfd;

        {
            std::lock_guard<std::mutex> lock(m_mutex);

            // Add the file descriptors to the list
            for (auto& conn : m_connList) {
                if (maxfd < conn->fd) {
                    maxfd = conn->fd;
                }
                if (conn->selectflags & SocketConnection::Read) {
                    FD_SET(conn->fd, &readfds);
                }
                if (conn->selectflags & SocketConnection::Write) {
                    FD_SET(conn->fd, &writefds);
                }
                if (conn->selectflags & SocketConnection::Error) {
                    FD_SET(conn->fd, &errorfds);
                }
            }
        }

        // Select on the listener fd
        FD_SET(listenfd, &readfds);

        // ipcfd will receive data when the thread needs to exit
        FD_SET(m_ipcfd_r, &readfds);

        // Select on the file descriptors
        select(maxfd + 1, &readfds, &writefds, &errorfds, nullptr);

        {
            std::lock_guard<std::mutex> lock(m_mutex);

            auto conn = m_connList.begin();
            while (conn != m_connList.end()) {
                if (FD_ISSET((*conn)->fd, &readfds)) {
                    // Handle reading
                    if (ReadPackets(conn->get()) == -1) {
                        conn = m_connList.erase(conn);
                        continue;
                    }
                }
                if (FD_ISSET((*conn)->fd, &writefds)) {
                    // Handle writing
                    (*conn)->writePackets();
                }
                if (FD_ISSET((*conn)->fd, &errorfds)) {
                    // Handle errors
                    conn = m_connList.erase(conn);
                    continue;
                }

                conn++;
            }
        }

        // Check for listener condition
        if (FD_ISSET(listenfd, &readfds)) {
            // Accept connections
            int fd = socket_accept(listenfd);

            if (fd != -1) {
                // Disable Nagle's algorithm
                int yes = 1;
                setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
                           reinterpret_cast<char*>(&yes), sizeof(yes));

                std::lock_guard<std::mutex> lock(m_mutex);
                // Add it to the list, this makes it a bit non-thread-safe
                m_connList.emplace_back(
                    std::make_unique<SocketConnection>(fd, m_ipcfd_w));
            }
        }

        // Handle IPC commands
        if (FD_ISSET(m_ipcfd_r, &readfds)) {
            read(m_ipcfd_r, reinterpret_cast<char*>(&ipccmd), 1);
        }
    }

    // We're done, clear the running flag and clean up
    m_running = false;

    // Close the listener file descriptor
    close(listenfd);
}

/* Listens on a specified port (listenport), and returns the file descriptor
 * to the listening socket.
 */
int LiveGrapher::socket_listen(int port, uint32_t s_addr) {
    sockaddr_in serv_addr;
    int sd = -1;

    try {
        // Create a TCP socket
        sd = socket(AF_INET, SOCK_STREAM, 0);
        if (sd == -1) {
            throw -1;
        }

// Allow rebinding to the socket later if the connection is interrupted
#ifndef __VXWORKS__
        int optval = 1;
        setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
#endif

        // Zero out the serv_addr struct
        std::memset(&serv_addr, 0, sizeof(sockaddr_in));

        // Set up the listener sockaddr_in struct
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = s_addr;
        serv_addr.sin_port = htons(port);

        // Bind the socket to the listener sockaddr_in
        if (bind(sd, reinterpret_cast<sockaddr*>(&serv_addr),
                 sizeof(sockaddr_in)) != 0) {
            throw -1;
        }

        // Listen on the socket for incoming connections
        if (listen(sd, 5) != 0) {
            throw -1;
        }
    } catch (int e) {
        std::perror("");
        if (sd != -1) {
            close(sd);
        }
        return -1;
    }

    // Make sure we aren't killed by SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    return sd;
}

int LiveGrapher::socket_accept(int listenfd) {
#ifdef __VXWORKS__
    int clilen;
#else
    unsigned int clilen;
#endif
    sockaddr_in cli_addr;

    clilen = sizeof(cli_addr);

    int new_fd = -1;

    try {
        // Accept a new connection
        new_fd =
            accept(listenfd, reinterpret_cast<sockaddr*>(&cli_addr), &clilen);

        // Make sure that the file descriptor is valid
        if (new_fd == -1) {
            throw -1;
        }

#ifdef __VXWORKS__
        // Set the socket non-blocking
        int on = 1;
        if (ioctl(new_fd, static_cast<int>(FIONBIO), on) == -1) {
            throw -1;
        }
#else
        // Set the socket non-blocking
        int flags = fcntl(new_fd, F_GETFL, 0);
        if (flags == -1) {
            throw -1;
        }

        if (fcntl(new_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            throw -1;
        }
#endif
    } catch (int e) {
        std::perror("");
        if (new_fd != -1) {
            close(new_fd);
        }
        return -1;
    }

    return new_fd;
}

int LiveGrapher::ReadPackets(SocketConnection* conn) {
    int error;
    uint8_t id;

    error = conn->recvData(reinterpret_cast<char*>(&id), 1);
    if (error == 0 || (error == -1 && errno != EAGAIN)) {
        return -1;
    }

    switch (packetID(id)) {
        case kHostConnectPacket:
            // Start sending data for the graph specified by the ID
            if (std::find(conn->dataSets.begin(), conn->dataSets.end(),
                          graphID(id)) == conn->dataSets.end()) {
                conn->dataSets.push_back(graphID(id));
            }
            break;
        case kHostDisconnectPacket:
            // Stop sending data for the graph specified by the ID
            conn->dataSets.erase(std::remove(conn->dataSets.begin(),
                                             conn->dataSets.end(), graphID(id)),
                                 conn->dataSets.end());
            break;
        case kHostListPacket:
            /* A graph count is compared against instead of the graph ID for
             * terminating list traversal because the std::map is sorted by
             * graph name instead of the ID. Since, the IDs are not necessarily
             * in order, early traversal termination could occur.
             */
            size_t graphCount = 0;
            for (auto& graph : m_graphList) {
                if (m_buf.length() < 1 + 1 + graph.first.length() + 1) {
                    m_buf.resize(1 + 1 + graph.first.length() + 1);
                }

                m_buf[0] = kClientListPacket | graph.second;
                m_buf[1] = graph.first.length();
                std::strncpy(&m_buf[2], graph.first.c_str(),
                             graph.first.length());

                // Is this the last element in the list?
                if (graphCount + 1 == m_graphList.size()) {
                    m_buf[2 + m_buf[1]] = 1;
                } else {
                    m_buf[2 + m_buf[1]] = 0;
                }

                // Queue the datagram for writing
                conn->queueWrite(m_buf.c_str(),
                                 1 + 1 + graph.first.length() + 1);

                graphCount++;
            }
            break;
    }

    return 0;
}
