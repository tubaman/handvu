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
  * $Id: MaintenanceApp.cpp,v 1.12 2005/10/30 23:00:43 matz Exp $
**/

#include "StdAfx.h"
#include <initguid.h>
#include "Common.h"
#include "iHandVuFilter.h"
#include "MaintenanceApp.h"
#include "HandVu.h"

CMaintenanceApp::CMaintenanceApp()
: m_img_width(0),
  m_img_height(0),
  m_active_app(0),
  m_last_ptr1_type(""),
  m_last_ptr1_recognized(false),
  m_voicerec_default_x(0),
  m_voicerec_ctr_y(0),
  m_imgcap_ctr_x(0),
  m_imgcap_default_y(0),
  m_woselect_ctr_x(0),
  m_show_hand_location(true),
  m_switchpos_x(0),
  m_switchpos_y(0)
{
  m_time_last_app_switch.time = 0;
  m_time_last_app_switch.millitm = 0;
}

CMaintenanceApp::~CMaintenanceApp()
{
}

void CMaintenanceApp::Initialize(int img_width, int img_height)
{
  if (img_height<0) {
    throw HVException("shouldn't be upside down anymore at this point");
  }
  m_img_width = img_width;
  m_img_height = img_height;
  BuildIconGroups();
}

/** gesture event handler - select a button, switch applications,
* draw the right hand pointer or detection aid
*/
void CMaintenanceApp::Tracked(const COverlayEvent& event)
{
  ASSERT(!event.m_ptr1_tracked
    || (0.0<=event.m_ptr1_x && event.m_ptr1_x<1.0
        && 0.0<=event.m_ptr1_y && event.m_ptr1_y<1.0));
  m_event = event;
  if (event.m_ptr1_tracked) {
    m_group_shown[IG_HAND_FLAT] = false;
  } else {
    // not detected 
    ShowGroupExclusive(IG_HAND_FLAT);
    return;
  }
  int x = cvRound(m_event.m_ptr1_x*(m_img_width-1));
  int y = cvRound(m_event.m_ptr1_y*(m_img_height-1));
  int orig_x = x, orig_y = y;
  if (m_event.m_ptr1_tracked) {
    PointerOver(&x, &y);
  }

  // reset selection
  for (int grpcnt=0; grpcnt<(int)m_icon_groups.size(); grpcnt++) {
    if (m_group_shown[grpcnt]) {
      m_icon_groups[grpcnt].ResetSelection();
    }
  }

  if (m_event.m_ptr1_recognized) {
    if (m_event.m_ptr1_type=="Lback") {
      Select(x, y);
    } else if (m_event.m_ptr1_type=="victory" || m_event.m_ptr1_type=="open") {
      // how long ago was the last switch?
      Time currtime;
      _ftime(&currtime);
      unsigned int d_t = 1000*(unsigned int)(currtime.time-m_time_last_app_switch.time);
      d_t += (currtime.millitm-m_time_last_app_switch.millitm);
      // or was the last recognized gesture something else but task switch?
      if (d_t>MIN_TIME_BTW_APP_SWITCH
          || (m_last_ptr1_type!=m_event.m_ptr1_type 
              && m_last_ptr1_recognized))
      {
        SwitchApplications();
        m_switchpos_x = orig_x;
        m_switchpos_y = orig_y;
        m_time_last_app_switch = currtime;
      }
    } else if (m_event.m_ptr1_type=="sidepoint") {
    }

    m_last_ptr1_type = m_event.m_ptr1_type;
  }

  m_last_ptr1_recognized = m_event.m_ptr1_recognized;
}
  
