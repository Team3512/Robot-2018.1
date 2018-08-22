// Copyright (c) 2016-2018 FRC Team 3512. All Rights Reserved.

#include "AutonomousModes/AutoLeftScale.hpp"

#include <DriverStation.h>

#include "Robot.hpp"

AutoLeftScale::AutoLeftScale() { autoTimer.Start(); }

void AutoLeftScale::Reset() { state = State::kInit; }

void AutoLeftScale::HandleEvent(Event event) {
    static std::string platePosition;
    static frc::TrapezoidProfile positionProfile(kRobotMaxV, kRobotTimeToMaxV);
    static frc::TrapezoidProfile angleProfile(kRobotMaxRotateRate,
                                              kRobotTimeToMaxRotateRate);

    switch (state) {
        case State::kInit:
            platePosition =
                frc::DriverStation::GetInstance().GetGameSpecificMessage();

            if (platePosition[kScale] == 'L') {
                positionProfile.SetGoal(324.0 - kRobotLength / 2.0);
                Robot::elevator.SetHeightReference(kScaleHeight);
            } else {
                positionProfile.SetGoal(252.0 - kRobotLength / 2.0);
            }
            angleProfile.SetGoal(0.0);
            Robot::robotDrive.Enable();

            Robot::elevator.StartClosedLoop();

            autoTimer.Reset();

            state = State::kInitialForward;
            break;

        case State::kInitialForward:
            if (positionProfile.AtGoal() ||
                autoTimer.Get() > positionProfile.ProfileTimeTotal() + 1.0) {
                angleProfile.SetGoal(90.0);
                autoTimer.Reset();
                if (platePosition[kScale] == 'L') {
                    state = State::kFinalRotate;
                } else {
                    state = State::kRightRotate;
                }
            }
            break;
        case State::kRightRotate:
            if (angleProfile.AtGoal() ||
                autoTimer.Get() > angleProfile.ProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetEncoders();
                Robot::elevator.SetHeightReference(kScaleHeight);
                positionProfile.SetGoal(199.0);
                autoTimer.Reset();

                state = State::kRightForward;
            }
            break;
        case State::kRightForward:
            if (positionProfile.AtGoal() ||
                autoTimer.Get() > positionProfile.ProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetGyro();
                angleProfile.SetGoal(-90.0);
                autoTimer.Reset();

                state = State::kFinalRotate;
            }
            break;
        case State::kFinalRotate:
            if (angleProfile.AtGoal() ||
                autoTimer.Get() > angleProfile.ProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetEncoders();
                autoTimer.Reset();
                if (platePosition[kScale] == 'L') {
                    /*                    positionProfile.SetGoal(24.0
                       + 6.0 - 6.0 + 24.0 + 12.0 - kRobotLength / 2.0);*/
                    Robot::intake.AutoOuttake();
                    state = State::kIdle;
                } else {
                    positionProfile.SetGoal(56.0 + 3.0 + 18.0 -
                                            kRobotLength / 2.0);
                    state = State::kFinalForward;
                }
            }
            break;
        case State::kFinalForward:
            if (positionProfile.AtGoal() ||
                autoTimer.Get() > positionProfile.ProfileTimeTotal() + 0.1) {
                Robot::intake.AutoOuttake();

                Robot::robotDrive.Disable();
                Robot::elevator.StopClosedLoop();
                state = State::kIdle;
            }
            break;
        case State::kIdle:
            Robot::robotDrive.Disable();
            break;
    }
}
