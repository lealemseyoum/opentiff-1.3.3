
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

# Install targets
install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets 
    INCLUDES 
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )


# Install header files or include directories
install(
    DIRECTORY ../inc/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
