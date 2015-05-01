/**
  * HandVu - a library for computer vision-based hand gesture
  * recognition.
  * Copyright (C) 2004 Mathias Kolsch, matz@cs.ucsb.edu
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place - Suite 330, 
  * Boston, MA  02111-1307, USA.
  *
  * $Id: CubicleWrapper.h,v 1.10 2004/11/11 01:58:12 matz Exp $
**/

#if !defined(__CUBICLE_H__INCLUDED_)
#define __CUBICLE_H__INCLUDED_

#include <cv.h>
#include <vector>
using namespace std;

#include "cubicles.h"
#include "Rect.h"


typedef vector<char> CStatusVector;


class CubicleWrapper {
 public:
  CubicleWrapper();
  ~CubicleWrapper();

  void Initialize(int width, int height);
  void Process(IplImage* grayImage);
  void DrawOverlay(IplImage* iplImage, int overlay_level) const;
  void DrawMatches(IplImage* iplImage, int overlay_level) const;
  CuScanMatch GetBestMatch();
  bool GotMatches() const { return m_matches.size()>0; }

 protected:
  CRect                     m_bbox;
  int                       m_min_width, m_max_width;
  int                       m_min_height, m_max_height;
  mutable CvFont            m_cvFont;
  CuScanMatchVector         m_matches;
};





#endif // __CUBICLE_H__INCLUDED_

