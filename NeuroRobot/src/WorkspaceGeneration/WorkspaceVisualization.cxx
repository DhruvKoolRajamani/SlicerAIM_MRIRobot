#include "../../include/WorkspaceGeneration/WorkspaceVisualization.h"
using std::endl;
using std::ofstream;

// A is treatment to tip, where treatment is the piezoelectric element,// A = 10mm
// B is robot to entry, this allows us to specify how close to the patient the physical robot can be,// B = 5mm
// C is cannula to treatment, we define this so the robot can compute the cannula length,// C = 5mm
// D is the robot to treatment distance,// D = 41mm
// Creating an object called Forward for FK
// In the neuroRobot.cpp the specs for the  probe are: 0,0,5,41

// Constructor
ForwardKinematics::ForwardKinematics(NeuroKinematics &NeuroKinematics) : Diff(68), pi(3.141)
{
  //counters
  i = 0.0;
  j = 0.0;
  k = 0.0;
  l = 0.0;
  ii = 0.0;
  counter = 0;
  // Min allowed seperation 75mm
  // Max allowed seperatio1f46mm
  Ry = 0.0;                   // Initializing the PitchRotation counter
  RyF_max = -37.0 * pi / 180; // in paper is 37.2
  RyF_max_degree = -37.0;
  RyB_max = +26.0 * pi / 180; // in paper is  30.6
  RyB_max_degree = 26.0;
  Rx = 0.0;                  // Initializing the YawRotation counter
  Rx_max = -88.0 * pi / 180; // Max YawRotation
  Rx_max_degree = -88.0;
  // Robot axis
  AxialHeadTranslation = 0.0;
  AxialFeetTranslation = 0.0;
  LateralTranslation = 0.0;
  PitchRotation = 0.0;
  YawRotation = 0.0;
  ProbeInsertion = 0.0;
  ProbeRotation = 0.0;

  NeuroKinematics_ = NeuroKinematics;
}

