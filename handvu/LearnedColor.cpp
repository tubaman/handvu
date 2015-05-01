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
  * $Id: LearnedColor.cpp,v 1.14 2005/01/18 22:02:42 matz Exp $
**/

#include "Common.h"
#include "LearnedColor.h"
#include "Exceptions.h"

//
// Constructor
//
LearnedColor::LearnedColor()
  : m_truth_map(NULL),
    m_truth_pos(0),
    m_truth_neg(0),
    m_rgb_lookup_numbins(0),
    m_rgb_lookup_binsize(0)
{
} // (Constructor)


LearnedColor::~LearnedColor()
{
}

void LearnedColor::Initialize(int width, int height)
{
  m_truth_map = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);
}

void LearnedColor::DrawOverlay(IplImage* rgbImage, int overlay_level, 
                            const CRect& roi)
{
  if (overlay_level>=3) {
    Backproject(rgbImage, roi);
  }
}

/* draw the backprojection of the learned RGB lookup table
 */
void LearnedColor::Backproject(IplImage* rgbImage, const CRect& roi)
{
  ASSERT(rgbImage && rgbImage->imageData);

  ColorBGR black; 
  black.red = black.green = black.blue = 0;

  int start_x = max(0, roi.left);
  int start_y = max(0, roi.top);
  int stop_x = min(rgbImage->width, roi.right);
  int stop_y = min(rgbImage->height, roi.bottom);

  for (int y=start_y; y<stop_y; y++) {
    ColorBGR* prgb = (ColorBGR*) rgbImage->imageData;
    prgb += y*rgbImage->width + start_x;
    for (int x=start_x; x<stop_x; x++, prgb++) {
      // if not foreground, set pixel to black
      prgb->red = prgb->green = prgb->blue = (BYTE) (255 * LookupProb(*prgb));
//      if (LookupProb(*prgb)<0.5) {
//        *prgb = black;
//      }
    }
  }
  
  int width = min(rgbImage->width-roi.left, 
                  min(roi.right, rgbImage->width-1)-roi.left);
  int height = min(rgbImage->height-roi.top, 
                   min(roi.bottom, rgbImage->height-1)-roi.top);
  CvRect cvBox = cvRect(roi.left, roi.top, width, height);
  cvSetImageROI(rgbImage, cvBox);
//  cvErode(rgbImage, rgbImage, NULL, 2);
  // cvDilate(rgbImage, rgbImage, NULL, 2);
  cvResetImageROI(rgbImage);
}

/* return (in "map") the probabilities based on the learned RGB lookup table
* map must be allocated image with IPL_DEPTH_8U
 */
void LearnedColor::CreateMap(const IplImage* rgbImage, IplImage* mapImage, const CRect& roi) const
{
  ASSERT(rgbImage && rgbImage->imageData);
  ASSERT(mapImage && mapImage->imageData);
  ASSERT(mapImage->width==rgbImage->width && mapImage->height==rgbImage->height);

  int start_x = max(0, roi.left);
  int start_y = max(0, roi.top);
  int stop_x = min(rgbImage->width, roi.right);
  int stop_y = min(rgbImage->height, roi.bottom);

  for (int y=start_y; y<stop_y; y++) {
    ColorBGR* prgb = (ColorBGR*) rgbImage->imageData;
    prgb += y*rgbImage->width + start_x;
    BYTE* pmap = (BYTE*) mapImage->imageData;
    pmap += y*mapImage->width + start_x;
    for (int x=start_x; x<stop_x; x++, prgb++, pmap++) {
      *pmap = (BYTE) (255*LookupProb(*prgb));
    }
  }
}


#ifdef DEBUG
// non-debug version is inlined in LearnedColor.h
int LearnedColor::GetRGBLookupIndex(int x, int y, int z, int size, int arraylen) const
{
  ASSERT(x>=0 && y>=0 && z>=0);
  ASSERT(size);
  int index = z*size*size + y*size + x; 
  ASSERT(0<=index && index<arraylen);
  return index;
}
#endif // DEBUG


/* look up the probability for the given color sample
 * in the learned RGB table
 */
double LearnedColor::LookupProb(ColorBGR sample) const
{
  ASSERT(m_rgb_lookup.size());
  ASSERT(m_rgb_lookup_binsize);
  int bR = sample.red/m_rgb_lookup_binsize;
  int bG = sample.green/m_rgb_lookup_binsize;
  int bB = sample.blue/m_rgb_lookup_binsize;
  ASSERT(bR<m_rgb_lookup_numbins 
         && bG<m_rgb_lookup_numbins && bB<m_rgb_lookup_numbins);
  double prob =
    m_rgb_lookup[GetRGBLookupIndex(bR, bG, bB, m_rgb_lookup_numbins,
                                   (int) m_rgb_lookup.size())];
  return prob;
}


