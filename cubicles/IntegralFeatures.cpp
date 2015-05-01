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
  * $Id: IntegralFeatures.cpp,v 1.61 2005/10/28 17:47:04 matz Exp $
**/

// IntegralFeatures describe the rectangular areas whose pixel
// sums are compared to a weak classifier's threshold.  There
// are many different types of IntegralFeatures.
// Functions for reading from files are flex/bison generated from
// CascadeFileScanner.lex and CascadeFileParser.yy.
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
#include "IntegralFeatures.h"
#include "Exceptions.h"
#include <math.h>
#include <iostream>


#ifdef _DEBUG
#ifdef USE_MFC
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif // USE_MFC
#endif // _DEBUG

#if defined(WIN32) && defined(DEBUG)
#include <streams.h>
#endif


#ifdef USE_MFC
CBrush* g_pBlackbrush = NULL;
CBrush* g_pWhitebrush = NULL;
#endif // USE_MFC


int g_verbose = 0;
FILE* g_ostream = stdout;

#ifdef WIN32
#include <malloc.h>
#define alloca _alloca
#endif


/////////////////////////////////////////////////////////////////////////////
//
// CIntegralFeature implementation
//
/////////////////////////////////////////////////////////////////////////////

const II_TYPE CIntegralFeature::SCALE_DIFFERENCE_EPSILON = .000000001;

CIntegralFeature::CIntegralFeature(int templateWidth, int templateHeight,
                                   featnum num_incarnations, bool is_partial,
                                   int cost)
:	m_template_width(templateWidth),
	m_template_height(templateHeight),
	m_num_incarnations(num_incarnations),
	m_is_partial(is_partial),
  m_cost(cost)
{
#ifdef USE_MFC
  if (g_pBlackbrush==NULL) {
    ASSERT(g_pWhitebrush==NULL);
    g_pWhitebrush = new CBrush(RGB(255, 255, 255));
    g_pBlackbrush = new CBrush(RGB(0, 0, 0));
  }
#endif // USE_MFC
}

ostream& operator<<(ostream& os, const CIntegralFeature& clsf)
{
  return clsf.output(os);
}

CIntegralFeature* CIntegralFeature::CreateFrom(istream& is,
					       int template_width, int template_height)
{
  CIntegralFeature* feature = NULL;
  string subclass;
  is >> subclass;
  if (subclass=="LeftRight") {
    feature = new CLeftRightIF(is, template_width, template_height);

  } else if (subclass=="UpDown") {
    feature = new CUpDownIF(is, template_width, template_height);

  } else if (subclass=="LeftCenterRight") {
    feature = new CLeftCenterRightIF(is, template_width, template_height);

  } else if (subclass=="SevenColumns") {
    feature = new CSevenColumnsIF(is, template_width, template_height);

  } else if (subclass=="Diag") {
    feature = new CDiagIF(is, template_width, template_height);

  } else if (subclass=="LeftRightSame") {
    feature = new CLeftRightSameIF(is, template_width, template_height);

  } else if (subclass=="UpDownSame") {
    feature = new CUpDownSameIF(is, template_width, template_height);

  } else if (subclass=="LeftCenterRightSame") {
    feature = new CLeftCenterRightSameIF(is, template_width, template_height);

  } else if (subclass=="SevenColumnsSame") {
    feature = new CSevenColumnsSameIF(is, template_width, template_height);

  } else if (subclass=="SevenColumnsSimilar") {
    feature = new CSevenColumnsSimilarIF(is, template_width, template_height);

  } else if (subclass=="DiagSame") {
    feature = new CDiagSameIF(is, template_width, template_height);

  } else if (subclass=="DiagSimilar") {
    feature = new CDiagSimilarIF(is, template_width, template_height);

  } else if (subclass=="FourBoxes") {
    feature = new CFourBoxesIF(is, template_width, template_height);

  } else if (subclass=="FourBoxesSame") {
    feature = new CFourBoxesSameIF(is, template_width, template_height);

  } else {
    // error
    VERBOSE1(1, "can not create IntegralFeature from string '%s'.", 
	     subclass.c_str());
    ASSERT(0);
  }
  return feature;
}


/* use ScaleEvenly to scale by non-integer values, otherwise use
* Scale
*/
void CIntegralFeature::ScaleEvenly(II_TYPE scale_x, II_TYPE scale_y, 
                                   int scaled_template_width, int scaled_template_height)
{
  Scale(scale_x, scale_y);
  EvenOutScales(&scale_x, &scale_y, scaled_template_width, scaled_template_height);
  m_global_scale = scale_x*scale_y;
}

featnum CIntegralFeature::GetNumIncarnations() const
{
  if (m_num_incarnations==IT_INVALID_FEATURE) {
    // count some other feature, we don't want to change
    // this one at all.
    CIntegralFeature* counter = this->Copy();
    counter->SetToFirstIncarnation();
    featnum cnt = 0;
    bool has_more_incarnations = true;
    do {
      cnt++;
      if (cnt==IT_MAX_VALID_FEATURE) {
        throw ITException("int overflow in GetNumIncarnations()");
      }
      has_more_incarnations = counter->SetToNextIncarnation();
    } while (has_more_incarnations);
    ((CIntegralFeature*) this)->m_num_incarnations = cnt;
    delete counter;
  }
  ASSERT(m_num_incarnations>=0 && 
	 m_num_incarnations!=IT_INVALID_FEATURE);
  return m_num_incarnations;
}




/////////////////////////////////////////////////////////////////////////////
//
// CLeftRightIF classes implementation
//
/////////////////////////////////////////////////////////////////////////////

CLeftRightIF::CLeftRightIF(int templateWidth, int templateHeight)
  : CIntegralFeature(templateWidth, templateHeight, IT_INVALID_FEATURE, 
		     false, 6*COST_GET+7*COST_ADD) 
{
}

CLeftRightIF::CLeftRightIF(const CLeftRightIF& frm)
  : CIntegralFeature(frm.m_template_width, frm.m_template_height,
		     frm.m_num_incarnations, frm.m_is_partial, 6*COST_GET+7*COST_ADD) 
{
  toprow = frm.toprow;
  bottomrow = frm.bottomrow;
  leftrect_leftcol = frm.leftrect_leftcol;
  centercol = frm.centercol;
  rightrect_rightcol = frm.rightrect_rightcol;
  if (m_is_partial) {
    start_toprow = frm.start_toprow;
    start_bottomrow = frm.start_bottomrow;
    start_leftrect_leftcol = frm.start_leftrect_leftcol;
    start_centercol = frm.start_centercol;
    start_rightrect_rightcol = frm.start_rightrect_rightcol;
    m_remaining_incarnations = frm.m_remaining_incarnations;
    m_stop_after_num_incarnations = frm.m_stop_after_num_incarnations;
  }
  SetNonOverlap();
}

CLeftRightIF::CLeftRightIF(int template_width, int template_height,
                           int _toprow, int _bottomrow,
                           int _leftrect_leftcol, int _centercol,
                           int _rightrect_rightcol)
  : CIntegralFeature(template_width, template_height, IT_INVALID_FEATURE, 0, 6*COST_GET+7*COST_ADD)
{
  toprow = _toprow;
  bottomrow = _bottomrow;
  leftrect_leftcol = _leftrect_leftcol;
  centercol = _centercol;
  rightrect_rightcol = _rightrect_rightcol;
  SetNonOverlap();
}

CLeftRightIF::CLeftRightIF(istream& is, int template_width, int template_height)
  : CIntegralFeature(template_width, template_height, IT_INVALID_FEATURE, 0, 6*COST_GET+7*COST_ADD)
{
  char a, b, c;
  string str;
  is >> toprow >> a >> bottomrow >> str
     >> leftrect_leftcol >> b >> centercol >> c >> rightrect_rightcol;
  if (a!=',' || b!=',' || c!=',' || str!="x") {
    char* buf = (char*) alloca((256+str.length())*sizeof(char));
    sprintf(buf, "error during construction of LeftRightIF (%c|%c|%c|%s)\n",
	   a, b, c, str.c_str());
    throw ITException(buf);
  }
  SetNonOverlap();
}

bool CLeftRightIF::Equals(const CLeftRightIF& frm) const
{
  bool equal = 
    (toprow == frm.toprow &&
     bottomrow == frm.bottomrow &&
     leftrect_leftcol == frm.leftrect_leftcol &&
     centercol == frm.centercol &&
     rightrect_rightcol == frm.rightrect_rightcol);
  return equal;
}

void CLeftRightIF::SetNonOverlap() {
  m_non_overlap =
    ((centercol-leftrect_leftcol) - (rightrect_rightcol-centercol))
    * (bottomrow-toprow);
}

II_TYPE CLeftRightIF::Compute(const CIntegralImage& image) const
{
	II_TYPE botcen = image.GetElement(centercol, bottomrow);
	ASSERT(!isnan(botcen));
	II_TYPE topcen = image.GetElement(centercol, toprow);
	ASSERT(!isnan(topcen));

	II_TYPE val_leftrect = 
		botcen
		- image.GetElement(leftrect_leftcol, bottomrow)
		- topcen
		+ image.GetElement(leftrect_leftcol, toprow);
	ASSERT(!isnan(val_leftrect));

	II_TYPE val_rightrect = 
		image.GetElement(rightrect_rightcol, bottomrow)
		- botcen
		- image.GetElement(rightrect_rightcol, toprow)
		+ topcen;
	ASSERT(!isnan(val_rightrect));

	ASSERT(!isnan(val_leftrect-val_rightrect));
	return val_leftrect-val_rightrect;
}

II_TYPE CLeftRightIF::ComputeScaled(const CIntegralImage& image, II_TYPE mean, int left, int top) const
{
	int placed_toprow = top + scaled_toprow;
	int placed_bottomrow = top + scaled_bottomrow;
	int placed_leftrect_leftcol = left + scaled_leftrect_leftcol;
	int placed_centercol = left + scaled_centercol;
	int placed_rightrect_rightcol = left + scaled_rightrect_rightcol;

	II_TYPE botcen = image.GetElement(placed_centercol, placed_bottomrow);
	II_TYPE topcen = image.GetElement(placed_centercol, placed_toprow);

	II_TYPE val_leftrect = 
		botcen
		- image.GetElement(placed_leftrect_leftcol, placed_bottomrow)
		- topcen
		+ image.GetElement(placed_leftrect_leftcol, placed_toprow);

	II_TYPE val_rightrect = 
		image.GetElement(placed_rightrect_rightcol, placed_bottomrow)
		- botcen
		- image.GetElement(placed_rightrect_rightcol, placed_toprow)
		+ topcen;

	II_TYPE val = val_leftrect-val_rightrect;
	II_TYPE scaled_val = val/m_global_scale;
	II_TYPE mean_adjust = m_non_overlap*mean;

  return scaled_val-mean_adjust;
}

void CLeftRightIF::Scale(II_TYPE scale_x, II_TYPE scale_y)
{
  if (toprow==-1) {
    scaled_toprow = -1;
    scaled_bottomrow = (int)((II_TYPE)(bottomrow+1)*scale_y) - 1;
  } else {
    scaled_toprow = (int)((II_TYPE)toprow*scale_y);
    scaled_bottomrow = (int)((II_TYPE)bottomrow*scale_y);
  }
  if (leftrect_leftcol==-1) {
    scaled_leftrect_leftcol = -1;
    scaled_centercol = (int)((II_TYPE)(centercol+1)*scale_x) - 1;
    scaled_rightrect_rightcol =
      (int)((II_TYPE)(rightrect_rightcol-centercol)*scale_x) + scaled_centercol;
  } else {
    scaled_leftrect_leftcol = (int)((II_TYPE)leftrect_leftcol*scale_x);
    scaled_centercol = (int)((II_TYPE)centercol*scale_x);
    scaled_rightrect_rightcol = (int)((II_TYPE)rightrect_rightcol*scale_x);
  }

  m_global_scale = scale_x*scale_y;
}

void CLeftRightIF::EvenOutScales(II_TYPE* pScale_x, II_TYPE* pScale_y,
        int scaled_template_width, int /*scaled_template_height*/)
{
  int leftrect = centercol-leftrect_leftcol;
  int rightrect = rightrect_rightcol-centercol;
  II_TYPE ratio = (II_TYPE)leftrect/(II_TYPE)rightrect;
  int sleftrect = scaled_centercol-scaled_leftrect_leftcol;
  int srightrect = scaled_rightrect_rightcol-scaled_centercol;
  II_TYPE sratio = (II_TYPE)sleftrect/(II_TYPE)srightrect;

  int scaled_width = scaled_rightrect_rightcol-scaled_leftrect_leftcol;
  bool too_wide = scaled_width > scaled_template_width;

  while (!too_wide && fabs(ratio-sratio)>=SCALE_DIFFERENCE_EPSILON) {
    if (ratio<sratio) {
      scaled_rightrect_rightcol++;
      srightrect++;
    } else {
      scaled_centercol++;
      scaled_rightrect_rightcol++;
      sleftrect++;
    }
    scaled_width = scaled_rightrect_rightcol-scaled_leftrect_leftcol;
    too_wide = scaled_width > scaled_template_width;

    sratio = (II_TYPE)sleftrect/(II_TYPE)srightrect;
  }

  int overlap = scaled_rightrect_rightcol-scaled_template_width+1;
  // overlap: by how much is rightmost too far to the right? 
  // it must not be larger than scaled_template_width-1
  if (overlap>0) {
    // went outside template boundaries
    if (too_wide) {
      // feature too big, use int scale
      *pScale_x = floor(*pScale_x);
      Scale(*pScale_x, *pScale_y);
    } else {
      // shift feature by overlap
      scaled_leftrect_leftcol -= overlap;
      scaled_centercol -= overlap;
      scaled_rightrect_rightcol -= overlap;
      *pScale_x = sratio;
    }
  } else {
    *pScale_x = sratio;
  }
}

void CLeftRightIF::SetToFirstIncarnation()
{
  if (m_is_partial) {
    toprow=start_toprow;
    bottomrow=start_bottomrow;
    leftrect_leftcol=start_leftrect_leftcol;
    centercol=start_centercol;
    rightrect_rightcol=start_rightrect_rightcol;
    m_remaining_incarnations=m_stop_after_num_incarnations;
  } else {
    toprow=-1;
    bottomrow=0;
    leftrect_leftcol=-1;
    centercol=0;
    rightrect_rightcol=1;
  }
  SetNonOverlap();
}

