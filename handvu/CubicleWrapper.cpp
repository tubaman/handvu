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
  * $Id: CubicleWrapper.cpp,v 1.13 2005/10/30 21:16:03 matz Exp $
**/

#include "Common.h"
#include "CubicleWrapper.h"
#include "HandVu.hpp"


//
// Constructor
//
CubicleWrapper::CubicleWrapper()
{
  cvInitFont( &m_cvFont, CV_FONT_VECTOR0, 0.5f /* hscale */, 
    0.5f /* vscale */, 0.1f /*italic_scale */, 
    1 /* thickness */);
  m_bbox = CRect(-1, -1, -1, -1);
}

CubicleWrapper::~CubicleWrapper()
{
}

void CubicleWrapper::Initialize(int width, int height)
{
  cuInitialize(width, height);
}

void CubicleWrapper::Process(IplImage* grayImage)
{
  cuScan(grayImage, m_matches);
  cuGetScannedArea(&m_bbox.left, &m_bbox.top, &m_bbox.right, &m_bbox.bottom);
  cuGetScaleSizes(&m_min_width, &m_max_width, &m_min_height, &m_max_height);
} // Process



void CubicleWrapper::DrawOverlay(IplImage* iplImage, int overlay_level) const
{
  if (overlay_level>=1) {
    DrawMatches(iplImage, overlay_level);
  }
}

void CubicleWrapper::DrawMatches(IplImage* iplImage, int overlay_level) const
{
  for (CuScanMatchVector::const_iterator it
         = m_matches.begin();
       it!=m_matches.end();
       it++)
  {
    cvLine(iplImage, cvPoint(it->left, it->top),
           cvPoint(it->right, it->top), CV_RGB(0, 255, 0));
    cvLine(iplImage, cvPoint(it->right, it->top),
           cvPoint(it->right, it->bottom), CV_RGB(0, 255, 0));
    cvLine(iplImage, cvPoint(it->left, it->bottom),
           cvPoint(it->right, it->bottom), CV_RGB(0, 255, 0));
    cvLine(iplImage, cvPoint(it->left, it->top),
           cvPoint(it->left, it->bottom), CV_RGB(0, 255, 0));
  }
  
  if (m_bbox.left>-1 && overlay_level>=2) {
    cvLine(iplImage, cvPoint(m_bbox.left, m_bbox.top),
           cvPoint(m_bbox.right, m_bbox.top), CV_RGB(255, 255, 255));
    cvLine(iplImage, cvPoint(m_bbox.right, m_bbox.top),
           cvPoint(m_bbox.right, m_bbox.bottom), CV_RGB(255, 255, 255));
    cvLine(iplImage, cvPoint(m_bbox.left, m_bbox.bottom),
           cvPoint(m_bbox.right, m_bbox.bottom), CV_RGB(255, 255, 255));
    cvLine(iplImage, cvPoint(m_bbox.left, m_bbox.top),
           cvPoint(m_bbox.left, m_bbox.bottom), CV_RGB(255, 255, 255));
  }

  if (m_bbox.left>-1 && overlay_level>=3) {
    int top;
    int left;
    // min scan size
    top = max(m_bbox.top, m_bbox.bottom-m_min_height);
    left = max(m_bbox.left, m_bbox.right-m_min_width);
    cvLine(iplImage, 
      cvPoint(m_bbox.right, top),
      cvPoint(left, top), 
      CV_RGB(0, 0, 255));
    cvLine(iplImage, 
      cvPoint(left, top),
      cvPoint(left, m_bbox.bottom), 
      CV_RGB(0, 0, 255));

    // max scan size
    top = max(m_bbox.top, m_bbox.bottom-m_max_height);
    left = max(m_bbox.left, m_bbox.right-m_max_width);
    cvLine(iplImage, 
      cvPoint(m_bbox.right, top),
      cvPoint(left, top), 
      CV_RGB(0, 0, 255));
    cvLine(iplImage, 
      cvPoint(left, top),
      cvPoint(left, m_bbox.bottom), 
      CV_RGB(0, 0, 255));
  }

}

CuScanMatch CubicleWrapper::GetBestMatch()
{
  int num_matches = (int)m_matches.size();
  ASSERT(num_matches);
  if (num_matches==1) {
    return m_matches[0];
  }
  // pick the smallest height for now
  int min_height = INT_MAX;
  int best_indx = -1;
  for (int mc=0; mc<num_matches; mc++) {
    int height = m_matches[mc].bottom-m_matches[mc].top;
    if (height<min_height) {
      min_height = height;
      best_indx = mc;
    }
  }
  ASSERT(best_indx!=-1);
  return m_matches[best_indx];
}

