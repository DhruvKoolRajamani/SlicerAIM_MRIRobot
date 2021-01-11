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

// Qt includes
#include <QButtonGroup>
#include <QFileDialog>
#include <QtGui>

#include "../Utilities/include/debug/errorhandler.hpp"

#include "qSlicerApplication.h"

// SlicerQt includes
#include "qSlicerWorkspaceGenerationModuleWidget.h"
#include "ui_qSlicerWorkspaceGenerationModuleWidget.h"

// Slicer includes
#include "vtkImageData.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTransformableNode.h"
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMatrix4x4.h"
#include "vtkProperty.h"
#include "vtkSmartPointer.h"
#include "vtkXMLImageDataReader.h"
#include "vtkXMLImageDataWriter.h"

// module includes
#include "vtkMRMLWorkspaceGenerationNode.h"
#include "vtkSlicerWorkspaceGenerationLogic.h"

// Isosurface creation
#include <vtkMarchingCubes.h>
#include <vtkStripper.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerWorkspaceGenerationModuleWidgetPrivate
  : public Ui_qSlicerWorkspaceGenerationModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerWorkspaceGenerationModuleWidget);

protected:
  qSlicerWorkspaceGenerationModuleWidget* const q_ptr;

public:
  qSlicerWorkspaceGenerationModuleWidgetPrivate(
    qSlicerWorkspaceGenerationModuleWidget& object);
  vtkSlicerWorkspaceGenerationLogic* logic() const;

  ProbeSpecifications ProbeSpecs;

  // Observed nodes (to keep GUI up-to-date)
  vtkMRMLWorkspaceGenerationNode* WorkspaceGenerationNode;

  // vtkMRMLVolumeNode* InputVolumeNode;
  // vtkMRMLAnnotationROINode* AnnotationROINode;
  vtkMRMLModelNode* WorkspaceMeshModelNode;

  vtkMRMLVolumeRenderingDisplayNode* InputVolumeRenderingDisplayNode;
  vtkMRMLModelDisplayNode*           WorkspaceMeshModelDisplayNode;
  vtkMRMLMarkupsDisplayNode*         EntryPointDisplayNode;
  vtkMRMLMarkupsDisplayNode*         TargetPointDisplayNode;

  vtkMRMLVolumePropertyNode* VolumePropertyNode;

  vtkMatrix4x4*         WorkspaceMeshRegistrationMatrix;
  vtkMRMLTransformNode* WorkspaceMeshTransformNode;
};

