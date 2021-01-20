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

// WorkspaceVisualization Logic includes
#include "vtkSlicerWorkspaceVisualizationLogic.h"

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
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLVolumeRenderingDisplayNode.h>

// VTK includes
#include <vtkCleanPolyData.h>
#include <vtkCollection.h>
#include <vtkCollectionIterator.h>
#include <vtkDelaunay3D.h>
#include <vtkGeometryFilter.h>
#include <vtkImageData.h>
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
class vtkMRMLVolumeNode;

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerWorkspaceVisualizationLogic);

//----------------------------------------------------------------------------
vtkSlicerWorkspaceVisualizationLogic::vtkSlicerWorkspaceVisualizationLogic()
{
  this->VolumesModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Volumes");
  this->VolumesLogic =
    this->VolumesModule ?
      vtkSlicerVolumesLogic::SafeDownCast(VolumesModule->logic()) :
      0;

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

  // Models module and logic
  this->ModelsModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Models");
  this->ModelsLogic =
    this->ModelsModule ?
      vtkSlicerModelsLogic::SafeDownCast(this->ModelsModule->logic()) :
      0;

  // Markups module and logic
  this->MarkupsModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Markups");
  this->MarkupsLogic =
    this->MarkupsModule ?
      vtkSlicerMarkupsLogic::SafeDownCast(this->MarkupsModule->logic()) :
      0;

  // Segmentations module and logic
  this->SegmentationsModule =
    qSlicerCoreApplication::application()->moduleManager()->module(
      "Segmentations");
  this->SegmentationsLogic = this->SegmentationsModule ?
                               vtkSlicerSegmentationsModuleLogic::SafeDownCast(
                                 this->SegmentationsModule->logic()) :
                               0;

  // CropVolume module and logic
  this->CropVolumeModule =
    qSlicerCoreApplication::application()->moduleManager()->module(
      "CropVolume");
  this->CropVolumeLogic =
    this->CropVolumeModule ?
      vtkSlicerCropVolumeLogic::SafeDownCast(this->CropVolumeModule->logic()) :
      0;
}

//----------------------------------------------------------------------------
vtkSlicerWorkspaceVisualizationLogic::~vtkSlicerWorkspaceVisualizationLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerVolumeRenderingLogic*
  vtkSlicerWorkspaceVisualizationLogic::getVolumeRenderingLogic()
{
  return this->VolumeRenderingLogic;
}

//----------------------------------------------------------------------------
qSlicerAbstractCoreModule*
  vtkSlicerWorkspaceVisualizationLogic::getVolumeRenderingModule()
{
  return this->VolumeRenderingModule;
}

//----------------------------------------------------------------------------
void vtkSlicerWorkspaceVisualizationLogic::setWorkspaceMeshSegmentationDisplayNode(
  vtkMRMLSegmentationDisplayNode* workspaceMeshSegmentationDisplayNode)
{
  if (!workspaceMeshSegmentationDisplayNode)
  {
    qCritical() << Q_FUNC_INFO << ": Workspace Mesh Model Display Node is NULL";
    return;
  }

  this->WorkspaceMeshSegmentationDisplayNode =
    workspaceMeshSegmentationDisplayNode;
}

//----------------------------------------------------------------------------
void vtkSlicerWorkspaceVisualizationLogic::setWorkspaceVisualizationNode(
  vtkMRMLWorkspaceVisualizationNode* wgn)
{
  qInfo() << Q_FUNC_INFO;

  if (!wgn)
  {
    qCritical() << Q_FUNC_INFO << ": Workspace Generation Node is NULL";
    return;
  }

  this->WorkspaceVisualizationNode = wgn;
}

