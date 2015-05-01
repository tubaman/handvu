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
  * $Id: OpticalFlow.cpp,v 1.20 2005/08/16 00:09:55 matz Exp $
**/

#include "Common.h"
#include "Skincolor.h"
#include "OpticalFlow.h"
#include "Exceptions.h"
#include <time.h>
#ifdef HAVE_FLOAT_H
#include <float.h>
#endif

#if defined(WIN32) && defined(DEBUG)
//#include <streams.h>
#endif

//
// Constructor
//
OpticalFlow::OpticalFlow()
  : m_num_features_tracked(0),
    m_recent_max_rdv(0),
    m_recent_max_rdv_decay(0),
    m_condens_is_tracking(false),
    m_pConDens(NULL),
    m_num_features_lost(0),
    m_winsize_width(-1),
    m_winsize_height(-1),
    m_min_distance(-1),
    m_pProbDistrProvider(NULL),
    m_num_pyramid_levels(3),
    m_target_num_features(-1),
    m_saved_prev_indx(-1),
    m_saved_curr_indx(-1),
    m_max_feature_error(-1),
    m_prev_buf_meaningful(false),
    m_prepared(false)
{
  m_pyramids[0] = NULL;
  m_pyramids[1] = NULL;
  m_tmpEVImage[0] = NULL;
  m_tmpEVImage[1] = NULL;
  m_mean_feature_pos.x = -1;
  m_mean_feature_pos.y = -1;

  srand((unsigned) time(NULL));
} // (Constructor)


OpticalFlow::~OpticalFlow()
{
  cvReleaseImage(&m_pyramids[0]);
  cvReleaseImage(&m_pyramids[1]);
  cvReleaseImage(&m_tmpEVImage[0]);
  cvReleaseImage(&m_tmpEVImage[1]);
  
  if (m_pConDens) {
    cvReleaseConDensation(&m_pConDens);
    m_pConDens = NULL;
  }
}

void OpticalFlow::Initialize(int width, int height)
{
  CvSize imgsize = cvSize(width, height);
  cvReleaseImage(&m_pyramids[0]);
  cvReleaseImage(&m_pyramids[1]);
  m_pyramids[0] = cvCreateImage(imgsize, IPL_DEPTH_8U, 1);
  m_pyramids[1] = cvCreateImage(imgsize, IPL_DEPTH_8U, 1);
  cvReleaseImage(&m_tmpEVImage[0]);
  cvReleaseImage(&m_tmpEVImage[1]);
  m_tmpEVImage[0] = cvCreateImage(imgsize, IPL_DEPTH_32F, 1);
  m_tmpEVImage[1] = cvCreateImage(imgsize, IPL_DEPTH_32F, 1);
}

#pragma warning (disable:4786)
void OpticalFlow::PrepareTracking(IplImage* rgbImage,
                                  IplImage* currGrayImage, int curr_indx,
                                  ProbDistrProvider* pProbProv,
                                  const CuScanMatch& match,
                                  ConstMaskIt mask,
                                  int target_num_features,
                                  int winsize_width,
                                  int winsize_height,
                                  double min_distance,
                                  double max_feature_error)
{
  m_pProbDistrProvider = pProbProv;
  m_target_num_features = target_num_features;
  m_num_features_tracked = 0;
  m_prev_buf_meaningful = false;
  m_winsize_width = winsize_width;
  m_winsize_height = winsize_height;
  m_min_distance = min_distance;
  m_max_feature_error = max_feature_error;
  
  // first find a big set of features that sits on corners
  int num_corners = m_target_num_features*3;
  CPointVector corners;
  corners.resize(num_corners);
  CRect bbox(match);
  FindGoodFeatures(currGrayImage, bbox, corners);

  // then play with the color probability distribution to pick
  // the ones that are on skin color, or if those aren't enough,
  // pick some additional ones on skin colored pixels
  m_features[0].resize(m_target_num_features);
  m_features[1].resize(m_target_num_features);
  m_feature_status.resize(m_target_num_features);
  m_errors.resize(m_target_num_features);
  PickSkinColoredFeatures(rgbImage, corners, m_features[curr_indx], match, mask);

  // fine-tune feature locations
  cvFindCornerSubPix(currGrayImage,
                     (CvPoint2D32f*) &m_features[curr_indx][0],
                     m_target_num_features, 
                     cvSize(5,5), cvSize(-1,-1),
                     cvTermCriteria( CV_TERMCRIT_ITER, 10, 0.1f ));

  // set status right for these features
  for (int i=0; i<m_target_num_features; i++) {
    m_feature_status[i] = 1;
  }
  GetAverage(m_features[curr_indx], m_mean_feature_pos);

  m_condens_is_tracking = false;
  m_condens_init_rect = CRect(match);

  m_prepared = true;
}
#pragma warning (default:4786)


