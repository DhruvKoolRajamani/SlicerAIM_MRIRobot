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

  // Observed nodes (to keep GUI up-to-date)
  vtkMRMLWorkspaceGenerationNode* WorkspaceGenerationNode;

  // vtkMRMLVolumeNode* InputVolumeNode;
  // vtkMRMLAnnotationROINode* AnnotationROINode;
  vtkMRMLModelNode* WorkspaceMeshModelNode;

  vtkMRMLVolumeRenderingDisplayNode* InputVolumeRenderingDisplayNode;
  vtkMRMLModelDisplayNode* WorkspaceMeshModelDisplayNode;

  vtkMRMLVolumePropertyNode* VolumePropertyNode;

  vtkMatrix4x4* WorkspaceMeshRegistrationMatrix;
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

  qDebug() << allInteractiveWidgets;

  // Connect buttons in UI
  this->setMRMLScene(d->logic()->GetMRMLScene());
  this->VolumeRenderingModule = d->logic()->getVolumeRenderingModule();
  this->VolumeRenderingLogic = d->logic()->getVolumeRenderingLogic();

  connect(d->ParameterNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onParameterNodeSelectionChanged()));
  connect(d->InputVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onInputVolumeNodeSelectionChanged(vtkMRMLNode*)));
  connect(d->InputVolumeNodeSelector, SIGNAL(nodeAddedByUser(vtkMRMLNode*)),
          this, SLOT(onInputVolumeNodeAdded(vtkMRMLNode*)));
  connect(d->ROINodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
          SLOT(onAnnotationROISelectionChanged(vtkMRMLNode*)));
  connect(d->ROINodeSelector, SIGNAL(nodeAddedByUser(vtkMRMLNode*)), this,
          SLOT(onAnnotationROINodeAdded(vtkMRMLNode*)));
  connect(d->InputVolumeSetVisibilityCheckBox, SIGNAL(toggled(bool)), this,
          SLOT(onInputVolumeVisibilityChanged(bool)));
  connect(d->InputVolumeRenderingPresetComboBox,
          SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
          SLOT(onPresetComboBoxNodeChanged(vtkMRMLNode*)));
  connect(d->InputVolumeRenderingPresetComboBox, SIGNAL(presetOffsetChanged()),
          this, SLOT(onPresetComboBoxNodeChanged(vtkMRMLNode*)));
  connect(d->WorkspaceMeshLoadBtn, SIGNAL(released()), this,
          SLOT(onWorkspaceLoadButtonClick()));
  connect(d->WorkspaceMeshModelNodeSelector,
          SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
          SLOT(onWorkspaceMeshModelNodeChanged(vtkMRMLNode*)));
  connect(d->WorkspaceMeshModelNodeSelector,
          SIGNAL(nodeAddedByUser(vtkMRMLNode*)), this,
          SLOT(onWorkspaceMeshModelNodeAdded(vtkMRMLNode*)));
  connect(d->WorkspaceMeshSetVisibilityCheckBox, SIGNAL(toggled(bool)), this,
          SLOT(onWorkspaceMeshVisibilityChanged(bool)));
  connect(d->ApplyTransformButton, SIGNAL(released()), this,
          SLOT(onApplyTransformClick()));
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
  if (d->ParameterNodeSelector->currentNode() == NULL)
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

    d->ParameterNodeSelector->setMRMLScene(this->mrmlScene());
    d->ParameterNodeSelector->setCurrentNode(node);
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
      d->ParameterNodeSelector->currentNode());

  qvtkReconnect(d->WorkspaceGenerationNode, selectedWorkspaceGenerationNode,
                vtkCommand::ModifiedEvent, this, SLOT(updateGUIFromMRML()));

  d->WorkspaceGenerationNode = selectedWorkspaceGenerationNode;
  d->logic()->UpdateSelectionNode(selectedWorkspaceGenerationNode);

  setCheckState(d->InputVolumeSetVisibilityCheckBox, false);
  setCheckState(d->WorkspaceMeshSetVisibilityCheckBox, false);

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onInputVolumeNodeSelectionChanged(
  vtkMRMLNode* nodeSelected)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector->currentNode());

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

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onInputVolumeNodeAdded(
  vtkMRMLNode* addedNode)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onAnnotationROISelectionChanged(
  vtkMRMLNode* selectedNode)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector->currentNode());

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

// --------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onInputVolumeVisibilityChanged(
  bool visible)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* selectedWorkspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector->currentNode());

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
  d->InputVolumeRenderingPresetComboBox->setEnabled(visible);

  // Update widget from display node of the volume node
  this->updateGUIFromMRML();
}