void CMaintenanceApp::PointerOver(int* pX, int* pY)
{
  const double scale = 2.0;
  const CRect& old_hand = m_icon_groups[IG_HAND_POINTER].m_areas[0];
  int width = old_hand.right-old_hand.left;
  int height = old_hand.bottom-old_hand.top;
  int halfwidth = width/2;
  int halfheight = height/2;
  switch (m_active_app) {
    case 0: // no app
      break;
    case 1: // voice recorder
      // constrain to movements in horizontal, scale horizontal
      *pX = (int)(m_img_width*m_voicerec_default_x)+(int)(scale*(double)(*pX-m_switchpos_x));
      *pY = (int)(m_img_height*m_voicerec_ctr_y)+halfheight; 
      break;
    case 2: // img/vid cap
      // constrain to movements in vertical, scale vertical
      *pX = (int)(m_img_width*m_imgcap_ctr_x);
      *pY = (int)((double)m_img_height*m_imgcap_default_y + scale*(double)(*pY-m_switchpos_y));
      break;
    case 3: // WO select
      // for now: constrain to movements in vertical
      *pX = (int)(m_img_width*m_woselect_ctr_x);
      break;
    case 4: // number select
      break;
    case 5: // 
      break;
    default: 
      ASSERT(0);
  }  
  for (int grpcnt=0; grpcnt<(int)m_icon_groups.size(); grpcnt++) {
    if (!m_group_shown[grpcnt]) {
      continue;
    }
    m_icon_groups[grpcnt].PointerOver(*pX, *pY);
  }
  int area_left = *pX-halfwidth;
  int area_top = *pY-halfheight;
  area_left = max(0, area_left);
  area_top = max(0, area_top);
  area_left = min(m_img_width-width-1, area_left);
  area_top = min(m_img_height-height-1, area_top);
  ASSERT(0<=area_left && area_left+width<m_img_width);
  ASSERT(0<=area_top && area_top+height<m_img_height);
  m_icon_groups[IG_HAND_POINTER].m_areas[0] =
    CRect(area_left, area_top, area_left+width, area_top+height);
}

void CMaintenanceApp::Select(int x, int y)
{
  bool got_one = false;
  for (int grpcnt=0; grpcnt<(int)m_icon_groups.size(); grpcnt++) {
    if (!m_group_shown[grpcnt]) {
      continue;
    }
    bool this_one = m_icon_groups[grpcnt].Select(x, y);
    if (this_one) got_one = true;
  }
  if (got_one) {
    // Beep(800, 70);
  }
}

void CMaintenanceApp::SwitchApplications()
{
  m_active_app++;
  if (m_active_app>NUM_APPS) {
    m_active_app = 0;
  }
  switch (m_active_app) {
    case 0: HideAllGroups(); break;
    case 1: ShowGroupExclusive(IG_VOICE_RECORDER); break;
    case 2: ShowGroupExclusive(IG_IMG_VID_CAP); break;
    case 3: ShowGroupExclusive(IG_WO_SELECT); break;
    case 4: ShowGroupExclusive(IG_NUM_SELECT); break;
    case 5: HideAllGroups(); break;
    default: ASSERT(0);
  }

  if (m_active_app==1 || m_active_app==2 || m_active_app==4) {
    m_group_shown[IG_HAND_POINTER] = true;
  } else {
    m_group_shown[IG_HAND_POINTER] = false;
  }
  m_show_hand_location = true;
  if (m_active_app==4) {
    m_show_hand_location = false;
  }

  if (m_active_app==5) {
    hvStartRecognition(1);
  } else if (m_active_app==0) {
    hvStopRecognition(1);
  }
}

