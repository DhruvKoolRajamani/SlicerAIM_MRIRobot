/**
 * @file WorkspaceVisualization.h
 * @author Dhruv Kool Rajamani (dkoolrajamani@wpi.edu)
 * @brief
 * @version 0.1
 * @date 2021-01-20
 *
 *
 */

#ifndef WORKSPACEVISUALIZATION_HPP
#define WORKSPACEVISUALIZATION_HPP

#include <NeuroRobotKinematics/NeuroRobotKinematics.hpp>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>

class WorkspaceVisualization
{

public:
  WorkspaceVisualization(NeuroKinematics& NeuroKinematics);

  // members
  double i, j, k, l, ii;  // counter initialization
                          // Min allowed seperation 75mm
  // Max allowed seperation  146mm
  const double Diff;  // Is the max allowed movement while one block is
                      // stationary 146-75 = 71 mm
  const double pi;
  double       Ry;       // Initializing the PitchRotation counter
  double       RyF_max;  // in paper is 37.2
  double       RyB_max;  // in paper is  30.6
  double       Rx;       // Initializing the YawRotation counter
  double       Rx_max;   // Max YawRotation
  double       RyF_max_degree;
  double       RyB_max_degree;
  double       Rx_max_degree;
  int          counter;  // counter for NaN checker
  // Robot axis
  double          AxialHeadTranslation;
  double          AxialFeetTranslation;
  double          LateralTranslation;
  double          PitchRotation;
  double          YawRotation;
  double          ProbeInsertion;
  double          ProbeRotation;
  NeuroKinematics NeuroKinematics_;

  // methods
  vtkSmartPointer< vtkPoints > get_General_Workspace(
    Eigen::Matrix4d registration, vtkSmartPointer< vtkPoints > points);
  vtkSmartPointer< vtkPoints > get_Sub_Workspace(
    Eigen::Matrix4d registration, Eigen::Vector4d entryPointScanner);
  void            nan_checker(Neuro_FK_outputs FK, int& counter);
  Eigen::Vector4d get_Transform(Eigen::Matrix4d  registration_inv,
                                Neuro_FK_outputs FK);
};

#endif  // WORKSPACEVISUALIZATION_HPP