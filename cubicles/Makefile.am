
CORE_FILES = \
IntegralFeatures.cpp IntegralFeaturesSame.cpp Classifiers.cpp \
CascadeFileParser.yy CascadeFileScanner.l Cascade.cpp Image.cpp \
Scanner.cpp Exceptions.cpp StringUtils.cpp

EXTRA_TRAIN_FILES = \
ExampleIntegral.cpp CascadeTrainer.cpp CascadeTrainer_Monolithic.cpp \
CascadeTrainer_Component.cpp Main.cpp NegativeExampleProducer.cpp \
Scanner_Train.cpp EvaluationSpace.cpp TrainsetManager.cpp StateSnapshot.cpp \
Cascade_Train.cpp IntegralFeatures_Train.cpp

CORE_HEADS = \
cubicles.hpp Cascade.h Exceptions.h Image.h Rect.h Classifiers.h \
IntegralFeatures.h Scanner.h IntegralImage.h

EXTRA_TRAIN_HEADS = \
ExampleIntegral.h MPI_TRACE.h NegativeExampleProducer.h CascadeTrainer.h \
Scanner.h StateSnapshot.h EvaluationSpace.h Main.h TrainsetManager.h

EXTRA_LIB_FILES = cubicles.cpp


# all header files that contain functionality and are to be published
include_HEADERS = cubicles.h

# header files that are not be installed
noinst_HEADERS = $(CORE_HEADS)

EXTRA_DIST = IntegralImage.cxx cubicles.vcproj


# compile C files as C++ code; we need this for the CascadeFileScanner.c
# and CascadeFileParser.c;
# also, generate a separate header file with $(YACC)
AM_CFLAGS := $(AM_CFLAGS) -x c++
AM_YFLAGS := $(AM_YFLAGS) -d

INCLUDES = $(INC_OPENCV) $(INC_MAGICK) $(INC_MPI)



#if WITH_TRAINING
#todo: enable WITH_TRAINING
#bin_PROGRAMS = it_train
#it_train_SOURCES = ${CORE_FILES} ${EXTRA_TRAIN_FILES}
#noinst_HEADERS = $(CORE_HEADS) $(EXTRA_TRAIN_HEADS)
#it_train_LDFLAGS = $(LIB_OPENCV)

#else

lib_LTLIBRARIES = $(top_srcdir)/lib/libcubicles.la
__top_srcdir__lib_libcubicles_la_SOURCES = $(CORE_FILES) $(EXTRA_LIB_FILES)
#osx doesnt like: libcubicles_la_LDFLAGS = -no-undefined

#endif

