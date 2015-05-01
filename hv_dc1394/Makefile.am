
bin_PROGRAMS = hvDC1394

hvDC1394_SOURCES = hv_dc1394.cpp


INCLUDES = $(INC_CUBICLES) $(INC_HANDVU) $(INC_OPENCV) $(RAW1394_CFLAGS)

hvDC1394_LDFLAGS = -L../lib -lcubicles -lhandvu \
	$(LIB_OPENCV) \
	$(LIB_DC1394) -ldc1394_control \
	$(LIB_X11) -lX11 -lXv

EXTRA_DIST = dc1394_help.sh
