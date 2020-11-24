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
  this->VolumeRenderingModule =
    qSlicerCoreApplication::application()->moduleManager()->module(
      "VolumeRendering");
  this->VolumeRenderingLogic = this->VolumeRenderingModule ?
                                 vtkSlicerVolumeRenderingLogic::SafeDownCast(
                                   VolumeRenderingModule->logic()) :
                                 0;
}

//----------------------------------------------------------------------------
vtkSlicerWorkspaceGenerationLogic::~vtkSlicerWorkspaceGenerationLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerVolumeRenderingLogic*
  vtkSlicerWorkspaceGenerationLogic::getVolumeRenderingLogic()
{
  return this->VolumeRenderingLogic;
}

//----------------------------------------------------------------------------
qSlicerAbstractCoreModule*
  vtkSlicerWorkspaceGenerationLogic::getVolumeRenderingModule()
{
  return this->VolumeRenderingModule;
}

//----------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::setWorkspaceMeshModelDisplayNode(
  vtkMRMLModelDisplayNode* workspaceMeshModelDisplayNode)
{
  if (!workspaceMeshModelDisplayNode)
  {
    qCritical() << Q_FUNC_INFO << ": Workspace Mesh Model Display Node is NULL";
    return;
  }

  this->WorkspaceMeshModelDisplayNode = workspaceMeshModelDisplayNode;
}

//----------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::setWorkspaceGenerationNode(
  vtkMRMLWorkspaceGenerationNode* wgn)
{
  if (!wgn)
  {
    qCritical() << Q_FUNC_INFO << ": Workspace Generation Node is NULL";
    return;
  }

  this->WorkspaceGenerationNode = wgn;
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
    this->setWorkspaceGenerationNode(workspaceGenerationModuleNode);
    this->UpdateVolumeRendering();
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
      this->setWorkspaceGenerationNode(workspaceGenerationNode);
      this->UpdateVolumeRendering();
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
    vtkUnObserveMRMLNodeMacro(workspaceGenerationNode);  // Remove
                                                         // previous
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
void vtkSlicerWorkspaceGenerationLogic::UpdateVolumeRendering()
{
  qDebug() << Q_FUNC_INFO;

  if (this->WorkspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO
                << ": No workspaceGenerationNode provided to "
                   "UpdateVolumeRendering. No operation performed.";
    return;
  }

  vtkMRMLVolumeNode* inputVolumeNode =
    this->WorkspaceGenerationNode->GetInputVolumeNode();
  if (inputVolumeNode == NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Input Node is null";
    return;
  }

  if (inputVolumeNode != NULL)
  {
    qInfo() << Q_FUNC_INFO << ": Rendering Volume.";

    inputVolumeNode = RenderVolume(inputVolumeNode);
  }
}

//------------------------------------------------------------------------------
vtkMRMLVolumeNode*
  vtkSlicerWorkspaceGenerationLogic::RenderVolume(vtkMRMLVolumeNode* volumeNode)
{
  qInfo() << Q_FUNC_INFO;

  if (VolumeRenderingLogic)
  {
    this->InputVolumeRenderingDisplayNode =
      VolumeRenderingLogic->GetFirstVolumeRenderingDisplayNode(volumeNode);

    if (this->InputVolumeRenderingDisplayNode == NULL)
    {
      qDebug() << Q_FUNC_INFO
               << ": Volume Rendering will take place in new node.";
      this->InputVolumeRenderingDisplayNode =
        VolumeRenderingLogic->CreateDefaultVolumeRenderingNodes(volumeNode);
    }
  }
  else
  {
    qCritical() << Q_FUNC_INFO
                << ": Volume Rendering Logic not found, returning.";

    return volumeNode;
  }

  VolumeRenderingLogic->SetMRMLScene(this->GetMRMLScene());
  vtkSmartPointer< vtkMRMLVolumeRenderingDisplayNode > displayNode =
    vtkSmartPointer< vtkMRMLVolumeRenderingDisplayNode >::Take(
      this->InputVolumeRenderingDisplayNode);

  this->GetMRMLScene()->AddNode(displayNode);
  volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
  VolumeRenderingLogic->UpdateDisplayNodeFromVolumeNode(displayNode,
                                                        volumeNode);

  this->AnnotationROINode = this->InputVolumeRenderingDisplayNode->GetROINode();
  this->WorkspaceGenerationNode->SetAndObserveAnnotationROINodeID(
    this->AnnotationROINode->GetID());

  return volumeNode;
}

//------------------------------------------------------------------------------
bool vtkSlicerWorkspaceGenerationLogic::LoadWorkspace(
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
    WorkspaceMeshModelNode = modelsLogic->AddModel(
      workspaceMeshFilePath.toLocal8Bit().data(), vtkMRMLStorageNode::RAS);

    return true;
  }

  return false;
}

//------------------------------------------------------------------------------
Eigen::Matrix4d
  vtkSlicerWorkspaceGenerationLogic::convertToEigenMatrix(vtkMatrix4x4* vtkMat)
{
  Eigen::Matrix4d eigMat;

  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      eigMat(i, j) = vtkMat->GetElement(i, j);
    }
  }

  return eigMat;
}

