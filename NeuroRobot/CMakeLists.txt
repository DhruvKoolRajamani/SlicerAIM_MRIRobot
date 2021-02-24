cmake_minimum_required(VERSION 3.0)
project(neurorobot)

include(GenerateExportHeader)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(NeuroRobot_INCLUDE_INSTALL_DESTINATION include/NeuroRobot)

find_package(Eigen3 REQUIRED NO_MODULE)
find_package(VTK COMPONENTS
  vtkCommonColor
  vtkCommonCore
  vtkCommonDataModel
  vtkFiltersPoints
  vtkFiltersSources
  vtkIOGeometry
  vtkIOLegacy
  vtkIOPLY
  vtkIOXML
  vtkInteractionStyle
  vtkRenderingContextOpenGL2
  vtkRenderingCore
  vtkRenderingFreeType
  vtkRenderingGL2PSOpenGL2
  vtkRenderingOpenGL2 QUIET
)

if(NOT VTK_FOUND)
  message(WARNING "Skipping workspace generation: ${VTK_NOT_FOUND_MESSAGE}")
  return()
endif(NOT VTK_FOUND)
message(STATUS "VTK_VERSION: ${VTK_VERSION}")

include_directories(include
  include/NeuroKinematics
  include/WorkspaceVisualization
)

file(GLOB_RECURSE NeuroRobot_SRCS 
  ${PROJECT_SOURCE_DIR}/src/NeuroKinematics/*.cpp
  ${PROJECT_SOURCE_DIR}/src/WorkspaceVisualization/*.cpp
)

# create a dynamic library for NeuroKinematics to be loaded at runtime?
add_library(NeuroRobot SHARED
  ${NeuroRobot_SRCS}
)

generate_export_header(NeuroRobot)

target_include_directories(NeuroRobot PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${NeuroRobot_INCLUDE_INSTALL_DESTINATION}>)

target_link_libraries(NeuroRobot PUBLIC Eigen3::Eigen ${VTK_LIBRARIES} utilities)

install(TARGETS NeuroRobot EXPORT NeuroRobot
    ARCHIVE DESTINATION lib # static and import libs installed to lib
    LIBRARY DESTINATION lib # .so installed to lib
    RUNTIME DESTINATION bin # .dlls installed to bin
    )

# install directory containing public headers to include/my_lib
install(DIRECTORY include DESTINATION ${NeuroRobot_INCLUDE_INSTALL_DESTINATION}
    FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

# create cmake config to lib/cmake/my_libConfig.cmake
install(EXPORT NeuroRobot
    DESTINATION cmake
    FILE NeuroRobotConfig.cmake)

export(EXPORT NeuroRobot FILE NeuroRobotConfig.cmake)

# add_executable(generate_ply src/WorkspaceGeneration/main.cpp)
# target_link_libraries(generate_ply PRIVATE NeuroKinematics ${VTK_LIBRARIES})

# Tests
message("Adding NeuroRobot Tests")
file(GLOB tests "tests/*_test.cpp")
foreach(test ${tests})
  get_filename_component(FILENAME ${test} NAME_WE)
  message("Test: " ${FILENAME})
  add_executable(${FILENAME} ${test})
  target_link_libraries(${FILENAME} PRIVATE NeuroRobot Eigen3::Eigen ${VTK_LIBRARIES} utilities)
endforeach(test ${tests})
message("Added NeuroRobot Tests")

#=========================================


