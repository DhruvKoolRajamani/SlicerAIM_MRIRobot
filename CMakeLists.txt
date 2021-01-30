cmake_minimum_required(VERSION 3.13.4)

project(SlicerAIM_MRIRobot)

#-----------------------------------------------------------------------------
# Debug flags
set(CMAKE_BUILD_TYPE Debug)

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

include(FetchContent)
set(nvidia_extension "SegmentEditorNvidiaAIAA")
set(${nvidia_extension}_SOURCE_DIR ${CMAKE_BINARY_DIR}/${nvidia_extension})
set(NvidiaAIAssistedAnnotation_BUILD_SLICER_EXTENSION True)
FetchContent_Declare(${nvidia_extension}
  SOURCE_DIR     ${${nvidia_extension}_SOURCE_DIR}
  GIT_REPOSITORY ${EP_GIT_PROTOCOL}://github.com/NVIDIA/ai-assisted-annotation-client.git
  GIT_TAG        origin/master
  GIT_PROGRESS   1
  PREFIX         ${CMAKE_SOURCE_DIR}/${nvidia_extension}
  # BINARY_DIR     ${CMAKE_BINARY_DIR}/${nvidia_extension}-build
  # INSTALL_DIR    ${CMAKE_BINARY_DIR}/${nvidia_extension}
  # BUILD_ALWAYS   True
  # CMAKE_ARGS     -D:BOOL=ON
  )

FetchContent_GetProperties(${nvidia_extension})
if (NOT ${nvidia_extension}_POPULATED)
  FetchContent_Populate(${nvidia_extension})
  add_subdirectory(${${nvidia_extension}_SOURCE_DIR} ${${nvidia_extension}_BINARY_DIR})
endif ()

list(APPEND Slicer_EXTENSION_SOURCE_DIRS ${${nvidia_extension}_SOURCE_DIR})

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_NAME SlicerAIM_MRIRobot)
set(EXTENSION_HOMEPAGE "https://www.slicer.org/wiki/Documentation/Nightly/Extensions/AIM_MRIRobot")
set(EXTENSION_CATEGORY "/home/dhruv/SlicerExtensions/SlicerAIM_MRIRobot/WorkspaceGeneration/Resources/home/dhruv/SlicerExtensions/SlicerAIM_MRIRobot/WorkspaceGeneration/Resources")
set(EXTENSION_CONTRIBUTORS "Dhruv Kool Rajamani (Automation and Interventional Medicine Lab)")
set(EXTENSION_DESCRIPTION "This extension provides a systematic environment to manage the workflow of the WPI-AIM MRI Robot for stereotactic surgeries")
set(EXTENSION_ICONURL "https://raw.githubusercontent.com/DhruvKoolRajamani/SlicerAIM_MRIRobot/master/AIM_MRIRobot.png?token=AIQ5C6G5QJ2GYLKYPHJBF7S7OWC7Y")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/AIM_MRIRobot/Screenshots/1.png")
set(EXTENSION_DEPENDS "${nvidia_extension}") # Specified as a list or "NA" if no dependencies

#-----------------------------------------------------------------------------
# set(NvidiaAIAssistedAnnotation_BUILD_SLICER_EXTENSION true)
# add_subdirectory(ai-assisted-annotation-client)

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(Utilities)
add_subdirectory(NeuroRobot)
set(NeuroKinematics_DIR ${CMAKE_BINARY_DIR}/NeuroRobot/CMakeFiles/Export/cmake/)

add_subdirectory(WorkspaceGeneration)
## NEXT_MODULE
# add_dependencies(qSlicerWorkspaceGenerationModuleWidgets SegmentEditorNvidiaAIAA)

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})