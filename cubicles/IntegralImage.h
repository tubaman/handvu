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
  * $Id: IntegralImage.h,v 1.71 2005/02/12 02:00:59 matz Exp $
**/

// IntegralImage is the basic data type for training and 
// recognition: a simple accumulative matrix generated from a 
// gray-level image, akin a "data cube" as known in the database
// community.
// This file includes IntegralImage.cxx
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


#if !defined(__INTEGRALIMAGE_H__INCLUDED_)
#define __INTEGRALIMAGE_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifdef WIN32
#define isnan _isnan
#endif // WIN32


//namespace {  // cubicles

extern int g_verbose; // defined in IntegralFeatures.cpp
extern FILE* g_ostream; // defined in IntegralFeatures.cpp

#if defined(DEBUG) && defined(WIN32_todo)
#define VERBOSE0(level, fmt) DbgLog((LOG_CUSTOM1, level, fmt))
#define VERBOSE1(level, fmt,x) DbgLog((LOG_CUSTOM1, level, fmt,x))
#define VERBOSE2(level, fmt,x,y) DbgLog((LOG_CUSTOM1, level, fmt,x,y))
#define VERBOSE3(level, fmt,x,y,z) DbgLog((LOG_CUSTOM1, level, fmt,x,y,z))
#define VERBOSE4(level, fmt,x,y,z,k) DbgLog((LOG_CUSTOM1, level, fmt,x,y,z,k))
#define VERBOSE5(level, fmt,x,y,z,k,l) DbgLog((LOG_CUSTOM1, level, fmt,x,y,z,k,l))
#define VERBOSE6(level, fmt,x,y,z,k,l,m) DbgLog((LOG_CUSTOM1, level, fmt,x,y,z,k,l,m))
#else // DEBUG && WIN32
#define VERBOSE0(level, fmt) if (g_verbose>=level) {fprintf(g_ostream, fmt); fprintf(g_ostream, "\n"); fflush(g_ostream);}
#define VERBOSE1(level, fmt,x) if (g_verbose>=level) {fprintf(g_ostream, fmt,x); fprintf(g_ostream, "\n"); fflush(g_ostream);}
#define VERBOSE2(level, fmt,x,y) if (g_verbose>=level) {fprintf(g_ostream, fmt,x,y); fprintf(g_ostream, "\n"); fflush(g_ostream);}
#define VERBOSE3(level, fmt,x,y,z) if (g_verbose>=level) {fprintf(g_ostream, fmt,x,y,z); fprintf(g_ostream, "\n"); fflush(g_ostream);}
#define VERBOSE4(level, fmt,x,y,z,k) if (g_verbose>=level) {fprintf(g_ostream, fmt,x,y,z,k); fprintf(g_ostream, "\n"); fflush(g_ostream);}
#define VERBOSE5(level, fmt,x,y,z,k,l) if (g_verbose>=level) {fprintf(g_ostream, fmt,x,y,z,k,l); fprintf(g_ostream, "\n"); fflush(g_ostream);}
#define VERBOSE6(level, fmt,x,y,z,k,l,m) if (g_verbose>=level) {fprintf(g_ostream, fmt,x,y,z,k,l,m); fprintf(g_ostream, "\n"); fflush(g_ostream);}
#endif // DEBUG

#define ITTS_VERSION_1_0_STRING "Integration-Templates TrainingSet file, version 1.0"
#define ITTS_VERSION_1_1_STRING "Integration-Templates TrainingSet file, version 1.1"
#define ITTS_VERSION_1_2_STRING "Integration-Templates TrainingSet file, version 1.2"
#define ITTS_VERSION_1_3_STRING "Integration-Templates TrainingSet file, version 1.3"

#define IT_INTEGRALS_VERSION_1_3_STRING "Integration-Templates TrainingIntegrals file, version 1.3"
#define IT_CONDUCTOR_VERSION_1_3_STRING "Integration-Templates VisionConductor file, version 1.3"
#define IT_CONDUCTOR_VERSION_1_4_STRING "Integration-Templates VisionConductor file, version 1.4"

typedef vector<int> CIntVector;
typedef vector<CIntVector> CIntMatrix;
typedef vector<double> CDoubleVector;
typedef vector<CDoubleVector> CDoubleMatrix;
typedef vector<float> CFloatVector;
typedef vector<CRect> CRectVector;


/////////////////////////////////////////////////////////////////////////////
// CIntegralImageT
/////////////////////////////////////////////////////////////////////////////

template<class TYPE>
class CIntegralImageT
{
  // Construction
 public:
  CIntegralImageT();
  ~CIntegralImageT();
  
  // Operations
 public:
  TYPE operator[](int i) const;
  TYPE GetElement(int col, int row) const;
  void SetElement(int col, int row, TYPE val);
  void IncElement(int col, int row, TYPE inc);
  void CreateFrom(const CByteImage& image, bool normalize);
  static void CreateSimpleNSquaredFrom(const CByteImage& image,
                                       CIntegralImageT<TYPE>& integral,
                                       CIntegralImageT<TYPE>& squared_integral,
                                       const CRect& roi);
  void SetSize(int width, int height);
  int GetWidth() const { return m_width; }
  int GetHeight() const { return m_height; }
  
  // raw data access, don't use this unless you implemented
  // CIntegralImageT yourself;
  // functions for MPI serialization
  const TYPE* GetRawData() const { return m_pPaddedData; }
  TYPE* GetRawData() { return m_pPaddedData; }
  int GetRawDataLen() const { return m_arraylen;}
  
  template< class TYPE2 >
    friend ostream& 
    operator<< (ostream& os, const CIntegralImageT<TYPE2>& clsf);
  
  // Implementation
 protected:
  TYPE*				m_pData;
  TYPE*				m_pPaddedData;
  int				m_width;
  int				m_padded_width;
  int				m_height;
  int                           m_arraylen;
};

// NOTE: the matrix is padded with one row of zeros on top of the
// actual matrix, and one column of zeros to the left of the actual
// matrix. this allows us to take col==-1 || row==-1 and return zero
// without a check. This padded datastructure is not visible to
// the outside.

#ifndef DEBUG  // debug version in CIntegralImage.cpp
/*
template<class TYPE>
inline TYPE CIntegralImageT::operator[](int i) const
{ return m_pData[i]; }
*/

template<class TYPE>
inline TYPE CIntegralImageT<TYPE>::GetElement(int col, int row) const
{ return m_pData[row*m_padded_width+col]; }

template<class TYPE>
inline void CIntegralImageT<TYPE>::SetElement(int col, int row, TYPE val) 
{ m_pData[row*m_padded_width+col]=val; }

template<class TYPE>
inline void CIntegralImageT<TYPE>::IncElement(int col, int row, TYPE inc) 
{ m_pData[row*m_padded_width+col]+=inc; }
#endif




/////////////////////////////////////////////////////////////////////////////
// CIntegralImage type instantiation
/////////////////////////////////////////////////////////////////////////////

// type of data in integration images:
#if defined(II_TYPE_FLOAT)
#define II_TYPE float
#elif defined(II_TYPE_DOUBLE)
#define II_TYPE double
#elif defined(II_TYPE_INT)
#define II_TYPE int
#elif defined(II_TYPE_UINT)
#define II_TYPE unsigned int
#else 
#error you must define II_TYPE
#endif

typedef CIntegralImageT<II_TYPE> CIntegralImage;

#include "IntegralImage.cxx"


//} // namespace cubicles

#endif // !defined(__INTEGRALIMAGE_H__INCLUDED__)