//----------------------------------------------------------------------------
void vtkSlicerWorkspaceVisualizationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  qInfo() << Q_FUNC_INFO;

  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerWorkspaceVisualizationLogic::SetMRMLSceneInternal(
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
void vtkSlicerWorkspaceVisualizationLogic::RegisterNodes()
{
  qInfo() << Q_FUNC_INFO;

  if (!this->GetMRMLScene())
  {
    vtkWarningMacro("MRML scene not yet created");
    return;
  }

  this->GetMRMLScene()->RegisterNodeClass(
    vtkSmartPointer< vtkMRMLWorkspaceVisualizationNode >::New());
}

//---------------------------------------------------------------------------
void vtkSlicerWorkspaceVisualizationLogic::UpdateFromMRMLScene()
{
  qInfo() << Q_FUNC_INFO;

  assert(this->GetMRMLScene() != 0);
}

//------------------------------------------------------------------------------
void vtkSlicerWorkspaceVisualizationLogic::ProcessMRMLNodesEvents(
  vtkObject* caller, unsigned long event, void* vtkNotUsed(callData))
{
  qInfo() << Q_FUNC_INFO;

  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast(caller);
  if (callerNode == NULL)
  {
    return;
  }

  vtkMRMLWorkspaceVisualizationNode* workspaceGenerationModuleNode =
    vtkMRMLWorkspaceVisualizationNode::SafeDownCast(callerNode);
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
    this->setWorkspaceVisualizationNode(workspaceGenerationModuleNode);
    this->UpdateVolumeRendering();
  }
}

//---------------------------------------------------------------------------
void vtkSlicerWorkspaceVisualizationLogic::OnMRMLSceneEndImport()
{
  qInfo() << Q_FUNC_INFO;

  vtkSmartPointer< vtkCollection > workspaceGenerationNodes =
    vtkSmartPointer< vtkCollection >::Take(
      this->GetMRMLScene()->GetNodesByClass("vtkMRMLWorkspaceVisualizationNode"));
  vtkNew< vtkCollectionIterator > workspaceGenerationNodeIt;
  workspaceGenerationNodeIt->SetCollection(workspaceGenerationNodes);
  for (workspaceGenerationNodeIt->InitTraversal();
       !workspaceGenerationNodeIt->IsDoneWithTraversal();
       workspaceGenerationNodeIt->GoToNextItem())
  {
    vtkMRMLWorkspaceVisualizationNode* workspaceGenerationNode =
      vtkMRMLWorkspaceVisualizationNode::SafeDownCast(
        workspaceGenerationNodeIt->GetCurrentObject());
    if (workspaceGenerationNode != NULL)
    {
      this->setWorkspaceVisualizationNode(workspaceGenerationNode);
      this->UpdateVolumeRendering();
    }
  }
}

//---------------------------------------------------------------------------
void vtkSlicerWorkspaceVisualizationLogic::OnMRMLSceneStartImport()
{
  qInfo() << Q_FUNC_INFO;
}

//---------------------------------------------------------------------------
void vtkSlicerWorkspaceVisualizationLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  qInfo() << Q_FUNC_INFO;

  if (node == NULL || this->GetMRMLScene() == NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid MRML scene or node";
    return;
  }

  vtkMRMLWorkspaceVisualizationNode* workspaceGenerationNode =
    vtkMRMLWorkspaceVisualizationNode::SafeDownCast(node);
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
void vtkSlicerWorkspaceVisualizationLogic::OnMRMLSceneNodeRemoved(
  vtkMRMLNode* node)
{
  qInfo() << Q_FUNC_INFO;

  if (node == NULL || this->GetMRMLScene() == NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid MRML scene or node";
    return;
  }

  if (node->IsA("vtkSlicerWorkspaceVisualizationNode"))
  {
    vtkUnObserveMRMLNodeMacro(node);
  }
}

