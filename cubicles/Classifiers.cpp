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
  * $Id: Classifiers.cpp,v 1.57 2005/10/28 17:47:04 matz Exp $
**/

// Classifiers.cpp : implementation file for weak and strong
// classifiers.  A weak classifier is an IntegralFeature and
// a threshold.  A strong classifier is a linear combination
// of weak classifiers.
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
#include "Classifiers.h"
#ifdef HAVE_FLOAT_H
#include <float.h>
#endif
#include <iostream>
#ifdef IMG_LIB_OPENCV
#include <cxcore.h>
#endif


#ifdef _DEBUG
#ifdef USE_MFC
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif // USE_MFC
#endif // _DEBUG

#include <math.h>
#include <iomanip>    // for setprecision
using std::ofstream;

/////////////////////////////////////////////////////////////////////////////
//
// CWeakClassifier implementation
//
/////////////////////////////////////////////////////////////////////////////

// "this->threshold" should lie within these boundaries:
const double CWeakClassifier::MIN_THRESHOLD = -(DBL_MAX/10.0);
const double CWeakClassifier::MAX_THRESHOLD = DBL_MAX/10.0;
const double CWeakClassifier::EPSILON_THRESHOLD = 6.8e-07;
const int CWeakClassifier::THRESHOLD_PRECISION = 6;

/* round to "precision" digits after the comma
 */
double round(double d, int precision)
{
  for (int p=0; p<precision; p++) {
    d *= 10.0;
  }
#ifdef IMG_LIB_OPENCV
  double r = cvRound(d); // optimized
#else
  double r = round(d);
#endif
  for (int p=0; p<precision; p++) {
    r /= 10.0;
  }
  return r;
}


CWeakClassifier::CWeakClassifier()
  : feature(NULL),
    sign_lt(true),
    threshold(0),
    train_error(DBL_MAX)
{
}

CWeakClassifier::CWeakClassifier(CIntegralFeature* pFeature, 
                                 bool is_lt, double thresh, double error)
  : feature(pFeature),
    sign_lt(is_lt),
    train_error(error)
{
  SetThreshold(thresh);
}

CWeakClassifier::CWeakClassifier(const CWeakClassifier& frm)
  : feature(NULL)
{
  CopyFrom(frm);
}

CWeakClassifier::~CWeakClassifier()
{
  delete feature;
  feature = NULL;
}

void CWeakClassifier::CopyFrom(const CWeakClassifier& frm_classf, const CIntegralFeature& frm_feature)
{
  delete feature;
  feature = frm_feature.Copy();
  threshold = frm_classf.threshold;
  sign_lt = frm_classf.sign_lt;
  train_error = frm_classf.train_error;
}

void CWeakClassifier::CopyFrom(const CWeakClassifier& frm_classf)
{
  delete feature;
  if (frm_classf.feature) {
    feature = frm_classf.feature->Copy();
  } else {
    feature = NULL;
  }
  threshold = frm_classf.threshold;
  sign_lt = frm_classf.sign_lt;
  train_error = frm_classf.train_error;
}

CWeakClassifier& CWeakClassifier::operator=(const CWeakClassifier& frm)
{
  CopyFrom(frm);
  return *this;
}

bool CWeakClassifier::operator==(const CWeakClassifier& from) const
{
  if (feature==NULL && from.feature==NULL) return true;
  if (threshold!=from.threshold) return false;
  if (sign_lt!=from.sign_lt) return false;
  if (feature==NULL || from.feature==NULL) return false;
  
  return feature->Equals(*from.feature);
}

const CIntegralFeature& 
CWeakClassifier::GetFeature() const
{
	ASSERT(feature!=NULL);
	return *feature;
}

void CWeakClassifier::SetThreshold(double thresh)
{
  threshold = round(thresh, THRESHOLD_PRECISION);
}

bool CWeakClassifier::Evaluate(const CIntegralImage& image) const
{
  ASSERT(feature);
  double feature_value = feature->Compute(image);

  if (sign_lt) {
    return feature_value<threshold;
  } else {
    return feature_value>=threshold;
  }
}

