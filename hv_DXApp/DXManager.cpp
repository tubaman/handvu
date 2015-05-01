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
  * $Id: DXManager.cpp,v 1.14 2005/10/30 23:00:43 matz Exp $
**/

#include "stdafx.h"
#include "HandVuFilterGUIDs.h"
#include <initguid.h> // need this before MainFrm.h
#include "Common.h"
#include "DXManager.h"
#include "DXAppView.h"
#include "Dvdmedia.h"
#include "Exceptions.h"

DXManager::DXManager() :
  m_video_scale(1.0),
  m_pParentView(NULL),
  m_handvu_logfilename(""),
  m_two_windows(false)
{
  NullComPtrs();

  /*
  // start GestureEventServer
  int m_gesture_server_port = 7047;
  int m_osc_port = 57120;
  string m_osc_ip = "128.111.28.75"; 
//  string m_osc_ip = "localhost"; 
  m_pGestureServer = new CGestureEventServer(m_gesture_server_port);
  m_pGestureServer->UseOSC(m_osc_ip, m_osc_port);
  m_pGestureServer->Start();
*/

  // for event timestamps
  _ftime(&m_startTime);

  // debugging
  m_ROTregister   = 0;
//  DbgInitialise(g_hInst);
  DbgSetModuleLevel(LOG_CUSTOM1, 3);
}

DXManager::~DXManager()
{
  StopPlayback();
  DestroyFilterGraph();

//  DbgDumpObjectRegister();
//  DbgTerminate();
}

void DXManager::NullComPtrs()
{
  m_GraphBuilder = NULL;
  m_MediaControl = NULL;
  m_MediaEventEx = NULL;
  m_FilterGraph = NULL;
  m_HandVuFilterProp = NULL;
  m_HandVuFilter = NULL;
  m_SourceFilter = NULL;
  m_CameraControl = NULL;
  m_VideoWindow = NULL;
  m_SourceVideoWindow = NULL;
  m_pIAMTCReader = NULL;
}