void LearnedColor::TestSegmentation(IplImage* rgbImage, 
                                        const CRect& roi,
                                        double* fpr, double* dr, bool draw)
{
  // let's look at each pixel(sample)
  ColorBGR black; 
  black.red = black.green = black.blue = 0;

  int start_x = max(0, roi.left);
  int start_y = max(0, roi.top);
  int stop_x = min(rgbImage->width, roi.right);
  int stop_y = min(rgbImage->height, roi.bottom);

  int miss_pos=0, hit_pos=0, miss_neg=0, hit_neg=0;
  for (int y=start_y; y<stop_y; y++) {
    ColorBGR* prgb = (ColorBGR*) rgbImage->imageData;
    float* pprob = (float*) m_truth_map->imageData;
    prgb += y*rgbImage->width + start_x;
    pprob += y*rgbImage->width + start_x;
    for (int x=start_x; x<stop_x; x++, prgb++, pprob++) {
      // only consider pixels in the positive and negative areas
      if (*pprob!=1.0 && *pprob!=-1.0) {
        continue;
      }
      
      bool is_fore = (LookupProb(*prgb)>=0.5);

      // write to output buffer
      if (is_fore) {
        if (*pprob==1) {
          hit_pos++;
        } else {
          miss_neg++;
        }
      } else {
        if (*pprob==-1) {
          hit_neg++;
        } else {
          miss_pos++;
        }
        if (draw) *prgb = black;
      }
    } // end x
  } // end y

  *fpr = (double)miss_neg/(double)(miss_neg+hit_neg);
  *dr = (double)hit_pos/(double)(hit_pos+miss_pos);
  if (draw) {
    int height = 200;
    cvRectangle(rgbImage, cvPoint(0, 0), cvPoint(20, height),
                CV_RGB(255,0,0), 1);
    cvRectangle(rgbImage, cvPoint(25, 0), cvPoint(45, height),
                CV_RGB(0,255,0), 1);
    cvRectangle(rgbImage, cvPoint(0, 0), cvPoint(20, (int)(height* *fpr)),
                CV_RGB(255,0,0), CV_FILLED);
    cvRectangle(rgbImage, cvPoint(25, 0), cvPoint(45, (int)(height* *dr)),
                CV_RGB(0,255,0), CV_FILLED);
  }
}

void LearnedColor::SortSamplesIntoBins(vector<ColorBGR> samples, int* cube,
                                    int num_bins)
{
  ASSERT(num_bins);
  int binsize = (int)(256.0/(double)num_bins);
  ASSERT(binsize);
  int num_samples = (int)samples.size();
  for (int indx=0; indx<num_samples; indx++) {
    int bR = samples[indx].red/binsize;
    int bG = samples[indx].green/binsize;
    int bB = samples[indx].blue/binsize;
    ASSERT(bR<num_bins && bG<num_bins && bB<num_bins);
    int index = bB*num_bins*num_bins + bG*num_bins + bR;
    cube[index]++;
  }
}

#pragma warning (disable:4786)
void LearnedColor::LearnFromGroundTruth(IplImage* rgbImage,
                                     const CuScanMatch& match,
                                     ConstMaskIt mask)
{
  VERBOSE0(5, "HandVu: learning color from ground truth");
  CRect bbox;
  SetGroundTruth(match, mask, bbox);
  LearnLookupCube(rgbImage, bbox);
}
#pragma warning (default:4786)

