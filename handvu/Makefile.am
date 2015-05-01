
if SMALL_COLOR
COLOR_FILE = skinrgb_32768.cpp
NOT_COLOR_FILE = skinrgb_262144.cpp
else
NOT_COLOR_FILE = skinrgb_32768.cpp
COLOR_FILE = skinrgb_262144.cpp
endif

HANDVU_FILES = HandVu.cpp HandVu_Cintf.cpp HandVu_img.cpp \
VisionConductor.cpp Skincolor.cpp CubicleWrapper.cpp \
OpticalFlow.cpp OpticalFlowPredict.cpp Exceptions.cpp \
Undistortion.cpp LearnedColor.cpp CamShift.cpp Mask.cpp \
FileHandling.cpp GestureServer.cpp OSCpacket.cpp Thread.cpp \
$(COLOR_FILE)


# all header files that contain functionality and are to be published
include_HEADERS = HandVu.h

# header files that are not be installed
noinst_HEADERS = \
HandVu.hpp CamShift.h ProbDistrProvider.h Undistortion.h \
Common.h Quadruple.h VisionConductor.h \
CubicleWrapper.h LearnedColor.h Rect.h \
Exceptions.h Mask.h Skincolor.h \
FileHandling.h OpticalFlow.h skinrgb.h GestureServer.h \
OSCpacket.h Thread.h

EXTRA_DIST = $(NOT_COLOR_FILE) HandVu.vcproj



lib_LTLIBRARIES = $(top_srcdir)/lib/libhandvu.la

__top_srcdir__lib_libhandvu_la_SOURCES = $(HANDVU_FILES)

# WIN32 doesn't like: __top_srcdir__lib_libhandvu_la_LDFLAGS = -no-undefined

INCLUDES = $(INC_CUBICLES) $(INC_OPENCV) $(INC_MAGICK)

LIBS = -L../lib -lcubicles


