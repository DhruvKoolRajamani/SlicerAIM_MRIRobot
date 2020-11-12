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
  vtkWeakPointer< vtkMRMLWorkspaceGenerationNode > WorkspaceGenerationNode;
  vtkWeakPointer< vtkMRMLModelDisplayNode > InputModelDisplayNode;
  vtkWeakPointer< vtkMRMLModelDisplayNode > OutputModelDisplayNode;
  vtkWeakPointer< vtkMRMLVolumeDisplayNode > InputVolumeDisplayNode;
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

  connect(d->WorkspaceMeshLoadBtn, SIGNAL(released()), this,
          SLOT(onWorkspaceLoadButtonClick()));
  connect(d->ParameterNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onParameterNodeSelectionChanged()));
  connect(d->InputNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this,
          SLOT(onInputNodeSelectionChanged(vtkMRMLNode*)));
  connect(d->InputNodeSelector, SIGNAL(nodeAddedByUser(vtkMRMLNode*)), this,
          SLOT(onInputNodeNodeAdded(vtkMRMLNode*)));
  connect(d->OutputModelNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onOutputModelSelectionChanged(vtkMRMLNode*)));
  connect(d->OutputModelNodeSelector, SIGNAL(nodeAddedByUser(vtkMRMLNode*)),
          this, SLOT(onOutputModelNodeAdded(vtkMRMLNode*)));
  connect(d->OutputModelSetVisibilityCheckBox, SIGNAL(toggled(bool)), this,
          SLOT(onOutputModelVisibilityChanged(bool)));
  connect(d->WorkspaceMeshSetVisibilityCheckBox, SIGNAL(toggled(bool)), this,
          SLOT(onWorkspaceMeshVisibilityChanged(bool)));
  connect(d->WorkspaceMeshPushButton, SIGNAL(released()), this,
          SLOT(onApplyTransformClick()));

  qDebug() << Q_FUNC_INFO << "OutputModelSelector is "
           << (d->OutputModelNodeSelector->isEnabled() ? "Enabled" :
                                                         "Disabled");

  // d->InputVolumeRenderingPresetComboBox->
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
}