//-----------------------------------------------------------------------------
// qSlicerWorkspaceGenerationModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationModuleWidgetPrivate::
  qSlicerWorkspaceGenerationModuleWidgetPrivate(
    qSlicerWorkspaceGenerationModuleWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
vtkSlicerWorkspaceGenerationLogic*
  qSlicerWorkspaceGenerationModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerWorkspaceGenerationModuleWidget);
  return vtkSlicerWorkspaceGenerationLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerWorkspaceGenerationModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationModuleWidget::qSlicerWorkspaceGenerationModuleWidget(
  QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerWorkspaceGenerationModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerWorkspaceGenerationModuleWidget::
  ~qSlicerWorkspaceGenerationModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::setup()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  qInfo() << Q_FUNC_INFO;

  QList< QWidget* > allWidgets = this->findChildren< QWidget* >(
    QRegularExpression("^((?![lL]abel)(?!qt_).)*$",
                       QRegularExpression::MultilineOption |
                         QRegularExpression::DotMatchesEverythingOption));

  allInteractiveWidgets = QList< QWidget* >();
  foreach (QWidget* w, allWidgets)
  {
    if (QString::compare(w->objectName(), "") != 0)
    {
      allInteractiveWidgets.append(w);
    }
  }

  // qDebug() << allInteractiveWidgets;

  // Connect buttons in UI
  this->setMRMLScene(d->logic()->GetMRMLScene());
  this->VolumeRenderingModule = d->logic()->getVolumeRenderingModule();
  this->VolumeRenderingLogic  = d->logic()->getVolumeRenderingLogic();

  connect(d->ParameterNodeSelector__1_1,
          SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
          SLOT(onParameterNodeSelectionChanged()));
  connect(d->InputVolumeNodeSelector__2_2,
          SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
          SLOT(onInputVolumeNodeSelectionChanged(vtkMRMLNode*)));
  connect(d->InputVolumeNodeSelector__2_2,
          SIGNAL(nodeAddedByUser(vtkMRMLNode*)), this,
          SLOT(onInputVolumeNodeAdded(vtkMRMLNode*)));
  connect(d->ROINodeSelector__2_4, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onAnnotationROISelectionChanged(vtkMRMLNode*)));
  connect(d->ROINodeSelector__2_4, SIGNAL(nodeAddedByUser(vtkMRMLNode*)), this,
          SLOT(onAnnotationROINodeAdded(vtkMRMLNode*)));
  connect(d->InputVolumeSetVisibilityCheckBox__2_3, SIGNAL(toggled(bool)), this,
          SLOT(onInputVolumeVisibilityChanged(bool)));
  // connect(d->InputVolumeRenderingPresetComboBox,
  //         SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
  //         SLOT(onPresetComboBoxNodeChanged(vtkMRMLNode*)));
  // connect(d->InputVolumeRenderingPresetComboBox,
  // SIGNAL(presetOffsetChanged()),
  //         this, SLOT(onPresetComboBoxNodeChanged(vtkMRMLNode*)));
  connect(d->WorkspaceModelSelector__3_2,
          SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
          SLOT(onWorkspaceMeshModelNodeChanged(vtkMRMLNode*)));
  connect(d->WorkspaceModelSelector__3_2, SIGNAL(nodeAddedByUser(vtkMRMLNode*)),
          this, SLOT(onWorkspaceMeshModelNodeAdded(vtkMRMLNode*)));
  connect(d->GenerateWorkspaceButton__3_11, SIGNAL(released()), this,
          SLOT(onGenerateWorkspaceClick()));
  connect(d->WorkspaceVisibilityToggle__3_12, SIGNAL(toggled(bool)), this,
          SLOT(onWorkspaceMeshVisibilityChanged(bool)));
  connect(d->EntryPointFiducialSelector__4_2,
          SIGNAL(nodeAddedByUser(vtkMRMLNode*)), this,
          SLOT(onEntryPointAdded(vtkMRMLNode*)));
  connect(d->EntryPointFiducialSelector__4_2,
          SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
          SLOT(onEntryPointSelectionChanged(vtkMRMLNode*)));
  connect(d->TargetPointFiducialSelector__4_4,
          SIGNAL(nodeAddedByUser(vtkMRMLNode*)), this,
          SLOT(onTargetPointAdded(vtkMRMLNode*)));
  connect(d->TargetPointFiducialSelector__4_4,
          SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
          SLOT(onTargetPointSelectionChanged(vtkMRMLNode*)));
  connect(d->GenerateIsosurfaceButton__5_2, SIGNAL(released()), this,
          SLOT(onGenerateIsoSurfaceClick()));

  d->EntryPointMarkupsPlaceWidget__4_3->setPlaceMultipleMarkups(
    qSlicerMarkupsPlaceWidget::PlaceMultipleMarkupsType::
      ForcePlaceSingleMarkup);
  d->TargetPointMarkupsPlaceWidget__4_5->setPlaceMultipleMarkups(
    qSlicerMarkupsPlaceWidget::PlaceMultipleMarkupsType::
      ForcePlaceSingleMarkup);
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onSceneImportedEvent()
{
  qInfo() << Q_FUNC_INFO;

  // Replace with registration/generation logic?
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::enter()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  this->Superclass::enter();

  qInfo() << Q_FUNC_INFO;

  if (this->mrmlScene() == NULL)
  {
    qCritical() << Q_FUNC_INFO << "Invalid scene!";
    return;
  }

  // For convenience, select a default parameter node.
  if (d->ParameterNodeSelector__1_1->currentNode() == NULL)
  {
    qCritical() << Q_FUNC_INFO << "No node available currently";

    vtkMRMLNode* node =
      this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLWorkspaceGenerationNode");
    if (node == NULL)
    {
      node =
        this->mrmlScene()->AddNewNodeByClass("vtkMRMLWorkspaceGenerationNode");
    }
    // Create a new parameter node if there is none in the scene.
    if (node == NULL)
    {
      qCritical() << Q_FUNC_INFO << "Failed to create module node";
      return;
    }

    d->ParameterNodeSelector__1_1->setMRMLScene(this->mrmlScene());
    d->ParameterNodeSelector__1_1->setCurrentNode(node);
  }

  // Need to update the GUI so that it observes whichever parameter node is
  // selected
  this->onParameterNodeSelectionChanged();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::exit()
{
  qInfo() << Q_FUNC_INFO;
  Superclass::exit();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  qInfo() << Q_FUNC_INFO;

  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  this->Superclass::setMRMLScene(scene);

  qvtkReconnect(d->logic(), scene, vtkMRMLScene::EndImportEvent, this,
                SLOT(onSceneImportedEvent()));
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onParameterNodeSelectionChanged()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* selectedWorkspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  qvtkReconnect(d->WorkspaceGenerationNode, selectedWorkspaceGenerationNode,
                vtkCommand::ModifiedEvent, this, SLOT(updateGUIFromMRML()));

  d->WorkspaceGenerationNode = selectedWorkspaceGenerationNode;
  d->logic()->UpdateSelectionNode(selectedWorkspaceGenerationNode);

  setCheckState(d->InputVolumeSetVisibilityCheckBox__2_3, false);
  setCheckState(d->WorkspaceVisibilityToggle__3_12, false);

  // Set default probe specs
  double _cannulaToTreatment{5.0};       // C
  double _treatmentToTip{10.0};          // A
  double _robotToEntry{5.0};             // B
  double _robotToTreatmentAtHome{41.0};  // D

  d->A_DoubleSpinBox__3_5->setValue(_treatmentToTip);
  d->B_DoubleSpinBox__3_6->setValue(_robotToEntry);
  d->C_DoubleSpinBox__3_7->setValue(_cannulaToTreatment);
  d->D_DoubleSpinBox__3_8->setValue(_robotToTreatmentAtHome);

  d->A_DoubleSpinBox__3_5->setEnabled(true);
  d->B_DoubleSpinBox__3_6->setEnabled(true);
  d->C_DoubleSpinBox__3_7->setEnabled(true);
  d->D_DoubleSpinBox__3_8->setEnabled(true);
  d->RegistrationMatrix__3_10->setEnabled(true);
  d->RegistrationMatrix__3_10->setEditable(true);

  // Set temporary values for temp mri image.
  // TODO: Remove once testing pig images
  auto regMat = d->RegistrationMatrix__3_10->values();
  regMat[3]   = -100;
  regMat[7]   = 300;
  regMat[11]  = 100;
  qDebug() << Q_FUNC_INFO << regMat;
  d->RegistrationMatrix__3_10->setValues(regMat);

  this->updateGUIFromMRML();
}

// 1.1 Load input volume and render volume
//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onInputVolumeNodeSelectionChanged(
  vtkMRMLNode* nodeSelected)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  if (workspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": invalid workspaceGenerationNode";
    return;
  }

  if (nodeSelected == NULL)
  {
    workspaceGenerationNode->SetAndObserveInputVolumeNodeID(NULL);
    qCritical() << Q_FUNC_INFO << ": unexpected input node type";

    return;
  }

  vtkMRMLVolumeNode* inputVolumeNode =
    vtkMRMLVolumeNode::SafeDownCast(nodeSelected);

  if (inputVolumeNode != NULL)
  {

    qInfo() << Q_FUNC_INFO << ": Input Volume Node selected.";

    workspaceGenerationNode->SetAndObserveInputVolumeNodeID(
      inputVolumeNode->GetID());
  }

  this->updateGUIFromMRML();
}

