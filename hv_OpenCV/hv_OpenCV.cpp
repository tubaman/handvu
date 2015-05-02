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
* $Id: hv_OpenCV.cpp,v 1.15 2006/01/03 21:44:15 matz Exp $
**/

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <cv.h>
#include <highgui.h>
#include <ctype.h>
#include <time.h>

#include "HandVu.h"


IplImage *capture_image = 0;
IplImage *display_image = 0;

bool async_processing = false;
int num_async_bufs = 30;
IplImage *m_async_image = 0;
int m_async_bufID = -1;
bool sync_display = true;

CvPoint origin;
int select_object = 0;
int sel_area_left=0, sel_area_top=0, sel_area_right=0, sel_area_bottom=0;
bool correct_distortion = false;


void OnMouse( int event, int x, int y, int /*flags*/, void* /*params*/ )
{
  if( !capture_image )
    return;

  if( capture_image->origin )
    y = capture_image->height - y;

  if( select_object )
  {
    sel_area_left = MIN(x,origin.x);
    sel_area_top = MIN(y,origin.y);
    sel_area_right = sel_area_left + CV_IABS(x - origin.x);
    sel_area_bottom = sel_area_top + CV_IABS(y - origin.y);

    sel_area_left = MAX( sel_area_left, 0 );
    sel_area_top = MAX( sel_area_top, 0 );
    sel_area_right = MIN( sel_area_right, capture_image->width );
    sel_area_bottom = MIN( sel_area_bottom, capture_image->height );

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


void showFrame(IplImage* img, hvAction action)
{
  if (action==HV_DROP_FRAME) {
    // HandVu recommends dropping the frame entirely
    // printf("HandVuFilter: dropping frame\n");
    return;
  } else if (action==HV_SKIP_FRAME) {
    // HandVu recommends displaying the frame, but not doing any further
    // processing on it - keep going
    // printf("HandVuFilter: supposed to skip frame\n");
  } else if (action==HV_PROCESS_FRAME) {
    // full processing was done and is recommended for following steps;
    // keep going
    //printf("HandVuFilter: processed frame\n");
  } else {
    assert(0); // unknown action
  }
  
  hvState state;
  hvGetState(0, state);
  
  cvShowImage( "HandVu", img );
}


void displayCallback(IplImage* img, hvAction action)
{
  if (sync_display) {
    cvCopy(img, display_image);
  } else {
    showFrame(img, action);
  }
}


int main( int argc, char** argv )
{
  CvCapture* capture = 0;

  if (argc<2) {
    printf("you need to specify a conductor file as first argument\n");
    printf("for example: ../config/default.conductor\n");
    return -1;
  }

  string conductor_fname(argv[1]);
  printf("will load conductor from file:\n%s\n", conductor_fname.c_str());

  if( argc == 2 || argc == 3) {
    int num = 0;
    if (argc==3) {
      num = atoi(argv[2]);
    }
    capture = cvCaptureFromCAM( num );
    if (!capture) {
      capture = cvCaptureFromAVI( argv[2] ); 
    }
  }

  if( !capture )
  {
    fprintf(stderr,"Could not initialize capturing through OpenCV.\n");
    return -1;
  }

  printf( "Hot keys: \n"
    "\tESC - quit the program\n"
    "\tr - restart the tracking\n"
    "\t0-3 - set the overlay (verbosity) level\n"
    "use the mouse to select the initial detection area\n" );
 
  int p = 0; // according to docs, these calls don't work in OpenCV beta 4 yet
  p = cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 640);
  p = cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 480);

  capture_image = cvQueryFrame( capture );
  if ( !capture_image ) {
    fprintf(stderr,"Could not retrieve image through OpenCV.\n");
    return -1;
  }

  /* allocate all the buffers */
  CvSize size = cvGetSize(capture_image);
  hvInitialize(size.width, size.height);
  hvLoadConductor(conductor_fname);
  hvStartRecognition();
  hvStartGestureServer(7045);
  hvSetOverlayLevel(2);
  if (async_processing) {
    hvAsyncSetup(num_async_bufs, displayCallback);
    if (sync_display) display_image = cvCloneImage(capture_image);
  }

  cvSetMouseCallback( "HandVu", OnMouse );
  int success = cvNamedWindow( "HandVu", 1 );
  if (success!=1) {
    printf("can't open window - did you compile OpenCV with highgui support?");
    return -1;
  }
  fprintf(stderr, "initialized highgui\n");

  for (;;) {
    int c;
    
    if (async_processing) {
      // asynchronous processing in HandVu

      if (sync_display) cvShowImage("HandVu", display_image);
      
      // ------- main library call ---------
      hvAsyncGetImageBuffer(&m_async_image, &m_async_bufID);
      cvCopy(capture_image, m_async_image);
      hvAsyncProcessFrame(m_async_bufID);
      // -------
      
    } else {
      // synchronous processing in HandVu
      
      // ------- main library call ---------
      hvAction action = HV_INVALID_ACTION;
      action = hvProcessFrame(capture_image);
      // -------
      
      showFrame(capture_image, action);
      
    }
    
    c = cvWaitKey(10);
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
      default:
        ;
      }
    
    // capture next image
    capture_image = cvQueryFrame( capture );
    if ( !capture_image ) {
      fprintf(stderr,"Could not retrieve image through OpenCV.\n");
      break;
    }
  }
  
  cvReleaseCapture( &capture );
  cvDestroyWindow("HandVu");

  return 0;
}
