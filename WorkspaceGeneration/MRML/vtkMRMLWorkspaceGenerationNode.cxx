#include <QDebug>

// WorkspaceGeneration includes
#include "vtkMRMLWorkspaceGenerationNode.h"

// slicer includes
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelNode.h"

// Other MRML includes
#include "vtkMRMLNode.h"

// VTK includes
#include <vtkNew.h>

// Other includes
#include <iostream>
#include <sstream>
#include <string.h>

static const char* INPUT_ROLE = "InputVolume";
static const char* ROI_ROLE   = "ROI";
static const char* WORKSPACEMESH_SEGMENTATION_ROLE =
  "WorkspaceMeshSegmentation";
static const char* EP_WORKSPACEMESH_SEGMENTATION_ROLE =
  "EntryPointWorkspaceMeshSegmentation";
static const char* SUB_WORKSPACEMESH_SEGMENTATION_ROLE =
  "SubWorkspaceMeshSegmentation";
static const char* BURRHOLE_SEGMENTATION_ROLE = "BurrHoleSegmentation";
static const char* BH_EXTREME_POINT_ROLE      = "BHExtremePoint";
static const char* ENTRY_POINT_ROLE           = "EntryPoint";
static const char* TARGET_POINT_ROLE          = "TargetPoint";

vtkMRMLNodeNewMacro(vtkMRMLWorkspaceGenerationNode);

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::SetBurrHoleParameters(
  BurrHoleParameters burrHoleParams)
{
  this->BurrHoleParams = burrHoleParams;
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::SetBurrHoleParameters(
  vtkVector3d center, double radius, vtkMRMLModelNode* drill_bit)
{
  this->BurrHoleParams = BurrHoleParameters();
  this->BurrHoleParams.setCenter(center);
  this->BurrHoleParams.setRadius(radius);
  this->BurrHoleParams.setDrillBit(drill_bit);
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::SetBurrHoleParameters(
  double center[], double radius, vtkMRMLModelNode* drill_bit)
{
  this->BurrHoleParams = BurrHoleParameters();
  this->BurrHoleParams.setCenter(vtkVector3d(center[0], center[1], center[2]));
  this->BurrHoleParams.setRadius(radius);
  this->BurrHoleParams.setDrillBit(drill_bit);
}

//-----------------------------------------------------------------
vtkMRMLWorkspaceGenerationNode::vtkMRMLWorkspaceGenerationNode()
{
  qInfo() << Q_FUNC_INFO;

  this->HideFromEditorsOff();
  this->SetSaveWithScene(true);

  vtkNew< vtkIntArray > inputVolumeEvents;
  inputVolumeEvents->InsertNextValue(vtkCommand::ModifiedEvent);

  vtkNew< vtkIntArray > workspaceMeshEvents;
  workspaceMeshEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  workspaceMeshEvents->InsertNextValue(vtkMRMLModelNode::MeshModifiedEvent);

  vtkNew< vtkIntArray > ePWorkspaceMeshEvents;
  ePWorkspaceMeshEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  ePWorkspaceMeshEvents->InsertNextValue(vtkMRMLModelNode::MeshModifiedEvent);

  vtkNew< vtkIntArray > subWorkspaceMeshEvents;
  subWorkspaceMeshEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  subWorkspaceMeshEvents->InsertNextValue(vtkMRMLModelNode::MeshModifiedEvent);

  vtkNew< vtkIntArray > burrHoleSegEvents;
  burrHoleSegEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  burrHoleSegEvents->InsertNextValue(vtkMRMLModelNode::MeshModifiedEvent);

  vtkNew< vtkIntArray > bHExtremePointMarkupEvents;
  bHExtremePointMarkupEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  bHExtremePointMarkupEvents->InsertNextValue(
    vtkMRMLMarkupsNode::PointAddedEvent);
  bHExtremePointMarkupEvents->InsertNextValue(
    vtkMRMLMarkupsNode::PointRemovedEvent);
  bHExtremePointMarkupEvents->InsertNextValue(
    vtkMRMLMarkupsNode::PointModifiedEvent);

  vtkNew< vtkIntArray > entryPointMarkupEvents;
  entryPointMarkupEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  entryPointMarkupEvents->InsertNextValue(vtkMRMLMarkupsNode::PointAddedEvent);
  entryPointMarkupEvents->InsertNextValue(
    vtkMRMLMarkupsNode::PointRemovedEvent);
  entryPointMarkupEvents->InsertNextValue(
    vtkMRMLMarkupsNode::PointModifiedEvent);

  vtkNew< vtkIntArray > targetPointMarkupEvents;
  targetPointMarkupEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  targetPointMarkupEvents->InsertNextValue(vtkMRMLMarkupsNode::PointAddedEvent);
  targetPointMarkupEvents->InsertNextValue(
    vtkMRMLMarkupsNode::PointRemovedEvent);
  targetPointMarkupEvents->InsertNextValue(
    vtkMRMLMarkupsNode::PointModifiedEvent);

  // inputVolumeEvents->InsertNextValue(vtkMRMLModelNode::MeshModifiedEvent);

  this->AddNodeReferenceRole(INPUT_ROLE, NULL, inputVolumeEvents.GetPointer());
  this->AddNodeReferenceRole(ROI_ROLE);
  this->AddNodeReferenceRole(WORKSPACEMESH_SEGMENTATION_ROLE, NULL,
                             workspaceMeshEvents.GetPointer());
  this->AddNodeReferenceRole(EP_WORKSPACEMESH_SEGMENTATION_ROLE, NULL,
                             ePWorkspaceMeshEvents.GetPointer());
  this->AddNodeReferenceRole(SUB_WORKSPACEMESH_SEGMENTATION_ROLE, NULL,
                             subWorkspaceMeshEvents.GetPointer());
  this->AddNodeReferenceRole(BURRHOLE_SEGMENTATION_ROLE, NULL,
                             burrHoleSegEvents.GetPointer());
  this->AddNodeReferenceRole(BH_EXTREME_POINT_ROLE, NULL,
                             bHExtremePointMarkupEvents.GetPointer());
  this->AddNodeReferenceRole(ENTRY_POINT_ROLE, NULL,
                             entryPointMarkupEvents.GetPointer());
  this->AddNodeReferenceRole(TARGET_POINT_ROLE, NULL,
                             targetPointMarkupEvents.GetPointer());

  this->AutoUpdateOutput = true;
  this->BurrHoleDetected = false;
  double center[3]       = {0.0, 0.0, 0.0};
  this->BurrHoleRadius   = 1.0;

  std::copy(this->BurrHoleCenter, this->BurrHoleCenter + 3, center);
  this->SetBurrHoleParameters(vtkVector3d(this->BurrHoleCenter),
                              this->BurrHoleRadius);
  // this->InputNodeType = NONE;
}

//-----------------------------------------------------------------
vtkMRMLWorkspaceGenerationNode::~vtkMRMLWorkspaceGenerationNode()
{
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::WriteXML(ostream& of, int nIndent)
{
  qInfo() << Q_FUNC_INFO;

  Superclass::WriteXML(of, nIndent);  // This will take care of referenced nodes
  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLBooleanMacro(AutoUpdateOutput, AutoUpdateOutput);
  vtkMRMLWriteXMLBooleanMacro(BurrHoleDetected, BurrHoleDetected);
  vtkMRMLWriteXMLVectorMacro(BurrHoleCenter, BurrHoleCenter, double, 3);
  vtkMRMLWriteXMLFloatMacro(BurrHoleRadius, BurrHoleRadius);
  // vtkMRMLWriteXMLIntMacro(InputNodeType, InputNodeType);
  vtkMRMLWriteXMLEndMacro();
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::ReadXMLAttributes(const char** atts)
{
  qInfo() << Q_FUNC_INFO;

  int disabledModify = this->StartModify();
  Superclass::ReadXMLAttributes(atts);  // This will take care of referenced
                                        // nodes
  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLBooleanMacro(AutoUpdateOutput, AutoUpdateOutput);
  vtkMRMLReadXMLBooleanMacro(BurrHoleDetected, BurrHoleDetected);
  vtkMRMLReadXMLVectorMacro(BurrHoleCenter, BurrHoleCenter, double, 3);
  vtkMRMLReadXMLFloatMacro(BurrHoleRadius, BurrHoleRadius);
  // vtkMRMLReadXMLBooleanMacro(InputNodeType, InputNodeType);
  vtkMRMLReadXMLEndMacro();
  this->EndModify(disabledModify);
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::Copy(vtkMRMLNode* anode)
{
  qInfo() << Q_FUNC_INFO;

  int disabledModify = this->StartModify();
  Superclass::Copy(anode);  // This will take care of referenced nodes
  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyBooleanMacro(AutoUpdateOutput);
  vtkMRMLCopyBooleanMacro(BurrHoleDetected);
  vtkMRMLCopyVectorMacro(BurrHoleCenter, double, 3);
  vtkMRMLCopyFloatMacro(BurrHoleRadius);
  // vtkMRMLCopyBooleanMacro(InputNodeType);
  vtkMRMLCopyEndMacro();
  this->EndModify(disabledModify);
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::PrintSelf(ostream& os, vtkIndent indent)
{
  qInfo() << Q_FUNC_INFO;

  Superclass::PrintSelf(os, indent);  // This will take care of referenced nodes
  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintBooleanMacro(AutoUpdateOutput);
  vtkMRMLPrintBooleanMacro(BurrHoleDetected);
  vtkMRMLPrintVectorMacro(BurrHoleCenter, double, 3);
  vtkMRMLPrintFloatMacro(BurrHoleRadius);
  // vtkMRMLPrintBooleanMacro(InputNodeType);
  vtkMRMLPrintEndMacro();
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::ProcessMRMLEvents(vtkObject* caller,
                                                       unsigned long /*event*/,
                                                       void* /*callData*/)
{
  vtkMRMLMarkupsFiducialNode* callerNode =
    vtkMRMLMarkupsFiducialNode::SafeDownCast(caller);
  if (callerNode == NULL)
    return;

  if ((this->GetEntryPointNode() && this->GetEntryPointNode() == caller) ||
      (this->GetTargetPointNode() && this->GetTargetPointNode() == caller))
  {
    this->InvokeCustomModifiedEvent(MarkupsPositionModifiedEvent);
  }
}

//-----------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLWorkspaceGenerationNode::GetInputVolumeNode()
{
  qInfo() << Q_FUNC_INFO;

  vtkMRMLVolumeNode* inputVolumeNode =
    vtkMRMLVolumeNode::SafeDownCast(this->GetNodeReference(INPUT_ROLE));

  if (!inputVolumeNode)
  {
    qWarning() << Q_FUNC_INFO << ": input volume node is null";
  }

  return inputVolumeNode;
}

//-----------------------------------------------------------------
vtkMRMLAnnotationROINode* vtkMRMLWorkspaceGenerationNode::GetAnnotationROINode()
{
  qInfo() << Q_FUNC_INFO;

  vtkMRMLAnnotationROINode* annotationROINode =
    vtkMRMLAnnotationROINode::SafeDownCast(this->GetNodeReference(ROI_ROLE));

  if (!annotationROINode)
  {
    qWarning() << Q_FUNC_INFO << ": annotationROI node is null";
    return NULL;
  }

  return annotationROINode;
}

//-----------------------------------------------------------------
vtkMRMLSegmentationNode*
  vtkMRMLWorkspaceGenerationNode::GetWorkspaceMeshSegmentationNode()
{
  qInfo() << Q_FUNC_INFO;

  vtkMRMLSegmentationNode* workspaceMeshSegmentationNode =
    vtkMRMLSegmentationNode::SafeDownCast(
      this->GetNodeReference(WORKSPACEMESH_SEGMENTATION_ROLE));

  if (!workspaceMeshSegmentationNode)
  {
    qWarning() << Q_FUNC_INFO << ": workspaceMeshSegmentationNode node is null";
    return NULL;
  }

  return workspaceMeshSegmentationNode;
}

//-----------------------------------------------------------------
vtkMRMLSegmentationNode*
  vtkMRMLWorkspaceGenerationNode::GetEPWorkspaceMeshSegmentationNode()
{
  qInfo() << Q_FUNC_INFO;

  vtkMRMLSegmentationNode* ePWorkspaceMeshSegmentationNode =
    vtkMRMLSegmentationNode::SafeDownCast(
      this->GetNodeReference(EP_WORKSPACEMESH_SEGMENTATION_ROLE));

  if (!ePWorkspaceMeshSegmentationNode)
  {
    qWarning() << Q_FUNC_INFO
               << ": ePWorkspaceMeshSegmentationNode node is null";
    return NULL;
  }

  return ePWorkspaceMeshSegmentationNode;
}

//-----------------------------------------------------------------
vtkMRMLSegmentationNode*
  vtkMRMLWorkspaceGenerationNode::GetSubWorkspaceMeshSegmentationNode()
{
  qInfo() << Q_FUNC_INFO;

  vtkMRMLSegmentationNode* subWorkspaceMeshSegmentationNode =
    vtkMRMLSegmentationNode::SafeDownCast(
      this->GetNodeReference(SUB_WORKSPACEMESH_SEGMENTATION_ROLE));

  if (!subWorkspaceMeshSegmentationNode)
  {
    qWarning() << Q_FUNC_INFO
               << ": subWorkspaceMeshSegmentationNode node is null";
    return NULL;
  }

  return subWorkspaceMeshSegmentationNode;
}

//-----------------------------------------------------------------
vtkMRMLSegmentationNode*
  vtkMRMLWorkspaceGenerationNode::GetBurrHoleSegmentationNode()
{
  qInfo() << Q_FUNC_INFO;

  vtkMRMLSegmentationNode* burrHoleSegmentationNode =
    vtkMRMLSegmentationNode::SafeDownCast(
      this->GetNodeReference(BURRHOLE_SEGMENTATION_ROLE));

  if (!burrHoleSegmentationNode)
  {
    qWarning() << Q_FUNC_INFO << ": burrHoleSegmentationNode node is null";
    return NULL;
  }

  return burrHoleSegmentationNode;
}

//-----------------------------------------------------------------
vtkMRMLMarkupsFiducialNode*
  vtkMRMLWorkspaceGenerationNode::GetBHExtremePointNode()
{
  qInfo() << Q_FUNC_INFO;

  vtkMRMLMarkupsFiducialNode* bHExtremePointNode =
    vtkMRMLMarkupsFiducialNode::SafeDownCast(
      this->GetNodeReference(BH_EXTREME_POINT_ROLE));

  if (!bHExtremePointNode)
  {
    qWarning() << Q_FUNC_INFO << ": bHExtremePointNode node is null";
    return NULL;
  }

  return bHExtremePointNode;
}

//-----------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkMRMLWorkspaceGenerationNode::GetEntryPointNode()
{
  qInfo() << Q_FUNC_INFO;

  vtkMRMLMarkupsFiducialNode* entryPointNode =
    vtkMRMLMarkupsFiducialNode::SafeDownCast(
      this->GetNodeReference(ENTRY_POINT_ROLE));

  if (!entryPointNode)
  {
    qWarning() << Q_FUNC_INFO << ": entryPointNode node is null";
    return NULL;
  }

  return entryPointNode;
}

//-----------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkMRMLWorkspaceGenerationNode::GetTargetPointNode()
{
  qInfo() << Q_FUNC_INFO;

  vtkMRMLMarkupsFiducialNode* targetPointNode =
    vtkMRMLMarkupsFiducialNode::SafeDownCast(
      this->GetNodeReference(TARGET_POINT_ROLE));

  if (!targetPointNode)
  {
    qWarning() << Q_FUNC_INFO << ": targetPointNode node is null";
    return NULL;
  }

  return targetPointNode;
}

//-----------------------------------------------------------------
BurrHoleParameters vtkMRMLWorkspaceGenerationNode::GetBurrHoleParams()
{
  return this->BurrHoleParams;
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::SetAndObserveInputVolumeNodeID(
  const char* inputId)
{
  qInfo() << Q_FUNC_INFO;

  if (inputId == NULL)
  {
    vtkErrorMacro("Input node cannot be null.");
    return;
  }

  this->SetAndObserveNodeReferenceID(INPUT_ROLE, inputId);
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::
  SetAndObserveWorkspaceMeshSegmentationNodeID(
    const char* workspaceMeshSegmentationNodeId)
{
  qInfo() << Q_FUNC_INFO;

  // error check
  const char* ePWorkspaceMeshSegmentationNodeId =
    this->GetNodeReferenceID(EP_WORKSPACEMESH_SEGMENTATION_ROLE);
  const char* subWorkspaceMeshSegmentationNodeId =
    this->GetNodeReferenceID(SUB_WORKSPACEMESH_SEGMENTATION_ROLE);
  const char* roiId = this->GetNodeReferenceID(ROI_ROLE);
  if (workspaceMeshSegmentationNodeId != NULL &&
      ePWorkspaceMeshSegmentationNodeId != NULL &&
      subWorkspaceMeshSegmentationNodeId != NULL && roiId != NULL &&
      strcmp(workspaceMeshSegmentationNodeId,
             ePWorkspaceMeshSegmentationNodeId) == 0 &&
      strcmp(workspaceMeshSegmentationNodeId,
             subWorkspaceMeshSegmentationNodeId) == 0 &&
      strcmp(workspaceMeshSegmentationNodeId, roiId) == 0)
  {
    vtkErrorMacro("Workspace Mesh node cannot match any other node");
    return;
  }

  this->SetAndObserveNodeReferenceID(WORKSPACEMESH_SEGMENTATION_ROLE,
                                     workspaceMeshSegmentationNodeId);
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::
  SetAndObserveEPWorkspaceMeshSegmentationNodeID(
    const char* ePWorkspaceMeshSegmentationNodeId)
{
  qInfo() << Q_FUNC_INFO;

  // error check
  const char* workspaceMeshSegmentationNodeId =
    this->GetNodeReferenceID(WORKSPACEMESH_SEGMENTATION_ROLE);
  const char* subWorkspaceMeshSegmentationNodeId =
    this->GetNodeReferenceID(SUB_WORKSPACEMESH_SEGMENTATION_ROLE);
  const char* roiId = this->GetNodeReferenceID(ROI_ROLE);
  if (ePWorkspaceMeshSegmentationNodeId != NULL &&
      workspaceMeshSegmentationNodeId != NULL &&
      subWorkspaceMeshSegmentationNodeId != NULL && roiId != NULL &&
      strcmp(ePWorkspaceMeshSegmentationNodeId,
             workspaceMeshSegmentationNodeId) == 0 &&
      strcmp(ePWorkspaceMeshSegmentationNodeId,
             subWorkspaceMeshSegmentationNodeId) == 0 &&
      strcmp(ePWorkspaceMeshSegmentationNodeId, roiId) == 0)
  {
    vtkErrorMacro("EP Workspace Mesh node cannot match any other node");
    return;
  }

  this->SetAndObserveNodeReferenceID(EP_WORKSPACEMESH_SEGMENTATION_ROLE,
                                     ePWorkspaceMeshSegmentationNodeId);
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::
  SetAndObserveSubWorkspaceMeshSegmentationNodeID(
    const char* subWorkspaceMeshSegmentationNodeId)
{
  qInfo() << Q_FUNC_INFO;

  // error check
  const char* workspaceMeshSegmentationNodeId =
    this->GetNodeReferenceID(WORKSPACEMESH_SEGMENTATION_ROLE);
  const char* ePWorkspaceMeshSegmentationNodeId =
    this->GetNodeReferenceID(SUB_WORKSPACEMESH_SEGMENTATION_ROLE);
  const char* roiId = this->GetNodeReferenceID(ROI_ROLE);
  if (subWorkspaceMeshSegmentationNodeId != NULL &&
      workspaceMeshSegmentationNodeId != NULL &&
      ePWorkspaceMeshSegmentationNodeId != NULL && roiId != NULL &&
      strcmp(subWorkspaceMeshSegmentationNodeId,
             workspaceMeshSegmentationNodeId) == 0 &&
      strcmp(subWorkspaceMeshSegmentationNodeId,
             ePWorkspaceMeshSegmentationNodeId) == 0 &&
      strcmp(subWorkspaceMeshSegmentationNodeId, roiId) == 0)
  {
    vtkErrorMacro("Sub Workspace Mesh node cannot match any other node");
    return;
  }

  this->SetAndObserveNodeReferenceID(SUB_WORKSPACEMESH_SEGMENTATION_ROLE,
                                     subWorkspaceMeshSegmentationNodeId);
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::SetAndObserveAnnotationROINodeID(
  const char* annotationROIId)
{
  qInfo() << Q_FUNC_INFO;

  // error check
  const char* workspaceMeshSegmentationNodeId =
    this->GetNodeReferenceID(WORKSPACEMESH_SEGMENTATION_ROLE);
  if (workspaceMeshSegmentationNodeId != NULL && annotationROIId != NULL &&
      strcmp(annotationROIId, workspaceMeshSegmentationNodeId) == 0)
  {
    vtkErrorMacro("Workspace Mesh node and annotation node cannot be null.");
    return;
  }

  this->SetAndObserveNodeReferenceID(ROI_ROLE, annotationROIId);
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::SetAndObserveBurrHoleSegmentationNodeID(
  const char* burrHoleSegmentationNodeId)
{
  qInfo() << Q_FUNC_INFO;

  // error check
  const char* workspaceMeshSegmentationNodeId =
    this->GetNodeReferenceID(WORKSPACEMESH_SEGMENTATION_ROLE);
  if (burrHoleSegmentationNodeId != NULL &&
      workspaceMeshSegmentationNodeId != NULL &&
      strcmp(burrHoleSegmentationNodeId, workspaceMeshSegmentationNodeId) == 0)
  {
    vtkErrorMacro(
      "Burr Hole Segmentation node and Workspace Mesh Segmentation Node cannot "
      "be same.");
    return;
  }

  this->SetAndObserveNodeReferenceID(BURRHOLE_SEGMENTATION_ROLE,
                                     burrHoleSegmentationNodeId);
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::SetAndObserveBHExtremePointNodeId(
  const char* bHExtremePointNodeId)
{
  qInfo() << Q_FUNC_INFO;

  if (bHExtremePointNodeId == NULL)
  {
    vtkErrorMacro("Burr hole point node id cannot be null.");
    return;
  }

  const char* targetPointNodeId = this->GetNodeReferenceID(TARGET_POINT_ROLE);
  const char* entryPointNodeId  = this->GetNodeReferenceID(ENTRY_POINT_ROLE);
  if ((targetPointNodeId != NULL &&
       strcmp(bHExtremePointNodeId, targetPointNodeId) == 0) &&
      (entryPointNodeId != NULL &&
       strcmp(bHExtremePointNodeId, entryPointNodeId) == 0))
  {
    vtkErrorMacro(
      "Extreme point cannot be the same as Entry point or target point.");
    return;
  }

  this->SetAndObserveNodeReferenceID(BH_EXTREME_POINT_ROLE,
                                     bHExtremePointNodeId);
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::SetAndObserveEntryPointNodeId(
  const char* entryPointNodeId)
{
  qInfo() << Q_FUNC_INFO;

  // error check
  const char* targetPointNodeId = this->GetNodeReferenceID(TARGET_POINT_ROLE);
  if (entryPointNodeId == NULL)
  {
    vtkErrorMacro("Entry point node id cannot be null.");
    return;
  }

  if (targetPointNodeId != NULL &&
      strcmp(targetPointNodeId, entryPointNodeId) == 0)
  {
    vtkErrorMacro("Entry point and target point cannot be the same.");
    return;
  }

  this->SetAndObserveNodeReferenceID(ENTRY_POINT_ROLE, entryPointNodeId);
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::SetAndObserveTargetPointNodeId(
  const char* targetPointNodeId)
{
  qInfo() << Q_FUNC_INFO;

  // error check
  const char* entryPointNodeId = this->GetNodeReferenceID(ENTRY_POINT_ROLE);
  if (targetPointNodeId == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": Target point node id cannot be null.";
    return;
  }

  if (entryPointNodeId != NULL &&
      strcmp(targetPointNodeId, entryPointNodeId) == 0)
  {
    qCritical() << Q_FUNC_INFO
                << ": Entry point and target point cannot be the same.";
    return;
  }

  this->SetAndObserveNodeReferenceID(TARGET_POINT_ROLE, targetPointNodeId);
}