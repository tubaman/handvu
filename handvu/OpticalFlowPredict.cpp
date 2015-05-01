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
  * $Id: OpticalFlowPredict.cpp,v 1.10 2005/08/16 00:09:55 matz Exp $
**/

#include "Common.h"
#include "Skincolor.h"
#include "OpticalFlow.h"
#include "Exceptions.h"


/** Initialize the Condensation data structure and state dynamics
*/
void OpticalFlow::InitCondensation(int condens_num_samples)
{
  // initialize Condensation data structure and set the
  // system dynamics
  m_sample_confidences.resize(condens_num_samples);
  if (m_pConDens) {
    cvReleaseConDensation(&m_pConDens);
  }
  m_pConDens = cvCreateConDensation(OF_CONDENS_DIMS, OF_CONDENS_DIMS, condens_num_samples);
  CvMat dyn = cvMat(OF_CONDENS_DIMS, OF_CONDENS_DIMS, CV_32FC1, m_pConDens->DynamMatr);
//  CvMat dyn = cvMat(OF_CONDENS_DIMS, OF_CONDENS_DIMS, CV_MAT3x3_32F, m_pConDens->DynamMatr);
  cvmSetIdentity(&dyn);
  cvmSet(&dyn, 0, 1, 0.0);
  cvmSet(&dyn, 2, 3, 0.0);

  // initialize bounds for state
  float lower_bound[OF_CONDENS_DIMS];
  float upper_bound[OF_CONDENS_DIMS];
  // velocity bounds highly depend on the frame rate that we will achieve,
  // increase the factor for lower frame rates;
  // it states how much the center can move in either direction in a single
  // frame, measured in terms of the width or height of the initial match size
  double velocity_factor = .25; 
  double cx = (m_condens_init_rect.left+m_condens_init_rect.right)/2.0;
  double cy = (m_condens_init_rect.top+m_condens_init_rect.bottom)/2.0;
  double width = (m_condens_init_rect.right-m_condens_init_rect.left)*velocity_factor;
  double height = (m_condens_init_rect.bottom-m_condens_init_rect.top)*velocity_factor;
  lower_bound[0] = (float) (cx-width);
  upper_bound[0] = (float) (cx+width);
  lower_bound[1] = (float) (-width);
  upper_bound[1] = (float) (+width);
  lower_bound[2] = (float) (cy-height);
  upper_bound[2] = (float) (cy+height);
  lower_bound[3] = (float) (-height);
  upper_bound[3] = (float) (+height);
  lower_bound[4] = (float) (-10.0*velocity_factor*M_PI/180.0);
  upper_bound[4] = (float) (+10.0*velocity_factor*M_PI/180.0);
  CvMat lb = cvMat(OF_CONDENS_DIMS, 1, CV_MAT3x1_32F, lower_bound);
  CvMat ub = cvMat(OF_CONDENS_DIMS, 1, CV_MAT3x1_32F, upper_bound);
  cvConDensInitSampleSet(m_pConDens, &lb, &ub);

  // set the state that will later be computed by condensation to
  // the currently observed state
  m_condens_state.x = cx;
  m_condens_state.y = cy;
  m_condens_state.vx = 0;
  m_condens_state.vy = 0;
  m_condens_state.angle = 0;

  // debugging:
//  DbgSetModuleLevel(LOG_CUSTOM1, 3);
}