void DXManager::StartPlayback() 
{
  if (m_HandVuFilterProp) {
    HRESULT hr = m_HandVuFilterProp->AddListener(this);
    ASSERT(SUCCEEDED(hr));
  }
  if (m_MediaControl) {
    m_VideoWindow->put_Owner((OAHWND)m_pParentView->m_hWnd);
    m_VideoWindow->put_WindowStyle(WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
    m_VideoWindow->put_MessageDrain((OAHWND)m_pParentView->m_hWnd);
    m_VideoWindow->put_Visible(OATRUE);
    /*
    if (m_MediaEventEx) {
      // Have the graph signal event via window callbacks for performance
      m_MediaEventEx->SetNotifyWindow((OAHWND)m_hWnd, WM_GRAPHNOTIFY, 0);
    }
    */
    m_MediaControl->Run();
  }
}

void DXManager::StopPlayback() 
{
  if (m_HandVuFilterProp) {
    HRESULT hr = m_HandVuFilterProp->RemoveListener(this);
    ASSERT(SUCCEEDED(hr));

  }
  if (m_MediaControl) {
    m_MediaControl->Stop();
    m_VideoWindow->put_Visible(OAFALSE);
    m_VideoWindow->put_Owner(NULL);
    m_VideoWindow->put_MessageDrain(0);
  }
}

bool DXManager::BuildGraph(bool two_windows, CDXAppView* parent)
{
  m_pParentView = parent;
  m_two_windows = two_windows;
  DestroyFilterGraph();
  bool success = RealBuildGraph();
  if (!success) {
    DestroyFilterGraph();
  }
  return success;
}

void DXManager::FrameTransformed(bool processed)
{
}

IHandVuFilter* DXManager::HV()
{
  ASSERT(m_HandVuFilterProp);
  return m_HandVuFilterProp.p;
}

void DXManager::ToggleExposureControl()
{
  SetExposureControl(-1);
}

void DXManager::SetExposureControl(int on/*=1*/)
{
  if (!m_CameraControl) {
    return;
  }

  if (!m_HandVuFilterProp) {
    return;
  }

  bool possible;
  HRESULT hr = m_HandVuFilterProp->CanAdjustExposure(&possible);
  ASSERT(SUCCEEDED(hr));
  if (!possible) {
    return;
  }

  if (on==-1) {
    bool is_on;
    hr = m_HandVuFilterProp->IsAdjustingExposure(&is_on);
    ASSERT(SUCCEEDED(hr));
    on = is_on?0:1;
  }

  if (on) {
    hr = m_HandVuFilterProp->SetAdjustExposure(true);
    ASSERT(SUCCEEDED(hr));
  } else {
    // turn it off in HandVu
    hr = m_HandVuFilterProp->SetAdjustExposure(false);
    ASSERT(SUCCEEDED(hr));
    // turn Auto exposure on in the camera itself
    SetCameraAutoExposure(true);
  }
}

/* CameraControl function
* returns true if camera was successfully set to adjust the exposure
*/
bool DXManager::SetCameraAutoExposure(bool enable) 
{
  long exposure, flags;
  HRESULT hr = m_CameraControl->Get(CameraControl_Exposure, &exposure, &flags);
  if (SUCCEEDED(hr)) {
    if (enable) {
      flags = CameraControl_Flags_Auto;
    } else {
      flags = CameraControl_Flags_Manual;
    }
    hr = m_CameraControl->Set(CameraControl_Exposure, exposure, flags);
    if (SUCCEEDED(hr)) {
      VERBOSE1(3, "camera does %sauto-expose now", ((enable)?"":"not "));
    }
  }
  if (!SUCCEEDED(hr)) {
    VERBOSE0(2, "can't set camera to do auto-exposure");
  }

  return (SUCCEEDED(hr));
}

/* CameraControl function
*/
double DXManager::GetCurrentExposure() 
{
  if (!m_CameraControl) {
    throw HVException("no camera control found");
  }
  long minExp, maxExp, stepping_delta, dflt, flags;
  HRESULT hr = m_CameraControl->GetRange(CameraControl_Exposure, &minExp, &maxExp, 
    &stepping_delta, &dflt, &flags);
  if (!SUCCEEDED(hr)) {
    throw HVException("could not get exposure range from camera control");
  }
  
  long currExp;
  hr = m_CameraControl->Get(CameraControl_Exposure, &currExp, &flags);
  if (!SUCCEEDED(hr)) {
    throw HVException("could not get current exposure from camera control");
  }
  
  double newval = (double)(currExp-minExp)/(double)(maxExp-minExp);

  VERBOSE1(5, "DXManager: GetCurrentExposure = %f",
    (float)newval);

  return newval;
}

/* CameraControl function
* true if change has an effect, false if step is too small
*/
bool DXManager::SetExposure(double exposure) 
{
  VERBOSE1(5, "DXManager: SetExposure(%f) ...",
    (float)exposure);

  if (!m_CameraControl) {
    throw HVException("no camera control found");
  }

  long minExp, maxExp, stepping_delta, dflt, flags;
  HRESULT hr = m_CameraControl->GetRange(CameraControl_Exposure, &minExp, &maxExp, 
    &stepping_delta, &dflt, &flags);
  if (!SUCCEEDED(hr)) {
    throw HVException("could not get exposure range from camera control");
  }
  
  long oldExp;
  hr = m_CameraControl->Get(CameraControl_Exposure, &oldExp, &flags);
  if (!SUCCEEDED(hr)) {
    throw HVException("could not get current exposure from camera control");
  }

  int newExp = (int) (exposure*(maxExp-minExp) + minExp);

  flags = CameraControl_Flags_Manual;
  hr = m_CameraControl->Set(CameraControl_Exposure, newExp, flags);
  if (!SUCCEEDED(hr)) {
    throw HVException("could not set new exposure for camera control");
  }

  long currExp;
  hr = m_CameraControl->Get(CameraControl_Exposure, &currExp, &flags);
  if (!SUCCEEDED(hr)) {
    throw HVException("could not get current exposure from camera control");
  }

  VERBOSE1(5, "DXManager: SetExposure completed (%s)",
    (oldExp!=currExp)?"changed":"unchanged");

  return oldExp!=currExp;
}

/* CameraControl function
*/
bool DXManager::CanAdjustExposure() 
{
  return m_CameraControl!=NULL;
}




void DXManager::ToggleFullscreen()
{
  long fullscreen;
  CComPtr<IVideoWindow> vw = NULL;
  vw = m_VideoWindow;
  HRESULT hr = vw->get_FullScreenMode(&fullscreen);
  if (SUCCEEDED(hr)) {
    if (fullscreen==OATRUE) {
      fullscreen = OAFALSE;
      ReleaseCapture();
      ShowCursor(true);
      m_pParentView->SetTrapMouse(false);
    } else {
      ASSERT(fullscreen==OAFALSE);
      fullscreen = OATRUE;
      SetCapture(m_pParentView->m_hWnd);
      ShowCursor(false);
      m_pParentView->SetTrapMouse(true);
    }
  	hr = vw->put_FullScreenMode(fullscreen);
  }
  if (!SUCCEEDED(hr)) {
    AfxMessageBox("can not render in full-screen mode");
  }
}



bool DXManager::RealBuildGraph()
{
  HRESULT hr;
//  hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, 
//                        IID_ICaptureGraphBuilder2, (void **)&m_GraphBuilder);
  hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
                        IID_IGraphBuilder, (void **)&m_GraphBuilder);
  if (!SUCCEEDED(hr) || !m_GraphBuilder) { 
    AfxMessageBox("Can't access graph builder interface");
    return false; 
  }

  CreateCamera();
  if (m_SourceFilter==0) {
    AfxMessageBox("Can't connect to a camera");
    return false;
  }

  if (!CreateHandVuFilter()) {
    AfxMessageBox("Can't create HandVu filter");
    return false;
  }


  /* create generic graph objects */
  m_GraphBuilder->QueryInterface(IID_IMediaControl,(void**)&m_MediaControl);
  m_GraphBuilder->QueryInterface(IID_IMediaEventEx,(void**)&m_MediaEventEx);
  m_GraphBuilder->QueryInterface(IID_IVideoWindow, (void**)&m_VideoWindow );
  m_GraphBuilder->QueryInterface(IID_IFilterGraph, (void**)&m_FilterGraph);

  if (!m_FilterGraph) {
    AfxMessageBox("Can't access filter graph interface");
    return false;
  }

  if (!ConnectFilters()) {
    int device;
    for (device=1; device<=10; device++) {
      CreateCamera(device);
      if (ConnectFilters()) {
        break;
      }
    }
    if (device==11) {
      AfxMessageBox("Can't connect to a camera, tried 10 devices");
      return false;
    }
  }
#ifdef DEBUG
  // ROT debugging stuff
  hr = AddToROT(m_GraphBuilder, &m_ROTregister);
  ASSERT(SUCCEEDED(hr));
#endif // DEBUG

  return true;
}

