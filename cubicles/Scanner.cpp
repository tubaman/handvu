/**
  * cubicles
  *
  * This is an implementation of the Viola-Jones object detection 
  * method and some extensions.  The code is mostly platform-
  * independent and uses only standard C and C++ libraries.  It
  * can make use of MPI for parallel training and a few Windows
  * MFC functions for classifier display.
  *
  * Mathias Kolsch, matz@cs.ucsb.edu
  *
  * $Id: Scanner.cpp,v 1.48 2004/11/11 01:58:58 matz Exp $
**/

// Scanner scans across an image and finds matches for the 
// classifier cascade.  There's also a Scanner_Train.cpp
// implementation file that for training-only functions.
//

////////////////////////////////////////////////////////////////////
//
// By downloading, copying, installing or using the software you 
// agree to this license.  If you do not agree to this license, 
// do not download, install, copy or use the software.
//
// Copyright (C) 2004, Mathias Kolsch, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in binary form, with or without 
// modification, is permitted for non-commercial purposes only.
// Redistribution in source, with or without modification, is 
// prohibited without prior written permission.
// If granted in writing in another document, personal use and 
// modification are permitted provided that the following two
// conditions are met:
//
// 1.Any modification of source code must retain the above 
//   copyright notice, this list of conditions and the following 
//   disclaimer.
//
// 2.Redistribution's in binary form must reproduce the above 
//   copyright notice, this list of conditions and the following 
//   disclaimer in the documentation and/or other materials provided
//   with the distribution.
//
// This software is provided by the copyright holders and 
// contributors "as is" and any express or implied warranties, 
// including, but not limited to, the implied warranties of 
// merchantability and fitness for a particular purpose are 
// disclaimed.  In no event shall the copyright holder or 
// contributors be liable for any direct, indirect, incidental, 
// special, exemplary, or consequential damages (including, but not 
// limited to, procurement of substitute goods or services; loss of 
// use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict 
// liability, or tort (including negligence or otherwise) arising 
// in any way out of the use of this software, even if advised of 
// the possibility of such damage.
//
////////////////////////////////////////////////////////////////////


#include "cubicles.hpp"
#include "Scanner.h"
#include "Cascade.h"
#include <math.h>
#include <iostream>

#ifdef _DEBUG
#ifdef USE_MFC
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif // USE_MFC
#endif // _DEBUG


// ----------------------------------------------------------------------
// class CImageScanner
// ----------------------------------------------------------------------

CImageScanner::CImageScanner() 
: m_is_active(true),
  m_post_process(false)
{
  SetScanParameters();
}

CImageScanner::CImageScanner(const CImageScanner& src) 
: m_is_active(src.m_is_active),
  m_start_scale(src.m_start_scale),
  m_stop_scale(src.m_stop_scale),
  m_scale_inc_factor(src.m_scale_inc_factor),
  m_translation_inc_x(src.m_translation_inc_x),
  m_translation_inc_y(src.m_translation_inc_y),
  m_scan_area(src.m_scan_area),
  m_post_process(src.m_post_process),
  m_min_scaled_template_width(-1),
  m_max_scaled_template_width(-1),
  m_min_scaled_template_height(-1),
  m_max_scaled_template_height(-1),
  m_integral(src.m_integral),
  m_squared_integral(src.m_squared_integral)
{
}

void CImageScanner::SetScanParameters(
                                      double start_scale /* = 1.0 */,
                                      double stop_scale /* = DBL_MAX */,
                                      double scale_inc_factor /* = 1.25 */,
                                      double translation_inc_x /* = 1.0 */,
                                      double translation_inc_y /* = 1.0 */,
                                      CRect scan_area /* = CRect(0, 0, INT_MAX, INT_MAX) */
                                      )
{
  SetScanScales(start_scale, stop_scale);
  m_scale_inc_factor = scale_inc_factor;
  m_translation_inc_x = translation_inc_x;
  m_translation_inc_y = translation_inc_y;
  m_scan_area = scan_area;
  m_min_scaled_template_width = -1;
  m_max_scaled_template_width = -1;
  m_min_scaled_template_height = -1;
  m_max_scaled_template_height = -1;
}

void CImageScanner::GetScanParameters(
    double* pStart_scale,
    double* pStop_scale,
    double* pScale_inc_factor,
    double* pTranslation_inc_x,
    double* pTranslation_inc_y,
    CRect& scan_area,
    bool* pPostProcessing,
    bool* pIsActive
    ) const
{
  *pStart_scale = m_start_scale;
  *pStop_scale = m_stop_scale;
  *pScale_inc_factor = m_scale_inc_factor;
  *pTranslation_inc_x = m_translation_inc_x;
  *pTranslation_inc_y = m_translation_inc_y;
  *pPostProcessing = m_post_process;
  *pIsActive = m_is_active;
  scan_area = m_scan_area;
}

