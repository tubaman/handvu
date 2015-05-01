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
  * $Id: IntegralImage.cxx,v 1.29 2005/02/12 02:00:59 matz Exp $
**/

// IntegralImage is the basic data type for training and 
// recognition: a simple accumulative matrix generated from a 
// gray-level image, akin a "data cube" as known in the database
// community.
//
// IntegralImage.cxx is the C++ template class implementation file, 
// it is included from IntegralImage.h.
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


#include <math.h>
#include <ostream>

#ifdef USE_MFC
#ifdef _DEBUG
#ifndef new
#define new DEBUG_NEW
#endif // new
#endif // _DEBUG
#endif // USE_MFC


/////////////////////////////////////////////////////////////////////////////
// CIntegralImageT
/////////////////////////////////////////////////////////////////////////////

template<class TYPE>
CIntegralImageT<TYPE>::CIntegralImageT()
: m_width(0),
  m_height(0),
  m_padded_width(0),
  m_arraylen(0),
  m_pData(NULL),
  m_pPaddedData(NULL)
{
}

template<class TYPE>
CIntegralImageT<TYPE>::~CIntegralImageT()
{
  m_width = m_height = m_padded_width = m_arraylen = 0;
  delete[] m_pPaddedData;
  m_pPaddedData = NULL;
  m_pData = NULL;
}

/* re/allocate and zero-initialize space
 */
template<class TYPE>
void CIntegralImageT<TYPE>::SetSize(int width, int height)
{
  m_width = width;
  m_padded_width = width+1;
  m_height = height;
  int data_len = m_width*m_height;
  int padded_len = data_len+m_height+m_padded_width;
  if (padded_len>m_arraylen) {
    m_arraylen = padded_len;
    delete[] m_pPaddedData;
    ASSERT(m_arraylen>0);
    m_pPaddedData = new TYPE[m_arraylen];
    ASSERT(m_pPaddedData);
  }
  memset(m_pPaddedData, 0, padded_len*sizeof(TYPE));
  m_pData = &m_pPaddedData[m_padded_width+1];
}

/* allocates data structures and generates the integral matrix from
 * a gray image with TYPE-size elements.
 */
template<class TYPE>
void CIntegralImageT<TYPE>::CreateFrom(const CByteImage& image, bool normalize)
{
  m_width = image.Width();
  m_padded_width = m_width+1;
  m_height = image.Height();
  int data_len = m_width*m_height;
  int padded_len = data_len+m_height+m_padded_width;
  if (padded_len>m_arraylen) {
    m_arraylen = padded_len;
    delete[] m_pPaddedData;
    ASSERT(m_arraylen>0);
    m_pPaddedData = new TYPE[m_arraylen];
    ASSERT(m_pPaddedData);
  }
  m_pData = &m_pPaddedData[m_padded_width+1];
  memset(m_pPaddedData, 0, padded_len*sizeof(TYPE));

  if (normalize) {
    // find avg
    int sum=0;
    {	for (int y=0; y<m_height; y++) {
      for (int x=0; x<m_width; x++) {
        sum += image.Pixel(x, y);
      }
    }	}
    double N = (double)(m_height*m_width);
    ASSERT(N);
    double avg = (double)sum/N;

    // find stddev
    double varsum=0;
    {	for (int y=0; y<m_height; y++) {
      for (int x=0; x<m_width; x++) {
        double val = (double)image.Pixel(x, y)-avg;
        varsum += val*val;
      }
    }	}
    if (varsum==0) {
      // zero variance - make integral all zero
      for (int y=0; y<m_height; y++) {
        for (int x=0; x<m_width; x++) {
          SetElement(x, y, 0);
        }
      }
      return;
    }
    double stddev = sqrt(varsum/N);

    // normalization - folded into the integration computation
    // fill integral image for the entire matrix
    for (int y=0; y<m_height; y++) {
      for (int x=0; x<m_width; x++) {
        TYPE left = GetElement(x-1, y);
        TYPE upper = GetElement(x, y-1);
        TYPE diag = GetElement(x-1, y-1);
#if defined(II_TYPE_FLOAT) || defined(II_TYPE_DOUBLE)
        TYPE img = (TYPE) (((double)image.Pixel(x, y)-avg)/stddev);
#elif defined(II_TYPE_INT) || defined(II_TYPE_UINT)
        double newpix = ((double)image.Pixel(x, y)-avg)/stddev;
        newpix *= 127.5;
        newpix += 127.5;
        TYPE img;
        if (newpix<0) {
          img = 0;
        } else if (newpix>255) {
          img = 255;
        } else {					
          img = (TYPE) newpix;
        }
#else
#error need to define functionality for this II_TYPE
#endif // II_TYPE
        TYPE val = left+upper-diag+img;
        SetElement(x, y, val);
      }
    }
    
    
  } else {
    // don't normalize

    // fill integral image for the entire matrix
    for (int y=0; y<m_height; y++) {
      for (int x=0; x<m_width; x++) {
        TYPE left = GetElement(x-1, y);
        TYPE upper = GetElement(x, y-1);
        TYPE diag = GetElement(x-1, y-1);
        TYPE img = image.Pixel(x, y);
        TYPE val = left+upper-diag+img;
        SetElement(x, y, val);
      }
    }
  }
}

