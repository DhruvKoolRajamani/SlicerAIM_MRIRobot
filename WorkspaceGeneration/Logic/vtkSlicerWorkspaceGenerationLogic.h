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

// Volume Rendering Logic includes
#include <vtkSlicerVolumeRenderingLogic.h>
#include <vtkSlicerVolumeRenderingModuleLogicExport.h>

// Models logic includes
#include <vtkSlicerModelsLogic.h>
#include <vtkSlicerModelsModuleLogicExport.h>

// Markups Logic includes
#include <vtkSlicerMarkupsLogic.h>
#include <vtkSlicerMarkupsModuleLogicExport.h>

// Segmentations Logic includes
#include <vtkSlicerSegmentationsModuleLogic.h>
#include <vtkSlicerSegmentationsModuleLogicExport.h>

// Volume Rendering Display Node
#include <vtkMRMLVolumeRenderingDisplayNode.h>

// Segmentation Display Node
#include <vtkMRMLSegmentationDisplayNode.h>

// Annotation ROI Node
#include <vtkMRMLAnnotationROINode.h>

// Slicer Module includes
#include <qSlicerAbstractModule.h>
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>

// vtk includes
#include "vtkMatrix4x4.h"
#include "vtkSmartPointer.h"
#include "vtkWeakPointer.h"
#include <vtkPolyData.h>

// MRML includes
#include "vtkMRMLWorkspaceGenerationNode.h"
#include <vtkMRMLModelNode.h>
#include <vtkMRMLVolumeNode.h>

// STD includes
#include <cstdlib>

// Eigen includes
#include <eigen3/Eigen/Core>

// Neurorobot includes
#include "NeuroKinematics/ForwardKinematics.h"

// Isosurface creation
#include <vtkContourFilter.h>
#include <vtkExtractVOI.h>
#include <vtkMarchingCubes.h>
#include <vtkPolyDataMapper.h>
#include <vtkStripper.h>

#include "vtkSlicerWorkspaceGenerationModuleLogicExport.h"

class vtkMRMLWorkspaceGenerationNode;
class vtkMRMLSegmentationNode;
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
  void UpdateVolumeRendering();

  // Update Markup Fiducial nodes for entry point and target point
  void UpdateMarkupFiducialNodes();

  // Update the subworkspace
  void UpdateSubWorkspace(vtkMRMLWorkspaceGenerationNode*, bool);

  // Identify the Burr Hole
  bool IdentifyBurrHole(vtkMRMLWorkspaceGenerationNode*);

  // Load workspace mesh
  bool LoadWorkspace(QString workspaceMeshFilePath);

  // Convert vtkMatrix to eigen Matrix
  static Eigen::Matrix4d convertToEigenMatrix(vtkMatrix4x4* vtkMat);

  // Generate Workspace
  void GenerateWorkspace(vtkMRMLSegmentationNode* segmentationNode, Probe probe,
                         vtkMatrix4x4* registrationMatrix);

  // Getters
  vtkSlicerVolumeRenderingLogic* getVolumeRenderingLogic();
  qSlicerAbstractCoreModule*     getVolumeRenderingModule();
  vtkMRMLSegmentationNode*       getWorkspaceMeshSegmentationNode();
  vtkMRMLVolumeRenderingDisplayNode*
    getCurrentInputVolumeRenderingDisplayNode();
  vtkMRMLSegmentationDisplayNode*
    getCurrentWorkspaceMeshSegmentationDisplayNode();

  // Setters
  void setWorkspaceGenerationNode(vtkMRMLWorkspaceGenerationNode* wgn);
  void setWorkspaceMeshSegmentationDisplayNode(
    vtkMRMLSegmentationDisplayNode* workspaceMeshSegmentationDisplayNode);

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

  // Prune any markups that are more than one.
  void PruneExcessMarkups(vtkMRMLMarkupsFiducialNode* mfn);

  // Parameter Nodes
  vtkMRMLWorkspaceGenerationNode* WorkspaceGenerationNode;

  // Input Nodes
  vtkMRMLVolumeNode*        InputVolumeNode;
  vtkMRMLAnnotationROINode* AnnotationROINode;

  // Robot Workspace Nodes
  vtkMRMLSegmentationNode* WorkspaceMeshSegmentationNode;

  // Display Nodes
  vtkMRMLVolumeRenderingDisplayNode* InputVolumeRenderingDisplayNode;
  vtkMRMLSegmentationDisplayNode*    WorkspaceMeshSegmentationDisplayNode;

  // Volume Rendering Logic
  vtkSlicerVolumeRenderingLogic* VolumeRenderingLogic;
  qSlicerAbstractCoreModule*     VolumeRenderingModule;

  // Models Logic
  vtkSlicerModelsLogic*      ModelsLogic;
  qSlicerAbstractCoreModule* ModelsModule;

  // Markups Logic
  vtkSlicerMarkupsLogic*     MarkupsLogic;
  qSlicerAbstractCoreModule* MarkupsModule;

  // Segmentations Logic
  vtkSlicerSegmentationsModuleLogic* SegmentationsLogic;
  qSlicerAbstractCoreModule*         SegmentationsModule;

private:
  vtkSlicerWorkspaceGenerationLogic(
    const vtkSlicerWorkspaceGenerationLogic&);               // Not implemented
  void operator=(const vtkSlicerWorkspaceGenerationLogic&);  // Not implemented
};

#endif