void CImageScanner::SetScanArea(const CRect& scan_area)
{
  m_scan_area = scan_area;
}

void CImageScanner::SetScanScales(double start_scale, double stop_scale)
{
  m_start_scale = start_scale;
  m_stop_scale = stop_scale;
  m_min_scaled_template_width = -1;
  m_max_scaled_template_width = -1;
  m_min_scaled_template_height = -1;
  m_max_scaled_template_height = -1;
}

// must be called after the actual scan, and the behavior with
// multiple active scanners is somewhat undetermined
void CImageScanner::GetScaleSizes(int* min_width, int* max_width,
				  int* min_height, int* max_height) const
{
  *min_width = m_min_scaled_template_width;
  *max_width = m_max_scaled_template_width;
  *min_height = m_min_scaled_template_height;
  *max_height = m_max_scaled_template_height;
}

void CImageScanner::SetAutoPostProcessing(bool on /*=true*/)
{
  m_post_process = on;
}

const CRect& CImageScanner::GetScanArea() const
{
  return m_scan_area;
}

int
CImageScanner::Scan(const CClassifierCascade& cascade,
		    const CByteImage& image, CScanMatchVector& posClsfd) const
{
  if (!m_is_active) return -1;

  // make integral of regular and squared image
  CIntegralImage::CreateSimpleNSquaredFrom(image, m_integral,
					   m_squared_integral, m_scan_area);
  return Scan(cascade, m_integral, m_squared_integral, posClsfd);
}

int
CImageScanner::Scan(const CClassifierCascade& cascade,
		    const CIntegralImage& integral,
                    const CIntegralImage& squared_integral,
                    CScanMatchVector& posClsfd) const
{
  if (!m_is_active) return -1;

  posClsfd.clear();  
  CScaleParams sclprms;
  InitScaleParams(cascade, sclprms);
  m_min_scaled_template_width = sclprms.scaled_template_width;
  m_min_scaled_template_height = sclprms.scaled_template_height;

  double N = sclprms.scaled_template_width * sclprms.scaled_template_height;
  
  int width = integral.GetWidth();
  int height = integral.GetHeight();
  
  CStringVector matches;
  int scancnt=0;
  while (sclprms.scaled_template_width<width && sclprms.scaled_template_height<height
    && sclprms.base_scale<m_stop_scale) 
  {
    cascade.ScaleFeaturesEvenly(sclprms.actual_scale_x, 
				sclprms.actual_scale_y,
				sclprms.scaled_template_width, 
				sclprms.scaled_template_height);

    // for each y-location in the image
    int top_stop = min(m_scan_area.bottom, height)-sclprms.scaled_template_height;
    for (int top=max(0, m_scan_area.top); top<top_stop; top+=(int)sclprms.translation_inc_y) {
      int bottom = top+sclprms.scaled_template_height;

      // for each x-location in the image
      int left_stop = min(m_scan_area.right, width)-sclprms.scaled_template_width;
      for (int left=max(0, m_scan_area.left); left<left_stop; left+=(int)sclprms.translation_inc_x) {
        int right = left+sclprms.scaled_template_width;

        double sum_x = 
          integral.GetElement(right-1, bottom-1) 
          - integral.GetElement(right-1, top-1)
          - integral.GetElement(left-1, bottom-1)
          + integral.GetElement(left-1, top-1);
        double mean =
          sum_x / N;
        double sum_x2 = 
          squared_integral.GetElement(right-1, bottom-1) 
          - squared_integral.GetElement(right-1, top-1)
          - squared_integral.GetElement(left-1, bottom-1)
          + squared_integral.GetElement(left-1, top-1);
        double stddev = sqrt(fabs(mean*mean - sum_x2/N));
        //  double stddev_equal = sqrt(fabs(mean*mean - 2.0*sum_x*mean/N + sum_x2/N));

        bool is_positive =
          cascade.Evaluate(integral, mean, stddev, left, top, matches);
        if (is_positive) {
          for (int m=0; m<(int)matches.size(); m++) {
            posClsfd.push_back(CScanMatch(left, top, right, bottom,
                                          sclprms.base_scale,
                                          sclprms.scale_x, sclprms.scale_y,
                                          matches[m]));
          }
          matches.clear();
        }
        scancnt++;
      }
    }

    m_max_scaled_template_width = sclprms.scaled_template_width;
    m_max_scaled_template_height = sclprms.scaled_template_height;
    NextScaleParams(sclprms);
    ASSERT(N!=sclprms.scaled_template_width*sclprms.scaled_template_height);
    N = sclprms.scaled_template_width * sclprms.scaled_template_height;
  }

  if (m_post_process) {
    PostProcess(posClsfd);
    return (int) posClsfd.size();
  } else {
    return scancnt;
  }
}

