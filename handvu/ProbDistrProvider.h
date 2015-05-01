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
  * $Id: ProbDistrProvider.h,v 1.5 2004/11/11 01:58:12 matz Exp $
**/

#if !defined(__PROB_DISTR_PROVIDER_H__INCLUDED_)
#define __PROB_DISTR_PROVIDER_H__INCLUDED_


#include <cv.h>
#include "Rect.h"

class ProbDistrProvider {
 public:
  virtual double LookupProb(ColorBGR color) const = 0;
  // map must be IPL_DEPTH_8U
  virtual void CreateMap(const IplImage* rgbImage, IplImage* map, const CRect& roi) const = 0;
};

#endif //__PROB_DISTR_PROVIDER_H__INCLUDED_
