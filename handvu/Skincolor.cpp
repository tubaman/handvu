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
  * $Id: Skincolor.cpp,v 1.6 2004/10/15 20:03:24 matz Exp $
**/

// fixed segmentation

#include "Common.h"
#include "Skincolor.h"
#include "skinrgb.h"

//
// Constructor
//
Skincolor::Skincolor()
  : m_draw_once(false),
    m_last_coverage(-1)
{
} // (Constructor)


Skincolor::~Skincolor()
{
}

void Skincolor::Initialize(int /*width*/, int /*height*/)
{
}

#pragma warning (disable:4786)
/* return the coverage with skin pixels, given the fixed lookup table
 */
double Skincolor::GetCoverage(IplImage *image, const CRect& roi,
                                   ConstMaskIt mask, 
                                   bool backproject)
{
  ColorBGR black; 
  black.red = black.green = black.blue = 0;

  const double scp_thresh = 0.7;  // skin color probability
  int skin = 0;
  int noskin = 0;
  int start_x = max(0, roi.left);
  int start_y = max(0, roi.top);
  int stop_x = min(image->width, roi.right);
  int stop_y = min(image->height, roi.bottom);
  double scale_x = (double)(stop_x-start_x-1)/((*mask).second.GetWidth()-1.);
  double scale_y = (double)(stop_y-start_y-1)/((*mask).second.GetHeight()-1.);
  for (int y=start_y; y<stop_y; y++) {
    ColorBGR* prgb = (ColorBGR*) image->imageData;
//    RGBTRIPLE* prgb = (RGBTRIPLE*) image->imageData;
    prgb += y*image->width + start_x;
    for (int x=start_x; x<stop_x; x++) {
      int m_x = cvRound((double)(x-start_x)/scale_x);
      int m_y = cvRound((double)(y-start_y)/scale_y);
      double scp = (*mask).second.GetProb(m_x, m_y);
      if (scp>=scp_thresh) {
        if (IsSkin_RGB(*prgb)) {
          skin++;
        } else {
          noskin++;
          if (backproject) {
            *prgb = black; 
          }
        }
      }
      prgb++;
    }
  }
  { // for drawing output only:
    m_draw_once = true;
    m_last_mask = mask;
  }
  m_last_coverage = (double)skin/(double)(skin+noskin);
  return m_last_coverage;
}

#pragma warning (default:4786)

void Skincolor::DrawOverlay(IplImage* rgbImage, int overlay_level, 
                            const CRect& roi)
{
  if (m_draw_once) {

    if (overlay_level>=3) {
      GetCoverage(rgbImage, roi, m_last_mask, true);
    }

    if (overlay_level>=2) {
      // draw coverage bar
      int img_height = rgbImage->height;
      int img_width = rgbImage->width;
      double height = img_height/2;
      int xpos = img_width-25;
      int width = 20;
      int space = 5;

      cvRectangle(rgbImage, cvPoint(xpos, img_height),
                  cvPoint(xpos+width, img_height-(int)height),
                  CV_RGB(255,255,0), 1);
      cvRectangle(rgbImage, cvPoint(xpos, img_height),
                  cvPoint(xpos+width, img_height-(int)(height*m_last_coverage)),
                  CV_RGB(255,255,0), CV_FILLED);
      xpos -= width+space;
    }

    m_draw_once = false;
  }
}

