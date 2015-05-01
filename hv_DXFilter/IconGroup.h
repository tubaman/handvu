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
  * $Id: IconGroup.h,v 1.5 2004/11/24 08:38:40 matz Exp $
**/

#ifndef __ICON_GROUP_H__INCLUDED__
#define __ICON_GROUP_H__INCLUDED__

#include <CV.h>
#include <vector>
#include <sys/timeb.h>
using namespace std;

typedef vector<IplImage*> CImageVector;

typedef vector<bool> CStatusVector;
typedef vector<CRect> CRectVector;



class CIconGroup {
public: 
  CIconGroup();
  ~CIconGroup();

//  void AddOverlay(string filename, CRect& area);
  void AddOverlay(string filename, CRect& area, bool highlite);
  void PointerOver(int x, int y);
  bool Select(int x, int y);
  bool Intersect(const CRect& area, int x, int y);
  void ResetSelection();

public:
   CImageVector         m_images;
   CRectVector          m_areas;
   CStatusVector        m_display_icon;
   CStatusVector        m_highlight_area;
   CStatusVector        m_highlight_active;
   CStatusVector        m_is_selected;
};


typedef vector<CIconGroup> CIconGroupVector;
typedef struct _timeb Time;



#endif // __ICON_GROUP_H__INCLUDED__