//------------------------------------------------------------------------------
void vtkSlicerWorkspaceVisualizationLogic::UpdateVolumeRendering()
{
  qInfo() << Q_FUNC_INFO;

  if (this->WorkspaceVisualizationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO
                << ": No workspaceGenerationNode provided to "
                   "UpdateVolumeRendering. No operation performed.";
    return;
  }

  vtkMRMLVolumeNode* inputVolumeNode =
    this->WorkspaceVisualizationNode->GetInputVolumeNode();
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
void vtkSlicerWorkspaceVisualizationLogic::UpdateMarkupFiducialNodes()
{
  qInfo() << Q_FUNC_INFO;

  if (this->WorkspaceVisualizationNode == NULL)
  {
    qCritical() << Q_FUNC_INFO
                << ": No workspaceGenerationNode provided. No operation "
                   "performed.";
    return;
  }

  vtkMRMLMarkupsFiducialNode* entryPoint =
    this->WorkspaceVisualizationNode->GetEntryPointNode();
  if (entryPoint != NULL)
  {
    this->PruneExcessMarkups(entryPoint);
  }

  vtkMRMLMarkupsFiducialNode* targetPoint =
    this->WorkspaceVisualizationNode->GetTargetPointNode();
  if (targetPoint != NULL)
  {
    this->PruneExcessMarkups(targetPoint);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerWorkspaceVisualizationLogic::PruneExcessMarkups(
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
bool vtkSlicerWorkspaceVisualizationLogic::IdentifyBurrHole(
  vtkMRMLWorkspaceVisualizationNode* wsgn)
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

  // question: #23 Use Markup Line instead of Fiducial for Entry Point Placement
  // Get the initial estimate of the axis of the drill bit by subtracting EP
  // from the TP.
  // Should I use the registration matrix to transform from MRI WS to ROBOT WS
  // here? Or later?
  vtkVector3d tp = tpNode->GetNthControlPointPositionVector(0);
  vtkVector3d ep = epNode->GetNthControlPointPositionVector(0);
  double      bAxisLineArr[3];
  vtkMath::Subtract(tp.GetData(), ep.GetData(), bAxisLineArr);
  vtkVector3d bAxisLine = vtkVector3d(bAxisLineArr);

  // Checking if BurrHoleROI node already exists.
  // TODO: Is this correct - Logic: If ROI exists and EP and TP have moved then
  // a new ROI should be drawn always?

  // vtkCollection* nodes = this->GetMRMLScene()->GetNodesByName("BurrHoleROI");
  // qDebug() << Q_FUNC_INFO << ": " << nodes;
  // if (nodes != nullptr && nodes->GetNumberOfItems() > 0)
  // {
  //   qDebug() << Q_FUNC_INFO << ": Entered node removal logic";
  //   std::vector< std::string > node_ids;
  //   for (int i = 0; i < nodes->GetNumberOfItems(); i++)
  //   {
  //     vtkMRMLNode* node =
  //     vtkMRMLNode::SafeDownCast(nodes->GetItemAsObject(i));
  //     node_ids.push_back(node->GetID());
  //   }

  //   for (const auto& node_id : node_ids)
  //   {
  //     qDebug() << Q_FUNC_INFO << ": Removing node with id: " << node_id;
  //     auto node = this->GetMRMLScene()->GetNodeByID(node_id);
  //     this->GetMRMLScene()->RemoveNode(node);
  //   }
  // }

  qDebug() << Q_FUNC_INFO << ": Cloning input volume";
  vtkMRMLScalarVolumeNode* scalarVolumeNode =
    vtkSlicerVolumesLogic::CloneVolume(
      this->GetMRMLScene(), wsgn->GetInputVolumeNode(), "BurrHoleROI");

  qDebug() << Q_FUNC_INFO << ": Created new segmentation node";
  vtkMRMLSegmentationNode* burrHoleSegmentationNode =
    vtkMRMLSegmentationNode::New();
  qDebug() << Q_FUNC_INFO << ": Set name of segmentation node";
  burrHoleSegmentationNode->SetName("BurrHoleSegmentation");

  qDebug() << Q_FUNC_INFO << ": Get and Set any dimensions";
  int* dimensions = scalarVolumeNode->GetImageData()->GetDimensions();
  int  inputDimensionsWidget[3] = {dimensions[0], dimensions[1], dimensions[2]};
  auto inputSpacingWidget       = scalarVolumeNode->GetSpacing();

  int    outputExtent[6]  = {0, -1, -0, -1, 0, -1};
  double outputSpacing[3] = {0};

  qDebug() << Q_FUNC_INFO << ": Create annotationROI node of Drill Bit Size";
  vtkMRMLAnnotationROINode* annotationROINode = vtkMRMLAnnotationROINode::New();
  // qDebug() << Q_FUNC_INFO << ": Setting name of annotationROI node";
  // annotationROINode->SetName("BurrHoleBitROI");
  qDebug() << Q_FUNC_INFO << ": Setting scale of annotationROI node";
  annotationROINode->SetROIAnnotationScale(1.0);
  double ep_Control_point[3] = {ep.GetX(), ep.GetY(), ep.GetZ()};
  double radius_Control_point[3];
  for (int i = 0; i < 3; i++)
  {
    radius_Control_point[i] = ep_Control_point[i] + 3;
  }
  qDebug() << Q_FUNC_INFO
           << ": Setting center control point of annotationROI node";
  annotationROINode->SetXYZ(ep_Control_point);
  qDebug() << Q_FUNC_INFO
           << ": Setting radius control point of annotationROI node";
  annotationROINode->SetRadiusXYZ(radius_Control_point);
  qDebug() << Q_FUNC_INFO << ": Adding annotationROI node to scene";
  this->GetMRMLScene()->AddNode(annotationROINode);

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

  qDebug() << Q_FUNC_INFO << ": Exiting burr hole identification";
  return true;
}

// feature: #18 Generate subworkspace given markup points. @FaridTavakol
//------------------------------------------------------------------------------
void vtkSlicerWorkspaceVisualizationLogic::UpdateSubWorkspace(
  vtkMRMLWorkspaceVisualizationNode* wsgn, bool apriori)
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
  vtkSlicerWorkspaceVisualizationLogic::RenderVolume(vtkMRMLVolumeNode* volumeNode)
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
  this->WorkspaceVisualizationNode->SetAndObserveAnnotationROINodeID(
    this->AnnotationROINode->GetID());

  return volumeNode;
}

/** ------------------------------- DEPRECATED ---------------------------------
//------------------------------------------------------------------------------
bool vtkSlicerWorkspaceVisualizationLogic::LoadWorkspace(
  QString workspaceMeshFilePath)
{
  qInfo() << Q_FUNC_INFO;

  if (this->ModelsLogic)
  {
    // Models Logic is available.

    this->ModelsLogic->SetMRMLScene(this->GetMRMLScene());
    WorkspaceMeshSegmentationNode = this->ModelsLogic->AddModel(
      workspaceMeshFilePath.toLocal8Bit().data(), vtkMRMLStorageNode::RAS);

    return true;
  }

  return false;
}
*/

//------------------------------------------------------------------------------
Eigen::Matrix4d
  vtkSlicerWorkspaceVisualizationLogic::convertToEigenMatrix(vtkMatrix4x4* vtkMat)
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
void vtkSlicerWorkspaceVisualizationLogic::GenerateWorkspace(
  vtkMRMLSegmentationNode* segmentationNode, Probe probe,
  vtkMatrix4x4* registrationMatrix)
{
  qInfo() << Q_FUNC_INFO;

  if (segmentationNode == NULL)
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
    vtkSlicerWorkspaceVisualizationLogic::convertToEigenMatrix(registrationMatrix),
    workspacePointCloud);
  vtkSmartPointer< vtkPolyData > polyDataWorkspace =
    vtkSmartPointer< vtkPolyData >::New();
  polyDataWorkspace->SetPoints(workspacePointCloud.GetPointer());

  // bug: Convert surface model to segmentation. @DhruvKoolRajamani
  vtkSmartPointer< vtkDelaunay3D > delaunay =
    vtkSmartPointer< vtkDelaunay3D >::New();
  delaunay->SetInputData(polyDataWorkspace);
  delaunay->SetAlpha(5);
  delaunay->SetTolerance(0.3);
  delaunay->SetOffset(5.0);
  delaunay->Update();

  vtkSmartPointer< vtkMRMLModelNode > modelNode =
    vtkSmartPointer< vtkMRMLModelNode >::New();
  vtkSmartPointer< vtkGeometryFilter > surfaceFilter =
    vtkSmartPointer< vtkGeometryFilter >::New();
  surfaceFilter->SetInputConnection(delaunay->GetOutputPort());
  surfaceFilter->Update();
  modelNode->SetAndObservePolyData(surfaceFilter->GetOutput());

  // auto segment =
  // vtkSlicerSegmentationsModuleLogic::CreateSegmentFromModelNode(
  //   modelNode, segmentationNode);

  segmentationNode->AddSegmentFromClosedSurfaceRepresentation(
    modelNode->GetPolyData(), "workspace_segment");

  WorkspaceMeshSegmentationNode = segmentationNode;
  // Attach a display node if needed
  vtkMRMLSegmentationDisplayNode* displayNode =
    vtkMRMLSegmentationDisplayNode::SafeDownCast(
      segmentationNode->GetDisplayNode());
  if (displayNode == NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Display node is null, creating a new one ";

    segmentationNode->CreateDefaultDisplayNodes();
    displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
      segmentationNode->GetDisplayNode());
  }

  if (displayNode)
  {
    std::string name =
      std::string(segmentationNode->GetName()).append("SegmentationDisplay");
    displayNode->SetName(name.c_str());
    // displayNode->SetColor(1, 1, 0);
    displayNode->Visibility2DOn();
    displayNode->Visibility3DOn();
    // displayNode->SetSliceDisplayModeToIntersection();
    // displayNode->SetSliceIntersectionVisibility(true);
    // displayNode->SetVisibility(true);
    displayNode->SetSliceIntersectionThickness(2);
    // qDebug() << Q_FUNC_INFO
    //          << displayNode->GetSliceDisplayModeAsString(
    //               displayNode->GetSliceDisplayMode());
  }

  WorkspaceMeshSegmentationNode = segmentationNode;
}

