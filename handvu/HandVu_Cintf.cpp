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
  * $Id: HandVu_Cintf.cpp,v 1.3 2005/10/30 23:00:43 matz Exp $
**/

// HandVu_Cintf.cpp: C interface to the C++ classes in HandVu.cpp
//

#include "Common.h"
#include "HandVu.hpp"
#include "HandVu.h"
#include "GestureServer.h"


#if defined(WIN32)

#if defined(GetMessage)
#undef GetMessage
#endif

#include <time.h>
#define CLOCKS_PER_USEC ((double)(CLOCKS_PER_SEC)/1000000.0)
class RefClockWindows : public RefClock {
public:
  virtual RefTime GetCurrentTimeUsec() const;
};
RefTime RefClockWindows::GetCurrentTimeUsec() const
{
  clock_t ct = clock();
  return (RefTime) (ct/CLOCKS_PER_USEC);
}
#define RefClockArch RefClockWindows

#else //WIN32

#define CLOCKS_PER_USEC ((double)(CLOCKS_PER_SEC)/1000000.0)
class RefClockLinux : public RefClock {
public:
  virtual RefTime GetCurrentTimeUsec() const;
};
RefTime RefClockLinux::GetCurrentTimeUsec() const
{
  clock_t ct = clock();
  return (RefTime) (ct/CLOCKS_PER_USEC);
}
#define RefClockArch RefClockLinux

#endif //WIN32


class DisplayCallbackCintf : public DisplayCallback {
 public:
  DisplayCallbackCintf(void (*cb)(IplImage* img, hvAction action)) :
    m_cb(cb) {}
  virtual void Display(IplImage* img, HandVu::HVAction action);
 protected:
  void (*m_cb)(IplImage*, hvAction);
};
void DisplayCallbackCintf::Display(IplImage* img, HandVu::HVAction action)
{
  CV_FUNCNAME( "hvDisplayCallback" ); // declare cvFuncName
  __BEGIN__;
  hvAction act;
  switch (action) {
  case HandVu::HV_INVALID_ACTION:
    act = HV_INVALID_ACTION; break;
  case HandVu::HV_PROCESS_FRAME:
    act = HV_PROCESS_FRAME; break;
  case HandVu::HV_SKIP_FRAME:
    act = HV_SKIP_FRAME; break;
  case HandVu::HV_DROP_FRAME:
    act = HV_DROP_FRAME; break;
  default:
    CV_ERROR(CV_StsError, "unknown HVAction code");
  }  
  m_cb(img, act);
  __END__;
}


typedef GestureServer* GestureServerPtr;
vector<GestureServerPtr> g_pservers;
HandVu* g_pHandVu = NULL;
RefClockArch* g_pClock = NULL;
DisplayCallbackCintf* g_displayCallback = NULL;

