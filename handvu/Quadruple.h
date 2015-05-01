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
  * $Id: Quadruple.h,v 1.4 2004/10/15 20:03:24 matz Exp $
**/

#ifndef __CQUADRUPLE_H__
#define __CQUADRUPLE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "Rect.h"


class CQuadruple {
public:
  CQuadruple() 
    : left(-1), top(-1), right(-1), bottom(-1) {};
  CQuadruple(double _left, double _top, double _right, double _bottom) 
    : left(_left), top(_top), right(_right), bottom(_bottom) {};
  CRect toRect(double scale_x, double scale_y) const;
  void fromRect(const CRect& rect, double scale_x, double scale_y);
  double left, top, right, bottom;
};

typedef vector<CQuadruple> CQuadrupleVector;


#endif // __CQUADRUPLE_H__



