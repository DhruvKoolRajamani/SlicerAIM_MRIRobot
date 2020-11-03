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

// .NAME vtkSlicerWorkspaceGenerationLogic - slicer logic class for volumes
// manipulation .SECTION Description This class manages the logic associated
// with reading, saving, and changing propertied of the volumes

#ifndef __vtkSlicerWorkspaceGenerationLogic_h
#define __vtkSlicerWorkspaceGenerationLogic_h

// QT Includes
#include <QString>

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// vtk includes
#include "vtkSmartPointer.h"
#include "vtkWeakPointer.h"
#include <vtkPolyData.h>

// MRML includes
#include "vtkMRMLWorkspaceGenerationNode.h"
#include <vtkMRMLModelNode.h>
#include <vtkMRMLVolumeNode.h>

// STD includes
#include <cstdlib>

// Utilities includes

#include "vtkSlicerWorkspaceGenerationModuleLogicExport.h"

class vtkMRMLWorkspaceGenerationNode;
class vtkMRMLModelNode;
class vtkPolyData;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_WORKSPACEGENERATION_MODULE_LOGIC_EXPORT
  vtkSlicerWorkspaceGenerationLogic : public vtkSlicerModuleLogic
{
public:
  static vtkSlicerWorkspaceGenerationLogic* New();
  vtkTypeMacro(vtkSlicerWorkspaceGenerationLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event,
                              void* callData) VTK_OVERRIDE;

  vtkMRMLVolumeNode* RenderVolume(vtkMRMLVolumeNode* volumeNode);

  // Updates the mouse selection type to create markups or to navigate the
  // scene.
  void UpdateSelectionNode(vtkMRMLWorkspaceGenerationNode* moduleNode);

  // Updates output model from file?
  void UpdateOutputModel(vtkMRMLWorkspaceGenerationNode* moduleNode);

  // Load workspace mesh
  void LoadWorkspace(QString workspaceMeshFilePath);

  // Get the points store in a vtkMRMLModelNode
  static void ModelToPoints(vtkMRMLModelNode* modelNode,
                            vtkPoints* outputPoints);

protected:
  vtkSlicerWorkspaceGenerationLogic();
  virtual ~vtkSlicerWorkspaceGenerationLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the
  /// MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneEndImport() VTK_OVERRIDE;
  virtual void OnMRMLSceneStartImport() VTK_OVERRIDE;
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node) VTK_OVERRIDE;
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) VTK_OVERRIDE;

  bool* isInputModelNode;
  vtkWeakPointer< vtkMRMLWorkspaceGenerationNode > WorkspaceGenerationNode;
  vtkWeakPointer< vtkMRMLModelNode > InputModelNode;
  vtkWeakPointer< vtkMRMLVolumeNode > InputVolumeNode;
  vtkWeakPointer< vtkMRMLModelNode > OutputModelNode;
  vtkWeakPointer< vtkMRMLDisplayNode > DisplayNode;

private:
  static void AssignPolyDataToOutput(vtkMRMLWorkspaceGenerationNode* moduleNode,
                                     vtkPolyData* polyData);

  vtkSlicerWorkspaceGenerationLogic(
    const vtkSlicerWorkspaceGenerationLogic&);               // Not implemented
  void operator=(const vtkSlicerWorkspaceGenerationLogic&);  // Not implemented
};

#endif
