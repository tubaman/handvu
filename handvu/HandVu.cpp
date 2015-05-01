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
  * $Id: HandVu.cpp,v 1.44 2006/01/03 21:44:15 matz Exp $
**/

// HandVu.cpp: main library implementation file
//

#include "Common.h"
#include "CubicleWrapper.h"
#include "Skincolor.h"
#include "LearnedColor.h"
#include "OpticalFlow.h"
#include "CamShift.h"
#include "Undistortion.h"
#include "VisionConductor.h"
#include "GestureServer.h"
#include "HandVu.hpp"

#include <fstream>

#if defined(WIN32) && defined(DEBUG)
//#include <streams.h>
#endif

#ifdef USE_MFC
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif // _DEBUG
#endif // USE_MFC


/////////////////////////////////////////////////////////////////////////////
// HandVu

HandVu::HandVu()
  : m_active(false),
    m_tracking(false),
    m_recognized(false),
    m_id1_on(false),
    m_do_track(true),
    m_video_width(0),
    m_video_height(0),
    m_alignment_crosses(1),
    m_buf_indx_cycler(-1),
    m_curr_buf_indx(-1),
    m_prev_buf_indx(-1),
    m_rgbImage(NULL),
    m_depthImage(NULL),
    m_rightGrayImage(NULL),
    m_center_depth(0),
    m_initialized(false),
    m_quit_thread(false),
    m_pAsyncThread(NULL),
    m_img_width(-1),
    m_img_height(-1),
    m_undistort(false),
    m_overlay_level(0),
    m_sample_time(0),
    m_num_succ_dropped_frames(0),
    m_max_normal_latency(-1),
    m_max_abnormal_latency(-1),
    m_determine_normal_latency(false),
    m_t_start_processing(0),
    m_time_to_learn_color(0),
    m_min_time_between_learning_color(0),
    m_adjust_exposure(false),
    m_adjust_exposure_at_time(0),
    m_pCameraController(NULL),
    m_pClock(NULL)
{
  m_pCubicle = new CubicleWrapper();
  m_pSkincolor = new Skincolor();
  m_pLearnedColor = new LearnedColor();
  m_pOpticalFlow = new OpticalFlow();
  m_pCamShift = new CamShift();
  m_pUndistortion = new Undistortion();
  m_pConductor = new VisionConductor();
  m_grayImages[0] = NULL;
  m_grayImages[1] = NULL;

//  g_ostream = fopen("c:\\tmp\\HVout.txt", "aw+");
  g_ostream = NULL;
  g_verbose = 0;
}

HandVu::~HandVu()
{
  // quit Aync thread
  if (m_pAsyncThread) {
    m_quit_thread = true; // will be set to false by thread upon exit
    m_pAsyncThread->Resume();
  }
  
  // our datastructures
  delete m_pCubicle;
  delete m_pSkincolor;
  delete m_pLearnedColor;
  delete m_pOpticalFlow;
  delete m_pCamShift;
  delete m_pUndistortion;
  delete m_pConductor;

  // images
  cvReleaseImage(&m_grayImages[0]);
  cvReleaseImage(&m_grayImages[1]);
  cvReleaseImage(&m_depthImage);
  cvReleaseImage(&m_rightGrayImage);

  // wait for Aync thread, then delete ring buffer
  if (m_pAsyncThread) {
     m_pAsyncThread->Join();
  }  
  for (int b=0; b<(int)m_ring_buffer.size(); b++) {
    cvReleaseImage(&m_ring_buffer[b]);
  }
}

/* pCamCon may be NULL
*/
void HandVu::Initialize(int width, int height, RefClock* pClock,
			CameraController* pCamCon)
{
  VERBOSE3(5, "HandVu: initializing with size %dx%d, CameraController: %s",
    width, height, pCamCon?"yes":"no");

  cvReleaseImage(&m_grayImages[0]);
  cvReleaseImage(&m_grayImages[1]);
  cvReleaseImage(&m_depthImage);
  cvReleaseImage(&m_rightGrayImage);
  m_grayImages[0] = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
  m_grayImages[1] = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
  m_rightGrayImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
  m_depthImage     = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);

  m_pCubicle->Initialize(width, height);
  m_pSkincolor->Initialize(width, height);
  m_pLearnedColor->Initialize(width, height);
  m_pOpticalFlow->Initialize(width, height);
  m_pUndistortion->Initialize(width, height);
  m_pCamShift->Initialize(width, height);
  m_img_width = width;
  m_img_height = height;

  if (!pClock) {
    throw HVException("no clock set!");
  }
  m_pClock = pClock;
  if (!pCamCon) {
    VERBOSE0(3, "no camera controller will be available");
  }
  m_pCameraController = pCamCon;
  m_scan_area = CRect(width/3, height/3, 2*width/3, 2*height/3);
  m_min_time_between_learning_color = 3*1000*1000; // 3 sec

  // normal latencies etc.
  m_determine_normal_latency = true;
  m_max_normal_latency = 3000; // 3ms
  m_max_abnormal_latency = 15000; // 15ms
  m_last_latencies.resize(1);

  m_initialized = true;
}



bool HandVu::ConductorLoaded() const
{
  return m_pConductor->IsLoaded();
}

void HandVu::LoadConductor(const string& filename)
{
  VERBOSE1(5, "HandVu: loading conductor from file %s",
    filename.c_str());

  m_pConductor->Load(filename);
  // load a calibration matrix, if available, and set active
  if (m_pConductor->m_camera_calib!="") {
    m_pUndistortion->Load(m_pConductor->m_camera_calib.c_str());
    m_undistort = true;
  }
  if (m_pConductor->m_adjust_exposure 
    && m_pCameraController
    && m_pCameraController->CanAdjustExposure())
  {
    bool turned_off = 
      m_pCameraController->SetCameraAutoExposure(false);
    if (!turned_off) {
      throw HVException("can not turn off camera auto exposure");
    }
    m_adjust_exposure = true;
  }

  VERBOSE0(5, "HandVu: done loading conductor");
}


