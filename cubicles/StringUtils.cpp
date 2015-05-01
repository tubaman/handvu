/**
  * cubicles
  *
  * This is an implementation of the Viola-Jones object detection 
  * method and some extensions.  The code is mostly platform-
  * independent and uses only standard C and C++ libraries.  It
  * can make use of MPI for parallel training and a few Windows
  * MFC functions for classifier display.
  *
  * Mathias Kolsch, matz@cs.ucsb.edu
  *
  * $Id: StringUtils.cpp,v 1.7 2004/11/11 01:58:58 matz Exp $
**/

// StringUtils.cpp implements <string> convenience functions and
// Unix/Windows path conversions.
//

////////////////////////////////////////////////////////////////////
//
// By downloading, copying, installing or using the software you 
// agree to this license.  If you do not agree to this license, 
// do not download, install, copy or use the software.
//
// Copyright (C) 2004, Mathias Kolsch, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in binary form, with or without 
// modification, is permitted for non-commercial purposes only.
// Redistribution in source, with or without modification, is 
// prohibited without prior written permission.
// If granted in writing in another document, personal use and 
// modification are permitted provided that the following two
// conditions are met:
//
// 1.Any modification of source code must retain the above 
//   copyright notice, this list of conditions and the following 
//   disclaimer.
//
// 2.Redistribution's in binary form must reproduce the above 
//   copyright notice, this list of conditions and the following 
//   disclaimer in the documentation and/or other materials provided
//   with the distribution.
//
// This software is provided by the copyright holders and 
// contributors "as is" and any express or implied warranties, 
// including, but not limited to, the implied warranties of 
// merchantability and fitness for a particular purpose are 
// disclaimed.  In no event shall the copyright holder or 
// contributors be liable for any direct, indirect, incidental, 
// special, exemplary, or consequential damages (including, but not 
// limited to, procurement of substitute goods or services; loss of 
// use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict 
// liability, or tort (including negligence or otherwise) arising 
// in any way out of the use of this software, even if advised of 
// the possibility of such damage.
//
////////////////////////////////////////////////////////////////////

#include "cubicles.hpp"
#include <string>
#include <istream>
using namespace std;

/////////////////////////////////////////////////////////////////////////////
// Helper functions

/** read a line from the input stream. then make sure that trailing
 * carriage-returns are removed from the string.
 */
void getline_crlf(istream& is, string& line)
{
  getline(is, line);
  if (line[line.length()-1]=='\r') {
    line.erase(line.length()-1);
  }
}

// substitute "what" with "with"
void ReplaceAll(string& mangle, const string what, const string with)
{
  size_t pos = mangle.find(what);
  while (pos!=string::npos) {
    mangle.replace(pos, what.size(), with);
    pos = mangle.find(what);
  }
}

// removes all carriage returns from string; CR == \r
void RemoveCR(string& str)
{
 size_t pos = str.find('\r');
  while (pos!=string::npos) {
    str.erase(pos);
    pos = str.find('\r');
  }
}

#ifdef WIN32
/* replace / with \
*/
string ConvertPathToWindows(const string& filename)
{
  // substitute \ for /
  string win_filename(filename);
  ReplaceAll(win_filename, "/", "\\");
  return win_filename;
}

/* replace \ with /
*/
string ConvertPathToStandard(const string& win_filename)
{
  // substitute / for \ 
  string std_filename(win_filename);
  ReplaceAll(std_filename, "\\", "/");
  return std_filename;
}

#endif // WIN32

