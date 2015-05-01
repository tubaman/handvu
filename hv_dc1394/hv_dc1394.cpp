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
  * $Id: hv_dc1394.cpp,v 1.7 2005/12/08 23:25:41 matz Exp $
**/

// hv_dc1394.c: frame capture from Linux' libdc1394 interface.  This
// code started from Rev 1.4 of dc1394_multiview.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xvlib.h>
#include <X11/keysym.h>
#include <getopt.h>

#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>

// OpenCV
#include <cxcore.h>
#include <cv.h>

// HandVu
#include "Common.h"
#include "HandVu.h"

/* uncomment the following to drop frames to prevent delays */
#define DROP_FRAMES 1
#define MAX_PORTS 3
#define MAX_CAMERAS 8
#define NUM_BUFFERS 8

/* ok the following constant should be by right included thru in Xvlib.h */
#ifndef XV_YV12
#define XV_YV12 0x32315659
#endif

#ifndef XV_YUY2
#define XV_YUY2 0x32595559
#endif

#ifndef XV_UYVY
#define XV_UYVY 0x59565955
#endif


/* declarations for libdc1394 */
int                       numPorts = MAX_PORTS;
raw1394handle_t           handles[MAX_PORTS];
int                       numCameras = 0;
dc1394_cameracapture      cameras[MAX_CAMERAS];
nodeid_t                 *camera_nodes;
dc1394_feature_set        features;

/* declarations for video1394 */
char                     *device_name = NULL;

/* declarations for Xwindows */
Display                  *display = NULL;
Window                    window = (Window) NULL;
long                      width, height;
long                      device_width, device_height;
int                       connection = -1;
XvImage                  *xv_image = NULL;
XvAdaptorInfo            *info;
long                      format = 0;
GC                        gc;

/* declarations for OpenCV and HandVu */
IplImage                 *iplImages[MAX_CAMERAS];
IplImage                 *readOnlyImg = NULL;
char* conductor_fname = "../config/default.conductor";

bool async_processing = false;
int num_async_bufs = 30;
int m_async_bufID = -1;
IplImage *m_async_image = 0;
bool sync_display = async_processing && true;
IplImage* async_display_image = 0;

/* Other declarations */
long                      frame_length;
long                      frame_free;
int                       frame = 0;
int                       adaptor = -1;

int                       freeze = 0;
int                       average = 0;
int                       fps;
int                       res;
int                       fullscreen;
unsigned char            *frame_buffer = NULL;
int                       verbose = 0;


static struct option      long_options[] = {
  {"device", 1, NULL, 0},
  {"fps", 1, NULL, 0},
  {"res", 1, NULL, 0},
  {"fullscreen", 1, NULL, 0},
  {"conductor", 1, NULL, 0},
  {"help", 0, NULL, 0},
  {NULL, 0, 0, 0}
};

void
get_options(int argc, char *argv[])
{
  int option_index = 0;
  fps = 15;
  res = 2;
  fullscreen = 0;

  while (getopt_long(argc, argv, "", long_options, &option_index) >= 0) {
    if (optarg) {
      switch (option_index) {
        /* case values must match long_options */
      case 0:
        device_name = strdup(optarg);
        break;
      case 1:
        fps = atoi(optarg);
        break;
      case 2:
        res = atoi(optarg);
        break;
      case 3:
        fullscreen = atoi(optarg);
        break;
      case 4:
        conductor_fname = strdup(optarg);
        break;
      }
    }
    if (option_index == 5) {
      printf("\n"
             "        %s - multi-cam monitor for libdc1394 and XVideo\n\n"
             "Usage:\n"
             "        %s [--fps=[1,3,7,15,30]] [--res=[0,1,2]] [--device=/dev/video1394/x]\n"
             "             --fps    - frames per second. default=15,\n"
             "                        30 not compatible with --res=2\n"
             "             --res    - resolution. 0 = 320x240,\n"
             "                        1 = 640x480 YUV4:1:1, 2 = 640x480 RGB8 (default)\n"
             "             --device - specifies video1394 device to use (optional)\n"
             "                        default = /dev/video1394/<port#>\n"
             "             --fullscreen\n"
             "             --conductor   - handvu configuration file\n"
             "             --help   - prints this message\n\n"
             "Keyboard Commands:\n"
             "        r = reset detection\n"
             "        u = toggle undistortion on/off\n"
             "        0-3 = set overlay level\n"
             "        ESC/q = quit\n"
             "        < -or- , = decrease scale\n"
             "        > -or- . = increase scale\n", argv[0], argv[0]);
      exit(0);
    }
  }

}

