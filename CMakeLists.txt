cmake_minimum_required(VERSION 3.10)
project(Opentiff VERSION 1.3.3)

set(CMAKE_CXX_STANDARD 11)

include(CMakeDependentOption)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(OPENTIFF_MAJOR_VESION 1)
set(OPENTIFF_MINOR_VERSION 3)
set(OPENTIFF_PATCH_VERSION 3)
set(OPENTIFF_VERSION ${OPENTIFF_MAJOR_VERSION}.${OPENTIFF_MINOR_VERSION}.${OPETNTIFF_PATCH_VERSION})

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

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

# Generate and install package configuration files
configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in 
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake 
    INSTALL_DESTINATION 
        ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
install(
    EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# Create package version cofiguration file
write_basic_package_version_file(
    "${PROJECT_NAME}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion)

# Install package configuration files
install(
    FILES 
        ${CMAKE_CURRENT_BINARY_DIR}/OpentiffConfig.cmake 
        ${CMAKE_CURRENT_BINARY_DIR}/OpentiffConfigVersion.cmake 
    DESTINATION 
        ${CMAKE_INSTALL_LIBDIR}/cmake/Opentiff
)