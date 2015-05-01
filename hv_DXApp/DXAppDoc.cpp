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
  * $Id: DXAppDoc.cpp,v 1.11 2004/11/24 08:38:40 matz Exp $
**/

#include "stdafx.h"
#include "Common.h"
#include "DXApp.h"
#include "DXAppDoc.h"
#include "DXAppView.h"

#include <direct.h>  // for getcwd

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDXAppDoc

IMPLEMENT_DYNCREATE(CDXAppDoc, CDocument)

BEGIN_MESSAGE_MAP(CDXAppDoc, CDocument)
  ON_COMMAND(ID_OVERLAY_LEVEL_0, OnOverlayLevel0)
  ON_COMMAND(ID_OVERLAY_LEVEL_1, OnOverlayLevel1)
  ON_COMMAND(ID_OVERLAY_LEVEL_2, OnOverlayLevel2)
  ON_COMMAND(ID_OVERLAY_LEVEL_3, OnOverlayLevel3)
  ON_COMMAND(ID_EXPOSURE_CONTROL, OnExposureControl)
  ON_COMMAND(ID_LATENCY, OnLatency)
  ON_COMMAND(ID_LOAD_CONDUCTOR, OnLoadConductor)
  ON_COMMAND(ID_UNDISTORT, OnUndistort)
  ON_COMMAND(ID_TOGGLE_FULLSCREEN, OnToggleFullscreen)
  ON_COMMAND(ID_RESTART, OnRestart)
  ON_COMMAND(ID_TOGGLE_MAINTENANCEAPP, OnToggleMaintenanceapp)
  ON_COMMAND(ID_TOGGLE_FDL_ONLY, OnToggleFDLOnly)
  ON_COMMAND(ID_SOURCE_WINDOW, OnSourceWindow)
  ON_COMMAND(ID_TAKE_SNAPSHOT, OnTakeSnapshot)
#if defined(HAVE_USER_STUDY)
  ON_COMMAND(ID_ABORT_STUDYHV1_TASK, OnAbortStudyHV1Task)
  ON_COMMAND(ID_ABORT_STUDYHV1_SESSION, OnAbortStudyHV1Session)
  ON_COMMAND(ID_START_STUDY_HV1, OnStartStudyHV1)
  ON_COMMAND(ID_START_STUDY_HV2, OnStartStudyHV2)
#endif
END_MESSAGE_MAP()


// CDXAppDoc construction/destruction

CDXAppDoc::CDXAppDoc()
: m_source_window(false)
{
}

CDXAppDoc::~CDXAppDoc()
{
}

BOOL CDXAppDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

bool CDXAppDoc::StartDXManager(bool print_version)
{
  bool success = false;
  try {
    // start playing live video
    POSITION pos = GetFirstViewPosition();
    CDXAppView* pView = (CDXAppView*) GetNextView( pos );
    ASSERT(pos==NULL);
    success = m_DXManager.BuildGraph(m_source_window, pView);
    if (success) {
      m_DXManager.StartPlayback();

      // resize the window to the video size
      int source_width, source_height;
      m_DXManager.GetVideoSourceSize(&source_width, &source_height);
      CRect viewRect, frameRect;
      pView->GetWindowRect(&viewRect);
	    CWnd* pFrame = AfxGetMainWnd();
      ASSERT(pFrame);
      if (pFrame) {
        pFrame->GetWindowRect(&frameRect);
        int border_width = frameRect.right-frameRect.left-(viewRect.right-viewRect.left);
        int border_height = frameRect.bottom-frameRect.top-(viewRect.bottom-viewRect.top);
        int mf_width = source_width+border_width+4;
        int mf_height = source_height+border_height+4;
        pFrame->SetWindowPos(&CWnd::wndTop, -1, -1, 
                            mf_width, mf_height,
                            SWP_NOMOVE);
        VERBOSE2(4, "AppDoc: mainframe width %d, height %d", 
          mf_width, mf_height);
      } else {
        success = false;
      }
    }
  } catch (HVException& hve) {
    AfxMessageBox(hve.GetMessage().c_str());
    success = false;
  }

  VERBOSE1(5, "AppDoc: starting DXManager %s", success?"succeeded":"failed!");

  // print version if requested
  if (print_version) {
    string version;
    m_DXManager.GetVersion(version);
    VERBOSE1(3, "%s", version.c_str());
    fprintf(stderr, "%s\n", version.c_str());
  }

  return success;
}


