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
  * $Id: HandVu.h,v 1.24 2005/10/30 21:16:04 matz Exp $
**/

// C-interface

#if !defined(__HANDVU_H__INCLUDED_)
#define __HANDVU_H__INCLUDED_

#include <cv.h>
#include <string>
#include <vector>
using namespace std;


/* reference time
*/
#ifdef WIN32
typedef __int64 REFERENCE_TIME;
typedef REFERENCE_TIME RefTime;
#else
typedef long long RefTime;
#endif // WIN32

/* state for an object such as the right hand
*/
typedef struct _hvState {
  int        obj_id;
  bool       tracked;
  bool       recognized;
  double     center_xpos, center_ypos;
  double     scale;
  string     posture;
  RefTime    tstamp;
} hvState;

enum hvAction {         // specify recommendations to application:
  HV_INVALID_ACTION = 0,
  HV_PROCESS_FRAME = 1, // fully process and display the frame
  HV_SKIP_FRAME = 2,    // display but do not further process
  HV_DROP_FRAME = 3     // do not display the frame
};
  
void hvInitialize(int width, int height);
void hvUninitialize();

void hvLoadConductor(const string& filename);
bool hvConductorLoaded();

void hvStartRecognition(int obj_id=0);
void hvStopRecognition(int obj_id=0);

hvAction hvProcessFrame(IplImage* inOutImage, IplImage* rightImage=NULL);
bool hvIsActive();

void hvAsyncSetup(int num_buffers, void (*cb)(IplImage* img, hvAction action));
void hvAsyncGetImageBuffer(IplImage** pImage, int* pBufferID);
void hvAsyncProcessFrame(int bufferID);

void hvGetState(int obj_id, hvState& state);

void hvSetDetectionArea(int left, int top, int right, int bottom);
void hvGetDetectionArea(int* pLeft, int* pTop, int* pRight, int* pBottom);
void hvRecomputeNormalLatency();

void hvSetOverlayLevel(int level);
int hvGetOverlayLevel();

void hvCorrectDistortion(bool enable=true);
bool hvIsCorrectingDistortion();
bool hvCanCorrectDistortion();

void hvSetAdjustExposure(bool enable=true);
bool hvCanAdjustExposure();
bool hvIsAdjustingExposure();

void hvSetLogfile(const string& filename);
void hvSaveScannedArea(IplImage* pImg, string& picfile);
void hvSaveImageArea(IplImage* pImg, int left, int top, int right, int bottom, string& picfile);
void hvSetSaveFilenameRoot(const string& fname_root);

void hvSetDoTrack(bool do_track);

void hvStartGestureServer(int port, int max_num_clients=10);
void hvStartOSCServer(const string& desthost, int destport);
void hvStopGestureServer(int port);
void hvStopOSCServer(const string& desthost, int destport);

/** verbosity: 0 minimal, 3 maximal
*/
void hvGetVersion(string& version, int verbosity);


#endif // __HANDVU_H__INCLUDED_

