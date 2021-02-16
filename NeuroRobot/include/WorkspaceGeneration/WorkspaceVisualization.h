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
  // Max allowed movement while one block is stationary 146-75 = 71 mm

  // Maximum allowed movement for legs from their widest seperation to
  // narrowest seperation
  const double max_leg_displacement_;
  // Max position for the axial head relative to its frame
  const double axial_head_upper_bound_;
  // Min position for the axial head relative to its frame
  const double axial_head_lower_bound_;
  // Max position for the axial feet relative to its frame
  const double axial_feet_upper_bound_;
  // Min position for the axial feet relative to its frame
  const double axial_feet_lower_bound_;
  // Closest that the two legs can be from each other
  const double min_leg_seperation;
  const double pi;
  double       Ry;       // Initializing the PitchRotation counter
  double       Rx;       // Initializing the YawRotation counter
  const double Rx_max;   // Max YawRotation
  const double RyF_max;  // in paper is 37.2
  const double RyB_max;  // in paper is  30.6
  const double RyF_max_degree;
  const double RyB_max_degree;
  const double Rx_max_degree;
  // Maximum allowed movement for minimum leg seperation(top of WS)
  const double Top_max_travel;
  // Maximum allowed movement for maximum leg seperation(bottom of WS)
  const double Bottom_max_travel;
  const double Lateral_translation_start;
  const double Lateral_translation_end;
  const double Probe_insert_max;
  const double Probe_insert_min;
  const double axial_resolution_;
  const double Lateral_resolution;
  const double desired_resolution;
  const double pitch_resolution_;
  const double yaw_resolution;
  const double probe_insertion_resolution;
  const double desired_resolution_general_ws;
  int          counter;
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
                                   Eigen::Vector3d  ep_in_robot_coordinate,
                                   Eigen::VectorXd& treatment_to_tp_dist);

  Eigen::Matrix3Xf GenerateFinalSubworkspacePointset(
    Eigen::Matrix3Xf validated_inverse_kinematic_rcm_pointset,
    Eigen::Vector3d  ep_in_robot_coordinate,
    Eigen::VectorXd& treatment_to_tp_dist);

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
