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
#include <QDir>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <ctime>
#include <qSlicerIOManager.h>
#include <qfileinfo.h>

// WorkspaceGeneration Logic includes
#include "vtkSlicerWorkspaceGenerationLogic.h"

// Slicer Module includes
#include <qSlicerAbstractModule.h>
#include <qSlicerCoreApplication.h>
#include <qSlicerCoreIOManager.h>
#include <qSlicerIO.h>
#include <qSlicerIOOptions.h>
#include <qSlicerModuleManager.h>

// MRML includes
#include "vtkMRMLModelNode.h"
#include "vtkMRMLSelectionNode.h"
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>

// Volume Rendering MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLVolumeRenderingDisplayNode.h>

// VTK includes
#include "vtkMRMLVolumePropertyNode.h"
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
#include <vtkXMLImageDataWriter.h>
// #include <vtkPolyDataMapper.h>
// #include <vtkPowerCrustSurfaceReconstruction.h>
#include <vtkGaussianSplatter.h>
#include <vtkMRMLMarkupsNode.h>
#include <vtkSmartPointer.h>
#include <vtkTriangleFilter.h>

// STD includes
#include <cassert>
#include <chrono>
#include <cmath>
#include <set>
#include <vector>

// NvidiaAIAA includes
#include <nlohmann/json.hpp>

// ITK Includes
#include <itkImageFileReader.h>
#include <itkLabelImageToLabelMapFilter.h>
#include <itkLabelMap.h>
// #include <itkLabelMapMaskImageFilter.h>
#include <itkLabelObject.h>
#include <itkNiftiImageIO.h>

#include <PointSetUtilities/PointSetUtilities.hpp>

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

  this->SegmentationsModule =
    qSlicerCoreApplication::application()->moduleManager()->module(
      "Segmentation");
  this->SegmentationsLogic = this->SegmentationsModule ?
                               vtkSlicerSegmentationsModuleLogic::SafeDownCast(
                                 this->SegmentationsModule->logic()) :
                               0;
}

