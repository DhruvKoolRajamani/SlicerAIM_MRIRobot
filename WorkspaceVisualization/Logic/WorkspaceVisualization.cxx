/**
 * @file WorkspaceVisualization.cxx
 * @author Dhruv Kool Rajamani (dkoolrajamani@wpi.edu)
 * @brief
 * @version 0.1
 * @date 2021-01-20
 *
 *
 */

#include "WorkspaceVisualization.h"
using std::endl;
using std::ofstream;

// A is treatment to tip, where treatment is the piezoelectric element,// A =
// 10mm B is robot to entry, this allows us to specify how close to the patient
// the physical robot can be,// B = 5mm C is cannula to treatment, we define
// this so the robot can compute the cannula length,// C = 5mm D is the robot to
// treatment distance,// D = 41mm Creating an object called Forward for FK In
// the neuroRobot.cpp the specs for the  probe are: 0,0,5,41

WorkspaceVisualization::WorkspaceVisualization(NeuroKinematics& NeuroKinematics)
  : Diff(68), pi(3.141)
{
  // counters
  i       = 0.0;
  j       = 0.0;
  k       = 0.0;
  l       = 0.0;
  ii      = 0.0;
  counter = 0;
  // Min allowed seperation 75mm
  // Max allowed seperatio1f46mm
  Ry             = 0.0;               // Initializing the PitchRotation counter
  RyF_max        = -37.0 * pi / 180;  // in paper is 37.2
  RyF_max_degree = -37.0;
  RyB_max        = +26.0 * pi / 180;  // in paper is  30.6
  RyB_max_degree = 26.0;
  Rx             = 0.0;               // Initializing the YawRotation counter
  Rx_max         = -88.0 * pi / 180;  // Max YawRotation
  Rx_max_degree  = -88.0;
  // Robot axis
  AxialHeadTranslation = 0.0;
  AxialFeetTranslation = 0.0;
  LateralTranslation   = 0.0;
  PitchRotation        = 0.0;
  YawRotation          = 0.0;
  ProbeInsertion       = 0.0;
  ProbeRotation        = 0.0;

  NeuroKinematics_ = NeuroKinematics;
  std::cout << "Constructor Called\n";
}