void LearnedColor::LearnLookupCube(IplImage* rgbImage, const CRect& bbox)
{
  // positive and negative areas should be set through SetTrainingAreas
  ASSERT(m_truth_pos>0 && m_truth_neg>0); 

  // collect pixels; that's faster than running through the image
  // multiple times
  vector<ColorBGR> foreground;
  vector<ColorBGR> background;
  foreground.reserve(m_truth_pos);
  background.reserve(m_truth_neg);
/*
  int start_x = 0;
  int start_y = 0;
  int stop_x = rgbImage->width;
  int stop_y = rgbImage->height;
  */
  int start_x = max(0, bbox.left);
  int start_y = max(0, bbox.top);
  int stop_x = min(rgbImage->width, bbox.right);
  int stop_y = min(rgbImage->height, bbox.bottom);

  for (int y=start_y; y<stop_y; y++) {
    ColorBGR* prgb = (ColorBGR*) rgbImage->imageData;
    float* pprob = (float*) m_truth_map->imageData;
    prgb += y*rgbImage->width + start_x;
    pprob += y*rgbImage->width + start_x;
    for (int x=start_x; x<stop_x; x++, prgb++, pprob++) {
      // only consider pixels in the positive and negative areas
      if (*pprob==1.0) {
        foreground.push_back(*prgb);
      } else if (*pprob==-1.0) {
        background.push_back(*prgb);
      }
    }
  }
  ASSERT(m_truth_pos==(int)foreground.size()); 
  ASSERT(m_truth_neg==(int)background.size()); 

  // init the learning/counting
  m_rgb_lookup_numbins = 2;
  CFloatVector lookup_prev;
  m_rgb_lookup.resize(0);
  int* F_curr = NULL;
  int* B_curr = NULL;
  const int min_num_samples = 5; // stop breaking cells up if less
  // than min_num_samples samples are in cell

  // increase the number of bins per dimension until the number of samples per
  // bin does not exceed a threshold
  double max_r_xyz_curr;
  do {
    m_rgb_lookup_numbins *= 2;

    int cubed_size =
      m_rgb_lookup_numbins*m_rgb_lookup_numbins*m_rgb_lookup_numbins;
    m_rgb_lookup.swap(lookup_prev);
    m_rgb_lookup.resize(cubed_size);
    delete[] F_curr;
    F_curr = new int[cubed_size];
    memset(F_curr, 0, cubed_size*sizeof(int));
    delete[] B_curr;
    B_curr = new int[cubed_size];
    memset(B_curr, 0, cubed_size*sizeof(int));
    SortSamplesIntoBins(foreground, F_curr, m_rgb_lookup_numbins);
    SortSamplesIntoBins(background, B_curr, m_rgb_lookup_numbins);

    max_r_xyz_curr = 0;
    for (int z=0; z<m_rgb_lookup_numbins; z++) {
      for (int y=0; y<m_rgb_lookup_numbins; y++) {
        for (int x=0; x<m_rgb_lookup_numbins; x++) {
          // find the probability of pixels in this bin to be foreground color
          int index = GetRGBLookupIndex(x,y,z,m_rgb_lookup_numbins,
                                        (int) m_rgb_lookup.size());
          int F = F_curr[index];
          int B = B_curr[index];
          int n_xyz_curr = F + B;
          if (n_xyz_curr<min_num_samples && (int)lookup_prev.size()>0) {
            m_rgb_lookup[index] = 
              lookup_prev[GetRGBLookupIndex(x/2,y/2,z/2,m_rgb_lookup_numbins/2,
                                            (int) lookup_prev.size())];

          } else {
            double rc = (double)n_xyz_curr/
              (double)(m_truth_pos+m_truth_neg);
            max_r_xyz_curr = max(max_r_xyz_curr, rc);

            double f = (double)F/(double)m_truth_pos;
            double b = (double)B/(double)m_truth_neg;
            if (f!=0 && b!=0) {
              m_rgb_lookup[index] = (float) (f/(f+b));

            } else if (f==0 && b!=0) {
              m_rgb_lookup[index] = 0.0f;

            } else if (f!=0 && b==0) {
              m_rgb_lookup[index] = 1.0f;

            } else if ((int)lookup_prev.size()>0) {
              m_rgb_lookup[index] =
                lookup_prev[GetRGBLookupIndex(x/2,y/2,z/2,
                                              m_rgb_lookup_numbins/2,
                                              (int) lookup_prev.size())];

            } else {
              m_rgb_lookup[index] = 0.0f;
            }
          }
          ASSERT(0<=m_rgb_lookup[index]);
          ASSERT(m_rgb_lookup[index]<=1);
        }
      }
    }

  } while (max_r_xyz_curr>=0.05 && m_rgb_lookup_numbins<64);

  m_rgb_lookup_binsize = (int)(256.0/(double)m_rgb_lookup_numbins);

  delete[] F_curr;
  delete[] B_curr;
}