/* image format conversion functions */

static inline void
iyu12yuy2(unsigned char *src, unsigned char *dest, int NumPixels)
{
  int                       i = 0, j = 0;
  register int              y0, y1, y2, y3, u, v;
  while (i < NumPixels * 3 / 2) {
    u = src[i++];
    y0 = src[i++];
    y1 = src[i++];
    v = src[i++];
    y2 = src[i++];
    y3 = src[i++];

    dest[j++] = y0;
    dest[j++] = u;
    dest[j++] = y1;
    dest[j++] = v;

    dest[j++] = y2;
    dest[j++] = u;
    dest[j++] = y3;
    dest[j++] = v;
  }
}


/* macro by Bart Nabbe */
#define RGB2YUV(r, g, b, y, u, v)\
  y = (9798*r + 19235*g + 3736*b)  / 32768;\
  u = (-4784*r - 9437*g + 14221*b)  / 32768 + 128;\
  v = (20218*r - 16941*g - 3277*b) / 32768 + 128;\
  y = y < 0 ? 0 : y;\
  u = u < 0 ? 0 : u;\
  v = v < 0 ? 0 : v;\
  y = y > 255 ? 255 : y;\
  u = u > 255 ? 255 : u;\
  v = v > 255 ? 255 : v

static inline void
rgb2yuy2(unsigned char *RGB, unsigned char *YUV, int NumPixels)
{
  int                       i, j;
  register int              y0, y1, u0, u1, v0, v1;
  register int              r, g, b;

  for (i = 0, j = 0; i < 3 * NumPixels; i += 6, j += 4) {
    r = RGB[i + 0];
    g = RGB[i + 1];
    b = RGB[i + 2];
    RGB2YUV(r, g, b, y0, u0, v0);
    r = RGB[i + 3];
    g = RGB[i + 4];
    b = RGB[i + 5];
    RGB2YUV(r, g, b, y1, u1, v1);
    YUV[j + 0] = y0;
    YUV[j + 1] = (u0 + u1) / 2;
    YUV[j + 2] = y1;
    YUV[j + 3] = (v0 + v1) / 2;
  }
}

/* helper functions */

void
set_frame_length(long size, int numCameras)
{
  frame_length = size;
  if (verbose) 
    fprintf(stderr, "Setting frame size to %ld kb\n", size / 1024);
  frame_free = 0;
  frame_buffer = (unsigned char *) malloc(size * numCameras);
}


void
capture_frame()
{
  static int                have_warned = 0;

  dc1394_dma_multi_capture(cameras, numCameras);

  if (!freeze && adaptor >= 0) {
    for (int i = 0; i < numCameras; i++) {
      // create OpenCV image
      switch (res) {
      case MODE_640x480_YUV411:
	if (!have_warned) {
	  have_warned = 1;
	  printf("WARNING: no OpenCV conversion available for this format\n");
	}
        //iyu12yuy2((unsigned char *) cameras[i].capture_buffer,
	//        frame_buffer + (i * frame_length),
	//        (device_width * device_height));
        break;

      case MODE_320x240_YUV422:
      case MODE_640x480_YUV422:
	if (!have_warned) {
	  have_warned = 1;
	  printf("WARNING: no OpenCV conversion available for this format\n");
	}
	//        memcpy(frame_buffer + (i * frame_length),
	//     cameras[i].capture_buffer, device_width * device_height * 2);
        break;

      case MODE_640x480_RGB:
	// don't convert for OpenCV, this is the correct format
	readOnlyImg->imageData = (char *) cameras[i].capture_buffer;
        if (async_processing) {
          if (i==0) {
            hvAsyncGetImageBuffer(&m_async_image, &m_async_bufID);
            cvCopy(readOnlyImg, m_async_image);
          }
        } else {
          cvCopy(readOnlyImg, iplImages[i]);
        }
        break;
      }
      
      if (verbose) {
        CvFont                    font;
        cvInitFont(&font, CV_FONT_VECTOR0, 0.5f /* hscale */ ,
                   0.5f /* vscale */ , 0.1f /*italic_scale */ ,
                   1 /* thickness */ , 8);
        char                      str[256];
        sprintf(str, "camera id: %d", i);
        CvSize                    textsize;
        int                       underline;
        cvGetTextSize(str, &font, &textsize, &underline);
        CvPoint                   pos =
          cvPoint(iplImages[i]->width - textsize.width - 5,
                  textsize.height + 10);
        cvPutText(iplImages[i], str, pos, &font, CV_RGB(0, 255, 0));

	//        cvRectangle(iplImages[i], cvPoint(10, 10), cvPoint(100, 100),
        //            cvScalar(255, 255, 255, 255), CV_FILLED, 8, 0);
      }
    }
  }

  for (int i = 0; i < numCameras; i++) {
    dc1394_dma_done_with_buffer(&cameras[i]);
  }
}

