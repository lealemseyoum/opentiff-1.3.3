
set(OPENTIFF_SRCS   "TiffCodec.cpp"
	                "TiffCodecJpeg.cpp"
	                "TiffCodecNone.cpp"
	                "TiffCodecPackBits.cpp"
	                "TiffDirectory.cpp"
	                "TiffFile.cpp"
	                "TiffFileIO.cpp"
                	"TiffIO.cpp"
                	"TiffImage.cpp"
                	"TiffImageFile.cpp"
                	"TiffMemoryIO.cpp"
                	"TiffStripImage.cpp"
                	"TiffTileImage.cpp"
                	"TiffUtil.cpp")

if(Enable_MMAP)
    list(APPEND OPENTIFF_SRCS TiffMemMapIO.cpp)
endif()

add_library(
    ${PROJECT_NAME} STATIC ${OPENTIFF_SRCS}
    )

target_include_directories(
    ${PROJECT_NAME} PUBLIC 
    $<BUILD_INTERFACE:${OPENTIFF_INCLUDE_DIRS}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)


# Install headers, libraries and executables
set(${PROJECT_NAME}_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}/ CACHE PATH
"Installation directory for config-file package cmake files")

install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}_lib_export 
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

install(
    EXPORT ${PROJECT_NAME}_lib_export
    FILE ${PROJECT_NAME}Config.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKEDIR} )

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_NAME}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion)


install(
    FILES 
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKEDIR}
)

install(
    DIRECTORY ../inc/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
