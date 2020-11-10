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
    qDebug() << Q_FUNC_INFO << ": vtkCommand::ModifiedEvent";
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

  vtkMRMLModelNode* inputModelNode =
    vtkMRMLModelNode::SafeDownCast(workspaceGenerationNode->GetInputNode());

  if (!inputModelNode)
  {
    qCritical() << Q_FUNC_INFO
                << ": Cannot assign polydata as input model is NULL";
    return;
  }

  vtkMRMLModelNode* outputModelNode =
    workspaceGenerationNode->GetOutputModelNode();

  if (outputModelNode == NULL)
  {
    qWarning() << Q_FUNC_INFO
               << ": Output model node is not specified. No operation "
                  "performed.";
    return;
  }

  qWarning() << Q_FUNC_INFO << ": Output polydata is "
             << ((outputPolyData) ? "not null, assigning to it." :
                                    "NULL, copying input");
  if (outputPolyData == NULL)
  {
    outputModelNode->CopyContent(inputModelNode, true);
  }
  else
  {
    outputModelNode->CopyContent(inputModelNode, true);
    if (outputPolyData)
    {
      outputModelNode->SetAndObserveMesh(outputPolyData);
    }
    else
    {
      outputModelNode->SetAndObserveMesh(inputModelNode->GetMesh());
    }
    // outputModelNode->SetAndObservePolyData(outputPolyData);
  }

  // Attach a display node if needed
  vtkMRMLModelDisplayNode* displayNode =
    vtkMRMLModelDisplayNode::SafeDownCast(inputModelNode->GetDisplayNode());
  if (displayNode == NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Display node is null, creating a new one";

    outputModelNode->CreateDefaultDisplayNodes();
    displayNode =
      vtkMRMLModelDisplayNode::SafeDownCast(outputModelNode->GetDisplayNode());
  }

  std::string name =
    std::string(outputModelNode->GetName()).append("ModelDisplay");
  displayNode->SetName(name.c_str());
  displayNode->SetColor(1, 1, 0);
  displayNode->Visibility3DOn();
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

  if (this->WorkspaceGenerationNode == NULL ||
      this->WorkspaceGenerationNode != workspaceGenerationNode)
  {
    this->WorkspaceGenerationNode = workspaceGenerationNode;
  }

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

  vtkMRMLModelNode* outputModelNode =
    workspaceGenerationNode->GetOutputModelNode();
  if (outputModelNode == NULL)
  {
    qWarning() << Q_FUNC_INFO
               << ": No output model node provided to UpdateOutputModel."
               << " Will render volume if input node is Volume node.";

    vtkMRMLVolumeNode* inputVolumeNode =
      vtkMRMLVolumeNode::SafeDownCast(inputNode);

    if (inputVolumeNode != NULL)
    {
      qInfo() << Q_FUNC_INFO << ": Rendering Volume.";

      inputVolumeNode = RenderVolume(inputVolumeNode);
    }

    return;
  }

  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(inputNode);

  if (modelNode)
  {
    qDebug() << Q_FUNC_INFO << "==============================================";

    qSlicerAbstractCoreModule* modelsModule =
      qSlicerCoreApplication::application()->moduleManager()->module("Models");
    vtkSlicerModelsLogic* modelsLogic =
      modelsModule ? vtkSlicerModelsLogic::SafeDownCast(modelsModule->logic()) :
                     0;

    if (modelsLogic)
    {
      qDebug() << Q_FUNC_INFO << ": Models Logic is available.";

      modelsLogic->SetMRMLScene(this->GetMRMLScene());
      vtkSmartPointer< vtkMRMLModelNode > outputModelNode =
        this->WorkspaceGenerationNode->GetOutputModelNode();

      if (outputModelNode == NULL)
      {
        qWarning() << Q_FUNC_INFO << ": Creating new Output Model Node";
        // outputModelNode = vtkMRMLModelNode::New();
      }

      qWarning() << Q_FUNC_INFO << ": Adding Model from models logic";
      outputModelNode = modelsLogic->AddModel(modelNode->GetPolyData());

      qWarning() << Q_FUNC_INFO << ": Setting observable display node";
      this->WorkspaceGenerationNode->GetOutputModelNode()
        ->SetAndObserveDisplayNodeID(outputModelNode->GetDisplayNodeID());

      qDebug() << Q_FUNC_INFO << ": Models Logic has added model";
    }

    // // Create the model from the points
    // vtkSmartPointer< vtkPolyData > outputPolyData =
    //   vtkSmartPointer< vtkPolyData >::New();

    // outputPolyData = modelNode->GetPolyData();

    // qDebug() << Q_FUNC_INFO << ": Assigned polydata";

    // vtkSlicerWorkspaceGenerationLogic::AssignPolyDataToOutput(
    //   workspaceGenerationNode, outputPolyData);
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

  vtkMRMLVolumeRenderingDisplayNode* volumeRenderingNodes =
    volumeRenderingLogic->GetFirstVolumeRenderingDisplayNode(volumeNode);

  if (volumeRenderingLogic && volumeRenderingNodes == NULL)
  {
    qDebug() << Q_FUNC_INFO
             << ": Volume Rendering will take place in new node.";
    volumeRenderingNodes =
      volumeRenderingLogic->CreateDefaultVolumeRenderingNodes(volumeNode);
  }
  else if (!volumeRenderingLogic)
  {
    qCritical() << Q_FUNC_INFO
                << ": Volume Rendering Logic not found, returning.";

    return volumeNode;
  }

  volumeRenderingLogic->SetMRMLScene(this->GetMRMLScene());
  vtkSmartPointer< vtkMRMLVolumeRenderingDisplayNode > displayNode =
    vtkSmartPointer< vtkMRMLVolumeRenderingDisplayNode >::Take(
      volumeRenderingNodes);
  this->GetMRMLScene()->AddNode(displayNode);
  volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
  volumeRenderingLogic->UpdateDisplayNodeFromVolumeNode(displayNode,
                                                        volumeNode);

  this->WorkspaceGenerationNode->SetAndObserveOutputModelNodeID(
    volumeRenderingNodes->GetROINodeID());
  // }

  displayNode->Visibility3DOn();

  return volumeNode;
}

//------------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::LoadWorkspace(
  QString workspaceMeshFilePath)
{
  qInfo() << Q_FUNC_INFO;

  qSlicerAbstractCoreModule* modelsModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Models");
  vtkSlicerModelsLogic* modelsLogic =
    modelsModule ? vtkSlicerModelsLogic::SafeDownCast(modelsModule->logic()) :
                   0;

  if (modelsLogic)
  {
    qDebug() << Q_FUNC_INFO << ": Models Logic is available.";

    modelsLogic->SetMRMLScene(this->GetMRMLScene());
    vtkMRMLModelNode* outputModelNode = modelsLogic->AddModel(
      workspaceMeshFilePath.toLocal8Bit().data(), vtkMRMLStorageNode::RAS);
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
    this->InputModelNode = modelNode;
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
    this->InputVolumeNode = volumeNode;
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