/* link the created filters */
bool DXManager::ConnectFilters()
{
  HRESULT hr;
  bool success = false;

  CComPtr<IPin> pSourceOut          = NULL;
  CComPtr<IPin> pHandVuIn           = NULL;
  CComPtr<IPin> pHandVuOut          = NULL;
  CComPtr<IBaseFilter> pSmartTee    = NULL;
  CComPtr<IBaseFilter> pColorConv   = NULL;
  CComPtr<IPin> pProcessThis        = NULL;
  CComPtr<IBaseFilter> pAppRenderer = NULL;
  CComPtr<IPin> pSmartTPreviewOut   = NULL;


  // get a video output pin
  hr = AddFilter(m_SourceFilter, L"Video Source" );
  pSourceOut = GetVideoPin(m_SourceFilter, PINDIR_OUTPUT);
  if (!SUCCEEDED(hr) || pSourceOut==NULL) goto release_and_return;

  // make two application windows
  if (!m_two_windows) {
    pProcessThis = pSourceOut;
  } else {
    hr = CoCreateInstance(CLSID_SmartTee, NULL, CLSCTX_INPROC_SERVER, 
		                      IID_IBaseFilter, (void**)&pSmartTee);
    if (!SUCCEEDED(hr)) goto release_and_return;

    hr = AddFilter(pSmartTee, L"SmartT");
    CComPtr<IPin> pSmartTIn         = GetPin(pSmartTee, PINDIR_INPUT);
    CComPtr<IPin> pSmartTCaptureOut = GetPin(pSmartTee, PINDIR_OUTPUT, 0);//, &PIN_CATEGORY_CAPTURE);
    pSmartTPreviewOut = GetPin(pSmartTee, PINDIR_OUTPUT, 1);//, &PIN_CATEGORY_PREVIEW);
    if (!SUCCEEDED(hr) || !pSmartTIn || !pSmartTCaptureOut || !pSmartTPreviewOut) goto release_and_return;

    hr = ConnectPins(pSourceOut, pSmartTIn);
    if (!SUCCEEDED(hr)) goto release_and_return;

    hr = CoCreateInstance( CLSID_VideoRenderer, NULL, CLSCTX_INPROC_SERVER, 
		      IID_IBaseFilter, (void**)&pAppRenderer );
    if (!SUCCEEDED(hr)) goto release_and_return;

    pAppRenderer->QueryInterface(IID_IVideoWindow,(void**)&m_SourceVideoWindow);
    if (!SUCCEEDED(hr) || !m_SourceVideoWindow) goto release_and_return;

    m_SourceWin.m_pDXManager = this;
    if (m_SourceWin.m_hWnd==NULL) {
  //    m_SourceWin.CreateEx(NULL, "", "source display", WS_POPUP | WS_MAXIMIZE, CRect(10, 10, 300, 200), m_pParent, 1, NULL);
      m_SourceWin.Create(NULL, "source display", WS_CHILD, CRect(50, 50, 160, 120), m_pParentView, 1, NULL);
    }
    m_SourceVideoWindow->put_Owner((OAHWND)m_SourceWin.m_hWnd);
    m_SourceVideoWindow->put_MessageDrain((OAHWND)m_SourceWin.m_hWnd);
    m_SourceVideoWindow->SetWindowPosition(50, 50, 160, 120);

    hr = CoCreateInstance( CLSID_Colour, NULL, CLSCTX_INPROC_SERVER, 
		    IID_IBaseFilter, (void**)&pColorConv );
    if (!SUCCEEDED(hr) || !pColorConv) goto release_and_return;
    CComPtr<IPin> pColorConvIn    = GetPin(pColorConv, PINDIR_INPUT);
    CComPtr<IPin> pColorConvOut   = GetPin(pColorConv, PINDIR_OUTPUT);
    hr = AddFilter(pColorConv, L"Source Renderer" );
    if (!SUCCEEDED(hr)) goto release_and_return;

    hr = ConnectPins(pSmartTCaptureOut, pColorConvIn );
    if (!SUCCEEDED(hr)) goto release_and_return;

    pSmartTIn.Release();

    pProcessThis = pColorConvOut;
  }

  hr = AddFilter(m_HandVuFilter, L"HandVuFilter");
  pHandVuIn         = GetPin(m_HandVuFilter, PINDIR_INPUT);
  pHandVuOut        = GetPin(m_HandVuFilter, PINDIR_OUTPUT);
  if (!SUCCEEDED(hr) || !pHandVuIn || !pHandVuOut) goto release_and_return;

  hr = ConnectPins(pProcessThis, pHandVuIn);      
  if (!SUCCEEDED(hr)) goto release_and_return;

  if (!InitHandVuFilter()) goto release_and_return;

  hr = m_GraphBuilder->Render(pHandVuOut);
  if (!SUCCEEDED(hr)) goto release_and_return;

  if (m_two_windows) {
    CComPtr<IPin> pAppRendererIn    = GetPin(pAppRenderer, PINDIR_INPUT);
    hr = AddFilter(pAppRenderer, L"Source Renderer" );
    if (!SUCCEEDED(hr) || !pAppRendererIn) goto release_and_return;

    hr = ConnectPins(pSmartTPreviewOut, pAppRendererIn );
    if (!SUCCEEDED(hr)) goto release_and_return;

    m_SourceWin.EnableWindow(TRUE);
    m_SourceWin.Invalidate();

    pAppRenderer.Release();
    pAppRendererIn.Release();
//    pSmartTCaptureOut.Release();

  }

  success = true;

release_and_return:
  pProcessThis.Release();
  pHandVuIn.Release();
  pHandVuOut.Release();
  pSmartTee.Release();
  pAppRenderer.Release();
  pSmartTPreviewOut.Release();

  return success;
}



