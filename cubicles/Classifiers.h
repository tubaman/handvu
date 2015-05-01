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
  * $Id: Classifiers.h,v 1.34 2004/11/17 00:12:13 matz Exp $
**/

// Classifiers.h: header file for weak and strong
// classifiers.  A weak classifier is an IntegralFeature and
// a threshold.  A strong classifier is a linear combination
// of weak classifiers.
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

#if !defined(_CLASSIFIERS_H__INCLUDED_)
#define _CLASSIFIERS_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "IntegralImage.h"
#include "IntegralFeatures.h"


/////////////////////////////////////////////////////////////////////////////
//
// class CWeakClassifier
//

class CWeakClassifier {
 public:
  CWeakClassifier();
  CWeakClassifier(const CWeakClassifier& frm);
  CWeakClassifier(CIntegralFeature* pFeature, 
                  bool is_lt, double thresh, double error);
  ~CWeakClassifier();
  
  CWeakClassifier& operator=(const CWeakClassifier& from);
  bool operator==(const CWeakClassifier& from) const;
  
  bool Evaluate(const CIntegralImage& image) const;
  bool Evaluate(const CIntegralImage& image, 
                double mean_adjust, double stddev, int left, int top) const;
#ifdef WITH_TRAINING
  bool Evaluate(const ExampleList::const_iterator example) const;
#endif // WITH_TRAINING
  void ScaleFeatureEvenly(double scale_x, double scale_y,
                          int scaled_template_width, 
                          int scaled_template_height);
  bool IsValid() const;
  void CopyFrom(const CWeakClassifier& frm, 
                const CIntegralFeature& feature);
  void CopyFrom(const CWeakClassifier& frm);
  const CIntegralFeature& GetFeature() const;
  void ParseFrom(istream& is, int template_width, int template_height);
  int GetComputeCost() const;

  double GetThreshold() const { return threshold; }
  void SetThreshold(double thresh);
  bool GetSignLT() const { return sign_lt; }
  void SetSignLT(bool lt) { sign_lt = lt; }
  double GetTrainingError() const { return train_error; }
  void SetTrainingError(double error) { train_error = error; }

  friend ostream& operator<<(ostream& os, const CWeakClassifier& clsf);
  
 private:
  bool				 sign_lt;	
  double			 threshold;
  double			 train_error;

 public:
  CIntegralFeature*	         feature;
  // less than if true, greater than otherwise
  static const double            MAX_THRESHOLD;
  static const double            MIN_THRESHOLD;
  static const double            EPSILON_THRESHOLD;
  static const int               THRESHOLD_PRECISION;
};


/////////////////////////////////////////////////////////////////////////////
//
// class CStrongClassifier
//

class CStrongClassifier {
 public:
  CStrongClassifier();
  CStrongClassifier(const CStrongClassifier& from);
  ~CStrongClassifier();
  
  CStrongClassifier& operator=(const CStrongClassifier& from);
  
  void AddWeakClassifier(CWeakClassifier* pClassifier, double alpha);
  int RemoveLastWeakClassifier();
  bool Evaluate(const CIntegralImage& image) const;
  bool Evaluate(const CIntegralImage& image,
                double mean_adjust, double stddev, int left, int top) const;
  void EvaluateThreshs(const CIntegralImage& image,
                       double mean_adjust, double stddev,
                       int left, int top,
                       CIntVector& numMatches,
                       const CDoubleVector& threshs) const;
  double GetAlphasThreshold() {return m_alphas_thresh;}
  void SetAlphasThreshold(double thresh) {m_alphas_thresh=thresh;}
  void ScaleFeaturesEvenly(double scale_x, double scale_y,
                           int scaled_template_width, 
                           int scaled_template_height);
  void ParseFrom(istream& is, int template_width, int template_height);
  int GetComputeCost() const;
  
  // member access
  int GetNumWeakClassifiers() const {return m_num_hyps; }
  const CWeakClassifier& GetWeakClassifier(int num) const;
  
  friend ostream& operator<<(ostream& os, const CStrongClassifier& clsf);
  
 private:
  CWeakClassifier**	                m_pClassifiers;
  int					m_num_hyps; 
  double*				m_alphas;
  double				m_sum_alphas;
  double				m_alphas_thresh;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(_CLASSIFIERS_H__INCLUDED_)
