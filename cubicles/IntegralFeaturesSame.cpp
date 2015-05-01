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
  * $Id: IntegralFeaturesSame.cpp,v 1.29 2004/11/11 01:58:58 matz Exp $
**/

// IntegralFeatures describe the rectangular areas whose pixel
// sums are compared to a weak classifier's threshold.  There
// are many different types of IntegralFeatures.
// IntegralFeaturesSame are features just as IntegralFeatures, 
// but their rectangular regions observe more stringent constraints
// during feature instance generation (see SetToNextIncarnation).
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
#include <iostream>



#ifdef _DEBUG
#ifdef USE_MFC
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif // USE_MFC
#endif // _DEBUG


#ifdef WIN32
#include <malloc.h>
#define alloca _alloca
#endif



/////////////////////////////////////////////////////////////////////////////
//
// CLeftRightSameIF classes implementation
//
/////////////////////////////////////////////////////////////////////////////

CLeftRightSameIF::CLeftRightSameIF(int templateWidth, int templateHeight)
  : CLeftRightIF(templateWidth, templateHeight) 
{
}

CLeftRightSameIF::CLeftRightSameIF(const CLeftRightSameIF& frm)
  : CLeftRightIF(frm) 
{
}

CLeftRightSameIF::CLeftRightSameIF(int templateWidth, int templateHeight,
                           int _toprow, int _bottomrow,
                           int _leftrect_leftcol, int _centercol,
                           int _rightrect_rightcol)
  : CLeftRightIF(templateWidth, templateHeight) 
{
  toprow = _toprow;
  bottomrow = _bottomrow;
  leftrect_leftcol = _leftrect_leftcol;
  centercol = _centercol;
  rightrect_rightcol = _rightrect_rightcol;
  SetNonOverlap();
}

CLeftRightSameIF::CLeftRightSameIF(istream& is, int template_width, int template_height)
  : CLeftRightIF(template_width, template_height)
{
  char a, b, c;
  string str;
  is >> toprow >> a >> bottomrow >> str
     >> leftrect_leftcol >> b >> centercol >> c >> rightrect_rightcol;
  if (a!=',' || b!=',' || c!=',' || str!="x") {
    char* buf = (char*) alloca((256+str.length())*sizeof(char));
    sprintf(buf, "error during construction of LeftRightSameIF (%c|%c|%c|%s)\n",
	   a, b, c, str.c_str());
    throw ITException(buf);
  }
  SetNonOverlap();
}

bool CLeftRightSameIF::SetToNextIncarnation()
{
	if (m_is_partial) {
		if (m_remaining_incarnations) {
			m_remaining_incarnations--;
		} else {
			return false;
		}
	}
	rightrect_rightcol+=2;
	centercol++;
	if (rightrect_rightcol>=m_template_width) {
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
		rightrect_rightcol=centercol+1;
	}
    SetNonOverlap();
	return true;
}

CIntegralFeature* CLeftRightSameIF::Copy() const
{
	return new CLeftRightSameIF(*this);
}

ostream& CLeftRightSameIF::output(ostream& os) const
{
  os << "LeftRightSame " << toprow << "," << bottomrow << " x ";
  os << leftrect_leftcol << "," << centercol << "," << rightrect_rightcol;
  return os;
}






/////////////////////////////////////////////////////////////////////////////
//
// CUpDownSameIF classes implementation
//
/////////////////////////////////////////////////////////////////////////////

CUpDownSameIF::CUpDownSameIF(int templateWidth, int templateHeight)
  : CUpDownIF(templateWidth, templateHeight) 
{
}

CUpDownSameIF::CUpDownSameIF(const CUpDownSameIF& frm)
  : CUpDownIF(frm) 
{
}

CUpDownSameIF::CUpDownSameIF(int templateWidth, int templateHeight,
                     int _toprect_toprow, int _centerrow,
                     int _bottomrect_bottomrow,
                     int _leftcol, int _rightcol)
  : CUpDownIF(templateWidth, templateHeight) 
{
  toprect_toprow = _toprect_toprow;
  centerrow = _centerrow;
  bottomrect_bottomrow = _bottomrect_bottomrow;
  leftcol = _leftcol;
  rightcol = _rightcol;
  SetNonOverlap();
}

