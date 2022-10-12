cmake_minimum_required(VERSION 3.10)
project(Opentiff VERSION 1.3.3)

set(CMAKE_CXX_STANDARD 11)

include(CMakeDependentOption)
include(GNUInstallDirs)

set(OPENTIFF_MAJOR_VESION 1)
set(OPENTIFF_MINOR_VERSION 3)
set(OPENTIFF_PATCH_VERSION 3)
set(OPENTIFF_VERSION ${OPENTIFF_MAJOR_VERSION}.${OPENTIFF_MINOR_VERSION}.${OPETNTIFF_PATCH_VERSION})

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")
set(CMAKE_INSTALL_PREFIX "/opt/opentiff-1.3.3")

option(ENABLE_MMAP "Enable memory mapping" OFF)
option(BUILD_TESTS "Enable building tests" OFF)

set(OPENTIFF_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/inc)

# Add src sub-directory
add_subdirectory(src)

# Add test sub-directory
if(${BUILD_TESTS})
    message(STATUS "Building tests.")
    add_subdirectory(test)
endif()

