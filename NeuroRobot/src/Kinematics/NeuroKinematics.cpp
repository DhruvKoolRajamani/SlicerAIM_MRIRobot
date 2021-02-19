//============================================================================
// Name        : NeuroKinematics.hpp
// Author      : Produced in the WPI AIM Lab
// Description : This file is where the custom forward and inverse kinematics
//				 code for the NeuroRobot resides
//============================================================================

#include "../../include/Kinematics/NeuroKinematics.hpp"

NeuroKinematics::NeuroKinematics()
{
  _lengthOfAxialTrapezoidSideLink = 0;
  _widthTrapezoidTop              = 0;
  _initialAxialSeperation         = 0;
  _xInitialRCM                    = 0;
  _yInitialRCM                    = 0;
  _zInitialRCM                    = 0;
  _robotToRCMOffset               = 0;

  // Object that stores probe specific configurations
  _probe = NULL;

  // Transformation that accounts for change in rotation between zFrame and RCM
  _zFrameToRCM = Eigen::Matrix4d::Identity();

  // Optimizations
  xRotationDueToYawRotationFK   = Eigen::Matrix3d::Identity();
  yRotationDueToPitchRotationFK = Eigen::Matrix3d::Identity();
  zRotationDueToProbeRotationFK = Eigen::Matrix3d::Identity();
  zFrameToRCMRotation           = Eigen::Matrix4d::Identity();
  zFrameToRCMPrime              = Eigen::Matrix4d::Identity();
  RCMToTreatment                = Eigen::Matrix4d::Identity();
  xRotationDueToYawRotationIK   = Eigen::Matrix3d::Identity();
  RCMToEntryPoint               = Eigen::Matrix4d::Identity();
  yRotationDueToPitchRotationIK = Eigen::Matrix3d::Identity();
  zRotationDueToProbeRotationIK = Eigen::Matrix3d::Identity();
  zFrameToTargetPointFinal      = Eigen::Matrix4d::Identity();
}

NeuroKinematics::NeuroKinematics(Probe* probe)
{
  // All values are in units of mm
  _lengthOfAxialTrapezoidSideLink = 60;   // L1 link
  _widthTrapezoidTop              = 30;   // L2 Link
  _initialAxialSeperation         = 143;  // delta_Zh in the formula
  _xInitialRCM                    = 18.32;
  _yInitialRCM                    = 178.39;
  _zInitialRCM                    = 72.91;
  _robotToRCMOffset               = 72.5;

  // Object that stores probe specific configurations
  _probe = probe;

  // Transformation that accounts for change in rotation between zFrame and RCM
  _zFrameToRCM << -1, 0, 0, 0, 0, 0, -1, 0, 0, -1, 0, 0, 0, 0, 0, 1;

  // Optimizations
  // These are the values for the RCM shown in the paper
  xRotationDueToYawRotationFK   = Eigen::Matrix3d::Identity();
  yRotationDueToPitchRotationFK = Eigen::Matrix3d::Identity();
  zRotationDueToProbeRotationFK = Eigen::Matrix3d::Identity();
  // Rotation from the fixed z-frame to the RCM point
  zFrameToRCMRotation = Eigen::Matrix4d::Identity();
  // What is the RCM prime?
  zFrameToRCMPrime = Eigen::Matrix4d::Identity();
  // RCM to the tooltip (probe's tip)
  RCMToTreatment = Eigen::Matrix4d::Identity();

  // IK related attributes
  xRotationDueToYawRotationIK   = Eigen::Matrix3d::Identity();
  yRotationDueToPitchRotationIK = Eigen::Matrix3d::Identity();
  zRotationDueToProbeRotationIK = Eigen::Matrix3d::Identity();
  zFrameToTargetPointFinal      = Eigen::Matrix4d::Identity();
}