bool intersect(const CScanMatch& a, const CScanMatch& b)
{
	if (a.left<=b.left && b.left<=a.right ||
		  a.left<=b.right && b.right<=a.right ||
			a.top<=b.top && b.top<=a.bottom ||
			a.top<=b.bottom && b.bottom<=a.bottom ||
			b.left<=a.left && a.left<=b.right ||
		  b.left<=a.right && a.right<=b.right ||
			b.top<=a.top && a.top<=b.bottom ||
			b.top<=a.bottom && a.bottom<=b.bottom)
	{
		return true;
	} else {
		return false;
	}
}

/** throw out some positives if they overlap, pick the average
* of each coordinate for every overlapping set
*/
void CImageScanner::PostProcess(CScanMatchVector& posClsfd) const
{
	int num_matches = (int)posClsfd.size();
	if (num_matches<2) return;

	// partition all matches into disjoint clusters
	CIntVector clustnums;
	clustnums.resize(num_matches);
	clustnums[0] = 0;
	int num_clusters = 1;
	for (int matchcnt=1; matchcnt<num_matches; matchcnt++) {
		clustnums[matchcnt] = -1;
		for (int prevcnt=0; prevcnt<matchcnt; prevcnt++) {
			if (intersect(posClsfd[prevcnt], posClsfd[matchcnt])) {
				if (clustnums[matchcnt]==-1) { 
					// prevcnt is first rectangle that matchcnt intersects with
					clustnums[matchcnt] = clustnums[prevcnt];
				} else {
					// prevcnt is not the first rectangle that matchcnt intersects with,
					// need to make sure that if prevcnt is from a different cluster
					// than the first one that we merge the two clusters
					if (clustnums[matchcnt]<clustnums[prevcnt]) {
						// need to merge cluster clustnums[prevcnt] int cluster clustnums[matchcnt]
						//int oldnum = clustnums[prevcnt];
						for (int patchcnt=0; patchcnt<matchcnt; patchcnt++) {
							if (clustnums[patchcnt]==clustnums[prevcnt]) {
								clustnums[patchcnt] = clustnums[matchcnt];
							}
						}
						/*
						if (oldnum!=num_clusters-1) {
							// we created a hole in the cluster numbering - fix it.
							for (int patchcnt=0; patchcnt<matchcnt; patchcnt++) {
								if (clustnums[patchcnt]>oldnum) {
									clustnums[patchcnt]--;
								}
							}
						}
						num_clusters--;
						*/
					} else if (clustnums[prevcnt]<clustnums[matchcnt]) {
						// need to merge cluster clustnums[matchcnt] into cluster clustnums[prevcnt]
						//int oldnum = clustnums[matchcnt];
						for (int patchcnt=0; patchcnt<=matchcnt; patchcnt++) {
							if (clustnums[patchcnt]==clustnums[matchcnt]) {
								clustnums[patchcnt] = clustnums[prevcnt];
							}
						}
						/*
						if (oldnum!=num_clusters-1) {
							// we created a hole in the cluster numbering - fix it.
							for (int patchcnt=0; patchcnt<matchcnt; patchcnt++) {
								if (clustnums[patchcnt]>oldnum) {
									clustnums[patchcnt]--;
								}
							}
						}
						num_clusters--;
						*/
					} else {
						// have the same cluster number already
					}
				}
			}
		}
		if (clustnums[matchcnt]==-1) {
			clustnums[matchcnt] = num_clusters;
			num_clusters++;
		}
	}

	// patch holes in cluster numbering
	CIntVector elements;
	elements.resize(num_clusters);
	for (int e=0; e<num_clusters; e++) elements[e] = 0;
	for (int mcnt=0; mcnt<num_matches; mcnt++) {
		elements[clustnums[mcnt]]++;
	}
	for (int ccnt=0; ccnt<num_clusters; ccnt++) {
		if (elements[ccnt]==0) {
			if (ccnt+1<num_clusters) {
				for (int mcnt=0; mcnt<num_matches; mcnt++) {
					if (clustnums[mcnt]>ccnt) {
						clustnums[mcnt]--;
					} else {
						ASSERT(clustnums[mcnt]!=ccnt);
					}
				}
				elements[ccnt] = elements[ccnt+1];
				ccnt--;
			}
			num_clusters--;
		}
	}

	// calculate mean for each corner of each cluster, do this in
	// a single pass over the match vector
	ASSERT(num_clusters>0);
	int seen_clustnum = 0;
	for (int mtchcnt=1; mtchcnt<num_matches; mtchcnt++) {
		int clustnum = clustnums[mtchcnt];
		if (clustnum<=seen_clustnum) {
			posClsfd[clustnum].left += posClsfd[mtchcnt].left;
			posClsfd[clustnum].top += posClsfd[mtchcnt].top;
			posClsfd[clustnum].right += posClsfd[mtchcnt].right;
			posClsfd[clustnum].bottom += posClsfd[mtchcnt].bottom;
		} else {
			// who knows what's in clustnum! put this one in
			posClsfd[clustnum].left = posClsfd[mtchcnt].left;
			posClsfd[clustnum].top = posClsfd[mtchcnt].top;
			posClsfd[clustnum].right = posClsfd[mtchcnt].right;
			posClsfd[clustnum].bottom = posClsfd[mtchcnt].bottom;
			seen_clustnum = clustnum;
		}
	}
	posClsfd.resize(num_clusters);
	for (int clustcnt=0; clustcnt<num_clusters; clustcnt++) {
		posClsfd[clustcnt].left /= elements[clustcnt];
		posClsfd[clustcnt].top /= elements[clustcnt];
		posClsfd[clustcnt].right /= elements[clustcnt];
		posClsfd[clustcnt].bottom /= elements[clustcnt];
	}
}




