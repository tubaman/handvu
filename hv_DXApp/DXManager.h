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
  * $Id: DXManager.h,v 1.8 2005/10/30 23:00:43 matz Exp $
**/

#include "iHandVuFilter.h"
#include "HandVuFilterGUIDs.h"

#include <sys/types.h>
#include <sys/timeb.h>


class DXManager;
class CDXAppView;


class CSourceView : public CScrollView
{
public:
  CSourceView();
	DECLARE_DYNCREATE(CSourceView)
	virtual ~CSourceView();

// Overrides
public:
  virtual void OnDraw(CDC* pDC) {};  // overridden to draw this view
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	virtual void OnInitialUpdate();

protected:
  void SetVideoWindowSize(int width, int height);

// Generated message map functions
protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

public:
  DXManager*      m_pDXManager;
};


class DXManager : public HVFilterEventListener, CameraController
{
public:
  DXManager();
  ~DXManager();

public:
  bool                       BuildGraph(bool two_windows, CDXAppView* parent);
  void                       StopPlayback();
  void                       StartPlayback();
  void                       SetVideoWindowPosition(int left, int top, int width, int height);
  void                       SetSourceVideoWindowPosition(int left, int top, int width, int height);
  void                       GetVideoSourceSize(int* pWidth, int* pHeight);
  IHandVuFilter*             HV();
  void                       ToggleExposureControl();
  void                       SetExposureControl(int on=1); // -1 toggles it
  void                       ToggleFullscreen();
  void                       SetLogfilename(const string& handvu_logfilename) 
                               { m_handvu_logfilename = handvu_logfilename; }
  void                       GetVersion(string& version) const;

// HVFilterEventListener callback:
  virtual void               FrameTransformed(bool processed);

// CameraController
  virtual double             GetCurrentExposure();
  virtual bool               SetExposure(double exposure); 
  virtual bool               SetCameraAutoExposure(bool enable=true);
  virtual bool               CanAdjustExposure();


protected:
  bool                       CreateHandVuFilter();
  bool                       InitHandVuFilter();
  bool                       CreateCamera(int idx=0);
  bool                       RealBuildGraph();
  bool                       ConnectFilters();
  void                       DestroyFilterGraph();
  void                       Render(IPin* pPin, CRect& rect);
  void                       NullComPtrs(); // set CComPtr's to NULL
  HRESULT                    AddFilter(IBaseFilter* filter, LPCWSTR name);
  HRESULT                    ConnectPins(IPin* from, IPin* to);
#ifdef DEBUG
  HRESULT                     AddToROT(IUnknown *pUnkGraph, DWORD *pdwRegister);
  void                        RemoveFromROT(DWORD pdwRegister);
#endif


 protected: 
  // DirectShow interface pointers
  CComPtr<IGraphBuilder>     m_GraphBuilder;
  CComPtr<IMediaControl>     m_MediaControl;
  CComPtr<IVideoWindow>      m_VideoWindow;
  CComPtr<IFilterGraph>      m_FilterGraph;
  CComPtr<IMediaEventEx>     m_MediaEventEx;
  CComPtr<IBaseFilter>       m_SourceFilter;
  CComPtr<IAMCameraControl>  m_CameraControl;
  CComPtr<IBaseFilter>       m_HandVuFilter;
  CComPtr<IHandVuFilter>     m_HandVuFilterProp;
  CComPtr<IAMTimecodeReader> m_pIAMTCReader;

  
  double                     m_video_scale;
  int                        m_source_width;
  int                        m_source_height;
  DWORD                      m_ROTregister;

  // source window
  CDXAppView*                m_pParentView;
  CSourceView                m_SourceWin;
  CComPtr<IVideoWindow>      m_SourceVideoWindow;
  bool                       m_two_windows;

  // end DirectShow related

  struct _timeb              m_startTime;
  string                     m_handvu_logfilename;
};


// other prototypes
BOOL 
PinMatchesCategory(CComPtr<IPin> pPin, const GUID& Category);

static CComPtr<IPin> 
GetPin(CComPtr<IBaseFilter> pFilter, PIN_DIRECTION PinDir, 
       int skip=0, const GUID* pCategory=NULL);

static CComPtr<IPin> 
GetVideoPin(CComPtr<IBaseFilter> pFilter, PIN_DIRECTION PinDir);