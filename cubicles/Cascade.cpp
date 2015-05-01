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
  * $Id: Cascade.cpp,v 1.46 2004/11/11 01:58:57 matz Exp $
**/

// Cascade.cpp : implementation file for ClassifierCascades.
// Functions for reading from files are flex/bison generated from
// CascadeFileScanner.lex and CascadeFileParser.yy.
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
#include "Cascade.h"
#include "CascadeFileParser.h"

#ifdef _DEBUG
#ifdef USE_MFC
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif // USE_MFC
#endif // _DEBUG

#include <fstream>
#include <math.h>
using std::ofstream;


// for parsing
extern CClassifierCascade* parse_cascade(FILE* fp);



/////////////////////////////////////////////////////////////////////////////
//
// 	CClassifierCascade implementation
//
/////////////////////////////////////////////////////////////////////////////

CClassifierCascade::CClassifierCascade()
  :	
  m_name("unnamed"),
  m_structure_type(CASCADE_TYPE_SEQUENTIAL),
  m_total_false_positive_rate(1.0),
  m_last_detection_rate(0.0),
  m_trainset_exhausted(false),
  m_template_width(-1),
  m_template_height(-1),
  m_image_area_ratio(-1)
{
  m_classifiers.reserve(20);
  m_lyr_false_positive_rates.reserve(20);
  m_lyr_detection_rates.reserve(20);
}

CClassifierCascade::CClassifierCascade(const CClassifierCascade& frm)
  :	
  m_name(frm.m_name),
  m_structure_type(frm.m_structure_type),
  m_total_false_positive_rate(frm.m_total_false_positive_rate),
  m_last_detection_rate(frm.m_last_detection_rate),
  m_trainset_exhausted(frm.m_trainset_exhausted),
  m_template_width(frm.m_template_width),
  m_template_height(frm.m_template_height),
  m_image_area_ratio(frm.m_image_area_ratio),
  m_classifiers(frm.m_classifiers),
  m_branch_classifiers(frm.m_branch_classifiers),
  m_branch_names(frm.m_branch_names),
  m_lyr_false_positive_rates(frm.m_lyr_false_positive_rates),
  m_lyr_detection_rates(frm.m_lyr_detection_rates),
  m_branch_lyr_false_positive_rates(frm.m_branch_lyr_false_positive_rates),
  m_branch_lyr_detection_rates(frm.m_branch_lyr_detection_rates),
  m_branch_false_positive_rates(frm.m_branch_false_positive_rates),
  m_branch_detection_rates(frm.m_branch_detection_rates)
{
}

CClassifierCascade::CClassifierCascade(int template_width, int template_height, double image_area_ratio)
  :
  m_name("unnamed"),
  m_structure_type(CASCADE_TYPE_SEQUENTIAL),
  m_total_false_positive_rate(1.0),
  m_last_detection_rate(0.0),
  m_trainset_exhausted(false),
  m_template_width(template_width),
  m_template_height(template_height),
  m_image_area_ratio(image_area_ratio)
{
  m_classifiers.reserve(20);
  m_lyr_false_positive_rates.reserve(20);
  m_lyr_detection_rates.reserve(20);
}

CClassifierCascade::~CClassifierCascade()
{
}

CClassifierCascade& CClassifierCascade::operator=(const CClassifierCascade& frm)
{
  m_name = frm.m_name;
  m_structure_type = frm.m_structure_type;
  m_total_false_positive_rate = frm.m_total_false_positive_rate;
  m_last_detection_rate = frm.m_last_detection_rate;
  m_trainset_exhausted = frm.m_trainset_exhausted;
  m_template_width = frm.m_template_width;
  m_template_height = frm.m_template_height;
  m_image_area_ratio = frm.m_image_area_ratio;
  m_classifiers = frm.m_classifiers;
  m_branch_classifiers = frm.m_branch_classifiers;
  m_branch_names = frm.m_branch_names;
  m_lyr_false_positive_rates = frm.m_lyr_false_positive_rates;
  m_lyr_detection_rates = frm.m_lyr_detection_rates;
  m_branch_lyr_false_positive_rates = frm.m_branch_lyr_false_positive_rates;
  m_branch_lyr_detection_rates = frm.m_branch_lyr_detection_rates;
  m_branch_false_positive_rates = frm.m_branch_false_positive_rates;
  m_branch_detection_rates = frm.m_branch_detection_rates;

  return *this;
}

/* add one strong classifier as the last stage of the cascade
 */
