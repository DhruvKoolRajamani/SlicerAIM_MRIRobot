/**
 * @file NeuroRobotKinematics.hpp
 * @author Dhruv Kool Rajamani (dkoolrajamani@wpi.edu)
 * @brief
 * @version 0.1
 * @date 2021-01-19
 *
 *
 */

#ifndef NEUROROBOTKINEMATICS_HPP_
#define NEUROROBOTKINEMATICS_HPP_

#include <NeuroKinematics/NeuroKinematics.hpp>
#include <eigen3/Eigen/Core>
#include <map>

typedef int                                                        JointIdx;
typedef float                                                      JointValue;
typedef Eigen::Matrix4f                                            TipPosition;
typedef std::map< JointIdx, JointValue >                           JointValues;
typedef std::map< JointIdx, std::tuple< JointValue, JointValue > > JointLimits;

class NeuroRobotKinematics
{
public:
  NeuroRobotKinematics(int solverType = 1);

  TipPosition ForwardKinematics(JointValues);
  JointValues InverseKinematics(TipPosition);

  // Robot Specific Functions
  double GetSphere();

  // Setters
  void SetNumberOfJoints(int joints);
  void SetSolverType(int solverType);

  // Getters
  int GetNumberOfJoints();
  int GetSolverType();

  // Pointer to joint limits
  void SetJointLimits();

private:
  int NumberOfJoints;
  int SolverType;
};

#endif  // NEUROROBOTKINEMATICS_HPP_