// Method to Create the surface mesh for General reachable Workspace visualization
vtkSmartPointer<vtkPoints> ForwardKinematics::get_General_Workspace(Eigen::Matrix4d registration, vtkSmartPointer<vtkPoints> points)
{
  // To visualize the transferred points in the slicer without using the Transform Module
  Eigen::Matrix4d registration_inv = registration.inverse();
  std::cerr << "Inverse of the registration matrix is: \n"
            << registration_inv << std::endl;

  // Vector to store points before transformation
  Eigen::Vector4d point(0.0, 0.0, 0.0, 0.0);

  // Object containing the 4x4 transformation matrix
  Neuro_FK_outputs FK{};
  ofstream myout("General_workspace.xyz");
  /*============================================================================================================
     =============================================FK computation============================================
      ==================================================================================================*/
  // Loop for visualizing the top
  AxialFeetTranslation = 68;
  AxialHeadTranslation = 0;
  double Top_max_travel{-146};
  for (i = Top_max_travel / 100; i > Top_max_travel; i += Top_max_travel / 100) // initial separation 143, min separation 75 => 143-75 = 68 mm
  {
    AxialHeadTranslation += Top_max_travel / 100;
    AxialFeetTranslation += Top_max_travel / 100;
    for (k = 0.0 - 49.0; k >= -49.0 - 49.0; k -= 7.0) // max lateral movement 0.0 ~ -49.47 (appx = -49)
    {
      LateralTranslation = k;
      FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                              LateralTranslation, ProbeInsertion,
                                              ProbeRotation, PitchRotation, YawRotation);
      Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
      transferred_point = get_Transform(registration_inv, FK);
      points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
      myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
    }
  }

  // loop for visualizing the bottom
  for (i = 0, j = -3; i > -87; i -= 8.7, j -= 8.7) //75
  {
    AxialHeadTranslation = i;
    AxialFeetTranslation = j;
    for (k = 0.0 - 49.0; k >= -49 - 49.0; k -= 7.0)
    {
      LateralTranslation = k;
      if (k >= -28 - 49.0)
      {

        YawRotation = Rx_max;
        PitchRotation = RyB_max;
        FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                LateralTranslation, ProbeInsertion,
                                                ProbeRotation, PitchRotation, YawRotation);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, FK);
        points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
        myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      }
      if (k <= -7 - 49.0)
      {
        YawRotation = Rx_max;
        PitchRotation = RyF_max;
        FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                LateralTranslation, ProbeInsertion,
                                                ProbeRotation, PitchRotation, YawRotation);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, FK);
        points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
        myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      }
    }
  }

  YawRotation = 0;
  PitchRotation = 0;
  AxialFeetTranslation = -3;
  AxialHeadTranslation = 0;

  // Loop for creating the head face
  for (j = 7.1; j <= 71; j += 7.1)
  {
    AxialFeetTranslation += 7.1;

    for (k = 0.0 - 49.0; k >= -49.0 - 49.0; k -= 7.0)
    {
      LateralTranslation = k;

      if (j == 71) // Top level
      {
        if (k == 0 - 49.0)
        {
          for (i = 0; i <= RyB_max_degree; i += 5.2)
          {
            PitchRotation = i * pi / 180;
            for (ii = 0; ii >= Rx_max_degree; ii += Rx_max_degree / 10)
            {
              YawRotation = ii * pi / 180;
              FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                      LateralTranslation, ProbeInsertion,
                                                      ProbeRotation, PitchRotation, YawRotation);
              Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
              transferred_point = get_Transform(registration_inv, FK);
              points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
              myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
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
              FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                      LateralTranslation, ProbeInsertion,
                                                      ProbeRotation, PitchRotation, YawRotation);
              Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
              transferred_point = get_Transform(registration_inv, FK);
              points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
              myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
            }
          }
        }
        else //if (k < 0.0 && k > -49.0) // between the bore and the face
        {
          for (l = Rx_max_degree / 8.8; l >= Rx_max_degree; l += Rx_max_degree / 8.8)
          {
            PitchRotation = 0;
            YawRotation = l * pi / 180;
            FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                    LateralTranslation, ProbeInsertion,
                                                    ProbeRotation, PitchRotation, YawRotation);
            Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
            transferred_point = get_Transform(registration_inv, FK);
            points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
            myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
          }
        }
      }
      else // Any other lvl from bottom to just a lvl before the top
      {
        if (k == 0 - 49.0) // Creating corner bore side
        {
          YawRotation = Rx_max;
          for (l = 0; l <= RyB_max_degree; l += 5.2) // lvl one bore side yaw lowered pitch lowering
          {
            PitchRotation = l * pi / 180;
            FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                    LateralTranslation, ProbeInsertion,
                                                    ProbeRotation, PitchRotation, YawRotation);
            Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
            transferred_point = get_Transform(registration_inv, FK);
            points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
            myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
          }
        }

        else if (k == -49.0 - 49.0) // Creating corner face side
        {
          YawRotation = Rx_max;
          for (l = 0; l >= RyF_max_degree; l -= 7.4) // lvl one face side yaw lowered pitch increasing
          {
            PitchRotation = l * pi / 180;
            FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                    LateralTranslation, ProbeInsertion,
                                                    ProbeRotation, PitchRotation, YawRotation);
            Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
            transferred_point = get_Transform(registration_inv, FK);
            points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
            myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
          }
        }

        else // Space between two corners
        {
          YawRotation = Rx_max;
          PitchRotation = 0;
          FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                  LateralTranslation, ProbeInsertion,
                                                  ProbeRotation, PitchRotation, YawRotation);
          Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
          transferred_point = get_Transform(registration_inv, FK);
          points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
          myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
        }
      }
    }
  }
  YawRotation = 0;
  PitchRotation = 0;
  AxialFeetTranslation = -89;
  AxialHeadTranslation = -86;

  // Loop for creating the feet face
  //only for the bottom level at Axial Head of -86 and Axial Feet of -89
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
        FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                LateralTranslation, ProbeInsertion,
                                                ProbeRotation, PitchRotation, YawRotation);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, FK);
        points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
        myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      }
    }
    if (k <= -7 - 49.0)
    {
      PitchRotation = RyF_max;
      for (i = 0; i >= Rx_max_degree; i -= 8.8)
      {
        YawRotation = i * pi / 180;
        FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                LateralTranslation, ProbeInsertion,
                                                ProbeRotation, PitchRotation, YawRotation);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, FK);
        points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
        myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      }
    }
  }

  YawRotation = 0;
  PitchRotation = 0;
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
        YawRotation = 0;
        FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                LateralTranslation, ProbeInsertion,
                                                ProbeRotation, PitchRotation, YawRotation);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, FK);
        points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
        myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      }
      if (k <= -7 - 49.0)
      {
        PitchRotation = RyF_max;
        YawRotation = 0;
        FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                LateralTranslation, ProbeInsertion,
                                                ProbeRotation, PitchRotation, YawRotation);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, FK);
        points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
        myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      }
    }
  }

  //loop for creating the sides
  AxialFeetTranslation = -3;
  AxialHeadTranslation = 0;
  LateralTranslation = 0;
  YawRotation = 0;
  PitchRotation = 0;
  double AxialHeadTranslation_old{};
  double AxialFeetTranslation_old{};
  double min_travel{-86};  // The max that the robot can move in z direction when at lowest height ( at each hight min level is changed)
  double max_travel{-157}; //157 The max that the robot can move in z direction when at highest height
  for (j = 0; j <= 71; j += 7.1)
  {
    AxialFeetTranslation += j; // For each loop it will lift the base by a constant value
    min_travel -= j;           // Takes care of the amount of Axial travel for the Axial head and feet

    AxialFeetTranslation_old = AxialFeetTranslation;
    AxialHeadTranslation_old = AxialHeadTranslation;
    for (ii = 0; ii >= min_travel; ii += (min_travel / 10)) // loop to move the base from Head to feet based on the allowable max movement range (min_travel)
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
                FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                        LateralTranslation, ProbeInsertion,
                                                        ProbeRotation, PitchRotation, YawRotation);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
              }
            }
            // 2) End of the track
            else if (ii == min_travel)
            {
              YawRotation = 0;
              FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                      LateralTranslation, ProbeInsertion,
                                                      ProbeRotation, PitchRotation, YawRotation);
              Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
              transferred_point = get_Transform(registration_inv, FK);
              points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
              myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
            }
            // 3) In between beginning and end
            else
            {
              for (i = 0; i > Rx_max_degree; i += Rx_max_degree / 10)
              {
                YawRotation = i * pi / 180;
                FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                        LateralTranslation, ProbeInsertion,
                                                        ProbeRotation, PitchRotation, YawRotation);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
              }
            }
          }

          // Only for the topmost level
          else if (j == 71)
          {
            // In between beginning and end
            if (ii < 0 && ii > min_travel)
            {
              for (l = RyB_max_degree / 5; l <= RyB_max_degree; l += RyB_max_degree / 5)
              {
                PitchRotation = l * pi / 180;
                YawRotation = 0;

                FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                        LateralTranslation, ProbeInsertion,
                                                        ProbeRotation, PitchRotation, YawRotation);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
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
                FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                        LateralTranslation, ProbeInsertion,
                                                        ProbeRotation, PitchRotation, YawRotation);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
              }
            }

            // 2) In between beginning and end
            else if (ii < 0 && ii > min_travel)
            {
              for (i = 0; i >= Rx_max_degree; i += Rx_max_degree / 10)
              {
                YawRotation = i * pi / 180;
                FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                        LateralTranslation, ProbeInsertion,
                                                        ProbeRotation, PitchRotation, YawRotation);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
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
                FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                        LateralTranslation, ProbeInsertion,
                                                        ProbeRotation, PitchRotation, YawRotation);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
              }
            }
            // 2) End of the track
            else if (ii == min_travel)
            {
              YawRotation = 0;
              FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                      LateralTranslation, ProbeInsertion,
                                                      ProbeRotation, PitchRotation, YawRotation);
              Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
              transferred_point = get_Transform(registration_inv, FK);
              points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
              myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
            }
            // 3) In between beginning and end
            else
            {
              for (i = 0; i > Rx_max_degree; i += Rx_max_degree / 10)
              {
                YawRotation = i * pi / 180;
                FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                        LateralTranslation, ProbeInsertion,
                                                        ProbeRotation, PitchRotation, YawRotation);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
              }
            }
          }

          // Only for the topmost level
          else if (j == 71)
          {
            // In between beginning and end
            if (ii < 0 && ii > min_travel)
            {
              for (l = RyF_max_degree / 3; l >= RyF_max_degree; l += RyF_max_degree / 3)
              {
                PitchRotation = l * pi / 180;
                YawRotation = 0;

                FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                        LateralTranslation, ProbeInsertion,
                                                        ProbeRotation, PitchRotation, YawRotation);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
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
                FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                        LateralTranslation, ProbeInsertion,
                                                        ProbeRotation, PitchRotation, YawRotation);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
              }
            }

            // 2) In between beginning and end
            else if (ii < 0 && ii > min_travel)
            {
              for (i = 0; i >= Rx_max_degree; i += Rx_max_degree / 10)
              {
                YawRotation = i * pi / 180;
                FK = NeuroKinematics_.ForwardKinematics(AxialHeadTranslation, AxialFeetTranslation,
                                                        LateralTranslation, ProbeInsertion,
                                                        ProbeRotation, PitchRotation, YawRotation);
                Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
                transferred_point = get_Transform(registration_inv, FK);
                points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
                myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
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
    min_travel = -86;
  }
  std::cout << "# of points: " << points->GetNumberOfPoints();
  myout.close();
  return points;
}

