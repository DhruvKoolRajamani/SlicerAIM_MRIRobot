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
static const char* ROI_ROLE = "AnnotationROI";

vtkMRMLNodeNewMacro(vtkMRMLWorkspaceGenerationNode);

//-----------------------------------------------------------------
vtkMRMLWorkspaceGenerationNode::vtkMRMLWorkspaceGenerationNode()
{
  qInfo() << Q_FUNC_INFO;

  this->HideFromEditorsOff();
  this->SetSaveWithScene(true);

  vtkNew< vtkIntArray > events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLModelNode::MeshModifiedEvent);

  this->AddNodeReferenceRole(INPUT_ROLE, NULL, events.GetPointer());
  this->AddNodeReferenceRole(ROI_ROLE);

  // this->AutoUpdateOutput = true;
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
  // vtkMRMLWriteXMLBooleanMacro(AutoUpdateOutput, AutoUpdateOutput);
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
  // vtkMRMLReadXMLBooleanMacro(AutoUpdateOutput, AutoUpdateOutput);
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
  // vtkMRMLCopyBooleanMacro(AutoUpdateOutput);
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
  // vtkMRMLPrintBooleanMacro(AutoUpdateOutput);
  // vtkMRMLPrintBooleanMacro(InputNodeType);
  vtkMRMLPrintEndMacro();
}

//-----------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLWorkspaceGenerationNode::GetInputVolumeNode()
{
  qInfo() << Q_FUNC_INFO;

  vtkMRMLVolumeNode* inputVolumeNode =
    vtkMRMLVolumeNode::SafeDownCast(this->GetNodeReference(INPUT_ROLE));
  return inputVolumeNode;
}

//-----------------------------------------------------------------
vtkMRMLAnnotationROINode* vtkMRMLWorkspaceGenerationNode::GetAnnotationROINode()
{
  qInfo() << Q_FUNC_INFO;

  vtkMRMLAnnotationROINode* annotationROINode =
    vtkMRMLAnnotationROINode::SafeDownCast(this->GetNodeReference(ROI_ROLE));
  return annotationROINode;
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::SetAndObserveInputVolumeNodeID(
  const char* inputId)
{
  qInfo() << Q_FUNC_INFO;

  // error check
  const char* roiId = this->GetNodeReferenceID(ROI_ROLE);
  if (inputId != NULL && roiId != NULL && strcmp(inputId, roiId) == 0)
  {
    vtkErrorMacro("Input node cannot be null.");
    return;
  }

  this->SetAndObserveNodeReferenceID(INPUT_ROLE, inputId);
}

//-----------------------------------------------------------------
void vtkMRMLWorkspaceGenerationNode::SetAndObserveAnnotationROINodeID(
  const char* annotationROIId)
{
  qInfo() << Q_FUNC_INFO;

  // error check
  const char* inputId = this->GetNodeReferenceID(INPUT_ROLE);
  if (inputId != NULL && annotationROIId != NULL &&
      strcmp(annotationROIId, inputId) == 0)
  {
    vtkErrorMacro("Input node and annotation node cannot be null.");
    return;
  }

  this->SetAndObserveNodeReferenceID(ROI_ROLE, annotationROIId);
}