bool CLeftRightIF::SetToNextIncarnation()
{
  if (m_is_partial) {
    if (m_remaining_incarnations) {
      m_remaining_incarnations--;
    } else {
      return false;
    }
  }
  rightrect_rightcol++;
  if (rightrect_rightcol>=m_template_width) {
    centercol++;
    if (centercol>=m_template_width-1) {
      leftrect_leftcol++;
      if (leftrect_leftcol==m_template_width-2) {
	bottomrow++;
	if (bottomrow==m_template_height) {
	  toprow++;
	  if (toprow==m_template_height-1) {
	    return false;
	  }
	  bottomrow=toprow+1;
	}
	leftrect_leftcol=-1;
      }
      centercol=leftrect_leftcol+1;
    }
    rightrect_rightcol=centercol+1;
  }
  SetNonOverlap();
  return true;
}

CIntegralFeature* CLeftRightIF::Copy() const
{
  return new CLeftRightIF(*this);
}

void CLeftRightIF::MakePartialFromCurrentForNumIncarnations(
  featnum num)
{
  m_is_partial=true;
  m_num_incarnations=num+1;
  start_toprow=toprow;
  start_bottomrow=bottomrow;
  start_leftrect_leftcol=leftrect_leftcol;
  start_centercol=centercol;
  start_rightrect_rightcol=rightrect_rightcol;
  m_remaining_incarnations=m_stop_after_num_incarnations=num;
}

/*
void CLeftRightIF::Transform(const CFeatureTransformer& transformer)
{
  ASSERT(!m_is_partial);
  transformer.TransformWidthHeight(&m_template_width, &m_template_height);
  transformer.TransformCoordinate();
  transformer.FeatureDone();

  toprow=start_toprow;
  bottomrow=start_bottomrow;
  leftrect_leftcol=start_leftrect_leftcol;
  centercol=start_centercol;
  rightrect_rightcol=start_rightrect_rightcol;
}
*/

#ifdef USE_MFC
void CLeftRightIF::Draw(CDC* pDC, int x_off, int y_off, int zoomfactor) const
{
  int ll = x_off + zoomfactor + leftrect_leftcol*zoomfactor;
  int tr = y_off + zoomfactor + toprow*zoomfactor;
  int cl = x_off + zoomfactor + centercol*zoomfactor;
  int br = y_off + zoomfactor + bottomrow*zoomfactor;
  int rr = x_off + zoomfactor + rightrect_rightcol*zoomfactor;
  pDC->FillRect(CRect(ll, tr, cl, br), g_pBlackbrush);
  pDC->Rectangle(cl, tr, rr, br);
}
#endif // USE_MFC

ostream& CLeftRightIF::output(ostream& os) const
{
  os << "LeftRight " << toprow << "," << bottomrow << " x "
     << leftrect_leftcol << "," << centercol << "," << rightrect_rightcol;
  return os;
}





/////////////////////////////////////////////////////////////////////////////
//
// CUpDownIF classes implementation
//
/////////////////////////////////////////////////////////////////////////////

CUpDownIF::CUpDownIF(int templateWidth, int templateHeight)
  : CIntegralFeature(templateWidth, templateHeight, IT_INVALID_FEATURE, false, 
		     6*COST_GET+7*COST_ADD) 
{
}

CUpDownIF::CUpDownIF(int templateWidth, int templateHeight,
                     int _toprect_toprow, int _centerrow,
                     int _bottomrect_bottomrow,
                     int _leftcol, int _rightcol)
  : CIntegralFeature(templateWidth, templateHeight, IT_INVALID_FEATURE, false,
		     6*COST_GET+7*COST_ADD) 
{
  toprect_toprow = _toprect_toprow;
  centerrow = _centerrow;
  bottomrect_bottomrow = _bottomrect_bottomrow;
  leftcol = _leftcol;
  rightcol = _rightcol;
  SetNonOverlap();
}

CUpDownIF::CUpDownIF(const CUpDownIF& frm)
  : CIntegralFeature(frm.m_template_width, frm.m_template_height,
		     frm.m_num_incarnations, frm.m_is_partial, 
		     6*COST_GET+7*COST_ADD) 
{
  toprect_toprow=frm.toprect_toprow;
  centerrow=frm.centerrow;
  bottomrect_bottomrow=frm.bottomrect_bottomrow;
  leftcol=frm.leftcol;
  rightcol=frm.rightcol;
  if (m_is_partial) {
    start_toprect_toprow=frm.start_toprect_toprow;
    start_centerrow=frm.start_centerrow;
    start_bottomrect_bottomrow=frm.start_bottomrect_bottomrow;
    start_leftcol=frm.start_leftcol;
    start_rightcol=frm.start_rightcol;
    m_remaining_incarnations = frm.m_remaining_incarnations;
    m_stop_after_num_incarnations = frm.m_stop_after_num_incarnations;
  }
  SetNonOverlap();
}

CUpDownIF::CUpDownIF(istream& is, int templateWidth, int templateHeight)
  : CIntegralFeature(templateWidth, templateHeight, IT_INVALID_FEATURE, false, 
		     6*COST_GET+7*COST_ADD) 
{
  char a, b, c;
  string str;
  is >> toprect_toprow >> a >> centerrow >> b
     >> bottomrect_bottomrow >> str >> leftcol >> c >> rightcol;
  if (a!=',' || b!=',' || c!=',' || str!="x") {
    char* buf = (char*) alloca((256+str.length())*sizeof(char));
    sprintf(buf, "error during construction of UpDownIF (%c|%c|%c|%s)\n",
	   a, b, c, str.c_str());
    throw ITException(buf);
  }
  SetNonOverlap();
}

bool CUpDownIF::Equals(const CUpDownIF& frm) const
{
  bool equal = 
    (toprect_toprow == frm.toprect_toprow &&
     centerrow == frm.centerrow &&
     bottomrect_bottomrow == frm.bottomrect_bottomrow &&
     leftcol == frm.leftcol &&
     rightcol == frm.rightcol);
  return equal;
}

void CUpDownIF::SetNonOverlap() {
  m_non_overlap =
    ((centerrow-toprect_toprow) - (bottomrect_bottomrow-centerrow))
    * (rightcol-leftcol);
}

II_TYPE CUpDownIF::Compute(const CIntegralImage& image) const
{
	II_TYPE ceri = image.GetElement(rightcol, centerrow);
	II_TYPE cele = image.GetElement(leftcol, centerrow);

	II_TYPE val_toprect = 
		ceri
		- cele
		- image.GetElement(rightcol, toprect_toprow)
		+ image.GetElement(leftcol, toprect_toprow);

	II_TYPE val_bottomrect = 
		image.GetElement(rightcol, bottomrect_bottomrow)
		- image.GetElement(leftcol, bottomrect_bottomrow)
		- ceri
		+ cele;

	return val_toprect-val_bottomrect;
}

II_TYPE CUpDownIF::ComputeScaled(const CIntegralImage& image, II_TYPE mean, int left, int top) const
{
	int placed_leftcol = left + scaled_leftcol;
	int placed_rightcol = left + scaled_rightcol;
	int placed_toprect_toprow = top + scaled_toprect_toprow;
	int placed_centerrow = top + scaled_centerrow;
	int placed_bottomrect_bottomrow = top + scaled_bottomrect_bottomrow;

	II_TYPE ceri = image.GetElement(placed_rightcol, placed_centerrow);
	II_TYPE cele = image.GetElement(placed_leftcol, placed_centerrow);

	II_TYPE val_toprect = 
		ceri
		- cele
		- image.GetElement(placed_rightcol, placed_toprect_toprow)
		+ image.GetElement(placed_leftcol, placed_toprect_toprow);

	II_TYPE val_bottomrect = 
		image.GetElement(placed_rightcol, placed_bottomrect_bottomrow)
		- image.GetElement(placed_leftcol, placed_bottomrect_bottomrow)
		- ceri
		+ cele;

	II_TYPE val = val_toprect-val_bottomrect;
	II_TYPE scaled_val = val/m_global_scale;
	II_TYPE mean_adjust = m_non_overlap*mean;

	return scaled_val-mean_adjust;
}

void CUpDownIF::Scale(II_TYPE scale_x, II_TYPE scale_y)
{
	if (leftcol==-1) {
		scaled_leftcol = -1;
		scaled_rightcol = (int)((II_TYPE)(rightcol+1)*scale_x) - 1;
	} else {
		scaled_leftcol = (int)((II_TYPE)leftcol*scale_x);
		scaled_rightcol = (int)((II_TYPE)rightcol*scale_x);
	}
	if (toprect_toprow==-1) {
		scaled_toprect_toprow = -1;
		scaled_centerrow = (int)((II_TYPE)(centerrow+1)*scale_y) - 1;
		scaled_bottomrect_bottomrow = (int)((II_TYPE)(bottomrect_bottomrow-centerrow)*scale_y) + scaled_centerrow;
	} else {
		scaled_toprect_toprow = (int)((II_TYPE)toprect_toprow*scale_y);
		scaled_centerrow = (int)((II_TYPE)centerrow*scale_y);
		scaled_bottomrect_bottomrow = (int)((II_TYPE)bottomrect_bottomrow*scale_y);
	}

  m_global_scale = scale_x*scale_y;
}

void CUpDownIF::EvenOutScales(II_TYPE* pScale_x, II_TYPE* pScale_y,
        int /*scaled_template_width*/, int scaled_template_height)
{
  int toprect = centerrow-toprect_toprow;
  int bottomrect = bottomrect_bottomrow-centerrow;
  II_TYPE ratio = (II_TYPE)toprect/(II_TYPE)bottomrect;
  int stoprect = scaled_centerrow-scaled_toprect_toprow;
  int sbottomrect = scaled_bottomrect_bottomrow-scaled_centerrow;
  II_TYPE sratio = (II_TYPE)stoprect/(II_TYPE)sbottomrect;

  int scaled_height = scaled_bottomrect_bottomrow-toprect_toprow;
  bool too_high = scaled_height > scaled_template_height;

  while (!too_high && fabs(ratio-sratio)>=SCALE_DIFFERENCE_EPSILON) {
    if (ratio<sratio) {
      scaled_bottomrect_bottomrow++;
      sbottomrect++;
    } else {
      scaled_centerrow++;
      scaled_bottomrect_bottomrow++;
      stoprect++;
    }
    scaled_height = scaled_bottomrect_bottomrow-toprect_toprow;
    too_high = scaled_height > scaled_template_height;

    sratio = (II_TYPE)stoprect/(II_TYPE)sbottomrect;
  }

  int overlap = scaled_bottomrect_bottomrow-scaled_template_height+1;
  // overlap: by how much is bottommost too far down? 
  // it must not be larger than scaled_template_height-1
  if (overlap>0) {
    // went outside template boundaries
    if (too_high) {
      // feature too big, use int scale
      *pScale_y = floor(*pScale_y);
      Scale(*pScale_x, *pScale_y);
    } else {
      // shift feature by overlap
      scaled_toprect_toprow -= overlap;
      scaled_centerrow -= overlap;
      scaled_bottomrect_bottomrow -= overlap;
      *pScale_y = sratio;
    }
  } else {
    *pScale_y = sratio;
  }
}

void CUpDownIF::SetToFirstIncarnation()
{
  if (m_is_partial) {
    toprect_toprow=start_toprect_toprow;
    centerrow=start_centerrow;
    bottomrect_bottomrow=start_bottomrect_bottomrow;
    leftcol=start_leftcol;
    rightcol=start_rightcol;
    m_remaining_incarnations=m_stop_after_num_incarnations;
  } else {
    toprect_toprow=-1;
    centerrow=0;
    bottomrect_bottomrow=1;
    leftcol=-1;
    rightcol=0;
  }
  SetNonOverlap();
}

bool CUpDownIF::SetToNextIncarnation()
{
	if (m_is_partial) {
		if (m_remaining_incarnations) {
			m_remaining_incarnations--;
		} else {
			return false;
		}
	}
	bottomrect_bottomrow++;
	if (bottomrect_bottomrow>=m_template_height) {
		centerrow++;
		if (centerrow>=m_template_height-1) {
			toprect_toprow++;
			if (toprect_toprow==m_template_height-2) {
				rightcol++;
				if (rightcol==m_template_width) {
					leftcol++;
					if (leftcol==m_template_width-1) {
						return false;
					}
					rightcol=leftcol+1;
				}
				toprect_toprow=-1;
			}
			centerrow=toprect_toprow+1;
		}
		bottomrect_bottomrow=centerrow+1;
	}
    SetNonOverlap();
	return true;
}

CIntegralFeature* CUpDownIF::Copy() const
{
	return new CUpDownIF(*this);
}

void CUpDownIF::MakePartialFromCurrentForNumIncarnations(
  featnum num)
{
  m_is_partial=true;
  m_num_incarnations=num+1;
  start_toprect_toprow=toprect_toprow;
  start_centerrow=centerrow;
  start_bottomrect_bottomrow=bottomrect_bottomrow;
  start_leftcol=leftcol;
  start_rightcol=rightcol;
  m_remaining_incarnations=m_stop_after_num_incarnations=num;
}

#ifdef USE_MFC
void CUpDownIF::Draw(CDC* pDC, int x_off, int y_off, int zoomfactor) const
{
	int lc = x_off + zoomfactor + leftcol*zoomfactor;
	int tt = y_off + zoomfactor + toprect_toprow*zoomfactor;
	int rc = x_off + zoomfactor + rightcol*zoomfactor;
	int cr = y_off + zoomfactor + centerrow*zoomfactor;
	int bb = y_off + zoomfactor + bottomrect_bottomrow*zoomfactor;
	pDC->FillRect(CRect(lc, tt, rc, cr), g_pBlackbrush);
	pDC->Rectangle(lc, cr, rc, bb);
}
#endif // USE_MFC

ostream& CUpDownIF::output(ostream& os) const
{
  os << "UpDown " << toprect_toprow << "," << centerrow 
     << "," << bottomrect_bottomrow << " x "
     << leftcol << "," << rightcol;
  return os;
}





/////////////////////////////////////////////////////////////////////////////
//
// CLeftCenterRightIF classes implementation
//
/////////////////////////////////////////////////////////////////////////////