CUpDownSameIF::CUpDownSameIF(istream& is, int templateWidth, int templateHeight)
  : CUpDownIF(templateWidth, templateHeight) 
{
  char a, b, c;
  string str;
  is >> toprect_toprow >> a >> centerrow >> b
     >> bottomrect_bottomrow >> str >> leftcol >> c >> rightcol;
  if (a!=',' || b!=',' || c!=',' || str!="x") {
    char* buf = (char*) alloca((256+str.length())*sizeof(char));
    sprintf(buf, "error during construction of UpDownSameIF (%c|%c|%c|%s)\n",
	   a, b, c, str.c_str());
    throw ITException(buf);
  }
  SetNonOverlap();
}

bool CUpDownSameIF::SetToNextIncarnation()
{
	if (m_is_partial) {
		if (m_remaining_incarnations) {
			m_remaining_incarnations--;
		} else {
			return false;
		}
	}
	bottomrect_bottomrow+=2;
	centerrow++;
	if (bottomrect_bottomrow>=m_template_height) {
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
		bottomrect_bottomrow=centerrow+1;
	}
    SetNonOverlap();
	return true;
}

CIntegralFeature* CUpDownSameIF::Copy() const
{
	return new CUpDownSameIF(*this);
}

ostream& CUpDownSameIF::output(ostream& os) const
{
  os << "UpDownSame " << toprect_toprow << "," << centerrow 
     << "," << bottomrect_bottomrow << " x ";
  os << leftcol << "," << rightcol;
  return os;
}







/////////////////////////////////////////////////////////////////////////////
//
// CLeftCenterRightSameIF classes implementation
//
/////////////////////////////////////////////////////////////////////////////

CLeftCenterRightSameIF::CLeftCenterRightSameIF(int templateWidth, int templateHeight)
  : CLeftCenterRightIF(templateWidth, templateHeight) 
{
}

CLeftCenterRightSameIF::CLeftCenterRightSameIF(int templateWidth, int templateHeight,
                                       int _toprow, int _bottomrow,
                                       int _leftrect_leftcol, int _leftrect_rightcol,
                                       int _rightrect_leftcol,
                                       int _rightrect_rightcol)
  : CLeftCenterRightIF(templateWidth, templateHeight) 
{
  toprow = _toprow;
  bottomrow = _bottomrow;
  leftrect_leftcol = _leftrect_leftcol;
  leftrect_rightcol = _leftrect_rightcol;
  rightrect_leftcol = _rightrect_leftcol;
  rightrect_rightcol = _rightrect_rightcol;
  SetNonOverlap();
}

CLeftCenterRightSameIF::CLeftCenterRightSameIF(const CLeftCenterRightSameIF& frm)
  : CLeftCenterRightIF(frm) 
{
}

CLeftCenterRightSameIF::CLeftCenterRightSameIF(istream& is, int templateWidth, int templateHeight)
  : CLeftCenterRightIF(templateWidth, templateHeight) 
{
  char a, b, c;
  string strx, strslash;
  is >> toprow >> a >> bottomrow >> strx 
     >> leftrect_leftcol >> b >> leftrect_rightcol >> strslash
     >> rightrect_leftcol >> c >> rightrect_rightcol;
  if (a!=',' || b!=',' || c!=',' || strx!="x" || strslash!="/") {
    char* buf = (char*) alloca((256+strx.length()+strslash.length())*sizeof(char));
    sprintf(buf, "error during construction of LeftCenterRightSameIF (%c|%c|%c|%s|%s)\n",
	   a, b, c, strx.c_str(), strslash.c_str());
    throw ITException(buf);
  }
  SetNonOverlap();
}

bool CLeftCenterRightSameIF::SetToNextIncarnation()
{
	if (m_is_partial) {
		if (m_remaining_incarnations) {
			m_remaining_incarnations--;
		} else {
			return false;
		}
	}
	rightrect_rightcol+=3;
	rightrect_leftcol+=2;
	leftrect_rightcol++;
	if (rightrect_rightcol>=m_template_width) {
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
		rightrect_leftcol=leftrect_rightcol+1;
		rightrect_rightcol=rightrect_leftcol+1;
	}
    SetNonOverlap();
	return true;
}