void process_frame()
{
  if (!freeze && adaptor >= 0) {
    // main HandVu call
    if (async_processing) {
      hvAsyncProcessFrame(m_async_bufID);
    } else {
      hvProcessFrame(iplImages[0], iplImages[1]);
    }
  }
}


void display_frame()
{
  if (!freeze && adaptor >= 0) {
    // copy into frame buffer for display
    for (int i = 0; i < numCameras; i++) {
      if (sync_display && i==0) {
        rgb2yuy2((unsigned char *) async_display_image->imageData,
                 frame_buffer + (i * frame_length),
                 (device_width * device_height));
      } else {
        rgb2yuy2((unsigned char *) iplImages[i]->imageData,
                 frame_buffer + (i * frame_length),
                 (device_width * device_height));
      }
   }

    xv_image =
      XvCreateImage(display, info[adaptor].base_id, format, 
		    (char*) frame_buffer,
                    device_width, device_height * numCameras);
    XvPutImage(display, info[adaptor].base_id, window, gc, xv_image, 0, 0,
               device_width, device_height * numCameras, 0, 0, width, height);

    xv_image = NULL;
  }

  XFlush(display);
}


void displayCallback(IplImage* image, hvAction action)
{
  if (sync_display) {
    cvCopy(image, async_display_image);
  } else {
    cvCopy(image, iplImages[0]);
    display_frame();
  }
}


void
cleanup(void)
{
  int                       i;
  for (i = 0; i < numCameras; i++) {
    dc1394_dma_unlisten(handles[cameras[i].port], &cameras[i]);
    dc1394_dma_release_camera(handles[cameras[i].port], &cameras[i]);
  }
  for (i = 0; i < numPorts; i++)
    raw1394_destroy_handle(handles[i]);
  if ((void *) window != NULL)
    XUnmapWindow(display, window);
  if (display != NULL)
    XFlush(display);
  if (frame_buffer != NULL)
    free(frame_buffer);

  // OpenCV cleanup
  for (i = 0; i < numCameras; i++) {
    cvReleaseImageHeader(&iplImages[i]);
  }
}


void
handle_events()
{
  XEvent                    xev;

  while (XPending(display) > 0) {
    XNextEvent(display, &xev);
    switch (xev.type) {
    case ConfigureNotify:
      width = xev.xconfigure.width;
      height = xev.xconfigure.height;
      //      capture_frame();
      //      process_frame();
      display_frame();
      break;
    case KeyPress:
      switch (XKeycodeToKeysym(display, xev.xkey.keycode, 0)) {
      case XK_q:
      case XK_Q:
      case XK_Escape:
        cleanup();
        exit(0);
        break;
      case XK_r:
      case XK_R:
	hvStopRecognition();
	hvStartRecognition();
        break;
      case XK_u:
      case XK_U:
        if (hvCanCorrectDistortion()) {
          hvCorrectDistortion(!hvIsCorrectingDistortion());
        }
        break;
      case XK_comma:
      case XK_less:
        width = (width*3)/4;
        height = (height*3)/4;
        XResizeWindow(display, window, width, height);
        //        capture_frame();
        //        process_frame();
        display_frame();
        break;
      case XK_period:
      case XK_greater:
        width = (int) (1.25 * width);
        height = (int) (1.25 * height);
        XResizeWindow(display, window, width, height);
        //        capture_frame();
        //        process_frame();
        display_frame();
        break;
      case XK_0:
	hvSetOverlayLevel(0);
        break;
      case XK_1:
	hvSetOverlayLevel(1);
        break;
      case XK_2:
	hvSetOverlayLevel(2);
        break;
      case XK_3:
	hvSetOverlayLevel(3);
        break;
      }
      break;
    }
  }
}