// Method to Create the RCM surface mesh for RCM visualization
vtkSmartPointer<vtkPoints> ForwardKinematics::get_RCM_Workspace(Eigen::Matrix4d registration, vtkSmartPointer<vtkPoints> points)
{
  // To visualize the transferred points in the slicer without using the Transform Module
  Eigen::Matrix4d registration_inv = registration.inverse();
  std::cerr << "Inverse of the registration matrix is: \n"
            << registration_inv << std::endl;

  // Vector to store points before transformation
  Eigen::Vector4d point(0.0, 0.0, 0.0, 0.0);

  // Object containing the 4x4 transformation matrix
  Neuro_FK_outputs RCM{};
  ofstream myout("RCM_workspace.xyz");
  /*============================================================================================================
     =============================================RCM computation============================================
      ==================================================================================================*/

  double min_seperation{71}; //71
  const double Abs_min_leg_separation{68};
  // Loop for visualizing the top
  AxialFeetTranslation = 68;
  AxialHeadTranslation = 0;
  double Top_max_travel{-157};
  for (i = Top_max_travel / 10; i >= Top_max_travel; i += Top_max_travel / 10) // initial separation 143, min separation 75 => 143-75 = 68 mm
  {
    AxialHeadTranslation += Top_max_travel / 10;
    AxialFeetTranslation += Top_max_travel / 10;
    for (k = -49.0; k >= -98.0; k += -49 / 10) // max lateral movement 0.0 ~ -49.47 (appx = -49)
    {
      ++counter;

      LateralTranslation = k;
      RCM = NeuroKinematics_.get_RCM(AxialHeadTranslation, AxialFeetTranslation,
                                     LateralTranslation, ProbeInsertion,
                                     ProbeRotation, PitchRotation, YawRotation);
      Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
      transferred_point = get_Transform(registration_inv, RCM);
      points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
      myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
    }
  }

  // loop for visualizing the bottom
  ++counter;
  AxialHeadTranslation = 0;
  AxialFeetTranslation = -3;
  double max_travel_bottom{-86};
  for (i = 0; i >= max_travel_bottom; i += max_travel_bottom / 10) //
  {
    if (i == 0) // for the beginning row
    {
      for (k = -49.0; k >= -98.0; k += -49 / 10)
      {
        LateralTranslation = k;
        RCM = NeuroKinematics_.get_RCM(AxialHeadTranslation, AxialFeetTranslation,
                                       LateralTranslation, ProbeInsertion,
                                       ProbeRotation, PitchRotation, YawRotation);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, RCM);
        points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
        myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      }
    }
    else
    {
      AxialHeadTranslation += max_travel_bottom / 10;
      AxialFeetTranslation += max_travel_bottom / 10;
      for (k = -49.0; k >= -98.0; k += -49 / 10)
      {
        LateralTranslation = k;
        RCM = NeuroKinematics_.get_RCM(AxialHeadTranslation, AxialFeetTranslation,
                                       LateralTranslation, ProbeInsertion,
                                       ProbeRotation, PitchRotation, YawRotation);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, RCM);
        points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
        myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      }
    }
  }
  AxialFeetTranslation = -3;
  AxialHeadTranslation = 0;

  // Loop for creating the head face
  ++counter;
  for (j = 7.1; j <= 71; j += 7.1)
  {
    AxialFeetTranslation += 7.1;

    for (k = -49.0; k >= -98.0; k += -49 / 10)
    {
      LateralTranslation = k;

      RCM = NeuroKinematics_.get_RCM(AxialHeadTranslation, AxialFeetTranslation,
                                     LateralTranslation, ProbeInsertion,
                                     ProbeRotation, PitchRotation, YawRotation);
      Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
      transferred_point = get_Transform(registration_inv, RCM);
      points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
      myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
    }
  }
  AxialFeetTranslation = -89;
  AxialHeadTranslation = -86;

  // Loop for creating the feet face
  ++counter;
  for (j = min_seperation / 10; j <= min_seperation; j += min_seperation / 10)
  {
    AxialHeadTranslation -= min_seperation / 10;

    for (k = -49.0; k >= -98.0; k += -49 / 10)
    {
      LateralTranslation = k;

      RCM = NeuroKinematics_.get_RCM(AxialHeadTranslation, AxialFeetTranslation,
                                     LateralTranslation, ProbeInsertion,
                                     ProbeRotation, PitchRotation, YawRotation);
      Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
      transferred_point = get_Transform(registration_inv, RCM);
      points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
      myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
    }
  }

  //loop for creating the sides
  ++counter;
  AxialFeetTranslation = -3;
  AxialHeadTranslation = 0;
  double min_travel{-86};  // The max that the robot can move in z direction when at lowest height (at each hight min travel is changed)
  double max_travel{-157}; //157 The max that the robot can move in z direction when at highest height
  for (j = min_seperation / 10; j <= Abs_min_leg_separation; j += min_seperation / 10)
  {
    AxialFeetTranslation += j;         // For each loop it will lift the base by a constant value
    min_travel -= min_seperation / 10; // Takes care of the amount of Axial travel for the Axial head and feet

    for (ii = 0; ii > min_travel + 1 && min_travel >= max_travel; ii += min_travel / 10) // loop to move the base from Head to feet based on the allowable max movement range (min_travel)
    {
      AxialHeadTranslation += min_travel / 10;
      AxialFeetTranslation += min_travel / 10;
      for (k = -49.0; k >= -49 * 2; k += -49)
      {
        LateralTranslation = k;
        RCM = NeuroKinematics_.get_RCM(AxialHeadTranslation, AxialFeetTranslation,
                                       LateralTranslation, ProbeInsertion,
                                       ProbeRotation, PitchRotation, YawRotation);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, RCM);
        points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
        myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      }
    }
    AxialFeetTranslation = -3;
    AxialHeadTranslation = 0;
  }
  std::cout << "# of points: " << points->GetNumberOfPoints();
  myout.close();
  return points;
}

