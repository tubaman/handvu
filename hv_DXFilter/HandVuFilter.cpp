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
  * $Id: HandVuFilter.cpp,v 1.22 2005/10/30 23:00:43 matz Exp $
**/

#include "StdAfx.h"
#include <initguid.h>
#if (1100 > _MSC_VER)
#include <olectlid.h>
#else
#include <olectl.h>
#endif

//#define USE_FDL

#include "HandVuFilterGUIDs.h"
#include "Common.h"
#include "HandVuFilter.h"
#include "HandVuFilterProp.h"
#include "MaintenanceApp.h"
#ifdef USE_FDL
#include "FrameDataLib.h"
#endif USE_FDL

#ifdef HAVE_USER_STUDY
#include "../hv_UserStudy/UserStudyA.h"
#include "../hv_UserStudy/UserStudyB.h"
#endif HAVE_USER_STUDY

#include "HandVu.h"

// wrong #define causes a warning in ipl.h ... annoying:
//#undef _VXWORKS
#include <CV.h>
#include "resource.h"

#ifndef M_PI
#define M_PI 3.141592653589793
#endif /* M_PI */





// Setup information

const AMOVIESETUP_MEDIATYPE sudPinTypes =
  {
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
  };

const AMOVIESETUP_PIN sudpPins[] =
  {
    { L"Input",             // Pins string name
      FALSE,                // Is it rendered
      FALSE,                // Is it an output
      FALSE,                // Are we allowed none
      FALSE,                // And allowed many
      &CLSID_NULL,          // Connects to filter
      NULL,                 // Connects to pin
      1,                    // Number of types
      &sudPinTypes          // Pin information
    },
    { L"Output",            // Pins string name
      FALSE,                // Is it rendered
      TRUE,                 // Is it an output
      FALSE,                // Are we allowed none
      FALSE,                // And allowed many
      &CLSID_NULL,          // Connects to filter
      NULL,                 // Connects to pin
      1,                    // Number of types
      &sudPinTypes          // Pin information
    }
  };

const AMOVIESETUP_FILTER sudHandVuFilter =
  {
    &CLSID_HandVuFilter,         // Filter CLSID
    L"HandVuFilter",         // String name
    MERIT_DO_NOT_USE,       // Filter merit
    2,                      // Number of pins
    sudpPins                // Pin information
  };


// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance

CFactoryTemplate g_Templates[] = {
  { L"HandVuFilter (Filter)"
    , &CLSID_HandVuFilter
    , CHandVuFilter::CreateInstance
    , NULL
    , &sudHandVuFilter }
  ,
  { L"HandVuFilter (Property Page)"
    , &CLSID_HandVuFilterPropertyPage
    , CHandVuFilterProperties::CreateInstance }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


//
// DllRegisterServer
//
// Handles sample registry and unregistry
//
STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
} // DllRegisterServer


//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
} // DllUnregisterServer


//
// Constructor
//
CHandVuFilter::CHandVuFilter(TCHAR *tszName,
                               LPUNKNOWN punk,
                               HRESULT *phr,
                               bool bModifiesData /* = true */)
: CTransInPlaceFilter(tszName, punk, CLSID_HandVuFilter, phr, bModifiesData),
  m_img_bottom_up(false),
  m_cxImage(-1),
  m_cyImage(-1),
  m_show_maintenanceapp(false),
  m_FDL_only(false),
  m_take_one_snapshot(false),
  m_is_initialized(false),
  m_FDL_is_initialized(false),
  m_pColorHeader(NULL),
  m_pOverlay(NULL),
#ifdef HAVE_USER_STUDY
  m_pUserStudyA(NULL),
  m_pUserStudyB(NULL),
#endif
  CPersistStream(punk, phr)
{
  string root_dir("C:\\hv_tmp");
  _mkdir(root_dir.c_str());
  string fname_root(root_dir + "\\hvsnap_");
  hvSetSaveFilenameRoot(fname_root);
  m_params.immediate_apply = true;

  // debugging:
  DbgSetModuleLevel(LOG_CUSTOM1, 3);
} // (Constructor)

CHandVuFilter::~CHandVuFilter()
{
  hvUninitialize();
}

