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
  * $Id: IntegralFeatures.h,v 1.37 2005/10/28 17:47:04 matz Exp $
**/

// IntegralFeatures describe the rectangular areas whose pixel
// sums are compared to a weak classifier's threshold.  There
// are many different types of IntegralFeatures.
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

#if !defined(_INTEGRALFEATURES_H__INCLUDED_)
#define _INTEGRALFEATURES_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IntegralFeatures.h : header file
//
/*
* IntegrationTemplates
*
* This is an implementation of the object recognition method suggested
* by Viola and Jones.
*
* Matz matz@cs.ucsb.edu
*/


#include "IntegralImage.h"
//#include "FeatureTransformers.h"

#ifdef WITH_TRAINING
#include "ExampleIntegral.h"
#endif // WITH_TRAINING


#pragma warning (disable: 1125)

//namespace {  // cubicles

// define the "featnum" data type - it should be larger than a 32bit integer
#ifdef WIN32
typedef unsigned __int64 featnum;
#define FEATNUM_MAX 0xffffffffffffffff
#define IT_INVALID_FEATURE FEATNUM_MAX
#define IT_MAX_VALID_FEATURE (FEATNUM_MAX-1)
#else
typedef unsigned long long int featnum;
#define FEATNUM_MAX ULONG_LONG_MAX
#define IT_INVALID_FEATURE ULONG_LONG_MAX
#define IT_MAX_VALID_FEATURE (ULONG_LONG_MAX-1)
#endif // WIN32

typedef vector<featnum> CFeatnumVector;


/////////////////////////////////////////////////////////////////////////////
//
// class CIntegralFeature
//

class CIntegralFeature {
protected:
  CIntegralFeature(int templateWidth, int templateHeight, 
                   featnum num_incarnations, bool is_partial, int cost);
  
 public:
  virtual II_TYPE Compute(const CIntegralImage& image) const = 0;
  virtual II_TYPE ComputeScaled(const CIntegralImage& image, 
                               II_TYPE mean, int left, int top) const = 0;
#ifdef WITH_TRAINING
  II_TYPE Compute(ExampleList::const_iterator example) const;
#endif // WITH_TRAINING
  virtual void SetToFirstIncarnation() = 0;
  virtual bool SetToNextIncarnation() = 0;   // true if has more incarnations
  virtual CIntegralFeature* Copy() const = 0;
  virtual void MakePartialFromCurrentForNumIncarnations(featnum num) = 0;
  static CIntegralFeature* CreateFrom(istream& is, 
                                      int template_width, int template_height);
  featnum GetNumIncarnations() const;
  int GetTemplateWidth() const { return m_template_width; }
  int GetTemplateHeight() const { return m_template_height; }
  void ScaleEvenly(II_TYPE scale_x, II_TYPE scale_y, 
                   int scaled_template_width, int scaled_template_height);
  int GetComputeCost() const {return m_cost;};
  virtual bool Equals(const CIntegralFeature& /*from*/) const 
    { return false; }
  //virtual void Transform(const CFeatureTransformer& transformer) = 0;
#ifdef USE_MFC
  virtual void Draw(CDC* pDC, int x_off, int y_off, int zoomfactor) const = 0;
#endif // USE_MFC
  
  virtual ostream& output(ostream& os) const = 0;

  friend ostream& operator<<(ostream& os, const CIntegralFeature& clsf);
  
 protected:
  virtual void Scale(II_TYPE scale_x, II_TYPE scale_y) = 0;
  virtual void SetNonOverlap() = 0;
  virtual void EvenOutScales(II_TYPE* pScale_x, II_TYPE* pScale_y, 
                             int scaled_template_width, 
                             int scaled_template_height) = 0;
  enum {
    COST_ADD = 0,
    COST_GET = 1
  };
  static const II_TYPE SCALE_DIFFERENCE_EPSILON;
  
