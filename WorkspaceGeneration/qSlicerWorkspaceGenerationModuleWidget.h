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

// Annotation ROI Node
#include <vtkMRMLAnnotationROINode.h>

//  Markups nodes
#include "vtkMRMLMarkupsFiducialNode.h"
#include <vtkMRMLMarkupsNode.h>

// Slicer Module includes
#include <qSlicerAbstractModule.h>
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>

#include "ctkPushButton.h"

#include "qSlicerWorkspaceGenerationModuleExport.h"
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLSegmentationDisplayNode.h>
#include <vtkMRMLSegmentationNode.h>
#include <vtkMRMLVolumeNode.h>

// Neurorobot includes
#include "NeuroKinematics/NeuroKinematics.hpp"

class qSlicerWorkspaceGenerationModuleWidgetPrivate;
class vtkMRMLNode;

struct ProbeSpecifications
{
  double A;
  double B;
  double C;
  double D;

  Probe convertToProbe()
  {
    Probe probe;
    probe._treatmentToTip         = A;
    probe._robotToEntry           = B;
    probe._cannulaToTreatment     = C;
    probe._robotToTreatmentAtHome = D;

    return probe;
  }
};

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
  void        setMRMLScene(vtkMRMLScene* scene);
  std::string GetClassName()
  {
    return "WorkspaceGenerationModuleWidget";
  }

protected slots:
  void onParameterNodeSelectionChanged();
  void onInputVolumeNodeSelectionChanged(vtkMRMLNode*);
  void onInputVolumeNodeAdded(vtkMRMLNode*);
  void onAnnotationROINodeAdded(vtkMRMLNode*);
  void onAnnotationROISelectionChanged(vtkMRMLNode*);
  void onPresetComboBoxNodeChanged(vtkMRMLNode*);
  void onBurrHoleSegmentationNodeAdded(vtkMRMLNode*);
  void onBurrHoleSegmentationNodeChanged(vtkMRMLNode*);
  void onBHExtremePointAdded(vtkMRMLNode*);
  void onBHExtremePointChanged(vtkMRMLNode*);
  void onEntryPointAdded(vtkMRMLNode*);
  void onEntryPointSelectionChanged(vtkMRMLNode*);
  void onTargetPointSelectionChanged(vtkMRMLNode*);
  void onTargetPointAdded(vtkMRMLNode*);
  void onMarkupChanged(vtkObject*, unsigned long, void*);
  void onPresetOffsetChanged(double, double, bool);
  void onWorkspaceMeshSegmentationNodeChanged(vtkMRMLNode*);
  void onWorkspaceMeshSegmentationNodeAdded(vtkMRMLNode*);
  void onGenerateWorkspaceClick();
  void onDetectBurrHoleClick();
  void onSceneImportedEvent();

  // // DEPRECATED
  // void onWorkspaceLoadButtonClick();
  // void onApplyTransformClick();

  void subscribeToMarkupEvents(vtkMRMLMarkupsFiducialNode*);
  void markupPlacedEventHandler(vtkMRMLMarkupsNode*);

  void updateGUIFromMRML();

  void blockAllSignals(bool block);
  void enableAllWidgets(bool enable);
  void disableWidgetsAfter(QWidget* widgetStart, QWidget* widgetEnd = NULL,
                           bool includingStart = false,
                           bool includingEnd   = true);
  void disableWidgetsBetween(QWidget* start, QWidget* end = NULL,
                             bool includeStart = false,
                             bool includeEnd   = false);
  void enableWidgets(QWidget* widget, bool enable);

  void onInputVolumeVisibilityChanged(bool visible);
  void onWorkspaceMeshVisibilityChanged(bool visible);
  void onBurrHoleVisibilityChanged(bool visible);

  void UpdateVolumeRendering();

protected:
  QList< QWidget* > allInteractiveWidgets;

  QScopedPointer< qSlicerWorkspaceGenerationModuleWidgetPrivate > d_ptr;

  vtkMRMLAnnotationROINode* GetAnnotationROINode();
  vtkMRMLVolumeNode*        GetInputVolumeNode();

  vtkSlicerVolumeRenderingLogic* VolumeRenderingLogic;
  qSlicerAbstractCoreModule*     VolumeRenderingModule;

  virtual void setup();
  virtual void enter();
  virtual void exit();

  void setCheckState(ctkPushButton* btn, bool state);

private:
  Q_DECLARE_PRIVATE(qSlicerWorkspaceGenerationModuleWidget);
  Q_DISABLE_COPY(qSlicerWorkspaceGenerationModuleWidget);
};

#endif
