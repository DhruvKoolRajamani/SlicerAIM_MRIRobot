/**
 * @file PointSetUtilities.cpp
 * @author Farid Tavakol (ftavakol@wpi.edu)
 * @brief
 * @version 0.1
 * @date 2021-02-23
 *
 *
 */

#include "PointSetUtilities/PointSetUtilities.hpp"
#include <fstream>
#include <iostream>

PointSetUtilities::PointSetUtilities(Eigen::Matrix3Xf eigenPointSet)
{
  EigenPointSet = eigenPointSet;
}

void PointSetUtilities::saveToXyz(const char* fileName)
{
  std::cout << "Number of points to be saved in " << fileName
            << " are: " << EigenPointSet.cols() << std::endl;
  std::ofstream output(fileName, std::ofstream::out);
  for (int i = 0; i < EigenPointSet.cols(); i++)
  {
    output << EigenPointSet(0, i) << " " << EigenPointSet(1, i) << " "
           << EigenPointSet(2, i) << " 0.00 0.00 0.00" << std::endl;
  }
  output.close();
}

void PointSetUtilities::setEigenPointSet(Eigen::Matrix3Xf eigenPointSet)
{
  EigenPointSet = eigenPointSet;
}

Eigen::Matrix3Xf PointSetUtilities::getEigenPointSet()
{
  return EigenPointSet;
}

vtkSmartPointer< vtkPoints > PointSetUtilities::getVTKPointSet()
{
  vtkSmartPointer< vtkPoints > pointSet = vtkSmartPointer< vtkPoints >::New();

  for (int i = 0; i < EigenPointSet.cols(); i++)
  {
    pointSet->InsertPoint(i, EigenPointSet.col(i).data());
  }

  return pointSet;
}