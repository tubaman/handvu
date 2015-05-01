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
  * $Id: HandVuFilterProp.cpp,v 1.5 2005/10/30 23:00:43 matz Exp $
**/

#include "StdAfx.h"
//#include <windowsx.h>
//#include <commctrl.h>
//#include <olectl.h>
//#include <memory.h>
//#include <tchar.h>
#include "resource.h"
#include "HandVuFilterGUIDs.h"
#include "Common.h"
#include "HandVuFilter.h"
#include "HandVuFilterProp.h"


//
// CreateInstance
//
// Used by the DirectShow base classes to create instances
//
CUnknown*
CHandVuFilterProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
  CUnknown *punk = new CHandVuFilterProperties(lpunk, phr);
  if (punk == NULL) {
    *phr = E_OUTOFMEMORY;
  }
  return punk;

} // CreateInstance


//
// Constructor
//
CHandVuFilterProperties::CHandVuFilterProperties(LPUNKNOWN pUnk, HRESULT *phr) :
  CBasePropertyPage(NAME("HandVuFilter Property Page"),
                    pUnk,IDD_HandVuFilterPROP,IDS_TITLE),
  m_pIHandVuFilter(NULL),
  m_bIsInitialized(FALSE)
{
  ASSERT(phr);
} // (Constructor)


//
// OnReceiveMessage
//
// Handles the messages for our property window
//
BOOL CHandVuFilterProperties::OnReceiveMessage(HWND hwnd,
                                                UINT uMsg,
                                                WPARAM wParam,
                                                LPARAM lParam)
{
  if (!m_bIsInitialized) {
    return (LRESULT) 1;
  }
  switch (uMsg)
    {
    case WM_HSCROLL:
      {	
        if (m_params.immediate_apply) {
          OnApplyChanges();
        } else {
          m_bDirty = TRUE;
          if (m_pPageSite) {
            m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
          }
        }
        return (LRESULT) 1;
        break;
      }

    case WM_COMMAND:
      {
        if (m_params.immediate_apply) {
          OnApplyChanges();
        } else {
          m_bDirty = TRUE;
          if (m_pPageSite) {
            m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
          }
        }
        return (LRESULT) 1;
        break;
      }

    }
  return (BOOL) CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
} // OnReceiveMessage


//
// OnConnect
//
// Called when we connect to a transform filter
//
HRESULT CHandVuFilterProperties::OnConnect(IUnknown *pUnknown)
{
  ASSERT(m_pIHandVuFilter == NULL);

  HRESULT hr = pUnknown->QueryInterface(IID_IHandVuFilter,
                                        (void **) &m_pIHandVuFilter);
  if (FAILED(hr)) {
    return E_NOINTERFACE;
  }

  ASSERT(m_pIHandVuFilter);

  // Get the initial image FX property
  m_pIHandVuFilter->GetHandVuFilterParams(m_params);
  m_bIsInitialized = FALSE ;
  return NOERROR;
} // OnConnect


//
// OnDisconnect
//
// Likewise called when we disconnect from a filter
//
HRESULT CHandVuFilterProperties::OnDisconnect()
{
  // Release of Interface after setting the appropriate old effect value

  if (m_pIHandVuFilter == NULL) {
    return E_UNEXPECTED;
  }

  m_pIHandVuFilter->Release();
  m_pIHandVuFilter = NULL;
  return NOERROR;
} // OnDisconnect


//
// OnActivate
//
// We are being activated
//
HRESULT CHandVuFilterProperties::OnActivate()
{
  CheckDlgButton(m_Dlg, IDC_IMMEDIATE_APPLY, m_params.immediate_apply );
  m_bIsInitialized = TRUE;
  return NOERROR;

} // OnActivate


//
// OnDeactivate
//
// We are being deactivated
//
HRESULT CHandVuFilterProperties::OnDeactivate(void)
{
  ASSERT(m_pIHandVuFilter);
  m_bIsInitialized = FALSE;
  GetControlValues();
  return NOERROR;

} // OnDeactivate