//
// CreateInstance
//
// Provide the way for COM to create a HandVuFilter object
//
CUnknown *CHandVuFilter::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
  CHandVuFilter *pNewObject =
    new CHandVuFilter(NAME("HandVuFilter"), punk, phr);
  if (pNewObject == NULL) {
    *phr = E_OUTOFMEMORY;
  }
  return pNewObject;
} // CreateInstance


//
// NonDelegatingQueryInterface
//
// Reveals IHandVuFilter and ISpecifyPropertyPages
//
STDMETHODIMP CHandVuFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
  CheckPointer(ppv,E_POINTER);

  if (riid == IID_IHandVuFilter) {
    return GetInterface((IHandVuFilter *) this, ppv);
  } else if (riid == IID_ISpecifyPropertyPages) {
    return GetInterface((ISpecifyPropertyPages *) this, ppv);
  } else {
    return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
  }
} // NonDelegatingQueryInterface


//
// Transform (in place)
//
HRESULT CHandVuFilter::Transform(IMediaSample *pMediaSample)
{
#if 0
  static dropme = false;
  if (dropme) {
    dropme = false;
    m_bSampleSkipped = true;
    return S_FALSE;
  } else {
    dropme = true;
  }
#endif //0 

  CAutoLock lock(&m_HandVuFilterLock);
  AM_MEDIA_TYPE* pType = &m_pInput->CurrentMediaType();
  VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pType->pbFormat;

  BYTE *pData;                // Pointer to the actual image buffer
  long lDataLen;              // Holds length of any given sample

  pMediaSample->GetPointer(&pData);
  lDataLen = pMediaSample->GetSize();
  m_bSampleSkipped = false;

  if (!m_is_initialized) {
    VERBOSE0(3, "HandVuFilter: init during playback");

    // Get the image properties from the BITMAPINFOHEADER
    int iPixelSize = pvi->bmiHeader.biBitCount / 8;
    int width      = pvi->bmiHeader.biWidth;
    int height     = pvi->bmiHeader.biHeight;
    
    HRESULT hr = Initialize(width, height, iPixelSize, NULL);
    if (!SUCCEEDED(hr)) {
      throw HVException("HandVuFilter: concurrent initialization");
    }
  }

  { // test if change from initialization
    int width      = pvi->bmiHeader.biWidth;
    int height     = pvi->bmiHeader.biHeight;
    bool img_bottom_up = true;
    if (height<0) {
      // norm for RGB images
      img_bottom_up = false;
      height = -height;
    }
    if (width!=m_cxImage || height!=m_cyImage || img_bottom_up!=m_img_bottom_up) {
      VERBOSE0(1, "HandVuFilter: format changed!");
      throw HVException("HandVuFilter: format changed!");
    }
  }

  m_pColorHeader->imageData = (char*) pData;
  m_pColorHeader->imageSize = lDataLen;

  REFERENCE_TIME st_start, st_end;
  pMediaSample->GetTime(&st_start, &st_end);
  m_t_sample = st_start/10;

#ifdef HAVE_USER_STUDY
  // check DV input for marks
  if (m_pUserStudyB) {
    if (!m_pIAMTCReader) {
      AfxMessageBox("need time code reader for StudyHV2");
    } else {
      m_pUserStudyB->AtTimecode(GetTimecode());
    }
  }
#endif

  if (m_img_bottom_up) {
    cvMirror(m_pColorHeader, NULL, 0); // flip on horizontal axis
  }

  // ------- main library call ---------
  VERBOSE0(5, "HandVuFilter: will process frame");
  hvAction action = HV_INVALID_ACTION;
  action = hvProcessFrame(m_pColorHeader);
  VERBOSE0(5, "HandVuFilter: HandVu finished processing frame");
  // -------

  for (int i=0; i<(int)m_HVFListenerPtrs.size(); i++) {
    m_HVFListenerPtrs[i]->FrameTransformed(action!=HV_DROP_FRAME);
  }

  if (action==HV_DROP_FRAME) {
    // HandVu recommends dropping the frame entirely
#ifdef HAVE_USER_STUDY
    if (m_pUserStudyA) {
      m_pUserStudyA->DontDraw(m_pColorHeader);
    }
#endif
    m_bSampleSkipped = true;
    VERBOSE0(3, "HandVuFilter: dropping frame");
    return S_FALSE;
  } else if (action==HV_SKIP_FRAME) {
    // HandVu recommends displaying the frame, but not doing any further
    // processing on it - keep going
  } else if (action==HV_PROCESS_FRAME) {
    // full processing was done and is recommended for following steps;
    // keep going
  } else {
    ASSERT(0); // unknown action
  }

  if (m_show_maintenanceapp) {
    ASSERT(m_pOverlay);
    //
    // overlay
    //
    hvState state;
    hvGetState(0, state);
    COverlayEvent event;
    event.m_ptr1_tracked = state.tracked;
    event.m_ptr1_recognized = state.recognized;
    event.m_ptr1_x = state.center_xpos;
    event.m_ptr1_y = state.center_ypos;
    event.m_ptr1_type = state.posture;
    m_pOverlay->Tracked(event);
    m_pOverlay->Draw(m_pColorHeader);
  }

#ifdef HAVE_USER_STUDY
  if (m_pUserStudyA) {
    hvVState state;
    hvGetState(0, state);
    COverlayEvent event;
    event.m_ptr1_tracked = state.tracked;
    event.m_ptr1_recognized = state.recognized;
    event.m_ptr1_x = state.center_xpos;
    event.m_ptr1_y = state.center_ypos;
    event.m_ptr1_type = state.posture;
    m_pUserStudyA->Tracked(event);
    m_pUserStudyA->Draw(m_pColorHeader);
  }

  if (m_pUserStudyB) {
    hvState state;
    hvGetState(0, state);
    COverlayEvent event;
    event.m_ptr1_tracked = state.tracked;
    event.m_ptr1_recognized = state.recognized;
    event.m_ptr1_x = state.center_xpos;
    event.m_ptr1_y = state.center_ypos;
    event.m_ptr1_type = state.posture;
    m_pUserStudyB->Tracked(event);
    m_pUserStudyB->Draw(m_pColorHeader);
  }
#endif

  if (m_take_one_snapshot) {
    m_take_one_snapshot = false;
    SaveImageArea(m_pColorHeader);
  }

#ifdef USE_FDL
  if (m_FDL_only && action==HV_PROCESS_FRAME) {
    if (!m_FDL_is_initialized) {
      int iPixelSize = pvi->bmiHeader.biBitCount / 8;
      FDL_Initialize(m_cxImage, m_cyImage, iPixelSize);
      m_FDL_is_initialized = true;
    }
    FDL_PutImage((BYTE*) m_pColorHeader->imageData);
    m_bSampleSkipped = true;
    VERBOSE0(4, "HandVuFilter: frame sent to FDL, no display");
    return S_FALSE;
  }
#endif USE_FDL

  if (m_img_bottom_up) {
    cvMirror(m_pColorHeader, NULL, 0); // flip on horizontal axis
  }

  VERBOSE0(3, "HandVuFilter: processed frame");

  return NOERROR;
} // Transform (in place)


