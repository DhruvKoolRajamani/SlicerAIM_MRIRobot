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
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLVolumeNode.h"
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
  connect(d->WorkspaceMeshLoadBtn, SIGNAL(released()), this,
          SLOT(onWorkspaceLoadButtonClick()));
  connect(d->WorkspaceMeshSetVisibilityCheckBox, SIGNAL(toggled(bool)), this,
          SLOT(onWorkspaceMeshVisibilityChanged(bool)));
  connect(d->WorkspaceMeshPushButton, SIGNAL(released()), this,
          SLOT(onApplyTransformClick()));
}

// --------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onApplyTransformClick()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
}

// --------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onWorkspaceMeshVisibilityChanged(
  bool visible)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

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
void qSlicerWorkspaceGenerationModuleWidget::onInputVolumeVisibilityChanged(
  bool visible)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  // Get volume rendering display node for volume. Create if absent.
  if (!d->InputVolumeRenderingDisplayNode)
  {
    qCritical() << Q_FUNC_INFO << ": No volume rendering display node";
    return;
  }

  d->InputVolumeRenderingDisplayNode->SetVisibility(visible);

  // Update widget from display node of the volume node
  this->updateGUIFromMRML();
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

  btn->setChecked(state);
  auto color = ((state) ? "background-color: green" : "background-color: red");
  btn->setStyleSheet(color);
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

    qDebug() << Q_FUNC_INFO << ": Standard call complete";
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

  d->logic()->LoadWorkspace(fileName);
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
void qSlicerWorkspaceGenerationModuleWidget::updateGUIFromMRML()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qDebug() << Q_FUNC_INFO;

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

  // Set mrml scene in input volume node selector
  d->InputVolumeNodeSelector->setMRMLScene(this->mrmlScene());

  // Node selectors
  vtkMRMLVolumeNode* inputVolumeNode =
    workspaceGenerationNode->GetInputVolumeNode();

  if (!inputVolumeNode)
  {
    qCritical() << Q_FUNC_INFO << ": No input volume node selected.";
    // return;
  }

  d->InputVolumeNodeSelector->setCurrentNode(inputVolumeNode);

  d->ROINodeSelector->setMRMLScene(this->mrmlScene());

  vtkMRMLAnnotationROINode* annotationROINode =
    workspaceGenerationNode->GetAnnotationROINode();

  if (!annotationROINode)
  {
    qCritical() << Q_FUNC_INFO << ": No Annotation ROI Node was selected.";
    return;
  }

  d->ROINodeSelector->setCurrentNode(annotationROINode);

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
    auto visibility = d->InputVolumeRenderingDisplayNode->GetVisibility();
    setCheckState(d->InputVolumeSetVisibilityCheckBox, visibility);

    // d->ModelOpacitySlider->setValue(modelDisplayNode->GetOpacity());

    // Should be a color picker instead of InputVolumeRenderingDisplayNode
    double* outputColor = d->InputVolumeRenderingDisplayNode->GetColor();
    QColor nodeOutputColor;
    nodeOutputColor.setRgbF(outputColor[0], outputColor[1], outputColor[2]);
    d->InputVolumeRenderingDisplayNode->SetColor(
      nodeOutputColor.red(), nodeOutputColor.green(), nodeOutputColor.blue());
  }
  else
  {
    setCheckState(d->InputVolumeSetVisibilityCheckBox, false);
    QColor nodeOutputColor;
    nodeOutputColor.setRgbF(0, 0, 0);
    d->InputVolumeRenderingDisplayNode->SetColor(
      nodeOutputColor.red(), nodeOutputColor.green(), nodeOutputColor.blue());
  }

  vtkMRMLModelNode* workspaceMeshModelNode =
    d->logic()->getWorkspaceMeshModelNode();

  if (workspaceMeshModelNode != NULL)
  {
    qDebug() << Q_FUNC_INFO << ": Workspace Mesh Model Node available.";
    d->WorkspaceMeshModelNode = workspaceMeshModelNode;

    // Workspace Generation display options
    vtkMRMLModelDisplayNode* modelDispNode =
      d->logic()->getCurrentWorkspaceMeshModelDisplayNode();
    d->WorkspaceMeshModelDisplayNode = modelDispNode;

    if (d->WorkspaceMeshModelDisplayNode != NULL)
    {
      auto visibility = d->WorkspaceMeshModelDisplayNode->GetVisibility();
      setCheckState(d->WorkspaceMeshSetVisibilityCheckBox, visibility);
      // d->ModelOpacitySlider->setValue(modelDisplayNode->GetOpacity());

      // Should be a color picker instead of InputVolumeRenderingDisplayNode
      double* outputColor = d->WorkspaceMeshModelDisplayNode->GetColor();
      QColor nodeOutputColor;
      nodeOutputColor.setRgbF(outputColor[0], outputColor[1], outputColor[2]);
      d->WorkspaceMeshModelDisplayNode->SetColor(
        nodeOutputColor.red(), nodeOutputColor.green(), nodeOutputColor.blue());
    }
    else
    {
      setCheckState(d->WorkspaceMeshSetVisibilityCheckBox, false);
      QColor nodeOutputColor;
      nodeOutputColor.setRgbF(0, 0, 0);
      d->WorkspaceMeshModelDisplayNode->SetColor(
        nodeOutputColor.red(), nodeOutputColor.green(), nodeOutputColor.blue());
    }
  }
  else
  {
    setCheckState(d->WorkspaceMeshSetVisibilityCheckBox, false);
    qCritical() << Q_FUNC_INFO << ": No Workspace Mesh Node available.";
  }

  this->blockAllSignals(false);
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::blockAllSignals(bool block)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  d->ParameterNodeSelector->blockSignals(block);
  d->InputVolumeNodeSelector->blockSignals(block);
  d->ROINodeSelector->blockSignals(block);
  d->WorkspaceMeshLoadBtn->blockSignals(block);
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::enableAllWidgets(bool enable)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;
  d->ParameterNodeSelector->setEnabled(enable);
  d->InputVolumeNodeSelector->setEnabled(enable);
  d->ROINodeSelector->setEnabled(enable);
  d->WorkspaceMeshLoadBtn->setEnabled(enable);
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::disableWidgetsAfter(
  QWidget* widget)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;

  bool enable = true;

  if (widget == NULL)
  {
    return;
  }
  else
  {
    if (QString::compare(widget->objectName(),
                         d->ParameterNodeSelector->objectName(),
                         Qt::CaseInsensitive))
    {
      d->ParameterNodeSelector->setEnabled(enable);
      d->InputVolumeNodeSelector->setEnabled(!enable);
      d->ROINodeSelector->setEnabled(!enable);
      d->WorkspaceMeshLoadBtn->setEnabled(!enable);
    }

    if (QString::compare(widget->objectName(),
                         d->InputVolumeNodeSelector->objectName(),
                         Qt::CaseInsensitive))
    {
      d->ParameterNodeSelector->setEnabled(enable);
      d->InputVolumeNodeSelector->setEnabled(enable);
      d->ROINodeSelector->setEnabled(!enable);
      d->WorkspaceMeshLoadBtn->setEnabled(!enable);
    }

    if (QString::compare(widget->objectName(), d->ROINodeSelector->objectName(),
                         Qt::CaseInsensitive))
    {
      d->ParameterNodeSelector->setEnabled(enable);
      d->InputVolumeNodeSelector->setEnabled(enable);
      d->ROINodeSelector->setEnabled(enable);
      d->WorkspaceMeshLoadBtn->setEnabled(!enable);
    }

    if (QString::compare(widget->objectName(),
                         d->WorkspaceMeshLoadBtn->objectName(),
                         Qt::CaseInsensitive))
    {
      d->ParameterNodeSelector->setEnabled(enable);
      d->InputVolumeNodeSelector->setEnabled(enable);
      d->ROINodeSelector->setEnabled(enable);
      d->WorkspaceMeshLoadBtn->setEnabled(enable);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::enableWidgets(QWidget* widget,
                                                           bool enable)
{
  widget->setEnabled(enable);
}