// put filters in the right state, supply scanners and cascades
//
void HandVu::StartRecognition(int id/*=0*/)
{
  if(!m_initialized) {
    throw HVException("HandVu not initialized, cannot start");
  }

  if(!m_pConductor->IsLoaded()) {
    throw HVException("no conductor loaded");
  }

  VERBOSE1(5, "HandVu: starting recognition for obj_id %d", id);

  if (id==1) {
    m_id1_on = true;
    m_id1_x = -1;
    m_id1_y = -1;
    if (m_active) return;
  } else if (id!=0) {
    throw HVException("unknown ID");
  }

  // other detection
  m_dt_first_match_time = 0;
  m_dt_first_match = CuScanMatch();

  // activate detection scanners
  for (int cc=0; cc<m_pConductor->m_dt_cascades_end; cc++) {
    cuSetScannerActive((CuCascadeID)cc, true);
    CRect area(m_pConductor->m_orig_areas[cc].toRect(m_img_width, m_img_height));
    cuSetScanArea((CuCascadeID)cc, area.left, area.top, area.right, area.bottom);
  }
  // de-activate recognition scanners
  for (int cc=m_pConductor->m_rc_cascades_start;
       cc<m_pConductor->m_rc_cascades_end; cc++) {
    cuSetScannerActive((CuCascadeID)cc, false);
  }

  // the scan area is mostly used during tracking, but also
  // to do the exposure control
  CQuadruple quad;
  GetDetectionArea(quad);
  m_scan_area = quad.toRect(m_img_width, m_img_height);

  m_frame_times.clear();
  m_processed_frame_times.clear();
  m_prcs_times.clear();

  m_buf_indx_cycler = 0;
  m_curr_buf_indx = m_buf_indx_cycler;
  m_prev_buf_indx = 1-m_buf_indx_cycler;

  m_do_track = true;

  m_active = true;
  m_tracking = false;
}

void HandVu::StopRecognition(int id/*=0*/)
{
  VERBOSE1(5, "HandVu: stopping recognition for obj_id %d", id);

  if (id==1) {
    m_id1_on = false;
    return;
  } else if (id!=0) {
    throw HVException("unknown ID");
  }

  m_active = false;
}


/* --------------------------------------------------------
* THE ALMIGHTY ProcessFrame routine !!!!!!!!!!!!!!!!!!!!!
* ----------------------------------------------------------
*/
HandVu::HVAction
HandVu::ProcessFrame(GrabbedImage& inOutImage, const IplImage* rightImage)
{
  m_rgbImage = inOutImage.GetImage();
  m_sample_time = inOutImage.GetSampleTime();

  // sanity checks
  if(!m_initialized) {
    throw HVException("HandVu not initialized, cannot process");
  }
  if (m_rgbImage->width!=m_img_width || m_rgbImage->height!=m_img_height) {
    throw HVException("image dimensions do not match initialization");
  }
  VERBOSE2(5, "HandVu: processing frame (active: %s, tracking: %s)",
    m_active?"yes":"no", m_tracking?"yes":"no");

  if (m_rgbImage->origin==1) {
    m_rgbImage->origin = 0;
    cvMirror(m_rgbImage, NULL, 0);
  }

  // check latency of the incoming frame - the result is used
  // after KLT tracking
  HVAction action = CheckLatency();

  // return if we're not supposed to do anything
  if (!m_active
      || m_scan_area.left>=m_scan_area.right 
      || m_scan_area.top>=m_scan_area.bottom) {
    if (action==HV_PROCESS_FRAME) {
      // adjust exposure
      CheckAndCorrectExposure();
      
      // undistort image, adjust location of centroid
      if (m_undistort) {
        m_pUndistortion->Undistort(m_rgbImage);
    //    m_pUndistortion->Transform(centroid);
      }
    }
    
    KeepStatistics(action);
    DrawOverlay();
    SendEvent();

    return action;
  }

  // set current buffer
  m_buf_indx_cycler = 1-m_buf_indx_cycler;
  m_curr_buf_indx = m_buf_indx_cycler;
  m_prev_buf_indx = 1-m_buf_indx_cycler;

  // create gray scale image
  {
    int scan_width = m_scan_area.right-m_scan_area.left;
    int cvt_left = max(0, m_scan_area.left-scan_width/2);
    int cvt_width = min(m_scan_area.right+scan_width/2, m_rgbImage->width)-cvt_left;
    int scan_height = m_scan_area.bottom-m_scan_area.top;
    int cvt_top = max(0, m_scan_area.top-scan_height/2);
    int cvt_height = min(m_scan_area.bottom+scan_height/2, m_rgbImage->height)-cvt_top;
    ASSERT(cvt_height>0 && cvt_width>0);
    if (!(cvt_height>0 && cvt_width>0)) {
      KeepStatistics(action);
      DrawOverlay();
      SendEvent();

      return action;
    }
    cvSetImageROI(m_rgbImage, cvRect(cvt_left, cvt_top, cvt_width, cvt_height));
    cvSetImageROI(m_grayImages[m_curr_buf_indx], cvRect(cvt_left, cvt_top, cvt_width, cvt_height));

    cvCvtColor(m_rgbImage, m_grayImages[m_curr_buf_indx], CV_BGR2GRAY);
    
    cvResetImageROI(m_rgbImage);
    cvResetImageROI(m_grayImages[m_curr_buf_indx]);
  }

  // do the all-important, fast KLT tracking
  m_recognized = false;
  if (m_tracking) {    
    m_tracking = DoTracking();
    if (!m_tracking) {
      // lost tracking
      StartRecognition();
    }
  }

  // take the recommendation from CheckLatency to heart
  if (action!=HV_PROCESS_FRAME) {
    KeepStatistics(action);
    DrawOverlay();
    SendEvent();

    return action;
  }

  if (m_tracking) {    
    m_recognized = DoRecognition();
    FindSecondHand();
  
  } else {
    m_recognized = DoDetection();
  }

  if (m_recognized && m_do_track) {
    InitializeTracking();
    m_tracking = true;
  }

  // restrict the general color segmentation ROI; this is
  // not really important for anything but high overlay_levels
  int scan_left = max(0, m_scan_area.left);
  int scan_right = min(m_scan_area.right, m_rgbImage->width);
  int scan_top = max(0, m_scan_area.top);
  int scan_bottom = min(m_scan_area.bottom, m_rgbImage->height);
  CRect scan_area(scan_left, scan_top, scan_right, scan_bottom);

  // use stereo correspondence to obtain distance of hand from camera
  if (m_tracking && rightImage) {
    int sz = (scan_right-scan_left)/5;
    CvRect area = cvRect((int)m_center_pos.x-sz, (int)m_center_pos.y-sz, 2*sz, 2*sz);
    CalculateDepth(rightImage, area);
  }
  
  // adjust exposure
  CheckAndCorrectExposure();

  // drawing
  m_pSkincolor->DrawOverlay(m_rgbImage, m_overlay_level, CRect(m_last_match));
  if (m_tracking) {
    m_pLearnedColor->DrawOverlay(m_rgbImage, m_overlay_level, scan_area);
  }
  m_pCubicle->DrawOverlay(m_rgbImage, m_overlay_level);

  if (m_tracking) {
    if (m_pConductor->m_tr_type & VisionConductor::VC_TT_CAMSHIFT) {
      m_pCamShift->DrawOverlay(m_rgbImage, m_overlay_level);
    } else {
      m_pOpticalFlow->DrawOverlay(m_rgbImage, m_overlay_level);
    }
  }
  // undistort image, adjust location of centroid
  if (m_undistort) {
    m_pUndistortion->Undistort(m_rgbImage);
//    m_pUndistortion->Transform(centroid);
  }

  KeepStatistics(action);
  DrawOverlay();
  SendEvent();

  return action;
}
/* --------------------------------------------------------
* ----------------------------------------------------------
*/