int OpticalFlow::Track(IplImage* rgbImage,
                       IplImage* prevImage, IplImage* currImage,
                       int prev_indx, int curr_indx, 
                       int last_width, int last_height,
                       bool flock, bool use_prob_distr)
{
  ASSERT(m_prepared);

  m_prev_buf_meaningful = true;

  LKPyramids(prevImage, currImage, prev_indx, curr_indx);

#if 0
  if (todo) {
    /* track a number of KLT features with an n-stage pyramid
    * and globally optimize their positions with Condensation
    */
    const int condens_num_samples = 128;
    if (!m_condens_is_tracking) {
      InitCondensation(condens_num_samples);
      ASSERT(m_pConDens->SamplesNum==condens_num_samples);
      m_condens_is_tracking = true;
    }

    UpdateCondensation(rgbImage, prev_indx, curr_indx);
    ASSERT(m_pConDens->SamplesNum==condens_num_samples);
  }
#endif

  if (flock) {
    ConcentrateFeatures(rgbImage, m_features[curr_indx], m_feature_status, 
                        last_width, last_height, use_prob_distr);

  } else {
    AddNewFeatures(rgbImage, m_features[curr_indx], m_feature_status, 
                   last_width, last_height, use_prob_distr);
  }

  GetAverage(m_features[curr_indx], m_mean_feature_pos);
  m_saved_prev_indx = prev_indx;
  m_saved_curr_indx = curr_indx;

  return m_num_features_tracked;
} // Track


void OpticalFlow::DrawOverlay(IplImage* iplImage, int overlay_level)
{
  DrawFeaturesStatistics(iplImage, overlay_level);
}

void OpticalFlow::GetMeanFeaturePos(CvPoint2D32f& mean)
{
  mean = m_mean_feature_pos;
}


/* track a number of KLT features with an n-stage pyramid
*/
void OpticalFlow::LKPyramids(IplImage* prevImage, IplImage* currImage,
                             int prev_indx, int curr_indx)
{
  CvTermCriteria criteria =
    cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03);
  int flags = 0;
  if (m_num_features_tracked>0) {
    flags |= CV_LKFLOW_PYR_A_READY;
  }

  /* note: m_num_pyramid_levels can only be changed before the
   * playback is started.  To release this restriction, the m_pyramids
   * must be re-initialized, that is pyr0_ready set appropriately.
  */

  // in last frame, and possibly during Get/SetFeatures, how many of
  // them were lost?
  ASSERT((int)m_feature_status.size()>=m_target_num_features);
  if (m_target_num_features>0) {
    cvCalcOpticalFlowPyrLK(prevImage, // frame A
                           currImage,   // frame B
                           m_pyramids[prev_indx], // buffer for pyramid for A
                           m_pyramids[curr_indx], // buffer for pyramid for B
                           // feature points to track in A
                           (CvPoint2D32f*) &m_features[prev_indx][0], 
                           // calculated positions in B
                           (CvPoint2D32f*) &m_features[curr_indx][0],
                           // number of feature points to track
                           m_target_num_features,
                           // search window size per pyramid level
                           cvSize(m_winsize_width, m_winsize_height),
                           // max number of pyramid levels
                           m_num_pyramid_levels,
                           // array pos will be set to 1 if corresponding
                           // feature point was found
                           &m_feature_status[0],
                           // may be NULL, diff btw old
                           // and new area around features
                           &m_errors[0],
                           criteria, // iteration termination criteria
                           flags  // todo: put estimate, see Documentation
      );

    int count = m_num_features_tracked = m_target_num_features;
    for (int cnt1=0, k=0; cnt1<count; cnt1++) {
      if (m_feature_status[cnt1] && m_errors[cnt1]<m_max_feature_error) {
        m_features[prev_indx][k] = m_features[prev_indx][cnt1];
        m_features[curr_indx][k] = m_features[curr_indx][cnt1];
        k++;
      } else {
        m_feature_status[cnt1] = 0;
        m_num_features_tracked --;
      }
    }
  }
  m_num_features_lost = m_target_num_features-m_num_features_tracked;
}







