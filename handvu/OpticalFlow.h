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
  * $Id: OpticalFlow.h,v 1.9 2005/08/16 00:09:55 matz Exp $
**/

#if !defined(__OPTICAL_FLOW_H__INCLUDED_)
#define __OPTICAL_FLOW_H__INCLUDED_

#include <cv.h>
#include <vector>
using namespace std;

#include "Mask.h"
#include "ProbDistrProvider.h"


typedef vector<CvPoint2D32f> CPointVector;
typedef vector<char> CStatusVector;
typedef vector<double> CDoubleVector;
typedef vector<CDoubleVector> CDoubleMatrix;
typedef vector<float> CFloatVector;


typedef struct _CondensState {
  double x, y;
  double vx, vy;
  double angle;
} CondensState;

#pragma warning (disable:4786)

class OpticalFlow {
 public:
  OpticalFlow();
  ~OpticalFlow();

 public:
  void Initialize(int width, int height);
  void PrepareTracking(IplImage* rgbImage,
                       IplImage* currGrayImage, int curr_indx,
                       ProbDistrProvider* pProbProv,
                       const CuScanMatch& match,
                       ConstMaskIt mask,
                       int target_num_features,
                       int winsize_width,
                       int winsize_height,
                       double min_distance,
                       double max_feature_error);
  int Track(IplImage* rgbImage,
            IplImage* prevImage, IplImage* currImage,
            int prev_indx, int curr_indx,
            int last_width, int last_height,
            bool flock, bool use_prob_distr);
  void DrawOverlay(IplImage* iplImage, int overlay_level);
  void GetMeanFeaturePos(CvPoint2D32f& mean);


 protected:
  void LKPyramids(IplImage* prevImage, IplImage* currImage,
                  int prev_indx, int curr_indx);
  void DrawFeaturesStatistics(IplImage* pImage, int overlay_level);
  void FindGoodFeatures(IplImage* grayImage, const CRect& bbox,
                        CPointVector& corners);
  void PickSkinColoredFeatures(IplImage* rgbImage,
                               const CPointVector& corners,
                               CPointVector& features,
                               const CuScanMatch& match,
                               ConstMaskIt mask);
  int FindCentroid(const CDoubleMatrix& distances,
                   int discard_num_furthest);
  void GetAverage(const CPointVector& features, CvPoint2D32f& avg);
  void DistanceMatrix(const CPointVector& features,
                      CDoubleMatrix& distances);
  void ConcentrateFeatures(IplImage* rgbImage,
                           CPointVector& features,
                           CStatusVector& status,
                           int last_width, int last_height,
                           bool use_prob_distr);
  void AddNewFeatures(IplImage* rgbImage,
                           CPointVector& features,
                           CStatusVector& status,
                           int last_width, int last_height,
                           bool use_prob_distr);
  bool TooCloseToOthers(const CPointVector& features, int indx, int len);
  void GetPixel(IplImage* rgbImage, int x, int y, ColorBGR** color);

  // Condens
#define OF_CONDENS_DIMS 5
  void InitCondensation(int condens_num_samples);
  void UpdateCondensation(IplImage* rgbImage, 
    int prev_indx, int curr_indx);
  void FollowObservationForSmallDiffs(const CPointVector& pred, 
                                                 const CPointVector& obs, 
                                                 CPointVector& corrected,
                                                 double diff);
  void PreparePredictFeatureLocations(const CondensState& base_state,
                                                 const CPointVector& base,
                                                 CDoubleVector& old_lens,
                                                 CDoubleVector& old_d_angles);
  void PredictFeatureLocations(const CDoubleVector& old_lens,
                                          const CDoubleVector& old_d_angles,
                                          const CondensState& predicted_state,
                                          CPointVector& prediction);
  double EstimateProbability(const CPointVector& prediction, 
                                        const CPointVector& observation,
                                        int discard_num_furthest);
  double EstimateProbability(const CPointVector& prediction,
                                        IplImage* rgbImage);
  

 protected:
  int                        m_num_features_tracked;
  double                     m_recent_max_rdv;
  int                        m_recent_max_rdv_decay;
  int                        m_num_features_lost;
  int                        m_winsize_width;
  int                        m_winsize_height;
  double                     m_min_distance;
  double                     m_max_feature_error;
  ProbDistrProvider*         m_pProbDistrProvider;
  int                        m_num_pyramid_levels;
  int                        m_target_num_features;
  bool                       m_prev_buf_meaningful;
  IplImage*                  m_pyramids[2];
  IplImage*                  m_tmpEVImage[2];
  CPointVector               m_features[2];
  CStatusVector              m_feature_status;
  CFloatVector               m_errors;
  CvPoint2D32f               m_mean_feature_pos;
  int                        m_saved_prev_indx;
  int                        m_saved_curr_indx;
  bool                       m_prepared;

  // condensation tracking of features
  bool                       m_condens_is_tracking;
  CvConDensation*            m_pConDens;
  CondensState               m_condens_state;
  CPointVector               m_tmp_predicted;
  CPointVector               m_features_observation;
  CDoubleVector              m_sample_confidences;
  CRect                      m_condens_init_rect;
};


#pragma warning (default:4786)



#endif // __OPTICAL_FLOW_H__INCLUDED_