/* C/C++ interface for AsyncProcessor
 */
void* asyncProcessor(void* arg)
{
  if (arg==NULL) {
    fprintf(stderr, "asyncProcessor: no argument!!\n");
    return NULL;
  }
  HandVu* hv = (HandVu*) arg;
  hv->AsyncProcessor();
}

/* allocate internal ring buffer, start processing thread
 */
void HandVu::AsyncSetup(int num_buffers, DisplayCallback* pDisplayCB)
{
  // sanity checks
  if(!m_initialized) {
    throw HVException("HandVu not initialized, cannot setup async processing");
  }

  if (num_buffers<=0 || 50<num_buffers) {
    throw HVException("num_buffers must be between 1..50");
  }

  if (pDisplayCB==NULL) {
    throw HVException("DisplayCallback can not be NULL");
  }

  // allocate ring buffer
  m_ring_buffer.resize(num_buffers);
  m_ring_buffer_occupied.resize(num_buffers);
  for (int b=0; b<num_buffers; b++) {
    CvSize size = cvSize(m_img_width, m_img_height);
    m_ring_buffer[b] = cvCreateImage(size, IPL_DEPTH_8U, 3);
    m_ring_buffer_occupied[b] = false;
  }

  m_pDisplayCallback = pDisplayCB;
  
  // start processing thread, it will suspended itself if
  // m_process_queue is empty
  m_pAsyncThread = new Thread(asyncProcessor, this);
  m_pAsyncThread->Start();
}


/* insert the frame in the processing queue
 */
void HandVu::AsyncProcessFrame(int id, RefTime& t)
{
  // insert in queue
  GrabbedImage gi(m_ring_buffer[id], t, id);
  m_process_queue.push_back(gi);
  
  // wake up processing thread
  m_pAsyncThread->Resume();
}


/* give the capture application a handle to an unused image in
   the ring buffer
*/
void HandVu::AsyncGetImageBuffer(IplImage** pImg, int* pID)
{
  // this is the only place where we occupy buffers, so we
  // don't have to use a mutex (of course, we might fail
  // for no good reason)
  for (int b=0; b<(int)m_ring_buffer.size(); b++) {
    if (!m_ring_buffer_occupied[b]) {
      *pImg = m_ring_buffer[b];
      *pID  = b;
      return;
    }
  }
  throw HVException("exceeded ring buffer size");
}


void HandVu::AsyncProcessor()
{
  for (;;) {
    // suspend this thread if processing queue is empty
    m_pAsyncThread->Lock();
    if (m_process_queue.empty()) {
      m_pAsyncThread->Suspend();
    }
    m_pAsyncThread->Unlock();
    
    while (!m_quit_thread && !m_process_queue.empty()) {
      // get front of queue - don't lock
      GrabbedImage gi = m_process_queue.front();
      m_process_queue.pop_front();
      
      ASSERT(m_ring_buffer_occupied[gi.GetBufferID()]);
      HVAction action = ProcessFrame(gi);
      m_pDisplayCallback->Display(gi.GetImage(), action);
      
      // free up the buffer - we don't have to lock
      m_ring_buffer_occupied[gi.GetBufferID()] = false;
    }
    
    if (m_quit_thread) {
      m_pAsyncThread->Stop();
    }
  }
}