CLeftCenterRightIF::CLeftCenterRightIF(int templateWidth, int templateHeight)
  : CIntegralFeature(templateWidth, templateHeight, IT_INVALID_FEATURE, false, 8*COST_GET+10*COST_ADD) 
{
}

CLeftCenterRightIF::CLeftCenterRightIF(int templateWidth, int templateHeight,
                                       int _toprow, int _bottomrow,
                                       int _leftrect_leftcol, int _leftrect_rightcol,
                                       int _rightrect_leftcol,
                                       int _rightrect_rightcol)
  : CIntegralFeature(templateWidth, templateHeight, IT_INVALID_FEATURE, false, 8*COST_GET+10*COST_ADD) 
{
  toprow = _toprow;
  bottomrow = _bottomrow;
  leftrect_leftcol = _leftrect_leftcol;
  leftrect_rightcol = _leftrect_rightcol;
  rightrect_leftcol = _rightrect_leftcol;
  rightrect_rightcol = _rightrect_rightcol;
  SetNonOverlap();
}

CLeftCenterRightIF::CLeftCenterRightIF(const CLeftCenterRightIF& frm)
  : CIntegralFeature(frm.m_template_width, frm.m_template_height,
		     frm.m_num_incarnations, frm.m_is_partial, 8*COST_GET+10*COST_ADD) 
{
  toprow=frm.toprow;
  bottomrow=frm.bottomrow;
  leftrect_leftcol=frm.leftrect_leftcol;
  leftrect_rightcol=frm.leftrect_rightcol;
  rightrect_leftcol=frm.rightrect_leftcol;
  rightrect_rightcol=frm.rightrect_rightcol;
  if (m_is_partial) {
    start_toprow=frm.start_toprow;
    start_bottomrow=frm.start_bottomrow;
    start_leftrect_leftcol=frm.start_leftrect_leftcol;
    start_leftrect_rightcol=frm.start_leftrect_rightcol;
    start_rightrect_leftcol=frm.start_rightrect_leftcol;
    start_rightrect_rightcol=frm.start_rightrect_rightcol;
    m_remaining_incarnations = frm.m_remaining_incarnations;
    m_stop_after_num_incarnations = frm.m_stop_after_num_incarnations;
  }
  SetNonOverlap();
}

CLeftCenterRightIF::CLeftCenterRightIF(istream& is, int templateWidth, int templateHeight)
  : CIntegralFeature(templateWidth, templateHeight, IT_INVALID_FEATURE, false, 8*COST_GET+10*COST_ADD) 
{
  char a, b, c;
  string strx, strslash;
  is >> toprow >> a >> bottomrow >> strx 
     >> leftrect_leftcol >> b >> leftrect_rightcol >> strslash
     >> rightrect_leftcol >> c >> rightrect_rightcol;
  if (a!=',' || b!=',' || c!=',' || strx!="x" || strslash!="/") {
    char* buf = (char*) alloca((256+strx.length()+strslash.length())*sizeof(char));
    sprintf(buf, "error during construction of LeftCenterRightIF (%c|%c|%c|%s|%s)\n",
	   a, b, c, strx.c_str(), strslash.c_str());
    throw ITException(buf);
  }
  SetNonOverlap();
}

bool CLeftCenterRightIF::Equals(const CLeftCenterRightIF& frm) const
{
  bool equal = 
    (toprow == frm.toprow &&
     bottomrow == frm.bottomrow &&
     leftrect_leftcol == frm.leftrect_leftcol &&
     leftrect_rightcol == frm.leftrect_rightcol &&
     rightrect_leftcol == frm.rightrect_leftcol &&
     rightrect_rightcol == frm.rightrect_rightcol);
  return equal;
}

void CLeftCenterRightIF::SetNonOverlap() {
  m_non_overlap = 
    ((leftrect_rightcol-leftrect_leftcol) 
     - (rightrect_leftcol-leftrect_rightcol)
     + (rightrect_rightcol-rightrect_leftcol)) * (bottomrow-toprow);
}

II_TYPE CLeftCenterRightIF::Compute(const CIntegralImage& image) const
{
	II_TYPE botleri = image.GetElement(leftrect_rightcol, bottomrow);
	II_TYPE topleri = image.GetElement(leftrect_rightcol, toprow);
	II_TYPE botrile = image.GetElement(rightrect_leftcol, bottomrow);
	II_TYPE toprile = image.GetElement(rightrect_leftcol, toprow);

	II_TYPE val_leftrect = 
		botleri
		- image.GetElement(leftrect_leftcol, bottomrow)
		- topleri
		+ image.GetElement(leftrect_leftcol, toprow);

	II_TYPE val_centerrect = 
		botrile
		- botleri
		- toprile
		+ topleri;
	
	II_TYPE val_rightrect = 
		image.GetElement(rightrect_rightcol, bottomrow)
		- botrile
		- image.GetElement(rightrect_rightcol, toprow)
		+ toprile;

	return val_leftrect-val_centerrect+val_rightrect;
}

II_TYPE CLeftCenterRightIF::ComputeScaled(const CIntegralImage& image, II_TYPE mean, int left, int top) const
{
	int placed_leftrect_leftcol = left + scaled_leftrect_leftcol;
	int placed_leftrect_rightcol = left + scaled_leftrect_rightcol;
	int placed_rightrect_leftcol= left + scaled_rightrect_leftcol;
	int placed_rightrect_rightcol = left + scaled_rightrect_rightcol;
	int placed_toprow = top + scaled_toprow;
	int placed_bottomrow = top + scaled_bottomrow;

	II_TYPE botleri = image.GetElement(placed_leftrect_rightcol, placed_bottomrow);
	II_TYPE topleri = image.GetElement(placed_leftrect_rightcol, placed_toprow);
	II_TYPE botrile = image.GetElement(placed_rightrect_leftcol, placed_bottomrow);
	II_TYPE toprile = image.GetElement(placed_rightrect_leftcol, placed_toprow);

	II_TYPE val_leftrect = 
		botleri
		- image.GetElement(placed_leftrect_leftcol, placed_bottomrow)
		- topleri
		+ image.GetElement(placed_leftrect_leftcol, placed_toprow);

	II_TYPE val_centerrect = 
		botrile
		- botleri
		- toprile
		+ topleri;
	
	II_TYPE val_rightrect = 
		image.GetElement(placed_rightrect_rightcol, placed_bottomrow)
		- botrile
		- image.GetElement(placed_rightrect_rightcol, placed_toprow)
		+ toprile;

	II_TYPE val = val_leftrect-val_centerrect+val_rightrect;
	II_TYPE scaled_val = val/m_global_scale;
	II_TYPE mean_adjust = m_non_overlap*mean;

	return scaled_val-mean_adjust;
}

void CLeftCenterRightIF::Scale(II_TYPE scale_x, II_TYPE scale_y)
{
	if (leftrect_leftcol==-1) {
		scaled_leftrect_leftcol = -1;
		scaled_leftrect_rightcol = (int)((II_TYPE)(leftrect_rightcol+1)*scale_x) -1;
		scaled_rightrect_leftcol= (int)((II_TYPE)(rightrect_leftcol-leftrect_rightcol)*scale_x) + scaled_leftrect_rightcol;
		scaled_rightrect_rightcol = (int)((II_TYPE)(rightrect_rightcol-rightrect_leftcol)*scale_x) + scaled_rightrect_leftcol;
	} else {
		scaled_leftrect_leftcol = (int)((II_TYPE)leftrect_leftcol*scale_x);
		scaled_leftrect_rightcol = (int)((II_TYPE)leftrect_rightcol*scale_x);
		scaled_rightrect_leftcol= (int)((II_TYPE)rightrect_leftcol*scale_x);
		scaled_rightrect_rightcol = (int)((II_TYPE)rightrect_rightcol*scale_x);
	}
	if (toprow==-1) {
		scaled_toprow = (int)((II_TYPE)(toprow+1)*scale_y) -1;
		scaled_bottomrow = (int)((II_TYPE)(bottomrow-toprow)*scale_y) + scaled_toprow;
	} else {
		scaled_toprow = (int)((II_TYPE)toprow*scale_y);
		scaled_bottomrow = (int)((II_TYPE)bottomrow*scale_y);
	}

  m_global_scale = scale_x*scale_y;
}

void CLeftCenterRightIF::EvenOutScales(II_TYPE* pScale_x, II_TYPE* pScale_y,
        int scaled_template_width, int /*scaled_template_height*/)
{
  int leftrect = leftrect_rightcol-leftrect_leftcol;
  int midrect = rightrect_leftcol-leftrect_rightcol;
  int rightrect = rightrect_rightcol-rightrect_leftcol;
  II_TYPE ratio1 = (II_TYPE)leftrect/(II_TYPE)midrect;
  II_TYPE ratio2 = (II_TYPE)midrect/(II_TYPE)rightrect;
  int sleftrect = scaled_leftrect_rightcol-scaled_leftrect_leftcol;
  int smidrect = scaled_rightrect_leftcol-scaled_leftrect_rightcol;
  int srightrect = scaled_rightrect_rightcol-scaled_rightrect_leftcol;
  II_TYPE sratio1 = (II_TYPE)sleftrect/(II_TYPE)smidrect;
  II_TYPE sratio2 = (II_TYPE)smidrect/(II_TYPE)srightrect;

  int scaled_width = scaled_rightrect_rightcol-scaled_leftrect_leftcol;
  bool too_wide = scaled_width > scaled_template_width;

  while (!too_wide && 
	 (fabs(ratio1-sratio1)>=SCALE_DIFFERENCE_EPSILON
	  || fabs(ratio2-sratio2)>=SCALE_DIFFERENCE_EPSILON)) {
    if (ratio1<sratio1) {
      scaled_rightrect_leftcol++;
      scaled_rightrect_rightcol++;
      smidrect++;
    } else if (ratio1>sratio1) {
      scaled_leftrect_rightcol++;
      scaled_rightrect_leftcol++;
      scaled_rightrect_rightcol++;
      sleftrect++;
    } else if (ratio2<sratio2) {
      scaled_rightrect_rightcol++;
      srightrect++;
    } else {
      ASSERT(ratio2>sratio2);
      scaled_rightrect_leftcol++;
      scaled_rightrect_rightcol++;
      smidrect++;
    }
    scaled_width = scaled_rightrect_rightcol-scaled_leftrect_leftcol;
    too_wide = scaled_width > scaled_template_width;

    sratio1 = (II_TYPE)sleftrect/(II_TYPE)smidrect;
    sratio2 = (II_TYPE)smidrect/(II_TYPE)srightrect;
  }
  ASSERT(too_wide || fabs((II_TYPE)leftrect/(II_TYPE)rightrect
              -(II_TYPE)sleftrect/(II_TYPE)srightrect)<SCALE_DIFFERENCE_EPSILON);

  int overlap = scaled_rightrect_rightcol-scaled_template_width+1;
  // overlap: by how much is rightmost too far to the right? 
  // it must not be larger than scaled_template_width-1
  if (overlap>0) {
    // went outside template boundaries
    if (too_wide) {
      // feature too big, use int scale
      *pScale_x = floor(*pScale_x);
      Scale(*pScale_x, *pScale_y);
    } else {
      // shift feature by overlap
      scaled_leftrect_leftcol -= overlap;
      scaled_leftrect_rightcol -= overlap;
      scaled_rightrect_leftcol -= overlap;
      scaled_rightrect_rightcol -= overlap;
      *pScale_x = sratio1;
    }
  } else {
    *pScale_x = sratio1;
  }
}

void CLeftCenterRightIF::SetToFirstIncarnation()
{
  if (m_is_partial) {
    toprow=start_toprow;
    bottomrow=start_bottomrow;
    leftrect_leftcol=start_leftrect_leftcol;
    leftrect_rightcol=start_leftrect_rightcol;
    rightrect_leftcol=start_rightrect_leftcol;
    rightrect_rightcol=start_rightrect_rightcol;
    m_remaining_incarnations=m_stop_after_num_incarnations;
  } else {
    toprow=-1;
    bottomrow=0;
    leftrect_leftcol=-1;
    leftrect_rightcol=0;
    rightrect_leftcol=1;
    rightrect_rightcol=2;
  }
  SetNonOverlap();
}

bool CLeftCenterRightIF::SetToNextIncarnation()
{
	if (m_is_partial) {
		if (m_remaining_incarnations) {
			m_remaining_incarnations--;
		} else {
			return false;
		}
	}
	rightrect_rightcol++;
	if (rightrect_rightcol>=m_template_width) {
		rightrect_leftcol++;
		if (rightrect_leftcol>=m_template_width-1) {
			leftrect_rightcol++;
			if (leftrect_rightcol>=m_template_width-2) {
				leftrect_leftcol++;
				if (leftrect_leftcol==m_template_width-3) {
					bottomrow++;
					if (bottomrow==m_template_height) {
						toprow++;
						if (toprow==m_template_height-1) {
							return false;
						}
						bottomrow=toprow+1;
					}
					leftrect_leftcol=0;
				}
				leftrect_rightcol=leftrect_leftcol+1;
			}
			rightrect_leftcol=leftrect_rightcol+1;
		}
		rightrect_rightcol=rightrect_leftcol+1;
	}
    SetNonOverlap();
	return true;
}

void CLeftCenterRightIF::MakePartialFromCurrentForNumIncarnations(
  featnum num)
{
  m_is_partial=true;
  m_num_incarnations=num+1;
  start_toprow=toprow;
  start_bottomrow=bottomrow;
  start_leftrect_leftcol=leftrect_leftcol;
  start_leftrect_rightcol=leftrect_rightcol;
  start_rightrect_leftcol=rightrect_leftcol;
  start_rightrect_rightcol=rightrect_rightcol;
  m_remaining_incarnations=m_stop_after_num_incarnations=num;
}

CIntegralFeature* CLeftCenterRightIF::Copy() const 
{
  return new CLeftCenterRightIF(*this);
}

