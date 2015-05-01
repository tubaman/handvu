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
  * $Id: ImageOverlay.h,v 1.4 2005/10/30 23:00:43 matz Exp $
**/

#ifndef __IMAGE_OVERLAY_H__INCLUDED__
#define __IMAGE_OVERLAY_H__INCLUDED__

class COverlayEvent {
public:
  COverlayEvent() : m_ptr1_tracked(false), m_ptr2_tracked(false) {}

public:
  double           m_ptr1_x, m_ptr1_y;
  double           m_ptr2_x, m_ptr2_y;
  bool             m_ptr1_tracked, m_ptr2_tracked;
  bool             m_ptr1_recognized, m_ptr2_recognized;
  string           m_ptr1_type, m_ptr2_type;
};

class CImageOverlay {
 public:
  virtual void Initialize(int img_width, int img_height) = 0;
  virtual void Tracked(const COverlayEvent& event) = 0;
  virtual void Draw(IplImage* out_img) = 0;
  virtual void DontDraw(IplImage* out_img) = 0;
};




#endif // __IMAGE_OVERLAY_H__INCLUDED__