void CMaintenanceApp::Draw(IplImage* out_img)
{
  for (int grpcnt=0; grpcnt<(int)m_icon_groups.size(); grpcnt++) {
    if (!m_group_shown[grpcnt]) {
      continue;
    }
    for (int imgcnt=0; imgcnt<(int)m_icon_groups[grpcnt].m_images.size(); imgcnt++) {
      if (!m_icon_groups[grpcnt].m_display_icon[imgcnt]) {
        continue;
      }
      const CRect& area = m_icon_groups[grpcnt].m_areas[imgcnt];
      IplImage* img = m_icon_groups[grpcnt].m_images[imgcnt];
      ASSERT(img);

      bool brighten = m_icon_groups[grpcnt].m_is_selected[imgcnt];

      // left upper corner is origin
      for (int y=area.top; y<area.bottom; y++) {
        RGBTRIPLE* pDst = (RGBTRIPLE*) out_img->imageData;
        pDst += y*out_img->width + area.left;
        RGBTRIPLE* pSrc = (RGBTRIPLE*) cvPtr2D(img, y-area.top, 0);
        for (int x=area.left; x<area.right; x++) {
          BYTE r, b, g;
          r = pSrc->rgbtRed; 
          g = pSrc->rgbtGreen; 
          b = pSrc->rgbtBlue; 
          if (r>0 || g>0 || b>0) {
            if (brighten) {
              pDst->rgbtRed = min(255, r*2);
              pDst->rgbtGreen = min(255, g*2);
              pDst->rgbtBlue = min(255, b*2);
            } else {
              pDst->rgbtRed = r;
              pDst->rgbtGreen = g;
              pDst->rgbtBlue = b;
            }

          }
          pSrc++;
          pDst++;
        }
      }

      if (m_icon_groups[grpcnt].m_highlight_active[imgcnt]
          && !(grpcnt==IG_WO_SELECT)) { // hack
        cvRectangle (out_img, cvPoint(area.left+3, area.top+3),
          cvPoint(area.right-4, area.bottom-4), CV_RGB(255,0,0), 5);
      }
      // hack:
      if (grpcnt==IG_WO_SELECT && imgcnt==1) {
        cvRectangle (out_img, cvPoint(area.left+3, area.top+3),
          cvPoint(area.right-4, area.bottom-4), CV_RGB(255,0,0), 5);
      }
    }
  }

  int x = cvRound(m_event.m_ptr1_x*(m_img_width-1));
  int y = cvRound(m_event.m_ptr1_y*(m_img_height-1));

  // second pointer/hand 
  if (m_active_app==5) {
    hvState state;
    hvGetState(1, state);
    if (state.tracked) {
      int green_pos_x = cvRound(state.center_xpos*(m_img_width-1));
      int green_pos_y = cvRound(state.center_ypos*(m_img_height-1));
      CRect area(min(x, green_pos_x), min(y, green_pos_y), max(x, green_pos_x), max(y, green_pos_y));
      DimEverythingBut(out_img, area);
      cvRectangle(out_img, cvPoint(area.left, area.top),
        cvPoint(area.right, area.bottom), CV_RGB(255,255,0), 3);
      cvCircle(out_img, cvPoint(green_pos_x, green_pos_y), 10, CV_RGB(0,255,0), CV_FILLED);
    }
  }

  if (m_show_hand_location && m_event.m_ptr1_tracked) {
    cvCircle(out_img, cvPoint(x, y), 10, CV_RGB(255,0,0), CV_FILLED);
  }
}

void CMaintenanceApp::DimEverythingBut(IplImage* pImage, const CRect& area)
{
  const double dimfac = .5;
  for (int y=0; y<pImage->height; y++) {
    RGBTRIPLE* pDst = (RGBTRIPLE*) pImage->imageData;
    pDst += y*pImage->width;
    for (int x=0; x<pImage->width; x++, pDst++) {
      if (area.top<=y && y<area.bottom && area.left<=x && x<area.right) {
        continue;
      }
      pDst->rgbtRed = (BYTE)((double)pDst->rgbtRed*dimfac);
      pDst->rgbtGreen = (BYTE)((double)pDst->rgbtGreen*dimfac);
      pDst->rgbtBlue = (BYTE)((double)pDst->rgbtBlue*dimfac);
    }
  }
}