void CImageScanner::InitScaleParams(const CClassifierCascade& cascade,
				    CScaleParams& params) const
{
  ASSERT(m_start_scale>=1.0);
  ASSERT(m_scale_inc_factor>1.0);
  ASSERT(m_translation_inc_x>=1);
  ASSERT(m_translation_inc_y>=1);

  // run cascade for each large enough sub-image
  params.template_width = cascade.GetTemplateWidth();
  params.template_height = cascade.GetTemplateHeight();
  double image_area_ratio = cascade.GetImageAreaRatio();

  // depending on the size ratio of the image area to test, stretch template
  // in height or width
  double template_ratio = (double)params.template_width/(double)params.template_height;
  if (image_area_ratio<template_ratio) { 
    // image area is narrower than template, stretch template in height
    params.scale_x = m_start_scale;
    params.scale_y = m_start_scale*template_ratio/image_area_ratio;
  } else { 
    // stretch template width
    params.scale_x = m_start_scale*image_area_ratio/template_ratio;
    params.scale_y = m_start_scale;
  }
  ASSERT(params.scale_x>=1.0);
  ASSERT(params.scale_y>=1.0);

  params.base_scale = m_start_scale;

  params.scaled_template_width = (int) (params.scale_x*params.template_width);
  params.scaled_template_height = (int) (params.scale_y*params.template_height);
  params.actual_scale_x = 
    (double) params.scaled_template_width/(double)params.template_width;
  params.actual_scale_y = 
    (double) params.scaled_template_height/(double)params.template_height;
  params.translation_inc_x = m_translation_inc_x;
  params.translation_inc_y = m_translation_inc_y;
}

void CImageScanner::NextScaleParams(CScaleParams& params) const
{
  // floating point scaling
  double old_actual_scale_x = params.actual_scale_x;
  double old_actual_scale_y = params.actual_scale_y;
  do {
    params.scale_x *= m_scale_inc_factor;
    params.scale_y *= m_scale_inc_factor;
    params.base_scale *= m_scale_inc_factor;
    params.translation_inc_x *= m_scale_inc_factor;
    params.translation_inc_y *= m_scale_inc_factor;
    params.scaled_template_width = (int) (params.scale_x*params.template_width);
    params.scaled_template_height = (int) (params.scale_y*params.template_height);
    params.actual_scale_x = 
      (double) params.scaled_template_width/(double)params.template_width;
    params.actual_scale_y = 
      (double) params.scaled_template_height/(double)params.template_height;
  } while (old_actual_scale_x==params.actual_scale_x 
	   || old_actual_scale_y==params.actual_scale_y);
}

ostream& operator<<(ostream& os, const CImageScanner& scanner)
{
  return scanner.output(os);
}

ostream& CImageScanner::output(ostream& os) const
{
  os << "start_scale " << m_start_scale
    << ", stop_scale " << m_stop_scale
    << ", scale_inc_factor " << m_scale_inc_factor
    << ", inc_x " << m_translation_inc_x
    << ", inc_y " << m_translation_inc_y << endl;
  os << "area: left " << m_scan_area.left
    << ", top " << m_scan_area.top
    << ", right " << m_scan_area.right
    << ", bottom " << m_scan_area.bottom << endl;
  os << (m_is_active? "active" : "inactive") << endl;
  return os;
}


