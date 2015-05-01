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
  * $Id: Image.cpp,v 1.35 2004/11/24 08:41:13 matz Exp $
**/

// Image.cpp defines a bunch of simple image container functions
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
#include "Image.h"
#include "IntegralImage.h"
#include <math.h>


// this for loading and writing images only

#if defined(IMG_LIB_MAGICK)
#pragma warning (disable: 4251)
#pragma warning (disable: 4786)
#include <Magick++.h>
using namespace Magick;
#pragma warning (default: 4251)
#pragma warning (default: 4786)
#elif defined(IMG_LIB_OPENCV)
#include <cv.h>
#include <highgui.h>
#elif defined(IMG_LIB_NONE)
// nothing
#else
#error at least IMG_LIB_NONE must be defined
#endif




#ifdef _DEBUG
#ifdef USE_MFC
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif // USE_MFC
#endif // _DEBUG


#ifndef M_PI
#define M_PI 3.141592653589793
#endif /* M_PI */

#ifdef USE_MFC
#define PREPARE_DIB PrepareDIB()
#else // USE_MFC
#define PREPARE_DIB 
#endif // USE_MFC

#ifdef DEBUG
bool g_printlots = false;
#endif // DEBUG

#ifdef WIN32
#include <malloc.h>
#define alloca _alloca
#endif


// ----------------------------------------------------------------------
// class CByteImage
// ----------------------------------------------------------------------

CByteImage::CByteImage()
: m_width(0),
  m_height(0),
  m_pData(NULL),
  m_data_is_local(true)
{
  PREPARE_DIB;
}

CByteImage::CByteImage(int width, int height)
: m_width(0),
  m_height(0),
  m_pData(NULL),
  m_data_is_local(true)
{
  Allocate(width, height);
  PREPARE_DIB;
}

CByteImage::CByteImage(BYTE* pData, int width, int height)
: m_width(width),
  m_height(height),
  m_pData(pData),
  m_data_is_local(false)
{
  ASSERT (m_width>0 && m_height>0);
  PREPARE_DIB;
}

CByteImage::~CByteImage()
{
  Deallocate();
}

void CByteImage::Deallocate()
{
  if (m_data_is_local) {
    m_data.resize(0);
  }
  m_pData = NULL;
}

void CByteImage::Allocate(int width, int height)
{
  ASSERT(width>0 && height>0);
  if (m_data_is_local && (int)m_data.size()>=width*height) {
    m_width = width;
    m_height = height;
  } else {
    ASSERT(width>=0 && height>=0);
    m_width = width;
    m_height = height;
    m_data.resize(m_width*m_height);
    if (m_width*m_height>0) {
      m_pData = &m_data[0];
      ASSERT(m_pData);
    } else {
      m_pData = NULL;
    }
  }
  m_data_is_local = true;
}

void CByteImage::Copy(const CByteImage& from)
{
  Allocate(from.m_width, from.m_height);
  m_data = from.m_data;
}

void CByteImage::CopyArea(const CByteImage& from, const CRect& from_area, 
              int new_width, int new_height) 
{
  Allocate(new_width, new_height);
  double step_x = (double)from_area.right/(double)m_width;
  double step_y = (double)from_area.bottom/(double)m_height;
  double from_y = from_area.top;
  for (int y=0; y<m_height; y++) {
    double from_x = from_area.left;
    for (int x=0; x<m_width; x++) {
      Pixel(x, y) = (BYTE) from.InterpolateLinear(from_x, from_y);
      from_x += step_x;
    }
    from_y += step_y;
  }
}

#ifdef DEBUG
BYTE CByteImage::Pixel(int x, int y) const 
{
  if (!m_pData) {
    throw ITException("m_pData zero");
  }
  if (!(0<=x && x<m_width && 0<=y && y<m_height)) {
    throw ITException("out of bounds");
  }
  return m_pData[x + y*m_width];
}

BYTE& CByteImage::Pixel(int x, int y)
{
  if (!m_pData) {
    throw ITException("m_pData zero");
  }
  if (!(0<=x && x<m_width && 0<=y && y<m_height)) {
    throw ITException("out of bounds");
  }
  return m_pData[x + y*m_width];
}
#endif // DEBUG