 protected:
  int           m_template_width, m_template_height;
  featnum       m_num_incarnations;
  bool          m_is_partial;
  featnum       m_remaining_incarnations, m_stop_after_num_incarnations;
  II_TYPE        m_global_scale;
  int           m_non_overlap;
  int           m_cost;
};


/////////////////////////////////////////////////////////////////////////////
//
// class CLeftRightIF
//

class CLeftRightIF : public CIntegralFeature {
public:
  CLeftRightIF(int templateWidth, int templateHeight);
  CLeftRightIF(int templateWidth, int templateHeight,
               int _toprow, int _bottomrow,
               int _leftrect_leftcol, int _centercol,
               int _rightrect_rightcol);
  CLeftRightIF(const CLeftRightIF& frm);
  CLeftRightIF(istream& is, int template_width, int template_height);
  
  virtual II_TYPE Compute(const CIntegralImage& image) const;
  virtual II_TYPE ComputeScaled(const CIntegralImage& image, 
                               II_TYPE mean, int left, int top) const;
  virtual void SetToFirstIncarnation();
  virtual bool SetToNextIncarnation();
  virtual CIntegralFeature* Copy() const;
  virtual void MakePartialFromCurrentForNumIncarnations(featnum num);
  virtual bool Equals(const CLeftRightIF& from) const;
  //virtual void Transform(const CFeatureTransformer& transformer);
#ifdef USE_MFC
  virtual void Draw(CDC* pDC, int x_off, int y_off, int zoomfactor) const;
#endif // USE_MFC
  
  virtual ostream& output(ostream& os) const;
  
 protected:
  virtual void Scale(II_TYPE scale_x, II_TYPE scale_y);
  virtual void SetNonOverlap();
  virtual void EvenOutScales(II_TYPE* pScale_x, II_TYPE* pScale_y, 
                             int scaled_template_width, 
                             int scaled_template_height);
  
  
 protected:
  int bottomrow, toprow;
  int leftrect_leftcol, centercol, rightrect_rightcol;
  int start_bottomrow, start_toprow;
  int start_leftrect_leftcol, start_centercol, start_rightrect_rightcol;
  int scaled_bottomrow, scaled_toprow;
  int scaled_leftrect_leftcol, scaled_centercol, scaled_rightrect_rightcol;
};


/////////////////////////////////////////////////////////////////////////////
//
// class CUpDownIF
//

class CUpDownIF : public CIntegralFeature {
public:
  CUpDownIF(int templateWidth, int templateHeight);
  CUpDownIF(int templateWidth, int templateHeight,
            int _toprect_toprow, int _centerrow,
            int _bottomrect_bottomrow,
            int _leftcol, int _rightcol);
  CUpDownIF(const CUpDownIF& frm);
  CUpDownIF(istream& is, int template_width, int template_height);
  
  virtual II_TYPE Compute(const CIntegralImage& image) const;
  virtual II_TYPE ComputeScaled(const CIntegralImage& image, 
                               II_TYPE mean, int left, int top) const;
  virtual void SetToFirstIncarnation();
  virtual bool SetToNextIncarnation();
  virtual CIntegralFeature* Copy() const;
  virtual void MakePartialFromCurrentForNumIncarnations(featnum num);
  virtual bool Equals(const CUpDownIF& from) const;
//virtual void Transform(const CFeatureTransformer& transformer);
#ifdef USE_MFC
  virtual void Draw(CDC* pDC, int x_off, int y_off, int zoomfactor) const;
#endif // USE_MFC
  
  virtual ostream& output(ostream& os) const;
  
 protected:
  virtual void Scale(II_TYPE scale_x, II_TYPE scale_y);
  virtual void SetNonOverlap();
  virtual void EvenOutScales(II_TYPE* pScale_x, II_TYPE* pScale_y, 
                             int scaled_template_width, 
                             int scaled_template_height);


protected:
  int toprect_toprow, centerrow, bottomrect_bottomrow;
  int leftcol, rightcol;
  int start_toprect_toprow, start_centerrow, start_bottomrect_bottomrow;
  int start_leftcol, start_rightcol;
  int scaled_toprect_toprow, scaled_centerrow, scaled_bottomrect_bottomrow;
  int scaled_leftcol, scaled_rightcol;
};


