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

// Volume Rendering MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLVolumeRenderingDisplayNode.h>

// VTK includes
#include <vtkCleanPolyData.h>
#include <vtkCollection.h>
#include <vtkCollectionIterator.h>
#include <vtkDelaunay3D.h>
#include <vtkGeometryFilter.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
// #include <vtkPolyDataMapper.h>
// #include <vtkPowerCrustSurfaceReconstruction.h>
#include <vtkSmartPointer.h>
#include <vtkTriangleFilter.h>

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

  if (this->VolumeRenderingLogic)
  {
    // Add a check for graphics card?
    VolumeRenderingLogic->SetDefaultRenderingMethod(
      "vtkMRMLGPURayCastVolumeRenderingDisplayNode");
  }

  this->ModelsModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Models");
  this->ModelsLogic =
    this->ModelsModule ?
      vtkSlicerModelsLogic::SafeDownCast(this->ModelsModule->logic()) :
      0;

  this->MarkupsModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Markups");
  this->MarkupsLogic =
    this->MarkupsModule ?
      vtkSlicerMarkupsLogic::SafeDownCast(this->MarkupsModule->logic()) :
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
  qInfo() << Q_FUNC_INFO;

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
    // qDebug() << Q_FUNC_INFO << ": Module node added.";
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
  qInfo() << Q_FUNC_INFO;

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