CIntegralFeature* CLeftCenterRightSameIF::Copy() const 
{
  return new CLeftCenterRightSameIF(*this);
}

ostream& CLeftCenterRightSameIF::output(ostream& os) const
{
  os << "LeftCenterRightSame " << toprow << "," << bottomrow << " x ";
  os << leftrect_leftcol << "," << leftrect_rightcol << " / ";
  os << rightrect_leftcol << "," << rightrect_rightcol;
  return os;
}




/////////////////////////////////////////////////////////////////////////////
//
// CSevenColumnsSameIF classes implementation
//
/////////////////////////////////////////////////////////////////////////////

CSevenColumnsSameIF::CSevenColumnsSameIF(int templateWidth, int templateHeight)
  : CSevenColumnsIF(templateWidth, templateHeight) 
{
}

CSevenColumnsSameIF::CSevenColumnsSameIF(int templateWidth, int templateHeight,
                                 int _toprow, int _bottomrow,
                                 int _1, int _2, int _3, int _4, int _5, int _6,
                                 int _7, int _8)
  : CSevenColumnsIF(templateWidth, templateHeight) 
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

CSevenColumnsSameIF::CSevenColumnsSameIF(const CSevenColumnsSameIF& frm)
  : CSevenColumnsIF(frm) 
{
}

CSevenColumnsSameIF::CSevenColumnsSameIF(istream& is, int template_width, int template_height)
  : CSevenColumnsIF(is, template_width, template_height)
{
}

bool CSevenColumnsSameIF::SetToNextIncarnation()
{
	if (m_is_partial) {
		if (m_remaining_incarnations) {
			m_remaining_incarnations--;
		} else {
			return false;
		}
	}
	col1_left += 1;
	col2_left += 2;
	col3_left += 3;
	col4_left += 4;
	col5_left += 5;
	col6_left += 6;
	col7_left += 7;
	col7_right += 8;
	if (col7_right>=m_template_width) {
    col1_left ++;
    if (col1_left>=m_template_width-7) {
		  bottomrow++;
		  if (bottomrow==m_template_height) {
			  toprow++;
			  if (toprow==m_template_height-1) {
				  return false;
			  }
			  bottomrow=toprow+1;
		  }
      col1_left = -1;
    }
    col2_left = col1_left+1;
    col3_left = col2_left+1;
    col4_left = col3_left+1;
    col5_left = col4_left+1;
    col6_left = col5_left+1;
    col7_left = col6_left+1;
    col7_right= col7_left+1;
	}
  SetNonOverlap();
	return true;
}

CIntegralFeature* CSevenColumnsSameIF::Copy() const
{
	return new CSevenColumnsSameIF(*this);
}

ostream& CSevenColumnsSameIF::output(ostream& os) const
{
  os << "SevenColumnsSame " << toprow << "," << bottomrow << " x ";
  os << col1_left << "," << col2_left << "," << col3_left << "," << col4_left << ",";
  os << col5_left << "," << col6_left << "," << col7_left << "," << col7_right;
  return os;
}




/////////////////////////////////////////////////////////////////////////////
//
// CSevenColumnsSimilarIF classes implementation
//
/////////////////////////////////////////////////////////////////////////////

CSevenColumnsSimilarIF::CSevenColumnsSimilarIF(int templateWidth, int templateHeight)
  : CSevenColumnsIF(templateWidth, templateHeight) 
{
}

CSevenColumnsSimilarIF::CSevenColumnsSimilarIF(int templateWidth, int templateHeight,
                                 int _toprow, int _bottomrow,
                                 int _1, int _2, int _3, int _4, int _5, int _6,
                                 int _7, int _8)
  : CSevenColumnsIF(templateWidth, templateHeight) 
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

CSevenColumnsSimilarIF::CSevenColumnsSimilarIF(const CSevenColumnsSimilarIF& frm)
  : CSevenColumnsIF(frm) 
{
}

CSevenColumnsSimilarIF::CSevenColumnsSimilarIF(istream& is, int template_width, int template_height)
  : CSevenColumnsIF(is, template_width, template_height)
{
}