bool CDXAppDoc::SetDefaults(const string& conductor_filename)
{
  VERBOSE1(5, "DXAppDoc: setting defaults and loading conductor %s",
    conductor_filename.c_str());

  try {
    m_DXManager.HV()->LoadConductor(conductor_filename);
  } catch (HVException& hve) {
    AfxMessageBox(hve.GetMessage().c_str());
    return false;
  }

  try {
    m_DXManager.SetExposureControl(1);
    m_DXManager.HV()->SetOverlayLevel(1);
    m_DXManager.HV()->StartRecognition(0);

    //m_DXManager.ToggleFullscreen();

  } catch (HVException& hve) {
    AfxMessageBox(hve.GetMessage().c_str());
    return false;
  }

  return true;
}

void CDXAppDoc::SetHandVuLogfilename(const string& handvu_logfilename) {
  m_DXManager.SetLogfilename(handvu_logfilename);
}


// CDXAppDoc commands

void CDXAppDoc::OnOverlayLevel0()
{
  m_DXManager.HV()->SetOverlayLevel(0);
}

void CDXAppDoc::OnOverlayLevel1()
{
  m_DXManager.HV()->SetOverlayLevel(1);
}

void CDXAppDoc::OnOverlayLevel2()
{
  m_DXManager.HV()->SetOverlayLevel(2);
}

void CDXAppDoc::OnOverlayLevel3()
{
  m_DXManager.HV()->SetOverlayLevel(3);
}

void CDXAppDoc::OnExposureControl()
{
  m_DXManager.ToggleExposureControl();
}

void CDXAppDoc::OnLatency()
{
  m_DXManager.HV()->RecomputeNormalLatency();
}

#define MAX_PATH_CHARS 50000
void CDXAppDoc::OnLoadConductor()
{
	// load vision conductor
	CFileDialog open_dlg( true, 0, 0, OFN_OVERWRITEPROMPT, 
		"vision conductor files (*.conductor)|*.conductor|All Files (*.*)|*.*||", NULL );
	char buf[MAX_PATH_CHARS];
	buf[0] = 0;
	open_dlg.m_ofn.lpstrFile = buf;
	open_dlg.m_ofn.nMaxFile = MAX_PATH_CHARS;
    
	if (open_dlg.DoModal()==IDOK) {
		string str = open_dlg.GetPathName();
		if (str.length()>0) {
      try {
        m_DXManager.HV()->LoadConductor(str);
        bool active;
        m_DXManager.HV()->IsActive(&active);
        m_DXManager.HV()->StartRecognition(0);
      } catch (HVException& hve) {
        AfxMessageBox(hve.GetMessage().c_str());
      }
    }
  }
}

void CDXAppDoc::OnUndistort()
{
  bool possible;
  m_DXManager.HV()->CanCorrectDistortion(&possible);
  if (possible) {
    bool onoff;
    m_DXManager.HV()->IsCorrectingDistortion(&onoff);
    m_DXManager.HV()->CorrectDistortion(!onoff);
  }
}

void CDXAppDoc::OnToggleFullscreen()
{
  m_DXManager.ToggleFullscreen();
}

void CDXAppDoc::OnRestart()
{
  m_DXManager.HV()->StartRecognition(0);
}

void CDXAppDoc::OnToggleMaintenanceapp()
{
  m_DXManager.HV()->ToggleMaintenanceApp();
}

void CDXAppDoc::OnToggleFDLOnly()
{
  m_DXManager.HV()->ToggleFDLOnly();
}

void CDXAppDoc::OnSourceWindow()
{
  m_DXManager.StopPlayback();
  m_source_window = !m_source_window;
  if (StartDXManager(false)) {
    try {
      m_DXManager.HV()->StartRecognition(0);
    } catch (HVException& hve) {
      AfxMessageBox(hve.GetMessage().c_str());
    }
    m_DXManager.HV()->RecomputeNormalLatency();
    POSITION pos = GetFirstViewPosition();
    CView* pView = GetNextView( pos );
    ASSERT(pos==NULL);
    pView->SendMessage(WM_SIZE, 0, 0);
  }
}

void CDXAppDoc::OnTakeSnapshot()
{
  m_DXManager.HV()->TakeSnapshot();
}

#if defined(HAVE_USER_STUDY)
void CDXAppDoc::OnAbortStudyHV1Task()
{
  m_DXManager.HV()->AbortStudyHV1Task();
}

void CDXAppDoc::OnAbortStudyHV1Session()
{
  m_DXManager.HV()->AbortStudyHV1Session();
}

void CDXAppDoc::OnStartStudyHV1()
{
  m_DXManager.HV()->StartStudyHV1();
}

void CDXAppDoc::OnStartStudyHV2()
{
  m_DXManager.HV()->StartStudyHV2();
}
#endif