//------------------------------------------------------------------------------
vtkMRMLSegmentationNode*
  vtkSlicerWorkspaceVisualizationLogic::getWorkspaceMeshSegmentationNode()
{
  qInfo() << Q_FUNC_INFO;

  if (!this->WorkspaceMeshSegmentationNode &&
      !WorkspaceVisualizationNode->GetWorkspaceMeshSegmentationNode())
  {
    qCritical() << Q_FUNC_INFO << ": No workspace mesh model node available";
    return NULL;
  }

  if (this->WorkspaceMeshSegmentationNode !=
      this->WorkspaceVisualizationNode->GetWorkspaceMeshSegmentationNode())
  {
    this->WorkspaceMeshSegmentationNode =
      this->WorkspaceVisualizationNode->GetWorkspaceMeshSegmentationNode();
  }

  return this->WorkspaceMeshSegmentationNode;
}

//------------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode*
  vtkSlicerWorkspaceVisualizationLogic::getCurrentInputVolumeRenderingDisplayNode()
{
  return this->InputVolumeRenderingDisplayNode;
}

//------------------------------------------------------------------------------
vtkMRMLSegmentationDisplayNode* vtkSlicerWorkspaceVisualizationLogic::
  getCurrentWorkspaceMeshSegmentationDisplayNode()
{
  return this->WorkspaceMeshSegmentationDisplayNode;
}