bool CSevenColumnsSimilarIF::SetToNextIncarnation()
{
	if (m_is_partial) {
		if (m_remaining_incarnations) {
			m_remaining_incarnations--;
		} else {
			return false;
		}
	}
	col2_left += 1;
	col3_left += 1;
	col4_left += 2;
	col5_left += 2;
	col6_left += 3;
	col7_left += 3;
	col7_right+= 4;
	if (col7_right>=m_template_width) {
		int new_even_width = col3_left-col2_left+1;
		col2_left = col1_left+1;
		col3_left = col2_left+new_even_width;
		col4_left = col3_left+1;
		col5_left = col4_left+new_even_width;
		col6_left = col5_left+1;
		col7_left = col6_left+new_even_width;
		col7_right= col7_left+1;
		if (col7_right>=m_template_width) {
			col1_left += 1;
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
			col3_left = col2_left+1;
			col4_left = col3_left+1;
			col5_left = col4_left+1;
			col6_left = col5_left+1;
			col7_left = col6_left+1;
			col7_right = col7_left+1;
		}
	}
	SetNonOverlap();
	return true;
}

CIntegralFeature* CSevenColumnsSimilarIF::Copy() const
{
	return new CSevenColumnsSimilarIF(*this);
}

ostream& CSevenColumnsSimilarIF::output(ostream& os) const
{
  os << "SevenColumnsSimilar " << toprow << "," << bottomrow << " x ";
  os << col1_left << "," << col2_left << "," << col3_left << "," << col4_left << ",";
  os << col5_left << "," << col6_left << "," << col7_left << "," << col7_right;
  return os;
}





/////////////////////////////////////////////////////////////////////////////
//
// CDiagSameIF classes implementation
//
/////////////////////////////////////////////////////////////////////////////

CDiagSameIF::CDiagSameIF(int templateWidth, int templateHeight)
  : CDiagIF(templateWidth, templateHeight) 
{
}

CDiagSameIF::CDiagSameIF(int templateWidth, int templateHeight,
        int _toprect_toprow, int _centerrow,
        int _bottomrect_bottomrow,
        int _leftrect_leftcol, int _centercol,
        int _rightrect_rightcol)
  : CDiagIF(templateWidth, templateHeight) 
{
  toprect_toprow = _toprect_toprow;
  centerrow = _centerrow;
  bottomrect_bottomrow = _bottomrect_bottomrow;
  leftrect_leftcol = _leftrect_leftcol;
  centercol = _centercol;
  rightrect_rightcol = _rightrect_rightcol;
  SetNonOverlap();
}

CDiagSameIF::CDiagSameIF(const CDiagSameIF& frm)
  : CDiagIF(frm) 
{
}

CDiagSameIF::CDiagSameIF(istream& is, int templateWidth, int templateHeight)
  : CDiagIF(templateWidth, templateHeight) 
{
  char a, b, c, d;
  string str;
  is >> toprect_toprow >> a >> centerrow >> b
     >> bottomrect_bottomrow >> str
     >> leftrect_leftcol >> c >> centercol >> d
     >> rightrect_rightcol;
  if (a!=',' || b!=',' || str!="x" || c!=',' || d!=',') {
    char* buf = (char*) alloca((256+str.length())*sizeof(char));
    sprintf(buf, "error during construction of DiagSameIF (%c|%c|%s|%c|%c)\n",
	   a, b, str.c_str(), c, d);
    throw ITException(buf);
  }
  SetNonOverlap();
}

bool CDiagSameIF::SetToNextIncarnation()
{
  if (m_is_partial) {
    if (m_remaining_incarnations) {
      m_remaining_incarnations--;
    } else {
      return false;
    }
  }
  bottomrect_bottomrow+=2;
  centerrow++;
  if (bottomrect_bottomrow>=m_template_height) {
    toprect_toprow++;
    if (toprect_toprow==m_template_height-2) {
      rightrect_rightcol+=2;
      centercol++;
      if (rightrect_rightcol>=m_template_width) {
	leftrect_leftcol++;
	if (leftrect_leftcol==m_template_width-2) {
	  return false;
	}
	centercol=leftrect_leftcol+1;
	rightrect_rightcol=centercol+1;
      }
      toprect_toprow=-1;
    }
    centerrow=toprect_toprow+1;
    bottomrect_bottomrow=centerrow+1;
  }
  SetNonOverlap();
  return true;
}

