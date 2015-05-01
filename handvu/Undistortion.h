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
  * $Id: Undistortion.h,v 1.8 2004/11/11 01:58:12 matz Exp $
**/

#if !defined(__UNDISTORTION_H__INCLUDED_)
#define __UNDISTORTION_H__INCLUDED_

#include <cv.h>
#include <string>
using namespace std;

typedef struct _CvCameraParams
{
    float   focalLength[2];
    float   distortion[4];
    float   principalPoint[2];
    float   matrix[9];
    float   rotMatr[9];
    float   transVect[3];
}
CvCameraParams;


class Undistortion {
 public:
  Undistortion();
  ~Undistortion();
  
 public:
  void Initialize(int width, int height);
  bool CanUndistort() const;
  void Load(string filename);
  void Undistort(IplImage* iplImage) const;
  void Undistort(IplImage* srcImg, IplImage* dstImg) const; 

protected:
  bool                    m_can_undistort;
  bool                    m_initialized;

  /* camera parameters */
  CvCameraParams          m_camera;
  IplImage*               m_undist_img;
  CvCameraParams          m_undistort_params;
  IplImage*               m_undistort_data;
};





#endif // __UNDISTORTION_H__INCLUDED_