CRect CMaintenanceApp::AbsoluteArea(double left, double top, 
                                  double right, double bottom)
{
  CRect abs_area;
  abs_area = CRect((int)(m_img_width*left),
    (int)(m_img_height*top),
    (int)(m_img_width*right),
    (int)(m_img_height*bottom));
  ASSERT(0<=abs_area.left && abs_area.left<=m_img_width);
  ASSERT(0<=abs_area.top && abs_area.top<=m_img_height);
  ASSERT(0<=abs_area.right && abs_area.right<=m_img_width);
  ASSERT(0<=abs_area.bottom && abs_area.bottom<=m_img_height);
  ASSERT(abs_area.left<abs_area.right);
  ASSERT(abs_area.top<abs_area.bottom);
  return abs_area;
}

void CMaintenanceApp::HideAllGroups()
{
  for (int g=0; g<(int)m_group_shown.size(); g++) {
    m_group_shown[g] = false;
  }
}

void CMaintenanceApp::ShowGroupExclusive(IconGroupID id)
{
  HideAllGroups();
  m_group_shown[id] = true;
}

void CMaintenanceApp::BuildIconGroups()
{
  DbgLog((LOG_CUSTOM1, 4, "loading icons..."));
  m_icon_groups.resize(IG_LEN);
  m_group_shown.resize(IG_LEN);

  // voice recorder group
  double vr_left = .05;
  double width = .28;
  double hor_gap = .02;
  m_voicerec_ctr_y = .11;
  m_voicerec_default_x = .5;
  double height = .18;
  double vr_top = m_voicerec_ctr_y-height/2.0;
  double vr_bottom = vr_top+height;
  CRect abs_area;
  abs_area = AbsoluteArea(vr_left, vr_top, vr_left+width, vr_bottom);
  m_icon_groups[IG_VOICE_RECORDER].AddOverlay("$IT_DATA/Icons/record.bmp", abs_area, true);
//  m_icon_groups[IG_VOICE_RECORDER].AddOverlay("$IT_DATA/Icons/play.bmp", abs_area, true);
  vr_left += width+hor_gap;
  abs_area = AbsoluteArea(vr_left, vr_top, vr_left+width, vr_bottom);
  m_icon_groups[IG_VOICE_RECORDER].AddOverlay("$IT_DATA/Icons/pause.bmp", abs_area, true);
  vr_left += width+hor_gap;
  abs_area = AbsoluteArea(vr_left, vr_top, vr_left+width, vr_bottom);
  m_icon_groups[IG_VOICE_RECORDER].AddOverlay("$IT_DATA/Icons/stop.bmp", abs_area, true);
  m_group_shown[IG_VOICE_RECORDER] = false;

  // image/video capture menu group
  m_imgcap_ctr_x = .2;
  m_imgcap_default_y = .5;
  width = .3;
  vr_left = m_imgcap_ctr_x-width/2.0;
  double vr_right = vr_left+width;
  double ver_gap = .02;
  vr_top = .15;
  height = .23;
  abs_area = AbsoluteArea(vr_left, vr_top, vr_right, vr_top+height);
  m_icon_groups[IG_IMG_VID_CAP].AddOverlay("$IT_DATA/Icons/snapshot.bmp", abs_area, true);
  vr_top += height+ver_gap;
  abs_area = AbsoluteArea(vr_left, vr_top, vr_right, vr_top+height);
  m_icon_groups[IG_IMG_VID_CAP].AddOverlay("$IT_DATA/Icons/frame.bmp", abs_area, true);
  vr_top += height+ver_gap;
  abs_area = AbsoluteArea(vr_left, vr_top, vr_right, vr_top+height);
  m_icon_groups[IG_IMG_VID_CAP].AddOverlay("$IT_DATA/Icons/liveVideo.bmp", abs_area, true);
  m_group_shown[IG_IMG_VID_CAP] = false;

  // image area snapshot
  /*
  abs_area = AbsoluteArea(vr_left, vr_top, vr_right, vr_bottom);
  m_icon_groups[IG_IMG_AREA].AddOverlay("$IT_DATA/Icons/frame.bmp", abs_area, true);
  abs_area = AbsoluteArea(vr_left, vr_top, vr_right, vr_bottom);
  m_icon_groups[IG_IMG_AREA].AddOverlay("$IT_DATA/Icons/liveVideo.bmp", abs_area, true);
  m_group_shown[IG_IMG_AREA] = false;
  */

  // work order selection
  m_woselect_ctr_x = 0.5;
  vr_top = 0.05;
  double vr_arrow_left = m_woselect_ctr_x-.06;
  double vr_arrow_right = m_woselect_ctr_x+.06;
  ver_gap = .03;
  height = .15;
  abs_area = AbsoluteArea(vr_arrow_left, vr_top, vr_arrow_right, vr_bottom);
  m_icon_groups[IG_WO_SELECT].AddOverlay("$IT_DATA/Icons/scrollup.bmp", abs_area, true);
  vr_left = m_woselect_ctr_x-.38;
  vr_right = m_woselect_ctr_x+.38;
  vr_top += height+ver_gap;
  abs_area = AbsoluteArea(vr_left, vr_top, vr_right, vr_top+height);
  m_icon_groups[IG_WO_SELECT].AddOverlay("$IT_DATA/Icons/option1.bmp", abs_area, true);
  vr_top += height+ver_gap;
  abs_area = AbsoluteArea(vr_left, vr_top, vr_right, vr_top+height);
  m_icon_groups[IG_WO_SELECT].AddOverlay("$IT_DATA/Icons/option2.bmp", abs_area, true);
  vr_top += height+ver_gap;
  abs_area = AbsoluteArea(vr_left, vr_top, vr_right, vr_top+height);
  m_icon_groups[IG_WO_SELECT].AddOverlay("$IT_DATA/Icons/option3.bmp", abs_area, true);
  vr_top += height+ver_gap;
  abs_area = AbsoluteArea(vr_arrow_left, vr_top, vr_arrow_right, vr_top+height);
  m_icon_groups[IG_WO_SELECT].AddOverlay("$IT_DATA/Icons/scrolldown.bmp", abs_area, true);
  m_group_shown[IG_WO_SELECT] = false;

  // number selector, a grid of numbers
  double xmargin = .08;
  double ymargin = .08;
  ver_gap = .023;
  hor_gap = .03;
  int num_per_col = 5; // must be >1
  width = (1.0-2*xmargin-(num_per_col-1)*ver_gap)/(double)num_per_col;
  height = width*.85;
  for (int number=0; number<18; number++) {
    const int buflen = 16;
    char buf[buflen];
//    StringCbPrintfA(buf, buflen*sizeof(char), "%d", (number+1));
    sprintf(buf, "%d", (number+1));
    string filename = "$IT_DATA/Icons/number";
    filename.append(buf).append(".bmp");
    // where?
    int xpos = number%num_per_col;
    int ypos = number/num_per_col;
    vr_left = xmargin+xpos*(ver_gap+width);
    vr_top = ymargin+ypos*(hor_gap+height);
    abs_area = AbsoluteArea(vr_left, vr_top, vr_left+width, vr_top+height);
    m_icon_groups[IG_NUM_SELECT].AddOverlay(filename, abs_area, true);
  }
  m_group_shown[IG_NUM_SELECT] = false;

  // hand pointer
  abs_area = AbsoluteArea(0, 0, 0.18, 0.18);
  m_icon_groups[IG_HAND_POINTER].AddOverlay("$IT_DATA/Icons/hand_pointer.bmp", abs_area, false);
  m_group_shown[IG_HAND_POINTER] = false;

  // hand pointer
  abs_area = AbsoluteArea(0.67, 0.47, 0.87, 0.78);
  m_icon_groups[IG_HAND_FLAT].AddOverlay("$IT_DATA/Icons/hand_flat.bmp", abs_area, false);
  m_group_shown[IG_HAND_FLAT] = false;
  DbgLog((LOG_CUSTOM1, 4, "... done loading icons"));
}
