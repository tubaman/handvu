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
  * $Id: HandVu.cpp,v 1.23 2004/10/10 02:12:38 matz Exp $
**/

#ifndef FRAMEDATALIB_H
#define FRAMEDATALIB_H

#ifdef __cplusplus
#undef EXPORT
#define EXPORT extern "C" __declspec (dllexport)
#else
#define EXPORT __declspec (dllexport)
#endif

#define byte unsigned char

//
// Initialize the dll to the correct size and color channels
//

EXPORT void FDL_Initialize(int width, int height, int channels);

//
// Wait for the dll to be initialized
//

EXPORT void FDL_WaitForInitialization();

//
// Get the dimensions of the image
//
 
EXPORT void FDL_GetDimensions(int* pwidth, int* pheight, int* pchannels); 

//
// Save image in the dll 
//

EXPORT void FDL_PutImage(const byte* img); 

//
// Get a pointer to dll-internal data structure
//

EXPORT void FDL_GetImage(byte** img);

#endif