#ifdef USE_MFC
void CLeftCenterRightIF::Draw(CDC* pDC, int x_off, int y_off, int zoomfactor) const
{
  int tr = y_off + zoomfactor + toprow*zoomfactor;
  int br = y_off + zoomfactor + bottomrow*zoomfactor;
  int ll = x_off + zoomfactor + leftrect_leftcol*zoomfactor;
  int lr = x_off + zoomfactor + leftrect_rightcol*zoomfactor;
  int rl = x_off + zoomfactor + rightrect_leftcol*zoomfactor;
  int rr = x_off + zoomfactor + rightrect_rightcol*zoomfactor;
	pDC->FillRect(CRect(ll, tr, lr, br), g_pBlackbrush);
	pDC->Rectangle(lr, tr, rl, br);
	pDC->FillRect(CRect(rl, tr, rr, br), g_pBlackbrush);
}
#endif // USE_MFC

ostream& CLeftCenterRightIF::output(ostream& os) const
{
  os << "LeftCenterRight " << toprow << "," << bottomrow << " x "
     << leftrect_leftcol << "," << leftrect_rightcol << " / "
     << rightrect_leftcol << "," << rightrect_rightcol;
  return os;
}




/////////////////////////////////////////////////////////////////////////////
//
// CSevenColumnsIF classes implementation
//
/////////////////////////////////////////////////////////////////////////////

CSevenColumnsIF::CSevenColumnsIF(int templateWidth, int templateHeight)
  : CIntegralFeature(templateWidth, templateHeight, IT_INVALID_FEATURE, false, 16*COST_GET+27*COST_ADD) 
{
}

CSevenColumnsIF::CSevenColumnsIF(int templateWidth, int templateHeight,
                                 int _toprow, int _bottomrow,
                                 int _1, int _2, int _3, int _4, int _5, int _6,
                                 int _7, int _8)
  : CIntegralFeature(templateWidth, templateHeight, IT_INVALID_FEATURE, false, 16*COST_GET+27*COST_ADD) 
{
  toprow = _toprow;
  bottomrow = _bottomrow;
  col1_left = _1;
  col2_left = _2;
  col3_left = _3;
  col4_left = _4;
  col5_left = _5;
  col6_left = _6;
  col7_left = _7;
  col7_right = _8;
  SetNonOverlap();
}

CSevenColumnsIF::CSevenColumnsIF(const CSevenColumnsIF& frm)
  : CIntegralFeature(frm.m_template_width, frm.m_template_height,
		     frm.m_num_incarnations, frm.m_is_partial, 16*COST_GET+27*COST_ADD) 
{
  toprow = frm.toprow;
  bottomrow = frm.bottomrow;
  col1_left = frm.col1_left;
  col2_left = frm.col2_left;
  col3_left = frm.col3_left;
  col4_left = frm.col4_left;
  col5_left = frm.col5_left;
  col6_left = frm.col6_left;
  col7_left = frm.col7_left;
  col7_right = frm.col7_right;
  if (m_is_partial) {
    start_toprow = frm.start_toprow;
    start_bottomrow = frm.start_bottomrow;
    start_col1_left = frm.start_col1_left;
    start_col2_left = frm.start_col2_left;
    start_col3_left = frm.start_col3_left;
    start_col4_left = frm.start_col4_left;
    start_col5_left = frm.start_col5_left;
    start_col6_left = frm.start_col6_left;
    start_col7_left = frm.start_col7_left;
    start_col7_right = frm.start_col7_right;
    m_remaining_incarnations = frm.m_remaining_incarnations;
    m_stop_after_num_incarnations = frm.m_stop_after_num_incarnations;
  }
  SetNonOverlap();
}

CSevenColumnsIF::CSevenColumnsIF(istream& is, int template_width, int template_height)
  : CIntegralFeature(template_width, template_height, IT_INVALID_FEATURE, 0, 16*COST_GET+27*COST_ADD)
{
  char a, b, c, d, e, f, g, h;
  string str;
  is >> toprow >> a >> bottomrow >> str
     >> col1_left >> b >> col2_left >> c >> col3_left >> d >> col4_left >> e
     >> col5_left >> f >> col6_left >> g >> col7_left >> h >> col7_right; 
  if (a!=',' || b!=',' || c!=',' || d!=',' || e!=',' || f!=',' || g!=',' || h!=',' || str!="x") {
    char* buf = (char*) alloca((256+str.length())*sizeof(char));
    sprintf(buf, "error during construction of SevenColumnsIF (%c|%c|%c|%c|%c|%c|%c|%c|%s)\n",
	   a, b, c, d, e, f, g, h, str.c_str());
    throw ITException(buf);
  }
  SetNonOverlap();
}

bool CSevenColumnsIF::Equals(const CSevenColumnsIF& frm) const
{
  bool equal = 
    (toprow == frm.toprow &&
     bottomrow == frm.bottomrow &&
     col1_left == frm.col1_left &&
     col2_left == frm.col2_left &&
     col3_left == frm.col3_left &&
     col4_left == frm.col4_left &&
     col5_left == frm.col5_left &&
     col6_left == frm.col6_left &&
     col7_left == frm.col7_left &&
     col7_right == frm.col7_right);
  return equal;
}

void CSevenColumnsIF::SetNonOverlap() {
  m_non_overlap =
    ((col2_left-col1_left) - (col3_left-col2_left) 
     + (col4_left-col3_left) - (col5_left-col4_left)
     + (col6_left-col5_left) - (col7_left-col6_left)
     + (col7_right-col7_left)
     )
    * (bottomrow-toprow);
}

II_TYPE CSevenColumnsIF::Compute(const CIntegralImage& image) const
{
	II_TYPE bot_col1_left = image.GetElement(col1_left, bottomrow);
	II_TYPE top_col1_left = image.GetElement(col1_left, toprow);
	II_TYPE bot_col2_left = image.GetElement(col2_left, bottomrow);
	II_TYPE top_col2_left = image.GetElement(col2_left, toprow);
	II_TYPE bot_col3_left = image.GetElement(col3_left, bottomrow);
	II_TYPE top_col3_left = image.GetElement(col3_left, toprow);
	II_TYPE bot_col4_left = image.GetElement(col4_left, bottomrow);
	II_TYPE top_col4_left = image.GetElement(col4_left, toprow);
	II_TYPE bot_col5_left = image.GetElement(col5_left, bottomrow);
	II_TYPE top_col5_left = image.GetElement(col5_left, toprow);
	II_TYPE bot_col6_left = image.GetElement(col6_left, bottomrow);
	II_TYPE top_col6_left = image.GetElement(col6_left, toprow);
	II_TYPE bot_col7_left = image.GetElement(col7_left, bottomrow);
	II_TYPE top_col7_left = image.GetElement(col7_left, toprow);
	II_TYPE bot_col7_right= image.GetElement(col7_right,bottomrow);
	II_TYPE top_col7_right= image.GetElement(col7_right,toprow);

	II_TYPE val_col1 = bot_col2_left - bot_col1_left - top_col2_left + top_col1_left;
	II_TYPE val_col2 = bot_col3_left - bot_col2_left - top_col3_left + top_col2_left;
	II_TYPE val_col3 = bot_col4_left - bot_col3_left - top_col4_left + top_col3_left;
	II_TYPE val_col4 = bot_col5_left - bot_col4_left - top_col5_left + top_col4_left;
	II_TYPE val_col5 = bot_col6_left - bot_col5_left - top_col6_left + top_col5_left;
	II_TYPE val_col6 = bot_col7_left - bot_col6_left - top_col7_left + top_col6_left;
	II_TYPE val_col7 = bot_col7_right- bot_col7_left - top_col7_right+ top_col7_left;

	return val_col1-val_col2+val_col3-val_col4+val_col5-val_col6+val_col7;
}

II_TYPE CSevenColumnsIF::ComputeScaled(const CIntegralImage& image, II_TYPE mean, int left, int top) const
{
	int placed_toprow = top + scaled_toprow;
	int placed_bottomrow = top + scaled_bottomrow;
	int placed_col1_left = left + scaled_col1_left;
	int placed_col2_left = left + scaled_col2_left;
	int placed_col3_left = left + scaled_col3_left;
	int placed_col4_left = left + scaled_col4_left;
	int placed_col5_left = left + scaled_col5_left;
	int placed_col6_left = left + scaled_col6_left;
	int placed_col7_left = left + scaled_col7_left;
	int placed_col7_right= left + scaled_col7_right;

	II_TYPE bot_col1_left = image.GetElement(placed_col1_left, placed_bottomrow);
	II_TYPE top_col1_left = image.GetElement(placed_col1_left, placed_toprow);
	II_TYPE bot_col2_left = image.GetElement(placed_col2_left, placed_bottomrow);
	II_TYPE top_col2_left = image.GetElement(placed_col2_left, placed_toprow);
	II_TYPE bot_col3_left = image.GetElement(placed_col3_left, placed_bottomrow);
	II_TYPE top_col3_left = image.GetElement(placed_col3_left, placed_toprow);
	II_TYPE bot_col4_left = image.GetElement(placed_col4_left, placed_bottomrow);
	II_TYPE top_col4_left = image.GetElement(placed_col4_left, placed_toprow);
	II_TYPE bot_col5_left = image.GetElement(placed_col5_left, placed_bottomrow);
	II_TYPE top_col5_left = image.GetElement(placed_col5_left, placed_toprow);
	II_TYPE bot_col6_left = image.GetElement(placed_col6_left, placed_bottomrow);
	II_TYPE top_col6_left = image.GetElement(placed_col6_left, placed_toprow);
	II_TYPE bot_col7_left = image.GetElement(placed_col7_left, placed_bottomrow);
	II_TYPE top_col7_left = image.GetElement(placed_col7_left, placed_toprow);
	II_TYPE bot_col7_right= image.GetElement(placed_col7_right,placed_bottomrow);
	II_TYPE top_col7_right= image.GetElement(placed_col7_right,placed_toprow);

	II_TYPE val_col1 = bot_col2_left - bot_col1_left - top_col2_left + top_col1_left;
	II_TYPE val_col2 = bot_col3_left - bot_col2_left - top_col3_left + top_col2_left;
	II_TYPE val_col3 = bot_col4_left - bot_col3_left - top_col4_left + top_col3_left;
	II_TYPE val_col4 = bot_col5_left - bot_col4_left - top_col5_left + top_col4_left;
	II_TYPE val_col5 = bot_col6_left - bot_col5_left - top_col6_left + top_col5_left;
	II_TYPE val_col6 = bot_col7_left - bot_col6_left - top_col7_left + top_col6_left;
	II_TYPE val_col7 = bot_col7_right- bot_col7_left - top_col7_right+ top_col7_left;

	II_TYPE val = val_col1-val_col2+val_col3-val_col4+val_col5-val_col6+val_col7;
	II_TYPE scaled_val = val/m_global_scale;
	II_TYPE mean_adjust = m_non_overlap*mean;

	return scaled_val-mean_adjust;
}

void CSevenColumnsIF::Scale(II_TYPE scale_x, II_TYPE scale_y)
{
	if (toprow==-1) {
		scaled_toprow = -1;
		scaled_bottomrow = (int)((II_TYPE)(bottomrow+1)*scale_y) - 1;
	} else {
		scaled_toprow = (int)((II_TYPE)toprow*scale_y);
		scaled_bottomrow = (int)((II_TYPE)bottomrow*scale_y);
	}
	if (col1_left==-1) {
		scaled_col1_left = -1;
		scaled_col2_left = (int)((II_TYPE)(col2_left+1)*scale_x) - 1;
		scaled_col3_left = (int)((II_TYPE)(col3_left-col2_left)*scale_x) + scaled_col2_left;
		scaled_col4_left = (int)((II_TYPE)(col4_left-col3_left)*scale_x) + scaled_col3_left;
		scaled_col5_left = (int)((II_TYPE)(col5_left-col4_left)*scale_x) + scaled_col4_left;
		scaled_col6_left = (int)((II_TYPE)(col6_left-col5_left)*scale_x) + scaled_col5_left;
		scaled_col7_left = (int)((II_TYPE)(col7_left-col6_left)*scale_x) + scaled_col6_left;
		scaled_col7_right= (int)((II_TYPE)(col7_right-col7_left)*scale_x) + scaled_col7_left;
	} else {
		scaled_col1_left = (int)((II_TYPE)col1_left*scale_x);
		scaled_col2_left = (int)((II_TYPE)col2_left*scale_x);
		scaled_col3_left = (int)((II_TYPE)col3_left*scale_x);
		scaled_col4_left = (int)((II_TYPE)col4_left*scale_x);
		scaled_col5_left = (int)((II_TYPE)col5_left*scale_x);
		scaled_col6_left = (int)((II_TYPE)col6_left*scale_x);
		scaled_col7_left = (int)((II_TYPE)col7_left*scale_x);
		scaled_col7_right= (int)((II_TYPE)col7_right*scale_x);
	}

  m_global_scale = scale_x*scale_y;
}

