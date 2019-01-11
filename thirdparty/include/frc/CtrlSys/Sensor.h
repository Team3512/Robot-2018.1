/*----------------------------------------------------------------------------*/
/* Copyright (c) 2017-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#include <frc/PIDSource.h>

#include "frc/CtrlSys/INode.h"

namespace frc {

/**
 * INode adapter for PIDSource subclasses.
 *
 * Wraps a PIDSource object in the INode interface by returning the output of
 * PIDGet() from GetOutput().
 */
class Sensor : public INode {
 public:
  Sensor(PIDSource& source);  // NOLINT
  virtual ~Sensor() = default;

  double GetOutput() override;

 private:
  PIDSource& m_source;
};

}  // namespace frc
