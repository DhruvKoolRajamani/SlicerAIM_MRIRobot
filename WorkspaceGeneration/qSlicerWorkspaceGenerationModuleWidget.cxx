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
#include <QDebug>
#include <QFileDialog>
#include <QtGui>

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
  vtkWeakPointer< vtkMRMLVolumeDisplayNode > VolumeDisplayNode;
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

  connect(d->WorkspaceOFDBtn, SIGNAL(released()), this,
          SLOT(onWorkspaceOFDButtonClick()));
  connect(d->WorkspaceLoadBtn, SIGNAL(released()), this,
          SLOT(onWorkspaceLoadButtonClick()));
  connect(d->SaveSceneBtn, SIGNAL(released()), this,
          SLOT(onSaveSceneButtonClick()));

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
  }

  vtkMRMLVolumeNode* inputVolumeNode = vtkMRMLVolumeNode::SafeDownCast(newNode);
  vtkMRMLModelNode* inputModelNode = vtkMRMLModelNode::SafeDownCast(newNode);
  if (inputVolumeNode != NULL)
  {
    workspaceGenerationNode->SetAndObserveInputNodeID(inputVolumeNode->GetID());

    // Observe display node so that we can make sure the module GUI always shows
    // up-to-date information (applies specifically to markups)
    inputVolumeNode->CreateDefaultDisplayNodes();
    vtkMRMLVolumeDisplayNode* inputVolumeDisplayNode =
      vtkMRMLVolumeDisplayNode::SafeDownCast(inputVolumeNode->GetDisplayNode());
    qvtkReconnect(d->VolumeDisplayNode, inputVolumeDisplayNode,
                  vtkCommand::ModifiedEvent, this, SLOT(updateGUIFromMRML()));
    d->VolumeDisplayNode = inputVolumeDisplayNode;
  }
  else if (inputModelNode != NULL)
  {
    workspaceGenerationNode->SetAndObserveInputNodeID(inputModelNode->GetID());
    // Observe display node so that we can make sure the module GUI always shows
    // up-to-date information (applies specifically to markups)
    inputModelNode->CreateDefaultDisplayNodes();
    vtkMRMLModelDisplayNode* inputModelDisplayNode =
      vtkMRMLModelDisplayNode::SafeDownCast(inputModelNode->GetDisplayNode());
    qvtkReconnect(d->InputModelDisplayNode, inputModelDisplayNode,
                  vtkCommand::ModifiedEvent, this, SLOT(updateGUIFromMRML()));
    d->InputModelDisplayNode = inputModelDisplayNode;
  }
  else
  {
    workspaceGenerationNode->SetAndObserveInputNodeID(NULL);
    qCritical() << Q_FUNC_INFO << ": unexpected input node type";
  }

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onInputNodeNodeAdded(
  vtkMRMLNode* addedNode)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;

  vtkMRMLVolumeNode* inputVolumeNode =
    vtkMRMLVolumeNode::SafeDownCast(addedNode);
  vtkMRMLModelNode* inputModelNode = vtkMRMLModelNode::SafeDownCast(addedNode);
  if (inputVolumeNode != NULL)
  {
    inputVolumeNode->CreateDefaultDisplayNodes();
    vtkMRMLVolumeDisplayNode* inputVolumeDisplayNode =
      vtkMRMLVolumeDisplayNode::SafeDownCast(inputVolumeNode->GetDisplayNode());
    if (inputVolumeDisplayNode)
    {
      // inputVolumeDisplayNode->SetTextScale(0.0);
    }
  }
  else if (inputModelNode != NULL)
  {
    inputModelNode->CreateDefaultDisplayNodes();
    vtkMRMLModelDisplayNode* inputModelDisplayNode =
      vtkMRMLModelDisplayNode::SafeDownCast(inputModelNode->GetDisplayNode());
    if (inputModelDisplayNode)
    {
      // inputVolumeDisplayNode->SetTextScale(0.0);
    }
  }
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

  vtkMRMLModelNode* outputModelNode = vtkMRMLModelNode::SafeDownCast(newNode);
  workspaceGenerationNode->SetAndObserveOutputModelNodeID(
    outputModelNode ? outputModelNode->GetID() : NULL);

  // Observe display node so that we can make sure the module GUI always shows
  // up-to-date information
  vtkMRMLModelDisplayNode* outputModelDisplayNode = NULL;  // temporary value
  if (outputModelNode != NULL)
  {
    outputModelNode->CreateDefaultDisplayNodes();
    outputModelDisplayNode =
      vtkMRMLModelDisplayNode::SafeDownCast(outputModelNode->GetDisplayNode());
  }
  qvtkReconnect(d->OutputModelDisplayNode, outputModelDisplayNode,
                vtkCommand::ModifiedEvent, this, SLOT(updateGUIFromMRML()));
  d->OutputModelDisplayNode = outputModelDisplayNode;

  this->updateGUIFromMRML();
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

  modelNode->CreateDefaultDisplayNodes();
  vtkMRMLModelDisplayNode* displayNode =
    vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
  if (displayNode)
  {
    displayNode->SetColor(1, 1, 0);
    displayNode->SliceIntersectionVisibilityOn();
    displayNode->SetSliceIntersectionThickness(2);
  }
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onWorkspaceOFDButtonClick()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;

  auto fileName = QFileDialog::getOpenFileName(this, tr("Open Workspace Mesh"),
                                               QDir::currentPath(),
                                               tr("Polymesh File (*.ply)"));
  d->WorkspacePathInputLineEdit->setText(fileName);
  workspaceMeshFilePath = fileName;
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onWorkspaceLoadButtonClick()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;

  if (!workspaceMeshFilePath.isEmpty())
  {
    // Return if no path is specified
    qCritical() << Q_FUNC_INFO << ": No filepath specified";
    return;
  }

  d->logic()->LoadWorkspace(workspaceMeshFilePath);
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::onSaveSceneButtonClick()
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  qInfo() << Q_FUNC_INFO;

  if (inputVolumeNode != NULL)
  {
    vtkSmartPointer< vtkXMLImageDataWriter > writer =
      vtkSmartPointer< vtkXMLImageDataWriter >::New();
    vtkSmartPointer< vtkImageData > imageData = inputVolumeNode->GetImageData();
    writer->SetInputData(imageData);
    writer->SetFileName("testvolume.vti");
    writer->Write();
    writer->Delete();
  }
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

  // set up the output model node if needed
  vtkMRMLModelNode* outputModelNode =
    workspaceGenerationNode->GetOutputModelNode();
  if (outputModelNode == NULL)
  {
    if (workspaceGenerationNode->GetScene() == NULL)
    {
      qCritical() << Q_FUNC_INFO
                  << ": Output model node is not specified and "
                     "workspaceGenerationNode is not associated with any "
                     "scene. No operation performed.";
      return;
    }
    outputModelNode = vtkMRMLModelNode::SafeDownCast(
      workspaceGenerationNode->GetScene()->AddNewNodeByClass("vtkMRMLModelNod"
                                                             "e"));
    if (workspaceGenerationNode->GetName())
    {
      std::string outputModelNodeName =
        std::string(workspaceGenerationNode->GetName()).append("Model");
      outputModelNode->SetName(outputModelNodeName.c_str());
    }
    workspaceGenerationNode->SetAndObserveOutputModelNodeID(
      outputModelNode->GetID());
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
    qCritical("Selected node not a valid module node");
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
  qInfo() << Q_FUNC_INFO;
  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(
      d->ParameterNodeSelector->currentNode());

  if (workspaceGenerationNode == NULL)
  {
    qCritical("Selected node not a valid module node");
    this->enableAllWidgets(false);
    return;
  }

  // if (!d->SaveSceneBtn->isEnabled())
  //   d->SaveSceneBtn->setEnabled(true);

  this->enableAllWidgets(true);  // unless otherwise specified, everything is
                                 // enabled

  d->InputNodeSelector->setMRMLScene(this->mrmlScene());
  // Node selectors
  vtkMRMLNode* inputNode = workspaceGenerationNode->GetInputNode();
  d->InputNodeSelector->setCurrentNode(inputNode);

  d->OutputModelNodeSelector->setMRMLScene(this->mrmlScene());
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
    QColor nodeOutputColor;
    nodeOutputColor.setRgbF(0, 0, 0);
    // d->ModelColorSelector->setColor(nodeOutputColor);
    // d->ModelSliceIntersectionCheckbox->setChecked(false);
  }

  this->blockAllSignals(false);
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::blockAllSignals(bool block)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);

  d->ParameterNodeSelector->blockSignals(block);
}

//-----------------------------------------------------------------------------
void qSlicerWorkspaceGenerationModuleWidget::enableAllWidgets(bool enable)
{
  Q_D(qSlicerWorkspaceGenerationModuleWidget);
  d->ParameterNodeSelector->setEnabled(enable);
  d->SaveSceneBtn->setEnabled(enable);
}
