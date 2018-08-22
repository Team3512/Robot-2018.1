// Copyright (c) 2016-2018 FRC Team 3512. All Rights Reserved.

#include "AutonomousModes/AutoCenterScale.hpp"

#include <DriverStation.h>

#include "Robot.hpp"

AutoCenterScale::AutoCenterScale() { autoTimer.Reset(); }

void AutoCenterScale::Reset() { state = State::kInit; }

void AutoCenterScale::HandleEvent(Event event) {
    static std::string platePosition;

    switch (state) {
        case State::kInit:
            platePosition =
                frc::DriverStation::GetInstance().GetGameSpecificMessage();

            Robot::robotDrive.SetGoal(pose(67.0 - kRobotLength / 2.0, 0.0, 0.0)
            Robot::robotDrive.Enable();

            Robot::elevator.SetHeightReference(kSwitchHeight);
            Robot::elevator.StartClosedLoop();

            autoTimer.Reset();

            state = State::kInitialForward;
            break;

        case State::kInitialForward:
            if (positionProfile.AtGoal() ||
                autoTimer.Get() > positionProfile.ProfileTimeTotal() + 1.0) {
                autoTimer.Reset();
                if (platePosition[kScale] == 'R') {
                    Robot::robotDrive.SetGoal(pose(0.0, 0.0, 90.0);
                } else {
                    Robot::robotDrive.SetGoal(pose(0.0, 0.0, -90.0);
                }

                state = State::kInitialRotate;
            }
            break;
        case State::kInitialRotate:
            if (angleProfile.AtGoal() ||
                autoTimer.Get() > angleProfile.ProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetEncoders();
                autoTimer.Reset();
                if (platePosition[kScale] == 'R') {
                    Robot::robotDrive.SetGoal(132.0 - kExchangeOffset -
                                            kRobotLength / 2.0, 0.0, 0.0);
                } else {
                    Robot::robotDrive.SetGoal(132.0 + kExchangeOffset -
                                            kRobotLength / 2.0, 0.0, 0.0);
                }

                state = State::kSecondForward;
            }
            break;
        case State::kSecondForward:
            if (positionProfile.AtGoal() ||
                autoTimer.Get() > positionProfile.ProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetGyro();
                autoTimer.Reset();
                if (platePosition[kScale] == 'R') {
                    Robot::robotDrive.SetGoal(0.0, 0.0, -90.0);
                } else {
                    Robot::robotDrive.SetGoal(0.0, 0.0, 90.0);
                }

                state = State::kSecondRotate;
            }
            break;
        case State::kSecondRotate:
            if (angleProfile.AtGoal() ||
                autoTimer.Get() > angleProfile.ProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetEncoders();
                Robot::robotDrive.SetGoal(260.0, 0.0, 0.0);
                autoTimer.Reset();

                state = State::kThirdForward;
            }
            break;
        case State::kThirdForward:
            if (positionProfile.AtGoal() ||
                autoTimer.Get() > positionProfile.ProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetGyro();
                autoTimer.Reset();
                if (platePosition[kScale] == 'R') {
                    Robot::robotDrive.SetGoal(0.0, 0.0, -90.0);
                } else {
                    Robot::robotDrive.SetGoal(0.0, 0.0, 90.0);
                }

                state = State::kFinalRotate;
            }
            break;
        case State::kFinalRotate:
            if (angleProfile.AtGoal() ||
                autoTimer.Get() > angleProfile.ProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetEncoders();
                Robot::robotDrive.SetGoal(40.0 - kRobotLength / 2.0, 0.0, 0.0);
                autoTimer.Reset();

                state = State::kFinalForward;
            }
            break;
        case State::kFinalForward:
            if (positionProfile.AtGoal() ||
                autoTimer.Get() > positionProfile.ProfileTimeTotal() + 1.0) {
                Robot::intake.Open();
                Robot::robotDrive.Disable();
                Robot::elevator.StopClosedLoop();

                state = State::kIdle;
            }
            break;
        case State::kIdle:
            break;
    }
}