// This method defines the forward kinematics for the neurosurgery robot.
// Returns(Neuro_FK_output): A 4x4 transformation from the robot base to the
// robot treatment zone Note: All rotations should be given in units of radians
/* Joint variables of the robot
   1) AxialHeadTranslation (corresponds to the delta_z_sup)
   2) AxialFeetTranslation (corresponds to the delta_z_inf )
   3) LateralTranslation (ranging from -37.5 to 0 mm)
   the first three degree of freedom of the robot positions the robot's RCM
   point at a target location 4) PitchRotation (Ry ranging from -37.2 to 30.6
   deg) 5) YawRotation (Rx ranging from -90deg to 0deg) 4 and 5 positions the
   needle or the probe to pass through the burr hole 6) ProbeRotation (Rz
   continuous deg) 7) ProbeInsertion (PI ranging from -40 to 0 mm)
   */
Neuro_FK_outputs NeuroKinematics::ForwardKinematics(
  double AxialHeadTranslation, double AxialFeetTranslation,
  double LateralTranslation, double ProbeInsertion, double ProbeRotation,
  double PitchRotation, double YawRotation)
{
  // Structure to return with the FK output( struct can be remove )
  struct Neuro_FK_outputs FK;

  // Z position of RCM is solely defined as the midpoint of the axial trapezoid
  double axialTrapezoidMidpoint =
    (AxialHeadTranslation - AxialFeetTranslation + _initialAxialSeperation) /
    2;  // initial position
  double zDeltaRCM = (AxialFeetTranslation + AxialHeadTranslation) / 2;

  // Y position of RCM is found by pythagorean theorem of the axial trapezoid
  double yTrapezoidHypotenuseSquared = pow(_lengthOfAxialTrapezoidSideLink, 2);
  double yTrapezoidSideSquared =
    pow((axialTrapezoidMidpoint - _widthTrapezoidTop / 2), 2);
  double yTrapezoidInitialSeparationSquared =
    pow((_initialAxialSeperation - _widthTrapezoidTop) / 2, 2);
  double yDeltaRCM =
    sqrt(yTrapezoidHypotenuseSquared - yTrapezoidSideSquared) -
    sqrt(yTrapezoidHypotenuseSquared - yTrapezoidInitialSeparationSquared);

  // X position of RCM is solely defined as the amount traveled in lateral
  // translation
  double xDeltaRCM = LateralTranslation;

  // Obtain basic Yaw, Pitch, and Roll Rotations
  xRotationDueToYawRotationFK << 1, 0, 0, 0, cos(YawRotation),
    -sin(YawRotation), 0, sin(YawRotation), cos(YawRotation);

  yRotationDueToPitchRotationFK << cos(PitchRotation), 0, sin(PitchRotation), 0,
    1, 0, -sin(PitchRotation), 0, cos(PitchRotation);

  zRotationDueToProbeRotationFK << cos(ProbeRotation), -sin(ProbeRotation), 0,
    sin(ProbeRotation), cos(ProbeRotation), 0, 0, 0, 1;

  // Calculate the XYZ Rotation
  zFrameToRCMRotation.block(0, 0, 3, 3) =
    (xRotationDueToYawRotationFK * yRotationDueToPitchRotationFK *
     zRotationDueToProbeRotationFK)
      .block(0, 0, 3, 3);

  // Calculate the XYZ Translation
  zFrameToRCMPrime << -1, 0, 0, _xInitialRCM + xDeltaRCM, 0, 0, -1,
    _yInitialRCM + yDeltaRCM, 0, -1, 0, _zInitialRCM + zDeltaRCM, 0, 0, 0, 1;

  // Now Calculate zFrame to RCM given the calculated values above
  Eigen::Matrix4d zFrameToRCM = zFrameToRCMPrime * zFrameToRCMRotation;

  // Create RCM to Treatment Matrix
  RCMToTreatment(2, 3) =
    ProbeInsertion + _probe->_robotToTreatmentAtHome - _robotToRCMOffset;

  // Finally calculate Base to Treatment zone using the measured transformation
  // for RCM to Treatment
  FK.zFrameToTreatment = zFrameToRCM * RCMToTreatment;

  return FK;
}
Neuro_FK_outputs NeuroKinematics::ForwardKinematics_EntryPoint(
  double AxialHeadTranslation, double AxialFeetTranslation,
  double LateralTranslation, double ProbeInsertion, double ProbeRotation,
  double PitchRotation, double YawRotation)
{
  // Structure to return with the FK output( struct can be remove )
  struct Neuro_FK_outputs FK;

  // Z position of RCM is solely defined as the midpoint of the axial trapezoid
  double axialTrapezoidMidpoint =
    (AxialHeadTranslation - AxialFeetTranslation + _initialAxialSeperation) /
    2;  // initial position
  double zDeltaRCM = (AxialFeetTranslation + AxialHeadTranslation) / 2;

  // Y position of RCM is found by pythagorean theorem of the axial trapezoid
  double yTrapezoidHypotenuseSquared = pow(_lengthOfAxialTrapezoidSideLink, 2);
  double yTrapezoidSideSquared =
    pow((axialTrapezoidMidpoint - _widthTrapezoidTop / 2), 2);
  double yTrapezoidInitialSeparationSquared =
    pow((_initialAxialSeperation - _widthTrapezoidTop) / 2, 2);
  double yDeltaRCM =
    sqrt(yTrapezoidHypotenuseSquared - yTrapezoidSideSquared) -
    sqrt(yTrapezoidHypotenuseSquared - yTrapezoidInitialSeparationSquared);

  // X position of RCM is solely defined as the amount traveled in lateral
  // translation
  double xDeltaRCM = LateralTranslation;

  // Obtain basic Yaw, Pitch, and Roll Rotations
  xRotationDueToYawRotationFK << 1, 0, 0, 0, cos(YawRotation),
    -sin(YawRotation), 0, sin(YawRotation), cos(YawRotation);

  yRotationDueToPitchRotationFK << cos(PitchRotation), 0, sin(PitchRotation), 0,
    1, 0, -sin(PitchRotation), 0, cos(PitchRotation);

  zRotationDueToProbeRotationFK << cos(ProbeRotation), -sin(ProbeRotation), 0,
    sin(ProbeRotation), cos(ProbeRotation), 0, 0, 0, 1;

  // Calculate the XYZ Rotation
  zFrameToRCMRotation.block(0, 0, 3, 3) =
    (xRotationDueToYawRotationFK * yRotationDueToPitchRotationFK *
     zRotationDueToProbeRotationFK)
      .block(0, 0, 3, 3);

  // Calculate the XYZ Translation
  zFrameToRCMPrime << -1, 0, 0, _xInitialRCM + xDeltaRCM, 0, 0, -1,
    _yInitialRCM + yDeltaRCM, 0, -1, 0, _zInitialRCM + zDeltaRCM, 0, 0, 0, 1;

  // Now Calculate zFrame to RCM given the calculated values above
  Eigen::Matrix4d zFrameToRCM = zFrameToRCMPrime * zFrameToRCMRotation;

  // Create RCM to Treatment Matrix
  RCMToEntryPoint(2, 3) = _probe->_robotToEntry - _robotToRCMOffset;

  // Finally calculate Base to Treatment zone using the measured transformation
  // for RCM to Treatment
  FK.zFrameToTreatment = zFrameToRCM * RCMToEntryPoint;

  return FK;
}
// This method defines the inverse kinematics for the neurosurgery robot
// Given: Vectors for the 3D location of the entry point and target point with
// respect to the zFrame Returns: The joint values for the given approach
Neuro_IK_outputs NeuroKinematics::InverseKinematics(
  Eigen::Vector4d entryPointzFrame, Eigen::Vector4d targetPointzFrame)
{

  // Structure to return the results of the IK
  struct Neuro_IK_outputs IK;

  // Get the entry point with respect to the orientation of the zFrame
  Eigen::Vector4d rcmToEntry = _zFrameToRCM.inverse() * entryPointzFrame;

  // Get the target point with respect to the orientation of the zFrame
  Eigen::Vector4d rcmToTarget = _zFrameToRCM.inverse() * targetPointzFrame;

  // The yaw and pitch components of the robot rely solely on the entry point's
  // location with respect to the target point This calculation is done with
  // respect to the RCM Orientation
  IK.YawRotation = (3.1415 / 2) + atan2(rcmToEntry(2) - rcmToTarget(2),
                                        rcmToEntry(1) - rcmToTarget(1));
  IK.PitchRotation =
    atan((rcmToEntry(0) - rcmToTarget(0)) / (rcmToEntry(2) - rcmToTarget(2)));

  // TODO: Add IK for probe Rotation
  IK.ProbeRotation = 0;

  // ==========================================================================================================

  // The Translational elements on the Inverse Kinematics rely on the final
  // location of the target point
  double XEntry = entryPointzFrame(0);
  double YEntry = entryPointzFrame(1);
  double ZEntry = entryPointzFrame(2);

  // The Lateral Translation is given by the desired distance in x
  IK.LateralTranslation = XEntry - _xInitialRCM -
                          _robotToRCMOffset * sin(IK.PitchRotation) +
                          _probe->_robotToEntry * sin(IK.PitchRotation);

  // Equations calculated through the symbolic equations for the Forward
  // Kinematics Substituting known values in the FK equations yields the value
  // for Axial Head and Feet
  IK.AxialHeadTranslation =
    ZEntry - _initialAxialSeperation / 2 + _widthTrapezoidTop / 2 -
    _zInitialRCM +
    sqrt(8 * YEntry * _yInitialRCM -
         2 * _initialAxialSeperation * _widthTrapezoidTop -
         4 * YEntry *
           sqrt(-pow(_initialAxialSeperation, 2) +
                2 * _initialAxialSeperation * _widthTrapezoidTop +
                4 * pow(_lengthOfAxialTrapezoidSideLink, 2) -
                pow(_widthTrapezoidTop, 2)) +
         4 * _yInitialRCM *
           sqrt(-pow(_initialAxialSeperation, 2) +
                2 * _initialAxialSeperation * _widthTrapezoidTop +
                4 * pow(_lengthOfAxialTrapezoidSideLink, 2) -
                pow(_widthTrapezoidTop, 2)) -
         4 * pow(YEntry, 2) + pow(_initialAxialSeperation, 2) +
         pow(_widthTrapezoidTop, 2) - 4 * pow(_yInitialRCM, 2) -
         4 * pow(_robotToRCMOffset, 2) * pow(cos(IK.PitchRotation), 2) *
           pow(cos(IK.YawRotation), 2) -
         4 * pow(_probe->_robotToEntry, 2) * pow(cos(IK.PitchRotation), 2) *
           pow(cos(IK.YawRotation), 2) +
         8 * _robotToRCMOffset * _probe->_robotToEntry *
           pow(cos(IK.PitchRotation), 2) * pow(cos(IK.YawRotation), 2) +
         8 * YEntry * _robotToRCMOffset * cos(IK.PitchRotation) *
           cos(IK.YawRotation) -
         8 * YEntry * _probe->_robotToEntry * cos(IK.PitchRotation) *
           cos(IK.YawRotation) -
         8 * _robotToRCMOffset * _yInitialRCM * cos(IK.PitchRotation) *
           cos(IK.YawRotation) +
         8 * _probe->_robotToEntry * _yInitialRCM * cos(IK.PitchRotation) *
           cos(IK.YawRotation) +
         4 * _robotToRCMOffset * cos(IK.PitchRotation) * cos(IK.YawRotation) *
           sqrt(-pow(_initialAxialSeperation, 2) +
                2 * _initialAxialSeperation * _widthTrapezoidTop +
                4 * pow(_lengthOfAxialTrapezoidSideLink, 2) -
                pow(_widthTrapezoidTop, 2)) -
         4 * _probe->_robotToEntry * cos(IK.PitchRotation) *
           cos(IK.YawRotation) *
           sqrt(-pow(_initialAxialSeperation, 2) +
                2 * _initialAxialSeperation * _widthTrapezoidTop +
                4 * pow(_lengthOfAxialTrapezoidSideLink, 2) -
                pow(_widthTrapezoidTop, 2))) /
      2 +
    _robotToRCMOffset * cos(IK.PitchRotation) * sin(IK.YawRotation) -
    _probe->_robotToEntry * cos(IK.PitchRotation) * sin(IK.YawRotation);
  IK.AxialFeetTranslation =
    ZEntry + _initialAxialSeperation / 2 - _widthTrapezoidTop / 2 -
    _zInitialRCM -
    sqrt(8 * YEntry * _yInitialRCM -
         2 * _initialAxialSeperation * _widthTrapezoidTop -
         4 * YEntry *
           sqrt(-pow(_initialAxialSeperation, 2) +
                2 * _initialAxialSeperation * _widthTrapezoidTop +
                4 * pow(_lengthOfAxialTrapezoidSideLink, 2) -
                pow(_widthTrapezoidTop, 2)) +
         4 * _yInitialRCM *
           sqrt(-pow(_initialAxialSeperation, 2) +
                2 * _initialAxialSeperation * _widthTrapezoidTop +
                4 * pow(_lengthOfAxialTrapezoidSideLink, 2) -
                pow(_widthTrapezoidTop, 2)) -
         4 * pow(YEntry, 2) + pow(_initialAxialSeperation, 2) +
         pow(_widthTrapezoidTop, 2) - 4 * pow(_yInitialRCM, 2) -
         4 * pow(_robotToRCMOffset, 2) * pow(cos(IK.PitchRotation), 2) *
           pow(cos(IK.YawRotation), 2) -
         4 * pow(_probe->_robotToEntry, 2) * pow(cos(IK.PitchRotation), 2) *
           pow(cos(IK.YawRotation), 2) +
         8 * _robotToRCMOffset * _probe->_robotToEntry *
           pow(cos(IK.PitchRotation), 2) * pow(cos(IK.YawRotation), 2) +
         8 * YEntry * _robotToRCMOffset * cos(IK.PitchRotation) *
           cos(IK.YawRotation) -
         8 * YEntry * _probe->_robotToEntry * cos(IK.PitchRotation) *
           cos(IK.YawRotation) -
         8 * _robotToRCMOffset * _yInitialRCM * cos(IK.PitchRotation) *
           cos(IK.YawRotation) +
         8 * _probe->_robotToEntry * _yInitialRCM * cos(IK.PitchRotation) *
           cos(IK.YawRotation) +
         4 * _robotToRCMOffset * cos(IK.PitchRotation) * cos(IK.YawRotation) *
           sqrt(-pow(_initialAxialSeperation, 2) +
                2 * _initialAxialSeperation * _widthTrapezoidTop +
                4 * pow(_lengthOfAxialTrapezoidSideLink, 2) -
                pow(_widthTrapezoidTop, 2)) -
         4 * _probe->_robotToEntry * cos(IK.PitchRotation) *
           cos(IK.YawRotation) *
           sqrt(-pow(_initialAxialSeperation, 2) +
                2 * _initialAxialSeperation * _widthTrapezoidTop +
                4 * pow(_lengthOfAxialTrapezoidSideLink, 2) -
                pow(_widthTrapezoidTop, 2))) /
      2 +
    _robotToRCMOffset * cos(IK.PitchRotation) * sin(IK.YawRotation) -
    _probe->_robotToEntry * cos(IK.PitchRotation) * sin(IK.YawRotation);

  // Probe Insertion is calculate as the distance between the entry point and
  // the target point in 3D space (with considerations for the final treatment
  // zone of the probe)
  IK.ProbeInsertion = sqrt(pow(entryPointzFrame(0) - targetPointzFrame(0), 2) +
                           pow(entryPointzFrame(1) - targetPointzFrame(1), 2) +
                           pow(entryPointzFrame(2) - targetPointzFrame(2), 2)) -
                      _probe->_robotToTreatmentAtHome + _probe->_robotToEntry;

  // Obtain basic Yaw, Pitch, and Roll Rotations
  xRotationDueToYawRotationIK << 1, 0, 0, 0, cos(IK.YawRotation),
    -sin(IK.YawRotation), 0, sin(IK.YawRotation), cos(IK.YawRotation);

  yRotationDueToPitchRotationIK << cos(IK.PitchRotation), 0,
    sin(IK.PitchRotation), 0, 1, 0, -sin(IK.PitchRotation), 0,
    cos(IK.PitchRotation);

  zRotationDueToProbeRotationIK << cos(IK.ProbeRotation),
    -sin(IK.ProbeRotation), 0, sin(IK.ProbeRotation), cos(IK.ProbeRotation), 0,
    0, 0, 1;
  // Calculate the XYZ Rotation
  zFrameToTargetPointFinal.block(0, 0, 3, 3) =
    (xRotationDueToYawRotationIK * yRotationDueToPitchRotationIK *
     zRotationDueToProbeRotationIK)
      .block(0, 0, 3, 3);
  zFrameToTargetPointFinal = _zFrameToRCM * zFrameToTargetPointFinal;

  // The X,Y,Z location is already given by the target point
  zFrameToTargetPointFinal(0, 3) = targetPointzFrame(0);
  zFrameToTargetPointFinal(1, 3) = targetPointzFrame(1);
  zFrameToTargetPointFinal(2, 3) = targetPointzFrame(2);

  // Obtain the FK of the target Pose
  IK.targetPose = zFrameToTargetPointFinal;

  return IK;
}
// Method to calculate the Cartesian base location and the Pitch and Yaw
// rotation of the robot given an EP and the RCM point as the TP.
Neuro_IK_outputs NeuroKinematics::InverseKinematicsWithZeroProbeInsertion(
  Eigen::Vector4d EntryPoint, Eigen::Vector4d TargetPoint)
{
  /* In this method the target point is going to be the the RCM point. The IK
  solver will try to find the values for lateral and Axial feet and Axial head
  translation that would result in the placement of the RCM on the given TP. The
  Yaw and Pitch values will be similar to the General InverseKinematics method.
  */
  Neuro_IK_outputs IK;
  //**Temporary values**
  IK.ProbeRotation  = 0;
  IK.ProbeInsertion = 0;

  double AxialHeadTranslation{}, AxialFeetTranslation{}, LateralTranslation{},
    yTrapezoidHypotenuseSquared{}, axialTrapezoidMidpoint{};
  double yTrapezoidSideSquared{}, yTrapezoidInitialSeparationSquared{};
  double zDeltaRCM{}, xDeltaRCM{}, yDeltaRCM{};
  // Get the entry point with respect to the orientation of the zFrame
  Eigen::Vector4d rcmToEntry = _zFrameToRCM.inverse() * EntryPoint;

  // Get the target point with respect to the orientation of the zFrame
  Eigen::Vector4d rcmToTarget = _zFrameToRCM.inverse() * TargetPoint;

  // The yaw and pitch components of the robot rely solely on the entry point's
  // location with respect to the target point This calculation is done with
  // respect to the RCM Orientation
  IK.YawRotation = (3.1415 / 2) + atan2(rcmToEntry(2) - rcmToTarget(2),
                                        rcmToEntry(1) - rcmToTarget(1));
  IK.PitchRotation =
    atan((rcmToEntry(0) - rcmToTarget(0)) / (rcmToEntry(2) - rcmToTarget(2)));
  // for the calculation of A * x = B, -->  x = inv(A) * B;
  Eigen::Vector2d x;
  Eigen::Matrix2d A;
  A << 1, -1, 1, 1;

  Eigen::Vector2d B;

  xDeltaRCM = TargetPoint(0) - _xInitialRCM;  // finding the delta values
  yDeltaRCM = TargetPoint(1) - _yInitialRCM;
  zDeltaRCM = TargetPoint(2) - _zInitialRCM;

  yTrapezoidHypotenuseSquared = pow(_lengthOfAxialTrapezoidSideLink, 2);
  yTrapezoidInitialSeparationSquared =
    pow((_initialAxialSeperation - _widthTrapezoidTop) / 2, 2);

  axialTrapezoidMidpoint =
    sqrt(-1 * (pow(sqrt(yTrapezoidHypotenuseSquared -
                        yTrapezoidInitialSeparationSquared),
                   2) +
               2 * yDeltaRCM *
                 sqrt(yTrapezoidHypotenuseSquared -
                      yTrapezoidInitialSeparationSquared) +
               pow(yDeltaRCM, 2) - yTrapezoidHypotenuseSquared)) +
    _widthTrapezoidTop / 2;

  // Finding the Lateral Translational value which is only dependant on the x
  // offset
  IK.LateralTranslation = xDeltaRCM;

  // Z position of RCM is solely defined as the midpoint of the axial trapezoid
  B(0) = axialTrapezoidMidpoint * 2 - _initialAxialSeperation;  // initial
                                                                // position
  // [1 -1][AxialHeadTranslation;AxialFeetTranslation]= axialTrapezoidMidpoint *
  // 2 - _initialAxialSeperation; // initial position

  B(1) = zDeltaRCM * 2;
  // [1 1][AxialHeadTranslation;AxialFeetTranslation]=zDeltaRCM * 2 ;

  x                       = A.inverse() * B;
  IK.AxialHeadTranslation = x(0);
  IK.AxialFeetTranslation = x(1);
  return IK;
};