#ifdef USE_MFC
void CByteImage::PrepareDIB()
{
  m_bmi.pMap[0] = 0xff000000;
  for (int color=1; color<256; color++) {
    m_bmi.pMap[color] = m_bmi.pMap[color-1]+0x010101;
  }
  m_bmi.header.biSize = sizeof(BITMAPINFOHEADER);
  m_bmi.header.biPlanes = 1;
  m_bmi.header.biBitCount = (WORD) 8;
  m_bmi.header.biCompression = BI_RGB;
  m_bmi.header.biSizeImage = 0;
  m_bmi.header.biXPelsPerMeter = 3150;
  m_bmi.header.biYPelsPerMeter = 3150;
  m_bmi.header.biClrUsed = 0;
  m_bmi.header.biClrImportant = 0;
}

void CByteImage::SetBitsToDevice(CDC* pDC) const
{
  m_bmi.header.biWidth = m_width;
  m_bmi.header.biHeight = -m_height;

  SetDIBitsToDevice(
    pDC->m_hDC,           // handle to DC
    0,                    // x-coord of destination upper-left corner
    0,                    // y-coord of destination upper-left corner 
    m_width,              // source rectangle width
    m_height,             // source rectangle height
    0,                    // x-coord of source lower-left corner
    0,                    // y-coord of source lower-left corner
    0,                    // first scan line in array
    m_height,             // number of scan lines
    m_pData,              // array of DIB bits
    (BITMAPINFO*) &m_bmi, // bitmap information
    DIB_RGB_COLORS        // RGB or palette indexes
    );
}
#endif // USE_MFC

void CByteImage::WriteToFile(FILE* fp)
{
  fprintf(fp, "%dx%d\n", m_width, m_height);
  for (int y=0; y<m_height; y++) {
    for (int x=0; x<m_width; x++) {
      fprintf(fp, "%d ", Pixel(x, y));
    }
    fprintf(fp, "\n");
  }
}

void CByteImage::ReadFromFile(FILE* fp)
{
  int width, height, scanned;
  scanned = fscanf(fp, "%dx%d\n", &width, &height);
  if (scanned!=2) {
    throw ITEFile("-", "file read error in CByteImage::ReadFromFile");
  }
  Allocate(width, height);

  for (int y=0; y<m_height; y++) {
    for (int x=0; x<m_width; x++) {
      int val;
      scanned = fscanf(fp, "%d ", &val);
      if (scanned!=1 || val<0 || 255<val) {
        char buf[128];
        sprintf(buf, 
          "file read error in CByteImage::ReadFromFile, at(%d, %d)", 
          x, y);
        throw ITEFile("-", buf);
      }
      Pixel(x, y) = (BYTE) val;
    }
  }
}

int iround(double f) {
  return (int)(f+0.5);
}

double CByteImage::InterpolateNearestNeighbor(double x, double y) const
{
  int rx = iround(x);
  if (rx<0 || m_width<=rx) {
    return 0;
  }
  int ry = iround(y);
  if (ry<0 || m_height<=ry) {
    return 0;
  }

  BYTE val = Pixel(rx, ry);
  return val;
}

double CByteImage::InterpolateLinear(double x, double y) const
{
  int left = (int) floor(x);
  if (left<-1 || m_width<=left) {
    return 0;
  }
  int top = (int) floor(y);
  if (top<-1 || m_height<=top) {
    return 0;
  }
  int right = left+1;
  int bottom = top+1;

  BYTE val_lt, val_lb, val_rt, val_rb;
  if (left==-1) {
    val_lt = 0;
    val_lb = 0;
  } else {
    if (top==-1) {
      val_lt = 0;
    } else {
      val_lt = Pixel(left, top);
    }
    if (bottom==m_height) {
      val_lb = 0;
    } else {
      val_lb = Pixel(left, bottom);
    }
  }
  if (right==m_width) {
    val_rt = 0;
    val_rb = 0;
  } else {
    if (top==-1) {
      val_rt = 0;
    } else {
      val_rt = Pixel(right, top);
    }
    if (bottom==m_height) {
      val_rb = 0;
    } else {
      val_rb = Pixel(right, bottom);
    }
  }

  double frac_x = x-(double)left;
  double frac_y = y-(double)top;
  double val = (1.0-frac_y)*((1.0-frac_x)*val_lt+frac_x*val_rt)
    + frac_y*((1.0-frac_x)*val_lb+frac_x*val_rb);
  return val;
}