// Destroy DirectShow Filter Graph
void DXManager::DestroyFilterGraph()
{
  if (m_FilterGraph) {
    CComPtr<IEnumFilters> pEnumFilters = 0;
    m_FilterGraph->EnumFilters( &pEnumFilters );

    if( pEnumFilters ) {
      CComPtr<IBaseFilter> pFilter = 0;
      ULONG cFetched = 0;

      while( pEnumFilters->Next( 1, &pFilter, &cFetched ) == S_OK && pFilter != 0 ) {
        m_FilterGraph->RemoveFilter( pFilter );
        pFilter.Release();
        cFetched = 0;
      }
      pEnumFilters.Release();
    }
  }
#ifdef DEBUG
  RemoveFromROT(m_ROTregister);
#endif

  m_GraphBuilder.Release();
  m_MediaControl.Release();
  m_MediaEventEx.Release();
  m_FilterGraph.Release();
  m_HandVuFilter.Release();
  m_HandVuFilterProp.Release();
  m_SourceFilter.Release();
  m_CameraControl.Release();
  m_VideoWindow.Release();
  m_SourceVideoWindow.Release();

  NullComPtrs();
}



bool DXManager::CreateHandVuFilter()
{
  HRESULT hr = CoCreateInstance( CLSID_HandVuFilter, NULL, CLSCTX_INPROC_SERVER, 
		                             IID_IBaseFilter, (void**)&m_HandVuFilter );
  if (SUCCEEDED(hr)) {
    m_HandVuFilter->QueryInterface(IID_IHandVuFilter, (void**)&m_HandVuFilterProp);
  }
  if (m_HandVuFilterProp) {
    
    hr = m_HandVuFilterProp->AddListener(this);
    ASSERT(SUCCEEDED(hr));
    
    if (m_pIAMTCReader) {
      hr = m_HandVuFilterProp->SetTimecodeReader(m_pIAMTCReader);
      ASSERT(SUCCEEDED(hr));
    }

    hr = m_HandVuFilterProp->SetVerbosity(g_verbose, m_handvu_logfilename);
    ASSERT(SUCCEEDED(hr));
  }
  return m_HandVuFilterProp != 0;
}

