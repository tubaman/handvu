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
  * $Id: Rect.h,v 1.4 2004/11/11 01:58:12 matz Exp $
**/

#ifndef __CRECT__INCLUDED_H_
#define __CRECT__INCLUDED_H_

#ifndef __ATLTYPES_H__

#include <cubicles.h>


// ----------------------------------------------------------------------
// class CRect
// ----------------------------------------------------------------------

class CRect {
 public:
  CRect() : left(-1), top(-1), right(-1), bottom(-1) {};
  CRect(int _left, int _top, int _right, int _bottom) :
    left(_left), top(_top), right(_right), bottom(_bottom) {};

  CRect(const CuScanMatch& m) :
    left(m.left), top(m.top), right(m.right), bottom(m.bottom) {};

 public:
  int left, top, right, bottom;
};

#endif // __ATLTYPES_H__

#endif // __CRECT__INCLUDED_H_


