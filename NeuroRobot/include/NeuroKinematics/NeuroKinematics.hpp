//============================================================================
// Name        : NeuroKinematics.hpp
// Author      : Produced in the WPI AIM Lab
// Description : This file is where the custom forward and inverse kinematics
//				 code for the robot resides
//============================================================================

#ifndef NEUROKINEMATICS_HPP_
#define NEUROKINEMATICS_HPP_

#include <iostream>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <eigen3/Eigen/Dense>

// TODO : Rename Prostate FK and IK structs following this convention
struct Neuro_FK_outputs
{
  Eigen::Matrix4d zFrameToTreatment;
};

struct Neuro_IK_outputs
{
  Eigen::Matrix4d targetPose;
  double          AxialFeetTranslation;
  double          AxialHeadTranslation;
  double          LateralTranslation;
  double          ProbeInsertion;
  double          ProbeRotation;
  double          YawRotation;
  double          PitchRotation;
};
struct IK_Solver_outputs
{
  double AxialFeetTranslation;
  double AxialHeadTranslation;
  double LateralTranslation;
};

struct Probe
{
  double _cannulaToTreatment;
  double _treatmentToTip;
  double _robotToEntry;
  double _robotToTreatmentAtHome;

  /**
   * @brief Construct a new Probe object
   *
   * @param double cTT _cannulaToTreatment
   * @param double tTT _treatmentToTip
   * @param double rTE _robotToEntry
   * @param double rTTTAH _robotToTreatmentAtHome
   */
  Probe(double cTT, double tTT, double rTE, double rTTTAH)
    : _cannulaToTreatment(cTT)
    , _treatmentToTip(tTT)
    , _robotToEntry(rTE)
    , _robotToTreatmentAtHome(rTTTAH)
  {
  }

  /**
   * @brief Construct a new DEFAULT Probe object
   *
   */
  Probe()
    : _cannulaToTreatment(0.0)
    , _treatmentToTip(0.0)
    , _robotToEntry(0.0)
    , _robotToTreatmentAtHome(0.0)
  {
  }
};

struct ProbeSpecifications
{
  double A;
  double B;
  double C;
  double D;

  bool Default = false;

  /**
   * @brief Construct a new Probe Specifications object
   *
   * @param double a _treatmentToTip
   * @param double b _robotToEntry
   * @param double c _cannulaToTreatment
   * @param double d _robotToTreatmentAtHome
   * @param bool   def Is this the default Probe
   */
  ProbeSpecifications(double a, double b, double c, double d, bool def = false)
    : A(a), B(b), C(c), D(d), Default(def)
  {
  }

  /**
   * @brief Construct a new DEFAULT Probe Specifications object
   *
   */
  ProbeSpecifications() : A(0.0), B(0.0), C(0.0), D(0.0), Default(false)
  {
  }

  Probe convertToProbe()
  {
    Probe probe;
    probe._treatmentToTip         = A;
    probe._robotToEntry           = B;
    probe._cannulaToTreatment     = C;
    probe._robotToTreatmentAtHome = D;

    return probe;
  }

  static ProbeSpecifications convertToProbeSpecifications(Probe probe)
  {
    ProbeSpecifications ps;
    ps.A       = probe._treatmentToTip;
    ps.B       = probe._robotToEntry;
    ps.C       = probe._cannulaToTreatment;
    ps.D       = probe._robotToTreatmentAtHome;
    ps.Default = false;

    return ps;
  }
};

inline bool operator==(ProbeSpecifications const& lhs,
                       ProbeSpecifications const& rhs)
{
  if ((lhs.A == rhs.A) && (lhs.B == rhs.B) && (lhs.C == rhs.C) &&
      (lhs.D == rhs.D))
    return true;
  return false;
}

inline bool operator!=(ProbeSpecifications const& lhs,
                       ProbeSpecifications const& rhs)
{
  if (lhs == rhs)
    return false;
  return true;
}

class NeuroKinematics
{

public:
  //================ Constructor ================
  NeuroKinematics();
  NeuroKinematics(Probe* probe);  /// This value comes from the robot probe
  // NeuroKinematics(Probe probe); /// This value comes from the robot probe

  //================ Parameters =================
  // Robot Specific Parameters
  double _lengthOfAxialTrapezoidSideLink;
  double _initialAxialSeperation;
  double _widthTrapezoidTop;
  double _xInitialRCM;
  double _yInitialRCM;
  double _zInitialRCM;
  double _robotToRCMOffset;

  Probe*          _probe;  // Object that stores probe specific configurations
  Eigen::Matrix4d _zFrameToRCM;  // Transformation that accounts for change in
                                 // rotation between zFrame and RCM

  // Parameters used in IK and FK calculations are moved out of their functions
  // to keep these methods running fast by eliminating the need to re-allocate
  // memory every time
  Eigen::Matrix3d xRotationDueToYawRotationFK;
  Eigen::Matrix3d yRotationDueToPitchRotationFK;
  Eigen::Matrix3d zRotationDueToProbeRotationFK;
  Eigen::Matrix4d zFrameToRCMRotation;
  Eigen::Matrix4d zFrameToRCMPrime;
  Eigen::Matrix4d RCMToTreatment;
  Eigen::Matrix4d RCMToEntryPoint;

  Eigen::Matrix3d xRotationDueToYawRotationIK;
  Eigen::Matrix3d yRotationDueToPitchRotationIK;
  Eigen::Matrix3d zRotationDueToProbeRotationIK;
  Eigen::Matrix4d zFrameToTargetPointFinal;

  //================ Public Methods ==============
  // Method to calculate the location of the treatment w.r.t Z-frame
  Neuro_FK_outputs ForwardKinematics(double AxialHeadTranslation,
                                     double AxialFeetTranslation,
                                     double LateralTranslation,
                                     double ProbeInsertion,
                                     double ProbeRotation, double PitchRotation,
                                     double YawRotation);

  // Forward kinematic that returns entry point location instead of treatment
  Neuro_FK_outputs ForwardKinematics_EntryPoint(
    double AxialHeadTranslation, double AxialFeetTranslation,
    double LateralTranslation, double ProbeInsertion, double ProbeRotation,
    double PitchRotation, double YawRotation);

  // Method for the calculation of the location of the RCM w.r.t Z-frame
  Neuro_FK_outputs GetRcm(double AxialHeadTranslation,
                          double AxialFeetTranslation,
                          double LateralTranslation, double ProbeInsertion,
                          double ProbeRotation, double PitchRotation,
                          double YawRotation);
  // Method to calculate joint values given a desired EP and TP
  Neuro_IK_outputs InverseKinematics(Eigen::Vector4d entryPointzFrame,
                                     Eigen::Vector4d targetPointzFrame);

  // IK Method for calculation of the cartesian base based on a given Entry
  // point and an RCM point as the target point
  Neuro_IK_outputs InverseKinematicsWithZeroProbeInsertion(
    Eigen::Vector4d entry_point, Eigen::Vector4d target_point);
};

#endif /* NEUROKINEMATICS_HPP_ */
