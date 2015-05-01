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
  * $Id: VisionConductor.cpp,v 1.23 2005/11/03 23:44:10 matz Exp $
**/

#include "Common.h"
#include "VisionConductor.h"
#include "FileHandling.h"
#include <fstream>
#ifdef HAVE_FLOAT_H
#include <float.h>
#endif

#ifdef USE_MFC
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif // _DEBUG
#endif // USE_MFC


#ifdef WIN32
#if defined(GetMessage)
#undef GetMessage
#endif
#else // WIN32
// otherwise declared in Common.h
void ReplaceAll(string& mangle, const string what, const string with);
#endif //WIN32

/////////////////////////////////////////////////////////////////////////////
// VisionConductor

VisionConductor::VisionConductor()
  : m_is_loaded(false),
    m_camera_calib(""),
    m_adjust_exposure(false),
  
    // detection:
    m_dt_radius(-1),
    m_dt_cascades_start(-1),
    m_dt_cascades_end(-1),
    m_dt_min_match_duration(-1),
    m_dt_min_color_coverage(-1),
    
    // tracking
    m_tr_num_KLT_features(-1),
    m_tr_min_KLT_features(-1),
    m_tr_cascades_start(-1),
    m_tr_cascades_end(-1),
    m_tr_winsize_width(-1),
    m_tr_winsize_height(-1),
    m_tr_min_feature_distance(-1),
    m_tr_type(VC_TT_UNDEF),

    // recognition
    m_rc_cascades_start(-1),
    m_rc_cascades_end(-1),
    m_rc_max_scan_width(-1),
    m_rc_max_scan_height(-1),
    m_rc_scale_tolerance(-1)
{
}

VisionConductor::~VisionConductor()
{
}

void VisionConductor::Load(string pathfile)
{
  // $IT_DATA environmet variable, NULL if not set
  const char *it_data = getenv("IT_DATA");
  string std_it_data;
  if (it_data!=NULL) {
    std_it_data = ConvertPathToStandard(it_data);
  } else {
    std_it_data = "";
  }

  // substitute $IT_DATA environment variable, if requested
  string::size_type pos = pathfile.find("$IT_DATA");
  if (pos!=string::npos) {
    if (it_data==NULL) {
      throw HVEFile(pathfile, "The filename requests the environment variable "
        "$IT_DATA to be set.");
    }
    // substitute "$IT_DATA" for the value of that environment variable
    ReplaceAll(pathfile, "$IT_DATA", std_it_data);
  }

  // all paths in the VisionConductor file "filename" are relative
  // to the path of "filename".  so, let's cd to that directory, load
  // all cascades etc., and in the end restore the previous cwd.
  string old_cwd = GetCWD();
  string vc_path, fname;
  SplitPathFile(pathfile, vc_path, fname);
  VERBOSE1(5, "HandVu: loading supplemental conductor files from path %s",
    vc_path.c_str());
  if (old_cwd!=vc_path) {
    SetCWD(vc_path);
  }

  m_masks.clear();
  m_orig_areas.clear();

  try {
    // the actual parsing function
    ParseFromFile(fname);
  } catch (HVException& hve) {
    if (old_cwd!=vc_path) {
      SetCWD(old_cwd);
    }
    throw HVException(hve.GetMessage() + "\n" +
      "note: paths in VisionConductor files are relative;\n"
      "in this case to " + vc_path);
  }

  if (old_cwd!=vc_path) {
    SetCWD(old_cwd);
  }

  SanityCheckMasks();

  m_is_loaded = true;
}


