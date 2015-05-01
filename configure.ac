#                                               
# Process this file with autoconf to produce a configure script.
#
# todo: before shipping, get new config.guess and config.sub from GNU site

AC_PREREQ(2.57)
AC_INIT(HandVu, beta3, google://handvu)
#AC_INIT(cubicles, version 1.5, google://handvu)
AM_INIT_AUTOMAKE()
# maintainer mode: a double-edged sword. look here:
# http://www.gnu.org/software/automake/manual/html_node/CVS.html
# http://www.gnu.org/software/automake/manual/html_node/maintainer-mode.html
AM_MAINTAINER_MODE
#AC_CONFIG_AUX_DIR([.cfg])
AM_CONFIG_HEADER([hvconfig.h])

abs_top_srcdir=`pwd`

# include macro definitions
#sinclude(ac_common.m4)

# Before checking for CXX and CC, set CFLAGS and CXXFLAGS because they
# are otherwise initialized to contain -g -O2
if test "x$CFLAGS" = "x"; then
	CFLAGS=
fi
if test "x$CXXFLAGS" = "x"; then
	CXXFLAGS=
fi

# Checks for programs.
AC_PROG_YACC
AC_PROG_CXX
AC_PROG_CC
ifdef([m4_pattern_allow],
      [m4_pattern_allow([^AM_PROG_LEX])])dnl
AM_PROG_LEX
#AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_PROG_LN_S

# Set language to C++
AC_LANG(C++)

# what platform are we building for?
# todo: should we check $target or $host?
case "$host" in
  *-win32*)
    ACC_ADDTO(AM_CPPFLAGS, '-DWIN32 -DUSE_MFC')
    # todo: build for CRTDLL.DLL?  use -mno-cygwin
    ;;
  *-cygwin* | *-mingw*)
    ACC_ADDTO(AM_CPPFLAGS, -DCYGWIN)
    ;;
  *)
    # Assume Unix.
    ;;
esac
ACC_ADDTO(AM_CPPFLAGS, -DTARGET_SYSTEM=\\\"$host\\\")


#
# what distribution is this - do we have cubicle source code?  handvu source?
#
have_cubicles_sources="no"
if test -f "./cubicles/cubicles.hpp"; then
	have_cubicles_sources="yes"
	SUBDIR_CUBICLES=cubicles
	AC_SUBST(SUBDIR_CUBICLES)
fi
have_cubicles_training_sources="no"
if test -f "./cubicles/Main.cpp"; then
	have_cubicles_training_sources="yes"
fi
have_handvu_sources="no"
if test -f "./handvu/HandVu.cpp"; then
	have_handvu_sources="yes"
	SUBDIR_HANDVU=handvu
	AC_SUBST(SUBDIR_HANDVU)
fi
have_hvopencv_sources="no"
if test -f "./hv_OpenCV/hv_OpenCV.cpp"; then
	have_hvopencv_sources="yes"
	SUBDIR_HVOPENCV=hv_OpenCV
	AC_SUBST(SUBDIR_HVOPENCV)
fi
have_hvcvcam_sources="no"
if test -f "./hv_CvCam/hv_CvCam.cpp"; then
        # don't build CvCam interface on *nix
        case "$host" in
          *-win32* | *-cygwin* | *-mingw*)
            have_hvcvcam_sources="yes"
            SUBDIR_HVCVCAM=hv_CvCam
	    AC_SUBST(SUBDIR_HVCVCAM)
            ;;
          *)
        # Assume *nix. only include in distribution, don't build
            have_hvcvcam_sources="no"
            SUBDIR_HVCVCAM_DIST=hv_CvCam
	    AC_SUBST(SUBDIR_HVCVCAM_DIST)
            ;;
        esac
fi
have_hvdc1394_sources="no"
if test -f "./hv_dc1394/hv_dc1394.cpp"; then
	have_hvdc1394_sources="yes"
fi
have_hvartk_sources="no"
if test -f "./hv_ARtk/hv_ARtk_demo.cpp"; then
	have_hvartk_sources="yes"