void OpticalFlow::DrawFeaturesStatistics(IplImage* pImage, int overlay_level)
{
#if 0
  // for drawing statistics bars
  double height = (double)pImage->height/2.0;
  int xpos = 0;
  const int width = 20;
  const int space = 5;

  if (overlay_level>=3) {
    // num_features
    double goal = m_target_num_features;
    double rf = (goal-(double)m_num_features_lost)/goal;
    cvRectangle (pImage, cvPoint(xpos, pImage->height),
      cvPoint(xpos+width, pImage->height-(int)height),
      CV_RGB(255,0,0), 1);
    cvRectangle (pImage, cvPoint(xpos, pImage->height),
      cvPoint(xpos+width, pImage->height-(int)(height*rf)),
      CV_RGB(255,0,0), CV_FILLED);
    xpos += width+space;
  }
#endif

  const CvScalar black = CV_RGB(0, 0, 0);
  const CvScalar blue  = CV_RGB(255, 0, 0);
//  const CvScalar green = CV_RGB(0, 255, 0);				
  const CvScalar red   = CV_RGB(0, 0, 255);
  const CvScalar yellow= CV_RGB(0, 255, 255);
  const CvScalar white = CV_RGB(255, 255, 255);

  if (m_condens_is_tracking && overlay_level>=3) {
    // a circle at each sample location, its size indicating its confidence
    int num_samples = m_pConDens->SamplesNum;
    double min_conf = DBL_MAX;
    double max_conf = DBL_MIN;
    for (int scnt=0; scnt<num_samples; scnt++) {
      min_conf = min(min_conf, m_sample_confidences[scnt]);
      max_conf = max(max_conf, m_sample_confidences[scnt]);
    }
    double size_scale = 10.0/(max_conf-min_conf);
    for (int scnt=0; scnt<num_samples; scnt++) {
      int x = cvRound(m_pConDens->flSamples[scnt][0]);
      int y = cvRound(m_pConDens->flSamples[scnt][2]);
      int size = cvRound(1.0+(m_sample_confidences[scnt]-min_conf)*size_scale);
      cvCircle(pImage, cvPoint(x, y), size, yellow, 1); 
//      VERBOSE3(3, "%d: %f -> %d", scnt, m_sample_confidences[scnt], size);
    }
  }

  if (overlay_level>=3) {
    for (int cnt2 = 0; cnt2 < m_num_features_tracked; cnt2 ++) {
      // predicted location - blue
      if ((int)m_tmp_predicted.size()>cnt2) {
        int xp = cvRound(m_tmp_predicted[cnt2].x);
        int yp = cvRound(m_tmp_predicted[cnt2].y);
        cvCircle(pImage, cvPoint(xp, yp), 3, blue, CV_FILLED); 
      }
#if 0
      // old location - white
      if (false && m_prev_buf_meaningful) {
        int xo = cvRound(m_features[m_saved_prev_indx][cnt2].x);
        int yo = cvRound(m_features[m_saved_prev_indx][cnt2].y);
        cvCircle(pImage, cvPoint(xo, yo), 2, white, CV_FILLED); 
      }
#endif
      // observation - red
      if ((int)m_features_observation.size()>cnt2) {
        int xp = cvRound(m_features_observation[cnt2].x);
        int yp = cvRound(m_features_observation[cnt2].y);
        cvCircle(pImage, cvPoint(xp, yp), 2, red, 1); 
      }
    }
  }

  if (overlay_level>=2) {
    for (int cnt2 = 0; cnt2 < m_num_features_tracked; cnt2 ++) {
      // new location - green
      int xn = cvRound(m_features[m_saved_curr_indx][cnt2].x);
      int yn = cvRound(m_features[m_saved_curr_indx][cnt2].y);
      cvCircle(pImage, cvPoint(xn, yn), 5, black, CV_FILLED);
      cvCircle(pImage, cvPoint(xn, yn), 3, white, CV_FILLED);
    }
  }

#if 0
  if (m_prev_buf_meaningful && overlay_level>=3) {
    // also draw rose during following loop
    int rose_x = 100;
    int rose_y = pImage->height-100;
    int len_factor = 2;
    double dx_sum = 0;
    double dy_sum = 0;
    double vel_sqr_sum = 0;
    for (int ft=0; ft<m_num_features_tracked; ft++) {
      // current
      double x1 = m_features[m_saved_curr_indx][ft].x;
      double y1 = m_features[m_saved_curr_indx][ft].y;
      // previous frame
      double x2 = m_features[m_saved_prev_indx][ft].x;
      double y2 = m_features[m_saved_prev_indx][ft].y;
      // velocity
      double dx = x1-x2;
      double dy = y1-y2;
      dx_sum += dx;
      dy_sum += dy;
      double vel = sqrt(dx*dx+dy*dy);
      vel_sqr_sum += vel*vel;
      cvLine(pImage, 
             cvPoint(rose_x, rose_y),
             cvPoint(rose_x+(int)dx*len_factor, rose_y+(int)dy*len_factor),
             CV_RGB(255,255,255), 2);
    }

    double dx_mean = dx_sum/(double)m_num_features_tracked;
    double dy_mean = dy_sum/(double)m_num_features_tracked;
    double vel_mean = sqrt(dx_mean*dx_mean+dy_mean*dy_mean);
    double dir_mean;
    if (vel_mean>0) {
      dir_mean = atan(dy_mean/dx_mean);
      if (dx_mean<0) dir_mean += M_PI;
      if (dir_mean<0) dir_mean += 2.0*M_PI;
      if (dir_mean>=2.0*M_PI) dir_mean -= 2.0*M_PI;
    } else {
      dir_mean = 0;
    }
    ASSERT(0<=dir_mean && dir_mean<2.0*M_PI);
    
    double vel_stddev = 0;
    double dir_stddev = 0;
    for (int ft=0; ft<m_num_features_tracked; ft++) {
      // current
      double x1 = m_features[m_saved_curr_indx][ft].x;
      double y1 = m_features[m_saved_curr_indx][ft].y;
      // previous frame
      double x2 = m_features[m_saved_prev_indx][ft].x;
      double y2 = m_features[m_saved_prev_indx][ft].y;
      // velocity
      double dx = x1-x2;
      double dy = y1-y2;
      dx_sum += dx;
      dy_sum += dy;
      double vel = sqrt(dx*dx+dy*dy);
      vel_stddev += (vel-vel_mean)*(vel-vel_mean);
      double dir = atan(dy/dx);
      if (dx<0) dir += M_PI;
      if (dir<0) dir += 2.0*M_PI;
      if (dir>=2.0*M_PI) dir -= 2.0*M_PI;
      double dirdiff = min(fabs(dir-dir_mean), fabs(dir-2.0*M_PI-dir_mean));
      dirdiff = min(dirdiff, fabs(dir+2.0*M_PI-dir_mean));
      dir_stddev += dirdiff*dirdiff;
    }
    vel_stddev /= (double)m_num_features_tracked;
    vel_stddev = sqrt(vel_stddev);
    dir_stddev /= (double)m_num_features_tracked;
    dir_stddev = sqrt(dir_stddev);
    
    // mean velocity and direction
    cvLine(pImage, 
           cvPoint(rose_x, rose_y), 
           cvPoint(rose_x+(int)dx_mean*len_factor,
                   rose_y+(int)dy_mean*len_factor),
           CV_RGB(255,0,0), 8);
    // +/- stddev velocity
    double vel_stddev_x = cos(dir_mean)*vel_stddev;
    double vel_stddev_y = sin(dir_mean)*vel_stddev;
    cvLine(pImage, 
           cvPoint(rose_x+(int)(dx_mean-vel_stddev_x)*len_factor,
                   rose_y+(int)(dy_mean-vel_stddev_y)*len_factor),
           cvPoint(rose_x+(int)(dx_mean+vel_stddev_x)*len_factor,
                   rose_y+(int)(dy_mean+vel_stddev_y)*len_factor),
           CV_RGB(255,255,0), 2);
    cvCircle( pImage, cvPoint(rose_x, rose_y), 30, CV_RGB(255,0,0), 1 );
    
    // bars for direction stddev
    /*
      double rd = dir_stddev/2*M_PI;
      cvRectangle (pImage, cvPoint(xpos, pImage->height),
      cvPoint(xpos+width, pImage->height-(int)height), CV_RGB(0,255,255), 1);
      cvRectangle (pImage, cvPoint(xpos, pImage->height),
      cvPoint(xpos+width, pImage->height-(int)(height*rd)),
      CV_RGB(0,255,255), CV_FILLED);
      xpos += width+space;
    */
    // bars for direction stddev multiplied with velocity
    if (m_recent_max_rdv_decay>0) {
      // draw peak bar
      m_recent_max_rdv_decay--;
      if (m_recent_max_rdv_decay==0) {
        m_recent_max_rdv = 0;
      } else {
        cvRectangle (pImage, cvPoint(xpos, pImage->height), 
                     cvPoint(xpos+width,
                             pImage->height-(int)(height*m_recent_max_rdv)),
                     CV_RGB(0,0,255), CV_FILLED);
      }
    }
    double rdv = vel_mean*dir_stddev/(2*M_PI*5);
    if (rdv>m_recent_max_rdv) {
      m_recent_max_rdv = rdv;
      m_recent_max_rdv_decay = 10;
    }
    cvRectangle (pImage, cvPoint(xpos, pImage->height),
                 cvPoint(xpos+width, pImage->height-(int)height),
                 CV_RGB(0,255,255), 1);
    cvRectangle (pImage, cvPoint(xpos, pImage->height),
                 cvPoint(xpos+width, pImage->height-(int)(height*rdv)),
                 CV_RGB(0,255,255), CV_FILLED);
    xpos += width+space;
  }
#endif
}


