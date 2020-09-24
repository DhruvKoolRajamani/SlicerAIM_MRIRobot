cmake_minimum_required(VERSION 3.13.4)

project(SlicerAIM_MRIRobot)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "https://www.slicer.org/wiki/Documentation/Nightly/Extensions/AIM_MRIRobot")
set(EXTENSION_CATEGORY "MRIRobot")
set(EXTENSION_CONTRIBUTORS "Dhruv Kool Rajamani (Automated and Interventional Medicine Lab)")
set(EXTENSION_DESCRIPTION "This extension provides a systematic environment to manage the workflow of the WPI-AIM MRI Robot for stereotactic surgeries")
set(EXTENSION_ICONURL "http://www.example.com/Slicer/Extensions/AIM_MRIRobot.png")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/AIM_MRIRobot/Screenshots/1.png")
set(EXTENSION_DEPENDS "NA") # Specified as a list or "NA" if no dependencies

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(WorkspaceGeneration)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