void CSevenColumnsIF::EvenOutScales(II_TYPE* pScale_x, II_TYPE* pScale_y,
        int scaled_template_width, int /*scaled_template_height*/)
{
  int rect1 = col2_left-col1_left;
  int rect2 = col3_left-col2_left;
  int rect3 = col4_left-col3_left;
  int rect4 = col5_left-col4_left;
  int rect5 = col6_left-col5_left;
  int rect6 = col7_left-col6_left;
  int rect7 = col7_right-col7_left;
  II_TYPE ratio1 = (II_TYPE)rect1/(II_TYPE)rect2;
  II_TYPE ratio2 = (II_TYPE)rect2/(II_TYPE)rect3;
  II_TYPE ratio3 = (II_TYPE)rect3/(II_TYPE)rect4;
  II_TYPE ratio4 = (II_TYPE)rect4/(II_TYPE)rect5;
  II_TYPE ratio5 = (II_TYPE)rect5/(II_TYPE)rect6;
  II_TYPE ratio6 = (II_TYPE)rect6/(II_TYPE)rect7;
  int srect1 = scaled_col2_left-scaled_col1_left;
  int srect2 = scaled_col3_left-scaled_col2_left;
  int srect3 = scaled_col4_left-scaled_col3_left;
  int srect4 = scaled_col5_left-scaled_col4_left;
  int srect5 = scaled_col6_left-scaled_col5_left;
  int srect6 = scaled_col7_left-scaled_col6_left;
  int srect7 = scaled_col7_right-scaled_col7_left;
  II_TYPE sratio1 = (II_TYPE)srect1/(II_TYPE)srect2;
  II_TYPE sratio2 = (II_TYPE)srect2/(II_TYPE)srect3;
  II_TYPE sratio3 = (II_TYPE)srect3/(II_TYPE)srect4;
  II_TYPE sratio4 = (II_TYPE)srect4/(II_TYPE)srect5;
  II_TYPE sratio5 = (II_TYPE)srect5/(II_TYPE)srect6;
  II_TYPE sratio6 = (II_TYPE)srect6/(II_TYPE)srect7;

  int scaled_width = scaled_col7_right-scaled_col1_left;
  bool too_wide = scaled_width > scaled_template_width;

  while (!too_wide && 
	 (fabs(ratio1-sratio1)>=SCALE_DIFFERENCE_EPSILON
	  || fabs(ratio2-sratio2)>=SCALE_DIFFERENCE_EPSILON
	  || fabs(ratio3-sratio3)>=SCALE_DIFFERENCE_EPSILON
	  || fabs(ratio4-sratio4)>=SCALE_DIFFERENCE_EPSILON
	  || fabs(ratio5-sratio5)>=SCALE_DIFFERENCE_EPSILON
	  || fabs(ratio6-sratio6)>=SCALE_DIFFERENCE_EPSILON)) {
    if (ratio1>sratio1) {
      scaled_col2_left++;
      scaled_col3_left++;
      scaled_col4_left++;
      scaled_col5_left++;
      scaled_col6_left++;
      scaled_col7_left++;
      scaled_col7_right++;
      srect1++;
    } else if (ratio1<sratio1 || ratio2>sratio2) {
      scaled_col3_left++;
      scaled_col4_left++;
      scaled_col5_left++;
      scaled_col6_left++;
      scaled_col7_left++;
      scaled_col7_right++;
      srect2++;
    } else if (ratio2<sratio2 || ratio3>sratio3) {
      scaled_col4_left++;
      scaled_col5_left++;
      scaled_col6_left++;
      scaled_col7_left++;
      scaled_col7_right++;
      srect3++;
    } else if (ratio3<sratio3 || ratio4>sratio4) {
      scaled_col5_left++;
      scaled_col6_left++;
      scaled_col7_left++;
      scaled_col7_right++;
      srect4++;
    } else if (ratio4<sratio4 || ratio5>sratio5) {
      scaled_col6_left++;
      scaled_col7_left++;
      scaled_col7_right++;
      srect5++;
    } else if (ratio5<sratio5 || ratio6>sratio6) {
      scaled_col7_left++;
      scaled_col7_right++;
      srect6++;
    } else if (ratio6<sratio6) {
      scaled_col7_right++;
      srect7++;
    } else {
      throw ITException("error during CSevenColumnsIF scaling - e too small?");
    }
    scaled_width = scaled_col7_right-scaled_col1_left;
    too_wide = scaled_width > scaled_template_width;

    sratio1 = (II_TYPE)srect1/(II_TYPE)srect2;
    sratio2 = (II_TYPE)srect2/(II_TYPE)srect3;
    sratio3 = (II_TYPE)srect3/(II_TYPE)srect4;
    sratio4 = (II_TYPE)srect4/(II_TYPE)srect5;
    sratio5 = (II_TYPE)srect5/(II_TYPE)srect6;
    sratio6 = (II_TYPE)srect6/(II_TYPE)srect7;
  }
  ASSERT(too_wide || fabs((II_TYPE)rect1/(II_TYPE)rect3
              -(II_TYPE)srect1/(II_TYPE)srect3)<SCALE_DIFFERENCE_EPSILON);
  ASSERT(too_wide || fabs((II_TYPE)rect1/(II_TYPE)rect4
              -(II_TYPE)srect1/(II_TYPE)srect4)<SCALE_DIFFERENCE_EPSILON);
  ASSERT(too_wide || fabs((II_TYPE)rect1/(II_TYPE)rect5
              -(II_TYPE)srect1/(II_TYPE)srect5)<SCALE_DIFFERENCE_EPSILON);
  ASSERT(too_wide || fabs((II_TYPE)rect1/(II_TYPE)rect6
              -(II_TYPE)srect1/(II_TYPE)srect6)<SCALE_DIFFERENCE_EPSILON);
  ASSERT(too_wide || fabs((II_TYPE)rect1/(II_TYPE)rect7
              -(II_TYPE)srect1/(II_TYPE)srect7)<SCALE_DIFFERENCE_EPSILON);

  int overlap = scaled_col7_right-scaled_template_width+1;
  // overlap: by how much is rightmost too far to the right? 
  // it must not be larger than scaled_template_width-1
  if (overlap>0) {
    // went outside template boundaries
    if (too_wide) {
      // feature too big, use int scale
      *pScale_x = floor(*pScale_x);
      Scale(*pScale_x, *pScale_y);
    } else {
      // shift feature by overlap
      scaled_col1_left -= overlap;
      scaled_col2_left -= overlap;
      scaled_col3_left -= overlap;
      scaled_col4_left -= overlap;
      scaled_col5_left -= overlap;
      scaled_col6_left -= overlap;
      scaled_col7_left -= overlap;
      scaled_col7_right -= overlap;
      *pScale_x = sratio1;
    }
  } else {
    *pScale_x = sratio1;
  }
}

void CSevenColumnsIF::SetToFirstIncarnation()
{
  if (m_is_partial) {
    toprow=start_toprow;
    bottomrow=start_bottomrow;
    col1_left = start_col1_left;
    col2_left = start_col2_left;
    col3_left = start_col3_left;
    col4_left = start_col4_left;
    col5_left = start_col5_left;
    col6_left = start_col6_left;
    col7_left = start_col7_left;
    col7_right= start_col7_right;
    m_remaining_incarnations=m_stop_after_num_incarnations;
  } else {
    toprow=-1;
    bottomrow=0;
    col1_left = -1;
    col2_left = 0;
    col3_left = 1;
    col4_left = 2;
    col5_left = 3;
    col6_left = 4;
    col7_left = 5;
    col7_right= 6;
  }
  SetNonOverlap();
}

bool CSevenColumnsIF::SetToNextIncarnation()
{
	if (m_is_partial) {
		if (m_remaining_incarnations) {
			m_remaining_incarnations--;
		} else {
			return false;
		}
	}
  col7_right++;
  if (col7_right>=m_template_width) {
    col7_left++;
    if (col7_left>=m_template_width-1) {
      col6_left++;
      if (col6_left>=m_template_width-2) {
        col5_left++;
		      if (col5_left>=m_template_width-3) {
            col4_left++;
            if (col4_left>=m_template_width-4) {
		            col3_left++;
                if (col3_left>=m_template_width-5) {
                  col2_left++;
                  if (col2_left>=m_template_width-6) {
                    col1_left++;
                    if (col1_left>=m_template_width-7) {
                      bottomrow++;
                      if (bottomrow==m_template_height) {
                        toprow++;
                        if (toprow==m_template_height-1) {
                          return false;
                        }
                        bottomrow = toprow+1;
                      }
                      col1_left = -1;
                    }
                    col2_left = col1_left+1;
                  }
                  col3_left = col2_left+1;
                }
                col4_left = col3_left+1;
            }
            col5_left = col4_left+1;
          }
          col6_left = col5_left+1;
      }
      col7_left = col6_left+1;
    }
    col7_right = col7_left+1;
  }
  SetNonOverlap();
	return true;
}

CIntegralFeature* CSevenColumnsIF::Copy() const
{
	return new CSevenColumnsIF(*this);
}

void CSevenColumnsIF::MakePartialFromCurrentForNumIncarnations(
  featnum num)
{
  m_is_partial = true;
  m_num_incarnations=num+1;
  start_toprow = toprow;
  start_bottomrow = bottomrow;
  start_col1_left = col1_left;
  start_col2_left = col2_left;
  start_col3_left = col3_left;
  start_col4_left = col4_left;
  start_col5_left = col5_left;
  start_col6_left = col6_left;
  start_col7_left = col7_left;
  start_col7_right= col7_right;
  m_remaining_incarnations = m_stop_after_num_incarnations=num;
}

#ifdef USE_MFC
void CSevenColumnsIF::Draw(CDC* pDC, int x_off, int y_off, int zoomfactor) const
{
	int tr = y_off + zoomfactor + toprow*zoomfactor;
	int br = y_off + zoomfactor + bottomrow*zoomfactor;
	int c1 = x_off + zoomfactor + col1_left*zoomfactor;
	int c2 = x_off + zoomfactor + col2_left*zoomfactor;
	int c3 = x_off + zoomfactor + col3_left*zoomfactor;
	int c4 = x_off + zoomfactor + col4_left*zoomfactor;
	int c5 = x_off + zoomfactor + col5_left*zoomfactor;
	int c6 = x_off + zoomfactor + col6_left*zoomfactor;
	int c7 = x_off + zoomfactor + col7_left*zoomfactor;
	int cr = x_off + zoomfactor + col7_right*zoomfactor;
	pDC->FillRect(CRect(c1, tr, c2, br), g_pBlackbrush);
	pDC->Rectangle(c2, tr, c3, br);
	pDC->FillRect(CRect(c3, tr, c4, br), g_pBlackbrush);
	pDC->Rectangle(c4, tr, c5, br);
	pDC->FillRect(CRect(c5, tr, c6, br), g_pBlackbrush);
	pDC->Rectangle(c6, tr, c7, br);
	pDC->FillRect(CRect(c7, tr, cr, br), g_pBlackbrush);
}
#endif // USE_MFC

ostream& CSevenColumnsIF::output(ostream& os) const
{
  os << "SevenColumns " << toprow << "," << bottomrow << " x "
     << col1_left << "," << col2_left << "," << col3_left << "," << col4_left << ","
     << col5_left << "," << col6_left << "," << col7_left << "," << col7_right;
  return os;
}





/////////////////////////////////////////////////////////////////////////////
//
// CDiagIF classes implementation
//
/////////////////////////////////////////////////////////////////////////////

CDiagIF::CDiagIF(int templateWidth, int templateHeight)
  : CIntegralFeature(templateWidth, templateHeight, IT_INVALID_FEATURE, false, 9*COST_GET+15*COST_ADD) 
{
}

CDiagIF::CDiagIF(int templateWidth, int templateHeight,
        int _toprect_toprow, int _centerrow,
        int _bottomrect_bottomrow,
        int _leftrect_leftcol, int _centercol,
        int _rightrect_rightcol)
  : CIntegralFeature(templateWidth, templateHeight, IT_INVALID_FEATURE, false, 9*COST_GET+15*COST_ADD) 
{
  toprect_toprow = _toprect_toprow;
  centerrow = _centerrow;
  bottomrect_bottomrow = _bottomrect_bottomrow;
  leftrect_leftcol = _leftrect_leftcol;
  centercol = _centercol;
  rightrect_rightcol = _rightrect_rightcol;
  SetNonOverlap();
}

CDiagIF::CDiagIF(const CDiagIF& frm)
  : CIntegralFeature(frm.m_template_width, frm.m_template_height,
		     frm.m_num_incarnations, frm.m_is_partial, 9*COST_GET+15*COST_ADD) 
{
  toprect_toprow=frm.toprect_toprow;
  centerrow=frm.centerrow;
  bottomrect_bottomrow=frm.bottomrect_bottomrow;
  leftrect_leftcol = frm.leftrect_leftcol;
  centercol = frm.centercol;
  rightrect_rightcol = frm.rightrect_rightcol;
  if (m_is_partial) {
    start_toprect_toprow=frm.start_toprect_toprow;
    start_centerrow=frm.start_centerrow;
    start_bottomrect_bottomrow=frm.start_bottomrect_bottomrow;
    start_leftrect_leftcol = frm.start_leftrect_leftcol;
    start_centercol = frm.start_centercol;
    start_rightrect_rightcol = frm.start_rightrect_rightcol;
    m_remaining_incarnations = frm.m_remaining_incarnations;
    m_stop_after_num_incarnations = frm.m_stop_after_num_incarnations;
  }
  SetNonOverlap();
}

CDiagIF::CDiagIF(istream& is, int templateWidth, int templateHeight)
  : CIntegralFeature(templateWidth, templateHeight, IT_INVALID_FEATURE, false, 9*COST_GET+15*COST_ADD) 
{
  char a, b, c, d;
  string str;
  is >> toprect_toprow >> a >> centerrow >> b
     >> bottomrect_bottomrow >> str
     >> leftrect_leftcol >> c >> centercol >> d
     >> rightrect_rightcol;
  if (a!=',' || b!=',' || str!="x" || c!=',' || d!=',') {
    char* buf = (char*) alloca((256+str.length())*sizeof(char));
    sprintf(buf, "error during construction of DiagIF (%c|%c|%s|%c|%c)\n",
	   a, b, str.c_str(), c, d);
    throw ITException(buf);
  }
  SetNonOverlap();
}

bool CDiagIF::Equals(const CDiagIF& frm) const
{
  bool equal = 
    (toprect_toprow == frm.toprect_toprow &&
     centerrow == frm.centerrow &&
     bottomrect_bottomrow == frm.bottomrect_bottomrow &&
     leftrect_leftcol  ==  frm.leftrect_leftcol &&
     centercol  ==  frm.centercol &&
     rightrect_rightcol  ==  frm.rightrect_rightcol);
  return equal;
}

void CDiagIF::SetNonOverlap() {
  // left_upper - right_upper - left_lower + right_lower
  m_non_overlap =
    (centercol-leftrect_leftcol)*(centerrow-toprect_toprow)
    - (rightrect_rightcol-centercol)*(centerrow-toprect_toprow)
    - (centercol-leftrect_leftcol)*(bottomrect_bottomrow-centerrow)
    + (rightrect_rightcol-centercol)*(bottomrect_bottomrow-centerrow);
}

II_TYPE CDiagIF::Compute(const CIntegralImage& image) const
{
  II_TYPE ceri = image.GetElement(rightrect_rightcol, centerrow);
  II_TYPE cele = image.GetElement(leftrect_leftcol, centerrow);
  II_TYPE cecen = image.GetElement(centercol, centerrow);
  II_TYPE topcen = image.GetElement(centercol, toprect_toprow);
  II_TYPE botcen = image.GetElement(centercol, bottomrect_bottomrow);
  
  II_TYPE val_topleft = 
    cecen
    - cele
    - topcen
    + image.GetElement(leftrect_leftcol, toprect_toprow);
  
  II_TYPE val_topright = 
    ceri
    - cecen
    - image.GetElement(rightrect_rightcol, toprect_toprow)
    + topcen;
  
  II_TYPE val_bottomleft = 
    botcen
    - image.GetElement(leftrect_leftcol, bottomrect_bottomrow)
    - cecen
    + cele;
  
  II_TYPE val_bottomright = 
    image.GetElement(rightrect_rightcol, bottomrect_bottomrow)
    - botcen
    - ceri
    + cecen;
  
  return val_topleft-val_topright-val_bottomleft+val_bottomright;
}