/** parsing function; do not call directly but 
* call "Load" instead
*/
void VisionConductor::ParseFromFile(const string& filename)
{
  ifstream file(filename.c_str());
  if (!file) {
    throw HVEFileNotFound(filename);
  }

  // $IT_DATA environmet variable, NULL if not set
  const char *it_data = getenv("IT_DATA");
  string std_it_data;
  if (it_data!=NULL) {
    std_it_data = ConvertPathToStandard(it_data);
  } else {
    std_it_data = "";
  }

  string line;
  do {
    getline(file, line);
  } while (line=="" || line[0]=='#');

  if (line==HV_CONDUCTOR_VERSION_1_5_STRING) {
	  // version 1.5

    do {
      getline(file, line);
    } while (line=="" || line[0]=='#');

    {
      char* buf = new char[line.length()];
      int scanned = sscanf(line.c_str(), "camera calibration: %s", buf);
      if (scanned!=1) {
        throw HVEFile(filename, string("expected camera calibration file, found: ")+line);
      }  
      m_camera_calib = string(buf);
      delete[] buf;

      // substitute $IT_DATA environment variable, if requested
      string::size_type pos = m_camera_calib.find("$IT_DATA");
      if (pos!=string::npos) {
        if (it_data==NULL) {
          throw HVEFile(filename, "The file requests the environment variable "
            "$IT_DATA to be set.");
        }
        // substitute "$IT_DATA" for the value of that environment variable
        ReplaceAll(m_camera_calib, "$IT_DATA", std_it_data);
      }

      if (m_camera_calib!="-") {
        FILE* fp = fopen(m_camera_calib.c_str(), "rb");
        if (!fp) {
          throw HVEFile(filename, string("can not find file: ")+m_camera_calib);
        }
      } else {
        m_camera_calib = "";
      }
    }


    // what sort of camera exposure control is desired
    do {
      getline(file, line);
    } while (line=="" || line[0]=='#');

    {
      char* buf = new char[line.length()];
      int scanned = sscanf(line.c_str(), "camera exposure: %s", buf);
      if (scanned!=1) {
        throw HVEFile(filename, string("expected camera exposure control setting, found: ")+line);
      }
      if (strcmp(buf, "camera")==0) {
        m_adjust_exposure = false;
      } else if (strcmp(buf, "software")==0) {
        m_adjust_exposure = true;
      } else {
        throw HVEFile(filename, string("expected camera exposure control setting (\"camera\" or \"software\"), found: ")+line);
      }
      delete[] buf;
    }

    // detection parameters
    do {
      getline(file, line);
    } while (line=="" || line[0]=='#');
    float coverage;
    float radius;
    int scanned = sscanf(line.c_str(), "detection params: coverage %f, duration %d, radius %f",
      &coverage, &m_dt_min_match_duration, &radius);
    if (scanned!=3) {
      throw HVEFile(filename, string("expected detection params, found: ")+line);
    }  
    m_dt_min_color_coverage = coverage;
    if (radius>1) {
      HVEFile(filename, "radius must be width-relative");
    }
    m_dt_radius = radius;

    // tracking parameters
    do {
      getline(file, line);
    } while (line=="" || line[0]=='#');
    float min_dist;
    float max_err;
    scanned = sscanf(line.c_str(), "tracking params: num_f %d, min_f %d, win_w %d, win_h %d, min_dist %f, max_err %f",
      &m_tr_num_KLT_features, &m_tr_min_KLT_features, &m_tr_winsize_width, &m_tr_winsize_height, &min_dist, &max_err);
    if (scanned!=6) {
      throw HVEFile(filename, string("expected tracking params, found: ")+line);
    }  
    m_tr_min_feature_distance = min_dist;
    if (max_err<=0) {
      m_tr_max_feature_error = DBL_MAX;  // DBL_MAX switches this off
    } else {
      m_tr_max_feature_error = max_err;
    }


    // tracking style
    do {
      getline(file, line);
    } while (line=="" || line[0]=='#');
    string::size_type pos = line.find("tracking style: ");
    if (pos==string::npos) {
      throw HVEFile(filename, string("expected tracking style, found: ")+line);
    }
    string style = line.substr(strlen("tracking style: "));
    if (style=="OPTICAL_FLOW_ONLY") {
      m_tr_type = VC_TT_OPTICAL_FLOW_ONLY;
    } else if (style=="OPTICAL_FLOW_COLOR") {
      m_tr_type = VC_TT_OPTICAL_FLOW_COLOR;
    } else if (style=="OPTICAL_FLOW_FLOCK") {
      m_tr_type = VC_TT_OPTICAL_FLOW_FLOCK;
    } else if (style=="OPTICAL_FLOW_COLORFLOCK") {
      m_tr_type = VC_TT_OPTICAL_FLOW_COLORFLOCK;
    } else if (style=="CAMSHIFT_HSV") {
      m_tr_type = VC_TT_CAMSHIFT_HSV;
    } else if (style=="CAMSHIFT_LEARNED") {
      m_tr_type = VC_TT_CAMSHIFT_LEARNED;
    } else {
      throw HVEFile(filename, string("wrong tracking style: ")+style);
    }

    // recognition params
    do {
      getline(file, line);
    } while (line=="" || line[0]=='#');
    float max_scan_width;
    float max_scan_height;
    scanned = sscanf(line.c_str(), "recognition params: max_scan_width %f, max_scan_height %f",
      &max_scan_width, &max_scan_height);
    if (scanned!=2) {
      throw HVEFile(filename, string("expected recognition params, found: ")+line);
    }  
    m_rc_max_scan_width = max_scan_width;
    m_rc_max_scan_height = max_scan_height;

    m_rc_scale_tolerance = 1.75;  // magic!


    int num;
    // detection cascades
    m_dt_cascades_start = 0;
    num = ReadScannerData(file, filename, "detection");
    m_dt_cascades_end = m_dt_cascades_start+num;

    // tracking cascades
    m_tr_cascades_start = m_dt_cascades_end;
    num = ReadScannerData(file, filename, "tracking");
    m_tr_cascades_end = m_tr_cascades_start+num;
    
    // recognition cascades
    m_rc_cascades_start = m_tr_cascades_end;
    num = ReadScannerData(file, filename, "recognition");
    m_rc_cascades_end = m_rc_cascades_start+num;

    //
    // load masks
    //
    do {
      getline(file, line);
    } while (line=="" || line[0]=='#');
    scanned = sscanf(line.c_str(), "%d masks", &num);
    if (scanned!=1) {
      throw HVEFile(filename, string("expected number of masks, found: ")+line);
    }
    for (int mcnt=0; mcnt<num; mcnt++) {
      Mask mask;
      string mask_filename;
      do {
        getline(file, mask_filename);
      } while (mask_filename=="" || mask_filename[0]=='#');
      // substitute $IT_DATA environment variable, if requested
      string::size_type pos = mask_filename.find("$IT_DATA");
      if (pos!=string::npos) {
        if (it_data==NULL) {
          throw HVEFile(filename, "The file requests the environment variable "
            "$IT_DATA to be set.");
        }
        // substitute "$IT_DATA" for the value of that environment variable
        ReplaceAll(mask_filename, "$IT_DATA", std_it_data);
      }

      mask.ParseFrom(ConvertPathToWindows(mask_filename).c_str());
      if (m_masks.find(mask.GetName())!=m_masks.end()) {
        throw HVException(string("mask '")+mask.GetName()+string("' exists already!"));
      }
      m_masks[mask.GetName()] = mask;
    }

  } else {
    throw HVEFile(filename, string("Unknown or no conductor file version: ")+line);
  }
}

