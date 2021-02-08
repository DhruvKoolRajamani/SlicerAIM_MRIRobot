#pragma once
#include "../Kinematics/NeuroKinematics.hpp"

class ForwardKinematics
{

public:
  ForwardKinematics(NeuroKinematics& NeuroKinematics);

  // members
  double i, j, k, l, ii;  // counter initialization
                          // Min allowed seperation 75mm
  // Max allowed seperation  146mm
  // Is the max allowed movement while one block is stationary 146-75 = 71 mm
  const double Diff;
  const double Diff_largest;
  const double pi;
  double       Ry;       // Initializing the PitchRotation counter
  double       Rx;       // Initializing the YawRotation counter
  const double Rx_max;   // Max YawRotation
  const double RyF_max;  // in paper is 37.2
  const double RyB_max;  // in paper is  30.6
  const double RyF_max_degree;
  const double RyB_max_degree;
  const double Rx_max_degree;
  const double Top_max_travel;
  const double Lateral_translation_start;
  const double Lateral_translation_end;
  const double Lateral_resolution;
  const double Probe_insert_max;
  const double Probe_insert_min;
  int          counter;  // counter for NaN checker
  // Robot axis
  double           AxialHeadTranslation;
  double           AxialFeetTranslation;
  double           LateralTranslation;
  double           PitchRotation;
  double           YawRotation;
  double           ProbeInsertion;
  double           ProbeRotation;
  NeuroKinematics  NeuroKinematics_;
  Eigen::Matrix3Xf rcm_point_set_;

  // methods

  // Method to generate Point cloud of the surface of general reachable
  // Workspace
  Eigen::Matrix3Xf GetGeneralWorkspace();

  // Method to generate Point cloud of the surface of the RCM Workspace
  Eigen::Matrix3Xf GetRcmWorkSpace();

  // Method to generate a point set from the RCM WS.
  Eigen::Matrix3Xf GetRcmPointSet();

  // Method to return a point set based on a given EP.
  Eigen::Matrix3Xf GetSubWorkspace(Eigen::Vector3d ep_in_robot_coordinate);

  void StorePoint(Eigen::Matrix3Xf& rcm_point_cloud,
                  Eigen::Matrix4d transformation_matrix, int counter);

  bool CheckSphere(Eigen::Vector3d ep_in_robot_coordinate,
                   Eigen::Vector3f rcm_point_set);

  Eigen::Matrix3Xf
    GetPointCloudInverseKinematics(Eigen::Matrix3Xf validated_point_set,
                                   Eigen::Vector3d  ep_in_robot_coordinate);

  Eigen::Matrix3Xf GenerateFinalSubworkspacePointset(
    Eigen::Matrix3Xf validated_inverse_kinematic_rcm_pointset,
    Eigen::Vector3d  ep_in_robot_coordinate);

  /* Method which takes a 4X4 transformation matrix and extracts the position
  vector and saves it inside an Eigen matrix*/
  void StorePointToEigenMatrix(Eigen::Matrix3Xf& point_set,
                               Eigen::Matrix4d   transformation_matrix);
  void StorePointToEigenMatrix(Eigen::Matrix3Xf& point_set, double x, double y,
                               double z);

  void CalculateTransform(Eigen::Matrix4d  registration_inv,
                          Eigen::Vector3d  ep_in_imager_coordinate,
                          Eigen::Vector3d& ep_in_robot_coordinate);
};
