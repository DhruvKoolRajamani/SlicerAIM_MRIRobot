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

// QT includes
#include <QDebug>

// WorkspaceGeneration Logic includes
#include "vtkSlicerWorkspaceGenerationLogic.h"

// Slicer Module includes
#include <qSlicerAbstractModule.h>
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>

// MRML includes
#include "vtkMRMLModelNode.h"
#include "vtkMRMLSelectionNode.h"
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>

// Volume Rendering Logic includes
#include <vtkSlicerVolumeRenderingLogic.h>
#include <vtkSlicerVolumeRenderingModuleLogicExport.h>

// Models logic includes
#include <vtkSlicerModelsLogic.h>
#include <vtkSlicerModelsModuleLogicExport.h>

// Volume Rendering MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLVolumeRenderingDisplayNode.h>

// VTK includes
#include <vtkCleanPolyData.h>
#include <vtkCollection.h>
#include <vtkCollectionIterator.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>

// STD includes
#include <cassert>
#include <cmath>
#include <set>
#include <vector>

class qSlicerAbstractCoreModule;
class vtkSlicerVolumeRenderingLogic;
class vtkMRMLVolumeRenderingDisplayNode;

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerWorkspaceGenerationLogic);

//----------------------------------------------------------------------------
vtkSlicerWorkspaceGenerationLogic::vtkSlicerWorkspaceGenerationLogic()
{
  this->isInputModelNode = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerWorkspaceGenerationLogic::~vtkSlicerWorkspaceGenerationLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  qInfo() << Q_FUNC_INFO;

  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::SetMRMLSceneInternal(
  vtkMRMLScene* newScene)
{
  qInfo() << Q_FUNC_INFO;

  vtkNew< vtkIntArray > events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::StartBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::StartImportEvent);
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::RegisterNodes()
{
  qInfo() << Q_FUNC_INFO;

  if (!this->GetMRMLScene())
  {
    vtkWarningMacro("MRML scene not yet created");
    return;
  }

  this->GetMRMLScene()->RegisterNodeClass(
    vtkSmartPointer< vtkMRMLWorkspaceGenerationNode >::New());
}

//---------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::UpdateFromMRMLScene()
{
  qInfo() << Q_FUNC_INFO;

  assert(this->GetMRMLScene() != 0);
}

//------------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::ProcessMRMLNodesEvents(
  vtkObject* caller, unsigned long event, void* vtkNotUsed(callData))
{
  qInfo() << Q_FUNC_INFO;

  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast(caller);
  if (callerNode == NULL)
  {
    return;
  }

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationModuleNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(callerNode);
  if (workspaceGenerationModuleNode == NULL ||
      !workspaceGenerationModuleNode->GetAutoUpdateOutput())
  {
    return;
  }

  if (this->GetMRMLScene() && (this->GetMRMLScene()->IsImporting() ||
                               this->GetMRMLScene()->IsRestoring() ||
                               this->GetMRMLScene()->IsClosing()))
  {
    return;
  }

  if (event == vtkCommand::ModifiedEvent)
  {
    qDebug() << Q_FUNC_INFO << "vtkCommand::ModifiedEvent";
    this->UpdateOutputModel(workspaceGenerationModuleNode);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::OnMRMLSceneEndImport()
{
  qInfo() << Q_FUNC_INFO;

  vtkSmartPointer< vtkCollection > workspaceGenerationNodes =
    vtkSmartPointer< vtkCollection >::Take(
      this->GetMRMLScene()->GetNodesByClass("vtkMRMLWorkspaceGenerationNode"));
  vtkNew< vtkCollectionIterator > workspaceGenerationNodeIt;
  workspaceGenerationNodeIt->SetCollection(workspaceGenerationNodes);
  for (workspaceGenerationNodeIt->InitTraversal();
       !workspaceGenerationNodeIt->IsDoneWithTraversal();
       workspaceGenerationNodeIt->GoToNextItem())
  {
    vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
      vtkMRMLWorkspaceGenerationNode::SafeDownCast(
        workspaceGenerationNodeIt->GetCurrentObject());
    if (workspaceGenerationNode != NULL)
    {
      this->UpdateOutputModel(workspaceGenerationNode);
    }
  }
}

//---------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::OnMRMLSceneStartImport()
{
  qInfo() << Q_FUNC_INFO;
}

//---------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  qInfo() << Q_FUNC_INFO;

  if (node == NULL || this->GetMRMLScene() == NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid MRML scene or node";
    return;
  }

  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceGenerationNode::SafeDownCast(node);
  if (workspaceGenerationNode)
  {
    qDebug() << Q_FUNC_INFO << ": Module node added.";
    vtkUnObserveMRMLNodeMacro(workspaceGenerationNode);  // Remove previous
                                                         // observers.
    vtkNew< vtkIntArray > events;
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(workspaceGenerationNode, events.GetPointer());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::OnMRMLSceneNodeRemoved(
  vtkMRMLNode* node)
{
  qInfo() << Q_FUNC_INFO;

  if (node == NULL || this->GetMRMLScene() == NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid MRML scene or node";
    return;
  }

  if (node->IsA("vtkSlicerWorkspaceGenerationNode"))
  {
    vtkUnObserveMRMLNodeMacro(node);
  }
}

//------------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::AssignPolyDataToOutput(
  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode,
  vtkPolyData* outputPolyData)
{
  qInfo() << Q_FUNC_INFO;

  vtkMRMLModelNode* outputModelNode =
    workspaceGenerationNode->GetOutputModelNode();
  if (outputModelNode == NULL)
  {
    qWarning() << Q_FUNC_INFO
               << ": Output model node is not specified. No operation "
                  "performed.";
    return;
  }
  outputModelNode->SetAndObservePolyData(outputPolyData);

  // Attach a display node if needed
  vtkMRMLModelDisplayNode* displayNode =
    vtkMRMLModelDisplayNode::SafeDownCast(outputModelNode->GetDisplayNode());
  if (displayNode == NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Display node is null, creating a new one";

    outputModelNode->CreateDefaultDisplayNodes();
    displayNode =
      vtkMRMLModelDisplayNode::SafeDownCast(outputModelNode->GetDisplayNode());
    std::string name =
      std::string(outputModelNode->GetName()).append("ModelDisplay");
    displayNode->SetName(name.c_str());
    displayNode->SetColor(1, 1, 0);
  }
  else
  {
    displayNode =
      vtkMRMLModelDisplayNode::SafeDownCast(outputModelNode->GetDisplayNode());
    std::string name =
      std::string(outputModelNode->GetName()).append("ModelDisplay");
    displayNode->SetName(name.c_str());
    displayNode->SetColor(1, 1, 0);
  }
}

//------------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::ModelToPoints(
  vtkMRMLModelNode* inputModelNode, vtkPoints* outputPoints)
{
  qInfo() << Q_FUNC_INFO;

  if (inputModelNode == NULL)
  {
    qWarning() << Q_FUNC_INFO
               << ": Input node is null. No points will be obtained.";
    return;
  }

  if (outputPoints == NULL)
  {
    qWarning() << Q_FUNC_INFO
               << ": Output vtkPoints is null. No points will be obtained.";
    return;
  }

  vtkPolyData* inputPolyData = inputModelNode->GetPolyData();
  if (inputPolyData == NULL)
  {
    return;
  }
  vtkPoints* inputPoints = inputPolyData->GetPoints();
  if (inputPoints == NULL)
  {
    return;
  }
  outputPoints->DeepCopy(inputPoints);
}

//------------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::UpdateOutputModel(
  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode)
{
  qDebug() << Q_FUNC_INFO;

  if (workspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO
                << ": No workspaceGenerationNode provided to "
                   "UpdateOutputModel. "
                   "No operation performed.";
    return;
  }

  vtkMRMLNode* inputNode = workspaceGenerationNode->GetInputNode();
  if (inputNode == NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Input Node is null";
    return;
  }

  if (workspaceGenerationNode->GetOutputModelNode() == NULL)
  {
    qCritical() << Q_FUNC_INFO
                << ": No output model node provided to UpdateOutputModel. No "
                   "operation performed.";
    return;
  }

  // extract the input points from the MRML node, according to its type
  vtkSmartPointer< vtkPoints > controlPoints =
    vtkSmartPointer< vtkPoints >::New();

  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(inputNode);

  if (!modelNode)
  {
    qWarning() << Q_FUNC_INFO << ": Input is not a model node, checking volume";

    vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(inputNode);
    if (!volumeNode)
    {
      qCritical() << Q_FUNC_INFO << ": Input is not a volume node either";
      return;
    }

    qCritical() << Q_FUNC_INFO << ": Logic has not been generated yet";
    // Convert Volume to Model

    return;

    // Create the model from the points
    vtkSmartPointer< vtkPolyData > outputPolyData =
      vtkSmartPointer< vtkPolyData >::New();

    volumeNode = RenderVolume(volumeNode);

    // outputPolyData->DeepCopy(volumeNode->GetPolyData());

    // vtkSlicerWorkspaceGenerationLogic::AssignPolyDataToOutput(
    //   workspaceGenerationNode, outputPolyData);
  }
  else
  {
    // Create the model from the points
    vtkSmartPointer< vtkPolyData > outputPolyData =
      vtkSmartPointer< vtkPolyData >::New();

    outputPolyData = modelNode->GetPolyData();

    vtkSlicerWorkspaceGenerationLogic::AssignPolyDataToOutput(
      workspaceGenerationNode, outputPolyData);
  }
}

