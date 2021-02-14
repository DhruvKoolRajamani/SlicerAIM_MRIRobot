#include "../../include/WorkspaceGeneration/WorkspaceVisualization.h"
using std::endl;

// A is treatment to tip, B is robot to entry, this allows us to specify how
// close to the patient the physical robot can be, C is cannula to treatment
//  D is the robot to treatment distance.

ForwardKinematics::ForwardKinematics(NeuroKinematics& NeuroKinematics)
  : max_leg_displacement_(71.)
  , min_leg_seperation(75.)
  , axial_head_upper_bound_(0.)
  , axial_head_lower_bound_(-145.)
  , axial_feet_upper_bound_(68.)
  , axial_feet_lower_bound_(-77.)
  , pi(3.141)
  , Lateral_translation_start(-49.0)
  , Lateral_translation_end(-98.0)
  , Top_max_travel(-145)
  , Bottom_max_travel(-74)
  , RyF_max(-26.0 * pi / 180)
  , RyF_max_degree(-26.0)
  , RyB_max(37.0 * pi / 180)
  , RyB_max_degree(37.0)
  , Rx_max(-88.0 * pi / 180)
  , Rx_max_degree(-88.0)
  , Probe_insert_max(40)
  , Probe_insert_min(0)
  , Lateral_resolution(15.)
  , axial_resolution_(65.)
  , pitch_resolution_(10.)
  , yaw_resolution(15.)
  , probe_insertion_resolution(10.0)

{
  // Counters
  i       = 0.0;
  j       = 0.0;
  k       = 0.0;
  l       = 0.0;
  ii      = 0.0;
  counter = 0;

  // Min allowed seperation 75mm
  // Max allowed seperation 146mm
  //******TODO:You have to change the Pitch Bore and Face values and swap them!!
  // meaning that the RyB_max is 37 and RyF_max is -26
  Ry = 0.0;
  Rx = 0.0;
  // Robot axis
  AxialHeadTranslation = 0.0;
  AxialFeetTranslation = 0.0;
  LateralTranslation   = 0.0;
  PitchRotation        = 0.0;
  YawRotation          = 0.0;
  ProbeInsertion       = 0.0;
  ProbeRotation        = 0.0;
  // RCM point cloud
  rcm_point_set_   = GetRcmPointSet();
  NeuroKinematics_ = NeuroKinematics;
}

