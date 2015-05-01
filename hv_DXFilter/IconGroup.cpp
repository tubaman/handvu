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
  * $Id: IconGroup.cpp,v 1.7 2004/11/24 08:38:40 matz Exp $
**/

#include "StdAfx.h"
#include "IconGroup.h"
#include "Common.h"
#include <highgui.h>

string TestAndConvertFilename(string filename)
{
  // $IT_DATA environmet variable, NULL if not set
  char *it_data = getenv("IT_DATA");

  if (it_data!=NULL) {
    // substitute for "$IT_DATA" the value of that environment variable;
    ReplaceAll(filename, "$IT_DATA", it_data);
  }
  
  string win_filename = ConvertPathToWindows(filename);
  FILE* fp = fopen(win_filename.c_str(), "r");
  if (fp==NULL) {
    AfxMessageBox(string("file does not exist: ").append(win_filename).c_str());
    return "";
  }
  fclose(fp);
  return win_filename;
}


////////////////////////////////////////////////////////////////////////////
// Class CIconGroup
//

CIconGroup::CIconGroup()
{
}

CIconGroup::~CIconGroup()
{
  for (int i=0; i<(int)m_images.size(); i++) {
    cvReleaseImage(&m_images[i]);
  }
}

void CIconGroup::AddOverlay(string filename, CRect& area, bool highlite)
{
  string win_filename = TestAndConvertFilename(filename);
  if (win_filename=="") {
    AfxMessageBox("empty filename in AddOverlay");
    return;
  }

  IplImage* loadimg = 0;
  loadimg = cvLoadImage(win_filename.c_str(), 1);
  if (!loadimg) {
    string msg = "error: can not load image ";
    msg = msg + win_filename.c_str() + " with OpenCV highgui";
    AfxMessageBox(msg.c_str());
    exit(-1);
  }

  CvRect cvBox = cvRect(0, 0, loadimg->width-2, loadimg->height-2);
  cvSetImageROI(loadimg, cvBox);
  IplImage* tmpimg = 
    cvCreateImage(cvSize(area.right-area.left, area.bottom-area.top), 
                  loadimg->depth, loadimg->nChannels);
  cvResize(loadimg, tmpimg);
  cvResetImageROI(loadimg);
  cvReleaseImage(&loadimg);

  m_images.push_back(tmpimg);
  m_areas.push_back(area);
  m_display_icon.push_back(true);
  m_highlight_area.push_back(highlite);
  m_highlight_active.push_back(false);
  m_is_selected.push_back(false);
}

bool CIconGroup::Intersect(const CRect& area, int x, int y)
{
  return area.left<=x && x<area.right && area.top<=y && y<area.bottom;
}

/** set to highlite the buttons that have the pointer over them
*/
void CIconGroup::PointerOver(int x, int y)
{
  for (int icnt=0; icnt<(int)m_display_icon.size(); icnt++) {
    if (m_display_icon[icnt] && m_highlight_area[icnt]) {
      if (Intersect(m_areas[icnt], x, y)) {
        m_highlight_active[icnt] = true;
      } else {
        m_highlight_active[icnt] = false;
      }
    }
  }
}

    // james: put app-hookup right here or in IconGroup::Select
bool CIconGroup::Select(int x, int y)
{
  bool got_one = false;
  for (int icnt=0; icnt<(int)m_display_icon.size(); icnt++) {
    if (m_display_icon[icnt] && m_highlight_area[icnt]) {
      if (Intersect(m_areas[icnt], x, y)) {
        m_is_selected[icnt] = true;
        got_one = true;
      }
    }
  }
  return got_one;
}

void CIconGroup::ResetSelection()
{
  for (int icnt=0; icnt<(int)m_display_icon.size(); icnt++) {
    m_is_selected[icnt] = false;
  }
}