void LearnedColor::GetMostRightUpBlob(IplImage* rgbImage,
  const CRect& roi, CvPoint2D32f& pos)
{
  int start_x = max(0, roi.left);
  int start_y = max(0, roi.top);
  int stop_x = min(rgbImage->width, roi.right);
  int stop_y = min(rgbImage->height, roi.bottom);
  int width = stop_x-start_x;
  int height = stop_y-start_y;

  ASSERT(rgbImage && rgbImage->imageData);

  int cnt_consecutive = 0;
  int diag = 1;
  do {
    ColorBGR* prgb = (ColorBGR*) rgbImage->imageData;
    prgb += start_y*rgbImage->width + stop_x-diag;
    for (int x=stop_x-diag, y=start_y; x<stop_x && y<stop_y; x++, y++) {
      if (LookupProb(*prgb)>=0.5) {
        cnt_consecutive ++;
        if (cnt_consecutive>20) {
          pos.x = (float)x;
          pos.y = (float)y;
          cvCircle(rgbImage, cvPoint(x, y), 12, CV_RGB(0, 255, 0), 1);
          return;
        }
      } else {
        cnt_consecutive = 0;
      }

      prgb += rgbImage->width + 1;
    }
    diag++;
  } while (diag<width+height);

  // no blob found
  pos.x = -1; pos.y = -1;
}

#pragma warning (disable:4786)
void LearnedColor::SetGroundTruth(const CuScanMatch& match,
                               ConstMaskIt mask, CRect& bbox)
{
  cvSet(m_truth_map, cvScalar(0));
  m_truth_pos = 0;
  m_truth_neg = 0;

  // set positive pixels where the mask has a skin-color
  // probability of at least scp_thresh (skin-color-prob)
  const double scp_thresh = 0.7;
  double scale_x = (double)(match.right-match.left-1)/((*mask).second.GetWidth()-1.);
  double scale_y = (double)(match.bottom-match.top-1)/((*mask).second.GetHeight()-1.);

  CvPixelPosition32f position;
  CV_INIT_PIXEL_POS(position, 
                    (float*)(m_truth_map->imageData),
                    m_truth_map->widthStep, 
                    cvSize(m_truth_map->width, m_truth_map->height), 0, 0,
                    m_truth_map->origin);
  ASSERT(m_truth_map->nChannels==1);

  for (int y=match.top; y<match.bottom; y++) {
    for (int x=match.left; x<match.right; x++) {
      int m_x = cvRound((double)(x-match.left)/scale_x);
      int m_y = cvRound((double)(y-match.top)/scale_y);
      double scp = (*mask).second.GetProb(m_x, m_y);
      if (scp>=scp_thresh) {
        CV_MOVE_TO(position, x, y, 1);
        *CV_GET_CURRENT(position, 1) = 1.0f;
        m_truth_pos ++;
      }
    }
  }

  // set negative area at a certain distance from the hand region
  int width = match.right-match.left;
  int height = match.bottom-match.top;
  int leftmost = match.left-width;
  int leftmid = match.left-width/2;
  int topmost = match.top-height;
  int topmid = match.top-height/2;
  int rightmost = match.right+width;
  int rightmid = match.right+width/2;
  int bottom = match.bottom;
  
  CRect neg_mask_left(leftmost, topmid, leftmid, bottom);
  ModifyGroundTruth(neg_mask_left, -1.0);

  CRect neg_mask_top(leftmost, topmost, rightmost, topmid);
  ModifyGroundTruth(neg_mask_top, -1.0);

  CRect neg_mask_right(rightmid, topmid, rightmost, bottom);
  ModifyGroundTruth(neg_mask_right, -1.0);
    
  bbox = CRect(leftmost, topmost, rightmost, bottom);
}
#pragma warning (default:4786)

/* add areas with certain probabilities to the ground truth map
   with reset==true, the probability mask is zeroed out first,
   otherwise it's kept.
   probability==1 means foreground color for sure,
   probability==0 means background color; if
   probability==-1, that area is used to train a specific
   background color model.
   The area consists of four absolute coordinates (not width, height).
   all the computation will be done the next time Transform is called.
*/
void LearnedColor::ModifyGroundTruth(const CRect& area, double probability)
{
  ASSERT(m_truth_map!=NULL);
  int x_to = min(area.right, m_truth_map->width);
  int x_from = max(0, area.left);
  int y_to = min(area.bottom, m_truth_map->height);

  CvPixelPosition32f position;
  CV_INIT_PIXEL_POS(position, 
                    (float*)(m_truth_map->imageData),
                    m_truth_map->widthStep, 
                    cvSize(m_truth_map->width, m_truth_map->height), 0, 0,
                    m_truth_map->origin);
  ASSERT(m_truth_map->nChannels==1);
  for (int y=max(0, area.top); y<y_to; y++) {
    CV_MOVE_TO(position, x_from, y, 1);
    for (int x=x_from; x<x_to; x++) {
      *CV_GET_CURRENT(position, 1) = (float) probability;
      CV_MOVE_RIGHT(position, 1);
      if (probability==1) {
        m_truth_pos ++;
      } else if (probability==-1) {
        m_truth_neg ++;
      }
    }
  }
}
