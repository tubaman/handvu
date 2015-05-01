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
  * $Id: HandVuFilter.h,v 1.11 2005/10/30 23:00:43 matz Exp $
**/

#include "IHandVuFilter.h"
#include "ImageOverlay.h"

class CUserStudyA;
class CUserStudyB;

class HandVu;

typedef vector<HVFilterEventListener*> HVFEListenerVector;

class CHandVuFilter : public CTransInPlaceFilter,
                      public IHandVuFilter,
                      public RefClock,
                      public ISpecifyPropertyPages,
                      public CPersistStream
{

 public:

  DECLARE_IUNKNOWN;
  static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

  // Reveals HandVuFilter and ISpecifyPropertyPages
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

  // CPersistStream stuff
  HRESULT ScribbleToStream(IStream *pStream);
  HRESULT ReadFromStream(IStream *pStream);

  // Overrriden from CTransformFilter base class

  HRESULT Transform(IMediaSample *pMediaSample);
  HRESULT CheckInputType(const CMediaType *mtIn);
  HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
  HRESULT DecideBufferSize(IMemAllocator *pAlloc,
                           ALLOCATOR_PROPERTIES *pProperties);
  HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

  // These implement the custom IHandVuFilter interface
  STDMETHODIMP GetHandVuFilterParams(HandVuFilterParams& params) const;
  STDMETHODIMP SetHandVuFilterParams(const HandVuFilterParams& params);
  STDMETHODIMP Initialize(int width, int height, int iPixelSize, CameraController* pCamCon);
  STDMETHODIMP LoadConductor(const string& filename);
  STDMETHODIMP ConductorLoaded(bool* pLoaded);
  STDMETHODIMP StartRecognition(int obj_id);
  STDMETHODIMP StopRecognition(int obj_id);
  STDMETHODIMP IsActive(bool* pActive);
  STDMETHODIMP GetState(int obj_id, hvState& state);
  STDMETHODIMP GetCenterPosition(double* pX, double* pY);
  STDMETHODIMP GetRecognizedPosture(string& descriptor);
  STDMETHODIMP SetOverlayLevel(int level);
  STDMETHODIMP GetOverlayLevel(int* pLevel);
  STDMETHODIMP CorrectDistortion(bool enable);
  STDMETHODIMP CanCorrectDistortion(bool* pPossible);
  STDMETHODIMP IsCorrectingDistortion(bool* pOn);
  STDMETHODIMP SetDetectionArea(int left, int top, int right, int bottom);
  STDMETHODIMP RecomputeNormalLatency();
  STDMETHODIMP SetAdjustExposure(bool enable);
  STDMETHODIMP CanAdjustExposure(bool* pLoaded);
  STDMETHODIMP IsAdjustingExposure(bool* pEnabled);

  STDMETHODIMP AddListener(HVFilterEventListener* pHVFListener);
  STDMETHODIMP RemoveListener(HVFilterEventListener* pHVFListener);
  STDMETHODIMP ToggleMaintenanceApp();
  STDMETHODIMP ToggleFDLOnly();
  STDMETHODIMP OnLButtonUp(UINT nFlags, double x, double y);
  STDMETHODIMP OnMouseMove(UINT nFlags, double x, double y);
  STDMETHODIMP TakeSnapshot();
  STDMETHODIMP GetVersion(string& version);

#if defined(HAVE_USER_STUDY)
  STDMETHODIMP AbortStudyHV1Task();
  STDMETHODIMP AbortStudyHV1Session();
  STDMETHODIMP StartStudyHV1();
  STDMETHODIMP StartStudyHV2();
#endif
  
  STDMETHODIMP SetTimecodeReader(IUnknown* tcr);

  STDMETHODIMP SetVerbosity(int level, const string& logfilename);

  // ISpecifyPropertyPages interface
  STDMETHODIMP GetPages(CAUUID *pPages);
    
  // CPersistStream override
  STDMETHODIMP GetClassID(CLSID *pClsid);

  virtual RefTime GetCurrentTimeUsec() const;
  virtual RefTime GetSampleTimeUsec() const;
    
 private:
  // Constructor
  CHandVuFilter(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr,
                bool bModifiesData=true);
  ~CHandVuFilter();
    
  // Look after doing the special effect
  BOOL CanPerformHandVuFilter(const CMediaType *pMediaType) const;
  void HandVuFramenumber();
  string GetTimecode();
  void SaveImageArea(IplImage* pImg);


 private:
  // Private play critical section
  mutable CCritSec         m_HandVuFilterLock;

  HandVuFilterParams       m_params;
  IplImage*                m_pColorHeader;
  bool                     m_img_bottom_up;
  int                      m_cxImage, m_cyImage; // image size, once we saw one

  CComPtr<IAMTimecodeReader> m_pIAMTCReader;

  RefTime                  m_t_sample;
  HVFEListenerVector       m_HVFListenerPtrs;

  CImageOverlay*           m_pOverlay;
#ifdef HAVE_USER_STUDY
  CUserStudyA*             m_pUserStudyA;
  CUserStudyB*             m_pUserStudyB;
#endif
  bool                     m_show_maintenanceapp;
  bool                     m_take_one_snapshot;
  bool                     m_is_initialized;

  bool                     m_FDL_only;
  bool                     m_FDL_is_initialized;
}; // HandVuFilter