/////////////////////////////////////////////////////////////////////////////
//
// class CLeftCenterRightIF
//

class CLeftCenterRightIF : public CIntegralFeature {
public:
  CLeftCenterRightIF(int templateWidth, int templateHeight);
  CLeftCenterRightIF(int templateWidth, int templateHeight,
                     int _toprow, int _bottomrow,
                     int _leftrect_leftcol, int _leftrect_rightcol,
                     int _rightrect_leftcol, int _rightrect_rightcol);
  CLeftCenterRightIF(const CLeftCenterRightIF& frm);
  CLeftCenterRightIF(istream& is, int template_width, int template_height);
  
  virtual II_TYPE Compute(const CIntegralImage& image) const;
  virtual II_TYPE ComputeScaled(const CIntegralImage& image, 
                               II_TYPE mean, int left, int top) const;
  virtual void SetToFirstIncarnation();
  virtual bool SetToNextIncarnation();
  virtual CIntegralFeature* Copy() const;
  virtual void MakePartialFromCurrentForNumIncarnations(featnum num);
  virtual bool Equals(const CLeftCenterRightIF& from) const;
  //virtual void Transform(const CFeatureTransformer& transformer);
#ifdef USE_MFC
  virtual void Draw(CDC* pDC, int x_off, int y_off, int zoomfactor) const;
#endif // USE_MFC
  
  virtual ostream& output(ostream& os) const;
  
 protected:
  virtual void Scale(II_TYPE scale_x, II_TYPE scale_y);
  virtual void SetNonOverlap();
  virtual void EvenOutScales(II_TYPE* pScale_x, II_TYPE* pScale_y, 
                             int scaled_template_width, 
                             int scaled_template_height);
  
  
 protected:
  int toprow, bottomrow;
  int leftrect_leftcol, leftrect_rightcol;
  int rightrect_leftcol, rightrect_rightcol;
  int start_toprow, start_bottomrow;
  int start_leftrect_leftcol, start_leftrect_rightcol;
  int start_rightrect_leftcol, start_rightrect_rightcol;
  int scaled_toprow, scaled_bottomrow;
  int scaled_leftrect_leftcol, scaled_leftrect_rightcol;
  int scaled_rightrect_leftcol, scaled_rightrect_rightcol;
};


/////////////////////////////////////////////////////////////////////////////
//
// class CSevenColumnsIF
//

class CSevenColumnsIF : public CIntegralFeature {
 public:
  CSevenColumnsIF(int templateWidth, int templateHeight);
  CSevenColumnsIF(int templateWidth, int templateHeight,
                  int _toprow, int _bottomrow,
                  int _1, int _2, int _3, int _4, int _5, int _6,
                  int _7, int _8);
  CSevenColumnsIF(const CSevenColumnsIF& frm);
  CSevenColumnsIF(istream& is, int template_width, int template_height);
  
  virtual II_TYPE Compute(const CIntegralImage& image) const;
  virtual II_TYPE ComputeScaled(const CIntegralImage& image, 
                               II_TYPE mean, int left, int top) const;
  virtual void SetToFirstIncarnation();
  virtual bool SetToNextIncarnation();
  virtual CIntegralFeature* Copy() const;
  virtual void MakePartialFromCurrentForNumIncarnations(featnum num);
  virtual bool Equals(const CSevenColumnsIF& from) const;
#ifdef USE_MFC
  virtual void Draw(CDC* pDC, int x_off, int y_off, int zoomfactor) const;
#endif // USE_MFC

  virtual ostream& output(ostream& os) const;
  
