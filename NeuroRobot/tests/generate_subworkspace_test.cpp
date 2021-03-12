#include <NeuroKinematics/NeuroKinematics.hpp>
#include <PointSetUtilities/PointSetUtilities.hpp>
#include <WorkspaceVisualization/WorkspaceVisualization.hpp>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>

int main(int argc, char** argv)
{
  double _cannulaToTreatment{0.0};
  double _treatmentToTip{0.0};
  double _robotToEntry{5.0};
  double _robotToTreatmentAtHome{41.0};
  Probe  probe_init = {_cannulaToTreatment, _treatmentToTip, _robotToEntry,
                      _robotToTreatmentAtHome};
  NeuroKinematics        NeuroKinematics_(&probe_init);
  WorkspaceVisualization WorkspaceVisualization_(NeuroKinematics_);

  // Eigen::Matrix3Xf general_workspace =
  // ForwardKinematics_.GetGeneralWorkspace(); SaveDataToFile
  // data_writer(general_workspace);
  // data_writer.SaveToXyz("GeneralWorkspace.xyz");

  // Eigen::Matrix3Xf entry_point_workspace =
  //   ForwardKinematics_.GetEntryPointWorkspace();
  // SaveDataToFile data_writer(entry_point_workspace);
  // data_writer.SaveToXyz("EntryPointWorkspace.xyz");

  // Eigen::Matrix3Xf rcm_workspace = ForwardKinematics_.GetRcmWorkSpace();
  // SaveDataToFile   data_writer1(rcm_workspace);
  // data_writer1.SaveToXyz("RcmWorkspace.xyz");

  Eigen::Matrix4d registration = Eigen::Matrix4d::Identity();
  registration(0, 3)           = -0.16;
  registration(1, 3)           = -124.35;
  registration(2, 3)           = 10.38;
  // Eigen::Vector4d ep_in_imager(-40, 130.172, 80, 1);
  // Eigen::Vector4d ep_in_imager(-66.598, 60.862, 63.71, 1);
  Eigen::Vector4d ep_in_imager(-62.009, 132.697, 65.521, 1);
  // Eigen::Vector4d ep_in_imager(-62.009, 132.697, 65.521, 1);

  Eigen::Vector4d ep_in_robot = registration.inverse() * ep_in_imager;
  std::cout << "Entry point in Robot Coordinate is :\n"
            << ep_in_robot(0) << " , " << ep_in_robot(1) << " , "
            << ep_in_robot(2) << std::endl;
  Eigen::Vector3d ep_in_robot_(ep_in_robot(0), ep_in_robot(1), ep_in_robot(2));

  Eigen::Matrix3Xf final_workspace;
  int              status =
    WorkspaceVisualization_.GetSubWorkspace(ep_in_robot_, final_workspace);
  PointSetUtilities data_writer4(final_workspace);
  data_writer4.saveToXyz("final_workspace.xyz");
  vtkSmartPointer< vtkPoints > vpointSet = data_writer4.getVTKPointSet();
  return 0;
}