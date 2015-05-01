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
  * $Id: VisionConductor.h,v 1.18 2005/10/30 21:16:04 matz Exp $
**/

#ifndef __VISIONCONDUCTOR_H__
#define __VISIONCONDUCTOR_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
using namespace std;

#include "HandVu.hpp"
#include "Mask.h"
#include "Quadruple.h"

class VisionConductor 
{
 public:
  VisionConductor();
  ~VisionConductor();

public:
  // Operations
 public:
  void Load(string filename);
  bool IsLoaded() const;
  ConstMaskIt GetMask(const string& name) const;

 protected:
#pragma warning (disable: 4786)
  void ParseFromFile(const string& filename);
  int ReadScannerData(ifstream& file, const string filename, const string type);
#pragma warning (default: 4786)
  void LoadMask();
  void SanityCheckMasks();

public:
  enum TrackingType {
    VC_TT_UNDEF = 0,
    VC_TT_OPTICAL_FLOW_ONLY = 1,
    VC_TT_OPTICAL_FLOW_FLOCK = 2,
    VC_TT_OPTICAL_FLOW_COLOR = 4,
    VC_TT_OPTICAL_FLOW_COLORFLOCK = 8,
    VC_TT_CAMSHIFT_HSV = 16,
    VC_TT_CAMSHIFT_LEARNED = 32,
    VC_TT_CAMSHIFT = VC_TT_CAMSHIFT_HSV|VC_TT_CAMSHIFT_LEARNED,
    VC_TT_OPTICAL_FLOW = VC_TT_OPTICAL_FLOW_ONLY|VC_TT_OPTICAL_FLOW_FLOCK|VC_TT_OPTICAL_FLOW_COLORFLOCK
  };

 protected:
  // general
  CQuadrupleVector        m_orig_areas;
  MaskMap                 m_masks;
  bool                    m_is_loaded;
  string                  m_camera_calib;
  bool                    m_adjust_exposure;

  // detection
  int                     m_dt_cascades_start;
  int                     m_dt_cascades_end;
  long                    m_dt_min_match_duration;
  double                  m_dt_radius;
  double                  m_dt_min_color_coverage;

  // tracking
  int                     m_tr_cascades_start;
  int                     m_tr_cascades_end;
  int                     m_tr_num_KLT_features;
  int                     m_tr_min_KLT_features; // how many of them must survive or tracking is aborted
  int                     m_tr_winsize_width;
  int                     m_tr_winsize_height;
  double                  m_tr_min_feature_distance;  // min pairwise distance
  double                  m_tr_max_feature_error;
  TrackingType            m_tr_type;

  // recognition
  int                     m_rc_cascades_start;
  int                     m_rc_cascades_end;
  double                  m_rc_max_scan_width;
  double                  m_rc_max_scan_height;
  double                  m_rc_scale_tolerance;

 public:
  friend class HandVu;
};

#endif // __VISIONCONDUCTOR_H__

