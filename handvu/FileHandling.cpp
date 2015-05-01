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
  * $Id: FileHandling.cpp,v 1.6 2005/08/16 00:14:26 matz Exp $
**/

#include "Common.h"
#include "FileHandling.h"
#include "Exceptions.h"
#include <errno.h>

#ifdef USE_MFC
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif // _DEBUG
#endif // USE_MFC




#ifdef WIN32
#include <direct.h>
#define getcwd _getcwd
#define chdir _chdir
#else
#include <unistd.h>
char* _fullpath(char* absPath, const char* relPath, size_t maxLength);
void _splitpath(const char* path, char* drive, char* dir, char* fname, char* ext);
#define _MAX_PATH 260
#define _MAX_FNAME 256
#define _MAX_EXT 256
#define _MAX_DRIVE 3
#define _MAX_DIR 256
#endif


/** return current working directory
*/
string GetCWD()
{
  char old_cwd[_MAX_PATH];
  if (getcwd(old_cwd, _MAX_PATH)==NULL) {
    if (errno == ERANGE) {
      throw HVException("getcwd needs larger buffer");
    }
    throw HVException("getcwd error");
  }
  return string(old_cwd);
}

/** set current working directory
*/
void SetCWD(const string& new_cwd)
{
  int error = chdir(new_cwd.c_str());
  if (error) {
    string err_str = "chdir error: can not find path: ";
    err_str = err_str + new_cwd;
    throw HVException(err_str);
  }
}

/** first extend pathfile to the full, resolved, path; then
* devide into drive+path and filename+extension
*/
void SplitPathFile(const string& pathfile, string& vc_path, string& filename)
{
  char fullpath[_MAX_PATH];
  if (_fullpath(fullpath, pathfile.c_str(), _MAX_PATH)==NULL) {
    throw HVException("fullpath error");
  }
  char fname[_MAX_FNAME];
  char ext[_MAX_EXT];
  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
  _splitpath(fullpath, drive, dir, fname, ext);
  size_t len = strlen(dir);
  while (len>=1 && dir[len-1]=='/' || dir[len-1]=='\\') {
    dir[len-1] = 0;
    len = strlen(dir);
  }
  vc_path = string(drive)+string(dir);
  filename = string(fname)+string(ext);
}



#if !defined(WIN32)
char* _fullpath(char* absPath, const char* relPath, size_t maxLength)
{
  if (getcwd(absPath, maxLength)==NULL) {
    if (maxLength>0) absPath[0] = 0;
    return NULL;
  }
  int pos = strlen(absPath);
  if (pos+strlen(relPath)+2<maxLength) {
    sprintf(&absPath[pos], "/%s", relPath);
    // todo: should use snprintf (WIN32: _snprintf)
  }
  char* up = strstr(absPath, "/..");
  while (up) {
    char* slashpos = up;
    while (*slashpos=='/' && slashpos>absPath) slashpos--;
    while (*slashpos!='/' && slashpos>absPath) slashpos--;
    strcpy(slashpos, up+3);
    up = strstr(absPath, "/..");
  }
  //  printf("fullpath: %s\n", absPath);
  return absPath;
}

void _splitpath(const char* path, char* drive, char* dir, char* fname, char* ext)
{
  drive[0] = 0;
  dir[0] = 0;
  fname[0] = 0;
  ext[0] = 0;
  
  char* slashpos = strrchr(path, '/');
  char* dotpos = strrchr(path, '.');
  
  if (slashpos==NULL) {
    if (dotpos==NULL) {
      strcpy(fname, path);
    } else {
      strncpy(fname, path, dotpos-path);
      fname[dotpos-path] = 0;
      strcpy(ext, dotpos);
    }

  } else {
    if (dotpos!=NULL) {
      strncpy(fname, slashpos+1, dotpos-slashpos-1);
      fname[dotpos-slashpos-1] = 0;
      strcpy(ext, dotpos);
    } else {
      strncpy(fname, slashpos+1, strlen(path)-strlen(slashpos)-1);
      fname[strlen(path)-strlen(slashpos)-1] = 0;
    }
    strncpy(dir, path, slashpos-path);
    dir[slashpos-path] = 0;
  }

  /*
  printf("path: %s, %d\n", path, strlen(path));
  printf("drive: %s, %d\n", drive, strlen(drive));
  printf("dir: %s, %d\n", dir, strlen(dir));
  printf("fname: %s, %d\n", fname, strlen(fname));
  printf("ext: %s, %d\n", ext, strlen(ext));
  */
}
#endif // WIN32