//------------------------------------------------------------------------------
void vtkSlicerWorkspaceVisualizationLogic::UpdateSelectionNode(
  vtkMRMLWorkspaceVisualizationNode* workspaceGenerationNode)
{
  qInfo() << Q_FUNC_INFO;

  if (!workspaceGenerationNode)
  {
    qWarning() << Q_FUNC_INFO << ": workspace generation node is null";
    return;
  }

  if (this->WorkspaceVisualizationNode != workspaceGenerationNode)
  {
    this->WorkspaceVisualizationNode = workspaceGenerationNode;
  }

  vtkMRMLVolumeNode* inputVolumeNode =
    this->WorkspaceVisualizationNode->GetInputVolumeNode();

  if (inputVolumeNode == NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Input Node is null";
    return;
  }

  vtkMRMLSegmentationNode* workspaceMeshNode =
    this->WorkspaceVisualizationNode->GetWorkspaceMeshSegmentationNode();
  if (workspaceMeshNode == NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Workspace Mesh Node is null";
    return;
  }

  vtkMRMLMarkupsFiducialNode* entryPoint =
    this->WorkspaceVisualizationNode->GetEntryPointNode();
  vtkMRMLMarkupsFiducialNode* targetPoint =
    this->WorkspaceVisualizationNode->GetTargetPointNode();
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