// 1.1 Load input volume and render volume
//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onInputVolumeNodeAdded(
  vtkMRMLNode* addedNode)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;
}

// 1.2 Rendered volume output automatically sets an ROI
//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onAnnotationROISelectionChanged(
  vtkMRMLNode* selectedNode)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  if (workspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": invalid workspaceGenerationNode";
    return;
  }

  vtkMRMLVolumeNode* inputVolumeNode =
    workspaceGenerationNode->GetInputVolumeNode();
  if (!inputVolumeNode)
  {
    qCritical() << Q_FUNC_INFO << ": input Node has not been added yet.";
    return;
  }

  vtkMRMLAnnotationROINode* annotationROINode =
    vtkMRMLAnnotationROINode::SafeDownCast(selectedNode);

  if (!annotationROINode)
  {
    qCritical() << Q_FUNC_INFO << ": No AnnotationROI node selected";
    workspaceGenerationNode->SetAndObserveAnnotationROINodeID(NULL);
    return;
  }

  workspaceGenerationNode->SetAndObserveAnnotationROINodeID(
    annotationROINode->GetID());

  // Create logic to accommodate creating a new annotation ROI node.
  // Should you transfer the data to the new node? Reset all visibility params?

  this->updateGUIFromMRML();
}

// 1.2 Rendered volume output automatically sets an ROI
//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onAnnotationROINodeAdded(
  vtkMRMLNode* addedNode)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;

  vtkMRMLAnnotationROINode* annotationROINode =
    vtkMRMLAnnotationROINode::SafeDownCast(addedNode);

  if (annotationROINode == NULL)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid node";
    return;
  }
}

// 1.3 Rendered volume visibility only changes after volume is rendered
// --------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onInputVolumeVisibilityChanged(
  bool visible)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* selectedWorkspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  if (selectedWorkspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": No workspace generation node created yet.";
    return;
  }

  if (selectedWorkspaceGenerationNode->GetInputVolumeNode() == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": No input volume specified.";
    return;
  }

  // Check if volume rendering display node for volume is null.
  if (d->InputVolumeRenderingDisplayNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": No volume rendering display node";
    return;
  }

  d->InputVolumeRenderingDisplayNode->SetVisibility(visible);
  // d->InputVolumeRenderingPresetComboBox->setEnabled(visible);

  // Update widget from display node of the volume node
  this->updateGUIFromMRML();
}

// =============================================================================
// ================================= TODO ======================================
// =============================================================================
// bug: #13 fix volume rendering preset combo box widget @DhruvKoolRajamani
// --------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onPresetComboBoxNodeChanged(
  vtkMRMLNode* selectedNode)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  if (workspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": invalid workspaceGenerationNode";
    return;
  }

  if (workspaceGenerationNode->GetInputVolumeNode() == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": No input volume specified.";
    return;
  }

  // Check if volume rendering display node for volume is null.
  if (d->InputVolumeRenderingDisplayNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": No volume rendering display node";
    return;
  }

  vtkMRMLVolumePropertyNode* volumePropertyNode =
    vtkMRMLVolumePropertyNode::SafeDownCast(selectedNode);

  if (volumePropertyNode == NULL)
  {
    qCritical() << Q_FUNC_INFO
                << ": Selected node is not a volume property node";
    return;
  }

  // Get the current Volume Property Node.
  // d->VolumePropertyNode =
  //   d->InputVolumeRenderingDisplayNode->GetVolumePropertyNode();

  // Have the preset combo box observe the vol rendering display property
  // node.
  // d->InputVolumeRenderingPresetComboBox->setMRMLVolumePropertyNode(
  //   selectedNode);

  // this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onPresetOffsetChanged(double,
                                                                   double, bool)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;
}
// =============================================================================

// 2.1 Generate general workspace of the robot
//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onWorkspaceMeshModelNodeChanged(
  vtkMRMLNode* nodeSelected)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  if (workspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": invalid workspaceGenerationNode";

    workspaceGenerationNode->SetAndObserveWorkspaceMeshModelNodeID(NULL);
    d->RegistrationMatrix__3_10->setDisabled(true);
    d->WorkspaceMeshModelDisplayNode = NULL;

    return;
  }

  if (nodeSelected == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": unexpected workspace mesh model node type";

    workspaceGenerationNode->SetAndObserveWorkspaceMeshModelNodeID(NULL);
    d->RegistrationMatrix__3_10->setDisabled(true);
    d->WorkspaceMeshModelDisplayNode = NULL;

    return;
  }

  vtkMRMLModelNode* workspaceMeshModelNode =
    vtkMRMLModelNode::SafeDownCast(nodeSelected);

  if (workspaceMeshModelNode == NULL)
  {
    qCritical() << Q_FUNC_INFO
                << ": workspace mesh node has not been added yet.";

    workspaceGenerationNode->SetAndObserveWorkspaceMeshModelNodeID(NULL);
    d->RegistrationMatrix__3_10->setDisabled(true);
    d->WorkspaceMeshModelDisplayNode = NULL;

    return;
  }

  workspaceGenerationNode->SetAndObserveWorkspaceMeshModelNodeID(
    workspaceMeshModelNode->GetID());
  auto modelDisplayNode = workspaceMeshModelNode->GetModelDisplayNode();
  qvtkReconnect(d->WorkspaceMeshModelDisplayNode, modelDisplayNode,
                vtkCommand::ModifiedEvent, this, SLOT(updateGUIFromMRML()));
  // d->WorkspaceMeshModelDisplayNode = modelDisplayNode;
  d->logic()->setWorkspaceMeshModelDisplayNode(modelDisplayNode);

  // Create logic to accommodate creating a new annotation ROI node.
  // Should you transfer the data to the new node? Reset all visibility params?

  this->updateGUIFromMRML();
}

