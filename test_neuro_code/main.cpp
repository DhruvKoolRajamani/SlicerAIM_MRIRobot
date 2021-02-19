#include "../NeuroRobot/include/WorkspaceGeneration/WorkspaceVisualization.h"
#include "../Utilities/include/SavePointCloudData/SavePointCloudData.hpp"
int main(int argc, char* argv[])
{
  double _cannulaToTreatment{0.0};
  double _treatmentToTip{0.0};
  double _robotToEntry{5.0};
  double _robotToTreatmentAtHome{41.0};
  Probe  probe_init = {_cannulaToTreatment, _treatmentToTip, _robotToEntry,
                      _robotToTreatmentAtHome};
  NeuroKinematics   NeuroKinematics_(&probe_init);
  ForwardKinematics ForwardKinematics_(NeuroKinematics_);

  // Eigen::Matrix3Xf general_workspace =
  // ForwardKinematics_.GetGeneralWorkspace(); SaveDataToFile
  // data_writer(general_workspace);
  // data_writer.SaveToXyz("GeneralWorkspace.xyz");

  // Eigen::Matrix3Xf rcm_workspace = ForwardKinematics_.GetRcmWorkSpace();
  // SaveDataToFile   data_writer1(rcm_workspace);
  // data_writer1.SaveToXyz("RcmWorkspace.xyz");

  // Eigen::Matrix3Xf rcm_pointset = ForwardKinematics_.rcm_point_set_;
  // SaveDataToFile   data_writer3(rcm_pointset);
  // data_writer3.SaveToXyz("RcmPointset.xyz");

  Eigen::Matrix4d registration = Eigen::Matrix4d::Identity();
  registration(0, 3)           = -0.16;
  registration(1, 3)           = -124.35;
  registration(2, 3)           = 10.38;
  // Eigen::Vector4d ep_in_imager(-40, 130.172, 80, 1);
  // Eigen::Vector4d ep_in_imager(-66.598, 60.862, 63.71, 1);
  // Eigen::Vector4d ep_in_imager(-66.598, 130.861, 63.71, 1);
  Eigen::Vector4d ep_in_imager(-62.009, 132.697, 65.521, 1);

  // Eigen::Vector4d anamoly(-61.7133, 177.481, -3.77333, 1);

  Eigen::Vector4d ep_in_robot = registration.inverse() * ep_in_imager;
  std::cout << "Entry point in Robot Coordinate is :\n"
            << ep_in_robot(0) << " , " << ep_in_robot(1) << " , "
            << ep_in_robot(2) << std::endl;
  Eigen::Vector3d ep_in_robot_(ep_in_robot(0), ep_in_robot(1), ep_in_robot(2));

  Eigen::Matrix3Xf final_workspace =
    ForwardKinematics_.GetSubWorkspace(ep_in_robot_);
  SaveDataToFile data_writer4(final_workspace);
  data_writer4.SaveToXyz("final_workspace.xyz");
  return 0;
}

// #include "NeuroKinematics/ForwardKinematics.h"
// #include "NeuroKinematics/NeuroKinematics.hpp"
// #include <cstdlib>
// #include <iostream>
// #include <math.h> /* isnan, sqrt */
// #include <string>
// #include <vtkCellArray.h>
// #include <vtkPCANormalEstimation.h>
// #include <vtkPLYWriter.h>
// #include <vtkPointData.h>
// #include <vtkPointSource.h>
// #include <vtkPoints.h>
// // #include <vtkPoissonReconstruction.h>
// #include <vtkPolyData.h>
// #include <vtkPowerCrustSurfaceReconstruction.h>
// #include <vtkProperty.h>
// #include <vtkSmartPointer.h>
// #include <vtkXMLPolyDataWriter.h>
// #include <vtksys/SystemTools.hxx>

// double _cannulaToTreatment{5.0};
// double _treatmentToTip{10.0};
// double _robotToEntry{5.0};
// double _robotToTreatmentAtHome{41.0};
// Probe probe_init = {_cannulaToTreatment, _treatmentToTip, _robotToEntry,
//                     _robotToTreatmentAtHome};

// int main(int argc, char* argv[])
// {
//   NeuroKinematics NeuroKinematics_(&probe_init);
//   ForwardKinematics ForwardKinematics_(NeuroKinematics_);
//   // General workspace computation
//   // 4x4 Registration matrix
//   // just a test case, use X= -53, Y= -119, Z= -121
//   Eigen::Matrix4d registration;
//   registration << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;

//   vtkSmartPointer< vtkPoints > General_Workspace_PC =
//     vtkSmartPointer< vtkPoints >::New();
//   std::cout << "Im here\n";
//   General_Workspace_PC = ForwardKinematics_.get_General_Workspace(
//     registration, General_Workspace_PC);
//   // std::cout << "# of points: " <<
//   General_Workspace_PC->GetNumberOfPoints();
//   // vtkSmartPointer<vtkPoints> RCM_points =
//   vtkSmartPointer<vtkPoints>::New();
//   // Create a polydata object and add the points to it.
//   vtkSmartPointer< vtkPolyData > polydata_General_Workspace_PC =
//     vtkSmartPointer< vtkPolyData >::New();
//   polydata_General_Workspace_PC->SetPoints(General_Workspace_PC);
//   std::cout << "# of points: "
//             << polydata_General_Workspace_PC->GetNumberOfPoints() <<
//             std::endl;

