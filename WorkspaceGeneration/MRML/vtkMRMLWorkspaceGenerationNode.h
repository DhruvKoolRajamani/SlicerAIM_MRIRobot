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

// Slicer includes
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"

// WorkspaceGeneration includes
#include "vtkSlicerWorkspaceGenerationModuleMRMLExport.h"

class vtkMRMLModelNode;

class
VTK_SLICER_WORKSPACEGENERATION_MODULE_MRML_EXPORT
vtkMRMLWorkspaceGenerationNode
: public vtkMRMLNode
{
public:

  enum Events
  {
    /// MarkupsPositionModifiedEvent is called when markup point positions are modified.
    /// This make it easier for logic or other classes to observe any changes in input data.
    // vtkCommand::UserEvent + 777 is just a random value that is very unlikely to be used for anything else in this class
    MarkupsPositionModifiedEvent = vtkCommand::UserEvent + 777
  };

  vtkTypeMacro( vtkMRMLWorkspaceGenerationNode, vtkMRMLNode );
  
  // Standard MRML node methods  
  static vtkMRMLWorkspaceGenerationNode *New();  

  virtual vtkMRMLNode* CreateNodeInstance() VTK_OVERRIDE;
  virtual const char* GetNodeTagName() VTK_OVERRIDE { return "WorkspaceGeneration"; };
  void PrintSelf( ostream& os, vtkIndent indent ) VTK_OVERRIDE;
  virtual void ReadXMLAttributes( const char** atts ) VTK_OVERRIDE;
  virtual void WriteXML( ostream& of, int indent ) VTK_OVERRIDE;
  virtual void Copy( vtkMRMLNode *node ) VTK_OVERRIDE;
  
  vtkGetMacro( AutoUpdateOutput, bool );
  vtkSetMacro( AutoUpdateOutput, bool );

protected:

  // Constructor/destructor methods
  vtkMRMLWorkspaceGenerationNode();
  virtual ~vtkMRMLWorkspaceGenerationNode();
  vtkMRMLWorkspaceGenerationNode ( const vtkMRMLWorkspaceGenerationNode& );
  void operator=( const vtkMRMLWorkspaceGenerationNode& );
  
public:

  void SetAndObserveInputNodeID( const char* inputNodeId );
  void SetAndObserveOutputModelNodeID( const char* outputModelNodeId );
  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData ) VTK_OVERRIDE;

  vtkMRMLNode * GetInputNode( );
  vtkMRMLModelNode* GetOutputModelNode( );

  // DEPRECATED - Get the output node
  vtkMRMLModelNode* GetModelNode( );

  // DEPRECATED - Set the output node
  void SetAndObserveModelNodeID( const char* id );

private:

  bool   AutoUpdateOutput;
};


#endif // __vtkMRMLWorkspaceGenerationNode_h