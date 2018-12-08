// Copyright (c) 2016-2018 FRC Team 3512. All Rights Reserved.

#pragma once

#include <Timer.h>

#include "ES/Service.hpp"

class AutoLeftDouble : public Service {
public:
    AutoLeftDouble();

    void Reset();

    void HandleEvent(Event event) override;

private:
    frc::Timer autoTimer;

    enum class State {
        kInit,
        kInitialForward,
        kLeftRotate,
        kLeftForward,
        kFinalRotate,
        kFinalForward,
        kDoubleRotate,
        kDoubleForward,
        kSpit,
        kIdle
    };

    State state;
};