 protected:
  virtual void Scale(II_TYPE scale_x, II_TYPE scale_y);
  virtual void SetNonOverlap();
  virtual void EvenOutScales(II_TYPE* pScale_x, II_TYPE* pScale_y, 
                             int scaled_template_width, 
                             int scaled_template_height);

  
 protected:
  int bottomrow, toprow;
  int col1_left, col2_left, col3_left, col4_left, 
    col5_left, col6_left, col7_left, col7_right;
  int start_bottomrow, start_toprow;
  int start_col1_left, start_col2_left, start_col3_left, start_col4_left, 
    start_col5_left, start_col6_left, start_col7_left, start_col7_right;
  int scaled_bottomrow, scaled_toprow;
  int scaled_col1_left, scaled_col2_left, scaled_col3_left, scaled_col4_left, 
    scaled_col5_left, scaled_col6_left, scaled_col7_left, scaled_col7_right;
};


/////////////////////////////////////////////////////////////////////////////
//
// class CDiagIF
//

class CDiagIF : public CIntegralFeature {
 public:
  CDiagIF(int templateWidth, int templateHeight);
  CDiagIF(int templateWidth, int templateHeight,
          int _toprect_toprow, int _centerrow,
          int _bottomrect_bottomrow,
          int _leftrect_leftcol, int _centercol,
          int _rightrect_rightcol);
  CDiagIF(const CDiagIF& frm);
  CDiagIF(istream& is, int template_width, int template_height);
  
  virtual II_TYPE Compute(const CIntegralImage& image) const;
  virtual II_TYPE ComputeScaled(const CIntegralImage& image, 
                               II_TYPE mean, int left, int top) const;
  virtual void SetToFirstIncarnation();
  virtual bool SetToNextIncarnation();
  virtual CIntegralFeature* Copy() const;
  virtual void MakePartialFromCurrentForNumIncarnations(featnum num);
  void ScaleX(II_TYPE scale_x);
  void ScaleY(II_TYPE scale_y);
  virtual bool Equals(const CDiagIF& from) const;
  //virtual void Transform(const CFeatureTransformer& transformer);
#ifdef USE_MFC
  virtual void Draw(CDC* pDC, int x_off, int y_off, int zoomfactor) const;
#endif // USE_MFC
  
  virtual ostream& output(ostream& os) const;
  
 protected:
  virtual void Scale(II_TYPE scale_x, II_TYPE scale_y);
  virtual void SetNonOverlap();
  virtual void EvenOutScales(II_TYPE* pScale_x, II_TYPE* pScale_y, 
                             int scaled_template_width, 
                             int scaled_template_height);
  
  
 protected:
  int toprect_toprow, centerrow, bottomrect_bottomrow;
  int leftrect_leftcol, centercol, rightrect_rightcol;
  int start_toprect_toprow, start_centerrow, start_bottomrect_bottomrow;
  int start_leftrect_leftcol, start_centercol, start_rightrect_rightcol;
  int scaled_toprect_toprow, scaled_centerrow, scaled_bottomrect_bottomrow;
  int scaled_leftrect_leftcol, scaled_centercol, scaled_rightrect_rightcol;
};


/////////////////////////////////////////////////////////////////////////////
//
// class CFourBoxesIF
//

class CFourBoxesIF : public CIntegralFeature {
 public:
  CFourBoxesIF(int templateWidth, int templateHeight);
  CFourBoxesIF(int templateWidth, int templateHeight,
               const CRect* pB1, const CRect* pB2,
               const CRect* pB3, const CRect* pB4);
  CFourBoxesIF(const CFourBoxesIF& frm);
  CFourBoxesIF(istream& is, int template_width, int template_height);
  
  virtual II_TYPE Compute(const CIntegralImage& image) const;
  virtual II_TYPE ComputeScaled(const CIntegralImage& image, 
                               II_TYPE mean, int left, int top) const;
  virtual void SetToFirstIncarnation();
  virtual bool SetToNextIncarnation();
  virtual CIntegralFeature* Copy() const;
  virtual void MakePartialFromCurrentForNumIncarnations(featnum num);
  void ScaleX(II_TYPE scale_x);
  void ScaleY(II_TYPE scale_y);
  virtual bool Equals(const CFourBoxesIF& from) const;
  //virtual voiad Transform(const CFeatureTransformer& transformer);
#ifdef USE_MFC
  virtual void Draw(CDC* pDC, int x_off, int y_off, int zoomfactor) const;
#endif // USE_MFC
  