bool DXManager::InitHandVuFilter()
{
  ASSERT(m_SourceFilter);

  CComPtr<IPin> pPin = GetVideoPin(m_SourceFilter, PINDIR_OUTPUT);
  AM_MEDIA_TYPE media_type;
  pPin->ConnectionMediaType(&media_type);
  BITMAPINFOHEADER* pHdr=NULL;
  if (media_type.formattype==FORMAT_VideoInfo) {
    pHdr = &((VIDEOINFOHEADER*) media_type.pbFormat)->bmiHeader;
  } else if (media_type.formattype==FORMAT_VideoInfo2) {
    pHdr = &((VIDEOINFOHEADER2*) media_type.pbFormat)->bmiHeader;
  } else {
    return false;
  }

  ASSERT(pHdr);
  int width = pHdr->biWidth;
	int height = pHdr->biHeight;
  int iPixelSize = pHdr->biBitCount / 8;
  HRESULT hr = m_HandVuFilterProp->Initialize(width, height, iPixelSize, this);
  ASSERT(SUCCEEDED(hr));

  return SUCCEEDED(hr);
}


bool DXManager::CreateCamera( int idx )
{
  CComPtr<ICreateDevEnum> pCreateDevEnum = 0;
  CComPtr<IEnumMoniker>   pEnumMon = 0;
  ULONG           cFetched = 0;

  m_SourceFilter.Release();

  CoCreateInstance( CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		                IID_ICreateDevEnum, (void**)&pCreateDevEnum );

  /* Create capture device */
  if( pCreateDevEnum ) {
    CComPtr<IMoniker> pMon = 0;
    pCreateDevEnum->CreateClassEnumerator( CLSID_VideoInputDeviceCategory, &pEnumMon, 0);

    if( pEnumMon && idx > 0 ) {
      pEnumMon->Skip( idx );
      ASSERT(0); // need to give user a choice
    }
    if( pEnumMon && SUCCEEDED( pEnumMon->Next(1, &pMon, &cFetched)) && cFetched == 1 ) {
      pMon->BindToObject(0, 0, IID_IBaseFilter, (void **)&m_SourceFilter );
      pMon.Release();
    }
    pEnumMon.Release();
    pCreateDevEnum.Release();
  }

  if (!m_SourceFilter) {
    // try file input
    CFileDialog open_dlg(true, 0, 0, OFN_FILEMUSTEXIST, 
      "All Media Files (*.avi;*.mpg;*.mpeg;*.mov;*.wm;*.wmv;*.wma)|*.avi; *.mpg; *.mpeg; *.mov; *.wm; *.wmv; *.wma|"
      "Movie Files (*.avi;*.mpg;*.mpeg;*.mov)|*.avi; *.mpg; *.mpeg; *.mov|"
      "Windows Media Files (*.wm;*.wmv;*.wma)|*.wm;*.wmv;*.wma|"
      "All Files (*.*)|*.*||");

    if (open_dlg.DoModal()==IDOK) {
      CString str = open_dlg.GetPathName();
      if (str.GetLength()>0) {
        WCHAR wname[1000] = L"";
        MultiByteToWideChar( CP_ACP, 0, str, -1, wname, sizeof(wname)/2 );

        HRESULT hr = m_GraphBuilder->AddSourceFilter(wname, L"Mmh da sauze filter", &m_SourceFilter);
        if (!SUCCEEDED(hr) || m_SourceFilter==NULL) {
          VERBOSE0(3, "DXManager: could not add source filter");
          return false;
        }
      }
    }
  }

  if (m_SourceFilter) {
    HRESULT hr = m_SourceFilter->QueryInterface(IID_IAMCameraControl, (void **)&m_CameraControl);
    if (SUCCEEDED(hr) && m_CameraControl) {
      VERBOSE0(5, "IAMCameraControl is implemented");
      // make sure it works
      try {
        double exposure = GetCurrentExposure();
        double new_exposure = SetExposure(exposure);
      } catch (HVException& hve) {
        VERBOSE1(2, "IAMCameraControl operation test failed: %s",
          hve.GetMessage().c_str());
        m_CameraControl.Release();
        m_CameraControl = NULL;
      }
      if (m_CameraControl) {
        VERBOSE0(5, "IAMCameraControl functional - got CameraController");
      }
    } else {
      VERBOSE0(2, "IAMCameraControl not implemented");
    }

    hr = m_SourceFilter->QueryInterface(IID_IAMTimecodeReader, (void **) &m_pIAMTCReader);
    if (!SUCCEEDED(hr)) {
      VERBOSE0(3, "DXManager: no DV TimecodeReader available");
      m_pIAMTCReader = NULL;
    }
  }

  return m_SourceFilter != 0;
}






