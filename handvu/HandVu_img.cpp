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
  * $Id: HandVu_img.cpp,v 1.16 2005/10/30 21:16:04 matz Exp $
**/

#include "Common.h"
#include "CubicleWrapper.h"
#include "HandVu.hpp"
#include "FileHandling.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <highgui.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
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




/////////////////////////////////////////////////////////////////////////////
// HandVu

void HandVu::SaveScannedArea(IplImage* pImg, string& picfile)
{
  SaveImageArea(pImg, m_scan_area, picfile);
}

void HandVu::SaveImageArea(IplImage* pImg, CRect area, string& picfile)
{
  string extension = ".ppm";
  if (picfile=="") {
    if (m_img_fname_root=="") {
      throw HVException("must SetSaveFilenameRoot if no filename given");
    }
    picfile = GetNextSnapshotFilename(m_img_fname_root, extension);
  }
  if (area.left>=area.right) { int t=area.left; area.left=area.right; area.right=t; }
  if (area.top>=area.bottom) { int t=area.top; area.top=area.bottom; area.bottom=t; }
  if (area.left<0) area.left = 0;
  if (area.right>pImg->width) area.right = pImg->width;;
  if (area.top<0) area.top = 0;;
  if (area.bottom>pImg->height) area.bottom = pImg->height;

  if (strcmp(pImg->channelSeq, "BGR")==0) {
    WriteAreaAsPPM_BGR(pImg, area, picfile);
  } else if (strcmp(pImg->channelSeq, "RGB")==0) {
    WriteAreaAsPPM_RGB(pImg, area, picfile);
  } else if (strcmp(pImg->channelSeq, "GRAY")==0) {
    WriteAreaAsPGM_Gray(pImg, area, picfile);
  } else {
    throw HVException("can not write this image format");
  }
}

void HandVu::WriteAreaAsBMP(IplImage* pImg, const CRect& area, const string& filename)
{
  IplImage* areaImg = 
    cvCreateImage(cvSize(area.right-area.left, area.bottom-area.top),
                  pImg->depth, pImg->nChannels);
  cvResize(pImg, areaImg);
  int success = cvSaveImage(filename.c_str(), areaImg);
  if (!success) {
    throw HVException(string("failure saving image to ") + filename);
  }
}


// write binary color unsigned char PPM format (the most standard)
void HandVu::WriteAreaAsPPM_BGR(IplImage* pImg, const CRect& area, const string& picfile)
{
  ASSERT(strcmp(pImg->channelSeq, "BGR")==0);

  FILE *fp = fopen(picfile.c_str(), "wb");  // open for writing in binary format

  if (!fp) {
    throw HVEFile(picfile, "cannot open file for writing");
  }

  // allocate buffer
  int width = area.right-area.left;
  size_t total_size = width*(area.bottom-area.top)*3;
  unsigned char* buf = (unsigned char*) alloca(total_size);
  ASSERT(pImg->nChannels==3);

  // fill buffer with image data
  unsigned char* pPix = buf;
  for (int y=area.top; y<area.bottom; y++) {
    ColorBGR* pSrc = (ColorBGR*) pImg->imageData;
    pSrc += y*pImg->width + area.left;
    for (int x=area.left; x<area.right; x++) {
      *pPix++ = (unsigned char) pSrc->red;
      *pPix++ = (unsigned char) pSrc->green;
      *pPix++ = (unsigned char) pSrc->blue;
      pSrc++;
    }
  }

  /*
  CvPixelPosition8u position;
  CV_INIT_PIXEL_POS(position, 
                    (unsigned char*)(pImg->imageData),
                    pImg->widthStep, 
                    cvSize(pImg->width, pImg->height), 0, 0,
                    pImg->origin);
  for (int y=top; y<bottom; y++) {
    CV_MOVE_TO(position, left, y, 3);
    for (int x=left; x<right; x++) {
      unsigned char* curr = CV_GET_CURRENT(position, 3);
      *pPix++ = curr[1];
      *pPix++ = curr[0];
      *pPix++ = curr[2];
      CV_MOVE_RIGHT(position, 3);
    }
  }
  */

  // write to file
  // header
  fprintf(fp, "P6\n%d %d\n%d\n", width, area.bottom-area.top, 255);  // binary
  // body
  size_t cnt = fwrite((void *) buf, 1, total_size, fp);
  fclose(fp);

  if ((int)cnt!=total_size) {
    throw HVEFile(picfile, "cannot write image data to file");
  }
}