CStrongClassifier& CClassifierCascade::AddStrongClassifier()
{
  ASSERT(m_structure_type==CASCADE_TYPE_SEQUENTIAL);
  int num_clsfs = (int)m_classifiers.size()+1;
  m_classifiers.resize(num_clsfs);
  m_lyr_false_positive_rates.resize(num_clsfs);
  m_lyr_detection_rates.resize(num_clsfs);
  m_classifiers[num_clsfs-1] = CStrongClassifier(); // make sure it's an empty strong classifier
  m_lyr_false_positive_rates[num_clsfs-1] = 1.0;
  m_lyr_detection_rates[num_clsfs-1] = 1.0;
  return m_classifiers[num_clsfs-1];
}


/* add one strong classifier as the last stage of the specified branch
 */
CStrongClassifier& CClassifierCascade::AddStrongClassifier(int branch, double fpr, double dr)
{
  if (m_structure_type==CASCADE_TYPE_SEQUENTIAL || branch==-1) {
    if (branch!=-1) {
      throw ITException("no branches other than common branch (-1) available for sequential cascade structure");
    }
    int num_clsfs = (int)m_classifiers.size()+1;
    m_classifiers.resize(num_clsfs);
    m_lyr_false_positive_rates.resize(num_clsfs);
    m_lyr_detection_rates.resize(num_clsfs);

    m_classifiers[num_clsfs-1] = CStrongClassifier(); // make sure it's an empty strong classifier
    m_lyr_false_positive_rates[num_clsfs-1] = fpr;
    m_lyr_detection_rates[num_clsfs-1] = dr;
    return m_classifiers[num_clsfs-1];

  } else if (m_structure_type==CASCADE_TYPE_FAN) {
    if (branch>=(int)m_branch_classifiers.size()) {
      throw ITException("branch number out of range");
    }
    ASSERT(branch<(int)m_branch_lyr_false_positive_rates.size());
    ASSERT(branch<(int)m_branch_lyr_detection_rates.size());

    int num_clsfs = (int)m_branch_classifiers[branch].size()+1;
    m_branch_classifiers[branch].resize(num_clsfs);
    m_branch_lyr_false_positive_rates[branch].resize(num_clsfs);
    m_branch_lyr_detection_rates[branch].resize(num_clsfs);
    m_branch_classifiers[branch][num_clsfs-1] = CStrongClassifier(); // make sure it's an empty strong classifier
    m_branch_lyr_false_positive_rates[branch][num_clsfs-1] = fpr;
    m_branch_lyr_detection_rates[branch][num_clsfs-1] = dr;
    return m_branch_classifiers[branch][num_clsfs-1];

  } else {
    ASSERT(0);
    throw ITException("tree type not implemented yet");
  }
}


int CClassifierCascade::RemoveLastStrongClassifier()
{
  ASSERT(m_structure_type==CASCADE_TYPE_SEQUENTIAL);
  ASSERT((int)m_classifiers.size());
  m_classifiers.pop_back();
  return (int)m_classifiers.size();
}

// returns number of remaining weak classifiers in last strong
// classifier
int CClassifierCascade::RemoveLastWeakClassifier()
{
  ASSERT(m_structure_type==CASCADE_TYPE_SEQUENTIAL);
  int num_clsf = (int)m_classifiers.size();
  ASSERT(num_clsf);
  return m_classifiers[num_clsf-1].RemoveLastWeakClassifier();
}

double CClassifierCascade::GetFalsePositiveRate(int clsf) const { 
  ASSERT(m_structure_type==CASCADE_TYPE_SEQUENTIAL);
  ASSERT(0<=clsf && clsf<(int)m_classifiers.size());
  return m_lyr_false_positive_rates[clsf]; 
}

double CClassifierCascade::GetDetectionRate(int clsf) const {
  ASSERT(m_structure_type==CASCADE_TYPE_SEQUENTIAL);
  ASSERT(0<=clsf && clsf<(int)m_classifiers.size());
  return m_lyr_detection_rates[clsf];
}

void CClassifierCascade::SetFalsePositiveRate(int clsf, double fpr) { 
  ASSERT(m_structure_type==CASCADE_TYPE_SEQUENTIAL);
  ASSERT(0<=clsf && clsf<(int)m_classifiers.size());
  m_lyr_false_positive_rates[clsf] = fpr;
  m_total_false_positive_rate = 1.0;
  for (int c=0; c<(int)m_classifiers.size(); c++) {
    m_total_false_positive_rate *= m_lyr_false_positive_rates[c];
  }
}

void CClassifierCascade::SetDetectionRate(int clsf, double dr) {
  ASSERT(m_structure_type==CASCADE_TYPE_SEQUENTIAL);
  ASSERT(0<=clsf && clsf<(int)m_classifiers.size());
  m_lyr_detection_rates[clsf] = dr;
  if (clsf==(int)m_classifiers.size()-1) {
    m_last_detection_rate = dr;
  }
}