// 2.1 Generate general workspace of the robot
//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onWorkspaceMeshModelNodeAdded(
  vtkMRMLNode* addedNode)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  if (workspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": invalid workspaceGenerationNode";
    return;
  }

  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(addedNode);
  if (modelNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": Failed, invalid node";
    return;
  }

  if (modelNode->GetName())
  {
    std::string outputModelNodeName = "GeneralWorkspace";
    // std::string(modelNode->GetName()).append("GeneralWorkspace");
    modelNode->SetName(outputModelNodeName.c_str());
  }
}

// 2.1 Generate general workspace of the robot
// --------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onGenerateWorkspaceClick()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  if (workspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": invalid workspaceGenerationNode";
    return;
  }

  vtkMRMLModelNode* workspaceMeshModelNode =
    workspaceGenerationNode->GetWorkspaceMeshModelNode();

  if (!workspaceMeshModelNode)
  {
    qCritical() << Q_FUNC_INFO << ": No workspace mesh model node created";
    return;
  }

  d->ProbeSpecs = {
    d->A_DoubleSpinBox__3_5->value(),  // _treatmentToTip
    d->B_DoubleSpinBox__3_6->value(),  // _robotToEntry
    d->C_DoubleSpinBox__3_7->value(),  // _cannulaToTreatment
    d->D_DoubleSpinBox__3_8->value()   // _robotToTreatmentAtHome
  };

  d->WorkspaceMeshRegistrationMatrix = vtkMatrix4x4::New();
  d->WorkspaceMeshRegistrationMatrix->DeepCopy(
    d->RegistrationMatrix__3_10->values().data());

  d->logic()->GenerateWorkspace(workspaceMeshModelNode,
                                d->ProbeSpecs.convertToProbe(),
                                d->WorkspaceMeshRegistrationMatrix);

  // d->WorkspaceMeshModelNode = d->logic()->getWorkspaceMeshModelNode();
  d->WorkspaceMeshModelNode = workspaceMeshModelNode;
  d->WorkspaceModelSelector__3_2->setCurrentNode(workspaceMeshModelNode);

  // workspaceMeshModelNode->ApplyTransformMatrix(
  //   d->WorkspaceMeshRegistrationMatrix);

  this->updateGUIFromMRML();
}

// 2.2 Workspace visibility can change after workspace is generated
// --------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onWorkspaceMeshVisibilityChanged(
  bool visible)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* selectedWorkspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  if (selectedWorkspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": No workspace generation node created yet.";
    return;
  }

  vtkMRMLModelNode* workspaceMeshModelNode =
    d->logic()->getWorkspaceMeshModelNode();

  if (!workspaceMeshModelNode)
  {
    qCritical() << Q_FUNC_INFO << ": No workspace mesh model node created";
    return;
  }

  // Get volume rendering display node for volume. Create if absent.
  if (!d->WorkspaceMeshModelDisplayNode)
  {
    qCritical() << Q_FUNC_INFO << ": No workspace mesh model display node";
    return;
  }

  d->WorkspaceMeshModelDisplayNode->SetVisibility(visible);

  // Update widget from display node of the volume node
  this->updateGUIFromMRML();
}

/** ------------------------------- DEPRECATED ---------------------------------
//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onWorkspaceLoadButtonClick()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;

  auto fileName = QFileDialog::getOpenFileName(this, tr("Open Workspace Mesh"),
                                               QDir::currentPath(),
                                               tr("Polymesh File (*.ply)"));

  if (fileName.isEmpty())
  {
    // Return if no path is specified
    qCritical() << Q_FUNC_INFO << ": No filepath specified";
    return;
  }

  if (d->logic()->LoadWorkspace(fileName))
  {
    d->WorkspaceMeshModelNode = d->logic()->getWorkspaceMeshModelNode();
    d->WorkspaceModelSelector__3_2->setCurrentNode(d->WorkspaceGenerationNode);
  }
}

// --------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onApplyTransformClick()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  if (workspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": invalid workspaceGenerationNode";
    return;
  }

  vtkMRMLModelNode* workspaceMeshModelNode =
    workspaceGenerationNode->GetWorkspaceMeshModelNode();

  if (workspaceMeshModelNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": Workspace Mesh Model Node does not exist";
    return;
  }

  d->WorkspaceMeshRegistrationMatrix = vtkMatrix4x4::New();
  d->WorkspaceMeshRegistrationMatrix->DeepCopy(
    d->RegistrationMatrix__3_10->values().data());

  workspaceMeshModelNode->ApplyTransformMatrix(
    d->WorkspaceMeshRegistrationMatrix);

  this->updateGUIFromMRML();
}
//-----------------------------------------------------------------------------
*/

// 1 + 2 = 3.1 Place Entry Point.
//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onEntryPointAdded(
  vtkMRMLNode* addedNode)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  if (workspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": invalid workspaceGenerationNode";
    return;
  }

  vtkMRMLMarkupsFiducialNode* markupNode =
    vtkMRMLMarkupsFiducialNode::SafeDownCast(addedNode);
  if (markupNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": Failed, invalid node";
    return;
  }

  if (markupNode->GetName())
  {
    std::string outputModelNodeName = "EntryPoint";
    // std::string(modelNode->GetName()).append("GeneralWorkspace");
    markupNode->SetName(outputModelNodeName.c_str());
  }
}