CIntegralFeature* CDiagSameIF::Copy() const
{
  return new CDiagSameIF(*this);
}

ostream& CDiagSameIF::output(ostream& os) const
{
  os << "DiagSame " << toprect_toprow << "," << centerrow 
     << "," << bottomrect_bottomrow << " x ";
  os << leftrect_leftcol << "," << centercol
     << "," << rightrect_rightcol;
  return os;
}







/////////////////////////////////////////////////////////////////////////////
//
// CDiagSimilarIF classes implementation
//
/////////////////////////////////////////////////////////////////////////////

CDiagSimilarIF::CDiagSimilarIF(int templateWidth, int templateHeight)
  : CDiagIF(templateWidth, templateHeight) 
{
}

CDiagSimilarIF::CDiagSimilarIF(int templateWidth, int templateHeight,
        int _toprect_toprow, int _centerrow,
        int _bottomrect_bottomrow,
        int _leftrect_leftcol, int _centercol,
        int _rightrect_rightcol)
  : CDiagIF(templateWidth, templateHeight) 
{
  toprect_toprow = _toprect_toprow;
  centerrow = _centerrow;
  bottomrect_bottomrow = _bottomrect_bottomrow;
  leftrect_leftcol = _leftrect_leftcol;
  centercol = _centercol;
  rightrect_rightcol = _rightrect_rightcol;
  SetNonOverlap();
}

CDiagSimilarIF::CDiagSimilarIF(const CDiagSimilarIF& frm)
  : CDiagIF(frm) 
{
}

CDiagSimilarIF::CDiagSimilarIF(istream& is, int templateWidth, int templateHeight)
  : CDiagIF(templateWidth, templateHeight) 
{
  char a, b, c, d;
  string str;
  is >> toprect_toprow >> a >> centerrow >> b
     >> bottomrect_bottomrow >> str
     >> leftrect_leftcol >> c >> centercol >> d
     >> rightrect_rightcol;
  if (a!=',' || b!=',' || str!="x" || c!=',' || d!=',') {
    char* buf = (char*) alloca((256+str.length())*sizeof(char));
    sprintf(buf, "error during construction of DiagSimilarIF (%c|%c|%s|%c|%c)\n",
	   a, b, str.c_str(), c, d);
    throw ITException(buf);
  }
  SetNonOverlap();
}

bool CDiagSimilarIF::SetToNextIncarnation()
{
  if (m_is_partial) {
    if (m_remaining_incarnations) {
      m_remaining_incarnations--;
    } else {
      return false;
    }
  }
  bottomrect_bottomrow+=2;
  centerrow++;
  if (bottomrect_bottomrow>=m_template_height) {
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
    bottomrect_bottomrow=centerrow+1;
  }
  SetNonOverlap();
  return true;
}

CIntegralFeature* CDiagSimilarIF::Copy() const
{
  return new CDiagSimilarIF(*this);
}

ostream& CDiagSimilarIF::output(ostream& os) const
{
  os << "DiagSimilar " << toprect_toprow << "," << centerrow 
     << "," << bottomrect_bottomrow << " x ";
  os << leftrect_leftcol << "," << centercol
     << "," << rightrect_rightcol;
  return os;
}


/////////////////////////////////////////////////////////////////////////////
//
// CFourBoxesSameIF classes implementation
//
/////////////////////////////////////////////////////////////////////////////

CFourBoxesSameIF::CFourBoxesSameIF(int templateWidth, int templateHeight)
  : CFourBoxesIF(templateWidth, templateHeight) 
{
}

CFourBoxesSameIF::CFourBoxesSameIF(int templateWidth, int templateHeight,
             const CRect* pB1, const CRect* pB2,
             const CRect* pB3, const CRect* pB4)
  : CFourBoxesIF(templateWidth, templateHeight) 
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

CFourBoxesSameIF::CFourBoxesSameIF(const CFourBoxesSameIF& frm)
  : CFourBoxesIF(frm)
{
}