void CClassifierCascade::ParseFrom(const string& filename)
{
  FILE* fp = fopen(filename.c_str(), "r");
  if (!fp) {
    throw ITEFileNotFound(filename);
  }

  char* buf = new char[10000];
  setvbuf(fp, buf, _IOFBF, sizeof(buf));
  CClassifierCascade* pCascade = parse_cascade(fp);
  delete buf;
  
  if (pCascade==NULL) {
    throw ITException("parsing exception");
  }
  *this = *pCascade;
  delete pCascade;

  return;
}

ostream& operator<<(ostream& os, const CClassifierCascade& casc)
{
  return casc.output(os);
}

ostream& CClassifierCascade::output(ostream& os) const {
  os << "ClassifierCascade \"" << m_name << "\", ";
  if (m_structure_type==CClassifierCascade::CASCADE_TYPE_SEQUENTIAL) {
    os << "sequential, ";
  } else if (m_structure_type==CClassifierCascade::CASCADE_TYPE_FAN) {
    os << "fan, ";
  } else if (m_structure_type==CClassifierCascade::CASCADE_TYPE_TREE) {
    os << "tree, ";
    ASSERT(0);
    throw ITException("not implemented yet");
  } else {
    ASSERT(0);
  }
  os << m_template_width << "x" << m_template_height 
     << ", ratio " << m_image_area_ratio
     << " (fpr:" << m_total_false_positive_rate
     << ", dr:" << m_last_detection_rate
     << ", " << (m_trainset_exhausted?"exhausted":"successful")
     << ")" << endl;
  if (m_structure_type==CClassifierCascade::CASCADE_TYPE_FAN) {
    os << (int) m_branch_classifiers.size() << " branches." << endl;
    os << "COMMON BRANCH" << endl;
  }
  os << (int)m_classifiers.size() << " strong classifiers." << endl;

  for (int hcnt=0; hcnt<(int)m_classifiers.size(); hcnt++) {
    os << "STRONG classifier " << hcnt <<" (fpr:" 
       << m_lyr_false_positive_rates[hcnt] 
       << ", dr:" << m_lyr_detection_rates[hcnt] << "):\n"
       << m_classifiers[hcnt];
  }

  for (int brc=0; brc<(int)m_branch_classifiers.size(); brc++) {
    os << "BRANCH " << brc << " \"" << m_branch_names[brc]
       << "\" (fpr:" << m_branch_false_positive_rates[brc]
       << ", dr:" << m_branch_detection_rates[brc] << ")" << endl;
    os << (int) m_branch_classifiers[brc].size() 
       << " strong classifiers." << endl;
    for (int hcnt=0; hcnt<(int)m_branch_classifiers[brc].size(); hcnt++) {
      os << "STRONG classifier " << hcnt+(int)m_classifiers.size() <<" (fpr:" 
         << m_branch_lyr_false_positive_rates[brc][hcnt] 
         << ", dr:" << m_branch_lyr_detection_rates[brc][hcnt] << "):\n"
         << m_branch_classifiers[brc][hcnt];
    }
  }

  return os;
}

bool CClassifierCascade::Evaluate(const CIntegralImage& image,
                                  CStringVector& matches) const
{
  for (CSClsfVector::const_iterator it=m_classifiers.begin();
       it!=m_classifiers.end(); it++) {
    bool is_pos = it->Evaluate(image);
    if (!is_pos) return false;
  }

  // what structure?
  if (m_structure_type==CASCADE_TYPE_SEQUENTIAL) {
    matches.push_back(m_name);
    return true;

  } else if (m_structure_type==CASCADE_TYPE_FAN) {
    ASSERT(matches.size()==0);
    int num_branches = (int) m_branch_classifiers.size();
    for (int brcnt=0; brcnt<num_branches; brcnt++) {
      bool is_pos = true;
      for (CSClsfVector::const_iterator it=m_branch_classifiers[brcnt].begin();
           it!=m_branch_classifiers[brcnt].end(); it++) {
        is_pos = it->Evaluate(image);
        if (!is_pos) break;
      }
      if (is_pos) {
        matches.push_back(m_branch_names[brcnt]);
      }
    }
    return matches.size()>0;

  } else if (m_structure_type==CASCADE_TYPE_TREE) {
    throw ITException("not implemented yet");

  } else {
    ASSERT(0);
    return 0;
  }
}

