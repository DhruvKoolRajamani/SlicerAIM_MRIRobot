#pragma once
#include "../Kinematics/NeuroKinematics.hpp"
#include <vtkPoints.h>
#include <vtkSmartPointer.h>

class ForwardKinematics
{

public:
  ForwardKinematics(NeuroKinematics& NeuroKinematics);

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

  // Method to calculate the Point cloud for the suface of the WS
  vtkSmartPointer< vtkPoints >
    GetGeneralWorkspace(vtkSmartPointer< vtkPoints > points);

  // Method to calculate the Point cloud for the suface of the RCM
  vtkSmartPointer< vtkPoints > GetRcmWorkSpace(
    Eigen::Matrix4d registration, vtkSmartPointer< vtkPoints > points);

  // Method to generate a point set from the RCM WS
  Eigen::Matrix3Xf GetRcmPointCloud(Eigen::Matrix4d registration);

  // Method to return a point set based on a given EP
  Eigen::Matrix3Xf GetSubWorkspace(Eigen::Vector3d ep_in_imager_coordinate,
                                   Eigen::Matrix4d registration,
                                   double          probe_init);

  void StorePoints(Eigen::Matrix3Xf& rcm_point_cloud,
                   Eigen::Vector4d& transferred_point, int counter);

  bool CheckSphere(Eigen::Vector3d ep_in_robot_coordinate,
                   Eigen::Vector3f rcm_point, double probe_init);

  Eigen::Matrix3Xf GetPointCloudInverseKinematics(
    Eigen::Matrix3Xf validated_pc, Eigen::Vector3d ep_in_robot_coordinate);

  Eigen::Matrix3Xf Create3DMesh(Eigen::Matrix3Xf subworkspace_rcm,
                                Eigen::Vector3d  ep_in_robot_coordinate);

  Eigen::Vector3d
    ExtractPositionVectorFrom4X4Matrix(Eigen::Matrix4d transformation_matrix);

  void CalcTransform(Eigen::Matrix4d  registration_inv,
                     Eigen::Vector3d  ep_in_imager_coordinate,
                     Eigen::Vector3d& ep_in_robot_coordinate);

  Eigen::Vector4d GetTransform(Eigen::Matrix4d  registration_inv,
                               Neuro_FK_outputs forward_kinematic);
};