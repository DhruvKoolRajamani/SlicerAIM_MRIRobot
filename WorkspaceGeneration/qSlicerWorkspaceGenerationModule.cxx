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
qSlicerWorkspaceGenerationModule::qSlicerWorkspaceGenerationModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerWorkspaceGenerationModulePrivate)
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
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerWorkspaceGenerationModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
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
  return QStringList() << "Examples";
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
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerWorkspaceGenerationModule
::createWidgetRepresentation()
{
  return new qSlicerWorkspaceGenerationModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerWorkspaceGenerationModule::createLogic()
{
  return vtkSlicerWorkspaceGenerationLogic::New();
}
