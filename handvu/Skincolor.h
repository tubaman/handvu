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
  * $Id: Skincolor.h,v 1.6 2004/11/11 01:58:12 matz Exp $
**/

#if !defined(__SKINCOLOR_H__INCLUDED_)
#define __SKINCOLOR__INCLUDED_

#include <cv.h>
#include "Mask.h"
#include "Rect.h"


#pragma warning (disable:4786)
class Skincolor {
 public:
  Skincolor();
  ~Skincolor();

 public:
  void Initialize(int width, int height);
  double GetCoverage(IplImage* rgbImage, const CRect& roi,
                     ConstMaskIt mask, bool backproject);
  void DrawOverlay(IplImage* rgbImage, int overlay_level, 
                   const CRect& roi);
  
 protected:
  bool                   m_draw_once;
  double                 m_last_coverage;
  ConstMaskIt            m_last_mask;
};


#pragma warning (default:4786)



#endif // __SKINCOLOR_H__INCLUDED_

