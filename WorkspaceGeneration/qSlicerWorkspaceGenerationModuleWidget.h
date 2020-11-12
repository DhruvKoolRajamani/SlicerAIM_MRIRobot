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

#ifndef __qSlicerWorkspaceGenerationModuleWidget_h
#define __qSlicerWorkspaceGenerationModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

// Volume Rendering includes
#include <qSlicerVolumeRenderingModuleWidget.h>
#include <qSlicerVolumeRenderingPresetComboBox.h>

// Volume Rendering Logic includes
#include <vtkSlicerVolumeRenderingLogic.h>
#include <vtkSlicerVolumeRenderingModuleLogicExport.h>

// Slicer Module includes
#include <qSlicerAbstractModule.h>
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>

#include "qSlicerWorkspaceGenerationModuleExport.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLVolumeNode.h"

class qSlicerWorkspaceGenerationModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_WORKSPACEGENERATION_EXPORT
  qSlicerWorkspaceGenerationModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerWorkspaceGenerationModuleWidget(QWidget* parent = 0);
  virtual ~qSlicerWorkspaceGenerationModuleWidget();

public slots:
  void setMRMLScene(vtkMRMLScene* scene);
  std::string GetClassName()
  {
    return "WorkspaceGenerationModuleWidget";
  }

protected slots:
  void onParameterNodeSelectionChanged();
  void onInputNodeSelectionChanged(vtkMRMLNode*);
  void onInputNodeNodeAdded(vtkMRMLNode*);
  void onOutputModelNodeAdded(vtkMRMLNode*);
  void onOutputModelSelectionChanged(vtkMRMLNode*);
  void onWorkspaceLoadButtonClick();
  void onApplyTransformClick();
  void onSceneImportedEvent();

  void updateGUIFromMRML();

  void blockAllSignals(bool block);
  void enableAllWidgets(bool enable);
  void disableWidgetsAfter(QWidget* widget);
  void enableWidgets(QWidget* widget, bool enable);

  void onOutputModelVisibilityChanged(bool visible);
  void onWorkspaceMeshVisibilityChanged(bool visible);

  void UpdateOutputModel();

protected:
  QScopedPointer< qSlicerWorkspaceGenerationModuleWidgetPrivate > d_ptr;

  vtkMRMLModelNode* GetOutputModelNode();
  vtkMRMLNode* GetInputNode();
  vtkMRMLVolumeNode* inputVolumeNode;

  QString workspaceMeshFilePath;

  vtkSlicerVolumeRenderingLogic* VolumeRenderingLogic;
  qSlicerAbstractCoreModule* VolumeRenderingModule;

  virtual void setup();
  virtual void enter();
  virtual void exit();

private:
  Q_DECLARE_PRIVATE(qSlicerWorkspaceGenerationModuleWidget);
  Q_DISABLE_COPY(qSlicerWorkspaceGenerationModuleWidget);
};

#endif
