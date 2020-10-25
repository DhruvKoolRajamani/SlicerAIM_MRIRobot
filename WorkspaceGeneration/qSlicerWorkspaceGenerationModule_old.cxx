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

// WorkspaceGeneration Logic includes
#include <vtkSlicerWorkspaceGenerationLogic.h>

// WorkspaceGeneration includes
#include "qSlicerWorkspaceGenerationModule.h"
#include "qSlicerWorkspaceGenerationModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerWorkspaceGenerationModulePrivate
{
public:
  qSlicerWorkspaceGenerationModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerWorkspaceGenerationModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationModulePrivate::qSlicerWorkspaceGenerationModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerWorkspaceGenerationModule methods

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationModule::qSlicerWorkspaceGenerationModule(QObject *_parent)
    : Superclass(_parent), d_ptr(new qSlicerWorkspaceGenerationModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationModule::~qSlicerWorkspaceGenerationModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerWorkspaceGenerationModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerWorkspaceGenerationModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant 5R01CA166379-07";
}

//-----------------------------------------------------------------------------
QStringList qSlicerWorkspaceGenerationModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Dhruv Kool Rajamani (AIM Lab)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerWorkspaceGenerationModule::icon() const
{
  return QIcon(":/Icons/WorkspaceGeneration.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerWorkspaceGenerationModule::categories() const
{
  return QStringList() << "AIM MRIRobot";
}

//-----------------------------------------------------------------------------
QStringList qSlicerWorkspaceGenerationModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModule::setup()
{
  this->Superclass::setup();

  qSlicerAbstractCoreModule *volumeRenderingModule =
      qSlicerCoreApplication::application()->moduleManager()->module("VolumeRendering");
  vtkSlicerVolumeRenderingLogic *volumeRenderingLogic =
      volumeRenderingModule ? vtkSlicerVolumeRenderingLogic::SafeDownCast(volumeRenderingModule->logic()) : 0;

  if (mrmlScene())
  {
    cout << "Entered if statement in setup" << endl;
    vtkMRMLVolumeNode *volumeNode = static_cast<vtkMRMLVolumeNode *>(mrmlScene()->GetNodeByID(0)); // ('vtkMRMLScalarVolumeNode1');
  }

  if (volumeRenderingLogic)
  {
    vtkSmartPointer<vtkMRMLVolumeRenderingDisplayNode> displayNode =
        vtkSmartPointer<vtkMRMLVolumeRenderingDisplayNode>::Take(volumeRenderingLogic->CreateVolumeRenderingDisplayNode());
    // mrmlScene()->AddNode(displayNode);
    //   volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
    //   volumeRenderingLogic->UpdateDisplayNodeFromVolumeNode(displayNode, volumeNode);
  }
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation *qSlicerWorkspaceGenerationModule ::createWidgetRepresentation()
{
  return new qSlicerWorkspaceGenerationModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic *qSlicerWorkspaceGenerationModule::createLogic()
{
  static vtkSlicerWorkspaceGenerationLogic *workspaceGenerationLogicInstance =
      vtkSlicerWorkspaceGenerationLogic::New();
  return workspaceGenerationLogicInstance;
}