/** find good features to track
 */
void OpticalFlow::FindGoodFeatures(IplImage* grayImage, const CRect& bbox,
                                   CPointVector& corners) {
  double quality = 0.1; 
  //only those corners are selected, which minimal eigen value is
  //non-less than maximum of minimal eigen values on the image,
  //multiplied by quality_level. For example, quality_level = 0.1
  //means that selected corners must be at least 1/10 as good as
  //the best corner.

  int count = (int) corners.size();
  int width = min(grayImage->width, bbox.right-bbox.left);
  int height = min(grayImage->height, bbox.bottom-bbox.top);
  CvRect cvBox = cvRect(bbox.left, bbox.top, width, height);
  cvSetImageROI(grayImage, cvBox);
  cvSetImageROI(m_tmpEVImage[0], cvBox);
  cvSetImageROI(m_tmpEVImage[1], cvBox);
  cvGoodFeaturesToTrack(grayImage, m_tmpEVImage[0], m_tmpEVImage[1],
    (CvPoint2D32f*) &corners[0], &count, quality, m_min_distance);
  ASSERT(count); // we want at least SOME features!
  cvResetImageROI(grayImage);
  cvResetImageROI(m_tmpEVImage[0]);
  cvResetImageROI(m_tmpEVImage[1]);

  corners.resize(count);

  // their locations are relative to the bbox origin - translate them
  for (int cnt=0; cnt<count; cnt++) {
    corners[cnt].x += bbox.left;
    corners[cnt].y += bbox.top;
  }
}

