/**
 * @file PointSetUtilities.hpp
 * @author Farid Tavakol (ftavakkolmoghadd@wpi.edu)
 * @brief
 * @version 0.1
 * @date 2021-02-23
 *
 *
 */

#ifndef POINTSETUTILITIES_HPP
#define POINTSETUTILITIES_HPP

#include <eigen3/Eigen/Dense>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
// QT Includes
#include <QDir>
#include <QString>

class PointSetUtilities
{
private:
  Eigen::Matrix3Xf EigenPointSet;

public:
  PointSetUtilities(Eigen::Matrix3Xf pointSet);

  // Setters
  void setEigenPointSet(Eigen::Matrix3Xf pointSet);

  // Getters
  Eigen::Matrix3Xf             getEigenPointSet();
  vtkSmartPointer< vtkPoints > getVTKPointSet();

  // Methods
  void saveToXyz(const char* fileName);
  // void saveToXyz(Qstring fileName);
};

#endif  // POINTSETUTILITES_HPP