//----------------------------------------------------------------------------
vtkSlicerWorkspaceGenerationLogic::~vtkSlicerWorkspaceGenerationLogic()
{
  delete NvidiaAIAAClient;
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
void vtkSlicerWorkspaceGenerationLogic::setWorkspaceMeshSegmentationDisplayNode(
  vtkMRMLSegmentationDisplayNode* workspaceMeshSegmentationDisplayNode)
{
  qInfo() << Q_FUNC_INFO;

  if (!workspaceMeshSegmentationDisplayNode)
  {
    qCritical() << Q_FUNC_INFO << ": Workspace Mesh Model Display Node is NULL";
    return;
  }

  this->WorkspaceMeshSegmentationDisplayNode =
    workspaceMeshSegmentationDisplayNode;
}

//------------------------------------------------------------------------------
void vtkSlicerWorkspaceGenerationLogic::setBurrHoleSegmentationDisplayNode(
  vtkMRMLSegmentationDisplayNode* burrHoleSegmentationDisplayNode)
{
  qInfo() << Q_FUNC_INFO;

  if (!burrHoleSegmentationDisplayNode)
  {
    qCritical() << Q_FUNC_INFO
                << ": BurrHole Segmentation Display Node is NULL";
    return;
  }

  this->BurrHoleSegmentationDisplayNode = burrHoleSegmentationDisplayNode;
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
bool vtkSlicerWorkspaceGenerationLogic::UpdateBHSegmentationMask(
  vtkMRMLWorkspaceGenerationNode* wsgn, nvidia::aiaa::PointSet extremePoints,
  const QString& maskFile, bool overwriteCurrentSegment,
  boost::optional< float > sliceIndex, int* cropBox)
{
  qInfo() << Q_FUNC_INFO;
  // Start timer (for response?)
  clock_t startTime = clock();

  vtkSmartPointer< vtkMRMLSegmentationNode > bHSegNode =
    wsgn->GetBurrHoleSegmentationNode();
  if (bHSegNode != NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Segmentation node is not NULL.";
    wsgn->SetAndObserveBurrHoleSegmentationNodeID(NULL);
    this->setBurrHoleSegmentationDisplayNode(NULL);
    // bHSegNode->RemoveAllObservers();
    this->GetMRMLScene()->RemoveNode(bHSegNode);
  }

  if (maskFile.isEmpty())
  {
    qCritical() << Q_FUNC_INFO << ": mask file is null, exiting.";
    return false;
  }

  if (FILE* file = fopen(maskFile.toUtf8().constData(), "r"))
  {
    qDebug() << Q_FUNC_INFO << ": mask file exists";
    fclose(file);
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": mask file does not exist! exiting.";
    return false;
  }

  QFileInfo inFileInfo = QFileInfo(maskFile);

  qSlicerIO::IOFileType fileType =
    qSlicerCoreApplication::application()->coreIOManager()->fileType(
      inFileInfo.absoluteFilePath());
  fileType = "SegmentationFile";
  QList< qSlicerIO::IOProperties > properties;
  qSlicerIO::IOProperties          property;
  property["fileName"] = inFileInfo.absoluteFilePath();
  property["fileType"] = "SegmentationFile";
  properties.append(property);
  QList< QString > file_types =
    qSlicerCoreApplication::application()->coreIOManager()->fileTypes(maskFile);
  qDebug() << Q_FUNC_INFO << ": " << file_types;
  vtkMRMLNode* node = qSlicerCoreApplication::application()
                        ->coreIOManager()
                        ->loadNodesAndGetFirst(fileType, property);

  if (node == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": Error loading file " + maskFile;
    return false;
  }

  bHSegNode = vtkMRMLSegmentationNode::SafeDownCast(node);

  bHSegNode->SetName("BurrHoleSegmentation");

  wsgn->SetAndObserveBurrHoleSegmentationNodeID(bHSegNode->GetID());
  if (bHSegNode->GetDisplayNode() == NULL)
  {
    qWarning() << Q_FUNC_INFO << ": Creating display node for segmentation";
    bHSegNode->CreateDefaultDisplayNodes();
  }

  bHSegNode->CreateClosedSurfaceRepresentation();

  vtkMRMLSegmentationDisplayNode* segDispNode =
    vtkMRMLSegmentationDisplayNode::SafeDownCast(bHSegNode->GetDisplayNode());
  segDispNode->Visibility2DOn();
  segDispNode->Visibility3DOn();
  segDispNode->SetSliceIntersectionThickness(2);
  segDispNode->SetAllSegmentsVisibility(true);
  segDispNode->SetAllSegmentsVisibility3D(true);
  // bHSegNode->CreateClosedSurfaceRepresentation();
  this->setBurrHoleSegmentationDisplayNode(segDispNode);

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerWorkspaceGenerationLogic::DebugIdentifyBurrHole(
  vtkMRMLWorkspaceGenerationNode* wsgn)
{
  qInfo() << Q_FUNC_INFO;

  if (wsgn == NULL)
  {
    qCritical() << Q_FUNC_INFO
                << ": Workspace Generation Node is not available.";
    return false;
  }

  QString maskfile(QDir::currentPath() + QDir::separator() +
                   "WorkspaceGeneration" + QDir::separator() + "Resources" +
                   QDir::separator() + "scenes" + QDir::separator() +
                   "testmask-label.nii.seg.nrrd");

  nvidia::aiaa::PointSet bHExtremePointSet;
  return UpdateBHSegmentationMask(wsgn, bHExtremePointSet, maskfile);
}

//-----------------------------------------------------------------------------
bool vtkSlicerWorkspaceGenerationLogic::IdentifyBurrHole(
  vtkMRMLWorkspaceGenerationNode* wsgn)
{
  qInfo() << Q_FUNC_INFO;

  int result = 0;

  if (wsgn == NULL)
  {
    qCritical() << Q_FUNC_INFO
                << ": Workspace Generation Node is not available.";
    return false;
  }

  vtkMRMLMarkupsFiducialNode* bHEPNode = wsgn->GetBHExtremePointNode();

  if (bHEPNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": Extreme Point Node has not been set.";
    return false;
  }

  vtkMRMLVolumeNode* inputVolumeNode = wsgn->GetInputVolumeNode();
  if (inputVolumeNode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": Input Volume Node has not been set.";
    return false;
  }

  vtkSmartPointer< vtkMRMLVolumeNode > outputVolumeNode;

  vtkSmartPointer< vtkMatrix4x4 > RASToIJKMatrix =
    vtkSmartPointer< vtkMatrix4x4 >::New();
  inputVolumeNode->GetRASToIJKMatrix(RASToIJKMatrix);
  // std::string in_file = NvidiaAIAAClient->getSession(inputVolumeNode);

  QString ext      = ".nii.gz";
  QString ext_type = "NifTI (.nii.gz)";
  /* initialize random seed: */
  srand(time(NULL));
  int     random = rand() % 10000 + 1;
  QString in_file(QDir::currentPath() + QDir::separator() + "in_file_" +
                  std::to_string(random).c_str());
  QString out_file(QDir::currentPath() + QDir::separator() + "out_file_" +
                   std::to_string(random).c_str() + "-label" + ext);

  QFileInfo inFileInfo = QFileInfo(in_file + ext);
  // QFileInfo outFileInfo = QFileInfo(out_file);
  qSlicerIO::IOProperties savingParameters;
  qSlicerIO::IOFileType   fileType =
    qSlicerCoreApplication::application()->coreIOManager()->fileWriterFileType(
      inputVolumeNode, ext_type);
  savingParameters["nodeID"]     = QString(inputVolumeNode->GetID());
  savingParameters["fileName"]   = inFileInfo.absoluteFilePath();
  savingParameters["fileFormat"] = ext_type;

  bool success =
    qSlicerCoreApplication::application()->coreIOManager()->saveNodes(
      fileType, savingParameters);

  std::string sessionID = "";

  if (!success)
  {
    qWarning() << Q_FUNC_INFO << ": in_file is Empty";
    // in_file = NvidiaAIAAClient->createSession(inputVolumeNode);
  }
  else
  {
    try
    {
      NvidiaAIAAClient = new nvidia::aiaa::Client("http://192.168.1.4:8123");

      std::string response = NvidiaAIAAClient->createSession(
        std::string(inFileInfo.absoluteFilePath().toUtf8().constData()));

      sessionID = response;
      // qDebug() << Q_FUNC_INFO << response.c_str();
      // nlohmann::json j = nlohmann::json::parse(response);
      // sessionID        = j.find("session_id") != j.end() ?
      //                      j["session_id"].get< std::string >() :
      //                      std::string();
    }
    catch (nvidia::aiaa::exception& e)
    {
      qCritical() << Q_FUNC_INFO
                  << "nvidia::aiaa::exception => nvidia.aiaa.error." << e.id
                  << "; description: " << e.name().c_str();
    }
    catch (nlohmann::json::parse_error& e)
    {
      qCritical() << Q_FUNC_INFO << e.what();
      // throw exception(exception::RESPONSE_PARSE_ERROR, e.what());
    }
    catch (nlohmann::json::type_error& e)
    {
      qCritical() << Q_FUNC_INFO << e.what();
      // throw exception(exception::RESPONSE_PARSE_ERROR, e.what());
    }
  }

  nvidia::aiaa::PointSet bHExtremePointSet;

  for (int i = 0; i < bHEPNode->GetNumberOfFiducials(); i++)
  {
    double coord[3] = {0.0, 0.0, 0.0};
    bHEPNode->GetNthFiducialPosition(i, coord);

    double world[4] = {0.0, 0.0, 0.0, 0.0};
    bHEPNode->GetNthFiducialWorldCoordinates(i, world);

    double             p_Ras[4] = {coord[0], coord[1], coord[2], 1.0};
    double*            p_Ijk    = RASToIJKMatrix->MultiplyDoublePoint(p_Ras);
    std::vector< int > points;
    for (int idx = 0; idx < 3; idx++)
    {
      points.push_back(( int ) p_Ijk[idx]);
    }

    bHExtremePointSet.points.push_back(points);
  }

  QString pointsStr;
  for (int i = 0; i < bHExtremePointSet.points.size(); i++)
  {
    std::vector< int > point = bHExtremePointSet.points[i];
    std::string        str =
      "Point " + std::to_string(i) + ": [" + std::to_string(point[0]) + ", " +
      std::to_string(point[1]) + ", " + std::to_string(point[2]) + "]";
    pointsStr.append(str.c_str());
  }

  qDebug() << Q_FUNC_INFO << ": Point List is";
  qDebug() << pointsStr;

  if (wsgn->GetInputVolumeNode() != nullptr)
  {
    try
    {
      // List all models
      nvidia::aiaa::ModelList modelList = NvidiaAIAAClient->models();
      qDebug() << Q_FUNC_INFO << "Models Supported by AIAA Server: "
               << modelList.toJson().c_str();

      // annotation_mri_brain_tumors_t1ce_tc -> label: brain tumor core
      nvidia::aiaa::Model model = modelList.getMatchingModel(
        "brain tumor core", nvidia::aiaa::Model::annotation);

      result = NvidiaAIAAClient->dextr3D(
        model, bHExtremePointSet, QString(in_file + ext).toUtf8().constData(),
        out_file.toUtf8().constData(), false, sessionID);

      // https://github.com/NVIDIA/ai-assisted-annotation-client/blob/df65b44448348f7d0d0af4feaaca772254aae3f1/slicer-plugin/NvidiaAIAA/SegmentEditorNvidiaAIAALib/SegmentEditorEffect.py#L231
      if (result == 0)
      {
        result = ( int ) this->UpdateBHSegmentationMask(wsgn, bHExtremePointSet,
                                                        out_file, true);
        if (result == 0)
        {
          qCritical() << Q_FUNC_INFO << ": BHSegmentation Failed, exiting";
        }
      }
      else if (result == -1)
      {
        qCritical() << Q_FUNC_INFO << ": Input file doesn't exist";
      }
      else if (result == -2)
      {
        qCritical() << Q_FUNC_INFO << ": Insufficient points in the input";
      }
    }
    catch (nvidia::aiaa::exception& e)
    {
      qCritical() << Q_FUNC_INFO
                  << "nvidia::aiaa::exception => nvidia.aiaa.error." << e.id
                  << "; description: " << e.name().c_str();
    }
  }

  // Clear temp files
  std::remove(QString(in_file + ext).toUtf8().constData());
  std::remove(out_file.toUtf8().constData());

  if (result == 1)
    return true;
  return false;
}

/** ------------------------------- DEPRECATED ---------------------------------
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

  // Get a ROI of slices from around the EP
  // vtkSmartPointer< vtkExtractVOI > voiHead =
  //   vtkSmartPointer< vtkExtractVOI >::New();
  // voiHead->SetInputConnection(
  //   wsgn->GetInputVolumeNode()->GetImageDataConnection());
  // voiHead->SetVOI(epNode->Get)

  if (wsgn->GetInputVolumeNode() != nullptr)
  {
    try
    {
      // List all models
      nvidia::aiaa::ModelList modelList = NvidiaAIAAClient->models();
      qDebug() << Q_FUNC_INFO << "Models Supported by AIAA Server: "
               << modelList.toJson().c_str();

      nvidia::aiaa::Model model =
        modelList.getMatchingModel("annotation_mri_brain_tumors_t1ce_tc");
    }
    catch (nvidia::aiaa::exception& e)
    {
      qCritical() << Q_FUNC_INFO
                  << "nvidia::aiaa::exception => nvidia.aiaa.error." << e.id
                  << "; description: " << e.name().c_str();
    }
  }

  return true;
}
*/

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
    VolumeRenderingLogic->SetMRMLScene(this->GetMRMLScene());

    this->InputVolumeRenderingDisplayNode =
      VolumeRenderingLogic->GetFirstVolumeRenderingDisplayNode(volumeNode);

    if (this->InputVolumeRenderingDisplayNode == NULL)
    {
      // Volume Rendering will take place in new node.
      this->InputVolumeRenderingDisplayNode =
        VolumeRenderingLogic->CreateDefaultVolumeRenderingNodes(volumeNode);
    }
    else
    {
      VolumeRenderingLogic->UpdateDisplayNodeFromVolumeNode(
        this->InputVolumeRenderingDisplayNode, volumeNode);
    }

    // Get the current Volume Property Node.
    vtkMRMLVolumePropertyNode* volumePropertyNode =
      this->InputVolumeRenderingDisplayNode->GetVolumePropertyNode();

    // Copy the MRI preset to the volume property node
    volumePropertyNode->Copy(
      VolumeRenderingLogic->GetPresetByName("MR-Default"));
  }
  else
  {
    qCritical() << Q_FUNC_INFO
                << ": Volume Rendering Logic not found, returning.";

    return volumeNode;
  }

  // vtkSmartPointer< vtkMRMLVolumeRenderingDisplayNode > displayNode =
  //   vtkSmartPointer< vtkMRMLVolumeRenderingDisplayNode >::Take(
  //     this->InputVolumeRenderingDisplayNode);

  // if (!this->GetMRMLScene()->IsNodePresent(
  //       this->InputVolumeRenderingDisplayNode))
  // {
  //   this->GetMRMLScene()->AddNode(this->InputVolumeRenderingDisplayNode);
  //   volumeNode->AddAndObserveDisplayNodeID(
  //     this->InputVolumeRenderingDisplayNode->GetID());
  // VolumeRenderingLogic->UpdateDisplayNodeFromVolumeNode(
  //   this->InputVolumeRenderingDisplayNode, volumeNode);
  // }

  this->AnnotationROINode = this->InputVolumeRenderingDisplayNode->GetROINode();
  this->WorkspaceGenerationNode->SetAndObserveAnnotationROINodeID(
    this->AnnotationROINode->GetID());

  return volumeNode;
}

//------------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerWorkspaceGenerationLogic::RenderVolume(
  vtkMRMLVolumeNode*                 volumeNode,
  vtkMRMLVolumeRenderingDisplayNode* volumeRenderingDisplayNode,
  vtkMRMLAnnotationROINode* annotationROINode, bool setPreset)
{
  qInfo() << Q_FUNC_INFO;

  if (VolumeRenderingLogic)
  {
    VolumeRenderingLogic->SetMRMLScene(this->GetMRMLScene());

    volumeRenderingDisplayNode =
      VolumeRenderingLogic->GetFirstVolumeRenderingDisplayNode(volumeNode);

    if (volumeRenderingDisplayNode == NULL)
    {
      // Volume Rendering will take place in new node.

      qCritical() << Q_FUNC_INFO << ": volumeRenderingDisplayNode Node is NULL";
      volumeRenderingDisplayNode =
        VolumeRenderingLogic->CreateDefaultVolumeRenderingNodes(volumeNode);
    }
    else
    {
      VolumeRenderingLogic->UpdateDisplayNodeFromVolumeNode(
        volumeRenderingDisplayNode, volumeNode);
    }

    if (setPreset)
    {
      // Get the current Volume Property Node.
      vtkMRMLVolumePropertyNode* volumePropertyNode =
        volumeRenderingDisplayNode->GetVolumePropertyNode();

      if (volumePropertyNode == NULL)
      {
        qCritical() << Q_FUNC_INFO << ": Volume property node is NULL";
        volumePropertyNode = vtkMRMLVolumePropertyNode::New();
      }

      // Copy the MRI preset to the volume property node
      volumePropertyNode->Copy(
        VolumeRenderingLogic->GetPresetByName("MR-Default"));

      volumePropertyNode->Delete();
    }
  }
  else
  {
    qCritical() << Q_FUNC_INFO
                << ": Volume Rendering Logic not found, returning.";

    return volumeNode;
  }

  // vtkSmartPointer< vtkMRMLVolumeRenderingDisplayNode > displayNode =
  //   vtkSmartPointer< vtkMRMLVolumeRenderingDisplayNode >::Take(
  //     this->InputVolumeRenderingDisplayNode);

  // if (!this->GetMRMLScene()->IsNodePresent(
  //       this->InputVolumeRenderingDisplayNode))
  // {
  //   this->GetMRMLScene()->AddNode(this->InputVolumeRenderingDisplayNode);
  //   volumeNode->AddAndObserveDisplayNodeID(
  //     this->InputVolumeRenderingDisplayNode->GetID());
  // VolumeRenderingLogic->UpdateDisplayNodeFromVolumeNode(
  //   this->InputVolumeRenderingDisplayNode, volumeNode);
  // }

  annotationROINode = volumeRenderingDisplayNode->GetROINode();
  if (annotationROINode == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": Annotation ROI Node is NULL";
  }

  return volumeNode;
}