void CByteImage::Rotate(double degrees, int xCenter, int yCenter)
{
  if (degrees==0.0) return;
  //#ifdef DEBUG
  if (xCenter<0 || xCenter>=m_width || yCenter<0 || yCenter>=m_height) {
    char* buf = (char*) alloca(512*sizeof(char));
    sprintf(buf, "%f %d %d %d %d\n", degrees, xCenter, yCenter, m_width, m_height);
    throw ITException(buf);
  }
  //#endif // DEBUG
  ASSERT(0<=xCenter && xCenter<m_width);
  ASSERT(0<=yCenter && yCenter<m_height);

  CByteImage old_img;
  old_img.Copy(*this);
  double rad = degrees*M_PI/180.0;
  double cos_rad = cos(rad);
  double sin_rad = sin(rad);
  for (int y=0; y<m_height; y++) {
    for (int x=0; x<m_width; x++) {
      double rel_x = x-xCenter;
      double rel_y = y-yCenter;
      double rot_x = rel_x*cos_rad - rel_y*sin_rad;
      double rot_y = rel_x*sin_rad + rel_y*cos_rad;
      double val = old_img.InterpolateLinear(xCenter+rot_x, yCenter+rot_y);
      ASSERT(0.0<=val && val<=255.0);
      Pixel(x, y) = (BYTE) val;
    }
  }  
}

void CByteImage::Crop(const CRect& area)
{
  ASSERT(area.left<area.right && area.top<area.bottom);
  ASSERT(0<=area.left && area.right<m_width);
  ASSERT(0<=area.top && area.bottom<m_height);

  int old_width = m_width;
  BYTE* pOldData = m_pData;
  if (!m_data_is_local) {
    Allocate(area.right-area.left, area.bottom-area.top);
    // previous call changed m_width and m_height also
  } else {
    m_width = area.right-area.left;
    m_height = area.bottom-area.top;
  }
  for (int y=0; y<m_height; y++) {
    for (int x=0; x<m_width; x++) {
      Pixel(x, y) = pOldData[(area.left+x) + (area.top+y)*old_width];
    }
  }
  m_data.resize(m_width*m_height);
}

/* this is not very efficient and should only be used for testing
*/
void CByteImage::Standardize(int ip_method)
{
  if (ip_method==0) {
    // equalize histogram
    ASSERT(0);
    /*
      IplImage* ipl_img = CreateIplImage();

      const int range = 256;
      IplLUT lut = { range+1, NULL,NULL,NULL, IPL_LUT_LOOKUP };
      IplLUT* plut = &lut;
      lut.key = new int[range+1];
      lut.value = new int[range];
      // Initialize the histogram levels
      for(int i=0; i<=range; i++) lut.key[i] = i;
      // Compute histogram
      iplComputeHisto( ipl_img, &plut );
      // Equalize histogram = rescale range of image data
      iplHistoEqualize( ipl_img, ipl_img, &plut );

      delete[] lut.key;
      delete[] lut.value;	
      DeleteIplImage(ipl_img);
    */

  } else if (ip_method==1) {
    // normalize image

    // find avg
    int sum=0;
    {	for (int y=0; y<m_height; y++) {
      for (int x=0; x<m_width; x++) {
        sum += Pixel(x, y);
      }
    }	}
    double N = m_height*m_width;
    double avg = (double)sum/N;

    // find stddev
    double varsum=0;
    {	for (int y=0; y<m_height; y++) {
      for (int x=0; x<m_width; x++) {
        double val = (double)Pixel(x, y)-avg;
        varsum += val*val;
      }
    }	}
    double stddev = sqrt(varsum/N);

    // normalize
    {	for (int y=0; y<m_height; y++) {
      for (int x=0; x<m_width; x++) {
        double newpix = ((double)Pixel(x, y)-avg) / stddev;
        newpix *= 127.5;
        newpix += 127.5;
        if (newpix<0) {
          Pixel(x, y) = 0;
        } else if (newpix>255) {
          Pixel(x, y) = 255;
        } else {					
          Pixel(x, y) = (BYTE) newpix;
        }
      }
    }	}

  } else if (ip_method==2) {
    // adjust image range

    BYTE min=255;
    BYTE max=0;
    {	for (int y=0; y<m_height; y++) {
      for (int x=0; x<m_width; x++) {
        BYTE pix = Pixel(x, y);
        if (max<pix) max=pix;
        if (min>pix) min=pix;
      }
    }	}
    double fac = 255.0/(double)(max-min);

    // move pixel values
    {	for (int y=0; y<m_height; y++) {
      for (int x=0; x<m_width; x++) {
        Pixel(x, y) = (BYTE) ((double)(Pixel(x, y)-min)*fac);
      }
    }	}

  } else {
    ASSERT(0);
  }
}