#ifdef DEBUG
/////////////////////////////////////////////////////////////////////////////
// this allows GraphEdit to connect to the filter graph
// (File ... connect)
HRESULT DXManager::AddToROT(IUnknown *pUnkGraph, DWORD *pdwRegister) 
{
  IMoniker * pMoniker;
  IRunningObjectTable *pROT;
  if (FAILED(GetRunningObjectTable(0, &pROT))) {
    return E_FAIL;
  }
  int const sz = 256;
  WCHAR wsz[sz];
  size_t cbDest = sz * sizeof(WCHAR);
//  StringCbPrintfW(wsz, cbDest, L"FilterGraph %08p pid %08x", (DWORD *) pUnkGraph, GetCurrentProcessId());
  wsprintfW(wsz, L"FilterGraph %08p pid %08x", (DWORD *) pUnkGraph, GetCurrentProcessId());
  HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
  //HRESULT hr = CreateItemMoniker(L"!", L"HandVuDXApp", &pMoniker);
  if (SUCCEEDED(hr)) {
    hr = pROT->Register(0, pUnkGraph, pMoniker, pdwRegister);
    pMoniker->Release();
  }
  pROT->Release();
  return hr;
}

/////////////////////////////////////////////////////////////////////////////
// remove filter graph from Running Object Table
void DXManager::RemoveFromROT(DWORD pdwRegister)
{
  IRunningObjectTable *pROT;
  if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) {
    pROT->Revoke(pdwRegister);
    pROT->Release();
  }
}
#endif // DEBUG