RefTime CHandVuFilter::GetSampleTimeUsec() const
{
  return m_t_sample;
}

RefTime CHandVuFilter::GetCurrentTimeUsec() const
{
  if (m_tStart.m_time==0) {
    return 0;
  }

  CRefTime t_curr;
  ((CHandVuFilter*)this)->StreamTime(t_curr);
  return t_curr.m_time/10;
}




// Check the input type is OK - return an error otherwise

HRESULT CHandVuFilter::CheckInputType(const CMediaType *mtIn)
{
  // check this is a VIDEOINFOHEADER type
  if (*mtIn->FormatType() != FORMAT_VideoInfo) {
    return E_INVALIDARG;
  }

  // Can we transform this type
  if (CanPerformHandVuFilter(mtIn)) {
    return NOERROR;
  }
  return E_FAIL;
}


//
// Checktransform
//
// Check a transform can be done between these formats
//
HRESULT CHandVuFilter::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
  if (CanPerformHandVuFilter(mtIn)) {
    if (*mtIn == *mtOut) {
      return NOERROR;
    }
  }
  return E_FAIL;
} // CheckTransform


//
// DecideBufferSize
//
// Tell the output pin's allocator what size buffers we
// require. Can only do this when the input is connected
//
HRESULT CHandVuFilter::DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties)
{
  // Is the input pin connected

  if (m_pInput->IsConnected() == FALSE) {
    return E_UNEXPECTED;
  }

  ASSERT(pAlloc);
  ASSERT(pProperties);
  HRESULT hr = NOERROR;

  pProperties->cBuffers = 1;
  pProperties->cbBuffer = m_pInput->CurrentMediaType().GetSampleSize();
  ASSERT(pProperties->cbBuffer);

  // Ask the allocator to reserve us some sample memory, NOTE the function
  // can succeed (that is return NOERROR) but still not have allocated the
  // memory that we requested, so we must check we got whatever we wanted

  ALLOCATOR_PROPERTIES Actual;
  hr = pAlloc->SetProperties(pProperties,&Actual);
  if (FAILED(hr)) {
    return hr;
  }

  ASSERT( Actual.cBuffers == 1 );

  if (pProperties->cBuffers > Actual.cBuffers ||
      pProperties->cbBuffer > Actual.cbBuffer) {
    return E_FAIL;
  }
  return NOERROR;

} // DecideBufferSize


