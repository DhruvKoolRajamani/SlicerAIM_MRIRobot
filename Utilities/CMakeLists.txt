cmake_minimum_required(VERSION 3.0)
project(utilities)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets REQUIRED)

include_directories(
  include
  ${PROJECT_SOURCE_DIR}/include
)

set (${PROJECT_NAME}_INCLUDE_DIRS
  "${PROJECT_SOURCE_DIR}/include/debug"
)

file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME} ${SRC_FILES})
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