//   // Write the .VTP (point cloud) file
//   vtkSmartPointer< vtkXMLPolyDataWriter > writer_General_Workspace_PC =
//     vtkSmartPointer< vtkXMLPolyDataWriter >::New();
//   writer_General_Workspace_PC->SetFileName("General_Workspace.vtp");
//   writer_General_Workspace_PC->SetInputData(polydata_General_Workspace_PC);
//   writer_General_Workspace_PC->Write();
//   // Optional - set the mode. The default is binary.
//   // writer->SetDataModeToBinary();
//   // writer->SetDataModeToAscii();

//   std::cerr << "Using PowerCrust Algorithm to create General Workspace
//   surface "
//                "mesh"
//             << std::endl;
//   vtkSmartPointer< vtkPowerCrustSurfaceReconstruction >
//     surface_General_Workspace =
//       vtkSmartPointer< vtkPowerCrustSurfaceReconstruction >::New();
//   surface_General_Workspace->SetInputData(polydata_General_Workspace_PC);
//   std::string filename_General_workspace_ply = "General_Workspace.ply";
//   vtkSmartPointer< vtkPLYWriter > plyWriter_General_Workspace =
//     vtkSmartPointer< vtkPLYWriter >::New();
//   plyWriter_General_Workspace->SetFileName(
//     filename_General_workspace_ply.c_str());
//   plyWriter_General_Workspace->SetInputConnection(
//     surface_General_Workspace->GetOutputPort());
//   std::cout << "Writing " << filename_General_workspace_ply << std::endl;
//   plyWriter_General_Workspace->Write();

//   // // Create a polydata object and add the points to it.
//   // vtkSmartPointer<vtkPolyData> polydata =
//   //     vtkSmartPointer<vtkPolyData>::New();
//   // polydata->SetPoints(points);
//   // std::cout << "# of points: " << polydata->GetNumberOfPoints() <<
//   std::endl;
//   // // Write the .VTP (point cloud) file
//   // vtkSmartPointer<vtkXMLPolyDataWriter> writer =
//   //     vtkSmartPointer<vtkXMLPolyDataWriter>::New();
//   // writer->SetFileName("FK.vtp");
//   // writer->SetInputData(polydata);
//   // writer->Write();
//   // // Optional - set the mode. The default is binary.
//   // //writer->SetDataModeToBinary();
//   // //writer->SetDataModeToAscii();
//   // std::string filename{"FK.ply"};

//   // cerr << "Using PowerCrust Algorithm" << std::endl;
//   // vtkSmartPointer<vtkPowerCrustSurfaceReconstruction> surface =
//   //     vtkSmartPointer<vtkPowerCrustSurfaceReconstruction>::New();
//   // surface->SetInputData(polydata);
//   // std::string filename = argv[1];
//   // vtkSmartPointer<vtkPLYWriter> plyWriter =
//   // vtkSmartPointer<vtkPLYWriter>::New();
//   // plyWriter->SetFileName(filename.c_str());
//   // plyWriter->SetInputConnection(surface->GetOutputPort());
//   // std::cout << "Writing " << filename << std::endl;
//   // plyWriter->Write();

