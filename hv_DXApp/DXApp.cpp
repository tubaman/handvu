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
  * $Id: DXApp.cpp,v 1.8 2004/11/24 08:38:40 matz Exp $
**/

#include "stdafx.h"
#include "Common.h"
#include "DXApp.h"
#include "MainFrm.h"
#include "DXAppDoc.h"
#include "DXAppView.h"
#include "DXCommandLineInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


int g_verbose = 0; // declared in Common.h
FILE* g_ostream = NULL; // declared in Common.h


// CDXApp

BEGIN_MESSAGE_MAP(CDXApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

// CDXApp construction

CDXApp::CDXApp()
{
}


// The one and only CDXApp object

CDXApp theApp;

// CDXApp initialization

BOOL CDXApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// Change the registry key under which our settings are stored
	SetRegistryKey(_T("Matz' Applications"));

  // check for the presence of some libraries etc.
  //CheckEnvironment();

  //	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
	
  // Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CDXAppDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CDXAppView));
	AddDocTemplate(pDocTemplate);
	// Parse command line for standard shell commands, DDE, file open
	CDXCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
  if (!ProcessShellCommand(cmdInfo)) {
		return FALSE;
  }

  // logging/debug output
  string logfilename = cmdInfo.GetLogFilename();
  if (logfilename!="") {
    g_ostream = fopen(logfilename.c_str(), "a+");
    g_verbose = cmdInfo.GetVerbosity();
    ASSERT(g_ostream);
    VERBOSE0(3, "Starting HandVu's DXApp");
    VERBOSE1(3, "logging to (this) file: %s", logfilename.c_str());
  }
  if (cmdInfo.PrintVersion()) {
    string version = "DXApp: $Id: DXApp.cpp,v 1.8 2004/11/24 08:38:40 matz Exp $";
    VERBOSE1(3, "%s", version.c_str());
    fprintf(stderr, "%s\n", version.c_str());
  }

  // we should be done with all initialization that is required before the DX stuff
  // can be initialized. do that.
  POSITION pos = pDocTemplate->GetFirstDocPosition();
  ASSERT(pos);
  if (!pos) return FALSE;
  CDXAppDoc* pDoc = (CDXAppDoc*) pDocTemplate->GetNextDoc(pos);
  ASSERT(pDoc);
  if (!pDoc) return FALSE;
  
  pDoc->SetHandVuLogfilename(logfilename);
  if (!pDoc->StartDXManager(cmdInfo.PrintVersion())) {
    return FALSE;
  }
  if (!pDoc->SetDefaults(cmdInfo.GetConductor())) {
    return FALSE;
  }

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand

  VERBOSE0(5, "Successfully started HandVu's DXApp");

  return TRUE;
}

BOOL CDXApp::CheckEnvironment()
{
  char pathbuffer[_MAX_PATH];
  char cv[] = "CV.dll";
  char magick[] = "CORE_RL_Magick++_.lib";
  char envvar[] = "PATH";

  /* Search for file in PATH environment variable: */
  _searchenv(cv, envvar, pathbuffer);
  if (*pathbuffer=='\0') {
    VERBOSE0(1, "CV.dll is not in the path, aborting");
    return FALSE;
  }

  _searchenv(magick, envvar, pathbuffer);
  if (*pathbuffer=='\0') {
    VERBOSE0(1, "CORE_RL_Magick++_.lib is not in the path, aborting");
    return FALSE;
  }

  return TRUE;
}



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CDXApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CDXApp message handlers

