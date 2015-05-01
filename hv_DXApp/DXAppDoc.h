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
  * $Id: DXAppDoc.h,v 1.8 2004/11/24 08:38:40 matz Exp $
**/

#include "DXManager.h"

#pragma once

class CDXAppDoc : public CDocument
{
protected: // create from serialization only
	CDXAppDoc();
	DECLARE_DYNCREATE(CDXAppDoc)
	virtual ~CDXAppDoc();

public:
  DXManager* GetDXManager() { return &m_DXManager; }

  bool StartDXManager(bool print_version);
  bool SetDefaults(const string& conductor_filename);
  void SetHandVuLogfilename(const string& handvu_logfilename);

protected:
  DXManager       m_DXManager;
  bool            m_source_window;

// Overrides
	public:
	virtual BOOL OnNewDocument();
//	virtual void Serialize(CArchive& ar);

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:
  afx_msg void OnOverlayLevel0();
  afx_msg void OnOverlayLevel1();
  afx_msg void OnOverlayLevel2();
  afx_msg void OnOverlayLevel3();
  afx_msg void OnExposureControl();
  afx_msg void OnLatency();
  afx_msg void OnLoadConductor();
  afx_msg void OnUndistort();
  afx_msg void OnToggleFullscreen();
  afx_msg void OnRestart();
  afx_msg void OnToggleMaintenanceapp();
  afx_msg void OnToggleFDLOnly();
  afx_msg void OnSourceWindow();
  afx_msg void OnTakeSnapshot();
#ifdef HAVE_USER_STUDY
  afx_msg void OnAbortStudyHV1Task();
  afx_msg void OnAbortStudyHV1Session();
  afx_msg void OnStartStudyHV1();
  afx_msg void OnStartStudyHV2();
#endif
};


