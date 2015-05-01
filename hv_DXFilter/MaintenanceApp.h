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
  * $Id: MaintenanceApp.h,v 1.7 2005/10/30 23:00:43 matz Exp $
**/

#ifndef __MAINTENANCE_APP_H__INCLUDED__
#define __MAINTENANCE_APP_H__INCLUDED__

#include <sys/timeb.h>
#include "ImageOverlay.h"
#include "IconGroup.h"


class CMaintenanceApp : public CImageOverlay {
 public:
  CMaintenanceApp();
  ~CMaintenanceApp();

 public:
  virtual void Initialize(int img_width, int img_height);
  virtual void Tracked(const COverlayEvent& event);
  virtual void Draw(IplImage* out_img);
  virtual void DontDraw(IplImage* /*out_img*/) {};


protected:
   enum IconGroupID {
     IG_VOICE_RECORDER = 0,
     IG_IMG_VID_CAP = 1,
     IG_IMG_DELAY = 2,
     IG_IMG_AREA = 3,
     IG_VID_CAP = 4,
     IG_WO_SELECT = 5,
     IG_WO_DETAIL = 6,
     IG_WO_ATTACH = 7,
     IG_NUM_SELECT = 8,
     IG_HAND_POINTER = 9,
     IG_HAND_FLAT = 10,
     IG_LEN = 11
   };

  void BuildIconGroups();
  CRect AbsoluteArea(double left, double top, double right, double bottom);
  void PointerOver(int* pX, int* pY);
  void Select(int x, int y);
  void SwitchApplications();
  void DimEverythingBut(IplImage* pImage, const CRect& area);
  void HideAllGroups();
  void ShowGroupExclusive(IconGroupID);

 protected:
   int                  m_img_width;
   int                  m_img_height;
   CIconGroupVector     m_icon_groups;
   CStatusVector        m_group_shown;
   COverlayEvent        m_event;

   // application variables
   int                  m_active_app;
   string               m_last_ptr1_type;
   bool                 m_last_ptr1_recognized;
   Time                 m_time_last_app_switch;
#define MIN_TIME_BTW_APP_SWITCH 1000
#define NUM_APPS 5
   double               m_voicerec_default_x;
   double               m_voicerec_ctr_y;
   double               m_imgcap_ctr_x;
   double               m_imgcap_default_y;
   double               m_woselect_ctr_x;
   bool                 m_show_hand_location;
   int                  m_switchpos_x;
   int                  m_switchpos_y;
};


#endif // __MAINTENANCE_APP_H__INCLUDED__
