#pragma once

class Pose {
public:
  double x = 0.0;
  double y = 0.0;
  double theta = 0.0;

  Pose() = default;
  Pose(double x, double y, double theta);

  Pose(const Pose&) = default;
  Pose& operator=(const Pose&) = default;
  Pose(Pose&&) noexcept = default;
  Pose& operator=(Pose&&) noexcept = default;
};
