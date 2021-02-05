#pragma once
#include <eigen3/Eigen/Dense>
class SaveDataToFile
{
public:
  SaveDataToFile(Eigen::Matrix3Xf point_set);

  Eigen::Matrix3Xf point_set_;
  // Methods
  void SaveToXyz(const char* file_name);
};