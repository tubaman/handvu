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
  * $Id: CamShift.cpp,v 1.7 2005/01/06 06:53:41 matz Exp $
**/

#include "Common.h"
#include "CamShift.h"
#include "Exceptions.h"


CamShift::CamShift() 
: m_bins(20),
  m_threshold(0),
  m_Smin(20),
  m_Vmin(40),
  m_Vmax(255),
  m_tracking(false),
  m_pProbProv(NULL)
{
}


void CamShift::Initialize(int width, int height)
{
  m_img_width = width;
  m_img_height = height;
  m_prob_map = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
}

//
// Process
//
// init or adjust CamShift 
//
void CamShift::PrepareTracking(IplImage* image, const ProbDistrProvider* pProbProv, const CRect& area)
{
  m_pProbProv = pProbProv;
  if (m_pProbProv) {
    m_comp.rect.x = area.left;
    m_comp.rect.y = area.top;
    m_comp.rect.width = area.right-area.left;
    m_comp.rect.height = area.bottom-area.top;
    DoExternal(image);

  } else {
    m_object.x = area.left;
    m_object.y = area.top;
    m_object.width = area.right-area.left;
    m_object.height = area.bottom-area.top;

    ASSERT(image->roi==0);

    DoHSV(image, true);
  }

  m_tracking = true;
} // Start



//
// Process
//
// init or adjust CamShift 
//
void CamShift::Track(IplImage* image)
{
  if (m_tracking) {
    if (m_pProbProv) {
      DoExternal(image);
    } else {
      DoHSV(image, false);
    }
  }
} // Process



//
// Draw
//
void CamShift::DrawOverlay(IplImage* image, int overlay_level) const
{
  if (m_tracking && overlay_level>1) {
    if (overlay_level>2 && m_pProbProv==NULL) {
      CheckBackProject(image, 1);
    }
    DrawCross(image);
  }
} // Draw





void CamShift::DoHSV(IplImage* image, bool initialize)
{
  m_cCamShift.set_hist_dims( 1, &m_bins );
  m_cCamShift.set_hist_bin_range( 0, 0, 180 );
  m_cCamShift.set_threshold( 0 );
  m_cCamShift.set_min_ch_val( 1, m_Smin );
  m_cCamShift.set_max_ch_val( 1, 255 );
  m_cCamShift.set_min_ch_val( 2, m_Vmin );
  m_cCamShift.set_max_ch_val( 2, m_Vmax );
  
  CvSize size;
  cvGetImageRawData( image, 0, 0, &size );

  if( m_object.x < 0 ) m_object.x = 0;
  if( m_object.x > size.width - m_object.width - 1 )
      m_object.x = MAX(0, size.width - m_object.width - 1);

  if( m_object.y < 0 ) m_object.y = 0;
  if( m_object.y > size.height - m_object.height - 1 )
      m_object.y = MAX(0, size.height - m_object.height - 1);

  if( m_object.width > size.width - m_object.x )
      m_object.width = MIN(size.width, size.width - m_object.x);

  if( m_object.height > size.height - m_object.y )
      m_object.height = MIN(size.height, size.height - m_object.y);

  m_cCamShift.set_window(m_object);
  
  if (initialize) {
      m_cCamShift.reset_histogram();
      m_cCamShift.update_histogram( image );
  }

  m_cCamShift.track_object( image );
  m_object = m_cCamShift.get_window();
}




void CamShift::DoExternal(IplImage* image)
{
  ASSERT(m_pProbProv);

  ASSERT(m_comp.rect.width != 0 && m_comp.rect.height != 0);
  
  CvRect rect = m_comp.rect;
  if( rect.x < 0 )
      rect.x = 0;
  if( rect.x + rect.width > m_img_width )
      rect.width = m_img_width - rect.x;
  if( rect.y < 0 )
      rect.y = 0;
  if( rect.y + rect.height > m_img_height )
      rect.height = m_img_height - rect.y;

  CRect roi;
  roi.left = m_comp.rect.x-m_comp.rect.width/2;
  roi.right = roi.left+m_comp.rect.width*2;
  roi.top = m_comp.rect.y-m_comp.rect.height/2;
  roi.bottom = roi.top+m_comp.rect.height*2;

  m_pProbProv->CreateMap(image, m_prob_map, roi);

  cvCamShift(m_prob_map, rect,
	     cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1),
	     &m_comp, &m_box );

  if( m_comp.rect.width == 0 || m_comp.rect.height == 0 )
      m_comp.rect = rect; // do not allow tracker to loose the object
}




