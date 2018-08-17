// Copyright (c) 2014-2018 FRC Team 3512. All Rights Reserved.

#pragma once

#include <stdint.h>

#include <list>

#include "Logging/LogSinkBase.hpp"

/**
 * A file sink for the logged events.
 */
class LogServerSink : public LogSinkBase {
public:
    virtual ~LogServerSink();

    /**
     * Write an event to the logfile.
     *
     * @param event The event to log.
     */
    void Log(LogEvent event) override;

    int StartServer(uint16_t port);

    int Acceptor(bool blocking);

private:
    int TcpListen(uint16_t port);
    int AcceptConnectionBlocking(void);
    int AcceptConnectionNonBlocking(void);

    int m_listensd = -1;
    std::list<int> m_connections;
};
