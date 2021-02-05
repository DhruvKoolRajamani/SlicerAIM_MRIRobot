#include "../../include/SavePointCloudData/SavePointCloudData.hpp"
#include <fstream>
#include <iostream>

SaveDataToFile::SaveDataToFile(Eigen::Matrix3Xf point_set)
{
  point_set_ = point_set;
}
void SaveDataToFile::SaveToXyz(const char* file_name)
{
  std::cout << "Number of points to be saved in " << file_name
            << " are: " << point_set_.cols() << std::endl;
  std::ofstream output(file_name, std::ofstream::out);
  for (int i = 0; i < point_set_.cols(); i++)
  {
    output << point_set_(0, i) << " " << point_set_(1, i) << " "
           << point_set_(2, i) << " 0.00 0.00 0.00" << std::endl;
  }
  output.close();
}