bool CWeakClassifier::Evaluate(const CIntegralImage& image, double mean, double stddev, int left, int top) const
{

  ASSERT(feature);

  double feature_value = feature->ComputeScaled(image, mean, left, top);
  feature_value /= stddev;

#if defined(II_TYPE_INT) || defined(II_TYPE_UINT)
  feature_value /= stddev;
  feature_value *= 127.5;
  feature_value += 127.5;
#endif

  if (sign_lt) {
    return feature_value<threshold;
  } else {
    return feature_value>=threshold;
  }
}

#ifdef WITH_TRAINING
bool CWeakClassifier::Evaluate(const ExampleList::const_iterator example) const
{
  ASSERT(feature);
  if (sign_lt) {
    return feature->Compute(example)<threshold;
  } else {
    return feature->Compute(example)>=threshold;
  }
}

bool CWeakClassifier::IsValid() const
{
  bool finite;
#ifdef HAVE_ISNAN
  finite = !isnan(train_error);
#else
#error how to check for overflow on this platform?
#endif

  return (finite
        && !(threshold>=CWeakClassifier::MAX_THRESHOLD)
        && !(threshold<=CWeakClassifier::MIN_THRESHOLD)
        && !(fabs(threshold)<CWeakClassifier::EPSILON_THRESHOLD));
}
#endif // WITH_TRAINING

ostream& operator<<(ostream& os, const CWeakClassifier& clsf)
{
	os << "[";
	if (clsf.feature) {
	  clsf.feature->output(os);
	} else {
	  os << "NULL";
	}
	os << "]" << (clsf.sign_lt?"<":">=");
	os << std::fixed
	   << std::setprecision (CWeakClassifier::THRESHOLD_PRECISION)
	   << clsf.threshold << " ";
	os << "(" << clsf.train_error << ")";
	return os;
}

void CWeakClassifier::ParseFrom(istream& is, int template_width, int template_height)
{
  delete feature;
  feature = NULL;
  char c, d, e, f;
  is >> c;
  if (c!='[') {
    is.putback(c);
    throw ITException(string("wrong character on input stream: '")+c+string("'"));
  }
  is >> c >> d >> e >> f;
  if (c=='N' && d=='U' && e=='L' && f=='L') {
    // feature = NULL;
  } else {
    is.putback(f), is.putback(e), is.putback(d), is.putback(c);
    feature = CIntegralFeature::CreateFrom(is, template_width, template_height);
  }
  is >> c;
  if (c!=']') {
    is.putback(c);
    throw ITException(string("wrong character on input stream: '")+c+string("'"));
  }
  is >> c >> d;
  if (c=='<') {
    sign_lt=true;
    is.putback(d);
  } else if (c=='>' && d=='=') {
    sign_lt=false;
  } else {
    char buf[256];
    sprintf(buf, "wrong characters on input stream (%c|%c)\n", c, d);
    throw ITException(buf);
  }
  double thresh;
  is >> thresh;
  SetThreshold(thresh);
  is >> c >> train_error >> d;
  if (c!='(' || d!=')') {
    is.putback(d);
    char s[32];
    sprintf(s, "%f", train_error);
    for (int i=0; i<(int)strlen(s); i++) is.putback(s[i]);
    is.putback(c);
    char buf[256];
    sprintf(buf, "wrong character on input stream: '%c|%f|%c'\n", 
            c, (float)train_error, d);
    throw ITException(buf);
  }
}

void CWeakClassifier::ScaleFeatureEvenly(double scale_x, double scale_y,
        int scaled_template_width, int scaled_template_height)
{
  feature->ScaleEvenly(scale_x, scale_y, 
		       scaled_template_width, scaled_template_height);
}

int CWeakClassifier::GetComputeCost() const
{
  ASSERT(feature);
  return feature->GetComputeCost();
}


/////////////////////////////////////////////////////////////////////////////
//
// 	CStrongClassifier implementation
//
/////////////////////////////////////////////////////////////////////////////

