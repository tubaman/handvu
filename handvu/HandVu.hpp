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
  * $Id: HandVu.hpp,v 1.1 2005/10/30 21:16:04 matz Exp $
**/

// C++ header file for HandVu class

#if !defined(__HANDVU_HPP__INCLUDED_)
#define __HANDVU_HPP__INCLUDED_

#include <cv.h>
#include <string>
#include <deque>
using namespace std;

#include "Exceptions.h"
#include "Quadruple.h"
#include "Rect.h"
#include "Thread.h"


#ifdef USE_MFC
//using namespace std;	
// this prompts a bug in VC++ (friends and private member access)
// thus we have to declare all usages individually:
using std::string;
#else // USE_MFC
using namespace std;
#endif // USE_MFC

class CubicleWrapper;
class Segmentation;
class OpticalFlow;
class Skincolor;
class LearnedColor;
class Undistortion;
class VisionConductor;
class CamShift;
class DisplayCallback;


/*
#ifndef CScanMatch_DEFINED
#define CScanMatch_DEFINED
class CScanMatch {
public:
  CScanMatch() 
    : left(-1), top(-1), right(-1), bottom(-1),
    scale(-1), scale_x(-1), scale_y(-1), name("") {};
  CScanMatch(int _left, int _top, int _right, int _bottom,
             double _scale, double _scale_x, double _scale_y, string _name) 
    : left(_left), top(_top), right(_right), bottom(_bottom), 
      scale(_scale), scale_x(_scale_x), scale_y(_scale_y), name(_name) {};

  CRect AsRect() const { return CRect(left, top, right, bottom); }

  int         left, top, right, bottom;
  double      scale, scale_x, scale_y;
  string      name;
};
#endif // CScanMatch_DEFINED
*/

class CameraController {
 public:
  virtual double GetCurrentExposure() = 0;        // [0..1]
  // true if change has an effect, false if step is too small
  virtual bool SetExposure(double exposure) = 0;  // [0..1]
  virtual bool SetCameraAutoExposure(bool enable=true) = 0;
  virtual bool CanAdjustExposure() = 0;
};


#ifdef WIN32
typedef __int64 REFERENCE_TIME;
typedef REFERENCE_TIME RefTime;
#else
typedef long long RefTime;
#endif // WIN32
typedef vector<RefTime> RefTimeVector;

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

// times in micro-seconds (Usec)
class RefClock {
public:
  virtual RefTime GetCurrentTimeUsec() const = 0;
};

class GrabbedImage {
 public:
  GrabbedImage(IplImage* img, RefTime& sample_time, int bufferID) :
    m_img(img), m_sample_time(sample_time), m_bufferID(bufferID) {}
  IplImage* GetImage() { return m_img; }
  RefTime& GetSampleTime() { return m_sample_time; }
  int GetBufferID() { return m_bufferID; }
 protected:
  IplImage* m_img;
  RefTime   m_sample_time;
  int       m_bufferID;
};

/* state for an object such as the right hand
*/
class HVState {
 public:
  int        m_obj_id;
  bool       m_tracked;
  bool       m_recognized;
  double     m_center_xpos, m_center_ypos;
  double     m_scale;
  string     m_posture;
  RefTime    m_tstamp;
};

class HandVu {
 public:
  enum HVAction {         // specify recommendations to application:
    HV_INVALID_ACTION = 0,
    HV_PROCESS_FRAME = 1, // fully process and display the frame
    HV_SKIP_FRAME = 2,    // display but do not further process
    HV_DROP_FRAME = 3     // do not display the frame
  };
  enum {
    MAX_OVERLAY_LEVEL = 3
  };

  
 public:
  HandVu();
  ~HandVu();

  void Initialize(int width, int height, RefClock* pClock, 
		  CameraController* pCamCon);
  void LoadConductor(const string& filename);
  bool ConductorLoaded() const;
  void StartRecognition(int obj_id=0);
  void StopRecognition(int obj_id=0);
  bool IsActive() const { return m_active; }
  HVAction ProcessFrame(GrabbedImage& inOutImage,
                        const IplImage* rightImage=NULL);

  void AsyncSetup(int num_buffers, DisplayCallback* pDisplayCB);
  void AsyncProcessFrame(int id, RefTime& t);
  void AsyncGetImageBuffer(IplImage** pImg, int* pID);

  void GetState(int obj_id, HVState& state) const;

  void SetDetectionArea(int left, int top, int right, int bottom);
  void GetDetectionArea(CQuadruple& area) const;
  void RecomputeNormalLatency() { m_determine_normal_latency = true; }