void hvInitialize(int image_width, int image_height)
{
  CV_FUNCNAME( "hvInitialize" ); // declare cvFuncName
  __BEGIN__;
  if (image_width<0 || image_height<0) {
    CV_ERROR(CV_BadImageSize, "negative image width or height");
  }
  if (g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu already initialized");
  }
  try {
    g_pHandVu = new HandVu();
    g_pClock = new RefClockArch();
    g_pHandVu->Initialize(image_width, image_height, g_pClock, NULL);
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

void hvUninitialize()
{
  CV_FUNCNAME( "hvUninitialize" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    g_pHandVu->~HandVu();
    g_pHandVu = NULL;
    delete g_pClock;
    g_pClock = NULL;
    if (g_displayCallback) {
      delete g_displayCallback;
    }

    // gesture servers
    for (int i=0; i<(int)g_pservers.size(); i++) delete g_pservers[i];
    g_pservers.clear();

  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

void hvLoadConductor(const string& filename)
{
  CV_FUNCNAME( "hvLoadConductor" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    g_pHandVu->LoadConductor(filename);
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

bool hvConductorLoaded()
{
  CV_FUNCNAME( "hvConductorLoaded" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    return g_pHandVu->ConductorLoaded();
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}


void hvStartRecognition(int obj_id)
{
  CV_FUNCNAME( "hvStartRecognition" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    g_pHandVu->StartRecognition(obj_id);
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

void hvStopRecognition(int obj_id)
{
  CV_FUNCNAME( "hvStopRecognition" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    g_pHandVu->StopRecognition(obj_id);
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

hvAction hvProcessFrame(IplImage* inOutImage, IplImage* rightImage)
{
  CV_FUNCNAME( "hvProcessFrame" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    RefTime t = g_pClock->GetCurrentTimeUsec();
    GrabbedImage gi(inOutImage, t, -1);
    HandVu::HVAction action = g_pHandVu->ProcessFrame(gi, rightImage);
    switch (action) {
      case HandVu::HV_INVALID_ACTION:
        return HV_INVALID_ACTION;
      case HandVu::HV_PROCESS_FRAME:
        return HV_PROCESS_FRAME;
      case HandVu::HV_SKIP_FRAME:
        return HV_SKIP_FRAME;
      case HandVu::HV_DROP_FRAME:
        return HV_DROP_FRAME;
      default:
        CV_ERROR(CV_StsError, "unknown HandVu::HVAction");
    }

  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

bool hvIsActive()
{
  CV_FUNCNAME( "hvIsActive" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    bool active = g_pHandVu->IsActive();
    return active;
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

void hvAsyncSetup(int num_buffers, void (*cb)(IplImage* img, hvAction action))
{
  CV_FUNCNAME( "hvAsyncSetup" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    if (g_displayCallback) {
      delete g_displayCallback;
    }
    g_displayCallback = new DisplayCallbackCintf(cb);
    g_pHandVu->AsyncSetup(num_buffers, g_displayCallback);
    
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

void hvAsyncGetImageBuffer(IplImage** pImage, int* pBufferID)
{
  CV_FUNCNAME( "hvAsyncGetImageBuffer" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    g_pHandVu->AsyncGetImageBuffer(pImage, pBufferID);
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

void hvAsyncProcessFrame(int bufferID)
{
  CV_FUNCNAME( "hvAsyncProcessFrame" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    RefTime t = g_pClock->GetCurrentTimeUsec();
    g_pHandVu->AsyncProcessFrame(bufferID, t);
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}


void hvGetState(int obj_id, hvState& state)
{
  CV_FUNCNAME( "hvGetState" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    HVState hsta;
    g_pHandVu->GetState(obj_id, hsta);
    state.obj_id = hsta.m_obj_id;
    state.tracked = hsta.m_tracked;
    state.recognized = hsta.m_recognized;
    state.center_xpos = hsta.m_center_xpos;
    state.center_ypos = hsta.m_center_ypos;
    state.scale = hsta.m_scale;
    state.posture = hsta.m_posture;
    state.tstamp = hsta.m_tstamp;
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}


void hvSetDetectionArea(int left, int top, int right, int bottom)
{
  CV_FUNCNAME( "hvSetDetectionArea" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    g_pHandVu->SetDetectionArea(left, top, right, bottom);
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

void hvGetDetectionArea(int* pLeft, int* pTop, int* pRight, int* pBottom)
{
  CV_FUNCNAME( "hvAsyncProcessFrame" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    CQuadruple area;
    g_pHandVu->GetDetectionArea(area);
    *pLeft = (int) area.left;
    *pTop = (int) area.top;
    *pRight = (int) area.right;
    *pBottom = (int) area.bottom;
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

void hvRecomputeNormalLatency()
{
  CV_FUNCNAME( "hvRecomputeNormalLatency" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    g_pHandVu->RecomputeNormalLatency();
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

void hvSetOverlayLevel(int level)
{
  CV_FUNCNAME( "hvSetOverlayLevel" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    g_pHandVu->SetOverlayLevel(level);
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

int hvGetOverlayLevel()
{
  CV_FUNCNAME( "hvRecomputeNormalLatency" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    int level = g_pHandVu->GetOverlayLevel();
    return level;
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}


void hvCorrectDistortion(bool enable)
{
  CV_FUNCNAME( "hvCorrectDistortion" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    g_pHandVu->CorrectDistortion(enable);
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

bool hvIsCorrectingDistortion()
{
  CV_FUNCNAME( "hvIsCorrectingDistortion" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    return g_pHandVu->IsCorrectingDistortion();
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

bool hvCanCorrectDistortion()
{
  CV_FUNCNAME( "hvCanCorrectDistortion" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    return g_pHandVu->CanCorrectDistortion();
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

void hvSetAdjustExposure(bool enable)
{
  CV_FUNCNAME( "hvSetAdjustExposure" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    g_pHandVu->SetAdjustExposure(enable);
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

bool hvCanAdjustExposure()
{
  CV_FUNCNAME( "hvCanAdjustExposure" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    return g_pHandVu->CanAdjustExposure();
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

bool hvIsAdjustingExposure()
{
  CV_FUNCNAME( "hvIsAdjustingExposure" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    return g_pHandVu->IsAdjustingExposure();
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}


void hvSetLogfile(const string& filename)
{
  CV_FUNCNAME( "hvSetLogfile" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    g_pHandVu->SetLogfile(filename);
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

void hvSaveScannedArea(IplImage* pImg, string& picfile)
{
  CV_FUNCNAME( "hvSaveScannedArea" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    g_pHandVu->SaveScannedArea(pImg, picfile);
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

void hvSaveImageArea(IplImage* pImg, int left, int top, int right, int bottom, string& picfile)
{
  CV_FUNCNAME( "hvSaveImageArea" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    g_pHandVu->SaveImageArea(pImg, CRect(left, top, right, bottom), picfile);
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

void hvSetSaveFilenameRoot(const string& fname_root)
{
  CV_FUNCNAME( "hvSetSaveFilenameRoot" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    g_pHandVu->SetSaveFilenameRoot(fname_root);
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}


void hvSetDoTrack(bool do_track)
{
  CV_FUNCNAME( "hvSetDoTrack" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    g_pHandVu->SetDoTrack(do_track);
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}


void hvStartGestureServer(int port, int max_num_clients)
{
  CV_FUNCNAME( "hvStartGestureServer" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    g_pservers.push_back(new GestureServerStream(port, max_num_clients));
    g_pservers.back()->Start();
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

void hvStartOSCServer(const string& desthost, int destport)
{
  CV_FUNCNAME( "hvStartOSCServer" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    g_pservers.push_back(new GestureServerOSC(desthost, destport));
    g_pservers.back()->Start();
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

void hvStopGestureServer(int /*port*/)
{
  CV_FUNCNAME( "hvStopGestureServer" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    throw HVException("sorry, stop server not implemented");
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

void hvStopOSCServer(const string& /*desthost*/, int /*destport*/)
{
  CV_FUNCNAME( "hvStopOSCServer" ); // declare cvFuncName
  __BEGIN__;
  if (!g_pHandVu) {
    CV_ERROR(CV_StsError, "HandVu not initialized");
  }
  try {
    throw HVException("sorry, stop server not implemented");
  } catch (HVException& hve) {
    CV_ERROR(CV_StsError, hve.GetMessage().c_str());
  }
  __END__;
}

void HandVu::SendEvent() const
{
  HVState state;
  GetState(0, state);
  for (int s=0; s<(int)g_pservers.size(); s++) {
    g_pservers[s]->Send(state);
  }
}


/** verbosity: 0 minimal, 3 maximal
*/
void hvGetVersion(string& version, int verbosity)
{
//  todo: version = HV_CURRENT_VERSION_STRING;
  version = "handvu version beta2";
  if (verbosity>=1) {
#if defined(WIN32)
    version = version + ", win32";
#elif defined(TARGET_SYSTEM)
    version = version + ", "TARGET_SYSTEM;
#else
#error TARGET_SYSTEM must be defined
#endif

#if defined(DEBUG)
    version = version + " debug";
#endif

#ifdef USE_MFC
    version = version + ", MFC";
#endif
  }

  if (verbosity>=2) {
    version = version + ", built on "__DATE__" at "__TIME__;
  }

  if (verbosity>=3) {
    version = version + "\nCVS id: $Id: HandVu_Cintf.cpp,v 1.3 2005/10/30 23:00:43 matz Exp $";
  }

  string cubicles_version;
  cuGetVersion(cubicles_version, verbosity);
  version = version + "\n" + cubicles_version;
}

