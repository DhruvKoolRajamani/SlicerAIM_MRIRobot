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
  vtkSmartPointer< vtkPoints > get_General_Workspace(
    Eigen::Matrix4d registration, vtkSmartPointer< vtkPoints > points);
  vtkSmartPointer< vtkPoints > get_RCM_Workspace(
    Eigen::Matrix4d registration, vtkSmartPointer< vtkPoints > points);
  Eigen::Matrix3Xf get_RCM_PC(Eigen::Matrix4d registration);
  Eigen::Matrix3Xf get_SubWorkspace(Eigen::Matrix3Xf RCM_PC,
                                    Eigen::Vector3d  EP_inImagerCoordinate,
                                    Eigen::Matrix4d  registration,
                                    double           probe_init);
  Eigen::Vector4d  get_Transform(Eigen::Matrix4d  registration_inv,
                                 Neuro_FK_outputs FK);
  void             calc_Transform(Eigen::Matrix4d  registration_inv,
                                  Eigen::Vector3d  EP_inImagerCoordinate,
                                  Eigen::Vector3d& EP_inRobotCoordinate);
  void             store_Point(Eigen::Matrix3Xf& RCM_Point_cloud,
                               Eigen::Vector4d& transferred_Point, int counter);
  bool             check_Sphere(Eigen::Vector3d EP_inRobotCoordinate,
                                Eigen::Vector3f RCM_point, double probe_init);
  Eigen::Matrix3Xf get_PointCloud_IK(Eigen::Matrix3Xf Validated_PC,
                                     Eigen::Vector3d  EP_inRobotCoordinate);
  Eigen::Matrix3Xf create_3D_Mesh(Eigen::Matrix3Xf Sub_Workspace_RCM,
                                  Eigen::Vector3d  EP_inRobotCoordinate);
};