HandVu::HVAction HandVu::CheckLatency()
{
  ASSERT(m_pClock);

  m_t_start_processing = m_pClock->GetCurrentTimeUsec();

  RefTime incoming_latency = 
    max((RefTime)0, m_t_start_processing-m_sample_time);
  if (m_determine_normal_latency) {
    const int drop_num_frames = 25;
    const int skip_first_num_frames = 15;

    m_last_latencies.push_back(incoming_latency);
    int num_frames_dropped = (int) m_last_latencies.size();
    if (num_frames_dropped<drop_num_frames) {
      // drop frame
      VERBOSE1(3, "HandVu: dropping frame during latency computation (latency %ldus)", 
               incoming_latency);
      return HV_DROP_FRAME;
    }
    
    // calculate average and max latencies
    RefTime sum_latencies = 0;
    RefTime max_latency = LONG_MIN;
    for (int frm=skip_first_num_frames; frm<num_frames_dropped; frm++) {
      RefTime lat = m_last_latencies[frm];
      sum_latencies += lat;
      if (lat>max_latency) {
        max_latency = lat;
      }
    }
    // average of (average and max), plus 5ms
    int counted = num_frames_dropped-skip_first_num_frames;
    m_max_normal_latency = min((max_latency + sum_latencies/counted)/2l
                                + 2000, // micro-second units -> 2ms
                               max_latency + 1000); // -> 1ms

    m_max_abnormal_latency = max(m_max_normal_latency*2, 
				      (RefTime)15000);

    VERBOSE5(3, "HandVu: avg latency in %d frames: %dus, max: %dus -> norm: %dus, abnorm: %dus", 
             counted, (int)(sum_latencies/counted), (int)(max_latency), 
             (int)(m_max_normal_latency), (int)(m_max_abnormal_latency));

    // stop latency computation
    m_determine_normal_latency = false;
    m_last_latencies.resize(1);
  }

  ASSERT(m_last_latencies.size()>0);
  m_last_latencies[0] = incoming_latency;
  if (incoming_latency>m_max_normal_latency) {
    if (incoming_latency>m_max_abnormal_latency) {
      const int max_succ_dropped_frames = 75; 
      // todo: should base this on seconds, not frames dropped.
      // like no frame for 5 sec: recompute
      m_num_succ_dropped_frames++;
      VERBOSE1(3, "HandVu: dropping frame (latency %ldms)", incoming_latency/1000);
      if (m_num_succ_dropped_frames>max_succ_dropped_frames) {
        m_determine_normal_latency = true;
        VERBOSE1(2, "HandVu: warning - %d contiguous frames dropped", 
                m_num_succ_dropped_frames);
        m_num_succ_dropped_frames = 0;
      }
      return HV_DROP_FRAME;
    }
    VERBOSE1(3, "HandVu: skipping frame (latency %ldms)", incoming_latency/1000);
    return HV_SKIP_FRAME;
  }

  m_num_succ_dropped_frames = 0;

  static bool quickreturn = false;
  //  quickreturn = !quickreturn;
  if (quickreturn) return HV_SKIP_FRAME;
  
  return HV_PROCESS_FRAME;
}


void HandVu::KeepStatistics(HVAction action)
{
  // times of frames: all frames
  RefTime t_curr = m_pClock->GetCurrentTimeUsec();
  m_frame_times.push_back(t_curr);
  if (action==HV_PROCESS_FRAME) {
    // completely processed frames
    m_processed_frame_times.push_back(t_curr);
  }
  // processing time
  RefTime prcs_time = t_curr-m_t_start_processing; // micro-second units
  if (action==HV_PROCESS_FRAME || action==HV_SKIP_FRAME) {
    m_prcs_times.push_back(prcs_time);
  } else {
    m_prcs_times.push_back(-1);
  }

  // keep only times from within the last second
  // in the three _times arrays

  // all frames that we saw
  RefTime ago = t_curr-m_frame_times.front();
  while (ago>1000000) { // 1 second
    m_frame_times.erase(m_frame_times.begin());
    m_prcs_times.erase(m_prcs_times.begin());
    ago = t_curr-m_frame_times.front();
    // we're guaranteed to have one element in there with ago==0,
    // so we won't pop the list empty
  }

  // completed frames
  while (!m_processed_frame_times.empty()) {
    ago = t_curr-m_processed_frame_times.front();
    if (ago<=1000000) { // 1 second
      break;
    }
    m_processed_frame_times.erase(m_processed_frame_times.begin());
  }
}

