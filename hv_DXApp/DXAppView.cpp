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
  * $Id: DXAppView.cpp,v 1.6 2004/10/19 01:44:06 matz Exp $
**/

#include "stdafx.h"
#include "Common.h"
#include "DXApp.h"
#include "DXAppDoc.h"
#include "DXAppView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDXAppView

IMPLEMENT_DYNCREATE(CDXAppView, CScrollView)

BEGIN_MESSAGE_MAP(CDXAppView, CScrollView)
  ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// CDXAppView construction/destruction

CDXAppView::CDXAppView()
: m_video_scale(-1),
  m_trap_mouse(false)
{
}

CDXAppView::~CDXAppView()
{
}

BOOL CDXAppView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

void CDXAppView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	SetScrollSizes(MM_TEXT, CSize(0, 0));
}

void CDXAppView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);

  VERBOSE2(4, "ApV: OnSize %d %d", cx, cy);
  SetVideoWindowSize(cx, cy);
//	Invalidate(true);
}

void CDXAppView::SetVideoWindowSize(int width, int height) 
{
  CDXAppDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

  int source_width, source_height;
  pDoc->GetDXManager()->GetVideoSourceSize(&source_width, &source_height);
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
  pDoc->GetDXManager()->SetVideoWindowPosition( rc.left, rc.top, width, height );
  m_video_scale = (double)(rc.right-rc.left)/(double)source_width;
  m_local_fixed_mouse.x = m_virtual_mouse.x = (rc.right-rc.left)/2;
  m_local_fixed_mouse.y = m_virtual_mouse.y = (rc.bottom-rc.top)/2;

  SetScrollSizes(MM_TEXT, CSize(source_width, source_height));

  VERBOSE2(4, "ApV: scroll size %d %d", source_width, source_height);
}


void CDXAppView::OnMouseMove(UINT nFlags, CPoint point) 
{
  if (m_trap_mouse) {
    /*
    VERBOSE2(3, TEXT("ApV: pt %d %d"), point.x, point.y));
    VERBOSE2(3, TEXT("ApV: fx %d %d"), m_local_fixed_mouse.x, m_local_fixed_mouse.y));
    VERBOSE2(3, TEXT("ApV: gg %d %d"), m_fixed_mouse.x, m_fixed_mouse.y));
    CPoint glob = m_fixed_mouse;
    ScreenToClient(&glob);
    VERBOSE2(3, TEXT("ApV: cc %d %d"), glob.x, glob.y));
    */
    if (point.x==m_local_fixed_mouse.x && point.y==m_local_fixed_mouse.y) {
      return;
    }

    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dx = m_local_fixed_mouse.x-point.x;
    input.mi.dy = m_local_fixed_mouse.y-point.y;
    input.mi.mouseData = 0;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    input.mi.time = 0;
  //  VERBOSE2(3, TEXT("ApV: will send %d %d"), input.mi.dx, input.mi.dy));
    int put = SendInput(1, &input, sizeof(INPUT));
    ASSERT(put==1);

    CRect rc;
    GetClientRect( &rc );
    m_virtual_mouse.x += point.x-m_local_fixed_mouse.x;
    m_virtual_mouse.y += point.y-m_local_fixed_mouse.y;
  //  VERBOSE2(3, TEXT("ApV: actual %d %d"), m_virtual_mouse.x, m_virtual_mouse.y));

    if (m_virtual_mouse.x<rc.left) m_virtual_mouse.x = rc.left;
    if (m_virtual_mouse.x>rc.right) m_virtual_mouse.x = rc.right;
    if (m_virtual_mouse.y<rc.top) m_virtual_mouse.y = rc.top;
    if (m_virtual_mouse.y>rc.bottom) m_virtual_mouse.y = rc.bottom;
    double x = (double)m_virtual_mouse.x/(double)(rc.right-rc.left);
    double y = (double)m_virtual_mouse.y/(double)(rc.bottom-rc.top);
    CDXAppDoc* pDoc = GetDocument();
	  ASSERT_VALID(pDoc);
    pDoc->GetDXManager()->HV()->OnMouseMove(nFlags, x, y);

  } else {
    // don't trap mouse
    CRect rc;
    GetClientRect( &rc );
    double x = (double)point.x/(double)(rc.right-rc.left);
    double y = (double)point.y/(double)(rc.bottom-rc.top);
    x = min(x, 1.0);
    y = min(y, 1.0);
    CDXAppDoc* pDoc = GetDocument();
	  ASSERT_VALID(pDoc);
    pDoc->GetDXManager()->HV()->OnMouseMove(nFlags, x, y);\

  	return CScrollView::OnMouseMove(nFlags, point);
  }
}

/* left button: click to set left-upper corner starting point of area
*/
void CDXAppView::OnLButtonDown(UINT nFlags, CPoint point) 
{
}

/* left button: after dragging, releasing button sets lower-right corner
* of area. update the Doc.
*/
void CDXAppView::OnLButtonUp(UINT nFlags, CPoint point) 
{
  CRect rc;
  GetClientRect( &rc );
  double x = (double)point.x/(double)(rc.right-rc.left);
  double y = (double)point.y/(double)(rc.bottom-rc.top);
  CDXAppDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
  pDoc->GetDXManager()->HV()->OnLButtonUp(nFlags, x, y);
}

void CDXAppView::OnDraw(CDC* /*pDC*/)
{
//	CDXAppDoc* pDoc = GetDocument();
//	ASSERT_VALID(pDoc);
}


// CDXAppView diagnostics

#ifdef _DEBUG
void CDXAppView::AssertValid() const
{
	CView::AssertValid();
}

void CDXAppView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CDXAppDoc* CDXAppView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDXAppDoc)));
	return (CDXAppDoc*)m_pDocument;
}
#endif //_DEBUG


// CDXAppView message handlers
