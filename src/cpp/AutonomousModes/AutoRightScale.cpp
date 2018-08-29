// Copyright (c) 2016-2018 FRC Team 3512. All Rights Reserved.

#include "AutonomousModes/AutoRightScale.hpp"

#include <DriverStation.h>

#include "Robot.hpp"

AutoRightScale::AutoRightScale() { autoTimer.Start(); }

void AutoRightScale::Reset() { state = State::kInit; }

void AutoRightScale::HandleEvent(Event event) {
    static std::string platePosition;

    switch (state) {
        case State::kInit:
            platePosition =
                frc::DriverStation::GetInstance().GetGameSpecificMessage();

            if (platePosition[kScale] == 'R') {
                Robot::robotDrive.SetPositionGoal(324.0 - kRobotLength / 2.0);
                Robot::elevator.SetReferences(kScaleHeight, 0.0);
            } else {
                Robot::robotDrive.SetPositionGoal(236.5 + 10.0 -
                                                  kRobotLength / 2.0);
            }
            Robot::robotDrive.SetAngleGoal(0.0);
            Robot::robotDrive.StartClosedLoop();

            Robot::elevator.Disable();

            autoTimer.Reset();

            state = State::kInitialForward;
            break;

        case State::kInitialForward:
            if (Robot::robotDrive.AtPositionGoal() ||
                autoTimer.Get() >
                    Robot::robotDrive.PositionProfileTimeTotal() + 1.0) {
                Robot::robotDrive.SetAngleGoal(-88.0);
                autoTimer.Reset();
                if (platePosition[kScale] == 'R') {
                    state = State::kFinalRotate;
                } else {
                    state = State::kLeftRotate;
                }
            }
            break;
        case State::kLeftRotate:
            if (Robot::robotDrive.AtAngleGoal() ||
                autoTimer.Get() >
                    Robot::robotDrive.AngleProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetEncoders();
                Robot::robotDrive.SetPositionGoal(199.0);
                Robot::elevator.SetReferences(kScaleHeight, 0.0);
                autoTimer.Reset();

                state = State::kLeftForward;
            }
            break;
        case State::kLeftForward:
            if (Robot::robotDrive.AtPositionGoal() ||
                autoTimer.Get() >
                    Robot::robotDrive.PositionProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetGyro();
                Robot::robotDrive.SetAngleGoal(90.0);
                autoTimer.Reset();

                state = State::kFinalRotate;
            }
            break;
        case State::kFinalRotate:
            if (Robot::robotDrive.AtAngleGoal() ||
                autoTimer.Get() >
                    Robot::robotDrive.AngleProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetEncoders();
                if (platePosition[kScale] == 'R') {
                    /*Robot::robotDrive.SetPositionGoal(24.0 + 24.0 -
                                                      kRobotLength / 2.0);*/
                    Robot::robotDrive.SetPositionGoal(10.0);
                } else {
                    Robot::robotDrive.SetPositionGoal(56.0 + 22.0 -
                                                      kRobotLength / 2.0);
                }

                state = State::kFinalForward;
                autoTimer.Reset();
            }
            break;
        case State::kFinalForward:
            if (Robot::robotDrive.AtPositionGoal() ||
                autoTimer.Get() >
                    Robot::robotDrive.PositionProfileTimeTotal() + 1.0) {
                Robot::intake.AutoOuttake();

                Robot::robotDrive.StopClosedLoop();
                Robot::elevator.Disable();
                state = State::kIdle;
                // state = State::kPrepReverse;
            }
            break;
        case State::kPrepReverse:
            if (autoTimer.HasPeriodPassed(1.0)) {
                if (platePosition[kScale] == 'R') {
                    Robot::robotDrive.SetPositionGoal(-24.0 - 33.0 +
                                                      kRobotLength / 2.0);
                } else {
                    Robot::robotDrive.SetPositionGoal(-56.0 - 9.0 - 12.0 +
                                                      kRobotLength / 2.0);
                }
                autoTimer.Reset();
                state = State::kPrepRotate;
            }
        case State::kPrepRotate:
            if (Robot::robotDrive.AtPositionGoal() ||
                autoTimer.Get() >
                    Robot::robotDrive.AngleProfileTimeTotal() + 1.0) {
                Robot::robotDrive.SetAngleGoal(-90.0);
                Robot::elevator.SetReferences(0.0, 0.0);
                autoTimer.Reset();
                state = State::kIdle;
            }
        case State::kIdle:
            Robot::robotDrive.StopClosedLoop();
            break;
    }
}