CStrongClassifier::CStrongClassifier(const CStrongClassifier& from)
:	m_num_hyps(from.m_num_hyps),
	m_alphas_thresh(from.m_alphas_thresh),
	m_sum_alphas(from.m_sum_alphas),
  m_pClassifiers(NULL),
  m_alphas(NULL)
{
  if (m_num_hyps) {
  	m_pClassifiers = new CWeakClassifier*[m_num_hyps];
	  m_alphas = new double[m_num_hyps];
  }
	double sum_alphas=0.0;
	for (int hcnt=0; hcnt<m_num_hyps; hcnt++) {
		m_pClassifiers[hcnt] = new CWeakClassifier(*from.m_pClassifiers[hcnt]);
		m_alphas[hcnt] = from.m_alphas[hcnt];
		sum_alphas += m_alphas[hcnt];
	}
	ASSERT(sum_alphas==m_sum_alphas);
}

CStrongClassifier::CStrongClassifier()
:	m_num_hyps(0),
	m_alphas_thresh(0.5),
  m_pClassifiers(NULL),
  m_alphas(NULL),
	m_sum_alphas(0.0)
{
}

CStrongClassifier::~CStrongClassifier()
{
	for (int hcnt=0; hcnt<m_num_hyps; hcnt++) {
		delete m_pClassifiers[hcnt];
		m_pClassifiers[hcnt] = NULL;
	}
 	delete[] m_pClassifiers;
	m_pClassifiers = NULL;
  delete[] m_alphas;
  m_alphas = NULL;
}

CStrongClassifier& CStrongClassifier::operator=(const CStrongClassifier& from)
{
	this->~CStrongClassifier();

	m_num_hyps = from.m_num_hyps;
	m_alphas_thresh = from.m_alphas_thresh;
	m_sum_alphas = from.m_sum_alphas;
  if (m_num_hyps) {
  	m_pClassifiers = new CWeakClassifier*[m_num_hyps];
	  m_alphas = new double[m_num_hyps];
  } else {
    m_pClassifiers = NULL;
    m_alphas = NULL;
  }
	double sum_alphas=0.0;
	for (int hcnt=0; hcnt<m_num_hyps; hcnt++) {
		m_pClassifiers[hcnt] = 
                  new CWeakClassifier(*from.m_pClassifiers[hcnt]);
		m_alphas[hcnt] = from.m_alphas[hcnt];
		sum_alphas += m_alphas[hcnt];
	}
	ASSERT(sum_alphas==m_sum_alphas);

	return *this;
}

void 
CStrongClassifier::AddWeakClassifier(CWeakClassifier* pClassifier, double alpha)
{
	// save old array pointers
	CWeakClassifier** tmp_pClassifiers=m_pClassifiers;
	double* tmp_alphas=m_alphas;
	int old_num_hyps=m_num_hyps;
	m_num_hyps++;
	// create new arrays
	m_pClassifiers=new CWeakClassifier*[m_num_hyps];
	m_alphas=new double[m_num_hyps];
	// copy old to new
	int hcnt=0;
	for (; hcnt<old_num_hyps; hcnt++) {
		m_pClassifiers[hcnt]=tmp_pClassifiers[hcnt];
		m_alphas[hcnt]=tmp_alphas[hcnt];
	}
	// delete old
	delete[] tmp_pClassifiers;
	delete[] tmp_alphas;
	// insert new
	m_pClassifiers[hcnt]=new CWeakClassifier(*pClassifier);
	m_alphas[hcnt]=alpha;
	m_sum_alphas+=alpha;
}

int
CStrongClassifier::RemoveLastWeakClassifier()
{
  if (m_num_hyps) {
    m_num_hyps--;
    delete m_pClassifiers[m_num_hyps];
    m_pClassifiers[m_num_hyps] = NULL;
    m_sum_alphas -= m_alphas[m_num_hyps];
    return m_num_hyps;
  } else {
    return m_num_hyps;
  }
}

const CWeakClassifier&
CStrongClassifier::GetWeakClassifier(int num) const
{
	ASSERT(num<m_num_hyps);
	return *m_pClassifiers[num];
}