// Method to Create the RCM Point Cloud for Sub-Workspace visualization
Eigen::Matrix3Xf ForwardKinematics::get_RCM_PC(Eigen::Matrix4d registration)
{ // To visualize the transferred points in the slicer without using the Transform Module
  Eigen::Matrix4d registration_inv = registration.inverse();
  std::cerr << "Inverse of the registration matrix is: \n"
            << registration_inv << std::endl;

  // Vector to store points before transformation
  Eigen::Vector4d point(0.0, 0.0, 0.0, 0.0);

  // Object containing the 4x4 transformation matrix
  Neuro_FK_outputs RCM_PC{};
  ofstream myout("RCM_PC.xyz");
  /*============================================================================================================
     =============================================RCM computation============================================
      ==================================================================================================*/

  // Initializing a 3 X 197,779 Eigen matrix to store the RCM point cloud.
  // Change the col length if the courseness of the PC is changed. The value for this length is printed upon running this method.
  Eigen::Matrix3Xf Point_cloud(3, 197779);

  double min_seperation{71};               //71
  const double division{100};              //100
  const double division_k{20};             //20
  const double lateral_start{-49};         //-49
  const double Abs_min_leg_separation{68}; //68
  // Loop for visualizing the top
  AxialFeetTranslation = 68;
  AxialHeadTranslation = 0;
  double Top_max_travel{-157};
  counter = 0;                                                                             //-157
  for (i = Top_max_travel / division; i >= Top_max_travel; i += Top_max_travel / division) // initial separation 143, min separation 75 => 143-75 = 68 mm
  {
    AxialHeadTranslation += Top_max_travel / division;
    AxialFeetTranslation += Top_max_travel / division;
    for (k = lateral_start; k >= lateral_start * 2; k += lateral_start / division_k) // max lateral movement 0.0 ~ -49.47 (appx = -49)
    {
      LateralTranslation = k;
      RCM_PC = NeuroKinematics_.get_RCM(AxialHeadTranslation, AxialFeetTranslation,
                                        LateralTranslation, ProbeInsertion,
                                        ProbeRotation, PitchRotation, YawRotation);
      Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
      transferred_point = get_Transform(registration_inv, RCM_PC);
      // points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
      myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      store_Point(Point_cloud, transferred_point, counter);
      ++counter;
    }
  }

  // loop for visualizing the bottom
  AxialHeadTranslation = 0;
  AxialFeetTranslation = -3;
  double max_travel_bottom{-86};
  for (i = 0; i >= max_travel_bottom; i += max_travel_bottom / division) //
  {
    if (i == 0) // for the beginning row
    {
      for (k = lateral_start; k >= lateral_start * 2; k += lateral_start / division_k)
      {
        LateralTranslation = k;
        RCM_PC = NeuroKinematics_.get_RCM(AxialHeadTranslation, AxialFeetTranslation,
                                          LateralTranslation, ProbeInsertion,
                                          ProbeRotation, PitchRotation, YawRotation);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, RCM_PC);
        myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
        store_Point(Point_cloud, transferred_point, counter);
        ++counter;
      }
    }
    else
    {
      AxialHeadTranslation += max_travel_bottom / division;
      AxialFeetTranslation += max_travel_bottom / division;
      for (k = lateral_start; k >= lateral_start * 2; k += lateral_start / division_k)
      {
        LateralTranslation = k;
        RCM_PC = NeuroKinematics_.get_RCM(AxialHeadTranslation, AxialFeetTranslation,
                                          LateralTranslation, ProbeInsertion,
                                          ProbeRotation, PitchRotation, YawRotation);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, RCM_PC);
        myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
        store_Point(Point_cloud, transferred_point, counter);
        ++counter;
      }
    }
  }
  AxialFeetTranslation = -3;
  AxialHeadTranslation = 0;

  // Loop for creating the head face
  for (j = min_seperation / division; j <= min_seperation; j += min_seperation / division)
  {
    AxialFeetTranslation += min_seperation / division;

    for (k = lateral_start; k >= lateral_start * 2; k += lateral_start / division_k)
    {
      LateralTranslation = k;

      RCM_PC = NeuroKinematics_.get_RCM(AxialHeadTranslation, AxialFeetTranslation,
                                        LateralTranslation, ProbeInsertion,
                                        ProbeRotation, PitchRotation, YawRotation);
      Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
      transferred_point = get_Transform(registration_inv, RCM_PC);
      myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      store_Point(Point_cloud, transferred_point, counter);
      ++counter;
    }
  }
  AxialFeetTranslation = -89;
  AxialHeadTranslation = -86;

  // Loop for creating the feet face
  for (j = min_seperation / division; j <= min_seperation; j += min_seperation / division)
  {
    AxialHeadTranslation -= min_seperation / division;

    for (k = lateral_start; k >= lateral_start * 2; k += lateral_start / division_k)
    {
      LateralTranslation = k;

      RCM_PC = NeuroKinematics_.get_RCM(AxialHeadTranslation, AxialFeetTranslation,
                                        LateralTranslation, ProbeInsertion,
                                        ProbeRotation, PitchRotation, YawRotation);
      Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
      transferred_point = get_Transform(registration_inv, RCM_PC);
      // points->InsertNextPoint(transferred_point(0), transferred_point(1), transferred_point(2));
      myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
      store_Point(Point_cloud, transferred_point, counter);
      ++counter;
    }
  }

  //loop for creating the sides
  AxialFeetTranslation = -3;
  AxialHeadTranslation = 0;
  double min_travel{-86};  // The max that the robot can move in z direction when at lowest height (at each hight min travel is changed)
  double max_travel{-157}; //157 The max that the robot can move in z direction when at highest height
  for (j = min_seperation / division; j <= Abs_min_leg_separation; j += min_seperation / division)
  {
    AxialFeetTranslation += j;               // For each loop it will lift the base by a constant value
    min_travel -= min_seperation / division; // Takes care of the amount of Axial travel for the Axial head and feet

    for (ii = 0; ii > min_travel + 1 && min_travel >= max_travel; ii += min_travel / division) // loop to move the base from Head to feet based on the allowable max movement range (min_travel)
    {
      AxialHeadTranslation += min_travel / division;
      AxialFeetTranslation += min_travel / division;
      for (k = lateral_start; k >= lateral_start * 2; k += lateral_start / division_k)
      {
        LateralTranslation = k;
        RCM_PC = NeuroKinematics_.get_RCM(AxialHeadTranslation, AxialFeetTranslation,
                                          LateralTranslation, ProbeInsertion,
                                          ProbeRotation, PitchRotation, YawRotation);
        Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);
        transferred_point = get_Transform(registration_inv, RCM_PC);
        myout << transferred_point(0) << " " << transferred_point(1) << " " << transferred_point(2) << " 0.00 0.00 0.00" << endl;
        store_Point(Point_cloud, transferred_point, counter);
        ++counter;
      }
    }
    AxialFeetTranslation = -3;
    AxialHeadTranslation = 0;
  }
  std::cout << "\nTotal # points inside the point cloud: " << counter - 1 << std::endl;
  myout.close();
  return Point_cloud;
}