// Method to generate Point cloud of the surface of general reachable Workspace
Eigen::Matrix3Xf ForwardKinematics::GetGeneralWorkspace()
{
  // Matrix to store point set
  Eigen::Matrix3Xf point_set(3, 1);
  point_set << 0., 0., 0.;

  // Object containing the 4x4 transformation matrix
  Neuro_FK_outputs FK{};

  // Visualization of the top of the Workspace
  AxialFeetTranslation = axial_feet_upper_bound_;
  AxialHeadTranslation = axial_head_upper_bound_;
  YawRotation          = 0;
  PitchRotation        = 0;
  ProbeInsertion       = Probe_insert_min;
  counter              = 0;

  // initial separation 143, min separation 75=> 143-75 = 68 mm
  for (i = Top_max_travel / axial_resolution_; i >= Top_max_travel;
       i += Top_max_travel / axial_resolution_)
  {

    AxialHeadTranslation += Top_max_travel / axial_resolution_;
    AxialFeetTranslation += Top_max_travel / axial_resolution_;
    for (k = counter = Lateral_translation_start; k >= Lateral_translation_end;
         k += Lateral_translation_start / Lateral_resolution,
        counter = floor(k))
    {
      LateralTranslation = k;
      if (counter == floor(Lateral_translation_start))
      {
        for (j = 0; j <= RyB_max; j += RyB_max / pitch_resolution_)
        {
          PitchRotation = j;
          FK            = NeuroKinematics_.ForwardKinematics(
            AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
            ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
          StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
        }
        PitchRotation = 0;
      }
      else if (counter == floor(Lateral_translation_end))
      {
        for (j = 0; j >= RyF_max; j += RyF_max / pitch_resolution_)
        {
          PitchRotation = j;
          FK            = NeuroKinematics_.ForwardKinematics(
            AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
            ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
          StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
        }
        PitchRotation = 0;
      }
      else
      {
        FK = NeuroKinematics_.ForwardKinematics(
          AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
          ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
        StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
      }
    }
  }

  // loop for visualizing the bottom
  ProbeInsertion = Probe_insert_max;
  // The combination of 0 for head and -3 for feet gives max seperation (146)
  // for bottom WS generation
  AxialFeetTranslation = -3;
  AxialHeadTranslation = axial_head_upper_bound_;
  for (i = 0; i >= Bottom_max_travel;
       i += Bottom_max_travel / axial_resolution_)
  {
    AxialHeadTranslation += Bottom_max_travel / axial_resolution_;
    AxialFeetTranslation += Bottom_max_travel / axial_resolution_;
    for (k = counter = Lateral_translation_start; k >= Lateral_translation_end;
         k += Lateral_translation_start / Lateral_resolution,
        counter = floor(k))
    {
      LateralTranslation = k;
      if (counter == floor(Lateral_translation_start))
      {
        for (j = 0; j >= RyF_max; j += RyF_max / 3)
        {
          PitchRotation = j;
          FK            = NeuroKinematics_.ForwardKinematics(
            AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
            ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
          StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
        }
      }
      else if (counter == floor(Lateral_translation_end))
      {
        for (j = 0; j <= RyB_max; j += RyB_max / 3)
        {
          PitchRotation = j;
          FK            = NeuroKinematics_.ForwardKinematics(
            AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
            ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
          StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
        }
      }
      else
      {
        PitchRotation = 0;
        FK            = NeuroKinematics_.ForwardKinematics(
          AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
          ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
        StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
      }
    }
  }

  YawRotation          = 0;
  PitchRotation        = 0;
  AxialFeetTranslation = -3;
  AxialHeadTranslation = axial_head_upper_bound_;
  ProbeInsertion       = Probe_insert_min;

  // Loop for creating the head face
  for (int counter_j = j = -3; j <= max_leg_displacement_;
       j += max_leg_displacement_ / Lateral_resolution, counter_j = floor(j))
  {
    AxialFeetTranslation = j;
    for (k = counter = Lateral_translation_start; k >= Lateral_translation_end;
         k += Lateral_translation_start / Lateral_resolution,
        counter = floor(k))
    {
      LateralTranslation = k;
      // Top level
      if (counter_j == round(axial_feet_upper_bound_))
      {
        // Bore side
        if (counter == round(Lateral_translation_start))
        {
          for (i = 0; i <= RyB_max; i += RyB_max / pitch_resolution_)
          {
            PitchRotation = i;
            for (ii = 0; ii >= Rx_max; ii += Rx_max / yaw_resolution)
            {
              YawRotation = ii;
              FK          = NeuroKinematics_.ForwardKinematics(
                AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
                ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
              StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
            }
          }
        }
        // Face side
        else if (counter == round(Lateral_translation_end))
        {
          for (i = 0; i >= RyF_max; i += RyF_max / pitch_resolution_)
          {
            PitchRotation = i;
            for (ii = 0; ii >= Rx_max; ii += Rx_max / yaw_resolution)
            {
              YawRotation = ii;
              FK          = NeuroKinematics_.ForwardKinematics(
                AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
                ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
              StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
            }
          }
        }
        // between the bore and the face
        else
        {
          for (ii = Rx_max / yaw_resolution; ii >= Rx_max;
               ii += Rx_max / yaw_resolution)
          {
            PitchRotation = 0;
            YawRotation   = ii;
            FK            = NeuroKinematics_.ForwardKinematics(
              AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
              ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
            StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
          }
        }
      }
      // Base level
      else if (j == -3)
      {
        // Creating corner bore side
        if (counter == floor(Lateral_translation_start))
        {
          YawRotation = Rx_max;
          // lvl one bore side yaw lowered pitch lowering
          for (i = 0; i <= RyB_max; i += RyB_max / pitch_resolution_)
          {
            PitchRotation = i;
            for (ii = Probe_insert_min; ii <= Probe_insert_max - 10;
                 ii += Probe_insert_max / probe_insertion_resolution)
            {
              ProbeInsertion = ii;
              FK             = NeuroKinematics_.ForwardKinematics(
                AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
                ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
              StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
            }
          }
        }
        // Creating corner face side
        else if (counter == floor(Lateral_translation_end))
        {
          YawRotation = Rx_max;
          // yaw lowered and increasing pitch
          for (i = 0; i >= RyF_max; i += RyF_max / pitch_resolution_)
          {
            PitchRotation = i;
            for (ii = Probe_insert_min; ii <= Probe_insert_max - 10;
                 ii += Probe_insert_max / probe_insertion_resolution)
            {
              ProbeInsertion = ii;
              FK             = NeuroKinematics_.ForwardKinematics(
                AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
                ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
              StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
            }
          }
        }
        // Space between two corners
        else
        {
          YawRotation   = Rx_max;
          PitchRotation = 0;
          for (ii = Probe_insert_min; ii <= Probe_insert_max - 10;
               ii += Probe_insert_max / probe_insertion_resolution)
          {
            ProbeInsertion = ii;
            FK             = NeuroKinematics_.ForwardKinematics(
              AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
              ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
            StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
          }
        }
        ProbeInsertion = Probe_insert_min;
      }

      // Any other lvl from bottom to just a lvl before the top
      else
      {
        YawRotation = Rx_max;
        // Creating corner bore side
        if (k == Lateral_translation_start)
        {
          // lvl one bore side yaw lowered pitch lowering
          for (i = 0; i <= RyB_max; i += RyB_max / pitch_resolution_)
          {
            PitchRotation = i;
            FK            = NeuroKinematics_.ForwardKinematics(
              AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
              ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
            StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
          }
        }
        // Creating corner face side
        else if (counter == floor(Lateral_translation_end))
        {
          // level one face side yaw lowered pitch increasing
          for (i = 0; i >= RyF_max; i += RyF_max / pitch_resolution_)
          {
            PitchRotation = i;
            FK            = NeuroKinematics_.ForwardKinematics(
              AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
              ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
            StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
          }
        }
        // Space between two corners
        else
        {
          PitchRotation = 0;
          FK            = NeuroKinematics_.ForwardKinematics(
            AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
            ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
          StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
        }
      }
    }
  }
  YawRotation          = 0;
  PitchRotation        = 0;
  AxialFeetTranslation = axial_feet_lower_bound_;
  // moving the base to the lowest configuration +3 makes leg separation 146
  AxialHeadTranslation = axial_feet_lower_bound_ + 3;
  ProbeInsertion       = Probe_insert_max;
  int counter_i{};
  // feet face first level from bottom
  for (j = 0; j >= Rx_max; j += Rx_max / yaw_resolution)
  {
    YawRotation = j;
    for (k = counter = Lateral_translation_start; k >= Lateral_translation_end;
         k += Lateral_translation_start / Lateral_resolution,
        counter = floor(k))
    {
      LateralTranslation = k;
      // creating curved corners
      if (counter == floor(Lateral_translation_start))
      {
        for (l = 0; l >= RyF_max; l += RyF_max / pitch_resolution_)
        {
          PitchRotation = l;
          FK            = NeuroKinematics_.ForwardKinematics(
            AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
            ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
          StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
          PitchRotation = 0;
        }
      }
      else if (counter == floor(Lateral_translation_end))
      {
        for (l = 0; l <= RyB_max; l += RyB_max / pitch_resolution_)
        {
          PitchRotation = l;
          FK            = NeuroKinematics_.ForwardKinematics(
            AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
            ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
          StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
          PitchRotation = 0;
        }
      }
      else
      {
        FK = NeuroKinematics_.ForwardKinematics(
          AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
          ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
        StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
      }
    }
  }

  // Feet face all other levels except the first
  YawRotation    = Rx_max;
  ProbeInsertion = Probe_insert_max;
  PitchRotation  = 0;
  for (i = counter_i = axial_feet_lower_bound_; i >= axial_head_lower_bound_;
       i +=
       (axial_head_lower_bound_ - axial_feet_lower_bound_) / Lateral_resolution,
      counter_i = floor(i))
  {
    AxialHeadTranslation = i;
    // Top level
    if (counter_i == floor(axial_head_lower_bound_))
    {
      for (k = Lateral_translation_start; k >= Lateral_translation_end;
           k += Lateral_translation_start / Lateral_resolution)
      {
        LateralTranslation = k;
        for (l = Probe_insert_max / 20; l <= Probe_insert_max - 10;
             l += Probe_insert_max / 20)
        {
          ProbeInsertion = l;
          YawRotation    = 0;

          FK = NeuroKinematics_.ForwardKinematics(
            AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
            ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
          StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
        }
      }
    }
    // All other levels between the bottom level and the top
    else
    {
      for (k = counter = Lateral_translation_start;
           k >= Lateral_translation_end;
           k += Lateral_translation_start / Lateral_resolution,
          counter = floor(k))
      {
        LateralTranslation = k;
        if (counter == floor(Lateral_translation_start))
        {
          for (l = 0; l >= RyF_max; l += RyF_max / yaw_resolution)
          {
            PitchRotation = l;
            FK            = NeuroKinematics_.ForwardKinematics(
              AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
              ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
            StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
            PitchRotation = 0;
          }
        }
        else if (counter == floor(Lateral_translation_end))
        {
          for (l = 0; l <= RyB_max; l += RyB_max / yaw_resolution)
          {
            PitchRotation = l;
            FK            = NeuroKinematics_.ForwardKinematics(
              AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
              ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
            StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
            PitchRotation = 0;
          }
        }
        else
        {
          FK = NeuroKinematics_.ForwardKinematics(
            AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
            ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
          StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
        }
      }
    }
  }

  // loop for creating the sides
  ProbeInsertion                    = Probe_insert_min;
  AxialFeetTranslation              = -3;
  AxialHeadTranslation              = axial_head_upper_bound_;
  YawRotation                       = 0;
  PitchRotation                     = 0;
  double axial_feet_translation_old = AxialFeetTranslation;
  // Loop for setting the max allowed movement for each level
  for (double max_travel = Bottom_max_travel,
              counter_i  = round(Bottom_max_travel);
       max_travel >= Top_max_travel;
       max_travel += (Top_max_travel - Bottom_max_travel) / Lateral_resolution,
              counter_i = floor(max_travel))
  {
    AxialFeetTranslation = axial_feet_translation_old;
    // Loop for moving feet and head in each level
    for (ii = AxialHeadTranslation = 0;
         round(AxialHeadTranslation) >= max_travel;
         AxialHeadTranslation += max_travel / Lateral_resolution,
        ii = max_travel / Lateral_resolution)
    {
      AxialFeetTranslation += ii;

      for (k = counter = Lateral_translation_start;
           k >= Lateral_translation_end;
           k += Lateral_translation_start, counter = floor(k))
      {
        LateralTranslation = k;
        // For the side towards bore
        if (counter == floor(Lateral_translation_start))
        {
          // For the first level
          if (counter_i == round(Bottom_max_travel))
          {
            PitchRotation = RyB_max;
            int counter_j{};
            for (counter_j = i = 0.; i >= Rx_max;
                 i += Rx_max / yaw_resolution, counter_j++)
            {
              YawRotation = i;
              if (counter_j == round(yaw_resolution - 1) ||
                  round(AxialHeadTranslation) == round(max_travel))
              {

                for (l = 0; l <= (Probe_insert_max);
                     l += (Probe_insert_max) / probe_insertion_resolution)
                {
                  ProbeInsertion = l;
                  FK             = NeuroKinematics_.ForwardKinematics(
                    AxialHeadTranslation, AxialFeetTranslation,
                    LateralTranslation, ProbeInsertion, ProbeRotation,
                    PitchRotation, YawRotation);
                  StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
                }
                ProbeInsertion = Probe_insert_min;
              }
              else
              {
                FK = NeuroKinematics_.ForwardKinematics(
                  AxialHeadTranslation, AxialFeetTranslation,
                  LateralTranslation, ProbeInsertion, ProbeRotation,
                  PitchRotation, YawRotation);
                StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
              }
            }
          }
          // For all levels between bottom and top levels
          else
          {
            PitchRotation = RyB_max;
            for (i = 0; i > Rx_max; i += Rx_max / yaw_resolution)
            {
              YawRotation = i;
              if (round(AxialHeadTranslation) == round(max_travel))
              {
                for (l = 0; l <= (Probe_insert_max);
                     l += (Probe_insert_max) / probe_insertion_resolution)
                {
                  ProbeInsertion = l;
                  FK             = NeuroKinematics_.ForwardKinematics(
                    AxialHeadTranslation, AxialFeetTranslation,
                    LateralTranslation, ProbeInsertion, ProbeRotation,
                    PitchRotation, YawRotation);
                  StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
                }
                ProbeInsertion = Probe_insert_min;
              }
              else
              {
                PitchRotation = RyB_max;
                FK            = NeuroKinematics_.ForwardKinematics(
                  AxialHeadTranslation, AxialFeetTranslation,
                  LateralTranslation, ProbeInsertion, ProbeRotation,
                  PitchRotation, YawRotation);
                StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
              }
            }
          }
        }
        // face side
        else  //(counter == floor(Lateral_translation_end))
        {
          PitchRotation = RyF_max;
          // For the first level
          if (counter_i == round(Bottom_max_travel))
          {
            PitchRotation = RyF_max;
            int counter_j{};
            for (counter_j = i = 0.; i >= Rx_max;
                 i += Rx_max / yaw_resolution, counter_j++)
            {
              YawRotation = i;
              if (counter_j == round(yaw_resolution - 1) ||
                  round(AxialHeadTranslation) == round(max_travel))
              {

                for (l = 0; l <= (Probe_insert_max);
                     l += (Probe_insert_max) / probe_insertion_resolution)
                {
                  ProbeInsertion = l;
                  FK             = NeuroKinematics_.ForwardKinematics(
                    AxialHeadTranslation, AxialFeetTranslation,
                    LateralTranslation, ProbeInsertion, ProbeRotation,
                    PitchRotation, YawRotation);
                  StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
                }
                ProbeInsertion = Probe_insert_min;
              }
              else
              {
                FK = NeuroKinematics_.ForwardKinematics(
                  AxialHeadTranslation, AxialFeetTranslation,
                  LateralTranslation, ProbeInsertion, ProbeRotation,
                  PitchRotation, YawRotation);
                StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
              }
            }
          }
          // For all levels between bottom and top levels
          else
          {
            PitchRotation = RyF_max;
            for (i = 0; i > Rx_max; i += Rx_max / yaw_resolution)
            {
              YawRotation = i;
              if (round(AxialHeadTranslation) == round(max_travel))
              {
                for (l = 0; l <= (Probe_insert_max);
                     l += (Probe_insert_max) / probe_insertion_resolution)
                {
                  ProbeInsertion = l;
                  FK             = NeuroKinematics_.ForwardKinematics(
                    AxialHeadTranslation, AxialFeetTranslation,
                    LateralTranslation, ProbeInsertion, ProbeRotation,
                    PitchRotation, YawRotation);
                  StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
                }
                ProbeInsertion = Probe_insert_min;
              }
              else
              {
                FK = NeuroKinematics_.ForwardKinematics(
                  AxialHeadTranslation, AxialFeetTranslation,
                  LateralTranslation, ProbeInsertion, ProbeRotation,
                  PitchRotation, YawRotation);
                StorePointToEigenMatrix(point_set, FK.zFrameToTreatment);
              }
            }
          }
        }
      }
    }
    axial_feet_translation_old -=
      (Top_max_travel - Bottom_max_travel) / Lateral_resolution;
  }

  return point_set;
}

// Method to generate Point cloud of the surface of the RCM Workspace
Eigen::Matrix3Xf ForwardKinematics::GetRcmWorkSpace()
{
  // Object containing the 4x4 transformation matrix
  Neuro_FK_outputs RCM{};
  // Matrix to store point set
  Eigen::Matrix3Xf point_set(3, 1);
  point_set << 0., 0., 0.;

  //  ++++RCM Point Cloud Generation+++

  // Visualization of the top of the Workspace
  AxialFeetTranslation = axial_feet_upper_bound_;
  AxialHeadTranslation = axial_head_upper_bound_;
  YawRotation          = 0;
  PitchRotation        = 0;
  ProbeInsertion       = Probe_insert_min;

  // initial separation 143, min separation 75=> 143-75 = 68 mm
  for (i = Top_max_travel / axial_resolution_; i >= Top_max_travel;
       i += Top_max_travel / axial_resolution_)
  {
    AxialHeadTranslation += Top_max_travel / axial_resolution_;
    AxialFeetTranslation += Top_max_travel / axial_resolution_;
    for (k = Lateral_translation_start; k >= Lateral_translation_end;
         k += Lateral_translation_start / Lateral_resolution)
    {
      LateralTranslation = k;
      RCM = NeuroKinematics_.GetRcm(AxialHeadTranslation, AxialFeetTranslation,
                                    LateralTranslation, ProbeInsertion,
                                    ProbeRotation, PitchRotation, YawRotation);
      StorePointToEigenMatrix(point_set, RCM.zFrameToTreatment);
    }
  }

  // loop for visualizing the bottom
  AxialHeadTranslation = axial_head_upper_bound_;
  AxialFeetTranslation = -3;
  for (i = 0; i >= Bottom_max_travel;
       i += Bottom_max_travel / axial_resolution_)
  {
    AxialHeadTranslation += Bottom_max_travel / axial_resolution_;
    AxialFeetTranslation += Bottom_max_travel / axial_resolution_;
    for (k = Lateral_translation_start; k >= Lateral_translation_end;
         k += Lateral_translation_start / Lateral_resolution)
    {
      LateralTranslation = k;

      RCM = NeuroKinematics_.GetRcm(AxialHeadTranslation, AxialFeetTranslation,
                                    LateralTranslation, ProbeInsertion,
                                    ProbeRotation, PitchRotation, YawRotation);
      StorePointToEigenMatrix(point_set, RCM.zFrameToTreatment);
    }
  }

  // Loop for creating the head face
  AxialFeetTranslation = -3;
  AxialHeadTranslation = axial_head_upper_bound_;

  for (int counter_j = j = -3; j <= max_leg_displacement_;
       j += max_leg_displacement_ / Lateral_resolution, counter_j = floor(j))
  {
    AxialFeetTranslation = j;

    for (k = Lateral_translation_start; k >= Lateral_translation_end;
         k += Lateral_translation_start / Lateral_resolution)
    {
      LateralTranslation = k;

      RCM = NeuroKinematics_.GetRcm(AxialHeadTranslation, AxialFeetTranslation,
                                    LateralTranslation, ProbeInsertion,
                                    ProbeRotation, PitchRotation, YawRotation);
      StorePointToEigenMatrix(point_set, RCM.zFrameToTreatment);
    }
  }

  // Loop for creating the feet face
  AxialFeetTranslation = axial_feet_lower_bound_;
  // moving the base to the lowest configuration +3 makes leg separation 146
  AxialHeadTranslation = axial_feet_lower_bound_ + 3;

  for (k = counter = Lateral_translation_start; k >= Lateral_translation_end;
       k += Lateral_translation_start / Lateral_resolution, counter = floor(k))
  {
    LateralTranslation = k;
    RCM = NeuroKinematics_.GetRcm(AxialHeadTranslation, AxialFeetTranslation,
                                  LateralTranslation, ProbeInsertion,
                                  ProbeRotation, PitchRotation, YawRotation);
    StorePointToEigenMatrix(point_set, RCM.zFrameToTreatment);
  }
  // Other levels
  for (i = axial_feet_lower_bound_; i >= axial_head_lower_bound_;
       i +=
       (axial_head_lower_bound_ - axial_feet_lower_bound_) / Lateral_resolution)
  {
    AxialHeadTranslation = i;
    for (k = Lateral_translation_start; k >= Lateral_translation_end;
         k += Lateral_translation_start / Lateral_resolution)
    {
      LateralTranslation = k;
      RCM = NeuroKinematics_.GetRcm(AxialHeadTranslation, AxialFeetTranslation,
                                    LateralTranslation, ProbeInsertion,
                                    ProbeRotation, PitchRotation, YawRotation);
      StorePointToEigenMatrix(point_set, RCM.zFrameToTreatment);
    }
  }

  // loop for creating the sides
  ProbeInsertion                    = Probe_insert_min;
  AxialFeetTranslation              = -3;
  AxialHeadTranslation              = axial_head_upper_bound_;
  YawRotation                       = 0;
  PitchRotation                     = 0;
  double axial_feet_translation_old = AxialFeetTranslation;
  for (double max_travel = Bottom_max_travel,
              counter_i  = round(Bottom_max_travel);
       max_travel >= Top_max_travel;
       max_travel += (Top_max_travel - Bottom_max_travel) / Lateral_resolution,
              counter_i = floor(max_travel))
  {
    AxialFeetTranslation = axial_feet_translation_old;
    // Loop for moving feet and head in each level
    for (ii = AxialHeadTranslation = 0;
         round(AxialHeadTranslation) >= max_travel;
         AxialHeadTranslation += max_travel / Lateral_resolution,
        ii = max_travel / Lateral_resolution)
    {
      AxialFeetTranslation += ii;
      for (k = counter = Lateral_translation_start;
           k >= Lateral_translation_end;
           k += Lateral_translation_start, counter = floor(k))
      {
        LateralTranslation = k;
        LateralTranslation = k;
        RCM                = NeuroKinematics_.GetRcm(
          AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
          ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
        StorePointToEigenMatrix(point_set, RCM.zFrameToTreatment);
      }
    }
    axial_feet_translation_old -=
      (Top_max_travel - Bottom_max_travel) / Lateral_resolution;
  }

  return point_set;
}

// Method to generate a point set containing all RCM points
Eigen::Matrix3Xf ForwardKinematics::GetRcmPointSet()
{
  // Object containing the 4x4 transformation matrix
  Neuro_FK_outputs RCM_PC{};

  // RCM point set generation

  /* Initializing a 3 X 197,779 Eigen matrix to store total RCM point-set.
  Change the col length if the coarseness of the PS is changed. The value
  for this length is printed upon running this method.*/
  Eigen::Matrix3Xf rcm_point_set(3, 197779);

  // Minimum allowed distance between the two legs {71}
  double min_seperation{71};
  // Division factor determining the coarseness of the PC in the Z direction
  // 100
  const double division{100};
  // Division factor determining the coarseness of the PC in the X direction
  // 20
  const double division_k{20};
  // Strarting position for the lateral head -49
  const double lateral_start{-49};
  const double Abs_min_leg_separation{68};  // 68

  // Loop for visualizing the top
  AxialFeetTranslation = 68;
  AxialHeadTranslation = 0;
  double Top_max_travel{-157};
  counter = 0;  //-157
  // initial separation 143, min separation 75 => 143-75 = 68 mm
  for (i = Top_max_travel / division; i >= Top_max_travel;
       i += Top_max_travel / division)
  {
    AxialHeadTranslation += Top_max_travel / division;
    AxialFeetTranslation += Top_max_travel / division;
    // max lateral movement 0.0 ~ -49.47 (appx = -49)
    for (k = lateral_start; k >= lateral_start * 2;
         k += lateral_start / division_k)
    {
      LateralTranslation = k;
      RCM_PC             = NeuroKinematics_.GetRcm(
        AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
        ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
      StorePoint(rcm_point_set, RCM_PC.zFrameToTreatment, counter);
      ++counter;
    }
  }

  // loop for visualizing the bottom
  AxialHeadTranslation = 0;
  AxialFeetTranslation = -3;
  double max_travel_bottom{-86};
  for (i = 0; i >= max_travel_bottom; i += max_travel_bottom / division)
  {
    // for the beginning row
    if (i == 0)
    {
      for (k = lateral_start; k >= lateral_start * 2;
           k += lateral_start / division_k)
      {
        LateralTranslation = k;
        RCM_PC             = NeuroKinematics_.GetRcm(
          AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
          ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
        StorePoint(rcm_point_set, RCM_PC.zFrameToTreatment, counter);
        ++counter;
      }
    }
    else
    {
      AxialHeadTranslation += max_travel_bottom / division;
      AxialFeetTranslation += max_travel_bottom / division;
      for (k = lateral_start; k >= lateral_start * 2;
           k += lateral_start / division_k)
      {
        LateralTranslation = k;
        RCM_PC             = NeuroKinematics_.GetRcm(
          AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
          ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
        StorePoint(rcm_point_set, RCM_PC.zFrameToTreatment, counter);
        ++counter;
      }
    }
  }
  AxialFeetTranslation = -3;
  AxialHeadTranslation = 0;

  // Loop for creating the head face
  for (j = min_seperation / division; j <= min_seperation;
       j += min_seperation / division)
  {
    AxialFeetTranslation += min_seperation / division;

    for (k = lateral_start; k >= lateral_start * 2;
         k += lateral_start / division_k)
    {
      LateralTranslation = k;

      RCM_PC = NeuroKinematics_.GetRcm(
        AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
        ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
      StorePoint(rcm_point_set, RCM_PC.zFrameToTreatment, counter);
      ++counter;
    }
  }
  AxialFeetTranslation = -89;
  AxialHeadTranslation = -86;

  // Loop for creating the feet face
  for (j = min_seperation / division; j <= min_seperation;
       j += min_seperation / division)
  {
    AxialHeadTranslation -= min_seperation / division;

    for (k = lateral_start; k >= lateral_start * 2;
         k += lateral_start / division_k)
    {
      LateralTranslation = k;

      RCM_PC = NeuroKinematics_.GetRcm(
        AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
        ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
      StorePoint(rcm_point_set, RCM_PC.zFrameToTreatment, counter);
      ++counter;
    }
  }

  // loop for creating the sides
  AxialFeetTranslation = -3;
  AxialHeadTranslation = 0;
  // The max that the robot can move in z direction when at lowest height
  // (at each hight min travel is changed)
  double min_travel{-86};
  // 157 The max that the robot can move in z direction when at highest
  // height.
  double max_travel{-146};

  for (j = min_seperation / division; j <= Abs_min_leg_separation;
       j += min_seperation / division)
  {
    // For each loop it will lift the base by a constant value
    AxialFeetTranslation += j;
    // Takes care of the amount of Axial travel for the Axial head and feet
    min_travel -= min_seperation / division;

    // loop to move the base from Head to Feet, based on the allowable max
    // movement range (min_travel)
    for (ii = 0; ii > min_travel + 1 && min_travel >= max_travel;
         ii += min_travel / division)
    {
      AxialHeadTranslation += min_travel / division;
      AxialFeetTranslation += min_travel / division;
      for (k = lateral_start; k >= lateral_start * 2;
           k += lateral_start / division_k)
      {
        LateralTranslation = k;
        RCM_PC             = NeuroKinematics_.GetRcm(
          AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
          ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
        StorePoint(rcm_point_set, RCM_PC.zFrameToTreatment, counter);
        ++counter;
      }
    }
    AxialFeetTranslation = -3;
    AxialHeadTranslation = 0;
  }
  return rcm_point_set;
}

// Method to return a point set based on a given EP.
Eigen::Matrix3Xf
  ForwardKinematics::GetSubWorkspace(Eigen::Vector3d ep_in_robot_coordinate)
{

  // Number of points inside the RCM point set
  int no_cols_rcm_pc = rcm_point_set_.cols();

  Eigen::Vector3f rcm_point_to_check;
  // Matrix which stores the validated point set after checking the sphere
  // cond
  Eigen::Matrix3Xf validated_point_set(3, 1);
  /* Loop which goes through each RCM points and checks for the validity of
  each point based on the sphere criteria.*/
  for (int i = 0; i < no_cols_rcm_pc; i++)
  {
    rcm_point_to_check << rcm_point_set_(0, i), rcm_point_set_(1, i),
      rcm_point_set_(2, i);
    if (CheckSphere(ep_in_robot_coordinate, rcm_point_to_check) == 1)
    {
      StorePointToEigenMatrix(validated_point_set, rcm_point_set_(0, i),
                              rcm_point_set_(1, i), rcm_point_set_(2, i));
    }
  }

  // Step to check the Inverse Kinematics for each validated RCM point set
  Eigen::Matrix3Xf validated_inverse_kinematic_rcm_pointset =
    GetPointCloudInverseKinematics(validated_point_set, ep_in_robot_coordinate);

  /* Step to populate the point set with equidistance points from the EP to
  each validated rcm points and past them with the max probe insertion
  criteria.*/
  Eigen::Matrix3Xf total_Sub_Workspace = GenerateFinalSubworkspacePointset(
    validated_inverse_kinematic_rcm_pointset, ep_in_robot_coordinate);

  return total_Sub_Workspace;
}

/* Method to store a point of the RCM Point Cloud. Points are stored inside
an Eigen matrix.*/
void ForwardKinematics::StorePoint(Eigen::Matrix3Xf& rcm_point_cloud,
                                   Eigen::Matrix4d   transformation_matrix,
                                   int               counter)
{
  rcm_point_cloud(0, counter) = transformation_matrix(0, 3);
  rcm_point_cloud(1, counter) = transformation_matrix(1, 3);
  rcm_point_cloud(2, counter) = transformation_matrix(2, 3);
}

// Method to check if the Entry point is within the bounds of a given RCM
// point
bool ForwardKinematics::CheckSphere(Eigen::Vector3d ep_in_robot_coordinate,
                                    Eigen::Vector3f rcm_point_set)
{
  /*Whether a point lies inside a sphere or not, depends upon its distance
  from the centre. A point (x, y, z) is inside the sphere with center (cx,
  cy, cz) and radius r if ( x-cx ) ^2 + (y-cy) ^2 + (z-cz) ^ 2 < r^2 */
  float       B_value = NeuroKinematics_._probe->_robotToEntry;
  const float radius  = 72.5 - B_value;  // RCM offset from Robot to RCM
                                         // point

  float distance{0};
  distance = pow(ep_in_robot_coordinate(0) - rcm_point_set(0), 2) +
             pow(ep_in_robot_coordinate(1) - rcm_point_set(1), 2) +
             pow(ep_in_robot_coordinate(2) - rcm_point_set(2), 2);
  // Entry point is inside the Sphere
  if (distance <= pow(radius, 2))
  {
    return 1;
  }
  // Entry point is outside of the Sphere
  else
  {
    return 0;
  }
}

/* Method to Check the IK for each point in the Validated point set and
stores the ones that are valid*/
Eigen::Matrix3Xf ForwardKinematics::GetPointCloudInverseKinematics(
  Eigen::Matrix3Xf validated_point_set, Eigen::Vector3d ep_in_robot_coordnt)
{
  // Initializng the sub_workspace matrix
  Eigen::Matrix3Xf sub_workspace_rcm_point_set(3, 1);
  sub_workspace_rcm_point_set << 0., 0., 0.;

  int counter{0};
  int no_cols_validated_point_set = validated_point_set.cols();

  /* The methods checks for validity of the filtered workspace Using
  InverseKinematicsWithZeroProbeInsertion Method. This Method takes two
  Eigen Vectors of size 4 i.e (x,y,z,1). First argument is the EP and the
  second argument is the RCM point which is considered as the TP.*/

  // Initializing the vectors for EP and TP.
  Eigen::Vector4d ep_in_robot_coordinate(
    ep_in_robot_coordnt(0), ep_in_robot_coordnt(1), ep_in_robot_coordnt(2), 1);
  Eigen::Vector4d tp_in_robot_coordinate(0, 0, 0, 1);

  // Initializing the limits for each axis of the robot.
  const double initial_Axial_separation  = 143;
  const double min_Axial_separation      = 75;
  const double max_Axial_separation      = 146;
  const double max_Lateral_translation   = -49;
  const double min_Lateral_translation   = -98;
  const double max_AxialHead_translation = 0;
  const double min_AxialHead_translation = -146;  // it may be -145
  const double max_AxialFeet_translation = 68;
  const double min_AxialFeet_translation = -78;
  const double max_Pitch_rotation        = +26.0 * pi / 180;
  const double min_Pitch_rotation        = -37.0 * pi / 180;
  const double max_Yaw_rotation          = 0.0;
  const double min_Yaw_rotation          = -88.0 * pi / 180;
  const double min_Probe_insertion       = 0;
  const double max_probe_insertion       = 50;
  double       Axial_Seperation{0};

  /* Object to store the output of the
  InverseKinematicsWithZeroProbeInsertion method*/
  Neuro_IK_outputs IK_output;

  /* Loop that goes through each point in the Validated PC and checks for
  the Validity of the IK output*/
  for (i = 0; i < no_cols_validated_point_set; i++)
  {
    // setting the TP for each point in the loop
    tp_in_robot_coordinate << validated_point_set(0, i),
      validated_point_set(1, i), validated_point_set(2, i), 1;

    IK_output = NeuroKinematics_.InverseKinematicsWithZeroProbeInsertion(
      ep_in_robot_coordinate, tp_in_robot_coordinate);

    Axial_Seperation =
      143 + IK_output.AxialHeadTranslation - IK_output.AxialFeetTranslation;

    if (Axial_Seperation > max_Axial_separation)
    {
      continue;
    }
    /*Axial Heads are farther away than the allowed value or Axial Heads are
    closer than the allowed value.*/
    if (Axial_Seperation > max_Axial_separation ||
        Axial_Seperation < min_Axial_separation)
    {
      continue;
    }
    // If Axial Head travels more than the max or min allowed range
    if (IK_output.AxialHeadTranslation < min_AxialHead_translation ||
        IK_output.AxialHeadTranslation > max_AxialHead_translation)

    {
      continue;
    }
    // If Axial Feet travels more than the max or min allowed range
    if (IK_output.AxialFeetTranslation < min_AxialFeet_translation ||
        IK_output.AxialFeetTranslation > max_AxialFeet_translation)
    {
      continue;
    }
    // If Lateral travels more than the max or min allowed range
    if (IK_output.LateralTranslation < min_Lateral_translation ||
        IK_output.LateralTranslation > max_Lateral_translation)
    {
      continue;
    }
    // If Yaw rotates more than the max or min allowed range
    if (IK_output.YawRotation < min_Yaw_rotation ||
        IK_output.YawRotation > max_Yaw_rotation)
    {
      continue;
    }
    // If Pitch rotates more than the max or min allowed range
    if (IK_output.PitchRotation < min_Pitch_rotation ||
        IK_output.PitchRotation > max_Pitch_rotation)
    {
      continue;
    }
    // This statement will increase the size of the Sub-workspace matrix
    // by one to store the new point
    if (counter > 0)
    {
      sub_workspace_rcm_point_set.conservativeResize(
        3, sub_workspace_rcm_point_set.cols() + 1);
    }

    // Storing the IK validated point in the sub-workspace matrix
    sub_workspace_rcm_point_set(0, counter) = tp_in_robot_coordinate(0);
    sub_workspace_rcm_point_set(1, counter) = tp_in_robot_coordinate(1);
    sub_workspace_rcm_point_set(2, counter) = tp_in_robot_coordinate(2);

    counter++;
  }
  if (sub_workspace_rcm_point_set.cols() == 1 &&
      sub_workspace_rcm_point_set(0, 0) == 0. &&
      sub_workspace_rcm_point_set(1, 0) == 0. &&
      sub_workspace_rcm_point_set(2, 0) == 0.)
  {
    std::cerr << "\nThe entry point is NOT reachable! Please select "
                 "another "
                 "point."
              << std::endl;
  }
  return sub_workspace_rcm_point_set;
}

// Method to create the 3D representing the sub-workspace
Eigen::Matrix3Xf ForwardKinematics::GenerateFinalSubworkspacePointset(
  Eigen::Matrix3Xf validated_inverse_kinematic_rcm_pointset,
  Eigen::Vector3d  ep_in_robot_coordinate)
{
  /* Step to create a full representative point cloud based on the
  sub-workspace In this step, additional points will be added starting from
  the Entry Point and passing through each RCM points which account for the
  full probe insertion. The final workspace will be returned to the VTK
  method to generate the 3D mesh for visualization.*/

  // Total number of points in the RCM sub-workspace
  int no_cols = validated_inverse_kinematic_rcm_pointset.cols();
  // Maximum range of motion for the Probe Insertion Axis
  float max_probe_insertion = 40.0;
  // Max point where the treatment can reach past the RCM point
  float distance_past_rcm =
    max_probe_insertion + NeuroKinematics_.RCMToTreatment(2, 3);
  // Vector starting from the entry point and ending at the RCM point,i.e.
  // X2-X1
  Eigen::Vector3d vector_ep_to_tp(0., 0., 0.);
  Eigen::Vector3d rcm_point(0., 0., 0.);
  Eigen::Vector3d coordinate_of_last_point(0., 0., 0.);
  Eigen::Vector3d intersection_point1(0., 0., 0.);
  Eigen::Vector3d intersection_point2(0., 0., 0.);

  /* division is the number of desired points to generate between the EP and
  the last point*/
  int division{5}, counter{0}, counter1{0};

  /* Matrix containing all the points within the sub-workspace starting from
  the EP to the last point*/
  Eigen::Matrix3Xf total_subworkspace_pointset(3, no_cols * division);
  total_subworkspace_pointset = 0 * total_subworkspace_pointset;

  /*To find the coordinate of the point past the RCM, a series of operations
  need to be evoked. A sphere of size equal to "distance_past_rcm" will be
  placed with it's origin at the RCM point. The intersection of the line
  from the EP to the RCM point with this sphere will give the coordinate
  of the point that the treatment will reach after passing the RCM.*/

  /* Below are coefficient to be found for the equation of line and it's
  intersection with the sphere*/
  double a{0}, b{0}, c{0}, t1{0}, t2{0}, x{0}, y{0}, z{0};

  for (i = 0; i < no_cols; i++)
  {
    rcm_point << validated_inverse_kinematic_rcm_pointset(0, i),
      validated_inverse_kinematic_rcm_pointset(1, i),
      validated_inverse_kinematic_rcm_pointset(2, i);
    // Finding the equation of a line for each pair of RCM and Entry points.
    vector_ep_to_tp = rcm_point - ep_in_robot_coordinate;
    a               = (pow(vector_ep_to_tp(0), 2) + pow(vector_ep_to_tp(1), 2) +
         pow(vector_ep_to_tp(2), 2));
    b               = -2 * a;
    c               = a - pow(distance_past_rcm, 2);
    /* coefficients that will be plugged into the eq of line which give the
    intersection points  coefficients that  will be plugged  into the eq of
    line which give the intersection points*/
    t1 = (-b + sqrt(pow(b, 2) - (4 * a * c))) / (2 * a);
    t2 = (-b - sqrt(pow(b, 2) - (4 * a * c))) / (2 * a);

    intersection_point1 << ep_in_robot_coordinate(0) + vector_ep_to_tp(0) * t1,
      ep_in_robot_coordinate(1) + vector_ep_to_tp(1) * t1,
      ep_in_robot_coordinate(2) + vector_ep_to_tp(2) * t1;
    intersection_point2 << ep_in_robot_coordinate(0) + vector_ep_to_tp(0) * t2,
      ep_in_robot_coordinate(1) + vector_ep_to_tp(1) * t2,
      ep_in_robot_coordinate(2) + vector_ep_to_tp(2) * t2;
    double dist1 =
      sqrt(pow(intersection_point1(0) - ep_in_robot_coordinate(0), 2) +
           pow(intersection_point1(1) - ep_in_robot_coordinate(1), 2) +
           pow(intersection_point1(2) - ep_in_robot_coordinate(2), 2));
    double dist2 =
      sqrt(pow(intersection_point2(0) - ep_in_robot_coordinate(0), 2) +
           pow(intersection_point2(1) - ep_in_robot_coordinate(1), 2) +
           pow(intersection_point2(2) - ep_in_robot_coordinate(2), 2));
    if (dist1 > dist2)
    {
      coordinate_of_last_point = intersection_point1;
    }
    else if (dist1 < dist2)
    {
      coordinate_of_last_point = intersection_point2;
    }
    // increments for x
    x = abs(coordinate_of_last_point(0) - ep_in_robot_coordinate(0)) / division;
    // increments for y
    y = abs(coordinate_of_last_point(1) - ep_in_robot_coordinate(1)) / division;
    // increments for z
    z = abs(coordinate_of_last_point(2) - ep_in_robot_coordinate(2)) / division;

    for (j = 1; j <= division; j++)
    {
      if (ep_in_robot_coordinate(0) < coordinate_of_last_point(0))
      {
        total_subworkspace_pointset(0, counter + j - 1) =
          ep_in_robot_coordinate(0) + (x * j);
      }
      else if (ep_in_robot_coordinate(0) > coordinate_of_last_point(0))
      {
        total_subworkspace_pointset(0, counter + j - 1) =
          ep_in_robot_coordinate(0) - (x * j);
      }
      total_subworkspace_pointset(1, counter + j - 1) =
        ep_in_robot_coordinate(1) - (y * j);
      if (ep_in_robot_coordinate(2) < coordinate_of_last_point(2))
      {
        total_subworkspace_pointset(2, counter + j - 1) =
          ep_in_robot_coordinate(2) + (z * j);
      }
      else if (ep_in_robot_coordinate(2) > coordinate_of_last_point(2))
      {
        total_subworkspace_pointset(2, counter + j - 1) =
          ep_in_robot_coordinate(2) - (z * j);
      }
    }
    counter += division;
  }
  return total_subworkspace_pointset;
}

/*Method which applies the transform to the given entry point defined in the
Imager's coordinate frame to define it in the Robot's frame.*/
void ForwardKinematics::CalculateTransform(
  Eigen::Matrix4d registration_inv, Eigen::Vector3d ep_in_imager_coordinate,
  Eigen::Vector3d& ep_in_robot_coordinate)
{
  // Creating a standard vector for matrix multiplication
  Eigen::Vector4d entry_point_robot(ep_in_robot_coordinate(0),
                                    ep_in_robot_coordinate(1),
                                    ep_in_robot_coordinate(2), 1);
  // Creating a standard vector for matrix multiplication
  Eigen::Vector4d entry_point_imager(ep_in_imager_coordinate(0),
                                     ep_in_imager_coordinate(1),
                                     ep_in_imager_coordinate(2), 1);
  // Finding the location of the EP W.R.T Robot's base frame
  entry_point_robot = registration_inv * entry_point_imager;
  // rounding step (to the tenth)
  for (int t = 0; t < 3; t++)
  {
    entry_point_robot(t)      = round(entry_point_robot(t) * 10) / 10;
    ep_in_robot_coordinate(t) = entry_point_robot(t);
  }
}

/* Method takes a 4x4 transformation matrix comrised of [R P;0001], and
extracts the position vector P, and appends it in a 3xN Eigen matrix.*/
void ForwardKinematics::StorePointToEigenMatrix(
  Eigen::Matrix3Xf& point_set, Eigen::Matrix4d transformation_matrix)
{
  int no_of_columns = point_set.cols();
  // The case where the point set is empty
  if (no_of_columns == 1 && point_set(0, 0) == 0. && point_set(1, 0) == 0. &&
      point_set(2, 0) == 0.)
  {
    for (int i = 0; i < 3; i++)
    {
      point_set(i, 0) = transformation_matrix(i, 3);
    }
  }
  // The case where the point set has at least one point in it
  else
  {
    point_set.conservativeResize(3, no_of_columns + 1);
    for (int i = 0; i < 3; i++)
    {
      point_set(i, no_of_columns) = transformation_matrix(i, 3);
    }
  }
}

void ForwardKinematics::StorePointToEigenMatrix(Eigen::Matrix3Xf& point_set,
                                                double x, double y, double z)
{
  int no_of_columns = point_set.cols();
  // The case for the first column
  if (no_of_columns == 1 && point_set(0, 0) == 0. && point_set(1, 0) == 0. &&
      point_set(2, 0) == 0.)
  {
    point_set(0, 0) = x;
    point_set(1, 0) = y;
    point_set(2, 0) = z;
  }
  // The case for all columns other than the first column
  else
  {
    point_set.conservativeResize(3, no_of_columns + 1);
    {
      point_set(0, no_of_columns) = x;
      point_set(1, no_of_columns) = y;
      point_set(2, no_of_columns) = z;
    }
  }
}