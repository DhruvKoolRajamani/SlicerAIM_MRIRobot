cmake_minimum_required(VERSION 3.0)
project(utilities)

include(GenerateExportHeader)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(${PROJECT_NAME}_INCLUDE_INSTALL_DESTINATION include/${PROJECT_NAME})

find_package(Qt5Widgets REQUIRED)

find_package(Eigen3 REQUIRED NO_MODULE)
find_package(VTK REQUIRED)

if(NOT VTK_FOUND)
  message(WARNING "Skipping utilities: ${VTK_NOT_FOUND_MESSAGE}")
  return()
endif(NOT VTK_FOUND)
message(STATUS "VTK_VERSION: ${VTK_VERSION}")


include_directories(
  include
  ${PROJECT_SOURCE_DIR}/include
)

set (${PROJECT_NAME}_INCLUDE_DIRS
  "${PROJECT_SOURCE_DIR}/include/debug"
  "${PROJECT_SOURCE_DIR}/include/PointSetUtilities"
)

file(GLOB_RECURSE SRC_FILES
  ${PROJECT_SOURCE_DIR}/src/*.cpp
  ${PROJECT_SOURCE_DIR}/src/debug/*.cpp
  ${PROJECT_SOURCE_DIR}/src/PointSetUtilities/*.cpp
)

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${${PROJECT_NAME}_INCLUDE_INSTALL_DESTINATION}>)
target_link_libraries(${PROJECT_NAME} Eigen3::Eigen ${VTK_LIBRARIES})

generate_export_header(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}
    ARCHIVE DESTINATION lib # static and import libs installed to lib
    LIBRARY DESTINATION lib # .so installed to lib
    RUNTIME DESTINATION bin # .dlls installed to bin
    )

install(DIRECTORY include DESTINATION ${${PROJECT_NAME}_INCLUDE_INSTALL_DESTINATION}
    FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(EXPORT ${PROJECT_NAME}
    DESTINATION cmake
    FILE ${PROJECT_NAME}Config.cmake)

export(EXPORT ${PROJECT_NAME} FILE ${PROJECT_NAME}Config.cmake)

qt5_use_modules(${PROJECT_NAME}
  Widgets
  Core
)

add_executable(${PROJECT_NAME}_debug ${PROJECT_SOURCE_DIR}/tests/debug_test.cpp)
qt5_use_modules(${PROJECT_NAME}_debug
  Widgets
  Core
)

target_link_libraries(${PROJECT_NAME}_debug ${PROJECT_NAME})