// write binary color unsigned char PPM format (the most standard)
void HandVu::WriteAreaAsPPM_RGB(IplImage* pImg, const CRect& area, const string& picfile)
{
  ASSERT(strcmp(pImg->channelSeq, "BGR")==0);

  FILE *fp = fopen(picfile.c_str(), "wb");  // open for writing in binary format

  if (!fp) {
    throw HVEFile(picfile, "cannot open file for writing");
  }

  // allocate buffer
  int width = area.right-area.left;
  size_t total_size = width*(area.bottom-area.top)*3;
  unsigned char* buf = (unsigned char*) alloca(total_size);
  ASSERT(pImg->nChannels==3);

  // fill buffer with image data
  unsigned char* pPix = buf;
  for (int y=area.top; y<area.bottom; y++) {
    ColorBGR* pSrc = (ColorBGR*) pImg->imageData;
    pSrc += y*pImg->width + area.left;
    for (int x=area.left; x<area.right; x++) {
      *pPix++ = (unsigned char) pSrc->blue;
      *pPix++ = (unsigned char) pSrc->green;
      *pPix++ = (unsigned char) pSrc->red;
      pSrc++;
    }
  }

  /*
  CvPixelPosition8u position;
  CV_INIT_PIXEL_POS(position, 
                    (unsigned char*)(pImg->imageData),
                    pImg->widthStep, 
                    cvSize(pImg->width, pImg->height), 0, 0,
                    pImg->origin);
  for (int y=top; y<bottom; y++) {
    CV_MOVE_TO(position, left, y, 3);
    for (int x=left; x<right; x++) {
      unsigned char* curr = CV_GET_CURRENT(position, 3);
      *pPix++ = curr[1];
      *pPix++ = curr[0];
      *pPix++ = curr[2];
      CV_MOVE_RIGHT(position, 3);
    }
  }
  */

  // write to file
  // header
  fprintf(fp, "P6\n%d %d\n%d\n", width, area.bottom-area.top, 255);  // binary
  // body
  size_t cnt = fwrite((void *) buf, 1, total_size, fp);
  fclose(fp);

  if ((int)cnt!=total_size) {
    throw HVEFile(picfile, "cannot write image data to file");
  }
}

// write binary gray unsigned char PGM format
void HandVu::WriteAreaAsPGM_Gray(IplImage* pImg, const CRect& area, const string& picfile)
{
  ASSERT(strcmp(pImg->channelSeq, "GRAY")==0);
  ASSERT(pImg->depth==IPL_DEPTH_32F);

  FILE *fp = fopen(picfile.c_str(), "wb");  // open for writing in binary format

  if (!fp) {
    throw HVEFile(picfile, "cannot open file for writing");
  }

  // allocate buffer
  int width = area.right-area.left;
  size_t total_size = width*(area.bottom-area.top);
  unsigned char* buf = (unsigned char*) alloca(total_size);
  ASSERT(pImg->nChannels==1);

  // get min and max
  float min=FLT_MAX, max=FLT_MIN;
  unsigned char* pPix = buf;
  for (int y=area.top; y<area.bottom; y++) {
    float* pSrc = (float*) pImg->imageData;
    pSrc += y*pImg->width + area.left;
    for (int x=area.left; x<area.right; x++) {
      if (*pSrc<min) {
        min = *pSrc;
      }
      if (*pSrc>max) {
        max = *pSrc;
      }
      pSrc++;
    }  }
  float spread = max-min;

  // fill buffer with image data
  for (int y=area.top; y<area.bottom; y++) {
    float* pSrc = (float*) pImg->imageData;
//    unsigned char* pSrc = (unsigned char*) pImg->imageData;
    pSrc += y*pImg->width + area.left;
    for (int x=area.left; x<area.right; x++) {
      *pPix++ = (unsigned char) (255.0 * (*pSrc-min)/spread);      pSrc++;
    }  }

  // write to file
  // header
  fprintf(fp, "P5\n%d %d\n%d\n", width, area.bottom-area.top, 255);  // binary
  // body
  size_t cnt = fwrite((void *) buf, 1, total_size, fp);
  fclose(fp);

  if ((int)cnt!=total_size) {
    throw HVEFile(picfile, "cannot write image data to file");
  }
}

string HandVu::GetNextSnapshotFilename(const string& base,
                                       const string& extension)
{
  static int g_last_archive_id=0;
  ASSERT(base!="");
  string next_name;
  for (;;) {
    g_last_archive_id++;
    char id[10];
    sprintf(id, "%d", g_last_archive_id);
    // todo: should use snprintf (WIN32: _snprintf)
    next_name = base + id + extension;
    FILE* test = fopen(next_name.c_str(), "r");
    if (test) {
      fclose(test);
    } else {
      break;
    }
  }
  
  return next_name;
}

#if defined(STAT_MACROS_BROKEN) || !defined(HAVE_CONFIG_H) && defined(WIN32)
#define S_ISDIR(mode) (mode&_S_IFDIR)
#endif

void HandVu::SetSaveFilenameRoot(const string& fname_root) 
{
  struct stat s;
  int error = stat(fname_root.c_str(), &s);
#ifdef HAVE_STAT_EMPTY_STRING_BUG
  if (fname_root.length()==0) {
    error = -1;
    errno = ENOENT;
  }
#endif

  if (error==-1 && errno==ENOENT) {
    // file does not exist
    // maybe fname_root is a combination of a 
    // path and a partial file name?
    string path, fname;
    SplitPathFile(fname_root, path, fname);
    error = stat(path.c_str(), &s);
#ifdef HAVE_STAT_EMPTY_STRING_BUG
    if (path.length()==0) {
      error = -1;
      errno = ENOENT;
    }
#endif

    if (error!=0 && errno==ENOENT || !S_ISDIR(s.st_mode)) {
      throw HVException(string("neither directory ") + fname_root +
		        " nor directory " + path + " exist, or they are not accessible");
    }

  } else if (error!=0 || !S_ISDIR(s.st_mode)) {
    throw HVException(string("directory ") + fname_root +
		    " does not exist");
  }
  m_img_fname_root = fname_root;
}