// 1 + 2 = 3.1 Place Entry Point.
//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onEntryPointSelectionChanged(
  vtkMRMLNode* selectedNode)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  if (workspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": invalid workspaceGenerationNode";

    workspaceGenerationNode->SetAndObserveEntryPointNodeId(NULL);
    d->EntryPointDisplayNode = NULL;

    return;
  }

  if (selectedNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": unexpected markup node";

    workspaceGenerationNode->SetAndObserveEntryPointNodeId(NULL);
    d->EntryPointDisplayNode = NULL;

    return;
  }

  vtkMRMLMarkupsFiducialNode* entryPointNode =
    vtkMRMLMarkupsFiducialNode::SafeDownCast(selectedNode);

  if (entryPointNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": Markup Fiducial has not been added yet.";

    workspaceGenerationNode->SetAndObserveEntryPointNodeId(NULL);
    d->EntryPointDisplayNode = NULL;

    return;
  }

  workspaceGenerationNode->SetAndObserveEntryPointNodeId(
    entryPointNode->GetID());
  if (workspaceGenerationNode->GetTargetPointNode() == NULL)
  {
    entryPointNode->CreateDefaultDisplayNodes();
    vtkMRMLMarkupsDisplayNode* entryPointDisplayNode =
      vtkMRMLMarkupsDisplayNode::SafeDownCast(entryPointNode->GetDisplayNode());
    qvtkReconnect(d->EntryPointDisplayNode, entryPointDisplayNode,
                  vtkCommand::ModifiedEvent, this, SLOT(updateGUIFromMRML()));
    d->EntryPointDisplayNode = entryPointDisplayNode;
  }

  subscribeToMarkupEvents(entryPointNode);

  // Create logic to accommodate creating a new annotation ROI node.
  // Should you transfer the data to the new node? Reset all visibility params?

  this->updateGUIFromMRML();
}

// 3.2 (can be independent of 3.1) - Place Target Point
//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onTargetPointAdded(
  vtkMRMLNode* addedNode)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  if (workspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": invalid workspaceGenerationNode";
    return;
  }

  vtkMRMLMarkupsFiducialNode* markupNode =
    vtkMRMLMarkupsFiducialNode::SafeDownCast(addedNode);
  if (markupNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": Failed, invalid node";
    return;
  }

  if (markupNode->GetName())
  {
    std::string outputModelNodeName = "TargetPoint";
    // std::string(modelNode->GetName()).append("GeneralWorkspace");
    markupNode->SetName(outputModelNodeName.c_str());
  }
}

// 3.2 (can be independent of 3.1) - Place Target Point
//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onTargetPointSelectionChanged(
  vtkMRMLNode* selectedNode)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  if (workspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": invalid workspaceGenerationNode";

    workspaceGenerationNode->SetAndObserveTargetPointNodeId(NULL);
    d->TargetPointDisplayNode = NULL;

    return;
  }

  if (selectedNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": unexpected markup node";

    workspaceGenerationNode->SetAndObserveTargetPointNodeId(NULL);
    d->TargetPointDisplayNode = NULL;

    return;
  }

  vtkMRMLMarkupsFiducialNode* targetPointNode =
    vtkMRMLMarkupsFiducialNode::SafeDownCast(selectedNode);

  if (targetPointNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": Markup Fiducial has not been added yet.";

    workspaceGenerationNode->SetAndObserveTargetPointNodeId(NULL);
    d->TargetPointDisplayNode = NULL;

    return;
  }

  workspaceGenerationNode->SetAndObserveTargetPointNodeId(
    targetPointNode->GetID());
  if (workspaceGenerationNode->GetEntryPointNode() == NULL)
  {
    targetPointNode->CreateDefaultDisplayNodes();
    vtkMRMLMarkupsDisplayNode* targetPointDisplayNode =
      vtkMRMLMarkupsDisplayNode::SafeDownCast(
        targetPointNode->GetDisplayNode());
    qvtkReconnect(d->TargetPointDisplayNode, targetPointDisplayNode,
                  vtkCommand::ModifiedEvent, this, SLOT(updateGUIFromMRML()));
    d->TargetPointDisplayNode = targetPointDisplayNode;
  }

  subscribeToMarkupEvents(targetPointNode);

  // Create logic to accommodate creating a new annotation ROI node.
  // Should you transfer the data to the new node? Reset all visibility params?

  this->updateGUIFromMRML();
}

// 3. Markup event handling!!!
//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::subscribeToMarkupEvents(
  vtkMRMLMarkupsFiducialNode* markup)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  markup->AddObserver(vtkMRMLMarkupsNode::PointModifiedEvent, this,
                      &qSlicerWorkspaceGenerationModuleWidget::onMarkupChanged);

  markup->AddObserver(vtkMRMLMarkupsNode::PointAddedEvent, this,
                      &qSlicerWorkspaceGenerationModuleWidget::onMarkupChanged);

  markup->AddObserver(vtkMRMLMarkupsNode::PointRemovedEvent, this,
                      &qSlicerWorkspaceGenerationModuleWidget::onMarkupChanged);

  markup->AddObserver(vtkMRMLMarkupsNode::PointStartInteractionEvent, this,
                      &qSlicerWorkspaceGenerationModuleWidget::onMarkupChanged);

  // Important to identify when marker has been dropped after moving
  markup->AddObserver(vtkMRMLMarkupsNode::PointEndInteractionEvent, this,
                      &qSlicerWorkspaceGenerationModuleWidget::onMarkupChanged);

  // Important to identify when marker has been dropped after adding
  markup->AddObserver(vtkMRMLMarkupsNode::PointPositionDefinedEvent, this,
                      &qSlicerWorkspaceGenerationModuleWidget::onMarkupChanged);

  markup->AddObserver(vtkMRMLMarkupsNode::PointPositionUndefinedEvent, this,
                      &qSlicerWorkspaceGenerationModuleWidget::onMarkupChanged);
}