void VisionConductor::SanityCheckMasks()
{
  for (int cc=0; cc<m_rc_cascades_end; cc++) {
    CuCascadeProperties cp;
    CuCascadeID id = (CuCascadeID) cc;
    cuGetCascadeProperties(id, cp);
    for (int nm=0; nm<(int)cp.names.size(); nm++) {
      if (m_masks.find(cp.names[nm])==m_masks.end()) {
        throw HVException(string("no mask for cascade '")+cp.names[nm]+string("'."));
      }
      ConstMaskIt mask = m_masks.find(cp.names[nm]);
      if ((*mask).second.GetWidth()!=cp.template_width
          || (*mask).second.GetHeight()!=cp.template_height)
      {
        throw HVException(string("mask size does not match cascade size ('")
                          + cp.names[nm] + string("')."));
      }
      double m = (*mask).second.GetImageAreaRatio();
      double c = cp.image_area_ratio;
      if (fabs(m-c)>0.00001)
      {
        throw HVException(string("mask ratio does not match cascade ratio ('")
                          + cp.names[nm]+string("')."));
      }
    }
  }
}

/* reads file names C and M and settings from the file, then
* loads cascade and mask from the file corresponding to C and M;
* adds the results to m_cascades and m_masks
*/
int VisionConductor::ReadScannerData(ifstream& file, const string filename, const string type) 
{
  // $IT_DATA environmet variable, NULL if not set
  const char *it_data = getenv("IT_DATA");
  string std_it_data;
  if (it_data!=NULL) {
    std_it_data = ConvertPathToStandard(it_data);
  } else {
    std_it_data = "";
  }

  string line;
  do {
    getline(file, line);
  } while (line=="" || line[0]=='#');
  int num;
  int scanned = sscanf(line.c_str(), (string("%d")
                                      +type+string(" cascades")).c_str(), &num);
  if (scanned!=1) {
    throw HVEFile(filename, string("expected number of ")
                  +type+string(" cascades, found: ")+line);
  }
  for (int cs=0; cs<num; cs++) {
    CuCascadeID cascadeID;
    //
    // load cascade
    //
    {
      string cascade_filename;
      do {
        getline(file, cascade_filename);
      } while (cascade_filename=="" || cascade_filename[0]=='#');
      // substitute $IT_DATA environment variable, if requested
      string::size_type pos = cascade_filename.find("$IT_DATA");
      if (pos!=string::npos) {
        if (it_data==NULL) {
          throw HVEFile(filename, "The file requests the environment variable "
            "$IT_DATA to be set.");
        }
        // substitute "$IT_DATA" for the value of that environment variable
        ReplaceAll(cascade_filename, "$IT_DATA", std_it_data);
      }

      cuLoadCascade(ConvertPathToWindows(cascade_filename).c_str(), &cascadeID);
    }

    //
    // read configuration of scanners
    //
    {
      float left, top, right, bottom;
      do {
        getline(file, line);
      } while (line=="" || line[0]=='#');
      scanned = 
        sscanf(line.c_str(), "area: left %f, top %f, right %f, bottom %f", 
        &left, &top, &right, &bottom);
      if (scanned!=4) {
        throw HVEFile(filename, string("expected area, found: ")+line);
      }
      if (!(0<=left && left<=1) || !(0<=top && top<=1)
          || !(0<=right && right<=1) || !(0<=bottom && bottom<=1)) 
      {
        throw HVEFile(filename, "areas must have relative coordinates");
      }
      do {
        getline(file, line);
      } while (line=="" || line[0]=='#');
      float start_scale, stop_scale, scale_inc_factor;
      scanned = sscanf(line.c_str(), 
        "params scaling: start %f, stop %f, inc_factor %f",
        &start_scale, &stop_scale, &scale_inc_factor);
      if (scanned!=3) {
        throw HVEFile(filename, string("expected params, found: ")+line);
      }
      float translation_inc_x, translation_inc_y;
      do {
        getline(file, line);
      } while (line=="" || line[0]=='#');
      int post_process;
      scanned = sscanf(line.c_str(), 
        "params misc: translation_inc_x %f, translation_inc_y %f, post_process %d",
        &translation_inc_x, &translation_inc_y, &post_process);
      if (scanned!=3) {
        throw HVEFile(filename, string("expected params, found: ")+line);
      } 

      CQuadruple orig_area(left, top, right, bottom);
      m_orig_areas.push_back(orig_area);

      CuScannerParameters sp;
      sp.active = false;
      sp.left = sp.top = sp.right = sp.bottom = -1;
      sp.start_scale = start_scale;
      sp.stop_scale = stop_scale;
      sp.scale_inc_factor = scale_inc_factor;
      sp.translation_inc_x = translation_inc_x;
      sp.translation_inc_y = translation_inc_y;
      sp.post_process = (post_process==1);
      cuSetScannerParameters(cascadeID, sp);
    }
  }
  return num;
}

bool VisionConductor::IsLoaded() const
{
  return m_is_loaded;
}

#pragma warning (disable:4786)
ConstMaskIt VisionConductor::GetMask(const string& name) const
{
  ConstMaskIt it = m_masks.find(name);
  if (it==m_masks.end()) {
    throw HVException(string("no mask for name ")+name);
  }
  return it;
}
#pragma warning (default:4786)


CRect CQuadruple::toRect(double scale_x, double scale_y) const
{
  return CRect((int)(left*scale_x), 
               (int)(top*scale_y),
               (int)(right*scale_x),
               (int)(bottom*scale_y));
}

void CQuadruple::fromRect(const CRect& rect, double scale_x, double scale_y) 
{
  left = (double)rect.left/scale_x;
  top = (double)rect.top/scale_y;
  right = (double)rect.right/scale_x;
  bottom = (double)rect.bottom/scale_y;
}


