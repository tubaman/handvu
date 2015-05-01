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
  * $Id: Exceptions.h,v 1.14 2004/09/24 22:26:53 matz Exp $
**/

// Exceptions.h: header file for all C++ exceptions
// that can occur during cubicle use.
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

#ifndef INTEGRATION_TEMPLATES_EXCEPTIONS_H
#define INTEGRATION_TEMPLATES_EXCEPTIONS_H

#include <string>


#ifdef USE_MFC
//using namespace std;	
// this prompts a bug in VC++ (friends and private member access)
// thus we have to declare all usages individually:
using std::string;

#else // USE_MFC

using namespace std;
#endif // USE_MFC

// Visual C++ doesn't understand exception specifications of functions:
#if defined(_MSC_VER) && _MSC_VER<1400
#define throw(x) /* throw(x) */
#endif

#if defined(WIN32) && defined(GetMessage)
#undef GetMessage
#endif


class ITException {
public:
  ITException(string msg);
  const string GetMessage() { return m_msg; }
  virtual ostream& output(ostream& os) const;
  
  friend ostream& operator<<(ostream& os, const ITException& ite);

 protected:
  string m_msg;
};


class ITETraining : public ITException {
 public:
  ITETraining(string msg);
};

class ITETrainedPerfectWeak : public ITETraining {
 public:
  ITETrainedPerfectWeak();  
};

class ITETrainBadSeparation : public ITETraining {
 public:
  ITETrainBadSeparation();  
};

class ITETrainingNoMoreNegs : public ITETraining {
 public:
  ITETrainingNoMoreNegs();
};


class ITEFile : public ITException {
 public:
  ITEFile(string filename, string msg);
 protected:
  string m_filename;
};

class ITEFileNotFound : public ITEFile {
 public:
  ITEFileNotFound(string filename);
};



#endif // INTEGRATION_TEMPLATES_EXCEPTIONS_H
