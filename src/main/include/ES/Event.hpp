// Copyright (c) 2018-2019 FRC Team 3512. All Rights Reserved.

#pragma once

#include <stdint.h>

/**
 * Event names to be polled and handled
 */
enum EventType {
    kNoEvent,
    kExit,
    kEntry,
    kButtonPressed,
    kButtonReleased,
    kButton,
    kAtSetHeight,
    kElevatorSetSwitch,
    kElevatorSetScale,
    kElevatorSetClimb,
    kTimeout
};

struct Event {
    Event() = default;
    Event(EventType type, int32_t param = 0);  // NOLINT(runtime/explicit)

    bool operator==(const Event& rhs) const;

    EventType type = kNoEvent;
    int32_t param = 0;
};