vtkSmartPointer< vtkPoints > WorkspaceVisualization::get_General_Workspace(
  Eigen::Matrix4d registration, vtkSmartPointer< vtkPoints > points)
{
  // To visualize the transferred points in the slicer without using the
  // Transform Module
  Eigen::Matrix4d registration_inv = registration.inverse();
  std::cerr << "Inverse of the registration matrix is: \n"
            << registration_inv << std::endl;

  // Vector to store points before transformation
  Eigen::Vector4d point(0.0, 0.0, 0.0, 0.0);

  // Object containing the 4x4 transformation matrix
  Neuro_FK_outputs FK{};
  ofstream         myout("General_workspace.xyz");
  /*============================================================================================================
     =============================================FK
     computation============================================
      ==================================================================================================*/
  // Loop for visualizing the top
  ++counter;
  AxialFeetTranslation = 68;
  AxialHeadTranslation = 0;
  double Top_max_travel{-146};
  for (i = Top_max_travel / 100; i > Top_max_travel;
       i += Top_max_travel / 100)  // initial separation 143, min separation 75
                                   // => 143-75 = 68 mm
  {
    AxialHeadTranslation += Top_max_travel / 100;
    AxialFeetTranslation += Top_max_travel / 100;
    for (k = 0.0 - 49.0; k >= -49.0 - 49.0; k -= 7.0)  // max lateral movement
                                                       // 0.0 ~ -49.47 (appx =
                                                       // -49)
    {
      LateralTranslation = k;
      FK                 = NeuroKinematics_.ForwardKinematics(
        AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
        ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
      nan_checker(FK, counter);
      Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
      transferred_point = get_Transform(registration_inv, FK);
      points->InsertNextPoint(transferred_point(0), transferred_point(1),
                              transferred_point(2));
      myout << transferred_point(0) << " " << transferred_point(1) << " "
            << transferred_point(2) << " 0.00 0.00 0.00" << endl;
    }
  }

  // loop for visualizing the bottom
  ++counter;
  for (i = 0, j = -3; i > -87; i -= 8.7, j -= 8.7)  // 75
  {
    AxialHeadTranslation = i;
    AxialFeetTranslation = j;
    for (k = 0.0 - 49.0; k >= -49 - 49.0; k -= 7.0)
    {
      LateralTranslation = k;
      if (k >= -28 - 49.0)
      {

        YawRotation   = Rx_max;
        PitchRotation = RyB_max;
        FK            = NeuroKinematics_.ForwardKinematics(
          AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
          ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
        nan_checker(FK, counter);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, FK);
        points->InsertNextPoint(transferred_point(0), transferred_point(1),
                                transferred_point(2));
        myout << transferred_point(0) << " " << transferred_point(1) << " "
              << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      }
      if (k <= -7 - 49.0)
      {
        YawRotation   = Rx_max;
        PitchRotation = RyF_max;
        FK            = NeuroKinematics_.ForwardKinematics(
          AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
          ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
        nan_checker(FK, counter);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, FK);
        points->InsertNextPoint(transferred_point(0), transferred_point(1),
                                transferred_point(2));
        myout << transferred_point(0) << " " << transferred_point(1) << " "
              << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      }
    }
  }

  YawRotation          = 0;
  PitchRotation        = 0;
  AxialFeetTranslation = -3;
  AxialHeadTranslation = 0;

  // Loop for creating the head face
  ++counter;
  for (j = 7.1; j <= 71; j += 7.1)
  {
    AxialFeetTranslation += 7.1;

    for (k = 0.0 - 49.0; k >= -49.0 - 49.0; k -= 7.0)
    {
      LateralTranslation = k;

      if (j == 71)  // Top level
      {
        if (k == 0 - 49.0)
        {
          for (i = 0; i <= RyB_max_degree; i += 5.2)
          {
            PitchRotation = i * pi / 180;
            for (ii = 0; ii >= Rx_max_degree; ii += Rx_max_degree / 10)
            {
              YawRotation = ii * pi / 180;
              FK          = NeuroKinematics_.ForwardKinematics(
                AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
                ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
              nan_checker(FK, counter);
              Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
              transferred_point = get_Transform(registration_inv, FK);
              points->InsertNextPoint(transferred_point(0),
                                      transferred_point(1),
                                      transferred_point(2));
              myout << transferred_point(0) << " " << transferred_point(1)
                    << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
            }
          }
        }
        else if (k == -49.0 - 49.0)
        {
          for (i = 0; i >= RyF_max_degree; i -= 7.4)
          {
            PitchRotation = i * pi / 180;
            for (ii = 0; ii >= Rx_max_degree; ii += Rx_max_degree / 10)
            {
              YawRotation = ii * pi / 180;
              FK          = NeuroKinematics_.ForwardKinematics(
                AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
                ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
              nan_checker(FK, counter);
              Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
              transferred_point = get_Transform(registration_inv, FK);
              points->InsertNextPoint(transferred_point(0),
                                      transferred_point(1),
                                      transferred_point(2));
              myout << transferred_point(0) << " " << transferred_point(1)
                    << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
            }
          }
        }
        else  // if (k < 0.0 && k > -49.0) // between the bore and the face
        {
          for (l = Rx_max_degree / 8.8; l >= Rx_max_degree;
               l += Rx_max_degree / 8.8)
          {
            PitchRotation = 0;
            YawRotation   = l * pi / 180;
            FK            = NeuroKinematics_.ForwardKinematics(
              AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
              ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
            nan_checker(FK, counter);
            Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
            transferred_point = get_Transform(registration_inv, FK);
            points->InsertNextPoint(transferred_point(0), transferred_point(1),
                                    transferred_point(2));
            myout << transferred_point(0) << " " << transferred_point(1) << " "
                  << transferred_point(2) << " 0.00 0.00 0.00" << endl;
          }
        }
      }
      else  // Any other lvl from bottom to just a lvl before the top
      {
        if (k == 0 - 49.0)  // Creating corner bore side
        {
          YawRotation = Rx_max;
          for (l = 0; l <= RyB_max_degree; l += 5.2)  // lvl one bore side yaw
                                                      // lowered pitch lowering
          {
            PitchRotation = l * pi / 180;
            FK            = NeuroKinematics_.ForwardKinematics(
              AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
              ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
            nan_checker(FK, counter);
            Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
            transferred_point = get_Transform(registration_inv, FK);
            points->InsertNextPoint(transferred_point(0), transferred_point(1),
                                    transferred_point(2));
            myout << transferred_point(0) << " " << transferred_point(1) << " "
                  << transferred_point(2) << " 0.00 0.00 0.00" << endl;
          }
        }

        else if (k == -49.0 - 49.0)  // Creating corner face side
        {
          YawRotation = Rx_max;
          for (l = 0; l >= RyF_max_degree; l -= 7.4)  // lvl one face side yaw
                                                      // lowered pitch
                                                      // increasing
          {
            PitchRotation = l * pi / 180;
            FK            = NeuroKinematics_.ForwardKinematics(
              AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
              ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
            nan_checker(FK, counter);
            Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
            transferred_point = get_Transform(registration_inv, FK);
            points->InsertNextPoint(transferred_point(0), transferred_point(1),
                                    transferred_point(2));
            myout << transferred_point(0) << " " << transferred_point(1) << " "
                  << transferred_point(2) << " 0.00 0.00 0.00" << endl;
          }
        }

        else  // Space between two corners
        {
          YawRotation   = Rx_max;
          PitchRotation = 0;
          FK            = NeuroKinematics_.ForwardKinematics(
            AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
            ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
          nan_checker(FK, counter);
          Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
          transferred_point = get_Transform(registration_inv, FK);
          points->InsertNextPoint(transferred_point(0), transferred_point(1),
                                  transferred_point(2));
          myout << transferred_point(0) << " " << transferred_point(1) << " "
                << transferred_point(2) << " 0.00 0.00 0.00" << endl;
        }
      }
    }
  }
  YawRotation          = 0;
  PitchRotation        = 0;
  AxialFeetTranslation = -89;
  AxialHeadTranslation = -86;

  // Loop for creating the feet face
  ++counter;
  // only for the bottom level at Axial Head of -86 and Axial Feet of -89
  for (k = 0.0 - 49.0; k >= -49.0 - 49.0; k -= 7.0)
  {
    LateralTranslation = k;
    // if on the bore side
    if (k >= -28 - 49.0)
    {
      PitchRotation = RyB_max;
      for (i = 0; i >= Rx_max_degree; i -= 8.8)
      {
        YawRotation = i * pi / 180;
        FK          = NeuroKinematics_.ForwardKinematics(
          AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
          ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
        nan_checker(FK, counter);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, FK);
        points->InsertNextPoint(transferred_point(0), transferred_point(1),
                                transferred_point(2));
        myout << transferred_point(0) << " " << transferred_point(1) << " "
              << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      }
    }
    if (k <= -7 - 49.0)
    {
      PitchRotation = RyF_max;
      for (i = 0; i >= Rx_max_degree; i -= 8.8)
      {
        YawRotation = i * pi / 180;
        FK          = NeuroKinematics_.ForwardKinematics(
          AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
          ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
        nan_checker(FK, counter);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, FK);
        points->InsertNextPoint(transferred_point(0), transferred_point(1),
                                transferred_point(2));
        myout << transferred_point(0) << " " << transferred_point(1) << " "
              << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      }
    }
  }

  YawRotation   = 0;
  PitchRotation = 0;
  ++counter;
  // Feet face for levels other than the first
  for (j = -7.1; j >= -71; j -= 7.1)
  {
    AxialHeadTranslation -= 7.1;

    for (k = 0.0 - 49.0; k >= -49.0 - 49.0; k -= 7.0)
    {
      LateralTranslation = k;
      // if on the bore side
      if (k >= -28 - 49.0)
      {
        PitchRotation = RyB_max;
        YawRotation   = 0;
        FK            = NeuroKinematics_.ForwardKinematics(
          AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
          ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
        nan_checker(FK, counter);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, FK);
        points->InsertNextPoint(transferred_point(0), transferred_point(1),
                                transferred_point(2));
        myout << transferred_point(0) << " " << transferred_point(1) << " "
              << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      }
      if (k <= -7 - 49.0)
      {
        PitchRotation = RyF_max;
        YawRotation   = 0;
        FK            = NeuroKinematics_.ForwardKinematics(
          AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
          ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
        nan_checker(FK, counter);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, FK);
        points->InsertNextPoint(transferred_point(0), transferred_point(1),
                                transferred_point(2));
        myout << transferred_point(0) << " " << transferred_point(1) << " "
              << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      }
    }
  }

  // loop for creating the sides
  ++counter;
  AxialFeetTranslation = -3;
  AxialHeadTranslation = 0;
  LateralTranslation   = 0;
  YawRotation          = 0;
  PitchRotation        = 0;
  double AxialHeadTranslation_old{};
  double AxialFeetTranslation_old{};
  double min_travel{-86};  // The max that the robot can move in z direction
                           // when at lowest height ( at each hight min level is
                           // changed)
  double max_travel{-157};  // 157 The max that the robot can move in z
                            // direction when at highest height
  for (j = 0; j <= 71; j += 7.1)
  {
    AxialFeetTranslation += j;  // For each loop it will lift the base by a
                                // constant value
    min_travel -= j;  // Takes care of the amount of Axial travel for the Axial
                      // head and feet

    AxialFeetTranslation_old = AxialFeetTranslation;
    AxialHeadTranslation_old = AxialHeadTranslation;
    for (ii = 0; ii >= min_travel; ii += (min_travel / 10))  // loop to move the
                                                             // base from Head
                                                             // to feet based on
                                                             // the allowable
                                                             // max movement
                                                             // range
                                                             // (min_travel)
    {
      AxialHeadTranslation += ii;
      AxialFeetTranslation += ii;

      for (k = 0.0 - 49.0; k >= -49.0 - 49.0; k -= 49.0)
      {
        LateralTranslation = k;
        // For the side towards bore
        if (k == 0.0 - 49.0)
        {
          // Conditions based on the position of the base
          // Only for the first level
          if (j == 0)
          {
            PitchRotation = RyB_max;

            // 1) Beginning of the track
            if (ii == 0)
            {
              for (i = 0; i > Rx_max_degree; i += Rx_max_degree / 10)
              {
                YawRotation = i * pi / 180;
                FK          = NeuroKinematics_.ForwardKinematics(
                  AxialHeadTranslation, AxialFeetTranslation,
                  LateralTranslation, ProbeInsertion, ProbeRotation,
                  PitchRotation, YawRotation);
                nan_checker(FK, counter);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0),
                                        transferred_point(1),
                                        transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1)
                      << " " << transferred_point(2) << " 0.00 0.00 0.00"
                      << endl;
              }
            }
            // 2) End of the track
            else if (ii == min_travel)
            {
              YawRotation = 0;
              FK          = NeuroKinematics_.ForwardKinematics(
                AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
                ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
              nan_checker(FK, counter);
              Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
              transferred_point = get_Transform(registration_inv, FK);
              points->InsertNextPoint(transferred_point(0),
                                      transferred_point(1),
                                      transferred_point(2));
              myout << transferred_point(0) << " " << transferred_point(1)
                    << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
            }
            // 3) In between beginning and end
            else
            {
              for (i = 0; i > Rx_max_degree; i += Rx_max_degree / 10)
              {
                YawRotation = i * pi / 180;
                FK          = NeuroKinematics_.ForwardKinematics(
                  AxialHeadTranslation, AxialFeetTranslation,
                  LateralTranslation, ProbeInsertion, ProbeRotation,
                  PitchRotation, YawRotation);
                nan_checker(FK, counter);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0),
                                        transferred_point(1),
                                        transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1)
                      << " " << transferred_point(2) << " 0.00 0.00 0.00"
                      << endl;
              }
            }
          }

          // Only for the topmost level
          else if (j == 71)
          {
            // In between beginning and end
            if (ii < 0 && ii > min_travel)
            {
              for (l = RyB_max_degree / 5; l <= RyB_max_degree;
                   l += RyB_max_degree / 5)
              {
                PitchRotation = l * pi / 180;
                YawRotation   = 0;

                FK = NeuroKinematics_.ForwardKinematics(
                  AxialHeadTranslation, AxialFeetTranslation,
                  LateralTranslation, ProbeInsertion, ProbeRotation,
                  PitchRotation, YawRotation);
                nan_checker(FK, counter);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0),
                                        transferred_point(1),
                                        transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1)
                      << " " << transferred_point(2) << " 0.00 0.00 0.00"
                      << endl;
              }
            }
          }

          // For all levels between bottom and top levels
          else
          {
            PitchRotation = RyB_max;

            // 1) Beginning of the track
            if (ii == 0)
            {
              for (i = 0; i > Rx_max_degree; i += Rx_max_degree / 10)
              {
                YawRotation = i * pi / 180;
                FK          = NeuroKinematics_.ForwardKinematics(
                  AxialHeadTranslation, AxialFeetTranslation,
                  LateralTranslation, ProbeInsertion, ProbeRotation,
                  PitchRotation, YawRotation);
                nan_checker(FK, counter);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0),
                                        transferred_point(1),
                                        transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1)
                      << " " << transferred_point(2) << " 0.00 0.00 0.00"
                      << endl;
              }
            }

            // 2) In between beginning and end
            else if (ii < 0 && ii > min_travel)
            {
              for (i = 0; i >= Rx_max_degree; i += Rx_max_degree / 10)
              {
                YawRotation = i * pi / 180;
                FK          = NeuroKinematics_.ForwardKinematics(
                  AxialHeadTranslation, AxialFeetTranslation,
                  LateralTranslation, ProbeInsertion, ProbeRotation,
                  PitchRotation, YawRotation);
                nan_checker(FK, counter);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0),
                                        transferred_point(1),
                                        transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1)
                      << " " << transferred_point(2) << " 0.00 0.00 0.00"
                      << endl;
              }
            }
          }
        }
        // For the side towards the patient
        else if (k == -49.0 - 49.0)
        {
          // Conditions based on the position of the base
          // Only for the first level
          if (j == 0)
          {
            PitchRotation = RyF_max;

            // 1) Beginning of the track
            if (ii == 0)
            {
              for (i = 0; i > Rx_max_degree; i += Rx_max_degree / 11)
              {
                YawRotation = i * pi / 180;
                FK          = NeuroKinematics_.ForwardKinematics(
                  AxialHeadTranslation, AxialFeetTranslation,
                  LateralTranslation, ProbeInsertion, ProbeRotation,
                  PitchRotation, YawRotation);
                nan_checker(FK, counter);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0),
                                        transferred_point(1),
                                        transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1)
                      << " " << transferred_point(2) << " 0.00 0.00 0.00"
                      << endl;
              }
            }
            // 2) End of the track
            else if (ii == min_travel)
            {
              YawRotation = 0;
              FK          = NeuroKinematics_.ForwardKinematics(
                AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
                ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
              nan_checker(FK, counter);
              Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
              transferred_point = get_Transform(registration_inv, FK);
              points->InsertNextPoint(transferred_point(0),
                                      transferred_point(1),
                                      transferred_point(2));
              myout << transferred_point(0) << " " << transferred_point(1)
                    << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
            }
            // 3) In between beginning and end
            else
            {
              for (i = 0; i > Rx_max_degree; i += Rx_max_degree / 10)
              {
                YawRotation = i * pi / 180;
                FK          = NeuroKinematics_.ForwardKinematics(
                  AxialHeadTranslation, AxialFeetTranslation,
                  LateralTranslation, ProbeInsertion, ProbeRotation,
                  PitchRotation, YawRotation);
                nan_checker(FK, counter);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0),
                                        transferred_point(1),
                                        transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1)
                      << " " << transferred_point(2) << " 0.00 0.00 0.00"
                      << endl;
              }
            }
          }

          // Only for the topmost level
          else if (j == 71)
          {
            // In between beginning and end
            if (ii < 0 && ii > min_travel)
            {
              for (l = RyF_max_degree / 3; l >= RyF_max_degree;
                   l += RyF_max_degree / 3)
              {
                PitchRotation = l * pi / 180;
                YawRotation   = 0;

                FK = NeuroKinematics_.ForwardKinematics(
                  AxialHeadTranslation, AxialFeetTranslation,
                  LateralTranslation, ProbeInsertion, ProbeRotation,
                  PitchRotation, YawRotation);
                nan_checker(FK, counter);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0),
                                        transferred_point(1),
                                        transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1)
                      << " " << transferred_point(2) << " 0.00 0.00 0.00"
                      << endl;
              }
            }
          }

          // For all levels between bottom and top levels
          else
          {
            PitchRotation = RyF_max;

            // 1) Beginning of the track
            if (ii == 0)
            {
              for (i = 0; i > Rx_max_degree; i += Rx_max_degree / 10)
              {
                YawRotation = i * pi / 180;
                FK          = NeuroKinematics_.ForwardKinematics(
                  AxialHeadTranslation, AxialFeetTranslation,
                  LateralTranslation, ProbeInsertion, ProbeRotation,
                  PitchRotation, YawRotation);
                nan_checker(FK, counter);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0),
                                        transferred_point(1),
                                        transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1)
                      << " " << transferred_point(2) << " 0.00 0.00 0.00"
                      << endl;
              }
            }

            // 2) In between beginning and end
            else if (ii < 0 && ii > min_travel)
            {
              for (i = 0; i >= Rx_max_degree; i += Rx_max_degree / 10)
              {
                YawRotation = i * pi / 180;
                FK          = NeuroKinematics_.ForwardKinematics(
                  AxialHeadTranslation, AxialFeetTranslation,
                  LateralTranslation, ProbeInsertion, ProbeRotation,
                  PitchRotation, YawRotation);
                nan_checker(FK, counter);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0),
                                        transferred_point(1),
                                        transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1)
                      << " " << transferred_point(2) << " 0.00 0.00 0.00"
                      << endl;
              }
            }
          }
        }
      }
      // bringing back the heads to the starting position
      AxialFeetTranslation = AxialFeetTranslation_old;
      AxialHeadTranslation = AxialHeadTranslation_old;
    }
    // resetting the location of the Axial Head and Feet to the home position
    AxialHeadTranslation = 0;
    AxialFeetTranslation = -3;
    min_travel           = -86;
  }
  std::cout << "# of points: " << points->GetNumberOfPoints();
  myout.close();
  return points;
}