//Method to check for RCM points that are within the reach of the entry point
Eigen::Matrix3Xf ForwardKinematics::get_SubWorkspace(Eigen::Matrix3Xf RCM_PC, Eigen::Vector3d EP_inImagerCoordinate, Eigen::Matrix4d registration, double probe_init)
{
  // Creating the PC for the Validated RCM points that satisfy the distance criteria
  ofstream myout("Validated_workspace.xyz");
  // number of columns of the RCM PC
  int no_cols_RCM_PC = RCM_PC.cols();

  // calculate the transformation from the robot to the the entry point
  Eigen::Matrix4d registration_inv = registration.inverse();
  Eigen::Vector3d EP_inRobotCoordinate(0.0, 0.0, 0.0);
  // Finds the location of the EP W.R.T the robot base
  calc_Transform(registration_inv, EP_inImagerCoordinate, EP_inRobotCoordinate);
  std::cout << "EP_inRobotCoordinate is :\n"
            << EP_inRobotCoordinate << std::endl;
  // Step to check individual points in the RCM Point cloud
  int no_of_cols_validated_pts{0};

  //loop to go through each RCM points and check for the validity of them W.R.T the EP
  for (int i = 0; i < no_cols_RCM_PC; i++)
  {
    Eigen::Vector3f RCM_Point_to_Check(RCM_PC(0, i), RCM_PC(1, i), RCM_PC(2, i));
    if (check_Sphere(EP_inRobotCoordinate, RCM_Point_to_Check, probe_init) == 1)
    {
      no_of_cols_validated_pts++;
    }
  }

  Eigen::Matrix3Xf Validated_PC(3, no_of_cols_validated_pts); // Matrix to store the validated points in
  std::cout << "Number of columns of the Validated PC " << Validated_PC.cols() << std::endl;

  no_of_cols_validated_pts = 0;

  for (i = 0; i < no_cols_RCM_PC; i++)
  {
    Eigen::Vector3f RCM_Point_to_Check(RCM_PC(0, i), RCM_PC(1, i), RCM_PC(2, i));
    if (check_Sphere(EP_inRobotCoordinate, RCM_Point_to_Check, probe_init) == 1)
    {
      Validated_PC(0, no_of_cols_validated_pts) = RCM_Point_to_Check(0);
      Validated_PC(1, no_of_cols_validated_pts) = RCM_Point_to_Check(1);
      Validated_PC(2, no_of_cols_validated_pts) = RCM_Point_to_Check(2);
      myout << RCM_Point_to_Check(0) << " " << RCM_Point_to_Check(1) << " " << RCM_Point_to_Check(2) << " 0.00 0.00 0.00" << endl;

      no_of_cols_validated_pts++;
    }
  }

  // Step to check the Inverse Kinematics for each Validated RCM point
  Eigen::Matrix3Xf Sub_Workspace_RCM = get_PointCloud_IK(Validated_PC, EP_inRobotCoordinate);

  Eigen::Matrix3Xf total_Sub_Workspace = create_3D_Mesh(Sub_Workspace_RCM, EP_inRobotCoordinate);

  myout.close();
  return Validated_PC;
}

