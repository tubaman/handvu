
bin_PROGRAMS = hvOpenCV

hvOpenCV_SOURCES = hv_OpenCV.cpp

EXTRA_DIST = hv_OpenCV.vcproj


INCLUDES = $(INC_CUBICLES) $(INC_HANDVU) $(INC_OPENCV)

hvOpenCV_LDFLAGS = -L../lib -lcubicles -lhandvu \
	$(LIB_OPENCV)


