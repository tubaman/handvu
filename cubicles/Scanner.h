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
  * $Id: Scanner.h,v 1.30 2005/02/12 02:00:59 matz Exp $
**/

// Scanner scans across an image and finds matches for the 
// classifier cascade.  There are two implementation files:
// Scanner_Train.cpp for training-only functions and Scanner.cpp
// for everything else.
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

#ifndef __SCANNER_H
#define __SCANNER_H

#include "IntegralImage.h"
#ifdef HAVE_FLOAT_H
#include <float.h>
#endif

//namespace { // cubicles

#ifndef CScanMatch_DEFINED
#define CScanMatch_DEFINED
class CScanMatch {
public:
  CScanMatch() 
    : left(-1), top(-1), right(-1), bottom(-1),
    scale(-1), scale_x(-1), scale_y(-1), name("") {};
  CScanMatch(int _left, int _top, int _right, int _bottom,
             double _scale, double _scale_x, double _scale_y, string _name) 
    : left(_left), top(_top), right(_right), bottom(_bottom), 
      scale(_scale), scale_x(_scale_x), scale_y(_scale_y), name(_name) {};

  CRect AsRect() const { return CRect(left, top, right, bottom); }

  int         left, top, right, bottom;
  double      scale, scale_x, scale_y;
  string      name;
};
#endif // CScanMatch_DEFINED

typedef vector<CScanMatch> CScanMatchVector;
typedef vector<CScanMatchVector> CScanMatchMatrix;

class CClassifierCascade;
class CScaleParams;

// ----------------------------------------------------------------------
// class CImageScanner
// ----------------------------------------------------------------------

class CImageScanner {
 public:
  CImageScanner();
  CImageScanner(const CImageScanner& src);
  ~CImageScanner() {};

  void SetScanParameters(
    double start_scale = 1.0,
    double stop_scale = DBL_MAX,
    double scale_inc_factor = 1.25,
    double translation_inc_x = 1.0,
    double translation_inc_y = 1.0,
    CRect scan_area = CRect(0, 0, INT_MAX, INT_MAX)
    );
  void GetScanParameters(
    double* pStart_scale,
    double* pStop_scale,
    double* pScale_inc_factor,
    double* pTranslation_inc_x,
    double* pTranslation_inc_y,
    CRect& scan_area,
    bool* pPostProcessing,
    bool* pIsActive
    ) const;
  void SetScanArea(const CRect& scan_area);
  void SetScanScales(double start_scale, double stop_scale);
  const CRect& GetScanArea() const;
  void GetScaleSizes(int* min_width, int* max_width,
		     int* min_height, int* max_height) const;
  void SetAutoPostProcessing(bool on=true);
  int Scan(const CClassifierCascade& cascade,
	   const CByteImage& image,
	   CScanMatchVector& matches) const;
  int Scan(const CClassifierCascade& cascade,
	   const CIntegralImage& integral,
	   const CIntegralImage& squared_integral,
	   CScanMatchVector& matches) const;
  void PostProcess(CScanMatchVector& posClsfd) const;
  bool IsActive() const {return m_is_active;};
  void SetActive(bool active=true) {m_is_active = active;}
#ifdef WITH_TRAINING
  int EvaluateThreshs(const CClassifierCascade& cascade,
		      const CIntegralImage& integral,
		      const CIntegralImage& squared_integral,
		      CIntMatrix& numMatches,
		      const CDoubleVector& threshs) const;
#endif // WITH_TRAINING

  ostream& output(ostream& os) const;

protected:
  void NextScaleParams(CScaleParams& params) const;
  void InitScaleParams(const CClassifierCascade& cascade,
		       CScaleParams& params) const;

  friend class CScaleParams;
  
 private:
  double                      m_start_scale;
  double                      m_stop_scale;
  double                      m_scale_inc_factor;
  double                      m_translation_inc_x;
  double                      m_translation_inc_y;
  CRect                       m_scan_area;
  bool                        m_post_process;
  bool                        m_is_active;
  mutable int                 m_min_scaled_template_width;
  mutable int                 m_max_scaled_template_width;
  mutable int                 m_min_scaled_template_height;
  mutable int                 m_max_scaled_template_height;

  // local buffer
  mutable CIntegralImage      m_integral;
  mutable CIntegralImage      m_squared_integral;
};

typedef vector<CImageScanner> CScannerVector;

ostream& operator<<(ostream& os, const CImageScanner& scanner);


// ----------------------------------------------------------------------
// class CScaleParams
// ----------------------------------------------------------------------

class CScaleParams {
public:
  CScaleParams() {};

public:
  double scale_x;
  double scale_y;
  double base_scale;
  double translation_inc_x;
  double translation_inc_y;
  int template_width;
  int template_height;
  int scaled_template_width;
  int scaled_template_height;
  double actual_scale_x;
  double actual_scale_y;

protected:
  // these four are only needed for m_integer_scaling
  int iscale_x;
  int iscale_y;
  double fscale_x;
  double fscale_y;

  friend void CImageScanner::NextScaleParams(CScaleParams& params) const;
  friend void CImageScanner::InitScaleParams(const CClassifierCascade& cascade,
					     CScaleParams& params) const;
};

//} // namespace cubicles

#endif // __SCANNER_H