II_TYPE CDiagIF::ComputeScaled(const CIntegralImage& image, II_TYPE mean, int left, int top) const
{
  int placed_leftrect_leftcol = left + scaled_leftrect_leftcol;
  int placed_centercol = left + scaled_centercol;
  int placed_rightrect_rightcol = left + scaled_rightrect_rightcol;
  int placed_toprect_toprow = top + scaled_toprect_toprow;
  int placed_centerrow = top + scaled_centerrow;
  int placed_bottomrect_bottomrow = top + scaled_bottomrect_bottomrow;
  
  II_TYPE ceri = image.GetElement(placed_rightrect_rightcol, placed_centerrow);
  II_TYPE cele = image.GetElement(placed_leftrect_leftcol, placed_centerrow);
  II_TYPE cecen = image.GetElement(placed_centercol, placed_centerrow);
  II_TYPE topcen = image.GetElement(placed_centercol, placed_toprect_toprow);
  II_TYPE botcen = image.GetElement(placed_centercol, placed_bottomrect_bottomrow);
  
  II_TYPE val_topleft = 
    cecen
    - cele
    - topcen
    + image.GetElement(placed_leftrect_leftcol, placed_toprect_toprow);
  
  II_TYPE val_topright = 
    ceri
    - cecen
    - image.GetElement(placed_rightrect_rightcol, placed_toprect_toprow)
    + topcen;
  
  II_TYPE val_bottomleft = 
    botcen
    - image.GetElement(placed_leftrect_leftcol, placed_bottomrect_bottomrow)
    - cecen
    + cele;
  
  II_TYPE val_bottomright = 
    image.GetElement(placed_rightrect_rightcol, placed_bottomrect_bottomrow)
    - botcen
    - ceri
    + cecen;
  
  II_TYPE val = val_topleft-val_topright-val_bottomleft+val_bottomright;
  II_TYPE scaled_val = val/m_global_scale;
  II_TYPE mean_adjust = m_non_overlap*mean;

  return scaled_val-mean_adjust;
}

void CDiagIF::ScaleX(II_TYPE scale_x)
{
  if (leftrect_leftcol==-1) {
    scaled_leftrect_leftcol = -1;
    scaled_centercol = (int)((II_TYPE)(centercol+1)*scale_x) - 1;
    scaled_rightrect_rightcol =
      (int)((II_TYPE)(rightrect_rightcol-centercol)*scale_x) + scaled_centercol;
  } else {
    scaled_leftrect_leftcol = (int)((II_TYPE)leftrect_leftcol*scale_x);
    scaled_centercol = (int)((II_TYPE)centercol*scale_x);
    scaled_rightrect_rightcol = (int)((II_TYPE)rightrect_rightcol*scale_x);
  }
}

void CDiagIF::ScaleY(II_TYPE scale_y)
{
  if (toprect_toprow==-1) {
    scaled_toprect_toprow = -1;
    scaled_centerrow = (int)((II_TYPE)(centerrow+1)*scale_y) - 1;
    scaled_bottomrect_bottomrow =
      (int)((II_TYPE)(bottomrect_bottomrow-centerrow)*scale_y)
      + scaled_centerrow;
  } else {
    scaled_toprect_toprow = (int)((II_TYPE)toprect_toprow*scale_y);
    scaled_centerrow = (int)((II_TYPE)centerrow*scale_y);
    scaled_bottomrect_bottomrow = (int)((II_TYPE)bottomrect_bottomrow*scale_y);
  }
}

void CDiagIF::Scale(II_TYPE scale_x, II_TYPE scale_y)
{
  ScaleX(scale_x);
  ScaleY(scale_y);
  m_global_scale = scale_x*scale_y;
}

void CDiagIF::EvenOutScales(II_TYPE* pScale_x, II_TYPE* pScale_y,
        int scaled_template_width, int scaled_template_height)
{
  int leftrect = centercol-leftrect_leftcol;
  int rightrect = rightrect_rightcol-centercol;
  II_TYPE ratio = (II_TYPE)leftrect/(II_TYPE)rightrect;
  int sleftrect = scaled_centercol-scaled_leftrect_leftcol;
  int srightrect = scaled_rightrect_rightcol-scaled_centercol;
  II_TYPE sratio = (II_TYPE)sleftrect/(II_TYPE)srightrect;

  int scaled_width = scaled_rightrect_rightcol-scaled_leftrect_leftcol;
  bool too_wide = scaled_width > scaled_template_width;

  while (!too_wide && fabs(ratio-sratio)>=SCALE_DIFFERENCE_EPSILON) {
    if (ratio<sratio) {
      scaled_rightrect_rightcol++;
      srightrect++;
    } else {
      scaled_centercol++;
      scaled_rightrect_rightcol++;
      sleftrect++;
    }
    scaled_width = scaled_rightrect_rightcol-scaled_leftrect_leftcol;
    too_wide = scaled_width > scaled_template_width;

    sratio = (II_TYPE)sleftrect/(II_TYPE)srightrect;
  }

  int overlap = scaled_rightrect_rightcol-scaled_template_width+1;
  // overlap: by how much is rightmost too far to the right? 
  // it must not be larger than scaled_template_width-1
  if (overlap>0) {
    // went outside template boundaries
    if (too_wide) {
      // feature too big, use int scale
      *pScale_x = floor(*pScale_x);
      ScaleX(*pScale_x);
    } else {
      // shift feature by overlap
      scaled_leftrect_leftcol -= overlap;
      scaled_centercol -= overlap;
      scaled_rightrect_rightcol -= overlap;
      *pScale_x = sratio;
    }
  } else {
    *pScale_x = sratio;
  }

  int toprect = centerrow-toprect_toprow;
  int bottomrect = bottomrect_bottomrow-centerrow;
  ratio = (II_TYPE)toprect/(II_TYPE)bottomrect;
  int stoprect = scaled_centerrow-scaled_toprect_toprow;
  int sbottomrect = scaled_bottomrect_bottomrow-scaled_centerrow;
  sratio = (II_TYPE)stoprect/(II_TYPE)sbottomrect;

  int scaled_height = scaled_bottomrect_bottomrow-toprect_toprow;
  bool too_high = scaled_height > scaled_template_height;

  while (!too_high && fabs(ratio-sratio)>=SCALE_DIFFERENCE_EPSILON) {
    if (ratio<sratio) {
      scaled_bottomrect_bottomrow++;
      sbottomrect++;
    } else {
      scaled_centerrow++;
      scaled_bottomrect_bottomrow++;
      stoprect++;
    }
    scaled_height = scaled_bottomrect_bottomrow-toprect_toprow;
    too_high = scaled_height > scaled_template_height;

    sratio = (II_TYPE)stoprect/(II_TYPE)sbottomrect;
  }

  overlap = scaled_bottomrect_bottomrow-scaled_template_height+1;
  // overlap: by how much is bottommost too far down? 
  // it must not be larger than scaled_template_height-1
  if (overlap>0) {
    // went outside template boundaries
    if (too_high) {
      // feature too big, use int scale
      *pScale_y = floor(*pScale_y);
      ScaleY(*pScale_y);
    } else {
      // shift feature by overlap
      scaled_toprect_toprow -= overlap;
      scaled_centerrow -= overlap;
      scaled_bottomrect_bottomrow -= overlap;
      *pScale_y = sratio;
    }
  } else {
    *pScale_y = sratio;
  }
}

void CDiagIF::SetToFirstIncarnation()
{
  if (m_is_partial) {
    toprect_toprow=start_toprect_toprow;
    centerrow=start_centerrow;
    bottomrect_bottomrow=start_bottomrect_bottomrow;
    leftrect_leftcol=start_leftrect_leftcol;
    centercol=start_centercol;
    rightrect_rightcol=start_rightrect_rightcol;
    m_remaining_incarnations=m_stop_after_num_incarnations;
  } else {
    toprect_toprow=-1;
    centerrow=0;
    bottomrect_bottomrow=1;
    leftrect_leftcol=-1;
    centercol=0;
    rightrect_rightcol=1;
  }
  SetNonOverlap();
}

bool CDiagIF::SetToNextIncarnation()
{
  if (m_is_partial) {
    if (m_remaining_incarnations) {
      m_remaining_incarnations--;
    } else {
      return false;
    }
  }
  bottomrect_bottomrow++;
  if (bottomrect_bottomrow>=m_template_height) {
    centerrow++;
    if (centerrow>=m_template_height-1) {
      toprect_toprow++;
      if (toprect_toprow==m_template_height-2) {
	rightrect_rightcol++;
	if (rightrect_rightcol==m_template_width) {
	  centercol++;
	  if (centercol==m_template_width-1) {
	    leftrect_leftcol++;
	    if (leftrect_leftcol==m_template_width-2) {
	      return false;
	    }
	    centercol=leftrect_leftcol+1;
	  }
	  rightrect_rightcol=centercol+1;
	}
	toprect_toprow=-1;
      }
      centerrow=toprect_toprow+1;
    }
    bottomrect_bottomrow=centerrow+1;
  }
  SetNonOverlap();
  return true;
}

CIntegralFeature* CDiagIF::Copy() const
{
	return new CDiagIF(*this);
}

void CDiagIF::MakePartialFromCurrentForNumIncarnations(
  featnum num)
{
  m_is_partial=true;
  m_num_incarnations=num+1;
  start_toprect_toprow=toprect_toprow;
  start_centerrow=centerrow;
  start_bottomrect_bottomrow=bottomrect_bottomrow;
  start_leftrect_leftcol=leftrect_leftcol;
  start_centercol=centercol;
  start_rightrect_rightcol=rightrect_rightcol;
  m_remaining_incarnations=m_stop_after_num_incarnations=num;
}

#ifdef USE_MFC
void CDiagIF::Draw(CDC* pDC, int x_off, int y_off, int zoomfactor) const
{
  int lc = x_off + zoomfactor + leftrect_leftcol*zoomfactor;
  int cc = x_off + zoomfactor + centercol*zoomfactor;
  int rc = x_off + zoomfactor + rightrect_rightcol*zoomfactor;
  int tr = y_off + zoomfactor + toprect_toprow*zoomfactor;
  int cr = y_off + zoomfactor + centerrow*zoomfactor;
  int br = y_off + zoomfactor + bottomrect_bottomrow*zoomfactor;
  pDC->FillRect(CRect(lc, tr, cc, cr), g_pBlackbrush);
  pDC->Rectangle(cc, tr, rc, cr);
  pDC->Rectangle(lc, cr, cc, br);
  pDC->FillRect(CRect(cc, cr, rc, br), g_pBlackbrush);
}
#endif // USE_MFC

ostream& CDiagIF::output(ostream& os) const
{
  os << "Diag " << toprect_toprow << "," << centerrow 
     << "," << bottomrect_bottomrow << " x "
     << leftrect_leftcol << "," << centerrow
     << "," << rightrect_rightcol;
  return os;
}


/////////////////////////////////////////////////////////////////////////////
//
// CFourBoxesIF classes implementation
//
/////////////////////////////////////////////////////////////////////////////

/* they look like this:
*   b3
* b1  b4
*   b2
* 
* b1,b4 can't overlap,
* b2,b3 can't overlap,
* b1_left must be left of b2_left and b3_left
* b1_right must be right of b2_left and b3_left
* ... and so on, see SetToNextIncarnation
*/
CFourBoxesIF::CFourBoxesIF(int templateWidth, int templateHeight)
  : CIntegralFeature(templateWidth, templateHeight, IT_INVALID_FEATURE, false, 16*COST_GET+15*COST_ADD) 
{
}

CFourBoxesIF::CFourBoxesIF(int templateWidth, int templateHeight,
             const CRect* pB1, const CRect* pB2,
             const CRect* pB3, const CRect* pB4)
  : CIntegralFeature(templateWidth, templateHeight, IT_INVALID_FEATURE, false, 16*COST_GET+15*COST_ADD) 
{
  b1_left = pB1->left;  b1_top = pB1->top;
  b1_right = pB1->right;  b1_bottom = pB1->bottom;
  b2_left = pB2->left;  b2_top = pB2->top;
  b2_right = pB2->right;  b2_bottom = pB2->bottom;
  b3_left = pB3->left;  b3_top = pB3->top;
  b3_right = pB3->right;  b3_bottom = pB3->bottom;
  b4_left = pB4->left;  b4_top = pB4->top;
  b4_right = pB4->right;  b4_bottom = pB4->bottom;
  SetNonOverlap();
}

CFourBoxesIF::CFourBoxesIF(const CFourBoxesIF& frm)
  : CIntegralFeature(frm.m_template_width, frm.m_template_height,
		     frm.m_num_incarnations, frm.m_is_partial, 16*COST_GET+15*COST_ADD) 
{
  b1_left = frm.b1_left;  b1_top = frm.b1_top;
  b1_right = frm.b1_right;  b1_bottom = frm.b1_bottom;
  b2_left = frm.b2_left;  b2_top = frm.b2_top;
  b2_right = frm.b2_right;  b2_bottom = frm.b2_bottom;
  b3_left = frm.b3_left;  b3_top = frm.b3_top;
  b3_right = frm.b3_right;  b3_bottom = frm.b3_bottom;
  b4_left = frm.b4_left;  b4_top = frm.b4_top;
  b4_right = frm.b4_right;  b4_bottom = frm.b4_bottom;
  if (m_is_partial) {
    start_b1_left = frm.start_b1_left;  start_b1_top = frm.start_b1_top;
    start_b1_right = frm.start_b1_right;  start_b1_bottom = frm.start_b1_bottom;
    start_b2_left = frm.start_b2_left;  start_b2_top = frm.start_b2_top;
    start_b2_right = frm.start_b2_right;  start_b2_bottom = frm.start_b2_bottom;
    start_b3_left = frm.start_b3_left;  start_b3_top = frm.start_b3_top;
    start_b3_right = frm.start_b3_right;  start_b3_bottom = frm.start_b3_bottom;
    start_b4_left = frm.start_b4_left;  start_b4_top = frm.start_b4_top;
    start_b4_right = frm.start_b4_right;  start_b4_bottom = frm.start_b4_bottom;
    m_remaining_incarnations = frm.m_remaining_incarnations;
    m_stop_after_num_incarnations = frm.m_stop_after_num_incarnations;
  }
  SetNonOverlap();
}