// Method to calculate the transform for a point to be described in the Imager's Cooridnate frame. Will be deprecated since this is now done in the Slicer module.
Eigen::Vector4d ForwardKinematics::get_Transform(Eigen::Matrix4d registration_inv, Neuro_FK_outputs FK)
{
  //Vector that stores the End effector's position defined in the robot's Z-frame
  Eigen::Vector4d point(0.0, 0.0, 0.0, 1.0);
  for (int t = 0; t < 3; t++)
  {
    point(t) = FK.zFrameToTreatment(t, 3);
  }

  // Vector that stores the transferred points defined W.R.T the imager's frame
  Eigen::Vector4d transferred_point(0.0, 0.0, 0.0, 1.0);

  //Finding the corresponding point W.R.T the imager's frame
  transferred_point = registration_inv * point;

  //rounding step (to the tenth)
  for (int t = 0; t < 3; t++)
  {
    transferred_point(t) = round(transferred_point(t) * 10) / 10;
  }
  return transferred_point;
}

// Method to store a point of the RCM Point Cloud. Points are stored inside an Eigen matrix.
void ForwardKinematics::store_Point(Eigen::Matrix3Xf &RCM_Point_cloud, Eigen::Vector4d &transferred_Point, int counter)
{
  RCM_Point_cloud(0, counter) = transferred_Point(0);
  RCM_Point_cloud(1, counter) = transferred_Point(1);
  RCM_Point_cloud(2, counter) = transferred_Point(2);
}

// Method which applis the transform to the given entry point defined in the Imager's coordinate frame to define it in the Robot's frame
void ForwardKinematics::calc_Transform(Eigen::Matrix4d registration_inv, Eigen::Vector3d EP_inImagerCoordinate, Eigen::Vector3d &EP_inRobotCoordinate)
{
  Eigen::Vector4d EP_R(EP_inRobotCoordinate(0), EP_inRobotCoordinate(1), EP_inRobotCoordinate(2), 1);    // Creating a standard vector for matrix multiplication
  Eigen::Vector4d EP_I(EP_inImagerCoordinate(0), EP_inImagerCoordinate(1), EP_inImagerCoordinate(2), 1); // Creating a standard vector for matrix multiplication
  //Finding the location of the EP W.R.T Robot's base frame
  EP_R = registration_inv * EP_I;

  //rounding step (to the tenth)
  for (int t = 0; t < 3; t++)
  {
    EP_R(t) = round(EP_R(t) * 10) / 10;
    EP_inRobotCoordinate(t) = EP_R(t);
  }
}

// Method to check if the Entry point is within the bounds of a given RCM point
bool ForwardKinematics::check_Sphere(Eigen::Vector3d EP_inRobotCoordinate, Eigen::Vector3f RCM_point, double probe_init)
{
  /*Whether a point lies inside a sphere or not, depends upon its distance from the centre.
  A point (x, y, z) is inside the sphere with center (cx, cy, cz) and radius r if
  ( x-cx ) ^2 + (y-cy) ^2 + (z-cz) ^ 2 < r^2 */
  float B_value = probe_init;
  const float radius = 72.5 - B_value; // RCM offset from Robot to RCM point

  float distance{0};
  distance = pow(EP_inRobotCoordinate(0) - RCM_point(0), 2) + pow(EP_inRobotCoordinate(1) - RCM_point(1), 2) + pow(EP_inRobotCoordinate(2) - RCM_point(2), 2);
  if (distance <= pow(radius, 2)) // EP is within the Sphere
  {
    return 1;
  }
  else // EP is outside of the Sphere
  {
    return 0;
  }
}

