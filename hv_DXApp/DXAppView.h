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
  * $Id: DXAppView.h,v 1.2 2004/10/19 01:44:06 matz Exp $
**/

#pragma once

class CDXAppDoc;

class CDXAppView : public CScrollView
{
protected: // create from serialization only
	CDXAppView();
	DECLARE_DYNCREATE(CDXAppView)

// Attributes
public:
	CDXAppDoc* GetDocument() const;
  void SetTrapMouse(bool trap) { m_trap_mouse = trap; }

// Operations
public:

// Overrides
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CDXAppView();
	virtual void OnInitialUpdate();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
  void SetVideoWindowSize(int width, int height);
  double m_video_scale;
  CPoint m_virtual_mouse, m_local_fixed_mouse;
  bool m_trap_mouse;

// Generated message map functions
protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in DXAppView.cpp
inline CDXAppDoc* CDXAppView::GetDocument() const
   { return reinterpret_cast<CDXAppDoc*>(m_pDocument); }
#endif

