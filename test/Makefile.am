
AUTOMAKE_OPTIONS = foreign

EXTRA_DIST	= BS_16.tif BS_None.tif BT_None.tif LS_16.tif LS_Jpeg.tif \
		  LS_None.tif LT_Jpeg.tif SingleStrip.tif TestOut.txt \
		  input.txt 

TESTS_ENVIRONMENT 	= top_builddir=$(top_builddir)
TESTS			= tiffTest

INCLUDES = -I$(top_srcdir)/inc 

##CXXFLAGS += -D BUILD_LOGICAL_DEFINITION_AND_LOGICAL_INSTANCE_TEST
noinst_PROGRAMS = tiffTest

tiffTest_SOURCES = test.cpp

tiffTest_LDADD = $(top_srcdir)/src/libopentiff.la 

clean-compile:
	-rm -f $(OBJECTS)









