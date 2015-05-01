/**
  * HandVu - a library for computer vision-based hand gesture
  * recognition.
  * Copyright (C) 2004 Mathias Kolsch, matz@cs.ucsb.edu
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place - Suite 330, 
  * Boston, MA  02111-1307, USA.
  *
  * $Id: Common.h,v 1.24 2006/01/08 19:32:11 matz Exp $
**/

#if !defined(__COMMON_H__INCLUDED_)
#define __COMMON_H__INCLUDED_




#if defined(WIN32)

// for Windows only: include all external headers here, they'll
// be put in the precompiled header file with quite a bit of
// compilation speedup.
// for all other platforms, however, include only what's necessary
// in each .cpp or maybe .h file


#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Visual C++ doesn't understand exception specifications of functions:
#if defined(_MSC_VER) && _MSC_VER<1400
//#define throw(x) /* throw(x) */
#endif

#pragma warning (disable:4312)
#include <cv.h>
#pragma warning (default:4312)
#include <cvaux.h>

#include <vector>
#include <math.h>
#include <float.h>
#include <fstream>
#include <string>
#include <ostream>
#pragma warning (disable: 4786)
#pragma warning (disable: 4702)
#include <map>
#include <list>
#pragma warning (default: 4786)
#pragma warning (default: 4702)
using namespace std;

#include <cubicles.h>
#include "Rect.h"

#include <malloc.h>
#define alloca _alloca

#define isnan _isnan



#endif // defined(WIN32)




/* for all platforms */

#if defined HAVE_CONFIG_H
#include "../hvconfig.h"
#endif

#include <stdio.h>



#ifdef DEBUG
#ifdef ASSERT
#undef ASSERT
#endif
#define ASSERT(x) \
  if (!(x)) { \
    printf("%s, line %d: assertion (%s) failed\n", __FILE__, __LINE__, #x); \
    throw HVException("assertion failed"); \
  }
#else  // DEBUG
#define ASSERT(x)  ((void)0)
#endif


extern int g_verbose; // defined in IntegralFeatures.cpp
extern FILE* g_ostream; // defined in IntegralFeatures.cpp

#if defined(DEBUG)
#define VERBOSE0(level, fmt) if (g_verbose>=level) {fprintf(g_ostream, fmt); fprintf(g_ostream, "\n"); fflush(g_ostream);}
#define VERBOSE1(level, fmt,x) if (g_verbose>=level) {fprintf(g_ostream, fmt,x); fprintf(g_ostream, "\n"); fflush(g_ostream);}
#define VERBOSE2(level, fmt,x,y) if (g_verbose>=level) {fprintf(g_ostream, fmt,x,y); fprintf(g_ostream, "\n"); fflush(g_ostream);}
#define VERBOSE3(level, fmt,x,y,z) if (g_verbose>=level) {fprintf(g_ostream, fmt,x,y,z); fprintf(g_ostream, "\n"); fflush(g_ostream);}
#define VERBOSE4(level, fmt,x,y,z,k) if (g_verbose>=level) {fprintf(g_ostream, fmt,x,y,z,k); fprintf(g_ostream, "\n"); fflush(g_ostream);}
#define VERBOSE5(level, fmt,x,y,z,k,l) if (g_verbose>=level) {fprintf(g_ostream, fmt,x,y,z,k,l); fprintf(g_ostream, "\n"); fflush(g_ostream);}
#define VERBOSE6(level, fmt,x,y,z,k,l,m) if (g_verbose>=level) {fprintf(g_ostream, fmt,x,y,z,k,l,m); fprintf(g_ostream, "\n"); fflush(g_ostream);}
#else // DEBUG
#define VERBOSE0(level, fmt) {}
#define VERBOSE1(level, fmt,x) {}
#define VERBOSE2(level, fmt,x,y) {}
#define VERBOSE3(level, fmt,x,y,z) {}
#define VERBOSE4(level, fmt,x,y,z,k) {}
#define VERBOSE5(level, fmt,x,y,z,k,l) {}
#define VERBOSE6(level, fmt,x,y,z,k,l,m) {}
#endif // DEBUG


#define HV_CONDUCTOR_VERSION_1_3_STRING "Integration-Templates VisionConductor file, version 1.3"
#define HV_CONDUCTOR_VERSION_1_4_STRING "Integration-Templates VisionConductor file, version 1.4"
#define HV_CONDUCTOR_VERSION_1_5_STRING "HandVu VisionConductor file, version 1.5"
#define HV_MASK_VERSION_1_0_STRING "HandVu Mask file, version 1.0"

// define the pi constant if needed
#ifndef M_PI
#define M_PI 3.141592653589793				 
//double pi = 4.0*atan(1.0);
#endif /* M_PI */



#ifdef WIN32
// in cubicles/StringUtils.cpp
string ConvertPathToWindows(const string& filename);
string ConvertPathToStandard(const string& win_filename);
void ReplaceAll(string& mangle, const string what, const string with);
#else
#define ConvertPathToWindows(x) (x)
#define ConvertPathToStandard(x) (x)
#endif // WIN32


typedef unsigned char BYTE;

typedef struct tagBGRtriple {
    BYTE blue;
    BYTE green; 
    BYTE red;
} ColorBGR; 


#endif // __COMMON_H__INCLUDED_