// bug: #13 fix volume rendering preset combo box widget @DhruvKoolRajamani
// --------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onPresetComboBoxNodeChanged(
  vtkMRMLNode* selectedNode)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector->currentNode());

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

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onWorkspaceMeshModelNodeChanged(
  vtkMRMLNode* nodeSelected)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector->currentNode());

  if (workspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": invalid workspaceGenerationNode";

    workspaceGenerationNode->SetAndObserveWorkspaceMeshModelNodeID(NULL);
    d->WorkspaceMeshMRMLMatrixWidget->setDisabled(true);
    d->WorkspaceMeshModelDisplayNode = NULL;

    return;
  }

  if (nodeSelected == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": unexpected workspace mesh model node type";

    workspaceGenerationNode->SetAndObserveWorkspaceMeshModelNodeID(NULL);
    d->WorkspaceMeshMRMLMatrixWidget->setDisabled(true);
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
    d->WorkspaceMeshMRMLMatrixWidget->setDisabled(true);
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

  d->WorkspaceMeshMRMLMatrixWidget->setEnabled(true);
  d->WorkspaceMeshMRMLMatrixWidget->setEditable(true);

  // Create logic to accommodate creating a new annotation ROI node.
  // Should you transfer the data to the new node? Reset all visibility params?

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onWorkspaceMeshModelNodeAdded(
  vtkMRMLNode* nodeAdded)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onWorkspaceLoadButtonClick()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;

  auto fileName = QFileDialog::getOpenFileName(this, tr("Open Workspace Mesh"),
                                               QDir::currentPath(),
                                               tr("Polymesh File (*.ply)"));

  qDebug() << Q_FUNC_INFO << ": Workspace path is " << fileName;

  if (fileName.isEmpty())
  {
    // Return if no path is specified
    qCritical() << Q_FUNC_INFO << ": No filepath specified";
    return;
  }

  if (d->logic()->LoadWorkspace(fileName))
  {
    d->WorkspaceMeshModelNode = d->logic()->getWorkspaceMeshModelNode();
    d->WorkspaceMeshModelNodeSelector->setCurrentNode(
      d->WorkspaceGenerationNode);
  }
}

// --------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onWorkspaceMeshVisibilityChanged(
  bool visible)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* selectedWorkspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector->currentNode());

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

