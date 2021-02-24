cmake_minimum_required(VERSION 3.13.4)

project(SlicerAIM_MRIRobot)

#-----------------------------------------------------------------------------
# Debug flags
set(CMAKE_BUILD_TYPE Debug)

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})
include_directories( ${Boost_INCLUDE_DIR} )

find_package(Boost REQUIRED)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

include(InstallRequiredSystemLibraries)
include(ExternalProject)

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
# Extension modules
add_subdirectory(Utilities)
add_subdirectory(NeuroRobot)
set(NeuroRobot_DIR ${CMAKE_BINARY_DIR}/NeuroRobot/CMakeFiles/Export/cmake/)

# set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/)
add_definitions(-DUSE_SUPERBUILD:BOOL=ON -DTHIRDPARTY_BINARY_DIR=${CMAKE_BINARY_DIR}/3rdparty -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/install)
set(USE_SUPERBUILD ON)
set(THIRDPARTY_BINARY_DIR ${CMAKE_BINARY_DIR}/3rdparty)
set(CMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/install)
# Output Directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(ai-assisted-annotation-client/cpp-client/CMakeExternals/nlohmann_json.cmake)

add_subdirectory(ai-assisted-annotation-client)
add_dependencies(NvidiaAIAAClient nlohmann_json)
export(TARGETS NvidiaAIAAClient FILE NvidiaAIAAClientConfig.cmake)

# set(lua_version "5.4.1")
# ExternalProject_Add(Lua
#   PREFIX            ${CMAKE_BINARY_DIR}/Lua
#   URL               http://www.lua.org/ftp/lua-${lua_version}.tar.gz
#   CONFIGURE_COMMAND ""
#   INSTALL_COMMAND   ""
#   BUILD_IN_SOURCE   1
#   BUILD_COMMAND     COMMAND make all
# )

# set(SimpleITK "SimpleITK")
# ExternalProject_Add(${SimpleITK}
#   PREFIX            ${CMAKE_BINARY_DIR}/${SimpleITK}
#   BINARY_DIR        ${CMAKE_BINARY_DIR}/${SimpleITK}-build
#   # SOURCE_DIR        ${CMAKE_BINARY_DIR}/${SimpleITK}/src
#   GIT_REPOSITORY    ${EP_GIT_PROTOCOL}://github.com/SimpleITK/SimpleITK.git
#   GIT_TAG           origin/master
#   GIT_PROGRESS      1
#   CMAKE_CACHE_ARGS  -DITK_DIR:PATH=${ITK_DIR}
#   DEPENDS           Lua
# )

add_subdirectory(WorkspaceGeneration)

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})

#-----------------------------------------------------------------------------

# set(${nvidia_extension}_SOURCE_DIR ${CMAKE_BINARY_DIR}/${nvidia_extension})

# # include(FetchContent)
# # set(NvidiaAIAssistedAnnotation_BUILD_SLICER_EXTENSION True)
# # FetchContent_Declare(${nvidia_extension}
# #   # SOURCE_DIR     ${${nvidia_extension}_SOURCE_DIR}
# #   GIT_REPOSITORY ${EP_GIT_PROTOCOL}://github.com/NVIDIA/ai-assisted-annotation-client.git
# #   GIT_TAG        origin/master
# #   GIT_PROGRESS   1
# #   PREFIX         ${CMAKE_SOURCE_DIR}/${nvidia_extension}
# #   # BINARY_DIR     ${CMAKE_BINARY_DIR}/${nvidia_extension}-build
# #   # INSTALL_DIR    ${CMAKE_BINARY_DIR}/${nvidia_extension}
# #   # BUILD_ALWAYS   True
# #   # CMAKE_ARGS     -D:BOOL=ON
# #   CONFIGURE_COMMAND ${CMAKE_COMMAND}
# #         <SOURCE_DIR>/DevIL
# #   )

# # FetchContent_GetProperties(${nvidia_extension})
# # if (NOT ${nvidia_extension}_POPULATED)
# #   FetchContent_Populate(${nvidia_extension})
# #   # add_subdirectory(_deps ${${nvidia_extension}_BINARY_DIR})
# # endif ()

# include(ExternalProject)

# ExternalProject_Add(${nvidia_extension}_download
#   PREFIX            ${CMAKE_BINARY_DIR}/external
#   SOURCE_DIR        ${CMAKE_BINARY_DIR}/external/src/${nvidia_extension}
#   GIT_REPOSITORY    ${EP_GIT_PROTOCOL}://github.com/NVIDIA/ai-assisted-annotation-client.git
#   # INSTALL_DIR       ${CMAKE_BINARY_DIR}/${nvidia_extension}
#   # BINARY_DIR        ${CMAKE_BINARY_DIR}/${nvidia_extension}-build
#   CONFIGURE_COMMAND ""
#   BUILD_COMMAND     ""
#   INSTALL_COMMAND   ""
#   )

# # ExternalProject_Add(${nvidia_extension}
# #   PREFIX            ${CMAKE_BINARY_DIR}
# #   DOWNLOAD_COMMAND  "" # Disable download step
# #   SOURCE_DIR        ${CMAKE_BINARY_DIR}/external/src/SegmentEditorNvidiaAIAA/slicer-plugin/NvidiaAIAA
# #   INSTALL_DIR       ${CMAKE_BINARY_DIR}/${nvidia_extension}
# #   # CMAKE_CACHE_ARGS  -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
# # )

# # set(Slicer_EXTENSION_SOURCE_DIRS
# add_subdirectory(${CMAKE_BINARY_DIR}/external/src/SegmentEditorNvidiaAIAA/slicer-plugin/NvidiaAIAA)

# # list(APPEND Slicer_EXTENSION_SOURCE_DIRS ${${nvidia_extension}_SOURCE_DIR})
