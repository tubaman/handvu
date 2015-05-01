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
  * $Id: IHandVuFilter.h,v 1.12 2005/10/30 23:00:43 matz Exp $
**/

#ifndef __IHANDVUFILTER__
#define __IHANDVUFILTER__

#include "HandVu.h"
#include "HandVu.hpp"


typedef struct _HandVuFilterParams
{
  int		       immediate_apply; // how to apply changes
} HandVuFilterParams;

class CameraController;


class HVFilterEventListener {
public:
  virtual void FrameTransformed(bool processed) = 0;
};


#ifdef __cplusplus
extern "C" {
#endif

  // {0C2E9517-6BC4-4b64-933F-B3BEF0074868}
  DEFINE_GUID(IID_IHandVuFilter, 
  0xc2e9517, 0x6bc4, 0x4b64, 0x93, 0x3f, 0xb3, 0xbe, 0xf0, 0x7, 0x48, 0x68);


  DECLARE_INTERFACE_(IHandVuFilter, IUnknown)
  {
    STDMETHOD(GetHandVuFilterParams)  (THIS_
                                       HandVuFilterParams& params
                                       ) const PURE;

    STDMETHOD(SetHandVuFilterParams)  (THIS_
                                       const HandVuFilterParams& params
                                       ) PURE;

    STDMETHOD (AddListener) (THIS_ HVFilterEventListener* pHVFListener) PURE;
    STDMETHOD (RemoveListener) (THIS_ HVFilterEventListener* pHVFListener) PURE;
    STDMETHOD (ToggleMaintenanceApp) (THIS_) PURE;
    STDMETHOD (ToggleFDLOnly) (THIS_) PURE;

    STDMETHOD (Initialize) (THIS_ int width, int height, int iPixelSize, CameraController* pCC) PURE;
    STDMETHOD (LoadConductor) (THIS_ const string& filename) PURE;
    STDMETHOD (ConductorLoaded) (THIS_ bool* pLoaded) PURE;
    STDMETHOD (StartRecognition) (THIS_ int obj_id) PURE;
    STDMETHOD (StopRecognition) (THIS_ int obj_id) PURE;
    STDMETHOD (IsActive) (THIS_ bool* pActive) PURE;
    STDMETHOD (GetState) (THIS_ int obj_id, hvState& state) PURE;
    STDMETHOD (SetOverlayLevel) (THIS_ int level) PURE;
    STDMETHOD (GetOverlayLevel) (THIS_ int* pLevel) PURE;
    STDMETHOD (CorrectDistortion) (THIS_ bool enable) PURE;
    STDMETHOD (CanCorrectDistortion) (THIS_ bool* pPossible) PURE;
    STDMETHOD (IsCorrectingDistortion) (THIS_ bool* pOn) PURE;
    STDMETHOD (SetDetectionArea) (THIS_ int left, int top, int right, int bottom) PURE;
    STDMETHOD (RecomputeNormalLatency) (THIS_ ) PURE;
    STDMETHOD (SetAdjustExposure) (THIS_ bool enable) PURE;
    STDMETHOD (CanAdjustExposure) (THIS_ bool* pPossible) PURE;
    STDMETHOD (IsAdjustingExposure) (THIS_ bool* pEnabled) PURE;

    STDMETHOD (OnLButtonUp) (THIS_ UINT nFlags, double x, double y) PURE;
	  STDMETHOD (OnMouseMove) (THIS_ UINT nFlags, double x, double y) PURE;
	  STDMETHOD (TakeSnapshot) (THIS_) PURE;
    STDMETHOD (GetVersion) (THIS_ string& version) PURE;

#if defined(HAVE_USER_STUDY)
	  STDMETHOD (AbortStudyHV1Task) (THIS_) PURE;
	  STDMETHOD (AbortStudyHV1Session) (THIS_) PURE;
	  STDMETHOD (StartStudyHV1) (THIS_) PURE;
	  STDMETHOD (StartStudyHV2) (THIS_) PURE;
#endif

	  STDMETHOD (SetTimecodeReader) (THIS_ IUnknown* tcr) PURE;

	  STDMETHOD (SetVerbosity) (THIS_ int level, const string& logfilename) PURE;
  };
  
#ifdef __cplusplus
}
#endif

#endif // __IHANDVUFILTER__

  