//
// GetMediaType
//
// I support one type, namely the type of the input pin
// This type is only available if my input is connected
//
HRESULT CHandVuFilter::GetMediaType(int iPosition, CMediaType *pMediaType)
{
  // Is the input pin connected

  if (m_pInput->IsConnected() == FALSE) {
    return E_UNEXPECTED;
  }

  // This should never happen

  if (iPosition < 0) {
    return E_INVALIDARG;
  }

  // Do we have more items to offer

  if (iPosition > 0) {
    return VFW_S_NO_MORE_ITEMS;
  }

  *pMediaType = m_pInput->CurrentMediaType();
  return NOERROR;

} // GetMediaType


//
// CanPerformHandVuFilter
//
// Check if this is a RGB24 true colour format
//
BOOL CHandVuFilter::CanPerformHandVuFilter(const CMediaType *pMediaType) const
{
  if (IsEqualGUID(*pMediaType->Type(), MEDIATYPE_Video)) {
    if (IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_RGB24)) {
      VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pMediaType->Format();
      return (pvi->bmiHeader.biBitCount == 24);
    }
  }
  return FALSE;

} // CanPerformHandVuFilter


#define WRITEOUT(var)  hr = pStream->Write(&var, sizeof(var), NULL); \
		       if (FAILED(hr)) return hr;

#define READIN(var)    hr = pStream->Read(&var, sizeof(var), NULL); \
		       if (FAILED(hr)) return hr;


//
// GetClassID
//
// This is the only method of IPersist
//
STDMETHODIMP CHandVuFilter::GetClassID(CLSID *pClsid)
{
  return CBaseFilter::GetClassID(pClsid);
} // GetClassID


//
// ScribbleToStream
//
// Overriden to write our state into a stream
//
HRESULT CHandVuFilter::ScribbleToStream(IStream *pStream)
{
  return NOERROR;

} // ScribbleToStream


//
// ReadFromStream
//
// Likewise overriden to restore our state from a stream
//
HRESULT CHandVuFilter::ReadFromStream(IStream *pStream)
{
  return NOERROR;

} // ReadFromStream


//
// GetPages
//
// Returns the clsid's of the property pages we support
//
STDMETHODIMP CHandVuFilter::GetPages(CAUUID *pPages)
{
  pPages->cElems = 1;
  pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
  if (pPages->pElems == NULL) {
    return E_OUTOFMEMORY;
  }
  *(pPages->pElems) = CLSID_HandVuFilterPropertyPage;
  return NOERROR;

} // GetPages


//
// GetHandVuFilterParams
//
STDMETHODIMP
CHandVuFilter::GetHandVuFilterParams(HandVuFilterParams& params) const
{
  CAutoLock cAutolock(&m_HandVuFilterLock);

  params = m_params;

  return NOERROR;
} // GetHandVuFilterParams