//------------------------------------------------------------------------------
vtkMRMLVolumeNode*
  vtkSlicerWorkspaceGenerationLogic::RenderVolume(vtkMRMLVolumeNode* volumeNode)
{
  qInfo() << Q_FUNC_INFO;

  qSlicerAbstractCoreModule* volumeRenderingModule =
    qSlicerCoreApplication::application()->moduleManager()->module(
      "VolumeRendering");
  vtkSlicerVolumeRenderingLogic* volumeRenderingLogic =
    volumeRenderingModule ? vtkSlicerVolumeRenderingLogic::SafeDownCast(
                              volumeRenderingModule->logic()) :
                            0;

  if (volumeRenderingLogic)
  {
    qDebug() << Q_FUNC_INFO << ": Volume Rendering Logic is available.";

    volumeRenderingLogic->SetMRMLScene(this->GetMRMLScene());
    volumeRenderingLogic->CreateDefaultVolumeRenderingNodes(volumeNode);
    vtkSmartPointer< vtkMRMLVolumeRenderingDisplayNode > displayNode =
      vtkSmartPointer< vtkMRMLVolumeRenderingDisplayNode >::Take(
        volumeRenderingLogic->CreateVolumeRenderingDisplayNode());
    this->GetMRMLScene()->AddNode(displayNode);
    volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
    volumeRenderingLogic->UpdateDisplayNodeFromVolumeNode(displayNode,
                                                          volumeNode);
  }

  return volumeNode;
}