fi
have_hvdxapp_sources="no"
if test -f "./hv_DXApp/DXApp.cpp"; then
	have_hvdxapp_sources="yes"
	SUBDIR_HVDXAPP=hv_DXApp
	AC_SUBST(SUBDIR_HVDXAPP)
fi
have_hvdxfilter_sources="no"
if test -f "./hv_DXFilter/HandVuFilter.cpp"; then
	have_hvdxfilter_sources="yes"
	SUBDIR_HVDXFILTER=hv_DXFilter
	AC_SUBST(SUBDIR_HVDXFILTER)
fi

#
# configure command line arguments
#

# install to /usr/local unless otherwise specified
if test "$prefix" = "NONE"; then
	prefix=/usr/local
#	prefix=$abs_top_srcdir
fi

# debug
e_debug="no"
AC_ARG_ENABLE(debug,
[  --enable-debug          turn on asserts and other debugging checks [no]],
[e_debug="yes"],[e_debug="no"])
if test "$e_debug" = "yes"; then
 ACC_ADDTO(AM_CFLAGS, -g -DDEBUG)
 ACC_ADDTO(AM_CPPFLAGS, -g -DDEBUG)
 ACC_ADDTO(AM_CXXFLAGS, -g)
else
 ACC_ADDTO(AM_CFLAGS, -O3)
 ACC_ADDTO(AM_CXXFLAGS, -O3)
fi
AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_CXXFLAGS)


# small-color
e_small_color="no"
AC_ARG_ENABLE(small-color,
[  --enable-small-color    use small skin color lookup table [no]],
[e_small_color="yes"])
AM_CONDITIONAL(SMALL_COLOR, test "$enable_small_color" = "yes")

# hv_ARToolKit demo, ARToolKit location
#inc_artk=$INC_ARTK - unsafe on Windows until fileseparator conversion
INC_ARTK=
AC_ARG_WITH(artk,
[  --with-artk=path/no     build ARToolKit demo @<:@build it@:>@],
[if test "$withval" = "no"; then 
   have_hvartk_sources="no"
 elif test "x$withval" != "x" && test "$withval" != "yes"; then
   inc_artk=$withval
 fi]
)

AC_ARG_WITH(dc1394,
[  --without-dc1394        do not build libdc1394 @<:@build it@:>@],
[if test "$withval" = "no"; then 
   have_hvdc1394_sources="no"
 fi]
)


# Magick or OpenCV for file i/o or none in cubicles?
#
w_opencv="yes"
#inc_opencv=$INC_OPENCV - unsafe on Windows until fileseparator conversion
INC_OPENCV=
IMG_LIB=IMG_LIB_OPENCV
AC_ARG_WITH(opencv,
[  --with-opencv=path/no   use OpenCV, also for file i/o @<:@yes@:>@],
[if test "$withval" = "no"; then 
   if test "$have_cubicles_sources" = "yes"; then
     IMG_LIB=IMG_LIB_NONE
     w_opencv="no"
   else
     AC_MSG_WARN(OpenCV is mandatory in this distribution)
   fi
 elif test "x$withval" != "x" && test "$withval" != "yes"; then
   inc_opencv=$withval
 fi]
)

w_magick="no"
#inc_magick=$INC_MAGICK - unsafe on Windows until fileseparator conversion
INC_MAGICK=
AC_ARG_WITH(magick,
[  --with-magick=path/no   use ImageMagick[=abspath] for file i/o @<:@no@:>@],
[if test "$have_cubicles_sources" = "yes"; then
   IMG_LIB=IMG_LIB_MAGICK
   w_magick="yes"
   w_opencv="no"
   if test "x$withval" != "x" && test "$withval" != "yes"; then
	inc_magick=$withval
   fi
 else
   AC_MSG_WARN(can not use Magick in this distribution)
 fi
],[])
ACC_ADDTO(AM_CPPFLAGS, -D$IMG_LIB)


