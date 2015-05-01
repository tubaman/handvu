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
  * $Id: Undistortion.cpp,v 1.8 2005/10/28 17:47:04 matz Exp $
**/

#include "Common.h"
#include "Undistortion.h"
#include "Exceptions.h"

Undistortion::Undistortion()
: m_can_undistort(false),
  m_initialized(false),
  m_undistort_data(NULL),
  m_undist_img(NULL)
{
  m_camera.matrix[0]=-1;
}

Undistortion::~Undistortion()
{
  if (m_undist_img) cvReleaseImageData( m_undist_img );
  if (m_undistort_data) cvReleaseImage( (IplImage**)&m_undistort_data );
}

void Undistortion::Initialize(int width, int height)
{
  if (m_undist_img) cvReleaseImageData( m_undist_img );
  if (m_undistort_data) cvReleaseImage( (IplImage**)&m_undistort_data );

  m_undist_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);

  /* create data image for faster undistortion */
  if (m_can_undistort) {
    m_undistort_data = cvCreateImage(cvSize(width, height), IPL_DEPTH_32S, 3);
    //    cvUnDistortInit( m_undist_img, m_undistort_data, m_camera.matrix,
    //        m_camera.distortion );
    // todo: fix for OpenCV 0.9.7
  }

  m_initialized = true;
}

bool Undistortion::CanUndistort() const
{
  return m_can_undistort;
}


void Undistortion::Load(string filename)
{
#define BUF_SIZE 10000
  char buffer[BUF_SIZE + 100];

  if(filename.size()) {
    FILE *file = fopen(filename.c_str(), "rb");

    if (file) {
      int i, j, k;
      float cameraMatrix[9];
      float distortion[4];

      int sz = (int) fread( buffer, 1, BUF_SIZE, file );
      char* ptr = buffer;
      buffer[sz] = '\0';

      /* read matrix */
      for( k = 0; k < 9; k++ )
      {
        ptr = strstr( ptr, "M[" );
        if( ptr )
        {
          int s = 0;
          ptr += 2;
          if( sscanf( ptr, "%d%*[.,]%d%n", &i, &j, &s ) == 2 && i == k/3 && j == k%3 )
          {
            ptr += s;
            ptr = strstr( ptr, "=" );
            if( ptr )
            {
              s = 0;
              ptr++;
              if( sscanf( ptr, "%f%n", cameraMatrix + k, &s ) == 1 )
              {
                ptr += s;
                continue;
              }
            }
          }
        }

        /* else report a bug */
        throw HVException("Invalid camera parameters file format");
      }

      /* read distortion */
      for( k = 0; k < 4; k++ )
      {
        ptr = strstr( ptr, "D[" );
        if( ptr )
        {
          int s = 0;
          ptr += 2;
          if( sscanf( ptr, "%d%n", &i, &s ) == 1 && i == k )
          {
            ptr += s;
            ptr = strstr( ptr, "=" );
            if( ptr )
            {
              s = 0;
              ptr++;
              if( sscanf( ptr, "%f%n", distortion + k, &s ) == 1 )
              {
                ptr += s;
                continue;
              }
            }
          }
        }

        /* else report a bug */
        throw HVException("Invalid camera parameters file format");
      }

      memcpy( m_camera.matrix, cameraMatrix, sizeof( cameraMatrix ));
      memcpy( m_camera.distortion, distortion, sizeof( distortion ));

      m_camera.focalLength[0] = m_camera.matrix[0];
      m_camera.focalLength[1] = m_camera.matrix[4];

      m_camera.principalPoint[0] = m_camera.matrix[2];
      m_camera.principalPoint[1] = m_camera.matrix[5];

      m_can_undistort = true;

      fclose(file);
    }
    else
    {
      throw HVException("Cannot open camera parameters file");
    }
  }

  if (m_initialized) {
    if (m_undistort_data) cvReleaseImage( (IplImage**)&m_undistort_data );
    m_undistort_data = cvCreateImage(cvSize(m_undist_img->width, m_undist_img->height), IPL_DEPTH_32S, 3);
    //cvUnDistortInit( m_undist_img, m_undistort_data, m_camera.matrix,
    //  m_camera.distortion );
    // todo: fix for OpenCV 0.9.7
  }
}


void Undistortion::Undistort(IplImage* iplImage) const
{
  if (!m_initialized) {
    throw HVException("Undistort not initialized");
  }

  if (!m_can_undistort) {
    return;
  }

  throw HVException("Undistortion not fixed for OPenCV 0.9.7 yet");
  //  cvUnDistort(iplImage, m_undist_img, m_undistort_data);
  cvCopyImage(m_undist_img, iplImage);
}

void Undistortion::Undistort(IplImage* /*srcImg*/, IplImage* /*dstImg*/) const
{
  throw HVException("Undistort(src, dst) not implemented");
}

 