#define MIN_PROB_FOREGROUND 0.5

/* pick features on foreground color
 */
#pragma warning (disable:4786)
void OpticalFlow::PickSkinColoredFeatures(IplImage* rgbImage,
                                          const CPointVector& corners,
                                          CPointVector& features,
                                          const CuScanMatch& match,
                                          ConstMaskIt mask)
{
  ASSERT(m_pProbDistrProvider);
  
  int num_features = (int) features.size();
  int poolsize = (int) corners.size();
  CDoubleVector corner_probs(poolsize);
  int width = match.right-match.left;
  int height = match.bottom-match.top;
  double scale_x = (double)(width-1)/((*mask).second.GetWidth()-1);
  double scale_y = (double)(height-1)/((*mask).second.GetHeight()-1);

  // create a vector with probabilities, combined of observed color and mask
  for (int pp=0; pp<poolsize; pp++) {
    ColorBGR* color;
    GetPixel(rgbImage, (int) corners[pp].x, (int) corners[pp].y, &color);
    double color_prob = m_pProbDistrProvider->LookupProb(*color);
    int m_x = cvRound((corners[pp].x-match.left)/scale_x);
    int m_y = cvRound((corners[pp].y-match.top)/scale_y);
    double loc_prob = (*mask).second.GetProb(m_x, m_y);
    corner_probs[pp] = color_prob*loc_prob;
  }

  // for each feature, find the most skin-colored corner and pick it
  for (int fcnt=0; fcnt<num_features; fcnt++) {
    double max_prob = -DBL_MAX;
    int max_corner_indx = -1;
    for (int pp=0; pp<poolsize; pp++) {
      if (corner_probs[pp]>max_prob) {
        max_prob = corner_probs[pp];
        max_corner_indx = pp;
      }
    }
    ASSERT(max_prob>=.0);
    if (max_prob>=MIN_PROB_FOREGROUND) {
      // found a (the best!) corner with skin color
      features[fcnt].x = (float)corners[max_corner_indx].x;
      features[fcnt].y = (float)corners[max_corner_indx].y;
      corner_probs[max_corner_indx] = .0;

    } else {
      // pick our own random location, somewhere with skin color
      // and far away enough from the other features if possible
      int x, y;
      double prob;
      int cnt = 0;
      bool too_close;
      // try to find a location that was segmented as foreground color
      do {
        x = match.left+(rand()%width);
        y = match.top+(rand()%height);
        x = min(x, rgbImage->width-1);
        y = min(y, rgbImage->height-1);
        x = max(x, 0);
        y = max(y, 0);
        ColorBGR* color;
        GetPixel(rgbImage, x, y, &color);
        double color_prob = m_pProbDistrProvider->LookupProb(*color);
        int m_x = cvRound((double)(x-match.left)/scale_x);
        int m_y = cvRound((double)(y-match.top)/scale_y);
        double loc_prob = (*mask).second.GetProb(m_x, m_y);
        prob = color_prob*loc_prob;

        too_close = TooCloseToOthers(features, fcnt, fcnt);

        cnt++;
      } while ((prob<MIN_PROB_FOREGROUND || too_close) && cnt<20);
      features[fcnt].x = (float)x;
      features[fcnt].y = (float)y;
    
    }
  }
}
#pragma warning (default:4786)