CFourBoxesIF::CFourBoxesIF(istream& is, int template_width, int template_height)
  : CIntegralFeature(template_width, template_height, IT_INVALID_FEATURE, 0, 16*COST_GET+15*COST_ADD)
{
  char a, b, c, d, e, f;
  is >> e >> b1_left >> a >> b1_top >> b >> b1_right >> c >> b1_bottom >> f >> d;
  if (a!=',' || b!=',' || c!=',' || d!=',' || e!='(' || f!=')') {
    char* buf = (char*) alloca(256*sizeof(char));
    sprintf(buf, "error during construction of FourBoxesIF (%c|%c|%c|%c|%c)\n",
	   e, a, b, c, f);
    throw ITException(buf);
  }
  is >> e >> b2_left >> a >> b2_top >> b >> b2_right >> c >> b2_bottom >> f >> d;
  if (a!=',' || b!=',' || c!=',' || d!=',' || e!='(' || f!=')') {
    char* buf = (char*) alloca(256*sizeof(char));
    sprintf(buf, "error during construction of FourBoxesIF (%c|%c|%c|%c|%c)\n",
	   e, a, b, c, f);
    throw ITException(buf);
  }
  is >> e >> b3_left >> a >> b3_top >> b >> b3_right >> c >> b3_bottom >> f >> d;
  if (a!=',' || b!=',' || c!=',' || d!=',' || e!='(' || f!=')') {
    char* buf = (char*) alloca(256*sizeof(char));
    sprintf(buf, "error during construction of FourBoxesIF (%c|%c|%c|%c|%c)\n",
	   e, a, b, c, f);
    throw ITException(buf);
  }
  is >> e >> b4_left >> a >> b4_top >> b >> b4_right >> c >> b4_bottom >> f;
  if (a!=',' || b!=',' || c!=',' || d!=',' || e!='(' || f!=')') {
    char* buf = (char*) alloca(256*sizeof(char));
    sprintf(buf, "error during construction of FourBoxesIF (%c|%c|%c|%c|%c)\n",
	   e, a, b, c, f);
    throw ITException(buf);
  }
  SetNonOverlap();
}

bool CFourBoxesIF::Equals(const CFourBoxesIF& frm) const
{
  bool equal =
    (b1_left == frm.b1_left &&  b1_top == frm.b1_top &&
     b1_right == frm.b1_right &&  b1_bottom == frm.b1_bottom &&
     b2_left == frm.b2_left &&  b2_top == frm.b2_top &&
     b2_right == frm.b2_right &&  b2_bottom == frm.b2_bottom &&
     b3_left == frm.b3_left &&  b3_top == frm.b3_top &&
     b3_right == frm.b3_right &&  b3_bottom == frm.b3_bottom &&
     b4_left == frm.b4_left &&  b4_top == frm.b4_top &&
     b4_right == frm.b4_right &&  b4_bottom == frm.b4_bottom);
  return equal;
}
    
void CFourBoxesIF::SetNonOverlap() {
  m_non_overlap =
    (b1_right-b1_left) * (b1_bottom-b1_top) 
    + (b2_right-b2_left) * (b2_bottom-b2_top)
    - (b3_right-b3_left) * (b3_bottom-b3_top)
    - (b4_right-b4_left) * (b4_bottom-b4_top);
}

II_TYPE CFourBoxesIF::Compute(const CIntegralImage& image) const
{
  // b1 is the left-top positive box, b2 is the right-bottom positive box,
  // b3 is the left-top negative box, b4 is the right-bottom positive box 
	II_TYPE val_b1 = 
    image.GetElement(b1_right, b1_bottom)
    - image.GetElement(b1_left, b1_bottom)
    - image.GetElement(b1_right, b1_top)
    + image.GetElement(b1_left, b1_top);
	II_TYPE val_b2 = 
    image.GetElement(b2_right, b2_bottom)
    - image.GetElement(b2_left, b2_bottom)
    - image.GetElement(b2_right, b2_top)
    + image.GetElement(b2_left, b2_top);
	II_TYPE val_b3 = 
    image.GetElement(b3_right, b3_bottom)
    - image.GetElement(b3_left, b3_bottom)
    - image.GetElement(b3_right, b3_top)
    + image.GetElement(b3_left, b3_top);
	II_TYPE val_b4 = 
    image.GetElement(b4_right, b4_bottom)
    - image.GetElement(b4_left, b4_bottom)
    - image.GetElement(b4_right, b4_top)
    + image.GetElement(b4_left, b4_top);

	return val_b1+val_b2-val_b3-val_b4;
}

II_TYPE CFourBoxesIF::ComputeScaled(const CIntegralImage& image, II_TYPE mean, int left, int top) const
{
	int placed_b1_left   = left + scaled_b1_left;
	int placed_b1_right  = left + scaled_b1_right;
	int placed_b1_top    = top  + scaled_b1_top;
	int placed_b1_bottom = top  + scaled_b1_bottom;
	int placed_b2_left   = left + scaled_b2_left;
	int placed_b2_right  = left + scaled_b2_right;
	int placed_b2_top    = top  + scaled_b2_top;
	int placed_b2_bottom = top  + scaled_b2_bottom;
	int placed_b3_left   = left + scaled_b3_left;
	int placed_b3_right  = left + scaled_b3_right;
	int placed_b3_top    = top  + scaled_b3_top;
	int placed_b3_bottom = top  + scaled_b3_bottom;
	int placed_b4_left   = left + scaled_b4_left;
	int placed_b4_right  = left + scaled_b4_right;
	int placed_b4_top    = top  + scaled_b4_top;
	int placed_b4_bottom = top  + scaled_b4_bottom;

  // b1 is the left-top positive box, b2 is the right-bottom positive box,
  // b3 is the left-top negative box, b4 is the right-bottom positive box 
	II_TYPE val_b1 = 
    image.GetElement(placed_b1_right, placed_b1_bottom)
    - image.GetElement(placed_b1_left, placed_b1_bottom)
    - image.GetElement(placed_b1_right, placed_b1_top)
    + image.GetElement(placed_b1_left, placed_b1_top);
	II_TYPE val_b2 = 
    image.GetElement(placed_b2_right, placed_b2_bottom)
    - image.GetElement(placed_b2_left, placed_b2_bottom)
    - image.GetElement(placed_b2_right, placed_b2_top)
    + image.GetElement(placed_b2_left, placed_b2_top);
	II_TYPE val_b3 = 
    image.GetElement(placed_b3_right, placed_b3_bottom)
    - image.GetElement(placed_b3_left, placed_b3_bottom)
    - image.GetElement(placed_b3_right, placed_b3_top)
    + image.GetElement(placed_b3_left, placed_b3_top);
	II_TYPE val_b4 = 
    image.GetElement(placed_b4_right, placed_b4_bottom)
    - image.GetElement(placed_b4_left, placed_b4_bottom)
    - image.GetElement(placed_b4_right, placed_b4_top)
    + image.GetElement(placed_b4_left, placed_b4_top);

	II_TYPE val = val_b1+val_b2-val_b3-val_b4;
	II_TYPE scaled_val = val/m_global_scale;
	II_TYPE mean_adjust = m_non_overlap*mean;

#ifdef DEBUG
	if (g_printlots) {
	  VERBOSE4(1, "val_b1 %f, val_b2 %f, val_b3 %f, val_b4 %f",
            val_b1, val_b1, val_b1, val_b1);
	  VERBOSE2(1, "scaled_val %f, mean_adjust %f", 
            scaled_val, mean_adjust);
	  VERBOSE4(1, "b1 scaled: left %f, top %f, right %f, bottom %f",
            scaled_b1_left, scaled_b1_top, scaled_b1_right, scaled_b1_bottom);
	  VERBOSE4(1, "b2 scaled: left %f, top %f, right %f, bottom %f",
            scaled_b2_left, scaled_b2_top, scaled_b2_right, scaled_b2_bottom);
	  VERBOSE4(1, "b3 scaled: left %f, top %f, right %f, bottom %f",
            scaled_b3_left, scaled_b3_top, scaled_b3_right, scaled_b3_bottom);
	  VERBOSE4(1, "b4 scaled: left %f, top %f, right %f, bottom %f",
            scaled_b4_left, scaled_b4_top, scaled_b4_right, scaled_b4_bottom);
	}
#endif // DEBUG
	return scaled_val-mean_adjust;
}

void CFourBoxesIF::ScaleX(II_TYPE scale_x)
{
  if (b1_left==-1) {
    scaled_b1_left = -1;
    scaled_b1_right = (int)((II_TYPE)(b1_right+1)*scale_x) - 1;
  } else {
    scaled_b1_left = (int)((II_TYPE)b1_left*scale_x);
    scaled_b1_right = (int)((II_TYPE)b1_right*scale_x);
  }
  if (b2_left==-1) {
    scaled_b2_left = -1;
    scaled_b2_right = (int)((II_TYPE)(b2_right+1)*scale_x) - 1;
  } else {
    scaled_b2_left = (int)((II_TYPE)b2_left*scale_x);
    scaled_b2_right = (int)((II_TYPE)b2_right*scale_x);
  }
  if (b3_left==-1) {
    scaled_b3_left = -1;
    scaled_b3_right = (int)((II_TYPE)(b3_right+1)*scale_x) - 1;
  } else {
    scaled_b3_left = (int)((II_TYPE)b3_left*scale_x);
    scaled_b3_right = (int)((II_TYPE)b3_right*scale_x);
  }
  if (b4_left==-1) {
    scaled_b4_left = -1;
    scaled_b4_right = (int)((II_TYPE)(b4_right+1)*scale_x) - 1;
  } else {
    scaled_b4_left = (int)((II_TYPE)b4_left*scale_x);
    scaled_b4_right = (int)((II_TYPE)b4_right*scale_x);
  }
  ASSERT(-1<=scaled_b1_left);
  ASSERT(scaled_b1_right<(int)((II_TYPE)m_template_width*scale_x));
  ASSERT(-1<=scaled_b2_left);
  ASSERT(scaled_b2_right<(int)((II_TYPE)m_template_width*scale_x));
  ASSERT(-1<=scaled_b3_left);
  ASSERT(scaled_b3_right<(int)((II_TYPE)m_template_width*scale_x));
  ASSERT(-1<=scaled_b4_left);
  ASSERT(scaled_b4_right<(int)((II_TYPE)m_template_width*scale_x));
}

void CFourBoxesIF::ScaleY(II_TYPE scale_y)
{
  if (b1_top==-1) {
    scaled_b1_top = -1;
    scaled_b1_bottom = (int)((II_TYPE)(b1_bottom+1)*scale_y) - 1;
  } else {
    scaled_b1_top = (int)((II_TYPE)b1_top*scale_y);
    scaled_b1_bottom = (int)((II_TYPE)b1_bottom*scale_y);
  }
  if (b2_top==-1) {
    scaled_b2_top = -1;
    scaled_b2_bottom = (int)((II_TYPE)(b2_bottom+1)*scale_y) - 1;
  } else {
    scaled_b2_top = (int)((II_TYPE)b2_top*scale_y);
    scaled_b2_bottom = (int)((II_TYPE)b2_bottom*scale_y);
  }
  if (b3_top==-1) {
    scaled_b3_top = -1;
    scaled_b3_bottom = (int)((II_TYPE)(b3_bottom+1)*scale_y) - 1;
  } else {
    scaled_b3_top = (int)((II_TYPE)b3_top*scale_y);
    scaled_b3_bottom = (int)((II_TYPE)b3_bottom*scale_y);
  }
  if (b4_top==-1) {
    scaled_b4_top = -1;
    scaled_b4_bottom = (int)((II_TYPE)(b4_bottom+1)*scale_y) - 1;
  } else {
    scaled_b4_top = (int)((II_TYPE)b4_top*scale_y);
    scaled_b4_bottom = (int)((II_TYPE)b4_bottom*scale_y);
  }
  ASSERT(-1<=scaled_b1_top);
  ASSERT(scaled_b1_bottom<(int)((II_TYPE)m_template_height*scale_y));
  ASSERT(-1<=scaled_b2_top);
  ASSERT(scaled_b2_bottom<(int)((II_TYPE)m_template_height*scale_y));
  ASSERT(-1<=scaled_b3_top);
  ASSERT(scaled_b3_bottom<(int)((II_TYPE)m_template_height*scale_y));
  ASSERT(-1<=scaled_b4_top);
  ASSERT(scaled_b4_bottom<(int)((II_TYPE)m_template_height*scale_y));
}

void CFourBoxesIF::Scale(II_TYPE scale_x, II_TYPE scale_y)
{
  ScaleX(scale_x);
  ScaleY(scale_y);
  m_global_scale = scale_x*scale_y;
}