# with training
e_training="no"
AC_ARG_ENABLE(training,
[  --enable-training       enable classifier training functionality @<:@no@:>@]],
[if test "$have_cubicles_training_sources" = "yes"; then
   e_training="yes"
   ACC_ADDTO(AM_CFLAGS, -DWITH_TRAINING)
   ACC_ADDTO(AM_CPPFLAGS, -DWITH_TRAINING)
 else
   AC_MSG_WARN(can not enable 'training' in this distribution)
 fi])
AM_CONDITIONAL(WITH_TRAINING, test "$enable_training" = "yes")

#
# MPI: no, mpich, or lam
#
USE_MPI="no"
#inc_mpi=$INC_MPI - unsafe on Windows until fileseparator conversion
INC_MPI=
AC_ARG_WITH(mpich,
[  --with-mpich=path/no    use MPI for training, mpich implementation @<:@no@:>@],
[if test "$have_cubicles_sources" = "yes"; then
   if test "$USE_MPI" != "no"; then
     AC_MSG_WARN(changing previously selected USE_MPI=$USE_MPI with "mpich")
   fi
   if test "x$withval" != "x" && test "$withval" != "yes"; then
	inc_mpi=$withval
   fi
   USE_MPI="mpich"
 else
   AC_MSG_WARN(can not enable 'mpi' in this distribution)
 fi]
)

AC_ARG_WITH(lam,
[  --with-lam=path/no      use MPI for training, LAM implementation @<:@no@:>@],
[if test "$have_cubicles_sources" = "yes"; then
   if test "$USE_MPI" != "no"; then
     AC_MSG_WARN(changing previously selected USE_MPI=$USE_MPI with "lam")
   fi
   if test "x$withval" != "x" && test "$withval" != "yes"; then
	inc_mpi=$withval
   fi
   USE_MPI="lam"
 else
   AC_MSG_WARN(can not enable 'mpi' in this distribution)
 fi]
)

if test "$USE_MPI" != "no"; then
	ACC_ADDTO(AM_CFLAGS, -DUSE_MPI)
	ACC_ADDTO(AM_CPPFLAGS, -DUSE_MPI)
fi

# debug MPI
e_debug_mpi="no"
AC_ARG_ENABLE(debug-mpi,
[  --enable-debug-mpi      asserts and other debugging checks for MPI @<:@no@:>@],
[if test "$have_cubicles_sources" = "yes"; then
   e_debug_mpi="yes"
   ACC_ADDTO(AM_CPPFLAGS, -DDEBUG_MPI)
 else
   AC_MSG_WARN(can not enable 'debug-mpi' in this distribution)
 fi]
)

# II_TYPE
ii_type="float"
AC_ARG_WITH(type,
[  --with-type=type        IntegralImage types: uint, @<:@float@:>@],
[if test "$have_cubicles_sources" = "yes"; then
   if test "$withval" = "float"; then
     ii_type=$withval
   elif test "$withval" = "uint"; then
     ii_type=$withval
   else
     AC_MSG_ERROR(invalid IntegralImage type: $withval)
   fi
 else
   AC_MSG_WARN(can not choose IntegralImage type in this distribution)
 fi]
)
if test "$ii_type" = "float"; then
     ACC_ADDTO(AM_CPPFLAGS, -DII_TYPE_FLOAT)
elif test "$ii_type" = "uint"; then
     ACC_ADDTO(AM_CPPFLAGS, -DII_TYPE_FLOAT) 
fi


# Checks for libraries.
# todo: check for libraries!!
#AC_CHECK_LIB([OpenCV], [cvCreateImage])

# Checks for general header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([float.h errno.h locale.h malloc.h stddef.h stdlib.h unistd.h])

# socket-related headers
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h sys/socket.h])

