// Copyright (c) 2016-2018 FRC Team 3512. All Rights Reserved.

#include "AutonomousModes/AutoRightSwitch.hpp"

#include <DriverStation.h>

#include "Robot.hpp"

AutoRightSwitch::AutoRightSwitch() { autoTimer.Start(); }

void AutoRightSwitch::Reset() { state = State::kInit; }

void AutoRightSwitch::HandleEvent(Event event) {
    static std::string platePosition;
    static frc::TrapezoidProfile positionProfile;
    static frc::TrapezoidProfile angleProfile;

    switch (state) {
        case State::kInit:
            platePosition =
                frc::DriverStation::GetInstance().GetGameSpecificMessage();

            if (platePosition[kFriendlySwitch] == 'R') {
                positionProfile.SetGoal(168.0 - kRobotLength / 2.0);
            } else {
                positionProfile.SetGoal(252.0 - 17.0 -
                                                  kRobotLength / 2.0);
            }
            Robot::robotDrive.SetAngleGoal(0.0);
            Robot::robotDrive.Enable();

            Robot::elevator.SetHeightReference(kSwitchHeight);
            Robot::elevator.StartClosedLoop();

            autoTimer.Reset();

            state = State::kInitialForward;
            break;

        case State::kInitialForward:
            if (positionProfile.AtGoal() ||
                autoTimer.Get() >
                    positionProfile.ProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetGyro();
                Robot::robotDrive.SetAngleGoal(-90.0);
                autoTimer.Reset();
                if (platePosition[kFriendlySwitch] == 'R') {
                    state = State::kFinalRotate;
                } else {
                    state = State::kRightRotate;
                }
            }
            break;
        case State::kRightRotate:
            if (Robot::robotDrive.AtAngleGoal() ||
                autoTimer.Get() >
                    Robot::robotDrive.AngleProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetEncoders();
                positionProfile.SetGoal(190.0 - 38.0);
                autoTimer.Reset();

                state = State::kRightForward;
            }
            break;
        case State::kRightForward:
            if (positionProfile.AtGoal() ||
                autoTimer.Get() >
                    positionProfile.ProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetGyro();
                Robot::robotDrive.SetAngleGoal(-90.0 + 35.0);
                autoTimer.Reset();

                state = State::kFinalRotate;
            }
            break;
        case State::kFinalRotate:
            if (Robot::robotDrive.AtAngleGoal() ||
                autoTimer.Get() >
                    Robot::robotDrive.AngleProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetEncoders();
                if (platePosition[kFriendlySwitch] == 'R') {
                    positionProfile.SetGoal(
                        65.0 + 12.0 - kRobotLength / 2.0 - kRobotWidth / 2.0);
                } else {
                    positionProfile.SetGoal(
                        36.0 + 24.0 - kRobotLength / 2.0);  // 28.0
                }
                autoTimer.Reset();

                state = State::kFinalForward;
            }
            break;
        case State::kFinalForward:
            if (positionProfile.AtGoal() ||
                autoTimer.Get() >
                    positionProfile.ProfileTimeTotal() + 0.1) {
                Robot::intake.AutoOuttake();
                Robot::robotDrive.Disable();
                Robot::elevator.StopClosedLoop();

                state = State::kIdle;
            }
            break;
        case State::kIdle:
            break;
    }
}
