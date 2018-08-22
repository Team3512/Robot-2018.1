// Copyright (c) 2016-2019 FRC Team 3512. All Rights Reserved.

#pragma once

#include <frc/Timer.h>

class AutoLeftPriority {
public:
    AutoLeftPriority();

    void Reset();

    void Run();

private:
    frc::Timer autoTimer;

    enum class State {
        kInit,
        kInitialForward,
        kRightRotate,
        kRightForward,
        kFinalRotate,
        kFinalForward,
        kAutoLine,
        kAutoSwitchRotate,
        kAutoSwitchForward,
        kAutoSwitch,
        kIdle
    };

    State state;
};