// --------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onOutputModelVisibilityChanged(
  bool visible)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  // Get volume rendering display node for volume. Create if absent.
  if (!d->InputVolumeDisplayNode)
  {
    qCritical() << Q_FUNC_INFO << ": No volume rendering display node";
    return;
  }

  d->InputVolumeDisplayNode->SetVisibility(visible);

  // Update widget from display node of the volume node
  this->updateGUIFromMRML();
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
  d->logic()->UpdateSelectionNode(d->WorkspaceGenerationNode);
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onInputNodeSelectionChanged(
  vtkMRMLNode* newNode)
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

  if (newNode == NULL)
  {
    workspaceGenerationNode->SetAndObserveInputNodeID(NULL);
    qCritical() << Q_FUNC_INFO << ": New node is NONE";

    return;
  }

  vtkMRMLVolumeNode* inputVolumeNode = vtkMRMLVolumeNode::SafeDownCast(newNode);
  vtkMRMLModelNode* inputModelNode = vtkMRMLModelNode::SafeDownCast(newNode);

  qDebug() << Q_FUNC_INFO << ": Input is a - "
           << ((inputVolumeNode != NULL) ? "Volume Node" : "Model Node");

  if (inputVolumeNode != NULL || inputModelNode != NULL)
  {
    if (inputVolumeNode != NULL)
    {
      qInfo() << Q_FUNC_INFO << ": Input Volume Node selected.";

      workspaceGenerationNode->SetAndObserveInputNodeID(
        inputVolumeNode->GetID(), vtkMRMLWorkspaceGenerationNode::VOLUME_NODE);

      // Observe display node so that we can make sure the module GUI always
      // shows up-to-date information (applies specifically to markups)
      vtkMRMLVolumeDisplayNode* inputVolumeDisplayNode =
        vtkMRMLVolumeDisplayNode::SafeDownCast(
          inputVolumeNode->GetDisplayNode());

      if (inputVolumeDisplayNode == NULL)
      {
        inputVolumeNode->CreateDefaultDisplayNodes();
        inputVolumeDisplayNode = vtkMRMLVolumeDisplayNode::SafeDownCast(
          inputVolumeNode->GetDisplayNode());
      }

      qvtkReconnect(d->InputVolumeDisplayNode, inputVolumeDisplayNode,
                    vtkCommand::ModifiedEvent, this, SLOT(updateGUIFromMRML()));

      d->InputVolumeDisplayNode = inputVolumeDisplayNode;
    }
    else if (inputModelNode != NULL)
    {
      qInfo() << Q_FUNC_INFO << ": Input Model Node selected.";

      workspaceGenerationNode->SetAndObserveInputNodeID(
        inputModelNode->GetID(), vtkMRMLWorkspaceGenerationNode::MODEL_NODE);

      // Observe display node so that we can make sure the module GUI always
      // shows up-to-date information (applies specifically to markups)
      vtkMRMLModelDisplayNode* inputModelDisplayNode =
        vtkMRMLModelDisplayNode::SafeDownCast(inputModelNode->GetDisplayNode());

      if (inputModelDisplayNode == NULL)
      {
        inputModelNode->CreateDefaultDisplayNodes();
        inputModelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(
          inputModelNode->GetDisplayNode());
      }

      qvtkReconnect(d->InputModelDisplayNode, inputModelDisplayNode,
                    vtkCommand::ModifiedEvent, this, SLOT(updateGUIFromMRML()));

      d->InputModelDisplayNode = inputModelDisplayNode;
    }

    this->disableWidgetsAfter(d->OutputModelNodeSelector);
  }
  else
  {
    workspaceGenerationNode->SetAndObserveInputNodeID(NULL);
    qCritical() << Q_FUNC_INFO << ": unexpected input node type";

    return;
  }

  this->disableWidgetsAfter(d->InputNodeSelector);

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onInputNodeNodeAdded(
  vtkMRMLNode* addedNode)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onOutputModelSelectionChanged(
  vtkMRMLNode* newNode)
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

  vtkMRMLNode* inputNode = workspaceGenerationNode->GetInputNode();
  if (!inputNode)
  {
    qCritical() << Q_FUNC_INFO << ": input Node has not been added yet.";
    // this->disableWidgetsAfter(d->InputNodeSelector);
    return;
  }

  vtkMRMLModelNode* outputModelNode = vtkMRMLModelNode::SafeDownCast(newNode);

  // Observe display node so that we can make sure the module GUI always shows
  // up-to-date information
  vtkMRMLModelDisplayNode* outputModelDisplayNode = NULL;
  // outputModelNode->GetModelDisplayNode();
  if (outputModelNode != NULL)  // && outputModelDisplayNode == NULL)
  {
    qDebug() << Q_FUNC_INFO << ": Output Model already exists!";

    outputModelDisplayNode = outputModelNode->GetModelDisplayNode();

    if (outputModelDisplayNode == NULL)
    {
      qWarning() << Q_FUNC_INFO
                 << ": Output Model Display Node Does Not exist!";
      outputModelNode->CreateDefaultDisplayNodes();
      outputModelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(
        outputModelNode->GetDisplayNode());
    }
  }

  qvtkReconnect(d->OutputModelDisplayNode, outputModelDisplayNode,
                vtkCommand::ModifiedEvent, this, SLOT(updateGUIFromMRML()));

  d->OutputModelDisplayNode = outputModelDisplayNode;

  this->updateGUIFromMRML();

  // workspaceGenerationNode->SetAndObserveOutputModelNodeID(
  //   outputModelNode ? outputModelNode->GetID() : NULL);

  // // Observe display node so that we can make sure the module GUI always
  // shows
  // // up-to-date information
  // vtkMRMLDisplayNode* outputDisplayNode = NULL;
  // // outputModelNode->GetModelDisplayNode();
  // if (outputModelNode != NULL)  // && outputModelDisplayNode == NULL)
  // {
  //   qDebug() << Q_FUNC_INFO << ": Output Model already exists!";

  //   outputDisplayNode = outputModelNode->GetDisplayNode();

  //   if (outputDisplayNode == NULL)
  //   {
  //     qWarning() << Q_FUNC_INFO
  //                << ": Output Model Display Node Does Not exist!";
  //     // outputModelNode->CreateDefaultDisplayNodes();
  //     outputDisplayNode =
  //       vtkMRMLDisplayNode::SafeDownCast(outputModelNode->GetDisplayNode());
  //   }
  // }

  // qvtkReconnect(d->OutputModelDisplayNode, outputDisplayNode,
  //               vtkCommand::ModifiedEvent, this, SLOT(updateGUIFromMRML()));

  // d->OutputModelDisplayNode = outputDisplayNode;

  // this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onOutputModelNodeAdded(
  vtkMRMLNode* addedNode)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;

  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(addedNode);
  if (modelNode == NULL)
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

  workspaceMeshFilePath = fileName;
  qDebug() << Q_FUNC_INFO << ": Workspace path is " << workspaceMeshFilePath;

  if (workspaceMeshFilePath.isEmpty())
  {
    // Return if no path is specified
    qCritical() << Q_FUNC_INFO << ": No filepath specified";
    return;
  }

  d->logic()->LoadWorkspace(workspaceMeshFilePath);
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
void qSlicerWorkspaceGenerationModuleWidget::UpdateOutputModel()
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

  d->logic()->UpdateOutputModel(workspaceGenerationNode);
}

