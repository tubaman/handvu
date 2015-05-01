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
  * $Id: DllTester.cpp,v 1.4 2005/03/31 02:09:43 matz Exp $
**/

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <stdlib.h>

#define CV_DLL       "cv096.dll"
#define CV_CXCORE    "cxcore096.dll"
#define CV_AUX       "cvaux096.dll"
#define CV_HIGHGUI   "highgui096.dll"
#define MAGICK       "CORE_RL_Magick++_.dll"
#define FDL          "FrameDataLib.dll"

// windows libraries
#define WINMM_DLL    "winmm.dll"
#define MFC_DLL      "mfc70.dll"
#define MSVCR_DLL    "msvcr70.dll"
#define KERNEL32_DLL "kernel32.dll"
#define USER32_DLL   "user32.dll"
#define ADVAPI32_DLL "advapi32.dll"
#define OLE32_DLL    "ole32.dll"
#define OLEAUT32_DLL "oleaut32.dll"
#define MSVCP_DLL    "msvcp70.dll"


BOOL test(LPCTSTR lpLibFileName, BOOL silent);


int _tmain(int argc, _TCHAR* argv[])
{
  BOOL silent = FALSE;
  if (argc>1 && strcmp(argv[1], "/s")==0) {
    silent = TRUE;
  }
  if (!silent) printf("Checking for libraries:\n");
  
  BOOL all_found = TRUE;

  if (!test(CV_DLL, silent)) all_found = FALSE;
  if (!test(CV_CXCORE, silent)) all_found = FALSE;
  if (!test(CV_AUX, silent)) all_found = FALSE;
  if (!test(CV_HIGHGUI, silent)) all_found = FALSE;
//  if (!test(FDL, silent)) all_found = FALSE;

  if (!test(WINMM_DLL, silent)) all_found = FALSE;
  if (!test(MFC_DLL, silent)) all_found = FALSE;
  if (!test(MSVCR_DLL, silent)) all_found = FALSE;
  if (!test(KERNEL32_DLL, silent)) all_found = FALSE;
  if (!test(USER32_DLL, silent)) all_found = FALSE;
  if (!test(ADVAPI32_DLL, silent)) all_found = FALSE;
  if (!test(OLE32_DLL, silent)) all_found = FALSE;
  if (!test(OLEAUT32_DLL, silent)) all_found = FALSE;
  if (!test(MSVCP_DLL, silent)) all_found = FALSE;

  if (all_found) {
    if (!silent) printf("All libraries found.\n");
  	return 0;

  } else {
    const char *path = getenv("PATH");
    if (!silent) printf("At least one library was not found. Make sure the missing file\n"
      "is in your system's search path:\n"
      "$(PATH) = %s\n", path);
    return -1;
  }
}


BOOL test(LPCTSTR lpLibFileName, BOOL silent)
{
  HMODULE hModule;
  BOOL unloaded;

  if (!silent) printf("%s ... ", lpLibFileName);
  hModule = LoadLibrary(lpLibFileName);
  if (hModule==NULL) {
    if (!silent) printf("FAILED\n");
    return FALSE;
  } else {
    if (!silent) printf("found\n");
  }
  unloaded = FreeLibrary(hModule);
  if (!unloaded) {
    if (!silent) printf("could not unload library\n");
    return FALSE;
  }
  return TRUE;
}


