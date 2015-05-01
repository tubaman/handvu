
bin_PROGRAMS = hvCvCam

hvCvCam_SOURCES = hv_CvCam.cpp

EXTRA_DIST = hv_CvCam.vcproj


INCLUDES = $(INC_CUBICLES) $(INC_HANDVU) $(INC_OPENCV)

hvCvCam_LDFLAGS = -L../lib -lcubicles -lhandvu \
	$(LIB_OPENCV) -lcvcam