void OpticalFlow::UpdateCondensation(IplImage* /*rgbImage*/,
                                     int prev_indx, int curr_indx)
{
  //VERBOSE5(3, "m_condens_state x %f, y %f, vx %f, vy %f, a %f", 
  //  m_condens_state.x, m_condens_state.y, m_condens_state.vx, m_condens_state.vy, m_condens_state.angle);

  // for each condensation sample, predict the feature locations,
  // compare to the observed KLT tracking, and check the probmask
  // at each predicted location. The combination of these yields the
  // confidence in this sample's estimate.
  int num_ft = (int) m_features[prev_indx].size();
  CPointVector predicted;
  predicted.resize(num_ft);
  CDoubleVector probs_locations;
  CDoubleVector probs_colors;
  probs_locations.reserve(m_pConDens->SamplesNum);
  probs_colors.reserve(m_pConDens->SamplesNum);
  double sum_probs_locations = 0.0;
  double sum_probs_colors = 0.0;
  CDoubleVector old_lens;
  CDoubleVector old_d_angles;
  // prepare data structures so that prediction based on centroid
  // is fast
  PreparePredictFeatureLocations(m_condens_state, m_features[prev_indx], old_lens, old_d_angles);
  CvPoint2D32f avg_obs, avg_prev;
  GetAverage(m_features[curr_indx], avg_prev);
//  GetAverage(m_features[prev_indx], avg_prev);
  GetAverage(m_features[curr_indx]/*_observation*/, avg_obs);
  double dvx = avg_obs.x - avg_prev.x;
  double dvy = avg_obs.y - avg_prev.y;

  // for each sample
  for (int scnt=0; scnt<m_pConDens->SamplesNum; scnt++) {
    // hack - todo
    if (scnt==m_pConDens->SamplesNum-1) {
      m_pConDens->flSamples[scnt][0] = avg_obs.x;
      m_pConDens->flSamples[scnt][2] = avg_obs.y;
      m_pConDens->flSamples[scnt][1] = (float) dvx;
      m_pConDens->flSamples[scnt][3] = (float) dvy;
    }

    // the Condensation sample's guess:
    CondensState sample_state;
    sample_state.x = m_pConDens->flSamples[scnt][0];
    sample_state.y = m_pConDens->flSamples[scnt][2];
    sample_state.vx = m_pConDens->flSamples[scnt][1];
    sample_state.vy = m_pConDens->flSamples[scnt][3];
    sample_state.angle = 0;//m_pConDens->flSamples[scnt][4];
    ASSERT(!isnan(sample_state.x) && !isnan(sample_state.y) && !isnan(sample_state.angle));
    
    double fac = (m_condens_init_rect.right-m_condens_init_rect.left)/3.0;
    double dx = avg_obs.x - sample_state.x;
    double dy = avg_obs.y - sample_state.y;
    double probloc = dx*dx+dy*dy;
    probloc = fac/(fac+probloc);
    probs_locations.push_back(probloc);
    sum_probs_locations += probloc;

#if 0
    PredictFeatureLocations(old_lens, old_d_angles, sample_state, predicted);

    // probability of predicted feature locations given the KLT observation
    int discard_num_distances = (int)(0.15*(double)num_ft);
    double probloc = EstimateProbability(predicted, m_features[curr_indx]/*_observation*/, discard_num_distances);
    probs_locations.push_back(probloc);
    sum_probs_locations += probloc;

    // probability of predicted feature locations given the outside probability map (color)
    double probcol = EstimateProbability(predicted, rgbImage);
    probs_colors.push_back(probcol);
    sum_probs_colors += probcol;
#endif

  } // end for each sample

//  ASSERT(!isnan(sum_probs_locations) && sum_probs_locations>0);

  //
  // normalize the individual probabilities and set sample confidence
  //
  int best_sample_indx = -1;
  double best_confidence = 0;
  for (int scnt=0; scnt<m_pConDens->SamplesNum; scnt++) {
    double norm_prob_locations = probs_locations[scnt]/sum_probs_locations;
//    double norm_prob_colors    = probs_colors[scnt]/sum_probs_colors;
    double confidence;
    if (sum_probs_colors>0) {
 //     confidence = norm_prob_locations*norm_prob_colors;
      confidence = norm_prob_locations;
    } else {
      confidence = norm_prob_locations;
    }
    m_pConDens->flConfidence[scnt] = (float) confidence;
    m_sample_confidences[scnt] = confidence;
    if (confidence>best_confidence) {
      best_confidence = confidence;
      best_sample_indx = scnt;
    }
  }
//  for (int scnt=0; scnt<m_pConDens->SamplesNum; scnt++) {
//    VERBOSE2(3, "%d: %f ", scnt, m_sample_confidences[scnt]);
//  }
  ASSERT(best_sample_indx!=-1);

  ASSERT(best_sample_indx==m_pConDens->SamplesNum-1);
 
  CondensState best_sample_state;
  best_sample_state.x = m_pConDens->flSamples[best_sample_indx][0];
  best_sample_state.y = m_pConDens->flSamples[best_sample_indx][2];
  best_sample_state.vx = m_pConDens->flSamples[best_sample_indx][1];
  best_sample_state.vy = m_pConDens->flSamples[best_sample_indx][3];
  best_sample_state.angle = m_pConDens->flSamples[best_sample_indx][4];
  //VERBOSE3(3, "sample_state %f, %f, %f", 
  // sample_state.x, sample_state.y, sample_state.angle);
  //    VERBOSE4(3, "sample_state %f, %f, %f, %f"), 
  //      sample_state.x, sample_state.y, sample_state.vx, sample_state.vy);
  ASSERT(!isnan(best_sample_state.x) && !isnan(best_sample_state.y) && !isnan(best_sample_state.angle));

  // probability of predicted feature locations given the KLT observation
  m_tmp_predicted.resize(m_features[0].size());
  PredictFeatureLocations(old_lens, old_d_angles, best_sample_state, m_tmp_predicted);

  //
  // do one condensation step, then get the state prediction from Condensation;
  //
  cvConDensUpdateByTime(m_pConDens);

#if 0
  if (false) { // todo
    m_condens_state.x = max(0, min(rgbImage->width-1, m_pConDens->State[0]));
    m_condens_state.y = max(0, min(rgbImage->height-1, m_pConDens->State[2]));
    m_condens_state.vx = m_pConDens->State[1];
    m_condens_state.vy = m_pConDens->State[3];
    m_condens_state.angle = m_pConDens->State[4];
  } else 
#endif
  {
    m_condens_state.x = best_sample_state.x;
    m_condens_state.y = best_sample_state.y;
    m_condens_state.vx = best_sample_state.vx;
    m_condens_state.vy = best_sample_state.vy ;
    m_condens_state.angle = best_sample_state.angle;
  }
  ASSERT(!isnan(m_condens_state.x) && !isnan(m_condens_state.y) && !isnan(m_condens_state.angle));
  ASSERT(!isnan(m_condens_state.vx) && !isnan(m_condens_state.vy));

  // now move the current features to where Condensation thinks they should be;
  // the observation is no longer needed
#if 0
  if (false) { // todo
    PredictFeatureLocations(old_lens, old_d_angles, m_condens_state, m_tmp_predicted);
    FollowObservationForSmallDiffs(m_tmp_predicted, m_features[curr_indx]/*observation*/, 
                                  m_features[curr_indx]/*output*/, 2.0);
  } else 
#endif
  {
    PredictFeatureLocations(old_lens, old_d_angles, m_condens_state, m_features[curr_indx]);
  }

  {
    // initialize bounds for state
    float lower_bound[OF_CONDENS_DIMS];
    float upper_bound[OF_CONDENS_DIMS];
    // velocity bounds highly depend on the frame rate that we will achieve,
    // increase the factor for lower frame rates;
    // it states how much the center can move in either direction in a single
    // frame, measured in terms of the width or height of the initial match size
    double velocity_factor = .25; 
    CvPoint2D32f avg;
    GetAverage(m_features[curr_indx]/*_observation*/, avg);
    double cx = avg.x;
    double cy = avg.y;
    double width = (m_condens_init_rect.right-m_condens_init_rect.left)*velocity_factor;
    double height = (m_condens_init_rect.bottom-m_condens_init_rect.top)*velocity_factor;
    lower_bound[0] = (float) (cx-width);
    upper_bound[0] = (float) (cx+width);
    lower_bound[1] = (float) (-width);
    upper_bound[1] = (float) (+width);
    lower_bound[2] = (float) (cy-height);
    upper_bound[2] = (float) (cy+height);
    lower_bound[3] = (float) (-height);
    upper_bound[3] = (float) (+height);
    lower_bound[4] = (float) (-10.0*velocity_factor*M_PI/180.0);
    upper_bound[4] = (float) (+10.0*velocity_factor*M_PI/180.0);
    CvMat lb = cvMat(OF_CONDENS_DIMS, 1, CV_MAT3x1_32F, lower_bound);
    CvMat ub = cvMat(OF_CONDENS_DIMS, 1, CV_MAT3x1_32F, upper_bound);
    cvConDensInitSampleSet(m_pConDens, &lb, &ub);
  }

}