//
// SetHandVuFilterParams
//
STDMETHODIMP
CHandVuFilter::SetHandVuFilterParams(const HandVuFilterParams& params)
{
  CAutoLock cAutolock(&m_HandVuFilterLock);

  m_params = params;

  SetDirty(TRUE);
  return NOERROR;
} // SetHandVuFilterParams


STDMETHODIMP CHandVuFilter::Initialize(int img_width, int img_height, int iPixelSize, CameraController* pCamCon)
{
  CAutoLock lock(&m_HandVuFilterLock);
  if (m_is_initialized) {
    return E_UNEXPECTED;
  }

  m_cxImage = img_width;
  if (img_height<0) {
    // norm for RGB images
    m_img_bottom_up = false;
    m_cyImage = -img_height;
  } else {
    m_img_bottom_up = true;
    m_cyImage = img_height;
  }

  ASSERT(m_pColorHeader==NULL);
  m_pColorHeader = cvCreateImageHeader(cvSize(m_cxImage, m_cyImage), IPL_DEPTH_8U, 3);

  hvInitialize(m_cxImage, m_cyImage);

  m_is_initialized = true;

  return NOERROR;
}

STDMETHODIMP CHandVuFilter::LoadConductor(const string& filename)
{
  CAutoLock lock(&m_HandVuFilterLock);
  hvLoadConductor(filename);
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::ConductorLoaded(bool* pLoaded)
{
  *pLoaded = hvConductorLoaded();
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::StartRecognition(int obj_id)
{
  CAutoLock lock(&m_HandVuFilterLock);
  hvStartRecognition(obj_id);
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::StopRecognition(int obj_id)
{
  CAutoLock lock(&m_HandVuFilterLock);
  hvStopRecognition(obj_id);
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::IsActive(bool* pActive)
{
  *pActive = hvIsActive();
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::GetState(int obj_id, hvState& state)
{
  hvGetState(obj_id, state);
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::SetOverlayLevel(int level)
{
  hvSetOverlayLevel(level);
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::GetOverlayLevel(int* pLevel)
{
  *pLevel = hvGetOverlayLevel();
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::CorrectDistortion(bool enable)
{
  hvCorrectDistortion(enable);
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::CanCorrectDistortion(bool* pPossible)
{
  *pPossible = hvCanCorrectDistortion();
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::IsCorrectingDistortion(bool* pOn)
{
  *pOn = hvIsCorrectingDistortion();
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::SetDetectionArea(int left, int top, int right, int bottom)
{
  hvSetDetectionArea(left, top, right, bottom);
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::RecomputeNormalLatency()
{
  hvRecomputeNormalLatency();
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::SetAdjustExposure(bool enable)
{
  hvSetAdjustExposure(enable);
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::CanAdjustExposure(bool* pPossible)
{
  *pPossible = hvCanAdjustExposure();
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::IsAdjustingExposure(bool* pEnabled)
{
  *pEnabled = hvIsAdjustingExposure();
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::AddListener(HVFilterEventListener* pHVFListener)
{
  CAutoLock lock(&m_HandVuFilterLock);
  m_HVFListenerPtrs.push_back(pHVFListener);
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::RemoveListener(HVFilterEventListener* pHVFListener)
{
  CAutoLock lock(&m_HandVuFilterLock);
  bool found = false;
  int old_size = (int)m_HVFListenerPtrs.size();
  for (int i=0; i<old_size; i++) {
    if (m_HVFListenerPtrs[i]==pHVFListener) {
      found = true;
    }
    if (found && i+1<old_size) {
      m_HVFListenerPtrs[i] = m_HVFListenerPtrs[i+1];
    }
  }
  if (found) {
    m_HVFListenerPtrs.resize(old_size-1);
    return NOERROR;
  } else {
    return E_INVALIDARG;
  }
}

STDMETHODIMP CHandVuFilter::ToggleMaintenanceApp()
{
  CAutoLock lock(&m_HandVuFilterLock);
  if (!m_pOverlay) {
    VERBOSE0(3, "HandVuFilter: loading MaintenanceApp");
    m_pOverlay = new CMaintenanceApp();
    m_pOverlay->Initialize(m_cxImage, m_cyImage);

    // $IT_DATA environmet variable, NULL if not set
    const char *it_data = getenv("IT_DATA");

    string filename = string(it_data)+"\\config\\default.conductor";

    try {
      hvLoadConductor(filename);
      hvStartRecognition();
      hvSetOverlayLevel(0);

    } catch (HVException& hve) {
      AfxMessageBox(hve.GetMessage().c_str());
      delete m_pOverlay;
      m_pOverlay = NULL;
      return S_FALSE;
    }
  }

  m_show_maintenanceapp = !m_show_maintenanceapp;
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::ToggleFDLOnly()
{
  m_FDL_only = !m_FDL_only;
  return NOERROR;
}

#if !defined(HAVE_USER_STUDY)
STDMETHODIMP CHandVuFilter::OnMouseMove(UINT nFlags, double x, double y)
{
  return NOERROR;
}

STDMETHODIMP CHandVuFilter::OnLButtonUp(UINT nFlags, double x, double y)
{
  return NOERROR;
}
#endif // ! HAVE_USER_STUDY

STDMETHODIMP CHandVuFilter::SetTimecodeReader(IUnknown* tcr)
{
  ASSERT(tcr);
  HRESULT hr;
  hr = tcr->QueryInterface(IID_IAMTimecodeReader, (void **) &m_pIAMTCReader);
  if (!SUCCEEDED(hr)) {
    VERBOSE0(1, "HVF: got corrupt DV TimecodeReader");
    m_pIAMTCReader = NULL;
    return hr;
  }

  return NOERROR;
}

string CHandVuFilter::GetTimecode()
{
  ASSERT(m_pIAMTCReader);

  TIMECODE_SAMPLE TimecodeSample;
  TimecodeSample.timecode.dwFrames = 0;
  static DWORD i1 = 0, i2 = 0, i3 = 0;

  TimecodeSample.dwFlags = ED_DEVCAP_TIMECODE_READ;

  // Query the TimeCode sample data
  HRESULT hr = m_pIAMTCReader->GetTimecode(&TimecodeSample);
  ASSERT(SUCCEEDED(hr));

  const int tcodelen = 64;
  TCHAR tcode[tcodelen];
  wsprintf(tcode, "%.2x:%.2x:%.2x:%.2x",
//  StringCbPrintf(tcode, tcodelen*sizeof(TCHAR), "%.2x:%.2x:%.2x:%.2x",
    ((TimecodeSample.timecode.dwFrames & 0xff000000) >> 24),
    ((TimecodeSample.timecode.dwFrames & 0x00ff0000) >> 16),
    ((TimecodeSample.timecode.dwFrames & 0x0000ff00) >>  8),
    (TimecodeSample.timecode.dwFrames & 0x000000ff));
  return string(tcode);
} 

STDMETHODIMP CHandVuFilter::TakeSnapshot() {
  CAutoLock lock(&m_HandVuFilterLock);
  if (m_pColorHeader==NULL) {
    return E_UNEXPECTED;
  }
  m_take_one_snapshot = true;
  return NOERROR;
}

void CHandVuFilter::SaveImageArea(IplImage* pImg)
{
  ASSERT(m_is_initialized);
  string picfile = "";
  hvSaveImageArea(pImg, 0, 0, INT_MAX, INT_MAX, picfile);
  VERBOSE1(3, "HandVuFilter: saved image in file %s", picfile.c_str());
}

STDMETHODIMP CHandVuFilter::SetVerbosity(int level, const string& logfilename)
{
  CAutoLock lock(&m_HandVuFilterLock);
  if (logfilename=="") {
    g_verbose = level;
    return NOERROR;
  }

  g_ostream = fopen(logfilename.c_str(), "a+");
  if (g_ostream==NULL) {
    return E_FAIL;
  }
  g_verbose = level;

  return NOERROR;
}

STDMETHODIMP CHandVuFilter::GetVersion(string& version)
{
  if (!m_is_initialized) {
    return E_UNEXPECTED;
  }

  version = "HandVuFilter CVS id: $Id: HandVuFilter.cpp,v 1.22 2005/10/30 23:00:43 matz Exp $";
  version = version + ", built on "__DATE__" at "__TIME__;

  string handvu_version;
  hvGetVersion(handvu_version, 3);
  version = version + "\n" + handvu_version;

  return NOERROR;
}
