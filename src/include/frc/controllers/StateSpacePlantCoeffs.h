// Copyright (c) 2018 FRC Team 3512. All Rights Reserved.

#pragma once

#include <Eigen/Core>

namespace frc {

/**
 * A container for all the state-space plant coefficients.
 */
template <int States, int Inputs, int Outputs>
struct StateSpacePlantCoeffs final {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    /**
     * System matrix.
     */
    const Eigen::Matrix<double, States, States> A;

    /**
     * Inverse system matrix (A^-1).
     */
    const Eigen::Matrix<double, States, States> Ainv;

    /**
     * Input matrix.
     */
    const Eigen::Matrix<double, States, Inputs> B;

    /**
     * Output matrix.
     */
    const Eigen::Matrix<double, Outputs, States> C;

    /**
     * Feedthrough matrix.
     */
    const Eigen::Matrix<double, Outputs, Inputs> D;

    /**
     * Construct the container for the state-space plant coefficients.
     *
     * @param A    System matrix.
     * @param Ainv Inverse of system matrix.
     * @param B    Input matrix.
     * @param C    Output matrix.
     * @param D    Feedthrough matrix.
     */
    StateSpacePlantCoeffs(const Eigen::Matrix<double, States, States>& A,
                          const Eigen::Matrix<double, States, States>& Ainv,
                          const Eigen::Matrix<double, States, Inputs>& B,
                          const Eigen::Matrix<double, Outputs, States>& C,
                          const Eigen::Matrix<double, Outputs, Inputs>& D);

    StateSpacePlantCoeffs(const StateSpacePlantCoeffs& rhs);
};

}  // namespace frc

#include "frc/controllers/StateSpacePlantCoeffs.inc"
