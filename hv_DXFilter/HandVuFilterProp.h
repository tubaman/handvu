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
  * $Id: HandVuFilterProp.h,v 1.2 2004/10/15 22:53:26 matz Exp $
**/

class CHandVuFilterProperties : public CBasePropertyPage
{

 public:

  static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

 private:

  BOOL OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
  HRESULT OnConnect(IUnknown *pUnknown);
  HRESULT OnDisconnect();
  HRESULT OnActivate();
  HRESULT OnDeactivate();
  HRESULT OnApplyChanges();

  void GetControlValues();
  void InitSlider(int id, int lower, int upper, int tic_freq );
  void SetSliderPos(int id, int pos);
  int GetSliderPos(int id);
  int GetText(int idc_textbox);
  void SetText(int idc_textbox, int val);
  int GetTextSliderCombination(int idc_textbox, int idc_slider, int val_old);
  void SetTextSliderCombination(int idc_textbox, int idc_slider, int val_old);

  CHandVuFilterProperties(LPUNKNOWN lpunk, HRESULT *phr);

  BOOL                 m_bIsInitialized;  // Used to ignore startup messages
  HandVuFilterParams  m_params;          // settings
  IHandVuFilter*      m_pIHandVuFilter; // The custom interface on the filter

}; // HandVuFilterProperties