// Method to Check the IK for the Validated point set
Eigen::Matrix3Xf ForwardKinematics::get_PointCloud_IK(Eigen::Matrix3Xf Validated_PC, Eigen::Vector3d EP_inRobotCoordinate)
{
  Eigen::Matrix3Xf Sub_Workspace_PC_RCM(3, 1);
  //Initializng the Sub_workspace matrix
  Sub_Workspace_PC_RCM << 0.,
      0.,
      0.;

  int counter{0};
  // float B_value = NeuroKinematics_._probe->_robotToEntry; // B value
  int no_Cols_Validated_PC = Validated_PC.cols();

  /* The methods checks for validity of the filtered workspace Using IK_solver Method. This Method takes
  two Eigen Vectors of size 4 i.e (x,y,z,1). First argument is the EP and the second argument is the RCM point
  which is considered as the TP.*/

  // Initializing the vectors for EP and TP.
  Eigen::Vector4d EP_R(EP_inRobotCoordinate(0), EP_inRobotCoordinate(1), EP_inRobotCoordinate(2), 1);
  Eigen::Vector4d TP_R(0, 0, 0, 1);

  // Initializing the limits for each axis of the robot.
  const double initial_Axial_separation = 143;
  const double min_Axial_separation = 75;
  const double max_Axial_separation = 146;
  const double max_Lateral_translation = -49;
  const double min_Lateral_translation = -98;
  const double max_AxialHead_translation = 0;
  const double min_AxialHead_translation = -146; // it may be -145
  const double max_AxialFeet_translation = 68;
  const double min_AxialFeet_translation = -78;
  const double max_Pitch_rotation = +26.0 * pi / 180;
  const double min_Pitch_rotation = -37.0 * pi / 180;
  const double max_Yaw_rotation = 0.0;
  const double min_Yaw_rotation = -88.0 * pi / 180;
  const double min_Probe_insertion = 0;
  const double max_Probe_insertion = 50;
  double Axial_Seperation{0};
  // Creating the object to store the output of the IK_Solver
  Neuro_IK_outputs IK_output;

  // Loop that goes through each point in the Validated PC and checks for the Validity of the IK output
  for (int i = 0; i < no_Cols_Validated_PC; i++)
  {
    // setting the TP for each point in the loop
    TP_R << Validated_PC(0, i), Validated_PC(1, i), Validated_PC(2, i), 1;

    IK_output = NeuroKinematics_.IK_solver(EP_R, TP_R);
    Axial_Seperation = 143 + IK_output.AxialHeadTranslation - IK_output.AxialFeetTranslation;

    if (Axial_Seperation > max_Axial_separation)
    {
      continue;
    }
    if (Axial_Seperation > max_Axial_separation || Axial_Seperation < min_Axial_separation) // Axial Heads are farther away than the allowed value or Axial Heads are closer than the allowed value
    {
      continue;
    }
    if (IK_output.AxialHeadTranslation < min_AxialHead_translation || IK_output.AxialHeadTranslation > max_AxialHead_translation) // If Axial Head travels more than the max or min allowed range
    {
      continue;
    }
    if (IK_output.AxialFeetTranslation < min_AxialFeet_translation || IK_output.AxialFeetTranslation > max_AxialFeet_translation) // If Axial Feet travels more than the max or min allowed range
    {
      continue;
    }
    if (IK_output.LateralTranslation < min_Lateral_translation || IK_output.LateralTranslation > max_Lateral_translation) // If Lateral travels more than the max or min allowed range
    {
      continue;
    }
    if (IK_output.YawRotation < min_Yaw_rotation || IK_output.YawRotation > max_Yaw_rotation) // If Yaw rotates more than the max or min allowed range
    {
      continue;
    }
    if (IK_output.PitchRotation < min_Pitch_rotation || IK_output.PitchRotation > max_Pitch_rotation) // If Pitch rotates more than the max or min allowed range
    {
      continue;
    }
    if (counter > 0)
    { // This if statement will increase the size of the Sub-workspace matrix by one to store the new point
      Sub_Workspace_PC_RCM.conservativeResize(3, Sub_Workspace_PC_RCM.cols() + 1);
    }

    // Storing the validated point in the final sub-workspace matrix
    Sub_Workspace_PC_RCM(0, counter) = TP_R(0);
    Sub_Workspace_PC_RCM(1, counter) = TP_R(1);
    Sub_Workspace_PC_RCM(2, counter) = TP_R(2);

    counter++;
  }
  if (Sub_Workspace_PC_RCM.cols() == 1 && Sub_Workspace_PC_RCM(0, 0) == 0. && Sub_Workspace_PC_RCM(1, 0) == 0. && Sub_Workspace_PC_RCM(2, 0) == 0.)
  {
    std::cerr << "\nThe entry point is NOT reachable! Please select another point." << std::endl;
  }
  else
  {
    std::cout << "\nNumber of Points in the Sub_workspace: " << Sub_Workspace_PC_RCM.cols() << std::endl;
    ofstream myout("sub_workspace_RCM.xyz");
    for (i = 0; i < Sub_Workspace_PC_RCM.cols(); i++)
    {
      myout << Sub_Workspace_PC_RCM(0, i) << " " << Sub_Workspace_PC_RCM(1, i) << " " << Sub_Workspace_PC_RCM(2, i) << " 0.00 0.00 0.00" << endl;
    }
    myout << EP_inRobotCoordinate(0) << " " << EP_inRobotCoordinate(1) << " " << EP_inRobotCoordinate(2) << " 0.00 0.00 0.00" << endl;
    myout.close();
  }
  return Sub_Workspace_PC_RCM;
}

