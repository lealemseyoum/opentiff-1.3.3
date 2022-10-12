
AUTOMAKE_OPTIONS = foreign

if ENABLE_MMAP
mmapSupportSource = TiffMemMapIO.cpp
mmapSupportHeader = ../inc/TiffMemMapIO.h
else
mmapSupportSource =
mmapSupportHeader = 
endif

# If we do a distribution on a system that does not support
# mmap, the memory map support sources will not appear in 
# xxx_SOURCES. However, we still want these memory map support
# files to be included in the distribution, hence they are
# added to the EXTRA_DIST macro. Note, automake does not
# support the conditional assignment of EXTRA_DIST.
EXTRA_DIST = TiffMemMapIO.cpp ../inc/TiffMemMapIO.h

lib_LTLIBRARIES = libopentiff.la

libopentiff_la_SOURCES = TiffCodec.cpp  \
	TiffCodecJpeg.cpp  \
	TiffCodecNone.cpp  \
	TiffCodecPackBits.cpp \
	TiffDirectory.cpp \
	TiffFile.cpp \
	TiffFileIO.cpp \
	TiffIO.cpp \
	TiffImage.cpp \
	TiffImageFile.cpp \
	TiffMemoryIO.cpp \
	TiffStripImage.cpp \
	$(mmapSupportSource) \
	TiffTileImage.cpp \
	TiffUtil.cpp

include_HEADERS = ../inc/TiffCodec.h \
	../inc/TiffCodecJpeg.h \
	../inc/TiffCodecNone.h \
	../inc/TiffCodecPackBits.h \
	../inc/TiffComp.h \
	../inc/TiffConf.h \
	../inc/TiffDefs.h \
	../inc/TiffDirectory.h \
	../inc/TiffError.h \
	../inc/TiffFile.h \
	../inc/TiffFileIO.h \
	../inc/TiffIO.h \
	../inc/TiffImage.h \
	../inc/TiffImageFile.h \
	../inc/TiffMemoryIO.h \
	../inc/TiffStripImage.h \
	../inc/TiffTagDefs.h \
	../inc/TiffTagEntry.h \
	../inc/TiffTileImage.h \
	$(mmapSupportHeader) \
	../inc/TiffTypeDefs.h \
	../inc/TiffUtil.h


INCLUDES = -I../inc

clean-compile:
	-rm -f $(OBJECTS)