void DXManager::GetVideoSourceSize(int* pWidth, int* pHeight) 
{
  *pWidth = -1;
	*pHeight = -1;
  if (m_SourceFilter!=0) {
    CComPtr<IPin> pPin = GetVideoPin(m_SourceFilter, PINDIR_OUTPUT);
    AM_MEDIA_TYPE media_type;
    pPin->ConnectionMediaType(&media_type);
    BITMAPINFOHEADER* pHdr=NULL;
    if (media_type.formattype==FORMAT_VideoInfo) {
      pHdr = &((VIDEOINFOHEADER*) media_type.pbFormat)->bmiHeader;
    } else if (media_type.formattype==FORMAT_VideoInfo2) {
      pHdr = &((VIDEOINFOHEADER2*) media_type.pbFormat)->bmiHeader;
    }
    if (pHdr!=NULL) {
      *pWidth = pHdr->biWidth;
	  	*pHeight = abs(pHdr->biHeight);
      VERBOSE2(4, "DXManager: got real size %d %d", *pWidth, *pHeight);
    }
  } 
  if (*pWidth==-1 || *pHeight==-1)  {
    *pWidth = 640;
	  *pHeight = 480;
    VERBOSE2(4, "DXManager: got fake size %d %d", *pWidth, *pHeight);
  }
}

void DXManager::SetVideoWindowPosition(int left, int top, int width, int height) 
{
  if (m_VideoWindow) {
    VERBOSE4(4, "DXManager: setting left %d, top %d, width %d, height %d", 
      left, top, width, height);
    m_VideoWindow->SetWindowPosition( left, top, width, height );
  }
}

void DXManager::SetSourceVideoWindowPosition(int left, int top, int width, int height) 
{
  if (m_SourceVideoWindow) {
    m_SourceVideoWindow->SetWindowPosition( left, top, width, height );
  }
}

HRESULT DXManager::AddFilter(IBaseFilter* filter, LPCWSTR name) {
  HRESULT hr = m_FilterGraph->AddFilter(filter, name);
#ifdef DEBUG
  if (!SUCCEEDED(hr)) {
    char buf[MAX_ERROR_TEXT_LEN];
    AMGetErrorText(hr, buf, MAX_ERROR_TEXT_LEN);
    ASSERT(0);
  }
#endif // DEBUG
  return hr;
}

HRESULT DXManager::ConnectPins(IPin* from, IPin* to) {
  HRESULT hr = m_GraphBuilder->Connect(from, to);
#ifdef DEBUG
  if (!SUCCEEDED(hr)) {
    char buf[MAX_ERROR_TEXT_LEN];
    AMGetErrorText(hr, buf, MAX_ERROR_TEXT_LEN);
    ASSERT(0);
  }
#endif // DEBUG
  return hr;
}

void DXManager::GetVersion(string& version) const
{
  string dxm_version = "DXManager: $Id: DXManager.cpp,v 1.14 2005/10/30 23:00:43 matz Exp $";
  string filter_version = "no HandVuFilter loaded";
  if (m_HandVuFilterProp) {
    HRESULT hr = m_HandVuFilterProp->GetVersion(filter_version);
    ASSERT(SUCCEEDED(hr));
  }
  version = dxm_version + "\n" + filter_version;
}