void CFourBoxesIF::EvenOutScales(II_TYPE* pScale_x, II_TYPE* pScale_y,
        int scaled_template_width, int scaled_template_height)
{
  int rect1 = b1_right-b1_left;
  int rect2 = b2_right-b2_left;
  int rect3 = b3_right-b3_left;
  int rect4 = b4_right-b4_left;
  II_TYPE xratio1 = (II_TYPE)rect1/(II_TYPE)rect2;
  II_TYPE xratio2 = (II_TYPE)rect2/(II_TYPE)rect3;
  II_TYPE xratio3 = (II_TYPE)rect3/(II_TYPE)rect4;
  int srect1 = scaled_b1_right-scaled_b1_left;
  int srect2 = scaled_b2_right-scaled_b2_left;
  int srect3 = scaled_b3_right-scaled_b3_left;
  int srect4 = scaled_b4_right-scaled_b4_left;
  II_TYPE xsratio1 = (II_TYPE)srect1/(II_TYPE)srect2;
  II_TYPE xsratio2 = (II_TYPE)srect2/(II_TYPE)srect3;
  II_TYPE xsratio3 = (II_TYPE)srect3/(II_TYPE)srect4;

  int rightmost = max(scaled_b2_right, scaled_b4_right);
  int leftmost = min(scaled_b1_left, scaled_b3_left);
  bool too_wide = rightmost-leftmost > scaled_template_width;

  while (!too_wide && 
	 (fabs(xratio1-xsratio1)>=SCALE_DIFFERENCE_EPSILON
	  || fabs(xratio2-xsratio2)>=SCALE_DIFFERENCE_EPSILON
	  || fabs(xratio3-xsratio3)>=SCALE_DIFFERENCE_EPSILON)) {
    if (xratio1>xsratio1) {
      scaled_b1_right++;
      if (scaled_b4_left<scaled_b1_right) {
        scaled_b4_left++;
        scaled_b4_right++;
      }
      srect1++;
    } else if (xratio1<xsratio1 || xratio2>xsratio2) {
      scaled_b2_right++;
      srect2++;
    } else if (xratio2<xsratio2 || xratio3>xsratio3) {
      scaled_b3_right++;
      srect3++;
    } else if (xratio3<xsratio3) {
      scaled_b4_right++;
      srect4++;
    } else {
      throw ITException("error during CFourBoxesIF xscaling - e too small?");
    }

    rightmost = max(scaled_b2_right, scaled_b4_right);
    leftmost = min(scaled_b1_left, scaled_b3_left);
    too_wide = rightmost-leftmost > scaled_template_width;

    xsratio1 = (II_TYPE)srect1/(II_TYPE)srect2;
    xsratio2 = (II_TYPE)srect2/(II_TYPE)srect3;
    xsratio3 = (II_TYPE)srect3/(II_TYPE)srect4;
  }
  ASSERT(too_wide || fabs((II_TYPE)rect1/(II_TYPE)rect3
              -(II_TYPE)srect1/(II_TYPE)srect3)<SCALE_DIFFERENCE_EPSILON);
  ASSERT(too_wide || fabs((II_TYPE)rect1/(II_TYPE)rect4
              -(II_TYPE)srect1/(II_TYPE)srect4)<SCALE_DIFFERENCE_EPSILON);


  int overlap = rightmost-scaled_template_width+1; 
  // overlap: by how much is rightmost too far to the right? 
  // it must not be larger than scaled_template_width-1
  if (overlap>0) {
    // went outside template boundaries
    if (too_wide) {
      // feature too big, use int scale
      *pScale_x = floor(*pScale_x);
      ScaleX(*pScale_x);
    } else {
      // shift feature by overlap
      scaled_b1_left -= overlap;
      scaled_b1_right -= overlap;
      scaled_b2_left -= overlap;
      scaled_b2_right -= overlap;
      scaled_b3_left -= overlap;
      scaled_b3_right -= overlap;
      scaled_b4_left -= overlap;
      scaled_b4_right -= overlap;
      *pScale_x = xsratio1;
    }
  } else {
    *pScale_x = xsratio1;
  }
  ASSERT(-1<=scaled_b1_left && scaled_b1_right<scaled_template_width);
  ASSERT(-1<=scaled_b2_left && scaled_b2_right<scaled_template_width);
  ASSERT(-1<=scaled_b3_left && scaled_b3_right<scaled_template_width);
  ASSERT(-1<=scaled_b4_left && scaled_b4_right<scaled_template_width);

  rect1 = b1_bottom-b1_top;
  rect2 = b2_bottom-b2_top;
  rect3 = b3_bottom-b3_top;
  rect4 = b4_bottom-b4_top;
  II_TYPE yratio1 = (II_TYPE)rect1/(II_TYPE)rect2;
  II_TYPE yratio2 = (II_TYPE)rect2/(II_TYPE)rect3;
  II_TYPE yratio3 = (II_TYPE)rect3/(II_TYPE)rect4;
  srect1 = scaled_b1_bottom-scaled_b1_top;
  srect2 = scaled_b2_bottom-scaled_b2_top;
  srect3 = scaled_b3_bottom-scaled_b3_top;
  srect4 = scaled_b4_bottom-scaled_b4_top;
  II_TYPE ysratio1 = (II_TYPE)srect1/(II_TYPE)srect2;
  II_TYPE ysratio2 = (II_TYPE)srect2/(II_TYPE)srect3;
  II_TYPE ysratio3 = (II_TYPE)srect3/(II_TYPE)srect4;
  
  int bottommost = max(scaled_b2_bottom, scaled_b4_bottom);
  int topmost = min(scaled_b1_top, scaled_b3_top);
  bool too_high = bottommost-topmost > scaled_template_height;

  while (!too_high 
	 && (fabs(yratio1-ysratio1)>=SCALE_DIFFERENCE_EPSILON
	     || fabs(yratio2-ysratio2)>=SCALE_DIFFERENCE_EPSILON
	     || fabs(yratio3-ysratio3)>=SCALE_DIFFERENCE_EPSILON)) 
  {
    if (yratio1>ysratio1) {
      scaled_b1_bottom++;
      if (scaled_b2_top<scaled_b1_bottom) {
        scaled_b2_top++;
        scaled_b2_bottom++;
      }
      srect1++;
    } else if (yratio1<ysratio1 || yratio2>ysratio2) {
      scaled_b2_bottom++;
      srect2++;
    } else if (yratio2<ysratio2 || yratio3>ysratio3) {
      scaled_b3_bottom++;
      if (scaled_b4_top<scaled_b3_bottom) {
        scaled_b4_top++;
        scaled_b4_bottom++;
      }
      srect3++;
    } else if (yratio3<ysratio3) {
      scaled_b4_bottom++;
      srect4++;
    } else {
      throw ITException("error during CFourBoxesIF yscaling - e too small?");
    }

    bottommost = max(scaled_b2_bottom, scaled_b4_bottom);
    topmost = min(scaled_b1_top, scaled_b3_top);
    too_high = bottommost-topmost > scaled_template_height;

    ysratio1 = (II_TYPE)srect1/(II_TYPE)srect2;
    ysratio2 = (II_TYPE)srect2/(II_TYPE)srect3;
    ysratio3 = (II_TYPE)srect3/(II_TYPE)srect4;
  }
  ASSERT(too_high || fabs((II_TYPE)rect1/(II_TYPE)rect3
              -(II_TYPE)srect1/(II_TYPE)srect3)<SCALE_DIFFERENCE_EPSILON);
  ASSERT(too_high || fabs((II_TYPE)rect1/(II_TYPE)rect4
              -(II_TYPE)srect1/(II_TYPE)srect4)<SCALE_DIFFERENCE_EPSILON);

  overlap = bottommost-scaled_template_height+1;
  // overlap: by how much is bottommost too far down? 
  // it must not be larger than scaled_template_height-1
  if (overlap>0) {
    // went outside template boundaries
    if (too_high) {
      // feature too big, use int scale
      *pScale_y = floor(*pScale_y);
      ScaleY(*pScale_y);
    } else {
      // shift feature by overlap
      scaled_b1_top -= overlap;
      scaled_b1_bottom -= overlap;
      scaled_b2_top -= overlap;
      scaled_b2_bottom -= overlap;
      scaled_b3_top -= overlap;
      scaled_b3_bottom -= overlap;
      scaled_b4_top -= overlap;
      scaled_b4_bottom -= overlap;
      *pScale_y = ysratio1;
    }
  } else {
    *pScale_y = ysratio1;
  }

  ASSERT(-1<=scaled_b1_top && scaled_b1_bottom<scaled_template_height);
  ASSERT(-1<=scaled_b2_top && scaled_b2_bottom<scaled_template_height);
  ASSERT(-1<=scaled_b3_top && scaled_b3_bottom<scaled_template_height);
  ASSERT(-1<=scaled_b4_top && scaled_b4_bottom<scaled_template_height);
}

void CFourBoxesIF::SetToFirstIncarnation()
{
  if (m_is_partial) {
    b1_left = start_b1_left;  b1_top = start_b1_top;
    b1_right = start_b1_right;  b1_bottom = start_b1_bottom;
    b2_left = start_b2_left;  b2_top = start_b2_top;
    b2_right = start_b2_right;  b2_bottom = start_b2_bottom;
    b3_left = start_b3_left;  b3_top = start_b3_top;
    b3_right = start_b3_right;  b3_bottom = start_b3_bottom;
    b4_left = start_b4_left;  b4_top = start_b4_top;
    b4_right = start_b4_right;  b4_bottom = start_b4_bottom;
    m_remaining_incarnations = m_stop_after_num_incarnations;
  } else {
    b1_left  =-1;  b1_top    = 0;
    b1_right = 1;  b1_bottom = 2;
    b2_left  = 0;  b2_top    = 2;
    b2_right = 2;  b2_bottom = 4;
    b3_left  = 0;  b3_top    =-1;
    b3_right = 2;  b3_bottom = 1;
    b4_left  = 1;  b4_top    = 1;
    b4_right = 3;  b4_bottom = 3;
  }
  SetNonOverlap();
}

bool CFourBoxesIF::SetToNextIncarnation()
{
	if (m_is_partial) {
		if (m_remaining_incarnations) {
			m_remaining_incarnations--;
		} else {
			return false;
		}
	}
  b4_right++;
  if (b4_right>=m_template_width) {
    b2_right++;
    b3_right++;
    if (b2_right>=m_template_width-1) {
      b1_right++;
      b4_left++;
      if (b1_right>=m_template_width-2) {
        b2_left++;
        b3_left++;
        if (b2_left>=m_template_width-3) {
          b1_left++;
          if (b1_left>=m_template_width-4) {
            b2_bottom++;
            if (b2_bottom>=m_template_height) {
              b4_bottom++;
              if (b4_bottom>=m_template_height-1) {
                b1_bottom++;
                b2_top++;
                if (b1_bottom>=m_template_height-2) {
                  b3_bottom++;
                  b4_top++;
                  if (b3_bottom>=m_template_height-3) {
                    b1_top++;
                    if (b1_top>=m_template_height-4) {
                      b3_top++;
                      if (b3_top>=m_template_height-5) {
                        return false;
                      }
                      b1_top = b3_top+1;
                    }
                    b3_bottom = b1_top+1;
                    b4_top = b3_bottom;
                  }
                  b1_bottom = b3_bottom+1;
                  b2_top = b1_bottom;
                }
                b4_bottom = b1_bottom+1;
              }
              b2_bottom = b4_bottom+1;
            }
            b1_left = -1;
          }
          b2_left = b1_left+1;
          b3_left = b2_left;
        }
        b1_right = b2_left+1;
        b4_left = b1_right;
      }
      b2_right = b1_right+1;
      b3_right = b2_right;
    }
    b4_right = b2_right+1;
  }
  SetNonOverlap();
	return true;
}

CIntegralFeature* CFourBoxesIF::Copy() const
{
	return new CFourBoxesIF(*this);
}

void CFourBoxesIF::MakePartialFromCurrentForNumIncarnations(
  featnum num)
{
  m_is_partial = true;
  m_num_incarnations=num+1;
  start_b1_left = b1_left;  start_b1_top = b1_top;
  start_b1_right = b1_right;  start_b1_bottom = b1_bottom;
  start_b2_left = b2_left;  start_b2_top = b2_top;
  start_b2_right = b2_right;  start_b2_bottom = b2_bottom;
  start_b3_left = b3_left;  start_b3_top = b3_top;
  start_b3_right = b3_right;  start_b3_bottom = b3_bottom;
  start_b4_left = b4_left;  start_b4_top = b4_top;
  start_b4_right = b4_right;  start_b4_bottom = b4_bottom;
  m_remaining_incarnations = m_stop_after_num_incarnations = num;
}

#ifdef USE_MFC
void CFourBoxesIF::Draw(CDC* pDC, int x_off, int y_off, int zoomfactor) const
{
	int l1 = x_off + zoomfactor + b1_left*zoomfactor;
	int t1 = y_off + zoomfactor + b1_top*zoomfactor;
	int r1 = x_off + zoomfactor + b1_right*zoomfactor;
	int b1 = y_off + zoomfactor + b1_bottom*zoomfactor;
	int l2 = x_off + zoomfactor + b2_left*zoomfactor;
	int t2 = y_off + zoomfactor + b2_top*zoomfactor;
	int r2 = x_off + zoomfactor + b2_right*zoomfactor;
	int b2 = y_off + zoomfactor + b2_bottom*zoomfactor;
	int l3 = x_off + zoomfactor + b3_left*zoomfactor;
	int t3 = y_off + zoomfactor + b3_top*zoomfactor;
	int r3 = x_off + zoomfactor + b3_right*zoomfactor;
	int b3 = y_off + zoomfactor + b3_bottom*zoomfactor;
	int l4 = x_off + zoomfactor + b4_left*zoomfactor;
	int t4 = y_off + zoomfactor + b4_top*zoomfactor;
	int r4 = x_off + zoomfactor + b4_right*zoomfactor;
	int b4 = y_off + zoomfactor + b4_bottom*zoomfactor;
  // box 1 and 2 in black
	pDC->FillRect(CRect(l1, t1, r1, b1), g_pBlackbrush);
	pDC->FillRect(CRect(l2, t2, r2, b2), g_pBlackbrush);
  // box 3 and 4 in outline, XOR the screen
  unsigned char bits[9];
  bits[0] = bits[1] = bits[3] = bits[4] = bits[6] = bits[7] = 0;
  bits[2] = bits[5] = bits[8] = 255;
  CBitmap bmp;
  bmp.CreateBitmap(3, 3, 1, 8, &bits);
  CBrush brush;
  brush.CreatePatternBrush(&bmp);
  pDC->FrameRect(CRect(l3, t3, r3, b3), &brush);
  pDC->FrameRect(CRect(l4, t4, r4, b4), &brush);
}
#endif // USE_MFC

ostream& CFourBoxesIF::output(ostream& os) const
{
  os << "FourBoxes " 
     << "(" << b1_left << "," << b1_top << "," << b1_right << "," << b1_bottom << "),"
     << "(" << b2_left << "," << b2_top << "," << b2_right << "," << b2_bottom << "),"
     << "(" << b3_left << "," << b3_top << "," << b3_right << "," << b3_bottom << "),"
     << "(" << b4_left << "," << b4_top << "," << b4_right << "," << b4_bottom << ")";
  return os;
}




