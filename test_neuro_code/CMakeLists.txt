# cmake_minimum_required(VERSION 3.0)
# project(neurorobot)

# include(GenerateExportHeader)

# set(CMAKE_AUTOMOC ON)
# set(CMAKE_INCLUDE_CURRENT_DIR ON)

# set(NeuroRobotKinematics_INCLUDE_INSTALL_DESTINATION include/NueroRobotKinematics)

# find_package(Eigen3 REQUIRED NO_MODULE)
# find_package(VTK COMPONENTS
#   vtkCommonColor
#   vtkCommonCore
#   vtkCommonDataModel
#   vtkFiltersPoints
#   vtkFiltersSources
#   vtkIOGeometry
#   vtkIOLegacy
#   vtkIOPLY
#   vtkIOXML
#   vtkInteractionStyle
#   # vtkPoissonReconstruction
#   # Powercrust
#   vtkRenderingContextOpenGL2
#   vtkRenderingCore
#   vtkRenderingFreeType
#   vtkRenderingGL2PSOpenGL2
#   vtkRenderingOpenGL2 QUIET
# )

# if(NOT VTK_FOUND)
#   message(WARNING "Skipping workspace generation: ${VTK_NOT_FOUND_MESSAGE}")
#   return()
# endif(NOT VTK_FOUND)
# message(STATUS "VTK_VERSION: ${VTK_VERSION}")

# include_directories(include
#   include/Kinematics
#   include/NeuroKinematics
#   include/NeuroRobotKinematics
# )

# file(GLOB_RECURSE NeuroRobotKinematics_SRCS 
#   ${PROJECT_SOURCE_DIR}/src/NeuroKinematics/*.cpp
#   ${PROJECT_SOURCE_DIR}/src/NeuroRobotKinematics/*.cpp
# )

# # create a dynamic library for NeuroRobotKinematics to be loaded at runtime?
# add_library(NeuroRobotKinematics SHARED
#   ${NeuroRobotKinematics_SRCS}
# )

# generate_export_header(NeuroRobotKinematics)

# target_include_directories(NeuroRobotKinematics PUBLIC
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#     $<INSTALL_INTERFACE:${NeuroRobotKinematics_INCLUDE_INSTALL_DESTINATION}>)

# target_link_libraries(NeuroRobotKinematics PUBLIC Eigen3::Eigen ${VTK_LIBRARIES})

# install(TARGETS NeuroRobotKinematics EXPORT NeuroRobotKinematics
#     ARCHIVE DESTINATION lib # static and import libs installed to lib
#     LIBRARY DESTINATION lib # .so installed to lib
#     RUNTIME DESTINATION bin # .dlls installed to bin
#     )

# # install directory containing public headers to include/my_lib
# install(DIRECTORY include DESTINATION ${NeuroRobotKinematics_INCLUDE_INSTALL_DESTINATION}
#     FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
#     DIRECTORY_PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

# # create cmake config to lib/cmake/my_libConfig.cmake
# install(EXPORT NeuroRobotKinematics
#     DESTINATION cmake
#     FILE NeuroRobotKinematicsConfig.cmake)

# export(EXPORT NeuroRobotKinematics FILE NeuroRobotKinematicsConfig.cmake)
#=========================================
cmake_minimum_required(VERSION 3.0.0)

project(Workspace_testing)

include_directories(../NeuroRobot/include/Kinematics ../NeuroRobot/include/WorkspaceGeneration/ ../Utilities/include/SavePointCloudData/)
file(GLOB_RECURSE SOURCES "./main.cpp" "../NeuroRobot/src/Kinematics/NeuroKinematics.cpp" "../NeuroRobot/src/WorkspaceGeneration/WorkspaceVisualization.cxx" "../Utilities/src/SavePointCloudData/SavePointCloudData.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})