/** ------------------------------- DEPRECATED ---------------------------------
//------------------------------------------------------------------------------
bool vtkSlicerWorkspaceGenerationLogic::LoadWorkspace(
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
  NeuroKinematics        neuro_kinematics(&probe);
  WorkspaceVisualization ws(neuro_kinematics);

  auto start = std::chrono::high_resolution_clock::now();
  vtkSmartPointer< vtkPoints > workspacePointCloud =
    vtkSmartPointer< vtkPoints >::New();
  Eigen::Matrix3Xf final_workspace = ws.GetGeneralWorkspace();
  auto checkpoint_workspace_gen    = std::chrono::high_resolution_clock::now();
  PointSetUtilities utils(final_workspace);
  workspacePointCloud          = utils.getVTKPointSet();
  auto checkpoint_vtk_pointset = std::chrono::high_resolution_clock::now();

  auto duration_workspace_gen =
    std::chrono::duration_cast< std::chrono::microseconds >(
      checkpoint_workspace_gen - start);

  auto duration_vtk_pointset =
    std::chrono::duration_cast< std::chrono::microseconds >(
      checkpoint_vtk_pointset - checkpoint_workspace_gen);

  qDebug() << Q_FUNC_INFO << ": Time taken to generate workspace = "
           << duration_workspace_gen.count();

  qDebug() << Q_FUNC_INFO << ": Time taken to cast Eigen Mat to pointset = "
           << duration_vtk_pointset.count();

  vtkSmartPointer< vtkPolyData > polyDataWorkspace =
    vtkSmartPointer< vtkPolyData >::New();
  polyDataWorkspace->SetPoints(workspacePointCloud.GetPointer());

  vtkSmartPointer< vtkMRMLModelNode > modelNode =
    vtkSmartPointer< vtkMRMLModelNode >::New();
  if (true)
  {
    // bug: Convert surface model to segmentation. @DhruvKoolRajamani
    vtkSmartPointer< vtkDelaunay3D > delaunay =
      vtkSmartPointer< vtkDelaunay3D >::New();
    delaunay->SetInputData(polyDataWorkspace);
    delaunay->SetAlpha(5);
    delaunay->SetTolerance(0.3);
    delaunay->SetOffset(5.0);
    delaunay->Update();

    vtkSmartPointer< vtkGeometryFilter > surfaceFilter =
      vtkSmartPointer< vtkGeometryFilter >::New();
    surfaceFilter->SetInputConnection(delaunay->GetOutputPort());
    surfaceFilter->Update();
    modelNode->SetAndObservePolyData(surfaceFilter->GetOutput());

    segmentationNode->AddSegmentFromClosedSurfaceRepresentation(
      modelNode->GetPolyData(), "workspace_segment");

    // auto segment =
    // vtkSlicerSegmentationsModuleLogic::CreateSegmentFromModelNode(
    //   modelNode, segmentationNode);
  }
  else
  {
    vtkSmartPointer< vtkGaussianSplatter > splatter =
      vtkSmartPointer< vtkGaussianSplatter >::New();
    splatter->SetInputData(polyDataWorkspace);
    splatter->Update();

    vtkSmartPointer< vtkMRMLLabelMapVolumeNode > labelMapVolumeNode =
      vtkSmartPointer< vtkMRMLLabelMapVolumeNode >::New();

    labelMapVolumeNode->SetAndObserveImageData(splatter->GetOutput());

    vtkIntArray* labelsArr = vtkIntArray::New();
    vtkSlicerSegmentationsModuleLogic::GetAllLabelValues(
      labelsArr, labelMapVolumeNode->GetImageData());

    QString   labels = "[";
    vtkIdType num    = labelsArr->GetNumberOfTuples();
    qDebug() << Q_FUNC_INFO << "Total lables: ";
    for (vtkIdType id = 0; id < num; ++id)
    {
      labels += labelsArr->GetValue(id) + ", ";
    }
    labels += "]";
    qDebug() << Q_FUNC_INFO << "Labelmap lables: " << labels;

    SegmentationsLogic->CreateSegmentFromLabelmapVolumeNode(labelMapVolumeNode,
                                                            segmentationNode);

    // vtkSmartPointer< vtkMRMLVolumeNode > volumeNode = labelMapVolumeNode;
    // if (volumeNode == NULL)
    // {
    //   qCritical() << Q_FUNC_INFO << ": Unable to create volume node";
    //   return;
    // }

    // vtkMRMLVolumeRenderingDisplayNode* volRenderingDisplayNode;
    // vtkMRMLAnnotationROINode*          annotationROINode;
    // volumeNode = this->RenderVolume(volumeNode, volRenderingDisplayNode,
    //                                 annotationROINode, false);
    // modelNode  = vtkMRMLModelNode::SafeDownCast(annotationROINode);

    // SegmentationsLogic->SetMRMLScene(this->GetMRMLScene());
    // SegmentationsLogic->AddSegmentFromClosedSurfaceRepresentation(model)
  }

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
  vtkSlicerWorkspaceGenerationLogic::getWorkspaceMeshSegmentationNode()
{
  qInfo() << Q_FUNC_INFO;

  if (!this->WorkspaceMeshSegmentationNode &&
      !WorkspaceGenerationNode->GetWorkspaceMeshSegmentationNode())
  {
    qCritical() << Q_FUNC_INFO << ": No workspace mesh model node available";
    return NULL;
  }

  if (this->WorkspaceMeshSegmentationNode !=
      this->WorkspaceGenerationNode->GetWorkspaceMeshSegmentationNode())
  {
    this->WorkspaceMeshSegmentationNode =
      this->WorkspaceGenerationNode->GetWorkspaceMeshSegmentationNode();
  }

  return this->WorkspaceMeshSegmentationNode;
}

//------------------------------------------------------------------------------
vtkMRMLSegmentationNode*
  vtkSlicerWorkspaceGenerationLogic::getBurrHoleSegmentationNode()
{
  qInfo() << Q_FUNC_INFO;

  if (!this->BurrHoleSegmentationNode &&
      !WorkspaceGenerationNode->GetBurrHoleSegmentationNode())
  {
    qCritical() << Q_FUNC_INFO << ": No burr hole segmentation node available";
    return NULL;
  }

  if (this->BurrHoleSegmentationNode !=
      this->WorkspaceGenerationNode->GetBurrHoleSegmentationNode())
  {
    this->BurrHoleSegmentationNode =
      this->WorkspaceGenerationNode->GetBurrHoleSegmentationNode();
  }

  return this->BurrHoleSegmentationNode;
}

//------------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode*
  vtkSlicerWorkspaceGenerationLogic::getCurrentInputVolumeRenderingDisplayNode()
{
  return this->InputVolumeRenderingDisplayNode;
}

//------------------------------------------------------------------------------
vtkMRMLSegmentationDisplayNode* vtkSlicerWorkspaceGenerationLogic::
  getCurrentWorkspaceMeshSegmentationDisplayNode()
{
  return this->WorkspaceMeshSegmentationDisplayNode;
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

  vtkMRMLSegmentationNode* workspaceMeshNode =
    this->WorkspaceGenerationNode->GetWorkspaceMeshSegmentationNode();
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