// 3. Markup event handling!!!
//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onMarkupChanged(
  vtkObject* caller, unsigned long event, void* vtkNotUsed(data))
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  // qDebug() <<
  // "===============================================================";
  std::string         eventName;
  vtkMRMLMarkupsNode* markupNode = vtkMRMLMarkupsNode::SafeDownCast(caller);
  switch (event)
  {
    case vtkMRMLMarkupsNode::PointAddedEvent:
      eventName = "vtkMRMLMarkupsNode::PointAddedEvent";
      d->logic()->UpdateMarkupFiducialNodes();
      break;
    case vtkMRMLMarkupsNode::PointRemovedEvent:
      eventName = "vtkMRMLMarkupsNode::PointRemovedEvent";
      break;
    case vtkMRMLMarkupsNode::PointModifiedEvent:
      eventName = "vtkMRMLMarkupsNode::PointModifiedEvent";
      break;
    case vtkMRMLMarkupsNode::PointStartInteractionEvent:
      eventName = "vtkMRMLMarkupsNode::PointStartInteractionEvent";
      break;
    case vtkMRMLMarkupsNode::PointEndInteractionEvent:
      eventName = "vtkMRMLMarkupsNode::PointEndInteractionEvent";
      // qDebug() << "Marker Name: " << markupNode->GetName();
      this->markupPlacedEventHandler(markupNode);
      break;
    case vtkMRMLMarkupsNode::PointPositionDefinedEvent:
      eventName = "vtkMRMLMarkupsNode::PointPositionDefinedEvent";
      // qDebug() << "Marker Name: " << markupNode->GetName();
      // d->logic()->UpdateMarkupFiducialNodes();
      this->markupPlacedEventHandler(markupNode);
      break;
    case vtkMRMLMarkupsNode::PointPositionUndefinedEvent:
      eventName = "vtkMRMLMarkupsNode::PointPositionUndefinedEvent";
      break;
    default:
      eventName = "UNKNOWN";
      break;
  }
  // qDebug() << eventName.c_str();
  // qDebug() <<
  // "===============================================================";
}

// 3. Markup event handling!!!
// Special function demands detailed description.
// Once steps 1. Input Volume, 2. Workspace Generation are complete.
// Event triggered: Marker placed and position is defined
//          - Entry Point:
//            - First Time:
//              Generate subworkspace with apriori parameters.
//              Use assumed static skull thickness from stl model of bit
//            - Successive Times:
//              Update subworkspace, this time use updated burr hole
//              specifications. (Consider using constant bit size, or
//              update the radius -> if disproportionate, move marker
//              until radius is equal???)
//          - Target Point:
//            - First time:
//              if Entry Point has been placed;
//                then draw axis from EP to TP and use for initial burr
//                hole estimate.
//                DO identifyBurrHoleRecursively()
//              else DO NOTHING
//            - Successive Times:
//              Only being moved around for planning
//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::markupPlacedEventHandler(
  vtkMRMLMarkupsNode* markup)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qDebug() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  vtkMRMLMarkupsFiducialNode* entryPointNode =
    workspaceGenerationNode->GetEntryPointNode();

  vtkMRMLMarkupsFiducialNode* targetPointNode =
    workspaceGenerationNode->GetTargetPointNode();

  bool burrholeSet = workspaceGenerationNode->GetBurrHoleDetected();

  if (entryPointNode == NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Entry point has not been created.";
    return;
  }

  if (targetPointNode == NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Target point has not been created.";
    return;
  }

  if (entryPointNode->GetNumberOfDefinedControlPoints() == 0)
  {
    qWarning() << Q_FUNC_INFO << ": Entry point has not been placed yet.";
    return;
  }

  // Calculate Subworkspace
  d->logic()->UpdateSubWorkspace(workspaceGenerationNode, burrholeSet);

  if (targetPointNode->GetNumberOfDefinedControlPoints() == 0)
  {
    qWarning() << Q_FUNC_INFO << ": Target point has not been placed yet.";
    return;
  }

  // Easy to modify this to a lock if asynchronousity is required.
  if (!burrholeSet)
  {
    burrholeSet = d->logic()->IdentifyBurrHole(workspaceGenerationNode);

    // Should be ideally moved to burr hole detection.
    workspaceGenerationNode->SetBurrHoleDetected(burrholeSet);
  }
}

// 4. Generate IsoSurface for burr hole identification
//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onGenerateIsoSurfaceClick()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;
}

//-----------------------------------------------------------------------------
vtkMRMLAnnotationROINode*
  qSlicerWorkspaceGenerationModuleWidget::GetAnnotationROINode()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  if (workspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": Selected node not a valid module node";
    return NULL;
  }

  return workspaceGenerationNode->GetAnnotationROINode();
}

