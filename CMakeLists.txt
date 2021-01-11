cmake_minimum_required(VERSION 3.13.4)

project(SlicerAIM_MRIRobot)

#-----------------------------------------------------------------------------
# Debug flags
set(CMAKE_BUILD_TYPE Debug)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_NAME SlicerAIM_MRIRobot)
set(EXTENSION_HOMEPAGE "https://www.slicer.org/wiki/Documentation/Nightly/Extensions/AIM_MRIRobot")
set(EXTENSION_CATEGORY "/home/dhruv/SlicerExtensions/SlicerAIM_MRIRobot/WorkspaceGeneration/Resources/home/dhruv/SlicerExtensions/SlicerAIM_MRIRobot/WorkspaceGeneration/Resources")
set(EXTENSION_CONTRIBUTORS "Dhruv Kool Rajamani (Automation and Interventional Medicine Lab)")
set(EXTENSION_DESCRIPTION "This extension provides a systematic environment to manage the workflow of the WPI-AIM MRI Robot for stereotactic surgeries")
set(EXTENSION_ICONURL "https://raw.githubusercontent.com/DhruvKoolRajamani/SlicerAIM_MRIRobot/master/AIM_MRIRobot.png?token=AIQ5C6G5QJ2GYLKYPHJBF7S7OWC7Y")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/AIM_MRIRobot/Screenshots/1.png")
set(EXTENSION_DEPENDS "NA") # Specified as a list or "NA" if no dependencies

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(Utilities)
add_subdirectory(NeuroRobot)
set(NeuroKinematics_DIR ${CMAKE_BINARY_DIR}/NeuroRobot/CMakeFiles/Export/cmake/)

add_subdirectory(WorkspaceGeneration)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})