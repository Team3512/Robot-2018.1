// Copyright (c) 2016-2018 FRC Team 3512. All Rights Reserved.

#include "AutonomousModes/AutoRightDouble.hpp"

#include <DriverStation.h>

#include "Robot.hpp"

AutoRightDouble::AutoRightDouble() { autoTimer.Start(); }

void AutoRightDouble::Reset() { state = State::kInit; }

void AutoRightDouble::HandleEvent(Event event) {
    static std::string platePosition;
    static frc::TrapezoidProfile positionProfile;
    static frc::TrapezoidProfile angleProfile;

    switch (state) {
        case State::kInit:
            platePosition =
                frc::DriverStation::GetInstance().GetGameSpecificMessage();

            positionProfile.SetGoal(236.5 - kRobotLength / 2.0);
            angleProfile.SetGoal(0.0);
            Robot::robotDrive.Enable();

            Robot::elevator.SetHeightReference(kScaleHeight);
            Robot::elevator.StartClosedLoop();

            autoTimer.Reset();

            state = State::kInitialForward;
            break;

        case State::kInitialForward:
            if (positionProfile.AtGoal() ||
                autoTimer.Get() >
                    positionProfile.ProfileTimeTotal() + 1.0) {
                angleProfile.SetGoal(-90.0);
                autoTimer.Reset();

                state = State::kRightRotate;
            }
            break;
        case State::kRightRotate:
            if (angleProfile.AtGoal() ||
                autoTimer.Get() >
                    angleProfile.ProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetEncoders();
                autoTimer.Reset();
                if (platePosition[kFriendlySwitch] == 'R' &&
                    platePosition[kScale] == 'R') {
                    positionProfile.SetGoal(20.0 -
                                                      kRobotLength / 2.0);
                } else if (platePosition[kFriendlySwitch] == 'R' &&
                           platePosition[kScale] == 'L') {
                    positionProfile.SetGoal(20.0 -
                                                      kRobotLength / 2.0);

                } else {
                    positionProfile.SetGoal(236.5 -
                                                      kRobotLength / 2.0);
                }

                state = State::kFirstForward;
            }
            break;
        case State::kFirstForward:
            if (positionProfile.AtGoal() ||
                autoTimer.Get() >
                    positionProfile.ProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetGyro();
                autoTimer.Reset();
                if (platePosition[kFriendlySwitch] == 'R' &&
                    platePosition[kScale] == 'R') {
                    angleProfile.SetGoal(90);
                } else {
                    angleProfile.SetGoal(-90);
                }

                state = State::kFinalRotate;
            }
            break;
        case State::kFinalRotate:
            if (angleProfile.AtGoal() ||
                autoTimer.Get() >
                    angleProfile.ProfileTimeTotal() + 1.0) {
                Robot::robotDrive.ResetEncoders();
                positionProfile.SetGoal(40.0 - kRobotLength /
                                                             2.0);  // ESTIMATE
                autoTimer.Reset();

                state = State::kFinalForward;
            }
            break;
        case State::kFinalForward:
            if (positionProfile.AtGoal() ||
                autoTimer.Get() >
                    positionProfile.ProfileTimeTotal() + 1.0) {
                Robot::intake.SetMotors(MotorState::kOuttake);
                Robot::robotDrive.ResetGyro();
                autoTimer.Reset();

                if (platePosition[kScale] == 'R' &&
                    platePosition[kFriendlySwitch] == 'R') {
                    angleProfile.SetGoal(-180.0);
                } else {
                    angleProfile.SetGoal(0.0);
                }

                state = State::kDoubleRotate;
            }
            break;
        case State::kDoubleRotate:
            if (angleProfile.AtGoal() ||
                autoTimer.Get() >
                    angleProfile.ProfileTimeTotal() + 1.0) {
                Robot::intake.SetMotors(MotorState::kIdle);
                Robot::intake.Open();
                Robot::elevator.SetHeightReference(kFloorHeight);
                Robot::robotDrive.ResetEncoders();
                if (platePosition[kScale] == 'R' &&
                    platePosition[kFriendlySwitch] == 'R') {
                    positionProfile.SetGoal(60.0);
                } else {
                    positionProfile.SetGoal(10.0);
                }
                autoTimer.Reset();

                state = State::kDoubleForward;
            }
            break;
        case State::kDoubleForward:
            if (positionProfile.AtGoal() ||
                autoTimer.Get() >
                    positionProfile.ProfileTimeTotal() + 1.0) {
                Robot::intake.Close();
                Robot::elevator.SetHeightReference(kSwitchHeight);

                state = State::kSpit;
            }
            break;
        case State::kSpit:
            if (Robot::elevator.HeightAtReference() &&
                autoTimer.HasPeriodPassed(3.0)) {
                Robot::intake.SetMotors(MotorState::kOuttake);

                Robot::robotDrive.Disable();
                Robot::elevator.StopClosedLoop();

                state = State::kIdle;
            }
            break;
        case State::kIdle:
            break;
    }
}
