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
  * $Id: LearnedColor.h,v 1.8 2004/11/11 01:58:12 matz Exp $
**/

#if !defined(__LEARNEDCOLOR_H__INCLUDED_)
#define __LEARNEDCOLOR__INCLUDED_

#include <cv.h>
#include "OpticalFlow.h"
#include "Mask.h"
#include "cubicles.h"
#include "Rect.h"

#pragma warning (disable:4786)
class LearnedColor : public ProbDistrProvider {
 public:
  LearnedColor();
  ~LearnedColor();

 public:
  void Initialize(int width, int height);
  void Backproject(IplImage* rgbImage, const CRect& roi);
  void BuildMap(const IplImage* rgbImage, IplImage* map, const CRect& roi);
  virtual double LookupProb(ColorBGR sample) const;
  virtual void CreateMap(const IplImage* rgbImage, IplImage* map, const CRect& roi) const;
  void TestSegmentation(IplImage* rgbImage, 
                        const CRect& roi,
                        double* fpr, double* dr, bool draw);
  void DrawOverlay(IplImage* rgbImage, int overlay_level, 
                   const CRect& roi);
  void LearnFromGroundTruth(IplImage* rgbImage,
                            const CuScanMatch& match, ConstMaskIt mask);
  void GetMostRightUpBlob(IplImage* rgbImage,
    const CRect& roi, CvPoint2D32f& pos);

 protected:
  void SortSamplesIntoBins(vector<ColorBGR> samples, int* cube, int num_bins);
  void ModifyGroundTruth(const CRect& area, double probability);
  void LearnLookupCube(IplImage* rgbImage, const CRect& bbox);
  void SetGroundTruth(const CuScanMatch& match, ConstMaskIt mask,
                      CRect& bbox);
#ifdef DEBUG
  int GetRGBLookupIndex(int x, int y, int z, int size, int arraylen) const;
#else
  int GetRGBLookupIndex(int x, int y, int z, int size, int) const
    { return z*size*size + y*size + x; }
#endif
  
 protected:
  IplImage*              m_truth_map;
  int                    m_truth_pos;
  int                    m_truth_neg;
  vector<float>          m_rgb_lookup;
  int                    m_rgb_lookup_numbins;
  int                    m_rgb_lookup_binsize;
};


#pragma warning (default:4786)



#endif // __SKINCOLOR_H__INCLUDED_