//---------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::UpdateMarkupFiducialNodes()
{
  qInfo() << Q_FUNC_INFO;

  if (this->WorkspaceGenerationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO
                << ": No workspaceGenerationNode provided. No operation "
                   "performed.";
    return;
  }

  vtkMRMLMarkupsFiducialNode* entryPoint =
    this->WorkspaceGenerationNode->GetEntryPointNode();
  if (entryPoint != NULL)
  {
    this->PruneExcessMarkups(entryPoint);
  }

  vtkMRMLMarkupsFiducialNode* targetPoint =
    this->WorkspaceGenerationNode->GetTargetPointNode();
  if (targetPoint != NULL)
  {
    this->PruneExcessMarkups(targetPoint);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::PruneExcessMarkups(
  vtkMRMLMarkupsFiducialNode* mfn)
{
  qInfo() << Q_FUNC_INFO;

  // auto markups = mfn->GetControlPoints();
  if (mfn->GetNumberOfControlPoints() > 1)
  {
    qWarning() << Q_FUNC_INFO
               << ": Removing previous Entry Point as new point has been "
                  "added.";
    if (mfn->GetNumberOfControlPoints() > 2)
    {
      qCritical() << Q_FUNC_INFO
                  << ": Excess markup nodes exist! Removing all but last.";
    }

    while (mfn->GetNumberOfControlPoints() > 1)
    {
      // qDebug() << Q_FUNC_INFO << ": Removing point with id - "
      //          << mfn->GetNumberOfControlPoints() - 1;
      mfn->RemoveNthControlPoint(
        0);  // RemoveNthControlPoint(mfn->GetNumberOfControlPoints()
             // - 1);
    }
  }

  mfn->SetNthControlPointLabel(0, mfn->GetName());
}

//-----------------------------------------------------------------------------
bool vtkSlicerWorkspaceGenerationLogic::IdentifyBurrHole(
  vtkMRMLWorkspaceGenerationNode* wsgn)
{
  qInfo() << Q_FUNC_INFO;

  if (wsgn == NULL)
  {
    qCritical() << Q_FUNC_INFO
                << ": Workspace Generation Node is not available.";
    return false;
  }

  vtkMRMLMarkupsFiducialNode* epNode = wsgn->GetEntryPointNode();
  vtkMRMLMarkupsFiducialNode* tpNode = wsgn->GetTargetPointNode();

  if (epNode == NULL || tpNode == NULL)
  {
    qCritical() << Q_FUNC_INFO
                << ": Please make sure to place both EP and TP for the first "
                   "pass at finding the burr hole.";
    return false;
  }

  // Get the initial estimate of the axis of the drill bit by subtracting EP
  // from the TP.
  // Should I use the registration matrix to transform from MRI WS to ROBOT WS
  // here? Or later?
  vtkVector3d tp = tpNode->GetNthControlPointPositionVector(0);
  vtkVector3d ep = epNode->GetNthControlPointPositionVector(0);
  double      bAxisLineArr[3];
  vtkMath::Subtract(tp.GetData(), ep.GetData(), bAxisLineArr);
  vtkVector3d bAxisLine = vtkVector3d(bAxisLineArr);

  // Get a ROI of slices from around the EP
  // vtkSmartPointer< vtkExtractVOI > voiHead =
  //   vtkSmartPointer< vtkExtractVOI >::New();
  // voiHead->SetInputConnection(
  //   wsgn->GetInputVolumeNode()->GetImageDataConnection());
  // voiHead->SetVOI(epNode->Get)

  if (wsgn->GetInputVolumeNode() != nullptr)
  {
    // Skin extraction here
    // vtkSmartPointer< vtkMarchingCubes > skinExtractor =
    //   vtkSmartPointer< vtkMarchingCubes >::New();

    // skinExtractor->SetInputConnection(
    //   wsgn->GetInputVolumeNode()->GetImageDataConnection());

    // skinExtractor->SetValue(0, 200);
    // skinExtractor->ComputeNormalsOn();
    // skinExtractor->Update();
    // qDebug() << Q_FUNC_INFO << ": Updated Marching Cubes";

    // vtkSmartPointer< vtkStripper > skinStripper =
    //   vtkSmartPointer< vtkStripper >::New();
    // skinStripper->SetInputConnection(skinExtractor->GetOutputPort());

    // skinStripper->Update();
    // qDebug() << Q_FUNC_INFO << ": Updated Stripper";
  }

  // vtkSmartPointer< vtkPolyDataMapper > skinMapper =
  //   vtkSmartPointer< vtkPolyDataMapper >::New();

  // skinMapper->SetInputConnection(skinStripper->GetOutputPort());
  // skinMapper->ScalarVisibilityOn();

  return true;
}

// feature: #18 Generate subworkspace given markup points. @FaridTavakol
//------------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::UpdateSubWorkspace(
  vtkMRMLWorkspaceGenerationNode* wsgn, bool apriori)
{
  qInfo() << Q_FUNC_INFO;

  if (apriori)
  {
    // Get Default burr hole parameters to perform fit
    // Use Burrholeparameters class, as of now assume radius to be constant.
    // center vector will change in xyz. initial step take:
    //      BurrHoleCenter = (Registration Matrix*(???)) Vector(TargetPoint -
    //      EntryPoint)
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
      // Volume Rendering will take place in new node.
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

  if (!this->GetMRMLScene()->IsNodePresent(displayNode))
  {
    this->GetMRMLScene()->AddNode(displayNode);
    volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
    VolumeRenderingLogic->UpdateDisplayNodeFromVolumeNode(displayNode,
                                                          volumeNode);
  }

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

  if (this->ModelsLogic)
  {
    // Models Logic is available.

    this->ModelsLogic->SetMRMLScene(this->GetMRMLScene());
    WorkspaceMeshModelNode = this->ModelsLogic->AddModel(
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
  NeuroKinematics   neuro_kinematics(&probe);
  ForwardKinematics fk(neuro_kinematics);

  vtkSmartPointer< vtkPoints > workspacePointCloud =
    vtkSmartPointer< vtkPoints >::New();
  workspacePointCloud = fk.get_General_Workspace(
    vtkSlicerWorkspaceGenerationLogic::convertToEigenMatrix(registrationMatrix),
    workspacePointCloud);
  vtkSmartPointer< vtkPolyData > polyDataWorkspace =
    vtkSmartPointer< vtkPolyData >::New();
  polyDataWorkspace->SetPoints(workspacePointCloud.GetPointer());

  // bug: Convert surface model to segmentation. @DhruvKoolRajamani
  vtkSmartPointer< vtkDelaunay3D > delaunay =
    vtkSmartPointer< vtkDelaunay3D >::New();
  delaunay->SetInputData(polyDataWorkspace);
  delaunay->SetAlpha(6);
  delaunay->SetTolerance(0.3);
  delaunay->SetOffset(5.0);
  delaunay->Update();

  vtkSmartPointer< vtkGeometryFilter > surfaceFilter =
    vtkSmartPointer< vtkGeometryFilter >::New();
  surfaceFilter->SetInputConnection(delaunay->GetOutputPort());
  surfaceFilter->Update();
  modelNode->SetAndObservePolyData(surfaceFilter->GetOutput());

  WorkspaceMeshModelNode = modelNode;
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
    // displayNode->SetColor(1, 1, 0);
    // displayNode->Visibility2DOn();
    // displayNode->Visibility3DOn();
    displayNode->SetSliceDisplayModeToDistanceEncodedProjection();
    displayNode->SetSliceIntersectionVisibility(true);
    displayNode->SetVisibility(true);
    displayNode->SetSliceIntersectionThickness(2);
    qDebug() << Q_FUNC_INFO
             << displayNode->GetSliceDisplayModeAsString(
                  displayNode->GetSliceDisplayMode());
  }

  WorkspaceMeshModelNode = modelNode;
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
    qWarning() << Q_FUNC_INFO << ": Input Node is null";
    return;
  }

  vtkMRMLModelNode* workspaceMeshNode =
    this->WorkspaceGenerationNode->GetWorkspaceMeshModelNode();
  if (workspaceMeshNode == NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Workspace Mesh Node is null";
    return;
  }

  vtkMRMLMarkupsFiducialNode* entryPoint =
    this->WorkspaceGenerationNode->GetEntryPointNode();
  vtkMRMLMarkupsFiducialNode* targetPoint =
    this->WorkspaceGenerationNode->GetTargetPointNode();
  if (!entryPoint || !targetPoint)
  {
    qWarning() << Q_FUNC_INFO << ": Entry or Target Points are null";
    return;
  }

  if (!this->GetMRMLScene())
  {
    qCritical() << Q_FUNC_INFO << ": no scene defined!";
    return;
  }

  // try the application logic first
  vtkMRMLApplicationLogic* mrmlAppLogic  = this->GetMRMLApplicationLogic();
  vtkMRMLSelectionNode*    selectionNode = NULL;
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

  const char* activeID = inputVolumeNode->GetID();
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