void HandVu::DrawOverlay()
{
  if (m_overlay_level>=1) {
    CvFont font;
    cvInitFont( &font, CV_FONT_VECTOR0, 0.5f /* hscale */, 
                0.5f /* vscale */, 0.1f /*italic_scale */, 
                1 /* thickness */);

    // frames per second and min/max processing times
    int fps = (int) m_frame_times.size();
    int processed_fps = (int) m_processed_frame_times.size();
    RefTime min_prcs_time=-1, max_prcs_time=-1;
    for (int f=0; f<(int)m_prcs_times.size(); f++) {
      if (m_prcs_times[f]==-1) {
        continue;
      }
      if (m_prcs_times[f]<min_prcs_time || min_prcs_time==-1) {
        min_prcs_time = m_prcs_times[f];
      } 
      if (m_prcs_times[f]>max_prcs_time || max_prcs_time==-1) {
        max_prcs_time = m_prcs_times[f];
      }
    }
    if (min_prcs_time==-1) {
      ASSERT(max_prcs_time==-1);
      min_prcs_time = 0;
      max_prcs_time = 0;
    }

    char str[256];
    sprintf(str, "%d (%d) fps, %d-%dms", fps, processed_fps, 
            (int)(min_prcs_time/1000), (int)(max_prcs_time/1000)); 
    CvSize textsize;
    int underline;
    cvGetTextSize( str, &font, &textsize, &underline );
    CvPoint pos = cvPoint(m_rgbImage->width-textsize.width-5, textsize.height+10);
    cvPutText(m_rgbImage, str, pos, &font, CV_RGB(0, 255, 0));

    VERBOSE4(3, "HandVu: %d (%d) fps, %d-%dms latency", 
             fps, processed_fps, (int)(min_prcs_time/1000), (int)(max_prcs_time/1000));

    if (m_overlay_level>=2 && m_last_latencies.size()>0) {
      RefTime last = m_last_latencies[m_last_latencies.size()-1];
      char str[256];
      sprintf(str, "(in latency: %dms)", (unsigned int)(last/1000)); 
      CvSize textsize;
      int underline;
      cvGetTextSize(str, &font, &textsize, &underline );
      CvPoint pos = cvPoint(m_rgbImage->width/2-textsize.width, textsize.height+10);
      cvPutText(m_rgbImage, str, pos, &font, CV_RGB(0, 255, 0));
    }

    if (m_tracking) {
      CvPoint pos = cvPoint(cvRound(m_center_pos.x), cvRound(m_center_pos.y));
      cvCircle(m_rgbImage, pos, 13, CV_RGB(0, 0, 0), CV_FILLED);
      cvCircle(m_rgbImage, pos, 10, CV_RGB(255, 255, 255), CV_FILLED);
//      cvCircle(m_rgbImage, cvPoint(cvRound(m_center_pos.x), cvRound(m_center_pos.y)), 
  //             5, CV_RGB(255, 0, 0), CV_FILLED);
    }

    if (m_overlay_level>=3) {
      char str[256];
      char* ptr = str;
      if (!m_active) {
        sprintf(ptr, "inactive ");
        ptr += 9;
      }
      if (m_scan_area.left>=m_scan_area.right
          || m_scan_area.top>=m_scan_area.bottom) 
      {
        sprintf(ptr, "zero-scan ");
        ptr += 10;
      }
      if (m_undistort) {
        sprintf(ptr, "u ");
        ptr += 2;
      }
      if (m_adjust_exposure) {
        sprintf(ptr, "e ");
        ptr += 2;
      }
      if (ptr!=str) {
        CvSize textsize;
        int underline;
        cvGetTextSize(str, &font, &textsize, &underline);
        CvPoint pos = cvPoint(10, textsize.height+10);
        cvPutText(m_rgbImage, str, pos, &font, CV_RGB(0, 255, 0));
      }
    }
  }
}

bool HandVu::DoDetection()
{
  // scan cubicles
  // todo RefTime before = m_pClock->GetCurrentTimeUsec();
  m_pCubicle->Process(m_grayImages[m_curr_buf_indx]);
  // todo RefTime after = m_pClock->GetCurrentTimeUsec();
  // todo FILE* fp = fopen("c:\\hv_tmp\\times.txt", "a+");
  // todo RefTime took = after-before;
  // todo fprintf(fp, "%u\n", took);
  // todo fclose(fp);
  if (m_pCubicle->GotMatches()) {
    m_last_match = m_pCubicle->GetBestMatch();

    // got a match, but how and where?
    // if m_dt_min_match_duration>0, we need more than a single match 
    // but instead a succession of matches within a certain radius
    // from each other
    if (m_dt_first_match_time==0) {
      m_dt_first_match = m_last_match;
      m_dt_first_match_time = m_pClock->GetCurrentTimeUsec();
    }
      
    VERBOSE4(4, "HandVu detection: area %d, %d, %d, %d", 
             m_last_match.left, m_last_match.top, 
             m_last_match.right, m_last_match.bottom);

    // was the match close enough to the first match to be considered 
    // within the same area?
    int curr_center_x = (m_last_match.left+m_last_match.right)/2;
    int curr_center_y = (m_last_match.top+m_last_match.bottom)/2;
    int first_center_x =
      (m_dt_first_match.left+m_dt_first_match.right)/2;
    int first_center_y =
      (m_dt_first_match.top+m_dt_first_match.bottom)/2;
    int dx = curr_center_x-first_center_x;
    int dy = curr_center_y-first_center_y;
    if (sqrt((double) (dx*dx+dy*dy))<m_pConductor->m_dt_radius*m_img_width) {

      // was the match duration long enough?
      RefTime curr_time = m_pClock->GetCurrentTimeUsec();
      if ((curr_time-m_dt_first_match_time)/1000
          >= m_pConductor->m_dt_min_match_duration) 
      {
        // color verification
        bool mostly_skin = VerifyColor();
        if (mostly_skin) {
          // that's it!
          // turn off detection
          for (int cc=0; cc<m_pConductor->m_dt_cascades_end; cc++) {
            cuSetScannerActive((CuCascadeID)cc, false);
          }
          m_dt_first_match_time = 0;

          // set scan area for tracking and recognition
          int halfwidth = (m_last_match.right-m_last_match.left)/2;
          int halfheight = (m_last_match.bottom-m_last_match.top)/2;
          SetScanAreaVerified(CRect(m_last_match.left-halfwidth, m_last_match.top-halfheight, 
                                    m_last_match.right+halfwidth, m_last_match.bottom+halfheight));

          return true;
        }
      }
    } else {
      m_dt_first_match_time = 0;
    }
  } else {
    m_dt_first_match_time = 0;
  }
  return false;
}