/* allocates data structures and generates the integral matrix from
 * a gray image with TYPE-size elements.
 */
template<class TYPE>
void CIntegralImageT<TYPE>::CreateSimpleNSquaredFrom(
   const CByteImage& image,
   CIntegralImageT<TYPE>& integral,
   CIntegralImageT<TYPE>& squared_integral,
   const CRect& roi)
{
  int width = integral.m_width = squared_integral.m_width = image.Width();
  ASSERT(width);
  int padded_width = integral.m_padded_width = squared_integral.m_padded_width =
    width+1;
  int height = integral.m_height = squared_integral.m_height = image.Height();
  ASSERT(height);

  // check sizes of internal data structures
  int old_array_len_integral = integral.m_arraylen;
  int old_array_len_squared_integral = squared_integral.m_arraylen;
  int data_len = width*height;
  int new_array_len = data_len+height+padded_width;
  if (new_array_len!=old_array_len_integral) {
    delete[] integral.m_pPaddedData;
    ASSERT(new_array_len>0);
#if defined(_MSC_VER) && _MSC_VER<1400
    // Visual C++ doesn't understand the nothrow
    integral.m_pPaddedData = new TYPE[new_array_len];
#else
    integral.m_pPaddedData = new (std::nothrow) TYPE[new_array_len];
#endif
    ASSERT(integral.m_pPaddedData);
    if (integral.m_pPaddedData==NULL) {
      throw ITException("CreateSimpleNSquaredFrom: out of memory (1)");
    }
    integral.m_arraylen = new_array_len;
    integral.m_pData = &integral.m_pPaddedData[padded_width+1];
  }
  if (new_array_len!=old_array_len_squared_integral) {
    delete[] squared_integral.m_pPaddedData;
#if defined(_MSC_VER) && _MSC_VER<1400
    // Visual C++ doesn't understand the nothrow
    squared_integral.m_pPaddedData = new TYPE[new_array_len];
#else
    squared_integral.m_pPaddedData = new (std::nothrow) TYPE[new_array_len];
#endif
    ASSERT(squared_integral.m_pPaddedData);
    if (squared_integral.m_pPaddedData==NULL) {
      throw ITException("CreateSimpleNSquaredFrom: out of memory (2)");
    }
    squared_integral.m_arraylen = new_array_len;
    squared_integral.m_pData = &squared_integral.m_pPaddedData[padded_width+1];
  }
  memset(integral.m_pPaddedData, 0, new_array_len*sizeof(TYPE));
  memset(squared_integral.m_pPaddedData, 0, new_array_len*sizeof(TYPE));

  // fill integral image for the ROI part of the image
  int y_stop = min(height, roi.bottom);
  int x_stop = min(width, roi.right);
  for (int y=max(0,roi.top); y<y_stop; y++) {
    for (int x=max(0,roi.left); x<x_stop; x++) {
      TYPE img = image.Pixel(x, y);
      { // regular integral
        TYPE left = integral.GetElement(x-1, y);
        TYPE upper = integral.GetElement(x, y-1);
        TYPE diag = integral.GetElement(x-1, y-1);
        TYPE val = left+upper-diag+img;
        integral.SetElement(x, y, val);
      }
      { // integral of squared image
        TYPE left = squared_integral.GetElement(x-1, y);
        TYPE upper = squared_integral.GetElement(x, y-1);
        TYPE diag = squared_integral.GetElement(x-1, y-1);
        TYPE val = left+upper-diag+img*img;
        squared_integral.SetElement(x, y, val);
      }
    }
  }
}


#ifdef DEBUG
/*
template<class TYPE>
TYPE CIntegralImageT<TYPE>::operator[](int i) const
{
  ASSERT(0<=i && i<m_data_len);
  return m_pData[i];
}
*/

template<class TYPE>
TYPE CIntegralImageT<TYPE>::GetElement(int col, int row) const
{
  //  if (!(-1<=row && row<m_height && -1<=col && col<m_width)) {
  //    cout << row << " " << m_height << " " << col << " " << m_width << endl;
  //  }
  ASSERT(-1<=row && row<m_height && -1<=col && col<m_width);
  return m_pData[row*m_padded_width+col]; 
}

template<class TYPE>
void CIntegralImageT<TYPE>::SetElement(int col, int row, TYPE val)
{ 
  ASSERT(0<=row && row<m_height && 0<=col && col<m_width);
  m_pData[row*m_padded_width+col]=val; 
}

template<class TYPE>
void CIntegralImageT<TYPE>::IncElement(int col, int row, TYPE inc)
{
  ASSERT(0<=row && row<m_height && 0<=col && col<m_width);
  m_pData[row*m_padded_width+col]+=inc; 
}
#endif //DEBUG


template<class TYPE>
ostream& operator<<(ostream& os, const CIntegralImageT<TYPE>& integral)
{
  for (int y=0; y<integral.m_height; y++) {
    for (int x=0; x<integral.m_width; x++) {
      os << integral.GetElement(x, y) << " ";
    }
    os << endl;
  }
  return os;
}