  void SetOverlayLevel(int level);
  int GetOverlayLevel();

  void CorrectDistortion(bool enable=true);
  bool IsCorrectingDistortion() const;
  bool CanCorrectDistortion() const;
  
  void SetAdjustExposure(bool enable=true);
  bool CanAdjustExposure() const;
  bool IsAdjustingExposure() const;

  void SaveScannedArea(IplImage* pImg, string& picfile);
  void SaveImageArea(IplImage* pImg, CRect area, string& picfile);
  void SetSaveFilenameRoot(const string& fname_root);

  void SetDoTrack(bool do_track) { m_do_track = do_track; }

  void SetLogfile(const string& filename);
  void GetVersion(string& version, int verbosity) const;

 protected:
  void InitializeTracking();
  bool VerifyColor();
  bool DoDetection();
  bool DoTracking();
  bool DoRecognition();
  HVAction CheckLatency();
  void DrawOverlay();
  void CheckAndCorrectExposure();
  void FindSecondHand();
  void SetScanAreaVerified(const CRect& area);
  void KeepStatistics(HVAction action);
  void SendEvent() const;
  void CalculateDepth(const IplImage* rightImage, const CvRect& area);

  string GetNextSnapshotFilename(const string& base, const string& extension);
  void WriteAreaAsBMP(IplImage* pImg, const CRect& area, const string& picfile);
  void WriteAreaAsPPM_BGR(IplImage* pImg, const CRect& area, const string& picfile);
  void WriteAreaAsPPM_RGB(IplImage* pImg, const CRect& area, const string& picfile);
  void WriteAreaAsPGM_Gray(IplImage* pImg, const CRect& area, const string& picfile);

  void AsyncProcessor();
  friend void* asyncProcessor(void* arg);
  
  
 protected:
  // processing state
  bool                    m_active;
  bool                    m_tracking;
  bool                    m_recognized;

  CubicleWrapper*         m_pCubicle;
  Skincolor*              m_pSkincolor;
  LearnedColor*           m_pLearnedColor;
  OpticalFlow*            m_pOpticalFlow;
  VisionConductor*        m_pConductor;
  CamShift*               m_pCamShift;

  // general
  int                     m_video_width;
  int                     m_video_height;
  string                  m_camera_calib;
  int                     m_alignment_crosses;
  IplImage*               m_rgbImage;
  IplImage*               m_grayImages[2];
  IplImage*               m_depthImage;
  IplImage*               m_rightGrayImage;
  bool                    m_initialized;
  int                     m_img_width;
  int                     m_img_height;
  int                     m_overlay_level;
  CRect                   m_scan_area;
  CuScanMatch             m_last_match;
  string                  m_logfile_name; // currently unused
  string                  m_img_fname_root; // for saving image areas

  // Async (internal buffering)
  deque<GrabbedImage>     m_process_queue;
  vector<IplImage*>       m_ring_buffer;
  vector<bool>            m_ring_buffer_occupied;
  bool                    m_quit_thread;
  DisplayCallback*        m_pDisplayCallback;
  Thread*                 m_pAsyncThread;
  
  // undistortion
  bool                    m_undistort;
  Undistortion*           m_pUndistortion;

  // camera exposure control
  CameraController*       m_pCameraController;
  bool                    m_adjust_exposure;
  RefTime                 m_adjust_exposure_at_time;
  double                  m_exposure_level; //[0-1]

  // latency stuff
  RefTime                 m_sample_time;
  int                     m_num_succ_dropped_frames;
  RefTime                 m_max_normal_latency;
  RefTime                 m_max_abnormal_latency;
  RefTime                 m_t_start_processing;
  bool                    m_determine_normal_latency;
  RefTimeVector           m_last_latencies;
  RefTimeVector           m_frame_times;
  RefTimeVector           m_processed_frame_times;
  RefTimeVector           m_prcs_times;
  RefClock*               m_pClock;

  // detection
  CuScanMatch             m_dt_first_match;
  RefTime                 m_dt_first_match_time;

  // tracking
  bool                    m_do_track;
  int                     m_buf_indx_cycler;
  int                     m_prev_buf_indx;
  int                     m_curr_buf_indx;
  CvPoint2D32f            m_center_pos;
  double                  m_center_depth;
  RefTime                 m_time_to_learn_color;
  RefTime                 m_min_time_between_learning_color;

  // second object (left hand)
  bool                    m_id1_on;
  double                  m_id1_x, m_id1_y;
};


class DisplayCallback {
 public:
  virtual void Display(IplImage* img, HandVu::HVAction action) = 0;
};



#endif // __HANDVU_HPP__INCLUDED_