bool HandVu::DoRecognition()
{
  // set scan areas and
  // activate detection scanners
  for (int cc=m_pConductor->m_rc_cascades_start;
       cc<m_pConductor->m_rc_cascades_end; cc++) {
    cuSetScanArea((CuCascadeID)cc, 
      m_scan_area.left, m_scan_area.top, m_scan_area.right, m_scan_area.bottom);
    double sct = m_pConductor->m_rc_scale_tolerance;
    cuSetScanScales((CuCascadeID)cc, m_last_match.scale/sct,
                                 m_last_match.scale*sct);
    cuSetScannerActive((CuCascadeID)cc, true);
  }

  m_pCubicle->Process(m_grayImages[m_curr_buf_indx]);
  if (m_pCubicle->GotMatches()) {
    m_last_match = m_pCubicle->GetBestMatch();
    VERBOSE4(4, "HandVu detection: area %d, %d, %d, %d", 
             m_last_match.left, m_last_match.top, 
             m_last_match.right, m_last_match.bottom);

    // set scan area for future
    int halfwidth = (m_last_match.right-m_last_match.left)/2;
    int halfheight = (m_last_match.bottom-m_last_match.top)/2;
    SetScanAreaVerified(CRect(m_last_match.left-halfwidth, m_last_match.top-halfheight, 
                              m_last_match.right+halfwidth, m_last_match.bottom+halfheight));

    return true;
  }
  return false;
}

bool HandVu::VerifyColor()
{
  ConstMaskIt mask = m_pConductor->GetMask(m_last_match.name);

  CRect roi(m_last_match);
  double coverage =
    m_pSkincolor->GetCoverage(m_rgbImage, roi, mask, false);
  VERBOSE0(3, "HandVu verifyed color");
  
  bool sufficient = (coverage>=m_pConductor->m_dt_min_color_coverage);
  if (sufficient) {
    VERBOSE0(3, "HandVu: color match!");
  } else {
    VERBOSE0(3, "HandVu: insufficient color match");
  }
  return sufficient;
}

void HandVu::InitializeTracking()
{
  ConstMaskIt mask = m_pConductor->GetMask(m_last_match.name);

  // if we haven't done so for some time, and the image was taken after
  // m_time_to_learn_color:
  if (m_time_to_learn_color<=m_sample_time) {
    // learn the RGB lookup table and 
    // use it for subsequent segmentations
    m_pLearnedColor->LearnFromGroundTruth(m_rgbImage, m_last_match, mask);
    m_time_to_learn_color = m_sample_time + m_min_time_between_learning_color;
  }

  if (m_pConductor->m_tr_type==VisionConductor::VC_TT_CAMSHIFT_HSV) {
    m_pCamShift->PrepareTracking(m_rgbImage, NULL, CRect(m_last_match));

  } else if (m_pConductor->m_tr_type==VisionConductor::VC_TT_CAMSHIFT_LEARNED) {
    m_pCamShift->PrepareTracking(m_rgbImage, m_pLearnedColor, CRect(m_last_match));

  } else {
    // color segmentation provides a probability distribution to the
    // optical flow filter to place features
    m_pOpticalFlow->PrepareTracking(m_rgbImage,
                                    m_grayImages[m_curr_buf_indx],
                                    m_curr_buf_indx,
                                    m_pLearnedColor, 
                                    m_last_match,
                                    mask,
                                    m_pConductor->m_tr_num_KLT_features,
                                    m_pConductor->m_tr_winsize_width,
                                    m_pConductor->m_tr_winsize_height,
                                    m_pConductor->m_tr_min_feature_distance,
                                    m_pConductor->m_tr_max_feature_error);
  }

  m_center_pos.x = (m_last_match.right+m_last_match.left)/2.0f;
  m_center_pos.y = (m_last_match.bottom+m_last_match.top)/2.0f;
}

bool HandVu::DoTracking()
{
  // the size of the last Cubicle match determines how far we let
  // KLT features spread (during the call to Track)
  int last_width  = m_last_match.right-m_last_match.left;
  int last_height = m_last_match.bottom-m_last_match.top;


  if (m_pConductor->m_tr_type & VisionConductor::VC_TT_CAMSHIFT) {
    m_pCamShift->Track(m_rgbImage);
    m_pCamShift->GetArea(m_scan_area);
    m_center_pos.x = (m_scan_area.right+m_scan_area.left)/2.0f;
    m_center_pos.y = (m_scan_area.bottom+m_scan_area.top)/2.0f;

  } else {
    bool flock = m_pConductor->m_tr_type==VisionConductor::VC_TT_OPTICAL_FLOW_FLOCK;
    bool color = m_pConductor->m_tr_type==VisionConductor::VC_TT_OPTICAL_FLOW_COLOR;
    if (m_pConductor->m_tr_type==VisionConductor::VC_TT_OPTICAL_FLOW_COLORFLOCK) {
      flock = color = true;
    }
    int num_tracked =
      m_pOpticalFlow->Track(m_rgbImage,
                            m_grayImages[m_prev_buf_indx],
                            m_grayImages[m_curr_buf_indx],
                            m_prev_buf_indx, m_curr_buf_indx,
                            last_width, last_height,
                            flock, color);
    
    if (num_tracked<m_pConductor->m_tr_min_KLT_features) {
      // lost tracking for sure
      //    Beep(300, 150);
      // don't change scan_area
      return false;
    }

    m_pOpticalFlow->GetMeanFeaturePos(m_center_pos);
    // VERBOSE1(4, "OpticalFlow features: %d\n", event.m_num_features_tracked);
  }

  int rnx = cvRound(m_center_pos.x), rny = cvRound(m_center_pos.y);
  SetScanAreaVerified(CRect(rnx-last_width, rny-last_height, 
			    rnx+last_width, rny+last_height));

  return true;
}




