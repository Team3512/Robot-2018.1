// Copyright (c) 2016-2019 FRC Team 3512. All Rights Reserved.

#include "AutonomousModes/AutoRightScale.hpp"

#include <frc/DriverStation.h>

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
                Robot::robotDrive.SetGoal(
                    Pose(324.0 - kRobotLength / 2.0, 0.0, 0.0));
                Robot::elevator.SetHeightReference(kScaleHeight);
            } else {
                Robot::robotDrive.SetGoal(
                    Pose(236.5 + 10.0 - kRobotLength / 2.0, 0.0, 0.0));
            }
            Robot::robotDrive.Enable();

            Robot::elevator.StartClosedLoop();

            autoTimer.Reset();

            state = State::kInitialForward;
            break;

        case State::kInitialForward:
            if (Robot::robotDrive.AtGoal()) {
                Robot::robotDrive.SetGoal(Pose(0.0, 0.0, -88.0));
                autoTimer.Reset();
                if (platePosition[kScale] == 'R') {
                    state = State::kFinalRotate;
                } else {
                    state = State::kLeftRotate;
                }
            }
            break;
        case State::kLeftRotate:
            if (Robot::robotDrive.AtGoal()) {
                Robot::robotDrive.ResetEncoders();
                Robot::robotDrive.SetGoal(Pose(199.0, 0.0, 0.0));
                Robot::elevator.SetHeightReference(kScaleHeight);
                autoTimer.Reset();

                state = State::kLeftForward;
            }
            break;
        case State::kLeftForward:
            if (Robot::robotDrive.AtGoal()) {
                Robot::robotDrive.SetGoal(Pose(0.0, 0.0, 90.0));
                autoTimer.Reset();

                state = State::kFinalRotate;
            }
            break;
        case State::kFinalRotate:
            if (Robot::robotDrive.AtGoal()) {
                Robot::robotDrive.ResetEncoders();
                if (platePosition[kScale] == 'R') {
                    /*Robot::robotDrive.SetGoal({24.0 + 24.0 -
                                                kRobotLength / 2.0, 0.0,
                       0.0});*/
                    Robot::robotDrive.SetGoal(Pose(10.0, 0.0, 0.0));
                } else {
                    Robot::robotDrive.SetGoal(
                        Pose(56.0 + 22.0 - kRobotLength / 2.0, 0.0, 0.0));
                }

                state = State::kFinalForward;
                autoTimer.Reset();
            }
            break;
        case State::kFinalForward:
            if (Robot::robotDrive.AtGoal()) {
                Robot::intake.AutoOuttake();

                Robot::robotDrive.Disable();
                Robot::elevator.StopClosedLoop();
                state = State::kIdle;
                // state = State::kPrepReverse;
            }
            break;
        case State::kPrepReverse:
            if (autoTimer.HasPeriodPassed(1.0)) {
                if (platePosition[kScale] == 'R') {
                    Robot::robotDrive.SetGoal(
                        Pose(-24.0 - 33.0 + kRobotLength / 2.0, 0.0, 0.0));
                } else {
                    Robot::robotDrive.SetGoal(Pose(
                        -56.0 - 9.0 - 12.0 + kRobotLength / 2.0, 0.0, 0.0));
                }
                autoTimer.Reset();
                state = State::kPrepRotate;
            }
        case State::kPrepRotate:
            if (Robot::robotDrive.AtGoal()) {
                Robot::robotDrive.SetGoal(Pose(0.0, 0.0, -90.0));
                Robot::elevator.SetHeightReference(0.0);
                autoTimer.Reset();
                state = State::kIdle;
            }
        case State::kIdle:
            Robot::robotDrive.Disable();
            break;
    }
}
