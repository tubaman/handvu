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
  * $Id: Mask.h,v 1.5 2004/11/11 01:58:12 matz Exp $
**/

#ifndef __HANDVU_MASK_H_INCLUDED__
#define __HANDVU_MASK_H_INCLUDED__

#include <vector>
#include <string>
#include <map>
using namespace std;

typedef vector<double> CDoubleVector;


class Mask {
public:
  Mask();
  ~Mask();

  void ParseFrom(string filename);
  const string& GetName() const { return m_name; }
  double GetProb(int x, int y) const;
  int GetWidth() const {return m_width;}
  int GetHeight() const {return m_height;}
  double GetImageAreaRatio() const { return m_image_area_ratio; }
  void SetImageAreaRatio(double r) { m_image_area_ratio = r; }

protected:
  int               m_width, m_height;
  double            m_image_area_ratio;
  string            m_name;
  CDoubleVector     m_probs;
};

typedef map<string, Mask> MaskMap;
typedef MaskMap::const_iterator ConstMaskIt;

class MaskScaler {
public:
  MaskScaler();
  ~MaskScaler();

public:
  ConstMaskIt    m_mask;
  double         m_scale_x;
  double         m_scale_y;
};



#endif // __HANDVU_MASK_H_INCLUDED__
