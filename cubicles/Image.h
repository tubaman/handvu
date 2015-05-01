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
  * $Id: Image.h,v 1.24 2005/01/06 06:53:53 matz Exp $
**/

// Image.h: declarations for a simple image container class
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

#ifndef __IMAGE_H
#define __IMAGE_H

#include <stdio.h>
#include <string>
#include <vector>

#include "Exceptions.h"
#include "Rect.h"

#ifdef DEBUG
#ifdef ASSERT
#undef ASSERT
#endif
#define ASSERT(x) \
  if (!(x)) { \
    printf("%s, line %d: assertion (%s) failed\n", __FILE__, __LINE__, #x); \
    throw ITException("assertion failed"); \
  }
#else  // DEBUG
#define ASSERT(x)  ((void)0)
#endif

#ifdef DEBUG
extern bool g_printlots;
#endif // DEBUG

// ----------------------------------------------------------------------
// class CByteImage
// ----------------------------------------------------------------------

//namespace { // cubicles

typedef unsigned char BYTE;
typedef vector<BYTE> CBYTEVector;

class CByteImage {
 public:
  CByteImage();
  CByteImage(int width, int height);
  CByteImage(BYTE* pData, int width, int height);
  ~CByteImage();

  void Allocate(int width, int height);
  void Deallocate();
  void Copy(const CByteImage& from);
  void CopyArea(const CByteImage& from, const CRect& from_area, 
    int new_width, int new_height);

  int Width() const {return m_width;}
  int Height() const {return m_height;}
  const BYTE* GetData() const {return m_pData;}
  BYTE Pixel(int x, int y) const;
  BYTE& Pixel(int x, int y);
  
  void WriteToFile(FILE* fp);
  void ReadFromFile(FILE* fp);
  void ImportFromFile(string filename) throw (ITEFile);
  void ExportToFile(string filename) throw (ITEFile);
  void Rotate(double degrees, int xCenter, int yCenter);
  void Crop(const CRect& area);
  double InterpolateNearestNeighbor(double x, double y) const;
  double InterpolateLinear(double x, double y) const;
  void Standardize(int ip_method);

#ifdef USE_MFC
	mutable struct {
		BITMAPINFOHEADER header;
		unsigned long pMap[256];
	} m_bmi;
  void PrepareDIB();
  void SetBitsToDevice(CDC* pDC) const;
#endif // USE_MFC
  
 private:
  CBYTEVector  m_data;
  BYTE*  m_pData; // always must point to &m_data[0] or NULL
  bool   m_data_is_local;
  int    m_width;
  int    m_height;
};

// inlined function
#ifndef DEBUG
inline BYTE CByteImage::Pixel(int x, int y) const {return m_pData[x + y*m_width];}
inline BYTE& CByteImage::Pixel(int x, int y) {return m_pData[x + y*m_width];}
#endif


/////////////////////////////////////////////////////////////////////////////
// Helper functions, implementation in StringUtils.cpp

void getline_crlf(istream& is, string& line);
void ReplaceAll(string& mangle, const string what, const string with);
void RemoveCR(string& str);

#ifdef WIN32
// in cubicles/StringUtils.cpp
string ConvertPathToWindows(const string& filename);
string ConvertPathToStandard(const string& win_filename);
#endif // WIN32

//} // namespace cubicles

#endif // __IMAGE_H