// load image from disk
void CByteImage::ImportFromFile(string filename) throw (ITEFile)
{
  if (filename.compare("")==0) {
    throw ITEFile("-", "no filename given");
  }
#ifdef WIN32
  filename = ConvertPathToWindows(filename);
#endif // WIN32

#if defined(IMG_LIB_MAGICK)

  try {
    Image tmpimg;
    tmpimg.read(filename.c_str());

    int width = tmpimg.baseColumns();
    int height = tmpimg.baseRows();
    
    const PixelPacket* pData = tmpimg.getConstPixels(0, 0, width, height);
    ASSERT(pData);

    Allocate(width, height);

    for (int y=0; y<height; y++) {
      for (int x=0; x<width; x++) {
        Pixel(x, y) =
          (BYTE) (0.212671 * (double)(pData->red>>(QuantumDepth-8)) +
                  0.715160 * (double)(pData->green>>(QuantumDepth-8)) +
                  0.072169 * (double)(pData->blue>>(QuantumDepth-8)));
        pData++;
      }
    }
  } catch (Magick::Exception &error) {
    throw ITEFile(filename,
		  string("exception during image reading with Magick:\n")
                  +error.what());
  }
    
#elif defined(IMG_LIB_OPENCV)
  // load image, don't change number of channels
  IplImage* img = cvLoadImage(filename.c_str(), -1);
  if (img==NULL) {
    throw ITEFile(filename, "load failed in OpenCV");
  }

  Allocate(img->width, img->height);

  if (img->nChannels==3) {
    CvSize size = cvSize(img->width, img->height);
    IplImage* gray = cvCreateImage(size, IPL_DEPTH_8U, 1);
    cvCvtColor(img, gray, CV_BGR2GRAY);
    cvReleaseImage(&img);
    img = gray;
  }

  if (img->nChannels==1) {
    ASSERT(m_pData);
    memcpy(img->imageData, m_pData, m_width*m_height*sizeof(BYTE));

  } else {
    throw ITEFile(filename, "can not import");
  }
  cvReleaseImage(&img);

#elif defined(IMG_LIB_NONE)
  throw ITEFile(filename, "file import functionality not available");

#else
#error IMG_LIB not defined - you must at least define IMG_LIB_NONE explicitely
#endif // IMG_LIB
}

void CByteImage::ExportToFile(string filename) throw (ITEFile)
{
  if (filename.compare("")==0) {
    throw ITEFile("-", "no filename given");
  }
#ifdef WIN32
  filename = ConvertPathToWindows(filename);
#endif // WIN32

#if defined(IMG_LIB_MAGICK)
  Image tmpimg(Geometry(m_width, m_height), Color("red"));
  tmpimg.modifyImage();
  PixelPacket* pTmpPixel = tmpimg.getPixels(0, 0, m_width, m_height);

  for (int y=0; y<m_height; y++) {
    for (int x=0; x<m_width; x++) {
      BYTE p = Pixel(x, y);
      pTmpPixel->red = p;
      pTmpPixel->green = p;
      pTmpPixel->blue = p;
      pTmpPixel++;
    }
  }
  tmpimg.type(GrayscaleType);
  tmpimg.syncPixels();

  try {
    tmpimg.write(filename.c_str());
  } catch (Exception &error) {
    throw ITEFile(filename,
		  string("exception in Magick.read: ")+error.what());
  }

#else // IMG_LIB
  throw ITEFile(filename, "file export functionality not available");
#endif // IMG_LIB
}