// Method to calculate the RCM location w.r.t Z-frame
Neuro_FK_outputs NeuroKinematics::GetRcm(
  double AxialHeadTranslation, double AxialFeetTranslation,
  double LateralTranslation, double ProbeInsertion, double ProbeRotation,
  double PitchRotation, double YawRotation)
{
  // Structure to return with the FK output( struct can be remove )
  struct Neuro_FK_outputs RCM;

  // Z position of RCM is solely defined as the midpoint of the axial trapezoid
  double axialTrapezoidMidpoint =
    (AxialHeadTranslation - AxialFeetTranslation + _initialAxialSeperation) /
    2;  // initial position
  double zDeltaRCM = (AxialFeetTranslation + AxialHeadTranslation) / 2;

  // Y position of RCM is found by pythagorean theorem of the axial trapezoid
  double yTrapezoidHypotenuseSquared = pow(_lengthOfAxialTrapezoidSideLink, 2);
  double yTrapezoidSideSquared =
    pow((axialTrapezoidMidpoint - _widthTrapezoidTop / 2), 2);
  double yTrapezoidInitialSeparationSquared =
    pow((_initialAxialSeperation - _widthTrapezoidTop) / 2, 2);
  double yDeltaRCM =
    sqrt(yTrapezoidHypotenuseSquared - yTrapezoidSideSquared) -
    sqrt(yTrapezoidHypotenuseSquared - yTrapezoidInitialSeparationSquared);

  // X position of RCM is solely defined as the amount traveled in lateral
  // translation
  double xDeltaRCM = LateralTranslation;

  // Obtain basic Yaw, Pitch, and Roll Rotations
  xRotationDueToYawRotationFK << 1, 0, 0, 0, cos(YawRotation),
    -sin(YawRotation), 0, sin(YawRotation), cos(YawRotation);

  yRotationDueToPitchRotationFK << cos(PitchRotation), 0, sin(PitchRotation), 0,
    1, 0, -sin(PitchRotation), 0, cos(PitchRotation);

  zRotationDueToProbeRotationFK << cos(ProbeRotation), -sin(ProbeRotation), 0,
    sin(ProbeRotation), cos(ProbeRotation), 0, 0, 0, 1;

  // Calculate the XYZ Rotation
  zFrameToRCMRotation.block(0, 0, 3, 3) =
    (xRotationDueToYawRotationFK * yRotationDueToPitchRotationFK *
     zRotationDueToProbeRotationFK)
      .block(0, 0, 3, 3);

  // Calculate the XYZ Translation
  zFrameToRCMPrime << -1, 0, 0, _xInitialRCM + xDeltaRCM, 0, 0, -1,
    _yInitialRCM + yDeltaRCM, 0, -1, 0, _zInitialRCM + zDeltaRCM, 0, 0, 0, 1;

  // Now Calculate zFrame to RCM given the calculated values above
  Eigen::Matrix4d zFrameToRCM = zFrameToRCMPrime * zFrameToRCMRotation;
  RCM.zFrameToTreatment       = zFrameToRCM;
  return RCM;
}