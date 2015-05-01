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
  * $Id: Exceptions.cpp,v 1.13 2004/11/11 01:58:58 matz Exp $
**/

// Exceptions.cpp : implementation file for all C++ exceptions
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


#include "cubicles.hpp"
#include "Exceptions.h"
#include <ostream>

ITException::ITException(string msg)
{
  m_msg = msg;
}

ostream& ITException::output(ostream& os) const
{
  return os << "ITException: " << m_msg;
}

ostream& operator<<(ostream& os, const ITException& ite) {
  return ite.output(os);
}




ITETraining::ITETraining(string msg)
  : ITException(msg)
{
}



ITETrainedPerfectWeak::ITETrainedPerfectWeak()
  : ITETraining("perfect weak classifier")
{
}




ITETrainBadSeparation::ITETrainBadSeparation()
  : ITETraining("weak classifier separates training set poorly or not at all")
{
}



ITETrainingNoMoreNegs::ITETrainingNoMoreNegs()
  : ITETraining("can not find enough negative examples to reach the desired false positive rate")
{
}


ITEFile::ITEFile(string filename, string msg)
  : ITException("error with file "+filename+":\n"+msg),
    m_filename(filename)
{
}



ITEFileNotFound::ITEFileNotFound(string filename)
  : ITEFile(filename, "could not find or open file")
{
}