//   // cerr << "Using Poisson's Algorithm" << std::endl;
//   // vtkSmartPointer<vtkPoissonReconstruction> surface =
//   //     vtkSmartPointer<vtkPoissonReconstruction>::New();
//   // surface->SetDepth(12);
//   // int sampleSize = polydata->GetNumberOfPoints() * .00005;
//   // if (sampleSize < 10)
//   // {
//   //     sampleSize = 10;
//   // }
//   // if (polydata->GetPointData()->GetNormals())
//   // {
//   //     std::cout << "Using normals from input file" << std::endl;
//   //     surface->SetInputData(polydata);
//   // }
//   // else
//   // {
//   //     std::cout << "Estimating normals using PCANormalEstimation" <<
//   //     std::endl; vtkSmartPointer<vtkPCANormalEstimation> normals =
//   //         vtkSmartPointer<vtkPCANormalEstimation>::New();
//   //     normals->SetInputData(polydata);
//   //     normals->SetSampleSize(sampleSize);
//   //     normals->SetNormalOrientationToGraphTraversal();
//   //     normals->FlipNormalsOff();
//   //     surface->SetInputConnection(normals->GetOutputPort());
//   // }
//   // std::string filename = argv[1];
//   // vtkSmartPointer<vtkPLYWriter> plyWriter =
//   // vtkSmartPointer<vtkPLYWriter>::New();
//   // plyWriter->SetFileName(filename.c_str());
//   // plyWriter->SetInputConnection(surface->GetOutputPort());
//   // std::cout << "Writing " << filename << std::endl;
//   // plyWriter->Write();
//   // vtkSmartPointer<vtkPoints> RCM_points =
//   vtkSmartPointer<vtkPoints>::New();
//   // RCM_points = create_RCM_workspace();
//   // // Create a polydata object and add the points to it.
//   // vtkSmartPointer<vtkPolyData> polydata_RCM =
//   //     vtkSmartPointer<vtkPolyData>::New();
//   // polydata_RCM->SetPoints(RCM_points);
//   // std::cout << "# of points: " << polydata_RCM->GetNumberOfPoints() <<
//   // std::endl;
//   // // Write the .VTP (point cloud) file
//   // vtkSmartPointer<vtkXMLPolyDataWriter> writer_RCM =
//   //     vtkSmartPointer<vtkXMLPolyDataWriter>::New();
//   // writer_RCM->SetFileName("RCM.vtp");
//   // writer_RCM->SetInputData(polydata_RCM);
//   // writer_RCM->Write();
//   // // Optional - set the mode. The default is binary.
//   // //writer->SetDataModeToBinary();
//   // //writer->SetDataModeToAscii();
//   // cerr << "Using PowerCrust Algorithm" << std::endl;
//   // vtkSmartPointer<vtkPowerCrustSurfaceReconstruction> surface_RCM =
//   //     vtkSmartPointer<vtkPowerCrustSurfaceReconstruction>::New();
//   // surface_RCM->SetInputData(polydata_RCM);
//   // std::string filename_RCM_ply = "RCM.ply";
//   // vtkSmartPointer<vtkPLYWriter> plyWriter_RCM =
//   // vtkSmartPointer<vtkPLYWriter>::New();
//   // plyWriter_RCM->SetFileName(filename_RCM_ply.c_str());
//   // plyWriter_RCM->SetInputConnection(surface_RCM->GetOutputPort());
//   // std::cout << "Writing " << filename_RCM_ply << std::endl;
//   // plyWriter_RCM->Write();

//   // // Create a polydata object and add the points to it.
//   // vtkSmartPointer<vtkPolyData> polydata =
//   //     vtkSmartPointer<vtkPolyData>::New();
//   // polydata->SetPoints(points);
//   // std::cout << "# of points: " << polydata->GetNumberOfPoints() <<
//   std::endl;
//   // // Write the .VTP (point cloud) file
//   // vtkSmartPointer<vtkXMLPolyDataWriter> writer =
//   //     vtkSmartPointer<vtkXMLPolyDataWriter>::New();
//   // writer->SetFileName("FK.vtp");
//   // writer->SetInputData(polydata);
//   // writer->Write();
//   // // Optional - set the mode. The default is binary.
//   // //writer->SetDataModeToBinary();
//   // //writer->SetDataModeToAscii();
//   // std::string filename{"FK.ply"};

//   // // choose the algorithm for surface generation

//   // cerr << "Using PowerCrust Algorithm" << std::endl;
//   // vtkSmartPointer<vtkPowerCrustSurfaceReconstruction> surface =
//   //     vtkSmartPointer<vtkPowerCrustSurfaceReconstruction>::New();
//   // surface->SetInputData(polydata);
//   // std::string filename = argv[1];
//   // vtkSmartPointer<vtkPLYWriter> plyWriter =
//   // vtkSmartPointer<vtkPLYWriter>::New();
//   // plyWriter->SetFileName(filename.c_str());
//   // plyWriter->SetInputConnection(surface->GetOutputPort());
//   // std::cout << "Writing " << filename << std::endl;
//   // plyWriter->Write();

//   // // creating a surface using Poisson's algorithm

//   // cerr << "Using Poisson's Algorithm" << std::endl;
//   // vtkSmartPointer<vtkPoissonReconstruction> surface =
//   //     vtkSmartPointer<vtkPoissonReconstruction>::New();
//   // surface->SetDepth(12);
//   // int sampleSize = polydata->GetNumberOfPoints() * .00005;
//   // if (sampleSize < 10)
//   // {
//   //     sampleSize = 10;
//   // }
//   // if (polydata->GetPointData()->GetNormals())
//   // {
//   //     std::cout << "Using normals from input file" << std::endl;
//   //     surface->SetInputData(polydata);
//   // }
//   // else
//   // {
//   //     std::cout << "Estimating normals using PCANormalEstimation" <<
//   //     std::endl; vtkSmartPointer<vtkPCANormalEstimation> normals =
//   //         vtkSmartPointer<vtkPCANormalEstimation>::New();
//   //     normals->SetInputData(polydata);
//   //     normals->SetSampleSize(sampleSize);
//   //     normals->SetNormalOrientationToGraphTraversal();
//   //     normals->FlipNormalsOff();
//   //     surface->SetInputConnection(normals->GetOutputPort());
//   // }
//   // std::string filename = argv[1];
//   // vtkSmartPointer<vtkPLYWriter> plyWriter =
//   // vtkSmartPointer<vtkPLYWriter>::New();
//   // plyWriter->SetFileName(filename.c_str());
//   // plyWriter->SetInputConnection(surface->GetOutputPort());
//   // std::cout << "Writing " << filename << std::endl;
//   // plyWriter->Write();
// }