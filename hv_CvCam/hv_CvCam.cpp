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
* $Id: hv_CvCam.cpp,v 1.4 2006/01/08 19:32:11 matz Exp $
**/

#include <stdio.h>
#include <cv.h>
#include <highgui.h>
#include <cvcam.h>

#include "HandVu.h"


CvPoint origin;
int select_object = 0;
int sel_area_left=0, sel_area_top=0, sel_area_right=0, sel_area_bottom=0;
bool correct_distortion = false;
int img_width = 0, img_height = 0;
char* mainWindow =  "HandVu";
bool is_initialized = false;
string conductor_fname;
bool verbose_output = false;
string logfilename;

void DirectShowCallback(IplImage* image);


void OnMouse( int event, int x, int y, int /*flags*/, void* /*params*/ )
{
  if( img_width==0 || img_height==0 )
    return;

  if( select_object )
  {
    sel_area_left = MIN(x,origin.x);
    sel_area_top = MIN(y,origin.y);
    sel_area_right = sel_area_left + CV_IABS(x - origin.x);
    sel_area_bottom = sel_area_top + CV_IABS(y - origin.y);

    sel_area_left = MAX( sel_area_left, 0 );
    sel_area_top = MAX( sel_area_top, 0 );
    sel_area_right = MIN( sel_area_right, img_width );
    sel_area_bottom = MIN( sel_area_bottom, img_height );

    if( sel_area_right-sel_area_left > 0 && sel_area_bottom-sel_area_top> 0 )
      hvSetDetectionArea(sel_area_left, sel_area_top,
                         sel_area_right, sel_area_bottom);
  }

  switch( event )
  {
  case CV_EVENT_LBUTTONDOWN:
    origin = cvPoint(x,y);
    sel_area_left = sel_area_right = x;
    sel_area_top = sel_area_bottom = y;
    select_object = 1;
    break;
  case CV_EVENT_LBUTTONUP:
    select_object = 0;
    break;
  }
}

extern int g_verbose;
extern FILE* g_ostream;
int main( int argc, char** argv )
{
  if (argc<2) {
    printf("you need to specify a conductor file as first argument\n");
    printf("for example: config/default.conductor\n");
    return -1;
  }
  if (argc>2) {
    verbose_output = true;
    printf("assuming command line arguments:\n");
    printf("conductor_file: %s\n", argv[1]);
    printf("log_file: %s\n",argv[2]);
    logfilename = string(argv[2]);
    string file_version = "hv_CvCam: $Id: hv_CvCam.cpp,v 1.4 2006/01/08 19:32:11 matz Exp $";
    printf("file version: %s\n", file_version.c_str());
  }    

  conductor_fname = string(argv[1]);
  printf("will load conductor from file:\n%s\n", conductor_fname.c_str());
  fflush(stdout);

  int success = cvNamedWindow( mainWindow, CV_WINDOW_AUTOSIZE );
  if (success!=1) {
    printf("can't open window - did you compile OpenCV with highgui support?");
    return -1;
  }

  int ncams = cvcamGetCamerasCount();
  if (ncams<1) {
    fprintf(stderr, "no cameras connected according to OpenCV CvCam interface\n");
    exit(-1);
  }

  // set up CvCam capture interface
  void* hwnd = cvGetWindowHandle(mainWindow);
  cvcamSetProperty(0, CVCAM_PROP_ENABLE, CVCAMTRUE);
  cvcamSetProperty(0, CVCAM_PROP_CALLBACK, (void*) DirectShowCallback);
  cvcamSetProperty(0, CVCAM_PROP_WINDOW, &hwnd);
  //cvcamSetProperty(0, CVCAM_PROP_RENDER, CVCAMFALSE);

  //Set Video Format Property
  VidFormat vidFmt = {640, 480, 30.0};
  cvcamSetProperty( 0, CVCAM_PROP_SETFORMAT, &vidFmt);

  if( !cvcamInit() ) {
    fprintf(stderr, "could not initialize OpenCV CvCam interface\n");
    exit(-1);
  }

  printf( "Hot keys: \n"
    "\tESC - quit the program\n"
    "\tr   - restart the tracking\n"
    "\t0-3 - set the overlay (verbosity) level\n"
    "\tp   - show camera property window\n"
    "use the mouse to select the initial detection area\n"
    "gesture server started on port 7045 - use telnet to connect\n" );
 
  int error = cvcamStart(); // this does not block
  if (error==-1) {
    fprintf(stderr, "could not start OpenCV CvCam interface\n");
    exit(-1);
  }

  for (;;) {
    int c;
    c = cvWaitKey(0);
    if( c == 27 || c == 'q' )
      break;
    switch( c )
      {
      case 'r':
        hvStopRecognition();
        hvStartRecognition();
        break;
      case '0':
        hvSetOverlayLevel(0);
        break;
      case '1':
        hvSetOverlayLevel(1);
        break;
      case '2':
        hvSetOverlayLevel(2);
        break;
      case '3':
        hvSetOverlayLevel(3);
        break;
      case 'u':
        if (hvCanCorrectDistortion()) {
          correct_distortion = !correct_distortion;
          hvCorrectDistortion(correct_distortion);
        }
        break;
      case 'p':
        cvcamGetProperty(0, CVCAM_CAMERAPROPS, NULL);
        break;
      default:
        ;
      }
  }

 	cvcamStop();
	cvcamExit();
  cvDestroyWindow(mainWindow);
	return 0;
}

void DirectShowCallback(IplImage* image)
{
  if (!is_initialized) {
    /* allocate all the buffers */
    CvSize size = cvGetSize(image);
    hvInitialize(size.width, size.height);
    if (verbose_output) {
      g_verbose = 3;
      // create and set log file
      g_ostream = fopen(logfilename.c_str(), "a+");
      if (!g_ostream) {
        printf("can not create log_file %s, aborting\n", logfilename.c_str());
        exit(-1);
      }
      fprintf(g_ostream, "Starting HandVu's hv_CvCam\n");
      fprintf(g_ostream, "logging to (this) file: %s\n", logfilename.c_str());
      //fprintf(g_ostream, "%s\n", file_version.c_str());
      string hv_version;
      hvGetVersion(hv_version, 3);
      fprintf(g_ostream, "%s\n", hv_version.c_str());
      fflush(g_ostream);
    }
    hvLoadConductor(conductor_fname);
    hvStartRecognition();
    hvSetOverlayLevel(2);
    hvStartGestureServer(7045);

    cvSetMouseCallback( mainWindow, OnMouse );
    cvShowImage(mainWindow, image); // to set the size right
    is_initialized = true;
  }

#if defined(WIN32) || defined(CYGWIN)
  image->origin = 1;
#endif

  // ------- main library call ---------
  hvAction action = HV_INVALID_ACTION;
  action = hvProcessFrame(image);
  // -------
  
#if defined(WIN32) || defined(CYGWIN)
  image->origin = 0;
  cvMirror(image, NULL, 0);
#endif
}