void HandVu::CheckAndCorrectExposure()
{
  if (!m_adjust_exposure
      || m_scan_area.left>=m_scan_area.right 
      || m_scan_area.top>=m_scan_area.bottom)
  {
    return;
  }

  ASSERT(m_pCameraController);
  ASSERT(m_pCameraController->CanAdjustExposure());

  // is it time to adjust camera?
  RefTime curr_time = m_pClock->GetCurrentTimeUsec();
  if (curr_time<m_adjust_exposure_at_time) {
    return;
  }

  // find out if we should change the exposure at all
  int bins = 5;
  CvHistogram* pHist = cvCreateHist(1, &bins, CV_HIST_ARRAY);
  CvRect rect = 
    cvRect(m_scan_area.left, m_scan_area.top, 
    m_scan_area.right-m_scan_area.left, m_scan_area.bottom-m_scan_area.top);
  cvSetImageROI(m_grayImages[m_curr_buf_indx], rect);
  cvCalcHist(&m_grayImages[m_curr_buf_indx], pHist, 0, NULL);
  cvResetImageROI(m_grayImages[m_curr_buf_indx]);
  double bright_pixels = cvQueryHistValue_1D(pHist, 4);
  bright_pixels /= (double) (rect.width*rect.height);
  cvReleaseHist(&pHist);
  VERBOSE1(4, "HandVu: bright_pixels %f", bright_pixels);

  double exposure_factor = 1.0;
  const double max_bright = 0.3;
  const double min_bright = 0.1;
  if (bright_pixels>max_bright) {
    if (bright_pixels!=0) {
      exposure_factor = max(0.75, max_bright/bright_pixels);
    } else {
      exposure_factor = 0.9;
    }
    m_adjust_exposure_at_time = curr_time+500000l; //.5 sec
  } else if (bright_pixels<min_bright) {
    if (bright_pixels!=0) {
      exposure_factor = min(1.25, min_bright/bright_pixels);
    } else {
      exposure_factor = 1.1;
    }
    m_adjust_exposure_at_time = curr_time+500000l; //.5 sec
  } else {
    // we're right on, don't check again for some time
    m_adjust_exposure_at_time = curr_time+1000000l; //1 sec
  }

  if (exposure_factor!=1.0) {
    if (m_exposure_level<=0.01) {
      m_exposure_level = 0.01;
    } else {
      m_exposure_level *= exposure_factor;
    }
    m_exposure_level = max(0.0, min(m_exposure_level, 1.0));
    bool changed = m_pCameraController->SetExposure(m_exposure_level);
    while (!changed && 0<m_exposure_level && m_exposure_level<1.0) {
      m_exposure_level *= exposure_factor;
      changed = m_pCameraController->SetExposure(m_exposure_level);
    } 
    VERBOSE2(4, "HandVu: set exposure level to %f (%d)\n", m_exposure_level, changed);
  }
}

void HandVu::FindSecondHand()
{
  if (m_id1_on) {
    CvPoint2D32f pos;
    m_pOpticalFlow->GetMeanFeaturePos(pos);
    double w = m_last_match.right - m_last_match.left;
    double h = m_last_match.bottom - m_last_match.top;
    CRect area(10, cvRound(pos.y+h/3), cvRound(pos.x-w/3), m_rgbImage->height-10);
    // pos is output parameter:
    m_pLearnedColor->GetMostRightUpBlob(m_rgbImage, area, pos);
    m_id1_x = pos.x;
    m_id1_y = pos.y;
  }
}

void HandVu::CalculateDepth(const IplImage* rightImage, const CvRect& area)
{
	cvSetImageROI((IplImage*)rightImage, area);
	cvSetImageROI(m_rightGrayImage, area);
  cvCvtColor(rightImage, m_rightGrayImage, CV_BGR2GRAY);
  cvResetImageROI((IplImage*)rightImage);
	
  cvSetImageROI(m_grayImages[m_curr_buf_indx], area);
  int maxDisparity = min(30, area.width-1);
  cvSetImageROI(m_depthImage, area);
  cvFindStereoCorrespondence(m_grayImages[m_curr_buf_indx],
                             m_rightGrayImage, CV_DISPARITY_BIRCHFIELD,
                             m_depthImage, 
                             maxDisparity, 15, 3, 6, 8, 15 );

  cvResetImageROI(m_grayImages[m_curr_buf_indx]);
  cvResetImageROI(m_rightGrayImage);

  cvConvertScale(m_depthImage, m_depthImage, 255.0/(double)maxDisparity);

  cvSetImageROI(m_rgbImage, area);
  cvCvtColor(m_depthImage, m_rgbImage, CV_GRAY2BGR);
  cvResetImageROI(m_rgbImage);

  CvScalar s = cvAvg(m_depthImage);
  m_center_depth = s.val[0];
}

/* SetDetectionArea sets the detection scan area in the video. 
* The coordinates need not be in a specific orientation, i.e. left and right
* can be switched and top and bottom can be switched.
*/
void HandVu::SetDetectionArea(int left, int top, int right, int bottom)
{
  // righten area, smaller numbers to left and up
  CRect scan_area(min(left, right), min(top, bottom),
                  max(left, right), max(top, bottom));
  for (int scc=0; scc<m_pConductor->m_dt_cascades_end; scc++) {
    cuSetScanArea((CuCascadeID)scc, 
      scan_area.left, scan_area.top, scan_area.right, scan_area.bottom);
    m_pConductor->m_orig_areas[scc].fromRect(scan_area, m_img_width, m_img_height);
  }
}