/*
* find feature that has the minimum
* cumulative distance from all other features;
* will discard the discard_num_furthest distances in the computation
*/
int OpticalFlow::FindCentroid(const CDoubleMatrix& distances,
                              int discard_num_furthest)
{
  double min_cum_dist = DBL_MAX;
  int min_cum_dist_indx = -1;
  int num_elems = (int)distances.size();
  ASSERT(num_elems>discard_num_furthest);
  vector<double> furthest; // will be sorted highest to smallest
  furthest.resize(discard_num_furthest);
  for (int icnt1=0; icnt1<num_elems; icnt1++) {
    double cum_dist=0;
    for (int f=0; f<discard_num_furthest; f++) furthest[f]=0;
    for (int icnt2=0; icnt2<num_elems; icnt2++) {
      double dist;
      if (icnt1<icnt2) {
        dist = distances[icnt1][icnt2];
      } else if (icnt2<icnt1) {
        dist = distances[icnt2][icnt1];
      } else {
        ASSERT(icnt1==icnt2);
        continue;
      }
      // check if it's a far-away feature, update "furthest" vector if so.
      // only add the distance if it's not too far away
      for (int f=0; f<discard_num_furthest; f++) {
        if (dist>furthest[f]) {
          // add smallest "furthest" dist to the sum before we kick it out
          // of the vector
          cum_dist += furthest[discard_num_furthest-1];
          for (int s=f; s<discard_num_furthest-1; s++) {
            furthest[s+1] = furthest[s];
          }
          furthest[f] = dist;
          break;
        }
      }
      cum_dist += dist;
    }
    if (cum_dist<min_cum_dist) {
      min_cum_dist = cum_dist;
      min_cum_dist_indx = icnt1;
    }
  }
  return min_cum_dist_indx;
}