CFourBoxesSameIF::CFourBoxesSameIF(istream& is, int template_width, int template_height)
  : CFourBoxesIF(template_width, template_height)
{
  char a, b, c, d, e, f;
  is >> e >> b1_left >> a >> b1_top >> b >> b1_right >> c >> b1_bottom >> f >> d;
  if (a!=',' || b!=',' || c!=',' || d!=',' || e!='(' || f!=')') {
    char* buf = (char*) alloca(256*sizeof(char));
    sprintf(buf, "error during construction of FourBoxesSameIF (%c|%c|%c|%c|%c)\n",
	   e, a, b, c, f);
    throw ITException(buf);
  }
  is >> e >> b2_left >> a >> b2_top >> b >> b2_right >> c >> b2_bottom >> f >> d;
  if (a!=',' || b!=',' || c!=',' || d!=',' || e!='(' || f!=')') {
    char* buf = (char*) alloca(256*sizeof(char));
    sprintf(buf, "error during construction of FourBoxesSameIF (%c|%c|%c|%c|%c)\n",
	   e, a, b, c, f);
    throw ITException(buf);
  }
  is >> e >> b3_left >> a >> b3_top >> b >> b3_right >> c >> b3_bottom >> f >> d;
  if (a!=',' || b!=',' || c!=',' || d!=',' || e!='(' || f!=')') {
    char* buf = (char*) alloca(256*sizeof(char));
    sprintf(buf, "error during construction of FourBoxesSameIF (%c|%c|%c|%c|%c)\n",
	   e, a, b, c, f);
    throw ITException(buf);
  }
  is >> e >> b4_left >> a >> b4_top >> b >> b4_right >> c >> b4_bottom >> f;
  if (a!=',' || b!=',' || c!=',' || e!='(' || f!=')') {
    char* buf = (char*) alloca(256*sizeof(char));
    sprintf(buf, "error during construction of FourBoxesSameIF (%c|%c|%c|%c|%c)\n",
	   e, a, b, c, f);
    throw ITException(buf);
  }
  SetNonOverlap();
}

void CFourBoxesSameIF::SetToFirstIncarnation()
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
    b4_left  = 1;  b4_top    = 0;
    b4_right = 3;  b4_bottom = 2;
  }
  SetNonOverlap();
}

bool CFourBoxesSameIF::SetToNextIncarnation()
{
	if (m_is_partial) {
		if (m_remaining_incarnations) {
			m_remaining_incarnations--;
		} else {
			return false;
		}
	}
        
  int width = b1_right-b1_left;
  b2_right++;
  b2_left++;
  b3_right++;
  b3_left++;
  if (b3_right>=m_template_width-2 || b3_left>=b1_right) {
    b1_right++;
    b1_left++;
    b4_right++;
    b4_left++;
    if (b1_right>=m_template_width-3) {
      int height = b1_bottom-b1_top;
      b3_bottom++;
      b3_top++;
      b4_bottom++;
      b4_top++;
      if (b3_bottom>=m_template_height-3) {
        b1_bottom++;
        b1_top++;
        b2_bottom++;
        b2_top++;
        if (b1_bottom>=m_template_height-2 || b1_top>=b3_bottom) {
          width++;
          if (width>m_template_width-3) {
            height++;
            if (height>m_template_height-2) {
              return false;
            }
            width = 2;
          }
          b1_top = 0; b1_bottom = b1_top+height;
          b2_top = 1; b2_bottom = b2_top+height;
        }
        b3_top =-1; b3_bottom = b3_top+height;
        b4_top = 1; b4_bottom = b4_top+height;
      }
      b1_left =-1; b1_right = b1_left+width;
      b4_left = 1; b4_right = b4_left+width;
    }
    b3_left = 0; b3_right = b3_left+width;
    b2_left = 0; b2_right = b2_left+width;
  }
  SetNonOverlap();
	return true;
}

CIntegralFeature* CFourBoxesSameIF::Copy() const
{
	return new CFourBoxesSameIF(*this);
}

ostream& CFourBoxesSameIF::output(ostream& os) const
{
  os << "FourBoxesSame " 
     << "(" << b1_left << "," << b1_top << "," << b1_right << "," << b1_bottom << "),"
     << "(" << b2_left << "," << b2_top << "," << b2_right << "," << b2_bottom << "),"
     << "(" << b3_left << "," << b3_top << "," << b3_right << "," << b3_bottom << "),"
     << "(" << b4_left << "," << b4_top << "," << b4_right << "," << b4_bottom << ")";
  return os;
}