void
QueryXv()
{
  int                       num_adaptors;
  int                       num_formats;
  XvImageFormatValues      *formats = NULL;
  int                       i, j;
  char                      xv_name[5];

  XvQueryAdaptors(display, DefaultRootWindow(display),
		  (unsigned int*) &num_adaptors, &info);

  for (i = 0; i < num_adaptors; i++) {
    formats = XvListImageFormats(display, info[i].base_id, &num_formats);
    for (j = 0; j < num_formats; j++) {
      xv_name[4] = 0;
      memcpy(xv_name, &formats[j].id, 4);
      if (formats[j].id == format) {
        if (verbose) 
	  fprintf(stderr, "using Xv format 0x%x %s %s\n", formats[j].id,
		  xv_name,
		  (formats[j].format == XvPacked) ? "packed" : "planar");
        if (adaptor < 0)
          adaptor = i;
      }
    }
  }
  XFree(formats);
  if (adaptor < 0)
    fprintf(stderr, "No suitable Xv adaptor found");

}


/* trap ctrl-c */
void
signal_handler(int sig)
{
  signal(SIGINT, SIG_IGN);
  cleanup();
  exit(0);
}

void
set_manual_exposure_gain(int camID, int exposure, int gain)
{
  int                       success =
    dc1394_auto_on_off(handles[cameras[camID].port], cameras[camID].node,
                       FEATURE_EXPOSURE, DC1394_FALSE);
  if (success != DC1394_SUCCESS) {
    fprintf(stderr, "can not turn off camera auto exposure\n");
    return;
  }

  success =
    dc1394_set_exposure(handles[cameras[camID].port], cameras[camID].node,
                        exposure);
  if (success != DC1394_SUCCESS) {
    fprintf(stderr, "set_exposure returned %d\n", success);
    return;
  }

  success =
    dc1394_set_gain(handles[cameras[camID].port], cameras[camID].node, gain);
  if (success != DC1394_SUCCESS) {
    fprintf(stderr, "set_gain returned %d\n", success);
    return;
  }
}