void CHandVuFilterProperties::InitSlider( int id, int lower, int upper,
                                           int tic_freq )
{
  HWND slider = GetDlgItem( m_Dlg, id );
  if( slider )
    {
      SendMessage( slider, TBM_SETRANGE, TRUE, MAKELONG(lower, upper) );

      if( tic_freq > 0 )
        {
          SendMessage( slider, TBM_SETTICFREQ, tic_freq, 0 );
        }
    }
}

void CHandVuFilterProperties::SetSliderPos( int id, int pos )
{
  HWND slider = GetDlgItem( m_Dlg, id );
  if( slider )
    {
      SendMessage( slider, TBM_SETPOS, TRUE, pos );
    }
}


int CHandVuFilterProperties::GetSliderPos( int id )
{
  HWND slider = GetDlgItem( m_Dlg, id );
  int pos = 0;

  if( slider )
    {
      pos = (int) SendMessage( slider, TBM_GETPOS, 0, 0 );
    }

  return pos;
}

void CHandVuFilterProperties::SetText( int idc_textbox, int pos ) {
  const int szlen = 4;
  TCHAR sz[szlen];
  Edit_GetText(GetDlgItem(m_Dlg, idc_textbox), sz, 4);
  if (pos != atoi(sz)) {
    //StringCbPrintf(sz, szlen*sizeof(TCHAR), TEXT("%2.d"), pos);
    _stprintf(sz, TEXT("%2.d"), pos);
    Edit_SetText(GetDlgItem(m_Dlg, idc_textbox), sz);
  }
}

int CHandVuFilterProperties::GetText( int idc_textbox ) {
  TCHAR sz[4];
  Edit_GetText(GetDlgItem(m_Dlg, idc_textbox), sz, 4);
  return atoi(sz);
}

void CHandVuFilterProperties::SetTextSliderCombination( int idc_textbox,
                                                         int idc_slider,
                                                         int pos ) {
  if (pos != GetSliderPos( idc_slider )) {
    SetSliderPos(idc_slider, pos);
  }
  const int szlen = 4;
  TCHAR sz[szlen];
  Edit_GetText(GetDlgItem(m_Dlg, idc_textbox), sz, 4);
  if (pos != atoi(sz)) {
    //StringCbPrintf(sz, szlen*sizeof(TCHAR), TEXT("%2.d"), pos);
    _stprintf(sz, TEXT("%2.d"), pos);
    Edit_SetText(GetDlgItem(m_Dlg, idc_textbox), sz);
  }
}


int CHandVuFilterProperties::GetTextSliderCombination(int idc_textbox,
                                                       int idc_slider,
                                                       int val_old) {
  int val_textbox;
  int val_slider;
  TCHAR sz[4];

  Edit_GetText(GetDlgItem(m_Dlg, idc_textbox), sz, 4);
  val_textbox = atoi(sz);

  // Quick validatation of the field

  if (val_textbox < 0) val_textbox = 0;
  if (val_textbox > 255) val_textbox = 255;

  val_slider = GetSliderPos( idc_slider );

  // textbox has priority
  if (val_old != val_textbox) {
    return val_textbox;
  } else if (val_old != val_slider) {
    return val_slider;
  } else {
    return val_old;
  }
}


//
// OnApplyChanges
//
// Apply any changes so far made
//
HRESULT CHandVuFilterProperties::OnApplyChanges()
{
  GetControlValues();
  m_pIHandVuFilter->SetHandVuFilterParams(m_params);

  return NOERROR;
} // OnApplyChanges


void CHandVuFilterProperties::GetControlValues()
{
  ASSERT(m_pIHandVuFilter);

  m_params.immediate_apply = IsDlgButtonChecked(m_Dlg, IDC_IMMEDIATE_APPLY );
}






