/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include <QDebug>
#include <QtGlobal>

// errorHandler include
#include "../Utilities/include/debug/errorhandler.hpp"

// vtk includes
#include <vtkSmartPointer.h>

// Slicer Module includes
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>

// VolumeRendering Logic includes
#include <vtkSlicerVolumeRenderingLogic.h>
#include <vtkSlicerVolumeRenderingModuleLogicExport.h>

// VolumeRendering MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLVolumeRenderingDisplayNode.h>

// WorkspaceVisualization Logic includes
#include <vtkSlicerWorkspaceVisualizationLogic.h>

// WorkspaceVisualization includes
#include "qSlicerWorkspaceVisualizationModule.h"
#include "qSlicerWorkspaceVisualizationModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerWorkspaceVisualizationModulePrivate
{
public:
  qSlicerWorkspaceVisualizationModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerWorkspaceVisualizationModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerWorkspaceVisualizationModulePrivate::
  qSlicerWorkspaceVisualizationModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerWorkspaceVisualizationModule methods

//-----------------------------------------------------------------------------
qSlicerWorkspaceVisualizationModule::qSlicerWorkspaceVisualizationModule(
  QObject* _parent)
  : Superclass(_parent), d_ptr(new qSlicerWorkspaceVisualizationModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerWorkspaceVisualizationModule::~qSlicerWorkspaceVisualizationModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerWorkspaceVisualizationModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerWorkspaceVisualizationModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerWorkspaceVisualizationModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Dhruv Kool Rajamani (AIM Lab WPI)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerWorkspaceVisualizationModule::icon() const
{
  return QIcon(":/Icons/WorkspaceVisualization.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerWorkspaceVisualizationModule::categories() const
{
  return QStringList() << "SlicerRIGT";
}

//-----------------------------------------------------------------------------
QStringList qSlicerWorkspaceVisualizationModule::dependencies() const
{
  return QStringList() << "Volumes"
                       << "Models";
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceVisualizationModule::setup()
{
  qInstallMessageHandler(errorHandler);
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation*
  qSlicerWorkspaceVisualizationModule ::createWidgetRepresentation()
{
  vtkInfoMacro("Creating new WorkspaceVisualization Module Widget");
  return new qSlicerWorkspaceVisualizationModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerWorkspaceVisualizationModule::createLogic()
{
  return vtkSlicerWorkspaceVisualizationLogic::New();
}