//-----------------------------------------------------------------------------
vtkMRMLModelNode* qSlicerWorkspaceGenerationModuleWidget::GetOutputModelNode()
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
  return workspaceGenerationNode->GetOutputModelNode();
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qSlicerWorkspaceGenerationModuleWidget::GetInputNode()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;
  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector->currentNode());
  if (workspaceGenerationNode == NULL)
  {
    qCritical("Selected node not a valid module node");
    return NULL;
  }
  return workspaceGenerationNode->GetInputNode();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::updateGUIFromMRML()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qDebug() << Q_FUNC_INFO << ": Enter";

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector->currentNode());

  qDebug() << Q_FUNC_INFO << ": WorkspaceGenerationNode Check";
  if (workspaceGenerationNode == NULL)
  {
    qCritical("Selected node not a valid module node");
    this->enableAllWidgets(false);
    return;
  }

  this->enableAllWidgets(true);  // unless otherwise specified, everything is
                                 // enabled

  d->InputNodeSelector->setMRMLScene(this->mrmlScene());
  // Node selectors
  vtkMRMLNode* inputNode = workspaceGenerationNode->GetInputNode();

  qDebug() << Q_FUNC_INFO << ": Set InputNodeSelector";
  d->InputNodeSelector->setCurrentNode(inputNode);

  d->OutputModelNodeSelector->setMRMLScene(this->mrmlScene());
  qDebug() << Q_FUNC_INFO << ": Set OutputNodeSelector";
  d->OutputModelNodeSelector->setCurrentNode(
    workspaceGenerationNode->GetOutputModelNode());

  // block ALL signals until the function returns
  // if a return is called after this line, then unblockAllSignals should also
  // be called.
  this->blockAllSignals(true);

  // Model display options
  vtkMRMLModelDisplayNode* modelDisplayNode =
    vtkMRMLModelDisplayNode::SafeDownCast(
      this->GetOutputModelNode() ?
        this->GetOutputModelNode()->GetDisplayNode() :
        NULL);

  if (modelDisplayNode != NULL)
  {
    // d->ModelVisiblityButton->setChecked(modelDisplayNode->GetVisibility());
    // d->ModelOpacitySlider->setValue(modelDisplayNode->GetOpacity());
    double* outputColor = modelDisplayNode->GetColor();
    QColor nodeOutputColor;
    nodeOutputColor.setRgbF(outputColor[0], outputColor[1], outputColor[2]);
    // d->ModelColorSelector->setColor(nodeOutputColor);
    // d->ModelSliceIntersectionCheckbox->setChecked(
    //   modelDisplayNode->GetSliceIntersectionVisibility());
  }
  else
  {
    // d->ModelVisiblityButton->setChecked(false);
    // d->ModelOpacitySlider->setValue(1.0);
    // QColor nodeOutputColor;
    // nodeOutputColor.setRgbF(0, 0, 0);
    // d->ModelColorSelector->setColor(nodeOutputColor);
    // d->ModelSliceIntersectionCheckbox->setChecked(false);
  }

  this->blockAllSignals(false);
}

// //
// ----------------------------------------------------------------------------
// void qSlicerVolumeRenderingModuleWidget::updateWidgetFromMRML()
// {
//   Q_D(qSlicerVolumeRenderingModuleWidget);

//   // Get display node
//   vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();

//   // Get first view node
//   vtkMRMLViewNode* firstViewNode = nullptr;
//   if (displayNode && displayNode->GetScene())
//   {
//     firstViewNode = displayNode->GetFirstViewNode();
//   }
// }

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::blockAllSignals(bool block)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  d->ParameterNodeSelector->blockSignals(block);
  d->InputNodeSelector->blockSignals(block);
  d->OutputModelNodeSelector->blockSignals(block);
  d->WorkspaceMeshLoadBtn->blockSignals(block);
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::enableAllWidgets(bool enable)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  qInfo() << Q_FUNC_INFO;
  d->ParameterNodeSelector->setEnabled(enable);
  d->InputNodeSelector->setEnabled(enable);
  d->OutputModelNodeSelector->setEnabled(enable);
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
      d->InputNodeSelector->setEnabled(!enable);
      d->OutputModelNodeSelector->setEnabled(!enable);
      d->WorkspaceMeshLoadBtn->setEnabled(!enable);
    }

    if (QString::compare(widget->objectName(),
                         d->InputNodeSelector->objectName(),
                         Qt::CaseInsensitive))
    {
      d->ParameterNodeSelector->setEnabled(enable);
      d->InputNodeSelector->setEnabled(enable);
      d->OutputModelNodeSelector->setEnabled(!enable);
      d->WorkspaceMeshLoadBtn->setEnabled(!enable);
    }

    if (QString::compare(widget->objectName(),
                         d->OutputModelNodeSelector->objectName(),
                         Qt::CaseInsensitive))
    {
      d->ParameterNodeSelector->setEnabled(enable);
      d->InputNodeSelector->setEnabled(enable);
      d->OutputModelNodeSelector->setEnabled(enable);
      d->WorkspaceMeshLoadBtn->setEnabled(!enable);
    }

    if (QString::compare(widget->objectName(),
                         d->WorkspaceMeshLoadBtn->objectName(),
                         Qt::CaseInsensitive))
    {
      d->ParameterNodeSelector->setEnabled(enable);
      d->InputNodeSelector->setEnabled(enable);
      d->OutputModelNodeSelector->setEnabled(enable);
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