// --------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onApplyTransformClick()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector->currentNode());

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
    d->WorkspaceMeshMRMLMatrixWidget->values().data());
  qDebug() << Q_FUNC_INFO << *(d->WorkspaceMeshRegistrationMatrix->GetData());

  workspaceMeshModelNode->ApplyTransformMatrix(
    d->WorkspaceMeshRegistrationMatrix);

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
vtkMRMLAnnotationROINode*
  qSlicerWorkspaceGenerationModuleWidget::GetAnnotationROINode()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector->currentNode());

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
      d->ParameterNodeSelector->currentNode());

  if (workspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": Selected node not a valid module node";
    return NULL;
  }

  return workspaceGenerationNode->GetInputVolumeNode();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::setCheckState(ctkPushButton* btn,
                                                           bool state)
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
      d->ParameterNodeSelector->currentNode());

  d->WorkspaceGenerationNode = workspaceGenerationNode;

  if (!d->WorkspaceGenerationNode)
  {
    qCritical() << Q_FUNC_INFO << ": Selected node not a valid module node";
    this->enableAllWidgets(false);
    return;
  }
  d->logic()->setWorkspaceGenerationNode(workspaceGenerationNode);

  this->enableAllWidgets(true);

  d->InputVolumeNodeSelector->blockSignals(true);
  // Set mrml scene in input volume node selector
  d->InputVolumeNodeSelector->setMRMLScene(this->mrmlScene());

  // Node selectors
  vtkMRMLVolumeNode* inputVolumeNode =
    workspaceGenerationNode->GetInputVolumeNode();

  if (!inputVolumeNode)
  {
    qCritical() << Q_FUNC_INFO << ": No input volume node selected.";
    // d->ROINodeSelector->setDisabled(true);
  }

  d->InputVolumeNodeSelector->setCurrentNode(inputVolumeNode);
  d->InputVolumeNodeSelector->blockSignals(false);

  d->WorkspaceMeshModelNodeSelector->setEnabled(true);
  d->WorkspaceMeshModelNodeSelector->blockSignals(true);
  d->WorkspaceMeshModelNodeSelector->setMRMLScene(this->mrmlScene());

  vtkMRMLModelNode* workspaceMeshModelNode =
    workspaceGenerationNode->GetWorkspaceMeshModelNode();

  d->WorkspaceMeshModelNodeSelector->setCurrentNode(workspaceMeshModelNode);
  d->WorkspaceMeshModelNodeSelector->blockSignals(false);

  if (!workspaceMeshModelNode)
  {
    setCheckState(d->WorkspaceMeshSetVisibilityCheckBox, false);
    qCritical() << Q_FUNC_INFO << ": No Workspace Mesh Node available.";
    d->WorkspaceMeshModelDisplayNode = NULL;
  }
  else
  {
    qDebug() << Q_FUNC_INFO << ": Workspace Mesh Model Node available.";
    d->WorkspaceMeshModelNode = workspaceMeshModelNode;

    // Workspace Generation display options
    d->WorkspaceMeshModelDisplayNode =
      workspaceMeshModelNode->GetModelDisplayNode();

    if (d->WorkspaceMeshModelDisplayNode != NULL)
    {
      auto visibility = d->WorkspaceMeshModelDisplayNode->GetVisibility();
      setCheckState(d->WorkspaceMeshSetVisibilityCheckBox, visibility);
      // d->ModelOpacitySlider->setValue(modelDisplayNode->GetOpacity());

      // Should be a color picker instead of InputVolumeRenderingDisplayNode
      // double* outputColor = d->WorkspaceMeshModelDisplayNode->GetColor();
      // QColor nodeOutputColor;
      // nodeOutputColor.setRgbF(outputColor[0], outputColor[1],
      // outputColor[2]); d->WorkspaceMeshModelDisplayNode->SetColor(
      //   nodeOutputColor.red(), nodeOutputColor.green(),
      //   nodeOutputColor.blue());
    }
    else
    {
      setCheckState(d->WorkspaceMeshSetVisibilityCheckBox, false);
    }
  }

  d->ROINodeSelector->setEnabled(true);
  d->ROINodeSelector->blockSignals(true);
  d->ROINodeSelector->setMRMLScene(this->mrmlScene());

  vtkMRMLAnnotationROINode* annotationROINode =
    workspaceGenerationNode->GetAnnotationROINode();

  if (!annotationROINode)
  {
    qCritical() << Q_FUNC_INFO << ": No Annotation ROI Node was selected.";
    return;
  }

  d->ROINodeSelector->setCurrentNode(annotationROINode);
  d->ROINodeSelector->blockSignals(false);

  // block ALL signals until the function returns
  // if a return is called after this line, then unblockAllSignals should also
  // be called.
  this->blockAllSignals(true);

  // Volume Rendering display options
  vtkMRMLVolumeRenderingDisplayNode* volRenderingDispNode =
    d->logic()->getCurrentInputVolumeRenderingDisplayNode();
  d->InputVolumeRenderingDisplayNode = volRenderingDispNode;

  if (d->InputVolumeRenderingDisplayNode != NULL)
  {
    // Get the current Volume Property Node.
    d->VolumePropertyNode =
      d->InputVolumeRenderingDisplayNode->GetVolumePropertyNode();

    // // Copy the MRI preset to the volume property node
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
    setCheckState(d->InputVolumeSetVisibilityCheckBox, visibility);
  }
  else
  {
    setCheckState(d->InputVolumeSetVisibilityCheckBox, false);
  }

  this->blockAllSignals(false);
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::UpdateVolumeRendering()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector->currentNode());

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
        qDebug() << Q_FUNC_INFO << ": Enabling: {";
        for (int i = 0; i <= startIndex; i++)
        {
          QWidget* w = allInteractiveWidgets[i];
          w->setEnabled(true);
          qDebug() << "\t\t" << w->objectName() << ",";
        }
        for (int i = endIndex; i < allInteractiveWidgets.length(); i++)
        {
          QWidget* w = allInteractiveWidgets[i];
          w->setEnabled(true);
          qDebug() << "\t\t" << w->objectName() << ",";
        }
        qDebug() << "}";
      }

      qDebug() << Q_FUNC_INFO << ": Disabling: {";
      for (int i = startIndex; i < endIndex; i++)
      {
        QWidget* w = allInteractiveWidgets[i];
        w->setDisabled(true);
        qDebug() << "\t\t" << w->objectName() << ",";
      }
      qDebug() << "}";
    }
    // trying to disable all widgets
    else
    {
      qDebug() << Q_FUNC_INFO << ": Disable all widgets";
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
                                                           bool enable)
{
  widget->setEnabled(enable);
}