/*
* find average of feature locations
*/
void OpticalFlow::GetAverage(const CPointVector& features, CvPoint2D32f& avg)
{
  double sum_x = 0;
  double sum_y = 0;
  int num_elems = (int) features.size();
  for (int icnt=0; icnt<num_elems; icnt++) {
    sum_x += features[icnt].x;
    sum_y += features[icnt].y;
  }
  avg.x = (float)(sum_x/(double)num_elems);
  avg.y = (float)(sum_y/(double)num_elems);
}

/* fill in half of the matrix with the distance measures;
* only the upper half is filled in, i.e. if row<col there's a value
*/
void OpticalFlow::DistanceMatrix(const CPointVector& features,
                                 CDoubleMatrix& distances)
{
  int num_features = (int)features.size();
  distances.resize(num_features);
  for (int row=0; row<num_features; row++) {
    distances[row].resize(num_features);
    for (int col=row+1; col<num_features; col++) {
      double dx = features[col].x-features[row].x;
      double dy = features[col].y-features[row].y;
      distances[row][col] = sqrt(dx*dx+dy*dy);
    }
  }
}

bool OpticalFlow::TooCloseToOthers(const CPointVector& features, int indx, int len)
{
  for (int fcnt=0; fcnt<len; fcnt++) {
    if (indx==fcnt) continue;
    double dx = features[fcnt].x-features[indx].x;
    double dy = features[fcnt].y-features[indx].y;
    double dist = sqrt(dx*dx+dy*dy);
    if (dist<m_min_distance) {
      return true;
    }
  }
  return false;
}

/* randomly re-place one of two points that are too close to each other,
* move others that are too far from the centroid towards the centroid
*/
void OpticalFlow::ConcentrateFeatures(IplImage* rgbImage,
                                      CPointVector& features,
                                      CStatusVector& status,
                                      int last_width, int last_height,
                                      bool use_prob_distr)
{
  ASSERT(m_pProbDistrProvider);

  int round_or_square = 2; // the area in which we will concentrate the features, 1 is round, 2 is rectangular
  CDoubleMatrix distances;
  DistanceMatrix(features, distances);
  // discard the highest 15percent of distances when computing centroid
  int discard_num_distances = (int)(0.15*(double)distances.size());
  int centroid_index = FindCentroid(distances, discard_num_distances);
  CvPoint2D32f centroid = features[centroid_index];
  double halfwidth = last_width/2.0;
  double halfheight = last_height/2.0;
  double left = centroid.x-halfwidth;
  double right = centroid.x+halfwidth;
  double top = centroid.y-halfheight;
  double bottom = centroid.y+halfheight;
  int num_features = (int)features.size();

  for (int fcnt=0; fcnt<num_features; fcnt++) {
    m_feature_status[fcnt] = 1;
    for (int scnd=fcnt+1; scnd<num_features; scnd++) {
      double dist;
      if (fcnt<scnd) {
        dist = distances[fcnt][scnd];
      } else { // scnd<fcnt
        dist = distances[scnd][fcnt];
      }
      int cnt=0;
      bool too_close = dist<m_min_distance;
      while (too_close && cnt<20) {
        // try to find a location that was segmented as foreground color
        // and one that's far away enough from all other features
        int x, y;
        double prob = 1.0;
        int prob_cnt = 0;
        do {
          x = (int) (centroid.x-last_width/2.0+(rand()%last_width));
          y = (int) (centroid.y-last_height/2.0+(rand()%last_height));
          x = min(x, rgbImage->width-1);
          y = min(y, rgbImage->height-1);
          x = max(x, 0);
          y = max(y, 0);
          ColorBGR* color;
          GetPixel(rgbImage, x, y, &color);
          if (use_prob_distr) {
            prob = m_pProbDistrProvider->LookupProb(*color);
          }
          prob_cnt++;
        } while (prob<MIN_PROB_FOREGROUND && prob_cnt<20);
        features[fcnt].x = (float)x;
        features[fcnt].y = (float)y;
        status[fcnt] = 0;

        too_close = TooCloseToOthers(features, fcnt, (int)features.size());

        cnt++;
      } // end while
    } // end for scnd

    if (centroid_index!=fcnt) {
      if (round_or_square==1) {
        // round
        double dist = -1;
        if (fcnt<centroid_index) {
          dist = distances[fcnt][centroid_index];
        } else if (centroid_index<fcnt) {
          dist = distances[centroid_index][fcnt];
        } else {
          // will not enter the following loop
        }
        while (dist>last_width/2.0) {
          features[fcnt].x = (float)((features[fcnt].x+centroid.x)/2.0);
          features[fcnt].y = (float)((features[fcnt].y+centroid.y)/2.0);
          m_feature_status[fcnt] = 0;
          double dx = features[fcnt].x-centroid.x;
          double dy = features[fcnt].y-centroid.y;
          dist = sqrt(dx*dx+dy*dy);;
        }
      } else {
        // rectangular area
        // move towards horizontal center
        while (features[fcnt].x<left || features[fcnt].x>right) {
          features[fcnt].x = (float)((features[fcnt].x+centroid.x)/2.0);
          status[fcnt] = 0;
        }
        // move towards vertical center
        while (features[fcnt].y<top || features[fcnt].y>bottom) {
          features[fcnt].y = (float)((features[fcnt].y+centroid.y)/2.0);
          status[fcnt] = 0;
        }
      }
    } // end centroid_index!=fcnt

    features[fcnt].x = min(max(features[fcnt].x, .0f), rgbImage->width-1.0f);
    features[fcnt].y = min(max(features[fcnt].y, .0f), rgbImage->height-1.0f);
  }
}

