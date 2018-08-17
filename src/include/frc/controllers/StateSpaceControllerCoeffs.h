// Copyright (c) 2018 FRC Team 3512. All Rights Reserved.

#pragma once

#include <Eigen/Core>

namespace frc {

/**
 * A container for all the controller coefficients.
 */
template <int States, int Inputs, int Outputs>
struct StateSpaceControllerCoeffs final {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    /**
     * Controller gain matrix.
     */
    const Eigen::Matrix<double, Inputs, States> K;

    /**
     * Controller feedforward gain matrix.
     */
    const Eigen::Matrix<double, Inputs, States> Kff;

    /**
     * Minimum control input.
     */
    const Eigen::Matrix<double, Inputs, 1> Umin;

    /**
     * Maximum control input.
     */
    const Eigen::Matrix<double, Inputs, 1> Umax;

    /**
     * Construct the container for the controller coefficients.
     *
     * @param K    Controller gain matrix.
     * @param Kff  Controller feedforward gain matrix.
     * @param Umin Minimum control input.
     * @param Umax Maximum control input.
     */
    StateSpaceControllerCoeffs(const Eigen::Matrix<double, Inputs, States>& K,
                               const Eigen::Matrix<double, Inputs, States>& Kff,
                               const Eigen::Matrix<double, Inputs, 1>& Umin,
                               const Eigen::Matrix<double, Inputs, 1>& Umax);

    StateSpaceControllerCoeffs(const StateSpaceControllerCoeffs& rhs);
};

}  // namespace frc

#include "frc/controllers/StateSpaceControllerCoeffs.inc"
