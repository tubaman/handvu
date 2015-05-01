
bin_PROGRAMS = hvARtk

hvARtk_SOURCES = main.cpp gsub.c sdlut.c drawing.cpp \
		callbacks.cpp \
		hv_process_frame.cpp LinuxDC1394CameraController.cpp \
		hv_ARtk_demo.cpp

noinst_HEADERS = main.h gsub.h sdlut.h drawing.h \
		callbacks.h LinuxDC1394CameraController.h \
		hv_ARtk_demo.h

# All the rest of the distributed files
EXTRA_DIST = data/single.dat data/ARmulti.dat data/camera_para.dat \
	data/patt.Q data/patt.arrow data/patt.tbars data/patt.idot \
	data/patt.swisscheese data/patt.tetris data/patt.flagdot \
	data/patt.kolping data/patt.books

INC_DC1394=-I /usr/local/include/libdc1394

# use ARToolkit's video support with firewire digital camera support.
# ARToolkit must be compiled properly to use dc1394 video.  
#
ARDC_LIBS = -lARvideo -lraw1394 -ldc1394_control


# SDL resources locations
#
SDL_INCS =
SDL_LIBS = $(shell /usr/bin/sdl-config --libs)


# OpenGL / X11 libraries
#
OGL_INCS =
OGL_LIBS = -lglut -lGLU -lGL \
	-L/usr/X11R6/lib/ -lXmu -lXi -lXext -lX11 -lXt

INCLUDES = $(INC_CUBICLES) $(INC_HANDVU) $(INC_OPENCV) $(INC_ARTK) \
	$(INC_DC1394) $(SDL_INCS) $(OGL_INCS)

hvARtk_LDFLAGS = -L../lib -lcubicles -lhandvu \
	$(LIB_OPENCV) \
	$(LIB_ARTK) -lAR -lARMulti \
	$(OGL_LIBS) $(ARDC_LIBS) $(SDL_LIBS)

