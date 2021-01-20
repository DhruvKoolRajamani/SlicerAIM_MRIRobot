project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")

# find_package(NeuroKinematics REQUIRED)

set(${KIT}_INCLUDE_DIRECTORIES
  ${qSlicerVolumesModuleWidgets_SOURCE_DIR}
  ${qSlicerVolumesModuleWidgets_BINARY_DIR}
  ${vtkVolumeRenderingModuleLogic_INCLUDE_DIRECTORIES}
  ${vtkVolumeRenderingModuleLogic_SRCS}
  ${vtkSlicerModelsModuleLogic_INCLUDE_DIRECTORIES}
  ${vtkSlicerModelsModuleLogic_SRCS}
  ${vtkSlicerSegmentationsModuleLogic_INCLUDE_DIRS}
  ${vtkSlicerSegmentationsModuleLogic_SRCS}
  ${vtkSlicerCropVolumeModuleLogic_INCLUDE_DIRS}
  ${vtkSlicerCropVolumeModuleLogic_SRCS}
  ${utilities_INCLUDE_DIRS}
  ${neurorobot_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  vtkSlicer${MODULE_NAME}Logic.cxx
  vtkSlicer${MODULE_NAME}Logic.h
  ${MODULE_NAME}.cxx
  ${MODULE_NAME}.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  vtkSlicer${MODULE_NAME}ModuleMRML
  # vtkSlicerModelsModuleMRML
  vtkSlicerVolumesModuleLogic
  vtkSlicerModelsModuleLogic
  vtkSlicerVolumeRenderingModuleLogic
  vtkSlicerMarkupsModuleMRML
  vtkSlicerMarkupsModuleLogic
  vtkSlicerSegmentationsModuleLogic
  vtkSlicerCropVolumeModuleLogic
  vtkSlicerVolumesModuleLogic
  # utilities
  NeuroRobotKinematics
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
