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
  * $Id: cubicles.cpp,v 1.21 2005/10/28 17:47:04 matz Exp $
**/

// cubicles.cpp: Implements the public C interface to the cubicles
// library.  Also needed for Windows DLL creation as it defines the
// entry point for the DLL application.
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

#include "cubicles.hpp"
#include "IntegralImage.h"
#include "Cascade.h"
#include "Scanner.h"

#if defined (IMG_LIB_OPENCV)
#include "cubicles.h"
#else
#error cubicles.cpp is currently only implemented with OpenCV
#endif

#if defined(WIN32)
BOOL APIENTRY DllMain( HANDLE /*hModule*/, 
                       DWORD  /*ul_reason_for_call*/, 
                       LPVOID /*lpReserved*/
					 )
{
    return TRUE;
}
#endif // WIN32

#ifdef __cplusplus
extern "C" {
#endif

//
// global variables
//
CCascadeVector                g_cu_cascades;
CScannerVector                g_cu_scanners;

CIntegralImage                g_cu_integral;
CIntegralImage                g_cu_squared_integral;

int                           g_cu_image_width = -1;
int                           g_cu_image_height = -1;
CRect                         g_cu_bbox;

int                           g_cu_min_width = -1;
int                           g_cu_max_width = -1;
int                           g_cu_min_height = -1;
int                           g_cu_max_height = -1;

// make sure cascadeID is typedef'ed as "unsigned int" or change this:
#define CHECK_CASCADE_ID \
  if (g_cu_cascades.size()<=cascadeID) { \
    CV_ERROR(CV_StsBadArg, "invalid cascadeID"); \
  }


/**
 * Initialize
 */
void cuInitialize(int image_width, int image_height)
{
  CV_FUNCNAME( "cuInitialize" ); // declare cvFuncName
  __BEGIN__;
  if (image_width<0 || image_height<0) {
    CV_ERROR(CV_BadImageSize, "negative image width or height");
  }
  try {
    g_cu_integral.SetSize(image_width, image_height);
    g_cu_squared_integral.SetSize(image_width, image_height);
  } catch (ITException& ite) {
    CV_ERROR(CV_StsError, ite.GetMessage().c_str());
  }
  // this serves as "initialized" flag
  g_cu_image_width = image_width;
  g_cu_image_height = image_height;
  __END__;
}

/**
 * Uninitialize -- call for garbage collection of global variables;
 * they are all stack-allocated, so this is not really necessary
 */
void cuUninitialize()
{
  CV_FUNCNAME( "cuUninitialize" ); // declare cvFuncName
  __BEGIN__;
  if (g_cu_image_width<0 || g_cu_image_height<0) {
    CV_ERROR(CV_StsError, "cubicles not initialized");
  }
  // clear out memory
  g_cu_cascades.clear();
  g_cu_scanners.clear();
  g_cu_integral.~CIntegralImage();
  g_cu_squared_integral.~CIntegralImage();

  // this serves as "initialized" flag
  g_cu_image_width = -1;
  g_cu_image_height = -1;
  __END__;
}

void cuLoadCascade(const string& filename, CuCascadeID* pID)
{
  CV_FUNCNAME( "cuLoadCascade" ); // declare cvFuncName
  __BEGIN__;
  if (filename.length()==0) {
    CV_ERROR(CV_StsBadArg, "no file name specified");
  }
  if (pID==NULL) {
    CV_ERROR(CV_StsBadArg, "pID: invalid pointer");
  }
  try {
    CClassifierCascade cascade;
#if defined(WIN32)
    cascade.ParseFrom(ConvertPathToWindows(filename).c_str());
#else
    cascade.ParseFrom(filename.c_str());
#endif // WIN32
    
    CuCascadeID cascadeID = (CuCascadeID) g_cu_cascades.size();
    g_cu_cascades.push_back(cascade);
    CImageScanner scanner;
    g_cu_scanners.push_back(scanner);
    *pID = cascadeID;

  } catch (ITException& ite) {
    string msg = "error while parsing cascade from file ";
    msg = msg + filename + string(":\n") + ite.GetMessage();
    CV_ERROR(CV_StsError, msg.c_str());
  }
  __END__;
}

void cuGetCascadeProperties(CuCascadeID cascadeID, CuCascadeProperties& cp)
{
  CV_FUNCNAME( "cuGetCascadeProperties" ); // declare cvFuncName
  __BEGIN__;
  CHECK_CASCADE_ID;
  try {
    cp.cascadeID = cascadeID;
    cp.names = g_cu_cascades[cascadeID].GetNames();
    cp.template_width = g_cu_cascades[cascadeID].GetTemplateWidth();
    cp.template_height = g_cu_cascades[cascadeID].GetTemplateHeight();
    cp.image_area_ratio = g_cu_cascades[cascadeID].GetImageAreaRatio();
  } catch (ITException& ite) {
    CV_ERROR(CV_StsError, ite.GetMessage().c_str());
  }
  __END__;
}



void cuGetScannerParameters(CuCascadeID cascadeID, CuScannerParameters& sp)
{
  CV_FUNCNAME( "cuGetScannerParameters" ); // declare cvFuncName
  __BEGIN__;
  CHECK_CASCADE_ID;
  try {
    CRect area;
    g_cu_scanners[cascadeID].GetScanParameters(&sp.start_scale,
                                            &sp.stop_scale,
                                            &sp.scale_inc_factor,
                                            &sp.translation_inc_x,
                                            &sp.translation_inc_y,
                                            area,
                                            &sp.post_process,
                                            &sp.active);
    sp.left = area.left;
    sp.top = area.top;
    sp.right = area.right;
    sp.bottom = area.bottom;
  } catch (ITException& ite) {
    CV_ERROR(CV_StsError, ite.GetMessage().c_str());
  }
  __END__;
}

void cuSetScannerParameters(CuCascadeID cascadeID, const CuScannerParameters& sp)
{
  CV_FUNCNAME( "cuSetScannerParameters" ); // declare cvFuncName
  __BEGIN__;
  CHECK_CASCADE_ID;
  try {
    CRect area(sp.left, sp.top, sp.right, sp.bottom);
    g_cu_scanners[cascadeID].SetActive(sp.active);
    g_cu_scanners[cascadeID].SetScanParameters(sp.start_scale,
                                           sp.stop_scale,
                                           sp.scale_inc_factor,
                                           sp.translation_inc_x,
                                           sp.translation_inc_y,
                                           area);
    g_cu_scanners[cascadeID].SetAutoPostProcessing(sp.post_process);
  } catch (ITException& ite) {
    CV_ERROR(CV_StsError, ite.GetMessage().c_str());
  }
  __END__;
}

void cuSetScannerActive(CuCascadeID cascadeID, bool active)
{
  CV_FUNCNAME( "cuSetScannerActive" ); // declare cvFuncName
  __BEGIN__;
  CHECK_CASCADE_ID;
  try {
    g_cu_scanners[cascadeID].SetActive(active);
  } catch (ITException& ite) {
    CV_ERROR(CV_StsError, ite.GetMessage().c_str());
  }
  __END__;
}

void cuSetScanArea(CuCascadeID cascadeID, int left, int top, int right, int bottom)
{
  CV_FUNCNAME( "cuSetScanArea" ); // declare cvFuncName
  __BEGIN__;
  CHECK_CASCADE_ID;
  try {
    CRect area(left, top, right, bottom);
    g_cu_scanners[cascadeID].SetScanArea(area);
  } catch (ITException& ite) {
    CV_ERROR(CV_StsError, ite.GetMessage().c_str());
  }
  __END__;
}

void cuSetScanScales(CuCascadeID cascadeID, double start_scale, double stop_scale)
{
  CV_FUNCNAME( "cuSetScanScales" ); // declare cvFuncName
  __BEGIN__;
  CHECK_CASCADE_ID;
  try {
    g_cu_scanners[cascadeID].SetScanScales(start_scale,
                                           stop_scale);
  } catch (ITException& ite) {
    CV_ERROR(CV_StsError, ite.GetMessage().c_str());
  }
  __END__;
}

void cuScan(const IplImage* grayImage, CuScanMatchVector& matches)
{
  CV_FUNCNAME( "cuScan" ); // declare cvFuncName
  __BEGIN__;
  if (g_cu_image_width<=0 || g_cu_image_height<=0) {
    CV_ERROR(CV_StsError, "cubicles has not been initialized");
  }
  if (grayImage==NULL) {
    CV_ERROR(CV_HeaderIsNull, "grayImage");
  }
  if (grayImage->nChannels!=1) {
    CV_ERROR(CV_BadNumChannels, "can only scan gray-level images");
  }
  if (grayImage->depth!=IPL_DEPTH_8U) {
    CV_ERROR(CV_BadDepth, "can only scan unsigned byte images");
  }
  if (grayImage->origin!=0) {
    CV_ERROR(CV_BadOrigin, "need image origin in top left corner");
  }
  if (grayImage->width!=g_cu_image_width 
      || grayImage->height!=g_cu_image_height) {
    CV_ERROR(CV_BadImageSize, "different from initialization");
  }
  try {
    g_cu_bbox = CRect(-1, -1, -1, -1);
    matches.clear();

    // todo: maybe sometime we should allow a maximum processing time
    // in order to guarantee a certain max latency. On the next call
    // to Process, we would pick up where we left off, e.g. at a
    // certain scanner with a certain scale
    
    // find bounding box around all scanners' scan_areas and
    // integrate image only within that bbox
    int num_cascades = (int) g_cu_cascades.size();
    CRect bbox = CRect(INT_MAX, INT_MAX, 0, 0);
    for (int sc=0; sc<num_cascades; sc++) {
      if (g_cu_scanners[sc].IsActive()) {
        const CRect& scan_area = g_cu_scanners[sc].GetScanArea();
        if (scan_area.left<bbox.left) bbox.left = scan_area.left;
        if (scan_area.right>bbox.right) bbox.right = scan_area.right;
        if (scan_area.top<bbox.top) bbox.top = scan_area.top;
        if (scan_area.bottom>bbox.bottom) bbox.bottom = scan_area.bottom;
      }
    }
    bbox.left = max(0, bbox.left);
    bbox.top = max(0, bbox.top);
    bbox.right = min(bbox.right, grayImage->width);
    bbox.bottom = min(bbox.bottom, grayImage->height);
    
    if (bbox.right-bbox.left<=0 || bbox.bottom-bbox.top<=0) {
      return;
    }
  
    CByteImage byteImage((BYTE*)grayImage->imageData,
                         grayImage->width,
                         grayImage->height);
    CIntegralImage::CreateSimpleNSquaredFrom(byteImage,
                                             g_cu_integral,
                                             g_cu_squared_integral, bbox);
    
    int num_active = 0;
    CScanMatchMatrix events;
    events.resize(num_cascades);
    for (int numc=0; numc<num_cascades; numc++) {
      if (g_cu_scanners[numc].IsActive()) {
        num_active++;
        ASSERT(g_cu_cascades[numc].GetNumStrongClassifiers()>0);

        // do the scan!
        g_cu_scanners[numc].Scan(g_cu_cascades[numc],
                                 g_cu_integral, g_cu_squared_integral,
                                 events[numc]);

        // this is a bit awkward and really not elegant, but we avoid
        // exposing all sorts of internal structures
        for (CScanMatchVector::const_iterator cm = events[numc].begin();
             cm!=events[numc].end();
             cm++)
        {
          CuScanMatch m;
          m.name = cm->name;
          m.left = cm->left;
          m.top = cm->top;
          m.right = cm->right;
          m.bottom = cm->bottom;
          m.scale = cm->scale;
          m.scale_x = cm->scale_x;
          m.scale_y = cm->scale_y;
          matches.push_back(m);
        }

	// must be called after the actual scan, and the behavior with
	// multiple active scanners is somewhat undetermined
	g_cu_scanners[numc].GetScaleSizes(&g_cu_min_width, &g_cu_max_width, 
					  &g_cu_min_height, &g_cu_max_height);
      }
    }
    if (num_active>0) {
      g_cu_bbox = bbox;
    }
    
  } catch (ITException& ite) {
    CV_ERROR(CV_StsError, ite.GetMessage().c_str());
  }
  __END__;
} // Scan

void cuGetScannedArea(int* pLeft, int* pTop, int* pRight, int* pBottom)
{
  CV_FUNCNAME( "cuGetScannedArea" ); // declare cvFuncName
  __BEGIN__;
  if (pLeft==NULL || pTop==NULL || pRight==NULL || pBottom==NULL) {
    CV_ERROR(CV_StsBadArg, "null pointer");
  }
  *pLeft = g_cu_bbox.left;
  *pTop = g_cu_bbox.top;
  *pRight = g_cu_bbox.right;
  *pBottom = g_cu_bbox.bottom;
  __END__;
}

void cuGetScaleSizes(int* min_width, int* max_width,
		     int* min_height, int* max_height)
{
  CV_FUNCNAME( "cuGetScaleSizes" ); // declare cvFuncName
  __BEGIN__;
  if (min_width==NULL || max_width==NULL ||
      min_height==NULL || max_height==NULL) {
    CV_ERROR(CV_StsBadArg, "null pointer");
  }
  *min_width = g_cu_min_width;
  *max_width = g_cu_max_width;
  *min_height = g_cu_min_height;
  *max_height = g_cu_max_height;
  __END__;
}

/** verbosity: 0 minimal, 3 maximal
*/
void cuGetVersion(string& version, int verbosity)
{
  CV_FUNCNAME( "cuGetScannedArea" ); // declare cvFuncName
  __BEGIN__;
  if (verbosity<0 || 3<verbosity) {
    CV_ERROR(CV_StsBadArg, "invalid verbosity");
  }

  version = "cubicles "CU_CURRENT_VERSION_STRING;
  if (verbosity>=1) {
#if defined(WIN32)
    version = version + ", win32";
#elif defined(TARGET_SYSTEM)
    version = version + ", "TARGET_SYSTEM;
#else
#error TARGET_SYSTEM must be defined
#endif

#if defined(DEBUG)
    version = version + " debug";
#endif

#if defined(II_TYPE_FLOAT)
    version = version + ", float";
#elif defined(II_TYPE_DOUBLE)
    version = version + ", double";
#elif defined(II_TYPE_INT)
    version = version + ", int";
#elif defined(II_TYPE_UINT)
    version = version + ", uint";
#else 
#error you must define II_TYPE
#endif

#if defined(WITH_TRAINING)
    version = version + ", training-enabled";
#endif

#if defined(IMG_LIB_MAGICK)
    version = version + ", Magick";
#elif defined(IMG_LIB_OPENCV)
    version = version + ", OpenCV";
#elif defined(IMG_LIB_NONE)
    version = version + ", no image library";
#else
#error at least IMG_LIB_NONE must be defined
#endif
  }

  if (verbosity>=2) {
    version = version + ", built on "__DATE__" at "__TIME__;
  }

  if (verbosity>=3) {
    version = version + "\nCVS id: $Id: cubicles.cpp,v 1.21 2005/10/28 17:47:04 matz Exp $";
    
    /* todo: OpenCV info
     */
    cvUseOptimized(0);
    int num_loaded_functions = cvUseOptimized(1);
    printf("OpenCV optimized functions: %d\n", num_loaded_functions);
    const char *pVersion, *pPlugins;
    cvGetModuleInfo(NULL, &pVersion, &pPlugins);
    printf("OpenCV version: %s\n", pVersion);
    printf("OpenCV plugins: %s\n", pPlugins);
  }

  __END__;
}

#ifdef __cplusplus
}
#endif