//------------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::LoadWorkspace(
  QString workspaceMeshFilePath)
{
  qInfo() << Q_FUNC_INFO;

  qSlicerAbstractCoreModule* modelsModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Modules");
  vtkSlicerModelsLogic* modelsLogic =
    modelsModule ? vtkSlicerModelsLogic::SafeDownCast(modelsModule->logic()) :
                   0;

  if (modelsLogic)
  {
    qDebug() << Q_FUNC_INFO << ": Modules Logic is available.";

    modelsLogic->SetMRMLScene(this->GetMRMLScene());
    modelsLogic->AddModel(workspaceMeshFilePath.toLocal8Bit().data(), 1);
  }
}

//------------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::UpdateSelectionNode(
  vtkMRMLWorkspaceGenerationNode* workspaceGenerationNode)
{
  qInfo() << Q_FUNC_INFO;

  if (!workspaceGenerationNode)
  {
    qWarning() << Q_FUNC_INFO << ": workspace generation node is null";
    return;
  }
  vtkMRMLNode* inputNode = workspaceGenerationNode->GetInputNode();

  if (inputNode == NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Input Node is null";
    return;
  }

  bool isModelNode = false;

  // Check if model node is available
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(inputNode);
  vtkMRMLVolumeNode* volumeNode = NULL;
  if (!modelNode)
  {
    qWarning() << Q_FUNC_INFO << ": No model node selected";
  }
  else
  {
    isModelNode = true;
    this->inputModelNode = modelNode;
  }

  if (!isModelNode)
  {
    // Try Volume Node
    volumeNode = vtkMRMLVolumeNode::SafeDownCast(inputNode);
    if (!volumeNode)
    {
      // No input node available selected
      qCritical() << Q_FUNC_INFO << ": No input node available selected";
      return;
    }
    this->inputVolumeNode = volumeNode;
  }

  *(this->isInputModelNode) = isModelNode;

  if (!this->GetMRMLScene())
  {
    qCritical() << Q_FUNC_INFO << ": no scene defined!";
    return;
  }

  // try the application logic first
  vtkMRMLApplicationLogic* mrmlAppLogic = this->GetMRMLApplicationLogic();
  vtkMRMLSelectionNode* selectionNode = NULL;
  if (mrmlAppLogic)
  {
    selectionNode = mrmlAppLogic->GetSelectionNode();
  }
  else
  {
    // try a default string
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
  }
  if (!selectionNode)
  {
    qCritical() << Q_FUNC_INFO << ": selection node is not available";
    return;
  }

  const char* activeID = (isModelNode) ?
                           (modelNode ? modelNode->GetID() : NULL) :
                           (volumeNode ? volumeNode->GetID() : NULL);
  if (!activeID)
  {
    return;
  }

  const char* selectionNodeActivePlaceNodeID =
    selectionNode->GetActivePlaceNodeID();
  if (selectionNodeActivePlaceNodeID != NULL && activeID != NULL &&
      !strcmp(selectionNodeActivePlaceNodeID, activeID))
  {
    // no change
    return;
  }

  selectionNode->SetReferenceActivePlaceNodeID(activeID);
}