/** if the distance between the predicted ("pred") and the observed feature
* location is smaller than diff, use the observation as "corrected" feature,
* otherwise use the "pred" location
*/
void OpticalFlow::FollowObservationForSmallDiffs(const CPointVector& pred, 
                                                 const CPointVector& obs, 
                                                 CPointVector& corrected,
                                                 double diff)
{
  int num_ft = (int) obs.size();
  ASSERT(num_ft);
  ASSERT((int)pred.size()==num_ft);
  ASSERT((int)corrected.size()==num_ft);
  for (int ft=0; ft<num_ft; ft++) {
    double dx = pred[ft].x-obs[ft].x;
    double dy = pred[ft].y-obs[ft].y;
    double len = sqrt(dx*dx+dy*dy);
    if (len<diff) {
      corrected[ft].x = obs[ft].x;
      corrected[ft].y = obs[ft].y;
    } else {
      corrected[ft].x = pred[ft].x;
      corrected[ft].y = pred[ft].y;
    }
  }
}


/* compute and store the relative location of each feature versus
* the base_state; save it as distance and angle from base_state
*/
void OpticalFlow::PreparePredictFeatureLocations(const CondensState& base_state,
                                                 const CPointVector& base,
                                                 CDoubleVector& old_lens,
                                                 CDoubleVector& old_d_angles)
{
  int num_ft = (int) base.size();
  ASSERT(num_ft);
  old_lens.reserve(num_ft);
  old_d_angles.reserve(num_ft);
  for (int ft=0; ft<num_ft; ft++) {
    double old_dx = base[ft].x-base_state.x;
    double old_dy = base[ft].y-base_state.y;
    double old_len = sqrt(old_dx*old_dx+old_dy*old_dy);
    double old_angle = atan(old_dy/old_dx);
    if (old_dx<0) old_angle += M_PI;
    double old_d_angle = old_angle-base_state.angle;

    old_lens.push_back(old_len);
    old_d_angles.push_back(old_d_angle);
  }
}