// Method to create the 3D representing the sub-workspace
Eigen::Matrix3Xf ForwardKinematics::create_3D_Mesh(Eigen::Matrix3Xf Sub_Workspace_RCM, Eigen::Vector3d EP_inRobotCoordinate)
{
  /* Step to create a full representative point cloud based on the sub-workspace
  In this step, additional points will be added starting from the Entry Point and passing
  through each RCM points which account for the full probe insertion. The final workspace will
  be returned to the VTK method to generate the 3D mesh for visualization.*/

  // Total number of points in the RCM sub-workspace
  int no_cols = Sub_Workspace_RCM.cols();
  float max_Probe_Insertion = 40.0;                                                  // Maximum range of motion for the Probe Insertion Axis
  float Dist_past_RCM = max_Probe_Insertion + NeuroKinematics_.RCMToTreatment(2, 3); // Max point where the treatment can reach past the RCM point
  Eigen::Vector3d vector(0., 0., 0.);                                                // Vector starting from the entry point and ending at the RCM point,i.e. X2-X1
  Eigen::Vector3d RCM_point(0., 0., 0.);
  Eigen::Vector3d coordinate_of_last_point(0., 0., 0.);
  Eigen::Vector3d intersection_point1(0., 0., 0.);
  Eigen::Vector3d intersection_point2(0., 0., 0.);
  int division{5}, counter{0}, counter1{0}; // division is the number of desired points to generate between the EP and the last point
  // Number of desired points between the EP and the last point
  Eigen::Matrix3Xf sub_workspace_PC(3, no_cols * division); // Matrix containing all the points within the sub-workspace starting from the EP to the last point
  sub_workspace_PC = 0 * sub_workspace_PC;                  // initializing

  /*To find the coordinate of the point past the RCM, a series of operations need to be evoked.
  A sphere of size equal to "Dist_past_RCM" will be placed with it's origin at the RCM point.
  The intersection of the line from the EP to the RCM point with this sphere will give the coordinate
  of the point that the treatment will reach after passing the RCM.*/
  double a{0}, b{0}, c{0}, t1{0}, t2{0}, x{0}, y{0}, z{0}; // coefficient to be found for the equation of line and it's intersection with the sphere
  for (int i = 0; i < no_cols; i++)
  {
    RCM_point << Sub_Workspace_RCM(0, i), Sub_Workspace_RCM(1, i), Sub_Workspace_RCM(2, i);
    // Finding the equation of a line for each pair of RCM and Entry points.
    vector = RCM_point - EP_inRobotCoordinate;
    a = (pow(vector(0), 2) + pow(vector(1), 2) + pow(vector(2), 2));
    b = -2 * a;
    c = a - pow(Dist_past_RCM, 2);
    t1 = (-b + sqrt(pow(b, 2) - (4 * a * c))) / (2 * a); // coefficients that will be plugged into the eq of line which give the intersection points
    t2 = (-b - sqrt(pow(b, 2) - (4 * a * c))) / (2 * a); // coefficients that will be plugged into the eq of line which give the intersection points
    intersection_point1 << EP_inRobotCoordinate(0) + vector(0) * t1, EP_inRobotCoordinate(1) + vector(1) * t1, EP_inRobotCoordinate(2) + vector(2) * t1;
    intersection_point2 << EP_inRobotCoordinate(0) + vector(0) * t2, EP_inRobotCoordinate(1) + vector(1) * t2, EP_inRobotCoordinate(2) + vector(2) * t2;
    double dist1 = sqrt(pow(intersection_point1(0) - EP_inRobotCoordinate(0), 2) + pow(intersection_point1(1) - EP_inRobotCoordinate(1), 2) + pow(intersection_point1(2) - EP_inRobotCoordinate(2), 2));
    double dist2 = sqrt(pow(intersection_point2(0) - EP_inRobotCoordinate(0), 2) + pow(intersection_point2(1) - EP_inRobotCoordinate(1), 2) + pow(intersection_point2(2) - EP_inRobotCoordinate(2), 2));
    if (dist1 > dist2)
    {
      coordinate_of_last_point = intersection_point1;
    }
    else if (dist1 < dist2)
    {
      coordinate_of_last_point = intersection_point2;
    }
    x = abs(coordinate_of_last_point(0) - EP_inRobotCoordinate(0)) / division; // increments for x
    y = abs(coordinate_of_last_point(1) - EP_inRobotCoordinate(1)) / division; // increments for y
    z = abs(coordinate_of_last_point(2) - EP_inRobotCoordinate(2)) / division; // increments for z

    for (int j = 1; j <= division; j++)
    {
      if (EP_inRobotCoordinate(0) < coordinate_of_last_point(0))
      {
        sub_workspace_PC(0, counter + j - 1) = EP_inRobotCoordinate(0) + (x * j);
      }
      else if (EP_inRobotCoordinate(0) > coordinate_of_last_point(0))
      {
        sub_workspace_PC(0, counter + j - 1) = EP_inRobotCoordinate(0) - (x * j);
      }
      sub_workspace_PC(1, counter + j - 1) = EP_inRobotCoordinate(1) - (y * j);
      if (EP_inRobotCoordinate(2) < coordinate_of_last_point(2))
      {
        sub_workspace_PC(2, counter + j - 1) = EP_inRobotCoordinate(2) + (z * j);
      }
      else if (EP_inRobotCoordinate(2) > coordinate_of_last_point(2))
      {
        sub_workspace_PC(2, counter + j - 1) = EP_inRobotCoordinate(2) - (z * j);
      }
    }
    counter += division;
  }
  std::cout << "\nnumber of points created for the final subworkspace: " << sub_workspace_PC.cols() << std::endl;
  //creating a xyz pc for test
  ofstream myout("final_sub_workspace.xyz");
  for (i = 0; i < sub_workspace_PC.cols(); i++)
  {
    myout << sub_workspace_PC(0, i) << " " << sub_workspace_PC(1, i) << " " << sub_workspace_PC(2, i) << " 0.00 0.00 0.00" << endl;
  }
  // Adding Entry point
  myout << EP_inRobotCoordinate(0) << " " << EP_inRobotCoordinate(1) << " " << EP_inRobotCoordinate(2) << " 0.00 0.00 0.00" << endl;
  myout.close();
  return sub_workspace_PC;
}
