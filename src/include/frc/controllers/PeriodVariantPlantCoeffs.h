// Copyright (c) 2018 FRC Team 3512. All Rights Reserved.

#pragma once

#include <Eigen/Core>

namespace frc {

/**
 * A container for all the period-variant plant coefficients.
 */
template <int States, int Inputs, int Outputs>
struct PeriodVariantPlantCoeffs final {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    /**
     * Continuous system matrix.
     */
    const Eigen::Matrix<double, States, States> Acontinuous;

    /**
     * Continuous input matrix.
     */
    const Eigen::Matrix<double, States, Inputs> Bcontinuous;

    /**
     * Output matrix.
     */
    const Eigen::Matrix<double, Outputs, States> C;

    /**
     * Feedthrough matrix.
     */
    const Eigen::Matrix<double, Outputs, Inputs> D;

    /**
     * Construct the container for the period-variant plant coefficients.
     *
     * @param Acontinuous Continuous system matrix.
     * @param Bcontinuous Continuous input matrix.
     * @param C           Output matrix.
     * @param D           Feedthrough matrix.
     */
    PeriodVariantPlantCoeffs(
        const Eigen::Matrix<double, States, States>& Acontinuous,
        const Eigen::Matrix<double, States, Inputs>& Bcontinuous,
        const Eigen::Matrix<double, Outputs, States>& C,
        const Eigen::Matrix<double, Outputs, Inputs>& D);

    PeriodVariantPlantCoeffs(const PeriodVariantPlantCoeffs& rhs);
};

}  // namespace frc

#include "frc/controllers/PeriodVariantPlantCoeffs.inc"