void CStrongClassifier::ParseFrom(istream& is, int template_width, int template_height)
{
  // delete old, make consistent state
  this->~CStrongClassifier();
  m_num_hyps = 0;

  string str, str2, str3;
  char c;
  int num_hyps;
  is >> num_hyps >> str >> str2 >> str3 >> m_alphas_thresh >> c;
  if (str!="weak" || str2!="classifiers," || str3!="threshold" || c!=':') {
    throw ITException("expected 'n weak classsfiers, threshold f:'");
  }
  // we have to post-parse assign m_num_hyps, and we have to 
  // do CWeakClassifier() allocation in a separate, early loop,
  // because we might bail out with "return false" at any moment
  // which would otherwise leave the StrongClassifier datastructure
  // in an inconsistent state, which is hard to delete
  m_num_hyps = num_hyps;
  if (m_num_hyps) {
    m_pClassifiers = new CWeakClassifier*[m_num_hyps];
    m_alphas = new double[m_num_hyps];
  } else {
    m_pClassifiers = NULL;
    m_alphas = NULL;
  }
  for (int hcnt=0; hcnt<m_num_hyps; hcnt++) {
    double alpha=-1;
    is >> alpha >> str;
    if (str!="*") {
      // check if all weak classifiers have numbers as alpha values
      throw ITException("expected '*'");
    }
    m_pClassifiers[hcnt] = new CWeakClassifier();
    m_pClassifiers[hcnt]->ParseFrom(is, template_width, template_height);
    /*
    ofstream out("c:\\tmp\\dump.txt", ios::out | ios::app);
    out << *m_pClassifiers[hcnt] << endl;
    out.flush();
    out.close();
*/

    m_alphas[hcnt]=alpha;
    m_sum_alphas+=alpha;
  }
}

ostream& operator<<(ostream& os, const CStrongClassifier& clsf)
{
  os << clsf.m_num_hyps << " weak classifiers, threshold " 
     << clsf.m_alphas_thresh << ":" << endl;
  for (int hcnt=0; hcnt<clsf.m_num_hyps; hcnt++) {
    os << clsf.m_alphas[hcnt] << " * ";
    os << *clsf.m_pClassifiers[hcnt];
    os << "\n";
  }
  return os;
}

bool CStrongClassifier::Evaluate(const CIntegralImage& image) const
{
	double sum=0.0;
	for (int hcnt=0; hcnt<m_num_hyps; hcnt++) {
		sum += m_alphas[hcnt] * m_pClassifiers[hcnt]->Evaluate(image);
	}
	ASSERT(m_alphas_thresh); // this should be greater than zero, otherwise
	// the strong classifier doesn't do any classification.

	return (sum >= m_alphas_thresh*m_sum_alphas);
}

bool CStrongClassifier::Evaluate(const CIntegralImage& image, double mean, double stddev, int left, int top) const
{
	double sum=0.0;
	for (int hcnt=0; hcnt<m_num_hyps; hcnt++) {
	  sum += 
	    m_alphas[hcnt] 
	    * m_pClassifiers[hcnt]->Evaluate(image, mean, stddev, left, top);
	}
	ASSERT(m_alphas_thresh); // this should be greater than zero, otherwise
	// the strong classifier doesn't do any classification.

	return (sum >= m_alphas_thresh*m_sum_alphas);
}

void CStrongClassifier::EvaluateThreshs(const CIntegralImage& image,
					double mean, double stddev,
					int left, int top,
					CIntVector& numMatches,
					const CDoubleVector& threshs) const
{
  double sum=0.0;
  for (int hcnt=0; hcnt<m_num_hyps; hcnt++) {
    sum += 
      m_alphas[hcnt] 
      * m_pClassifiers[hcnt]->Evaluate(image, mean, stddev, left, top);
  }

  // do the classification for each threshold
  int num_threshs = (int) threshs.size();
  for (int tcnt=0; tcnt<num_threshs; tcnt++) {
    double thresh = threshs[tcnt];
    if (sum >= thresh*m_sum_alphas) {
      numMatches[tcnt]++;
    }
  }
}

void CStrongClassifier::ScaleFeaturesEvenly(double scale_x, double scale_y,
					    int scaled_template_width, 
					    int scaled_template_height)
{
  for (int hcnt=0; hcnt<m_num_hyps; hcnt++) {
    m_pClassifiers[hcnt]->ScaleFeatureEvenly(scale_x, scale_y,
                                             scaled_template_width, 
					     scaled_template_height);
  }
}

int CStrongClassifier::GetComputeCost() const
{
  int sum = 0;
  for (int hcnt=0; hcnt<m_num_hyps; hcnt++) {
    sum += m_pClassifiers[hcnt]->GetComputeCost();
  }
  return sum;
}