//-----------------------------------------------------------------------------
vtkMRMLVolumeNode* qSlicerWorkspaceGenerationModuleWidget::GetInputVolumeNode()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  if (workspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": Selected node not a valid module node";
    return NULL;
  }

  return workspaceGenerationNode->GetInputVolumeNode();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::setCheckState(ctkPushButton* btn,
                                                           bool           state)
{
  qInfo() << Q_FUNC_INFO;

  if (!btn)
  {
    qCritical() << Q_FUNC_INFO << ": Button does not exist!";
    return;
  }

  btn->setCheckable(true);
  btn->setChecked(state);
  auto color = ((state) ? "background-color: green" : "background-color: red");
  btn->setStyleSheet(color);
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::updateGUIFromMRML()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  // Check if workspace generation node exists
  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  d->WorkspaceGenerationNode = workspaceGenerationNode;

  if (!d->WorkspaceGenerationNode)
  {
    qCritical() << Q_FUNC_INFO << ": Selected node not a valid module node";
    this->enableAllWidgets(false);
    return;
  }

  d->logic()->setWorkspaceGenerationNode(workspaceGenerationNode);

  this->enableAllWidgets(true);

  // d->InputVolumeNodeSelector__2_2->blockSignals(true);
  // Set mrml scene in input volume node selector
  d->InputVolumeNodeSelector__2_2->setMRMLScene(this->mrmlScene());

  // Node selectors
  vtkMRMLVolumeNode* inputVolumeNode =
    workspaceGenerationNode->GetInputVolumeNode();

  d->InputVolumeNodeSelector__2_2->setCurrentNode(inputVolumeNode);
  // d->InputVolumeNodeSelector__2_2->blockSignals(false);

  if (!inputVolumeNode)
  {
    qWarning() << Q_FUNC_INFO << ": No input volume node selected.";
    // d->ROINodeSelector__2_4->setDisabled(true);
  }

  // d->WorkspaceModelSelector__3_2->setEnabled(true);
  // d->WorkspaceModelSelector__3_2->blockSignals(true);
  d->WorkspaceModelSelector__3_2->setMRMLScene(this->mrmlScene());

  vtkMRMLModelNode* workspaceMeshModelNode =
    workspaceGenerationNode->GetWorkspaceMeshModelNode();

  d->WorkspaceModelSelector__3_2->setCurrentNode(workspaceMeshModelNode);
  // d->WorkspaceModelSelector__3_2->blockSignals(false);

  if (!workspaceMeshModelNode)
  {
    setCheckState(d->WorkspaceVisibilityToggle__3_12, false);
    qWarning() << Q_FUNC_INFO << ": No Workspace Mesh Node available.";
    d->WorkspaceMeshModelDisplayNode = NULL;
    setCheckState(d->WorkspaceVisibilityToggle__3_12, false);
  }
  else
  {
    d->WorkspaceMeshModelNode = workspaceMeshModelNode;

    // Workspace Generation display options
    d->WorkspaceMeshModelDisplayNode =
      workspaceMeshModelNode->GetModelDisplayNode();

    if (d->WorkspaceMeshModelDisplayNode != NULL)
    {
      auto visibility = d->WorkspaceMeshModelDisplayNode->GetVisibility();
      setCheckState(d->WorkspaceVisibilityToggle__3_12, visibility);
    }
    else
    {
      setCheckState(d->WorkspaceVisibilityToggle__3_12, false);
    }
  }

  // d->ROINodeSelector__2_4->setEnabled(true);
  // d->ROINodeSelector__2_4->blockSignals(true);
  d->ROINodeSelector__2_4->setMRMLScene(this->mrmlScene());

  vtkMRMLAnnotationROINode* annotationROINode =
    workspaceGenerationNode->GetAnnotationROINode();

  d->ROINodeSelector__2_4->setCurrentNode(annotationROINode);
  // d->ROINodeSelector__2_4->blockSignals(false);

  if (!annotationROINode)
  {
    qWarning() << Q_FUNC_INFO << ": No Annotation ROI Node was selected.";
    // return;
  }

  // d->EntryPointFiducialSelector__4_2->setEnabled(true);
  // d->EntryPointFiducialSelector__4_2->blockSignals(true);
  d->EntryPointFiducialSelector__4_2->setMRMLScene(this->mrmlScene());

  vtkMRMLMarkupsFiducialNode* entryPoint =
    workspaceGenerationNode->GetEntryPointNode();

  d->EntryPointFiducialSelector__4_2->setCurrentNode(entryPoint);
  if (entryPoint != NULL)
  {
    d->EntryPointMarkupsPlaceWidget__4_3->setCurrentNode(entryPoint);
  }
  else
  {
    qWarning() << Q_FUNC_INFO << ": Entry point is NULL";
    d->EntryPointMarkupsPlaceWidget__4_3->setCurrentNode(NULL);
  }
  // d->EntryPointFiducialSelector__4_2->blockSignals(false);

  // d->TargetPointFiducialSelector__4_4->setEnabled(true);
  // d->TargetPointFiducialSelector__4_4->blockSignals(true);
  d->TargetPointFiducialSelector__4_4->setMRMLScene(this->mrmlScene());
  vtkMRMLMarkupsFiducialNode* targetPoint =
    workspaceGenerationNode->GetTargetPointNode();
  d->TargetPointFiducialSelector__4_4->setCurrentNode(targetPoint);
  if (targetPoint != NULL)
  {
    d->TargetPointMarkupsPlaceWidget__4_5->setCurrentNode(targetPoint);
  }
  else
  {
    qWarning() << Q_FUNC_INFO << ": Target point is NULL";
    d->TargetPointMarkupsPlaceWidget__4_5->setCurrentNode(NULL);
  }
  // d->TargetPointFiducialSelector__4_4->blockSignals(false);

  // block ALL signals until the function returns
  // if a return is called after this line, then unblockAllSignals should also
  // be called.
  this->blockAllSignals(true);

  if (annotationROINode != NULL)
  {
    // Volume Rendering display options
    vtkMRMLVolumeRenderingDisplayNode* volRenderingDispNode =
      d->logic()->getCurrentInputVolumeRenderingDisplayNode();
    d->InputVolumeRenderingDisplayNode = volRenderingDispNode;

    if (d->InputVolumeRenderingDisplayNode != NULL)
    {
      // Get the current Volume Property Node.
      d->VolumePropertyNode =
        d->InputVolumeRenderingDisplayNode->GetVolumePropertyNode();

      // Copy the MRI preset to the volume property node
      d->VolumePropertyNode->Copy(
        this->VolumeRenderingLogic->GetPresetByName("MR-Default"));

      // Set the current mrml scene in the preset combo box widget
      // d->InputVolumeRenderingPresetComboBox->setMRMLScene(this->mrmlScene());

      // Have the preset combo box observe the vol rendering display property
      // node.
      // d->InputVolumeRenderingPresetComboBox->setMRMLVolumePropertyNode(
      //   d->VolumePropertyNode);

      // Set the current node to the preset combo box
      // d->InputVolumeRenderingPresetComboBox->setCurrentNode(
      //   this->VolumeRenderingLogic->GetPresetByName("MR-Default"));

      auto visibility = d->InputVolumeRenderingDisplayNode->GetVisibility();
      setCheckState(d->InputVolumeSetVisibilityCheckBox__2_3, visibility);
    }
  }
  else
  {
    setCheckState(d->InputVolumeSetVisibilityCheckBox__2_3, false);
  }

  d->EntryPointMarkupsPlaceWidget__4_3->setMRMLScene(this->mrmlScene());
  d->TargetPointMarkupsPlaceWidget__4_5->setMRMLScene(this->mrmlScene());

  // Determine visibility of widgets
  bool isEntryPoint =
    (vtkMRMLMarkupsFiducialNode::SafeDownCast(entryPoint) != NULL);
  bool isTargetPoint =
    (vtkMRMLMarkupsFiducialNode::SafeDownCast(targetPoint) != NULL);

  qDebug() << Q_FUNC_INFO << ": Entry Point"
           << ((isEntryPoint) ? "true" : "false");
  qDebug() << Q_FUNC_INFO << ": Target Point"
           << ((isTargetPoint) ? "true" : "false");

  d->EntryPointMarkupsPlaceWidget__4_3->setVisible(isEntryPoint);
  d->TargetPointMarkupsPlaceWidget__4_5->setVisible(isTargetPoint);

  this->blockAllSignals(false);
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::UpdateVolumeRendering()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector__1_1->currentNode());

  if (workspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO
                << ": Model node changed with no module node selection";
    return;
  }

  d->logic()->UpdateVolumeRendering();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::blockAllSignals(bool block)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  foreach (QWidget* w, allInteractiveWidgets)
  {
    w->blockSignals(block);
  }
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::enableAllWidgets(bool enable)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  foreach (QWidget* w, allInteractiveWidgets)
  {
    // qDebug() << "Enabling: " << w->objectName();
    w->setEnabled(enable);
  }
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::disableWidgetsAfter(
  QWidget* widgetStart, QWidget* widgetEnd, bool includingStart,
  bool includingEnd)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  bool enableRest = false;

  if (widgetStart == NULL)
  {
    return;
  }
  else
  {
    // // Lambdas
    // auto condition = [&](QString const mainWidgetName) -> bool {
    //   // If start widget matches any widget in the list of all interactive
    //   // widgets then return true, else return false
    //   return QString::compare(widgetStart->objectName(), mainWidgetName,
    //                           Qt::CaseInsensitive) == 0;
    // };

    // auto getCurrentIndex = [&]() -> int {
    //   // Get the index of the start widget from the list of interactive
    //   widgets
    //   // Return -1 if the widget is not found
    //   foreach (QWidget* w, allInteractiveWidgets)
    //   {
    //     if (condition(w->objectName()))
    //     {
    //       return allInteractiveWidgets.indexOf(w);
    //     }
    //   }

    //   return -1;
    // };

    // Set widgetEnd to last widget if widgetEnd is NULL
    if (widgetEnd == NULL)
    {
      widgetEnd = allInteractiveWidgets.last();
    }

    int currentIndex =
      allInteractiveWidgets.indexOf(widgetStart);  // getCurrentIndex();
    // Terminate if current index was not found.
    if (currentIndex == -1)
    {
      qCritical() << Q_FUNC_INFO << ": Widget is not available";
      return;
    }

    // Start index is currentIndex + 0 if includingStart is true
    // Start index is currentIndex + 1 if includingStart is false
    int startIndex = currentIndex + ( int ) (!includingStart);
    // End index is indexOf Last Widget + 1 if includingEnd is true
    // End index is indexOf Last Widget + 0 if includingEnd is false
    int endIndex =
      allInteractiveWidgets.indexOf(widgetEnd) + ( int ) (includingEnd);

    // if currentIndex is not 0 and does not include start and endIndex is not
    // last
    if ((currentIndex != 0 && includingStart != true) &&
        endIndex != allInteractiveWidgets.indexOf(allInteractiveWidgets.last()))
    {
      // If enable Rest of widgets
      if (enableRest)
      {
        // qDebug() << Q_FUNC_INFO << ": Enabling: {";
        for (int i = 0; i <= startIndex; i++)
        {
          QWidget* w = allInteractiveWidgets[i];
          w->setEnabled(true);
          // qDebug() << "\t\t" << w->objectName() << ",";
        }
        for (int i = endIndex; i < allInteractiveWidgets.length(); i++)
        {
          QWidget* w = allInteractiveWidgets[i];
          w->setEnabled(true);
          // qDebug() << "\t\t" << w->objectName() << ",";
        }
        // qDebug() << "}";
      }

      // qDebug() << Q_FUNC_INFO << ": Disabling: {";
      for (int i = startIndex; i < endIndex; i++)
      {
        QWidget* w = allInteractiveWidgets[i];
        w->setDisabled(true);
        // qDebug() << "\t\t" << w->objectName() << ",";
      }
      // qDebug() << "}";
    }
    // trying to disable all widgets
    else
    {
      // qDebug() << Q_FUNC_INFO << ": Disable all widgets";
      enableAllWidgets(false);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::disableWidgetsBetween(
  QWidget* start, QWidget* end, bool includeStart, bool includeEnd)
{
  disableWidgetsAfter(start, end, includeStart, includeEnd);
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::enableWidgets(QWidget* widget,
                                                           bool     enable)
{
  widget->setEnabled(enable);
}