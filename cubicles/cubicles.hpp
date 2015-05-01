/**
  * cubicles
  *
  * This is an implementation of the Viola-Jones object detection 
  * method and some extensions.  The code is mostly platform-
  * independent and uses only standard C and C++ libraries.  It
  * can make use of MPI for parallel training and a few Windows
  * MFC functions for classifier display.
  *
  * Mathias Kolsch, matz@cs.ucsb.edu
  *
  * $Id: cubicles.hpp,v 1.4 2004/11/24 20:45:36 matz Exp $
**/

// cubicles.hpp is little more than the file through which VC++
// generates precompiled headers. 
//

////////////////////////////////////////////////////////////////////
//
// By downloading, copying, installing or using the software you 
// agree to this license.  If you do not agree to this license, 
// do not download, install, copy or use the software.
//
// Copyright (C) 2004, Mathias Kolsch, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in binary form, with or without 
// modification, is permitted for non-commercial purposes only.
// Redistribution in source, with or without modification, is 
// prohibited without prior written permission.
// If granted in writing in another document, personal use and 
// modification are permitted provided that the following two
// conditions are met:
//
// 1.Any modification of source code must retain the above 
//   copyright notice, this list of conditions and the following 
//   disclaimer.
//
// 2.Redistribution's in binary form must reproduce the above 
//   copyright notice, this list of conditions and the following 
//   disclaimer in the documentation and/or other materials provided
//   with the distribution.
//
// This software is provided by the copyright holders and 
// contributors "as is" and any express or implied warranties, 
// including, but not limited to, the implied warranties of 
// merchantability and fitness for a particular purpose are 
// disclaimed.  In no event shall the copyright holder or 
// contributors be liable for any direct, indirect, incidental, 
// special, exemplary, or consequential damages (including, but not 
// limited to, procurement of substitute goods or services; loss of 
// use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict 
// liability, or tort (including negligence or otherwise) arising 
// in any way out of the use of this software, even if advised of 
// the possibility of such damage.
//
////////////////////////////////////////////////////////////////////


#if !defined(__CUBICLES_HPP_INCLUDED_)
#define __CUBICLES_HPP_INCLUDED_

#if defined(WIN32)

// for Windows only: include all external headers here, they'll
// be put in the precompiled header file with quite a bit of
// compilation speedup.
// for all other platforms, however, include only what's necessary
// in each .cpp or maybe .h file

#pragma once
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// Windows Header Files, both for using MFC and for not using MFC
#ifdef USE_MFC

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#include <afxtempl.h>		// MFC templates
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#else // USE_MFC

#include <windows.h>

#endif // USE_MFC



#pragma warning (disable : 4786)
#include <string>
#include <iostream>
#include <sstream>
#include <list>
#include <vector>
#include <fstream>
#pragma warning (default: 4786)

#ifdef USE_MFC
#else // USE_MFC
#include <float.h>
#endif // USE_MFC


#if _MSC_VER<=1300
using namespace std;
#else
// this prompts a bug in VC++ (friends and private member access)
// thus we have to declare all usages individually:
using std::string;
using std::istream;
using std::ostream;
using std::endl;
using std::list;
using std::vector;
#endif // COMPILER VC++


//#include <limits.h>
#include <limits>


#endif // defined(WIN32)



/* for all platforms */

#if defined HAVE_CONFIG_H
#include "../hvconfig.h"
#endif

#define CU_CURRENT_VERSION_STRING "cubicles version 1.4"


#endif // __CUBICLES_HPP_INCLUDED_