#ifdef DEBUG
bool CClassifierCascade::EvaluateDebug(const CIntegralImage& image, 
                                       CStringVector& matches,
                                       ostream& os) const
{
  os << "Cascade evaluating image, " << (int)m_classifiers.size() << " strong classifiers." << endl;
  int hcnt=0;
  for (CSClsfVector::const_iterator it=m_classifiers.begin(); it!=m_classifiers.end(); it++, hcnt++) {
    bool is_pos = it->Evaluate(image);
    os << "STRONG " << hcnt << ": " << is_pos << endl;
    if (!is_pos) return false;
  }

  // what structure?
  if (m_structure_type==CASCADE_TYPE_SEQUENTIAL) {
    matches.push_back(m_name);
    return true;

  } else if (m_structure_type==CASCADE_TYPE_FAN) {
    ASSERT(matches.size()==0);
    int num_branches = (int) m_branch_classifiers.size();
    for (int brcnt=0; brcnt<num_branches; brcnt++) {
      bool is_pos = true;
      for (CSClsfVector::const_iterator it=m_branch_classifiers[brcnt].begin();
           it!=m_branch_classifiers[brcnt].end(); it++) {
        is_pos = it->Evaluate(image);
        if (!is_pos) break;
      }
      if (is_pos) {
        matches.push_back(m_branch_names[brcnt]);
      }
    }
    return matches.size()>0;

  } else if (m_structure_type==CASCADE_TYPE_TREE) {
    throw ITException("not implemented yet");

  } else {
    ASSERT(0);
    return 0;
  }
}
#endif // DEBUG

bool CClassifierCascade::Evaluate(
  const CIntegralImage& image,
  double mean, double stddev, int left, int top,
  CStringVector& matches) const
{
  for (CSClsfVector::const_iterator it=m_classifiers.begin();
       it!=m_classifiers.end();
       it++) {
    bool is_pos = it->Evaluate(image, mean, stddev, left, top);
    if (!is_pos) return false;
  }

  // what structure?
  if (m_structure_type==CASCADE_TYPE_SEQUENTIAL) {
    matches.push_back(m_name);
    return true;

  } else if (m_structure_type==CASCADE_TYPE_FAN) {
    ASSERT(matches.size()==0);
    int num_branches = (int) m_branch_classifiers.size();
    for (int brcnt=0; brcnt<num_branches; brcnt++) {
      bool is_pos = true;
      for (CSClsfVector::const_iterator it=m_branch_classifiers[brcnt].begin();
           it!=m_branch_classifiers[brcnt].end(); it++) {
        is_pos = it->Evaluate(image, mean, stddev, left, top);
        if (!is_pos) break;
      }
      if (is_pos) {
        matches.push_back(m_branch_names[brcnt]);
      }
    }
    return matches.size()>0;

  } else if (m_structure_type==CASCADE_TYPE_TREE) {
    throw ITException("not implemented yet");

  } else {
    ASSERT(0);
    return 0;
  }
}

void CClassifierCascade::ScaleFeaturesEvenly(double scale_x, double scale_y,
        int scaled_template_width, int scaled_template_height) const
{
  CSClsfVector& mutable_classifers = (CSClsfVector&) m_classifiers;
  for (CSClsfVector::iterator it=mutable_classifers.begin(); 
       it!=mutable_classifers.end(); it++) {
    it->ScaleFeaturesEvenly(scale_x, scale_y,
                            scaled_template_width, scaled_template_height);
  }
  if (m_structure_type==CASCADE_TYPE_FAN) {
    for (int brcnt=0; brcnt<(int)m_branch_classifiers.size(); brcnt++) {
      CSClsfVector& mutable_classifers = 
        (CSClsfVector&) m_branch_classifiers[brcnt];
      for (CSClsfVector::iterator it=mutable_classifers.begin(); 
           it!=mutable_classifers.end(); it++) {
        
        it->ScaleFeaturesEvenly(scale_x, scale_y,
                                scaled_template_width, 
                                scaled_template_height);
      }
    }
  }
}

// return a copy to name(s)
CStringVector CClassifierCascade::GetNames() const
{
  if (m_structure_type==CASCADE_TYPE_SEQUENTIAL) {
    CStringVector sv;
    sv.push_back(m_name);
    return sv;

  } else if (m_structure_type==CASCADE_TYPE_FAN) {
    return m_branch_names;
  
  } else {
    throw ITException("not implemented yet");
  }
}

int CClassifierCascade::GetNumStrongClassifiers(int branch/*=-1*/) const
{
  if (branch==-1) {
    return (int)m_classifiers.size(); 
  } else {
    if (m_structure_type==CASCADE_TYPE_SEQUENTIAL) {
      throw ITException("no such branch for sequential type");

    } else if (m_structure_type==CASCADE_TYPE_FAN) {
      if (branch<0 || branch>=(int)m_branch_classifiers.size()) {
        throw ITException("branch number out of range");
      }
      return (int)m_branch_classifiers[branch].size(); 
    
    } else {
      ASSERT(0);
      throw ITException("structure type not implemented");
    }
  }
}