int
main(int argc, char *argv[])
{
  XGCValues                 xgcv;
  long                      background = 0x010203;
  unsigned int              channel;
  unsigned int              speed;
  int                       i, p, cn;
  raw1394handle_t           raw_handle;
  struct raw1394_portinfo   ports[MAX_PORTS];

  get_options(argc, argv);
  /* process options */
  switch (fps) {
  case 1:
    fps = FRAMERATE_1_875;
    break;
  case 3:
    fps = FRAMERATE_3_75;
    break;
  case 15:
    fps = FRAMERATE_15;
    break;
  case 30:
    fps = FRAMERATE_30;
    break;
  case 60:
    fps = FRAMERATE_60;
    break;
  default:
    fps = FRAMERATE_7_5;
    break;
  }
  switch (res) {
  case 1:
    res = MODE_640x480_YUV411;
    device_width = 640;
    device_height = 480;
    format = XV_YUY2;
    break;
  case 2:
    res = MODE_640x480_RGB;
    device_width = 640;
    device_height = 480;
    format = XV_YUY2;
    break;
  default:
    res = MODE_320x240_YUV422;
    device_width = 320;
    device_height = 240;
    format = XV_UYVY;
    break;
  }

  /* get the number of ports (cards) */
  raw_handle = raw1394_new_handle();
  if (raw_handle == NULL) {
    perror("Unable to aquire a raw1394 handle\n");
    perror("did you load the drivers?\n");
    exit(-1);
  }

  numPorts = raw1394_get_port_info(raw_handle, ports, numPorts);
  raw1394_destroy_handle(raw_handle);
  if (verbose) printf("number of ports = %d\n", numPorts);

  /* get dc1394 handle to each port */
  for (p = 0; p < numPorts; p++) {
    int                       camCount;

    handles[p] = dc1394_create_handle(p);
    if (handles[p] == NULL) {
      perror("Unable to aquire a raw1394 handle\n");
      perror("did you load the drivers?\n");
      cleanup();
      exit(-1);
    }

    /* get the camera nodes and describe them as we find them */
    camera_nodes = dc1394_get_camera_nodes(handles[p], &camCount, verbose);

    /* setup cameras for capture */
    for (i = 0; i < camCount; i++) {
      cameras[numCameras].node = camera_nodes[i];

      if (dc1394_get_camera_feature_set
          (handles[p], cameras[numCameras].node,
           &features) != DC1394_SUCCESS) {
        printf("unable to get feature set\n");
      } else if (verbose) {
        dc1394_print_feature_set(&features);
      }

      if (dc1394_get_iso_channel_and_speed
          (handles[p], cameras[numCameras].node, &channel,
           &speed) != DC1394_SUCCESS) {
        printf("unable to get the iso channel number\n");
        cleanup();
        exit(-1);
      }

      if (dc1394_dma_setup_capture
          (handles[p], cameras[numCameras].node, i + 1 /*channel */ ,
           FORMAT_VGA_NONCOMPRESSED, res, SPEED_400, fps, NUM_BUFFERS,
           DROP_FRAMES, device_name,
           &cameras[numCameras]) != DC1394_SUCCESS) {
        fprintf(stderr,
                "unable to setup camera - check line %d of %s to make sure\n",
                __LINE__, __FILE__);
        perror("that the video mode, framerate and format are supported\n");
        printf("by your camera(s)\n");
        cleanup();
        exit(-1);
      }


      /*have the camera start sending us data */
      if (dc1394_start_iso_transmission
          (handles[p], cameras[numCameras].node) != DC1394_SUCCESS) {
        perror("unable to start camera iso transmission\n");
        cleanup();
        exit(-1);
      }
      numCameras++;
    }
  }

  fflush(stdout);
  if (numCameras < 1) {
    perror("no cameras found :(\n");
    cleanup();
    exit(-1);
  }


  //set_manual_exposure_gain(0, 440, 30);

  switch (format) {
  case XV_YV12:
    set_frame_length(device_width * device_height * 3 / 2, numCameras);
    break;
  case XV_YUY2:
  case XV_UYVY:
    set_frame_length(device_width * device_height * 2, numCameras);
    break;
  default:
    fprintf(stderr, "Unknown format set (internal error)\n");
    exit(255);
  }

  /* create OpenCV image wrappers */
  for (cn = 0; cn < MAX_CAMERAS; cn++) {
    if (cn < numCameras) {
      iplImages[cn] =
        cvCreateImage(cvSize(device_width, device_height),
		      IPL_DEPTH_8U, 3);
      readOnlyImg =
        cvCreateImageHeader(cvSize(device_width, device_height),
                            IPL_DEPTH_8U, 3);
    } else {
      iplImages[cn] = NULL;
    }
  }

  /* initialize handvu */
  hvInitialize(device_width, device_height);
  hvLoadConductor(string(conductor_fname));
  hvStartRecognition();
  hvSetOverlayLevel(3);
  if (async_processing) {
    hvAsyncSetup(num_async_bufs, displayCallback);
    hvAsyncGetImageBuffer(&m_async_image, &m_async_bufID);
    if (sync_display) async_display_image = cvCloneImage(iplImages[0]);
  }  

  /* make the window */
  display = XOpenDisplay(getenv("DISPLAY"));
  if (display == NULL) {
    fprintf(stderr, "Could not open display \"%s\"\n", getenv("DISPLAY"));
    cleanup();
    exit(-1);
  }

  QueryXv();

  if (adaptor < 0) {
    cleanup();
    exit(-1);
  }

  width = device_width;
  height = device_height * numCameras;

  window =
    XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0,
                        width, height, 0, WhitePixel(display,
                                                     DefaultScreen
                                                     (display)), background);

  XSelectInput(display, window, StructureNotifyMask | KeyPressMask);
  XMapWindow(display, window);
  connection = ConnectionNumber(display);

  gc = XCreateGC(display, window, 0, &xgcv);

  /* local main event loop */
  while (1) {

    if (async_processing) {
      // asynchronous processing in HandVu

      capture_frame();
      process_frame();
      if (sync_display) display_frame();
      handle_events();
      
    } else {
      // synchronous processing in HandVu
      capture_frame();
      process_frame();
      display_frame();
      handle_events();
    }

    /* XPending */

  }                             /* while not interrupted */

  exit(0);
}
