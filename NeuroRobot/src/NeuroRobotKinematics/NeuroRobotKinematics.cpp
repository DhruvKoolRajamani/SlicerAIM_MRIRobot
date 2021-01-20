/**
 * @file NeuroRobotKinematics.cpp
 * @author Dhruv Kool Rajamani (dkoolrajamani@wpi.edu)
 * @brief
 * @version 0.1
 * @date 2021-01-19
 *
 *
 */

#include <NeuroRobotKinematics/NeuroRobotKinematics.hpp>

NeuroRobotKinematics::NeuroRobotKinematics(int solverType)
{
  SolverType = solverType;
}

TipPosition NeuroRobotKinematics::ForwardKinematics(JointValues jointValues)
{
  NeuroKinematics neuro(/** Probe Specs */);

  TipPosition      tipPosition;
  Neuro_FK_outputs outputs;
  // outputs  = neuro.ForwardKinematics();
  // position = outputs;

  return tipPosition;
}

JointValues NeuroRobotKinematics::InverseKinematics(TipPosition tipPosition)
{
  JointValues jointValues;
  jointValues[0] = 0.0;

  JointLimits jointLimits;
  jointLimits[0] = std::make_tuple(-0.5, 0.5);

  float lower_limit = std::get< 0 >(jointLimits[0]);
  float upper_limit = std::get< 1 >(jointLimits[0]);

  // NeuroKinematics nk;

  // // Function pointer
  // std::function< IK_Solver_outputs > inverse_kinematics;

  // switch (SolverType)
  // {
  //   case 1:
  //     inverse_kinematics = nk.IK_solver();
  //     break;
  //   default:
  //     throw std::exception("Error");
  //     break;
  // }

  // Neuro_FK_outputs outputs;
  // outputs  = inverse_kinematics(tipPosition);
  // position = outputs;

  return jointValues;
}