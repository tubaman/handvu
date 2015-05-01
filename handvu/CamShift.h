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
  * $Id: CamShift.h,v 1.6 2004/11/11 01:58:12 matz Exp $
**/

#if !defined(___HV_CAMSHIFT_INCLUDED__)
#define ___HV_CAMSHIFT_INCLUDED__

#include <cv.h>
#include <cvaux.h>
#include "Rect.h"
#include "ProbDistrProvider.h"

class CamShift {
public:
  CamShift();

public:
  void Initialize(int width, int height);
  void PrepareTracking(IplImage* image, const ProbDistrProvider* pProbProv, const CRect& area);
  void Track(IplImage* image);
  void DrawOverlay(IplImage* image, int overlay_level) const;
  void GetArea(CRect& area) const;

protected:
  void DoHSV(IplImage* image, bool initialize);
  void DoExternal(IplImage* image);
  void CheckBackProject(IplImage* image, int view) const;
  void DrawCross(IplImage* image) const;

protected:
  bool                      m_tracking;
  int                       m_img_width, m_img_height;

  // for external probability image
  const ProbDistrProvider*  m_pProbProv;
  IplImage*                 m_prob_map;
  CvBox2D                   m_box;
  CvConnectedComp           m_comp;

  // for internal probability map based on HSV histogram
  CvCamShiftTracker         m_cCamShift;
  CvRect                    m_object;

  // HSV histogram parameters
  int                       m_Smin, m_Vmin, m_Vmax;
  int                       m_threshold;
  int                       m_bins;
};


#endif // ___HV_CAMSHIFT_INCLUDED__