BOOL PinMatchesCategory(CComPtr<IPin> pPin, const GUID& Category)
{
  if (pPin == NULL) {
    return false;
  }

  BOOL bFound = FALSE;
  IKsPropertySet *pKs;
  HRESULT hr = pPin->QueryInterface(IID_IKsPropertySet, (void **)&pKs);
  if (SUCCEEDED(hr))
  {
    GUID PinCategory;
    DWORD cbReturned;
    hr = pKs->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0, 
      &PinCategory, sizeof(GUID), &cbReturned);
    if (SUCCEEDED(hr))
    {
      bFound = (PinCategory == Category);
    }
    pKs->Release();
  }
  return bFound;
}

static CComPtr<IPin> GetPin(CComPtr<IBaseFilter> pFilter, PIN_DIRECTION PinDir, int skip/*=0*/, const GUID* pCategory /*= NULL*/)
{
    CComPtr<IEnumPins>  pEnum;
    CComPtr<IPin>       pPin;

    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr)) {
        return NULL;
    }
    while((hr=pEnum->Next(1, &pPin.p, 0)) == S_OK) {
        PIN_DIRECTION PinDirThis;
        pPin->QueryDirection(&PinDirThis);
        if (PinDir==PinDirThis) {
          if (pCategory==NULL || PinMatchesCategory(pPin, *pCategory)) {
            if (skip) {
              skip--;
            } else {
              break;
            }
          }
        }
        pPin.Release();
    }
    pEnum.Release();
    return pPin;  
}

// get a video output pin
static CComPtr<IPin> GetVideoPin(CComPtr<IBaseFilter> pFilter, PIN_DIRECTION PinDir) //, int skip/*=0*/, const GUID* pCategory /*= NULL*/)
{
  CComPtr<IPin>       pPin;

  bool got_one = false;
  int skip = 0;
  BITMAPINFOHEADER* pHdr=NULL;
  do {
    pPin = GetPin(pFilter, PINDIR_OUTPUT, skip);
    if (pPin==NULL) {
      return NULL;
    }

    CComPtr<IEnumMediaTypes> pEnum;
    HRESULT hr = pPin->EnumMediaTypes(&pEnum);
    if (!SUCCEEDED(hr) || !pEnum) return NULL;
    AM_MEDIA_TYPE* pMediaType = NULL;
    hr = pEnum->Next(1, &pMediaType, NULL);
    while (hr==S_OK) {
      if (pMediaType->formattype==FORMAT_VideoInfo || 
          pMediaType->majortype==MEDIATYPE_Video || 
          pMediaType->majortype==MEDIATYPE_Interleaved || 
          pMediaType->majortype==MEDIATYPE_Stream || 
          pMediaType->majortype==MEDIATYPE_AnalogVideo) 
      {
        DeleteMediaType(pMediaType);
        pMediaType = NULL;
        got_one = true;
        break;
      }
      DeleteMediaType(pMediaType);
      pMediaType = NULL;
      hr = pEnum->Next(1, &pMediaType, NULL);
    }
    skip++;
    if (!got_one) {
      pPin.Release();
    }
  } while (!got_one);

  return pPin;
}








IMPLEMENT_DYNCREATE(CSourceView, CScrollView)

BEGIN_MESSAGE_MAP(CSourceView, CScrollView)
  ON_WM_SIZE()
END_MESSAGE_MAP()


CSourceView::CSourceView()
{
}

CSourceView::~CSourceView()
{
}

BOOL CSourceView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

void CSourceView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
  SetVideoWindowSize(0, 0);
}

void CSourceView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);

  SetVideoWindowSize(cx, cy);
	Invalidate(true);
}

void CSourceView::SetVideoWindowSize(int width, int height) 
{
  int source_width, source_height;
  m_pDXManager->GetVideoSourceSize(&source_width, &source_height);
  if (source_height<0) {
    source_height = -source_height;
  }
  CRect rc;
  GetClientRect( &rc );
  if (width==0 && height==0) {
    width = source_width;
    height = source_height;
//      MoveWindow(&rc);
  } else {
    width = max(width, source_width);
    height = max(height, source_height);
    double ratio = (double)source_width/(double)source_height;
    width = min(width, (int)((double)height*ratio));
    height = min(height, (int)((double)width/ratio));
  }
  m_pDXManager->SetSourceVideoWindowPosition( rc.left, rc.top, width, height );

	SetScrollSizes(MM_TEXT, CSize(source_width, source_height));
}