AC_MSG_CHECKING(for <string>)
AC_COMPILE_IFELSE([#include <string>], 
        AC_MSG_RESULT(yes), AC_MSG_FAILURE(not found!))
AC_MSG_CHECKING(for <vector>)
AC_TRY_CPP([#include <vector>],
        AC_MSG_RESULT(yes), AC_MSG_FAILURE(not found!))


# Checks for cubicles header and library
ACC_FIND_FILE_IN_DIRS(cubicles.h, 
        "$abs_top_srcdir/cubicles" \
        /usr/local/include, , cubicles_incdir)
INC_CUBICLES="-I$cubicles_incdir"
AC_SUBST(INC_CUBICLES)

if test "$have_cubicles_sources" = "yes"; then
ACC_ADDTO(AM_LDFLAGS, '-L$abs_top_srcdir/lib')
else
ACC_FIND_FILE_IN_DIRS(libcubicles.a,
        "$abs_top_srcdir/lib" \
	/usr/local/lib \
        "$abs_top_srcdir/cubicles", , cubicles_libdir)
ACC_ADDTO(AM_LDFLAGS, '-L$cubicles_libdir')
fi

# Checks for handvu header
if test "$have_handvu_sources" = "yes"; then
ACC_FIND_FILE_IN_DIRS(HandVu.h, 
        "$abs_top_srcdir/handvu" \
        /usr/local/include, , handvu_incdir)
INC_HANDVU="-I$handvu_incdir"
AC_SUBST(INC_HANDVU)
fi


# Checks for OpenCV headers
if test "$w_opencv" = "yes"; then

  # first try with pkg-config
  PKG_CHECK_MODULES(OPENCV, opencv >= 0.9.6, ,
        [echo "searching for OpenCV without pkg-config:"])
  INC_OPENCV="$OPENCV_CFLAGS"
  LIB_OPENCV="$OPENCV_LIBS"

  if test "x$INC_OPENCV" = "x"; then
    ACC_FIND_DIRS(OpenCV root,
        $inc_opencv \
        /usr/local/include/opencv \
        /sw/include/opencv \
        /cygdrive/c/Progra~1/opencv-0.9.6 \
        /cygdrive/c/Progra~1/OpenCV \
        /cygdrive/c/Progra~1/OpenCV_097 \
        /cygdrive/c/packages/opencv-0.9.6 \
        /cygdrive/c/packages/OpenCV_097 \
        /cygdrive/c/packages/OpenCV,
        opencv_rootdirs)

    ACC_FIND_FILE_IN_DIRS(cv.h, $opencv_rootdirs, cv/include, cv_incdir)
    ACC_FIND_FILE_IN_DIRS(cxcore.h, $opencv_rootdirs, cxcore/include, 
	cxcore_incdir)
    ACC_FIND_FILE_IN_DIRS(cvaux.h, $opencv_rootdirs, cvaux/include,
	cvaux_incdir)
    ACC_FIND_FILE_IN_DIRS(highgui.h, $opencv_rootdirs, otherlibs/highgui, 
	highgui_incdir)

    INC_OPENCV="-I$cv_incdir -I$cxcore_incdir -I$cvaux_incdir -I$highgui_incdir"
    # todo: we should check in library section
    case "$host" in
      *-cygwin*)
	ACC_FIND_FILE_IN_DIRS(cvcam.h, $opencv_rootdirs,
          otherlibs/cvcam/include, cvcam_incdir)
	INC_OPENCV="$INC_OPENCV -I$cvcam_incdir"
        ACC_FIND_FILE_IN_DIRS(cv.lib, $opencv_rootdirs, lib, cv_libdir)
        LIB_OPENCV="-L$cv_libdir -lcv -lcxcore -lcvaux -lhighgui"
        ;;
      *)
        # Assume Unix.
        ;;
    esac
  fi

  AC_SUBST(INC_OPENCV)
  AC_SUBST(LIB_OPENCV)

fi

# Checks for Magick headers
if test "$w_magick" = "yes"; then
  ACC_FIND_DIRS(Magick root,
	$inc_magick \
        '/usr/local/include' \
        '/usr/local/include/magick' \
        $HOME/Magick/ImageMagick-6.0.4 \
        /cygdrive/c/Progra~1/ImageMagick \
        /cygdrive/c/Progra~1/ImageMagick/6.0.0-Q16,
        magick_rootdirs)

  ACC_FIND_FILE_IN_DIRS(Magick++.h, $magick_rootdirs, include, magickpp_incdir)
  INC_MAGICK="-I$magickpp_incdir"