//------------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::GenerateWorkspace(
  vtkMRMLModelNode* modelNode, Probe probe, vtkMatrix4x4* registrationMatrix)
{
  qInfo() << Q_FUNC_INFO;

  if (modelNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": output model node is invalid";
    return;
  }

  // Initialize NeuroKinematics
  NeuroKinematics neuro_kinematics(&probe);
  ForwardKinematics fk(neuro_kinematics);

  vtkSmartPointer< vtkPoints > General_Workspace_PC =
    vtkSmartPointer< vtkPoints >::New();
  General_Workspace_PC = fk.get_General_Workspace(
    vtkSlicerWorkspaceGenerationLogic::convertToEigenMatrix(registrationMatrix),
    General_Workspace_PC);
  vtkSmartPointer< vtkPolyData > polydata_General_Workspace_PC =
    vtkSmartPointer< vtkPolyData >::New();
  polydata_General_Workspace_PC->SetPoints(General_Workspace_PC);

  qDebug() << Q_FUNC_INFO << ": Number of points";
  qDebug() << polydata_General_Workspace_PC->GetNumberOfPoints();

  modelNode->SetAndObservePolyData(polydata_General_Workspace_PC);
  // Attach a display node if needed
  vtkMRMLModelDisplayNode* displayNode =
    vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
  if (displayNode == NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Display node is null, creating a new one ";

    modelNode->CreateDefaultDisplayNodes();
    displayNode =
      vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
  }

  if (displayNode)
  {
    std::string name = std::string(modelNode->GetName()).append("ModelDisplay");
    displayNode->SetName(name.c_str());
    displayNode->SetColor(1, 1, 0);
    displayNode->Visibility2DOn();
    displayNode->Visibility3DOn();
    displayNode->SetVisibility(true);
    displayNode->SetSliceIntersectionThickness(2);
  }

  WorkspaceMeshModelNode = modelNode;

  // qSlicerAbstractCoreModule* modelsModule =
  //   qSlicerCoreApplication::application()->moduleManager()->module("Models");
  // vtkSlicerModelsLogic* modelsLogic =
  //   modelsModule ? vtkSlicerModelsLogic::SafeDownCast(modelsModule->logic())
  //   :
  //                  0;

  // if (modelsLogic)
  // {
  //   qDebug() << Q_FUNC_INFO << ": Models Logic is available.";

  //   modelsLogic->SetMRMLScene(this->GetMRMLScene());
  //   WorkspaceMeshModelNode = modelsLogic->AddModel(
  //     polydata_General_Workspace_PC);  //, vtkMRMLStorageNode::RAS);

  //   // modelNode->SetPolyDataConnection(
  //   //   generateWorkspace->GetPolyDataConnection());
  //   // modelNode->SetAndObservePolyData(generateWorkspace->GetPolyData());

  //   // WorkspaceMeshModelNode = modelNode;
  // }
}

//------------------------------------------------------------------------------
vtkMRMLModelNode* vtkSlicerWorkspaceGenerationLogic::getWorkspaceMeshModelNode()
{
  qInfo() << Q_FUNC_INFO;

  if (!this->WorkspaceMeshModelNode &&
      !WorkspaceGenerationNode->GetWorkspaceMeshModelNode())
  {
    qCritical() << Q_FUNC_INFO << ": No workspace mesh model node available";
    return NULL;
  }

  if (this->WorkspaceMeshModelNode !=
      this->WorkspaceGenerationNode->GetWorkspaceMeshModelNode())
  {
    this->WorkspaceMeshModelNode =
      this->WorkspaceGenerationNode->GetWorkspaceMeshModelNode();
  }

  return this->WorkspaceMeshModelNode;
}

//------------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode*
  vtkSlicerWorkspaceGenerationLogic::getCurrentInputVolumeRenderingDisplayNode()
{
  return this->InputVolumeRenderingDisplayNode;
}

//------------------------------------------------------------------------------
vtkMRMLModelDisplayNode*
  vtkSlicerWorkspaceGenerationLogic::getCurrentWorkspaceMeshModelDisplayNode()
{
  return this->WorkspaceMeshModelDisplayNode;
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

  if (this->WorkspaceGenerationNode != workspaceGenerationNode)
  {
    this->WorkspaceGenerationNode = workspaceGenerationNode;
  }

  vtkMRMLVolumeNode* inputVolumeNode =
    this->WorkspaceGenerationNode->GetInputVolumeNode();

  if (inputVolumeNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": Input Node is null";
    return;
  }

  if (this->InputVolumeNode != inputVolumeNode)
  {
    this->InputVolumeNode = inputVolumeNode;
  }

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

  const char* activeID = this->InputVolumeNode->GetID();
  if (!activeID)
  {
    qCritical() << Q_FUNC_INFO << ": No active ID available!";
    return;
  }

  const char* selectionNodeActivePlaceNodeID =
    selectionNode->GetActivePlaceNodeID();
  if (selectionNodeActivePlaceNodeID != NULL && activeID != NULL &&
      !strcmp(selectionNodeActivePlaceNodeID, activeID))
  {
    qCritical() << Q_FUNC_INFO << ": Active ID does not match selected node.";
    return;
  }

  selectionNode->SetReferenceActivePlaceNodeID(activeID);
}