void CamShift::CheckBackProject(IplImage* image, int view) const
{
  if( view == 1 )
  {
    IplImage* src = ((CvCamShiftTracker&)m_cCamShift).get_back_project();
    if( src && src->imageData && image )
    {
      cvSetImageROI( image, cvRect( m_object.x, m_object.y, m_object.width, m_object.height )); 
      cvSetImageROI( src, cvRect( m_object.x, m_object.y, m_object.width, m_object.height )); 
      cvCvtColor( src, image, CV_GRAY2BGR );
      cvResetImageROI( image );
      cvResetImageROI( src );
    }
  }
  else if( view == 2 && m_tracking ) 
  {
    int i, dims;
    CvSize size;

    m_cCamShift.get_hist_dims( &dims );
    cvGetImageRawData( image, 0, 0, &size );

    for( i = 0; i < dims; i++ )
    {
      int val = cvRound(m_cCamShift.query(&i));
      CvPoint p[4];

      p[0].x = p[1].x = i*size.width/(2*dims);
      p[2].x = p[3].x = (i+1)*size.width/(2*dims);

      p[1].y = p[2].y = 0;
      p[0].y = p[3].y = (val*size.height)/(3*255);

      cvFillConvexPoly( image, p, 4, CV_RGB(255,0,0));
    }
  }
}


void  CamShift::DrawCross( IplImage* image ) const
{
  if (m_pProbProv) {
    float cs = (float)cos( m_box.angle );
    float sn = (float)sin( m_box.angle );

    int x = m_comp.rect.x + m_comp.rect.width / 2;
    int y = m_comp.rect.y + m_comp.rect.height / 2;

    CvPoint p1 = {(int)(x + m_box.size.height * cs / 2),
      (int)(y + m_box.size.height * sn / 2)};
    CvPoint p2 = {(int)(x - m_box.size.height * cs / 2),
      (int)(y - m_box.size.height * sn / 2)};
    CvPoint p3 = {(int)(x + m_box.size.width * sn / 2),
      (int)(y - m_box.size.width * cs / 2)};
    CvPoint p4 = {(int)(x - m_box.size.width * sn / 2),
      (int)(y + m_box.size.width * cs / 2)};
    cvLine( image, p1, p2, CV_RGB(0,255,0) );
    cvLine( image, p4, p3, CV_RGB(0,255,0) );

  } else {
    float cs = (float)cos( m_cCamShift.get_orientation() );
    float sn = (float)sin( m_cCamShift.get_orientation() );

    int x = m_object.x + m_object.width / 2;
    int y = m_object.y + m_object.height / 2;

    CvPoint p1 = {(int)(x + m_cCamShift.get_length() * cs / 2),
      (int)(y + m_cCamShift.get_length() * sn / 2)};
    CvPoint p2 = {(int)(x - m_cCamShift.get_length() * cs / 2),
      (int)(y - m_cCamShift.get_length() * sn / 2)};
    CvPoint p3 = {(int)(x + m_cCamShift.get_width() * sn / 2),
      (int)(y - m_cCamShift.get_width() * cs / 2)};
    CvPoint p4 = {(int)(x - m_cCamShift.get_width() * sn / 2),
      (int)(y + m_cCamShift.get_width() * cs / 2)};
    cvLine( image, p1, p2, CV_RGB(255,255,255) );
    cvLine( image, p4, p3, CV_RGB(255,255,255) );
  }
}


void CamShift::GetArea(CRect& area) const {
  if (m_pProbProv) {
    area.left = m_comp.rect.x;
    area.top = m_comp.rect.y;
    area.right = m_comp.rect.x+m_comp.rect.width;
    area.bottom = m_comp.rect.y+m_comp.rect.height;
  } else {
    area.left = m_object.x;
    area.top = m_object.y;
    area.right = m_object.x+m_object.width;
    area.bottom = m_object.y+m_object.height;
  }
}