fi
AC_SUBST(INC_MAGICK)

# Checks for MPI headers
if test "$USE_MPI" = "mpich"; then
  ACC_FIND_DIRS(MPI mpich root,
        $inc_mpi \
        '/usr/local/include/mpi' \
        '/cluster/opt/mpi/include',
        mpich_rootdirs)

  ACC_FIND_FILE_IN_DIRS(MPI.h, $mpich_rootdirs, include, mpich_incdir)
  INC_MPI="-I$mpich_incdir"

  # libs are at /cluster/opt/mpi/lib
  # ACC_ADDTO(LIBS, -lmpich)
  CC=/cluster/opt/mpi/bin/mpiCC
  AC_SUBST(CC)
fi
AC_SUBST(INC_MPI)

if test "$USE_MPI" = "lam"; then
  ACC_FIND_DIRS(MPI lam root,
        '/usr/include/lam', 
        lam_rootdirs)

  ACC_FIND_FILE_IN_DIRS(MPI.h, $lam_rootdirs, include, lam_incdir)
  INC_MPI="-I$lam_incdir"

  # libs are at /usr/lib/lam/lib
  # ACC_ADDTO(LIBS, -l)
  CC=/usr/bin/mpiCC
  AC_SUBST(CC)
fi

# don't build libdc1394 interface on Windows,
# don't build CvCam interface on *nix
case "$host" in
  *-win32*)
    have_hvdc1394_sources="no"
    ;;
  *-cygwin* | *-mingw*)
    have_hvdc1394_sources="no"
    ;;
  *)
    # Assume Unix.
    ;;
esac

# Checks for headers/libraries required for libdc1394
if test "$have_hvdc1394_sources" = "yes"; then
  PKG_CHECK_MODULES(RAW1394, libraw1394 >= 1.1.0, ,
        [echo "searching for libraw1394 without pkg-config:"])

  ACC_FIND_FILE_IN_DIRS(libdc1394/dc1394_control.h,
	/usr /usr/local, 
	include, dc1394_incdir2, warn)

  if test "x$RAW1394_CFLAGS" = "x" || test "x$dc1394_incdir2" = "x"; then
	AC_MSG_WARN(libdc1394 headers not found - will not build hv_dc1394)
	have_hvdc1394_sources="no"
  fi
  AC_SUBST(RAW1394_CFLAGS)

  # X11 headers/library
  ACC_FIND_FILE_IN_DIRS(libX11.a, 
	/usr/X11R6 /usr/local/X11R6, 
	lib, dc1394_libdir, warn)

  if test "x$dc1394_libdir" = "x"; then
	AC_MSG_WARN(X11 libraries not found - will not build hv_dc1394)
	have_hvdc1394_sources="no"
  else
  	LIB_DC1394="$RAW1394_LIBS -L$dc1394_libdir"
	AC_SUBST(LIB_DC1394)
  fi

  AC_CHECK_HEADERS([X11/Xlib.h X11/Xutil.h X11/keysym.h])
fi
if test "$have_hvdc1394_sources" = "yes"; then
   SUBDIR_HVDC1394=hv_dc1394
   AC_SUBST(SUBDIR_HVDC1394)
fi