  virtual ostream& output(ostream& os) const;
  
 protected:
  virtual void Scale(II_TYPE scale_x, II_TYPE scale_y);
  virtual void SetNonOverlap();
  virtual void EvenOutScales(II_TYPE* pScale_x, II_TYPE* pScale_y, 
                             int scaled_template_width, 
                             int scaled_template_height);
  
 protected:
  int b1_left, b1_top, b1_right, b1_bottom;
  int b2_left, b2_top, b2_right, b2_bottom;
  int b3_left, b3_top, b3_right, b3_bottom;
  int b4_left, b4_top, b4_right, b4_bottom;
  int start_b1_left, start_b1_top, start_b1_right, start_b1_bottom;
  int start_b2_left, start_b2_top, start_b2_right, start_b2_bottom;
  int start_b3_left, start_b3_top, start_b3_right, start_b3_bottom;
  int start_b4_left, start_b4_top, start_b4_right, start_b4_bottom;
  int scaled_b1_left, scaled_b1_top, scaled_b1_right, scaled_b1_bottom;
  int scaled_b2_left, scaled_b2_top, scaled_b2_right, scaled_b2_bottom;
  int scaled_b3_left, scaled_b3_top, scaled_b3_right, scaled_b3_bottom;
  int scaled_b4_left, scaled_b4_top, scaled_b4_right, scaled_b4_bottom;
};


/////////////////////////////////////////////////////////////////////////////
//
// class CLeftRightSameIF
//

class CLeftRightSameIF : public CLeftRightIF {
 public:
  CLeftRightSameIF(int templateWidth, int templateHeight);
  CLeftRightSameIF(int templateWidth, int templateHeight,
                   int _toprow, int _bottomrow,
                   int _leftrect_leftcol, int _centercol,
                   int _rightrect_rightcol);
  CLeftRightSameIF(const CLeftRightSameIF& frm);
  CLeftRightSameIF(istream& is, int template_width, int template_height);
  
  virtual bool SetToNextIncarnation();
  virtual CIntegralFeature* Copy() const;
  virtual ostream& output(ostream& os) const;
};


/////////////////////////////////////////////////////////////////////////////
//
// class CUpDownSameIF
//

class CUpDownSameIF : public CUpDownIF {
 public:
  CUpDownSameIF(int templateWidth, int templateHeight);
  CUpDownSameIF(int templateWidth, int templateHeight,
                int _toprect_toprow, int _centerrow,
                int _bottomrect_bottomrow,
                int _leftcol, int _rightcol);
  CUpDownSameIF(const CUpDownSameIF& frm);
  CUpDownSameIF(istream& is, int template_width, int template_height);
  
  virtual bool SetToNextIncarnation();
  virtual CIntegralFeature* Copy() const;
  virtual ostream& output(ostream& os) const;
};


/////////////////////////////////////////////////////////////////////////////
//
// class CLeftCenterRightSameIF
//

class CLeftCenterRightSameIF : public CLeftCenterRightIF {
 public:
  CLeftCenterRightSameIF(int templateWidth, int templateHeight);
  CLeftCenterRightSameIF(int templateWidth, int templateHeight,
                         int _toprow, int _bottomrow,
                         int _leftrect_leftcol, int _leftrect_rightcol,
                         int _rightrect_leftcol,
                         int _rightrect_rightcol);
  CLeftCenterRightSameIF(const CLeftCenterRightSameIF& frm);
  CLeftCenterRightSameIF(istream& is, int template_width, int template_height);
  
  virtual bool SetToNextIncarnation();
  virtual CIntegralFeature* Copy() const;
  virtual ostream& output(ostream& os) const;
};


/////////////////////////////////////////////////////////////////////////////
//
// class CSevenColumnsSameIF
//