/* randomly re-place one of two points that are too close to each other,
* move others that are too far from the centroid towards the centroid
*/
void OpticalFlow::AddNewFeatures(IplImage* rgbImage,
                                 CPointVector& features,
                                 CStatusVector& status,
                                 int last_width, int last_height,
                                 bool use_prob_distr)
{
  ASSERT(m_pProbDistrProvider);

  CDoubleMatrix distances;
  DistanceMatrix(features, distances);
  // discard the highest 15percent of distances when computing centroid
  int discard_num_distances = (int)(0.15*(double)distances.size());
  int centroid_index = FindCentroid(distances, discard_num_distances);
  CvPoint2D32f centroid = features[centroid_index];
//  double halfwidth = last_width/2.0;
//  double halfheight = last_height/2.0;
//  double left = centroid.x-halfwidth;
//  double right = centroid.x+halfwidth;
//  double top = centroid.y-halfheight;
//  double bottom = centroid.y+halfheight;

  for (int fcnt=m_num_features_tracked; fcnt<m_target_num_features; fcnt++) {
    // try to find a location that was segmented as foreground color
    int x, y;
    double prob = 1.0;
    int prob_cnt = 0;
    do {
      x = (int) (centroid.x-last_width/2.0+(rand()%last_width));
      y = (int) (centroid.y-last_height/2.0+(rand()%last_height));
      x = min(x, rgbImage->width-1);
      y = min(y, rgbImage->height-1);
      x = max(x, 0);
      y = max(y, 0);
      ColorBGR* color;
      GetPixel(rgbImage, x, y, &color);
      if (use_prob_distr) {
        prob = m_pProbDistrProvider->LookupProb(*color);
      }
      prob_cnt++;
    } while (prob<MIN_PROB_FOREGROUND && prob_cnt<20);

    features[fcnt].x = (float)x;
    features[fcnt].y = (float)y;
    status[fcnt] = 0;
  } // end for fcnt
}

void OpticalFlow::GetPixel(IplImage* rgbImage, int x, int y, ColorBGR** color)
{
  ASSERT(rgbImage->nChannels==3);
  *color = (ColorBGR*) cvPtr2D(rgbImage, y, x);
}