/** given a (predicted) state, where do the features end up?
* This requires that PreparePredict.. has been called before
* to obtain an intermediate feature representation that is free
* of the the old state.
*/
void OpticalFlow::PredictFeatureLocations(const CDoubleVector& old_lens,
                                          const CDoubleVector& old_d_angles,
                                          const CondensState& predicted_state,
                                          CPointVector& prediction)
{
  int num_ft = (int)prediction.size();
  ASSERT((int)old_lens.size()==num_ft);
  ASSERT((int)old_d_angles.size()==num_ft);
  for (int ft=0; ft<num_ft; ft++) {
    double old_d_angle = old_d_angles[ft];
    double new_angle = predicted_state.angle+old_d_angle;
    double old_len = old_lens[ft];
    double new_dx = old_len*cos(new_angle);
    double new_dy = old_len*sin(new_angle);

    prediction[ft].x = (float) (predicted_state.x+new_dx);
    prediction[ft].y = (float) (predicted_state.y+new_dy);
//    VERBOSE2(3, "predicted %f, %f", 
//      prediction[ft].x, prediction[ft].y);
  }
}


/* estimate the likelihood that the given prediction is correct,
* given the observation and discarding the furthest-away features
*/
double OpticalFlow::EstimateProbability(const CPointVector& prediction, 
                                        const CPointVector& observation,
                                        int discard_num_furthest)
{
  int num_ft = (int) prediction.size();
  ASSERT((int)observation.size()==num_ft);
  ASSERT(num_ft>discard_num_furthest);
  vector<double> furthest; // will be sorted highest to smallest
  furthest.resize(discard_num_furthest);
  double cum_dist = 0.0;
  for (int ft=0; ft<num_ft; ft++) {
    double dx = prediction[ft].x-observation[ft].x;
    double dy = prediction[ft].y-observation[ft].y;
    double dist = sqrt(dx*dx+dy*dy);

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
  double prob = 1.0/(1.0+cum_dist);
  return prob;
}


/** given the locations of features for a predicted state,
* do they fall on skin-colored pixels?
*/
double OpticalFlow::EstimateProbability(const CPointVector& prediction,
                                        IplImage* rgbImage)
{
  int num_ft = (int) prediction.size();
  ASSERT(num_ft);
  int max_x = rgbImage->width-1;
  int max_y = rgbImage->height-1;
  double prob = 0;
  for (int ft=0; ft<num_ft; ft++) {
    int x = (int) prediction[ft].x;
    int y = (int) prediction[ft].y;
    x = max(0, min(x, max_x));
    y = max(0, min(y, max_y));
    ColorBGR* color;
    GetPixel(rgbImage, x, y, &color);
    double p = m_pProbDistrProvider->LookupProb(*color);
    prob += p;
  }
  prob = prob/(double)num_ft;
  return prob;
}