class CSevenColumnsSameIF : public CSevenColumnsIF {
 public:
  CSevenColumnsSameIF(int templateWidth, int templateHeight);
  CSevenColumnsSameIF(int templateWidth, int templateHeight,
                      int _toprow, int _bottomrow,
                      int _1, int _2, int _3, int _4, int _5, int _6,
                      int _7, int _8);
  CSevenColumnsSameIF(const CSevenColumnsSameIF& frm);
  CSevenColumnsSameIF(istream& is, int template_width, int template_height);
  
  
  virtual bool SetToNextIncarnation();
  virtual CIntegralFeature* Copy() const;
  virtual ostream& output(ostream& os) const;
};


/////////////////////////////////////////////////////////////////////////////
//
// class CSevenColumnsSimilarIF
//

class CSevenColumnsSimilarIF : public CSevenColumnsIF {
 public:
  CSevenColumnsSimilarIF(int templateWidth, int templateHeight);
  CSevenColumnsSimilarIF(int templateWidth, int templateHeight,
                         int _toprow, int _bottomrow,
                         int _1, int _2, int _3, int _4, int _5, int _6,
                         int _7, int _8);
  CSevenColumnsSimilarIF(const CSevenColumnsSimilarIF& frm);
  CSevenColumnsSimilarIF(istream& is, int template_width, int template_height);
  
  virtual bool SetToNextIncarnation();
  virtual CIntegralFeature* Copy() const;
  virtual ostream& output(ostream& os) const;
};


/////////////////////////////////////////////////////////////////////////////
//
// class CDiagSameIF
//

class CDiagSameIF : public CDiagIF {
 public:
  CDiagSameIF(int templateWidth, int templateHeight);
  CDiagSameIF(int templateWidth, int templateHeight,
              int _toprect_toprow, int _centerrow,
              int _bottomrect_bottomrow,
              int _leftrect_leftcol, int _centercol,
              int _rightrect_rightcol);
  CDiagSameIF(const CDiagSameIF& frm);
  CDiagSameIF(istream& is, int template_width, int template_height);
  
  virtual bool SetToNextIncarnation();
  virtual CIntegralFeature* Copy() const;
  virtual ostream& output(ostream& os) const;
};


/////////////////////////////////////////////////////////////////////////////
//
// class CDiagSimilarIF
//

class CDiagSimilarIF : public CDiagIF {
 public:
  CDiagSimilarIF(int templateWidth, int templateHeight);
  CDiagSimilarIF(int templateWidth, int templateHeight,
                 int _toprect_toprow, int _centerrow,
                 int _bottomrect_bottomrow,
                 int _leftrect_leftcol, int _centercol,
                 int _rightrect_rightcol);
  CDiagSimilarIF(const CDiagSimilarIF& frm);
  CDiagSimilarIF(istream& is, int template_width, int template_height);
  
  virtual bool SetToNextIncarnation();
  virtual CIntegralFeature* Copy() const;
  virtual ostream& output(ostream& os) const;
};


/////////////////////////////////////////////////////////////////////////////
//
// class CFourBoxesSameIF
//

class CFourBoxesSameIF : public CFourBoxesIF {
 public:
  CFourBoxesSameIF(int templateWidth, int templateHeight);
  CFourBoxesSameIF(int templateWidth, int templateHeight,
                   const CRect* pB1, const CRect* pB2,
                   const CRect* pB3, const CRect* pB4);
  CFourBoxesSameIF(const CFourBoxesSameIF& frm);
  CFourBoxesSameIF(istream& is, int template_width, int template_height);
  
  virtual void SetToFirstIncarnation();
  virtual bool SetToNextIncarnation();
  virtual CIntegralFeature* Copy() const;
  virtual ostream& output(ostream& os) const;
};

//} // namespace cubicles 

/////////////////////////////////////////////////////////////////////////////

#pragma warning (default: 1125)

#endif // !defined(_INTEGRALFEATURES_H__INCLUDED_)
