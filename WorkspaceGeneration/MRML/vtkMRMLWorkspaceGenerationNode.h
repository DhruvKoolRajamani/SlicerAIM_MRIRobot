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

// .NAME vtkMRMLWorkspaceGenerationNode - MRML Node
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes

#ifndef __vtkMRMLWorkspaceGenerationNode_h
#define __vtkMRMLWorkspaceGenerationNode_h

// std includes
#include <iostream>
#include <list>

// vtk includes
#include <vtkCommand.h>
#include <vtkObject.h>
#include <vtkObjectBase.h>
#include <vtkObjectFactory.h>
#include <vtkVector.h>

// Volume MRML Node
#include <vtkMRMLVolumeNode.h>

// Annotation ROI Node
#include <vtkMRMLAnnotationROINode.h>

// Model Node
#include <vtkMRMLModelNode.h>

// Segmentation Node
#include <vtkMRMLSegmentationNode.h>

// Markups Fiducial Node
#include <vtkMRMLMarkupsFiducialDisplayNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>

// Slicer includes
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"

// WorkspaceGeneration includes
#include "vtkSlicerWorkspaceGenerationModuleMRMLExport.h"

class vtkMRMLModelNode;

class BurrHoleParameters
{
public:
  // Setters
  void setCenter(vtkVector3d center)
  {
    center = _center;
  }
  void setRadius(double radius)
  {
    _radius = radius;
  }
  void setDrillBit(vtkMRMLModelNode* drill_bit)
  {
    _drill_bit = drill_bit;
  }

  // Getters
  vtkVector3d getCenter() const
  {
    return _center;
  }
  double getRadius() const
  {
    return _radius;
  }
  vtkMRMLModelNode* getDrillBit() const
  {
    return _drill_bit;
  }

private:
  vtkVector3d       _center;
  double            _radius;
  vtkMRMLModelNode* _drill_bit;
};

class VTK_SLICER_WORKSPACEGENERATION_MODULE_MRML_EXPORT
  vtkMRMLWorkspaceGenerationNode : public vtkMRMLNode
{
public:
  enum Events
  {
    /// MarkupsPositionModifiedEvent is called when markup point positions are
    /// modified. This make it easier for logic or other classes to observe any
    /// changes in input data.
    // vtkCommand::UserEvent + 777 is just a random value that is very unlikely
    // to be used for anything else in this class
    MarkupsPositionModifiedEvent = vtkCommand::UserEvent + 777
  };

  vtkTypeMacro(vtkMRMLWorkspaceGenerationNode, vtkMRMLNode);

  // Standard MRML node methods
  static vtkMRMLWorkspaceGenerationNode* New();

  virtual vtkMRMLNode* CreateNodeInstance() VTK_OVERRIDE;
  virtual const char*  GetNodeTagName() VTK_OVERRIDE
  {
    return "WorkspaceGeneration";
  };
  void         PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;
  virtual void ReadXMLAttributes(const char** atts) VTK_OVERRIDE;
  virtual void WriteXML(ostream& of, int indent) VTK_OVERRIDE;
  virtual void Copy(vtkMRMLNode* node) VTK_OVERRIDE;

  vtkGetMacro(AutoUpdateOutput, bool);
  vtkSetMacro(AutoUpdateOutput, bool);

  vtkGetMacro(BurrHoleDetected, bool);
  vtkSetMacro(BurrHoleDetected, bool);

  vtkGetVector3Macro(BurrHoleCenter, double);
  vtkSetVector3Macro(BurrHoleCenter, double);

  vtkGetMacro(BurrHoleRadius, float);
  vtkSetMacro(BurrHoleRadius, float);

protected:
  // Constructor/destructor methods
  vtkMRMLWorkspaceGenerationNode();
  virtual ~vtkMRMLWorkspaceGenerationNode();
  vtkMRMLWorkspaceGenerationNode(const vtkMRMLWorkspaceGenerationNode&);
  void operator=(const vtkMRMLWorkspaceGenerationNode&);

public:
  void SetBurrHoleParameters(BurrHoleParameters burrHoleParams);
  void SetBurrHoleParameters(vtkVector3d center, double radius,
                             vtkMRMLModelNode* drill_bit = NULL);
  void SetBurrHoleParameters(double center[3], double radius,
                             vtkMRMLModelNode* drill_bit = NULL);

  void SetAndObserveInputVolumeNodeID(const char* inputNodeId);
  void SetAndObserveAnnotationROINodeID(const char* annotationROINodeId);
  void SetAndObserveWorkspaceMeshSegmentationNodeID(
    const char* workspaceMeshSegmentationNodeId);
  void SetAndObserveEntryPointNodeId(const char* entryPointNodeId);
  void SetAndObserveTargetPointNodeId(const char* targetPointNodeId);
  void ProcessMRMLEvents(vtkObject* caller, unsigned long event,
                         void* callData) VTK_OVERRIDE;

  vtkMRMLVolumeNode*          GetInputVolumeNode();
  vtkMRMLAnnotationROINode*   GetAnnotationROINode();
  vtkMRMLSegmentationNode*    GetWorkspaceMeshSegmentationNode();
  vtkMRMLMarkupsFiducialNode* GetEntryPointNode();
  vtkMRMLMarkupsFiducialNode* GetTargetPointNode();
  BurrHoleParameters          GetBurrHoleParams();

private:
  bool               AutoUpdateOutput;
  bool               BurrHoleDetected;
  double             BurrHoleCenter[3];
  float              BurrHoleRadius;
  BurrHoleParameters BurrHoleParams;

  // int InputNodeType;
};

#endif  // __vtkMRMLWorkspaceGenerationNode_h