# Checks for ARtoolkit headers and library
if test "$have_hvartk_sources" = "yes"; then
  ACC_FIND_FILE_IN_DIRS(AR/ar.h, 
	$inc_artk \
	"$abs_top_srcdir/ARToolKit2.61" \
	$HOME/ARToolKit2.61 \
	/usr/local, 
	include, artk_incdir, warn)

  if test "x$artk_incdir" = "x"; then
	AC_MSG_WARN(ARToolKit headers not found - will not build hv_ARtk)
	have_hvartk_sources="no"
  else
  	INC_ARTK="-I$artk_incdir"
	AC_SUBST(INC_ARTK)
  fi

  ACC_FIND_FILE_IN_DIRS(libAR.a, 
	$inc_artk \
	"$abs_top_srcdir/ARToolKit2.61" \
	$HOME/ARToolKit2.61 \
	/usr/local, 
	lib, artk_libdir, warn)

  if test "x$artk_libdir" = "x"; then
	AC_MSG_WARN(ARToolKit libraries not found - will not build hv_ARtk)
	have_hvartk_sources="no"
  else
  	LIB_ARTK="-L$artk_libdir"
	AC_SUBST(LIB_ARTK)
  fi

  AC_CHECK_HEADERS([GL/gl.h GL/glut.h SDL/SDL.h])
fi
if test "$have_hvartk_sources" = "yes"; then
   SUBDIR_HVARTK=hv_ARtk
   AC_SUBST(SUBDIR_HVARTK)
fi


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
ifdef([m4_pattern_allow],
      [m4_pattern_allow([^AM_CFLAGS$])])dnl
#AC_C_RESTRICT
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ALLOCA
# the next two functions cause problems via hvconfig.h:
# they add: #define malloc rpl_malloc    etc.
# AC_FUNC_MALLOC
# AC_FUNC_REALLOC
AC_FUNC_SETVBUF_REVERSED
AC_CHECK_FUNCS([floor gethostname getcwd mkdir memset setlocale sqrt strrchr strstr isnan])
AC_CHECK_FUNCS([inet_ntoa socket strerror])
AC_TYPE_MODE_T
AC_HEADER_STAT
AC_FUNC_STAT

# generate output files
AC_CONFIG_FILES(Makefile)
if test "$have_cubicles_sources" = "yes"; then
AC_CONFIG_FILES(cubicles/Makefile)
fi
if test "$have_handvu_sources" = "yes"; then
AC_CONFIG_FILES(handvu/Makefile)
fi
if test "$have_hvopencv_sources" = "yes"; then
AC_CONFIG_FILES(hv_OpenCV/Makefile)
fi
if test "$have_hvcvcam_sources" = "yes"; then
AC_CONFIG_FILES(hv_CvCam/Makefile)
fi
if test "$have_hvdc1394_sources" = "yes"; then
AC_CONFIG_FILES(hv_dc1394/Makefile)
fi
if test "$have_hvartk_sources" = "yes"; then
AC_CONFIG_FILES(hv_ARtk/Makefile)
fi
if test "$have_hvdxapp_sources" = "yes"; then
AC_CONFIG_FILES(hv_DXApp/Makefile)
fi
if test "$have_hvdxfilter_sources" = "yes"; then
AC_CONFIG_FILES(hv_DXFilter/Makefile)
fi

AC_OUTPUT

echo "
configuration:
    build cubicles:           ${have_cubicles_sources}
    build handvu:             ${have_handvu_sources}
    build OpenCV demo:        ${have_hvopencv_sources}
    build CvCam demo:         ${have_hvcvcam_sources}
    build libdc1394 demo:     ${have_hvdc1394_sources}
    build ARtk demo:          ${have_hvartk_sources}
    have DXApp code:          ${have_hvdxapp_sources}
    have DXFilter code:       ${have_hvdxfilter_sources}
    host system:              ${host}
    compiler:                 ${CXX} 
    CFLAGS:                   ${AM_CFLAGS} ${CFLAGS} 
    CPPFLAGS:                 ${AM_CPPFLAGS} ${CPPFLAGS}
    CXXFLAGS:                 ${AM_CXXFLAGS} ${CXXFLAGS}
    install path:             ${prefix}
    use OpenCV:               ${w_opencv}
    use Magick:               ${w_magick}
    small skin color table:   ${e_small_color}
    debug:                    ${e_debug}"

if test "$have_cubicles_training_sources" = "yes"; then
echo "configuration for cubicles training:
    with training:            ${e_training}
    use MPI:                  ${USE_MPI}
    debug MPI:                ${e_debug_mpi}
    integral image type:      ${ii_type}"
fi

echo "now run make."