void HandVu::GetDetectionArea(CQuadruple& area) const
{
  // righten area, smaller numbers to left and up
  area.left = area.top = 1.0;
  area.right = area.bottom = 0.0;
  for (int scc=0; scc<m_pConductor->m_dt_cascades_end; scc++) {
    const CQuadruple& curr = m_pConductor->m_orig_areas[scc];
    area.left = min(area.left, curr.left);
    area.right = max(area.right, curr.right);
    area.top = min(area.top, curr.top);
    area.bottom = max(area.bottom, curr.bottom);
  }

  // if no detection cascades, or wrong data, set all to zero
  if (area.left>=area.right || area.top>=area.bottom) {
    area.left = area.right = area.top = area.bottom = 0.0;
  }
}

void HandVu::SetOverlayLevel(int level)
{
  if (m_overlay_level<0 || m_overlay_level>MAX_OVERLAY_LEVEL) {
    throw HVException("invalid overlay level");
  }
  m_overlay_level = level;
}

int HandVu::GetOverlayLevel()
{
  return m_overlay_level;
}

bool HandVu::CanCorrectDistortion() const
{
  return m_pUndistortion->CanUndistort();
}

bool HandVu::IsCorrectingDistortion() const 
{
  if (!m_pUndistortion->CanUndistort()) {
    return false;
  }
  return m_undistort;
}

void HandVu::CorrectDistortion(bool enable)
{
  if (enable && !m_pUndistortion->CanUndistort()) {
    throw HVException("can not undistort");
  }
  m_undistort = enable;
}

void HandVu::SetAdjustExposure(bool enable/*=true*/)
{
  if (enable == m_adjust_exposure) {
    return;
  }

  if (!m_pCameraController) {
    throw HVException("no camera controller set");
  }
  if (!enable) {
    m_pCameraController->SetCameraAutoExposure(true);
    m_adjust_exposure = false;
    return;
  }
  if (!m_pCameraController->CanAdjustExposure()) {
    throw HVException("camera controller incapable of setting exposure");
  }
  m_exposure_level = m_pCameraController->GetCurrentExposure();
  m_adjust_exposure = enable;
}

bool HandVu::CanAdjustExposure() const
{
  return (m_pCameraController && m_pCameraController->CanAdjustExposure());
}

bool HandVu::IsAdjustingExposure() const
{
  return m_adjust_exposure;
}

void HandVu::GetState(int id, HVState& state) const
{
  ASSERT(m_pClock);
  state.m_tstamp = m_sample_time;
  state.m_obj_id = id;

  if (!m_active) {
    state.m_tracked = false;
    state.m_recognized = false;
    return;
  }

  if (id==0) {
    state.m_tracked = m_tracking;
    state.m_recognized = m_recognized;
    if (m_tracking || m_recognized) {
      state.m_center_xpos = m_center_pos.x/(float)m_img_width;
      state.m_center_ypos = m_center_pos.y/(float)m_img_height;
      state.m_scale = m_center_depth?m_center_depth:m_last_match.scale;
    } else {
      state.m_center_xpos = -1;
      state.m_center_ypos = -1;
      state.m_scale = 0;
    }
    if (m_recognized) {
      state.m_posture = m_last_match.name;
    } else {
      state.m_posture = "";
    }
    
  } else if (id==1) {
    if (!m_id1_on) {
      throw HVException("object ID1 is not being searched for");
    }
    if (m_id1_x==-1 || m_id1_y==-1) {
      state.m_tracked = false;
      state.m_center_xpos = -1;
      state.m_center_ypos = -1;
      state.m_recognized = false;
      state.m_posture = "";
      
    } else {
      state.m_tracked = true;
      state.m_center_xpos = m_id1_x / m_img_width;
      state.m_center_ypos = m_id1_y / m_img_height;
      state.m_recognized = false;
      state.m_posture = "";
    }
    
  } else {
    throw HVException("nothing known about this ID");
  }
}

void HandVu::SetLogfile(const string& filename)
{
  FILE* fp = fopen(filename.c_str(), "r");
  while (fp) {
    fclose(fp);
    string str("HandVu will not destroy existing file, please delete:\n");
    str.append(filename);
    throw HVException(str);
//    fp = fopen(filename.c_str(), "r");
  }
  m_logfile_name = filename;

/* todo:  log stuff!
frame number
exposure control levels
latency result
state, actions -> dropped, skipped, processed
scan area
num features tracked, lost
coverage, TestLearned
*/
}

void HandVu::SetScanAreaVerified(const CRect& area)
{
  double maxwidth = m_img_width*m_pConductor->m_rc_max_scan_width;
  if (area.right-area.left > maxwidth) {
    double halfwidth = maxwidth/2.0;
    double center = (double)(area.right+area.left)/2.0;
    m_scan_area.left = (int)(center-halfwidth);
    m_scan_area.right = (int)(center+halfwidth);
  } else {
    m_scan_area.left = area.left;
    m_scan_area.right = area.right;
  }
  double maxheight = m_img_height*m_pConductor->m_rc_max_scan_height;
  if (area.bottom-area.top > maxheight) {
    double halfheight = maxheight/2.0;
    double center = (double)(area.top+area.bottom)/2.0;
    m_scan_area.top = (int)(center-halfheight);
    m_scan_area.bottom = (int)(center+halfheight);
  } else {
    m_scan_area.top = area.top;
    m_scan_area.bottom = area.bottom;
  }
  VERBOSE4(4, "Set scan area to %d, %d, %d, %d", 
           m_scan_area.left, m_scan_area.top, m_scan_area.right, m_scan_area.bottom);
}

