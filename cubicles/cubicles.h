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
  * $Id: cubicles.h,v 1.13 2005/01/07 23:20:52 matz Exp $
**/

// cubicles.h contains the public C bindings and hides all other
// implementation details.
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


#if !defined(__CUBICLES_H__INCLUDED__)
#define __CUBICLES_H__INCLUDED__

#include <cv.h>
#include <string>
#include <vector>
using namespace std;



#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int CuCascadeID;

typedef struct _CuCascadeProperties {
  CuCascadeID        cascadeID;
  vector<string>     names;
  int                template_width, template_height;
  double             image_area_ratio;
} CuCascadeProperties;

typedef struct _CuScannerParameters {
  bool               active;
  int                left, top, right, bottom;
  double             start_scale, stop_scale, scale_inc_factor;
  double             translation_inc_x, translation_inc_y;
  bool               post_process;
} CuScannerParameters;

typedef struct _CuScanMatch {
  int                left, top, right, bottom;
  double             scale, scale_x, scale_y;
  string             name;
} CuScanMatch;

typedef vector<CuScanMatch> CuScanMatchVector;



void cuInitialize(int image_width, int image_height);

void cuUninitialize();

void cuLoadCascade(const string& filename, CuCascadeID* pID);

void cuGetCascadeProperties(CuCascadeID cascadeID, CuCascadeProperties& cp);

void cuGetScannerParameters(CuCascadeID cascadeID, CuScannerParameters& sp);

void cuSetScannerParameters(CuCascadeID cascadeID, const CuScannerParameters& sp);

void cuSetScannerActive(CuCascadeID cascadeID, bool active);

void cuSetScanArea(CuCascadeID cascadeID, int left, int top, int right, int bottom);

void cuSetScanScales(CuCascadeID cascadeID, double start_scale, double stop_scale);

void cuGetScaleSizes(int* min_width, int* max_width,
		     int* min_height, int* max_height);

/** Scan a gray-level image,
 *  returns the resulting matches in the ScanMatchVector
 */
void cuScan(const IplImage* pImage, CuScanMatchVector& matches);

/** *pLeft is set to -1 of no scanner was active
 */
void cuGetScannedArea(int* pLeft, int* pTop, int* pRight, int* pBottom);

/** verbosity: 0 minimal, 3 maximal
*/
void cuGetVersion(string& version, int verbosity);

#ifdef __cplusplus
}
#endif


#endif // !defined(__CUBICLES_H__INCLUDED__)