vtkSmartPointer< vtkPoints > WorkspaceVisualization::get_Sub_Workspace(
  Eigen::Matrix4d registration, Eigen::Vector4d entryPointScanner)
{
  // Create points.
  vtkSmartPointer< vtkPoints > points_RCM = vtkSmartPointer< vtkPoints >::New();
  // Object containing the 4x4 transformation matrix
  Neuro_FK_outputs FK{};
  //----------------------------------FK computation
  //--------------------------------------------------------
  AxialHeadTranslation = 0.0;
  AxialFeetTranslation = 0.0;
  LateralTranslation   = 0.0;
  PitchRotation        = 0.0;
  YawRotation          = 0.0;
  ProbeInsertion       = 31.5;
  ProbeRotation        = 0.0;
  // loop for visualizing the bottom
  double i{}, j{}, k{}, l{};                      // initializing the counters
  for (i = 0, j = 0; i < 87; i += 8.6, j += 8.6)  // 75
  {
    AxialFeetTranslation = i;
    AxialHeadTranslation = j;
    for (k = 0; k <= 37.5; k += 7.5)
    {
      LateralTranslation = k;
      FK                 = NeuroKinematics_.ForwardKinematics(
        AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
        ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
      points_RCM->InsertNextPoint(FK.zFrameToTreatment(0, 3),
                                  FK.zFrameToTreatment(1, 3),
                                  FK.zFrameToTreatment(2, 3));
    }
  }

  // Loop for visualizing the top
  for (i = 0, j = -71; i < 157; i += 6, j += 6)  // 75
  {
    AxialFeetTranslation = i;
    AxialHeadTranslation = j;
    for (k = 0; k <= 37.5; k += 7.5)
    {
      LateralTranslation = k;
      FK                 = NeuroKinematics_.ForwardKinematics(
        AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
        ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
      points_RCM->InsertNextPoint(FK.zFrameToTreatment(0, 3),
                                  FK.zFrameToTreatment(1, 3),
                                  FK.zFrameToTreatment(2, 3));
    }
  }
  const double Diff{71};
  AxialFeetTranslation = 0;
  AxialHeadTranslation = 0;

  int nan_checker_row{};
  int nan_checker_col{};
  i = 0;
  j = -1;
  k = 0;
  // Loop for creating the feet face
  for (j = -1; Diff > abs(AxialHeadTranslation - AxialFeetTranslation); --j)
  {
    AxialHeadTranslation = j;
    for (k = 0; k <= 37.5; k += 7.5)
    {
      LateralTranslation = k;
      FK                 = NeuroKinematics_.ForwardKinematics(
        AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
        ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
      points_RCM->InsertNextPoint(FK.zFrameToTreatment(0, 3),
                                  FK.zFrameToTreatment(1, 3),
                                  FK.zFrameToTreatment(2, 3));
    }
  }

  // Loop for creating the Head face
  AxialHeadTranslation = 86;
  AxialFeetTranslation = 86;
  nan_checker_row      = 0;
  nan_checker_col      = 0;
  i                    = 86;
  j                    = 86;
  k                    = 0;

  for (i = 87; Diff > abs(AxialHeadTranslation - AxialFeetTranslation); i += 2)
  {
    AxialFeetTranslation = i;
    for (k = 0; k <= 37.5; k += 7.5)
    {
      LateralTranslation = k;
      FK                 = NeuroKinematics_.ForwardKinematics(
        AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
        ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
      points_RCM->InsertNextPoint(FK.zFrameToTreatment(0, 3),
                                  FK.zFrameToTreatment(1, 3),
                                  FK.zFrameToTreatment(2, 3));
    }
  }

  // loop for creating the sides
  AxialFeetTranslation = 0;
  AxialHeadTranslation = 0;
  LateralTranslation   = 0;
  nan_checker_row      = 0;
  nan_checker_col      = 0;
  i                    = 0;
  j                    = -1;
  k                    = 0;

  // double jj{};
  double min_travel{86};
  double max_travel{200};
  // double Old_AxialHeadTranslation{};
  for (j = -1; Diff > abs(j); --j)
  {
    AxialHeadTranslation = j;
    ++min_travel;

    for (ii = 0; ii <= min_travel && min_travel <= max_travel; ii += +4)
    {
      AxialHeadTranslation += 4;
      AxialFeetTranslation += 4;

      for (k = 0; k <= 37.5; k += 37.5)
      {
        LateralTranslation = k;
        FK                 = NeuroKinematics_.ForwardKinematics(
          AxialHeadTranslation, AxialFeetTranslation, LateralTranslation,
          ProbeInsertion, ProbeRotation, PitchRotation, YawRotation);
        points_RCM->InsertNextPoint(FK.zFrameToTreatment(0, 3),
                                    FK.zFrameToTreatment(1, 3),
                                    FK.zFrameToTreatment(2, 3));
      }
    }
    AxialHeadTranslation = 0;
    AxialFeetTranslation = 0;
    LateralTranslation   = 0;
  }

  return points_RCM;
}

// Method to search for NaN values in the FK output
void WorkspaceVisualization::nan_checker(Neuro_FK_outputs FK, int& counter)
{
  bool Flag{false};
  int  nan_checker_row{};
  int  nan_checker_col{};
  for (nan_checker_row = 0; nan_checker_row < 4; ++nan_checker_row)  // Loop for
                                                                     // checking
                                                                     // NaN
  {
    if (Flag == false)
    {
      for (nan_checker_col = 0; nan_checker_col < 4; ++nan_checker_col)
      {
        if (Flag == false)
        {
          if (isnan(FK.zFrameToTreatment(nan_checker_row, nan_checker_col)))
          {
            // std::cout << "\nrow: " << nan_checker_row << " ,column: " <<
            // nan_checker_col << " is nan!\n";
            std::cout << "\ncounter is: " << counter << std::endl;
            Flag = true;
          }
        }
        else
        {
          break;
        }
      }
    }
    else
    {
      break;
    }
  }
}

Eigen::Vector4d WorkspaceVisualization::get_Transform(
  Eigen::Matrix4d registration_inv, Neuro_FK_outputs FK)
{
  // Vector that stores the End effector's position defined in the robot's
  // Z-frame
  Eigen::Vector4d point(0.0, 0.0, 0.0, 1.0);
  for (int t = 0; t < 3; t++)
  {
    point(t) = FK.zFrameToTreatment(t, 3);
  }

  // Vector that stores the transferred points defined W.R.T the imager's frame
  Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);

  // Finding the corresponding point W.R.T the imager's frame
  transferred_point = registration_inv * point;

  // rounding step (to the tenth)
  for (int t = 0; t < 3; t++)
  {
    transferred_point(t) = round(transferred_point(t) * 10) / 10;
  }
  return transferred_point;
}
