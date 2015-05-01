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
  * $Id: Cascade.h,v 1.30 2004/11/11 01:58:58 matz Exp $
**/

// Cascade.h: header file for ClassifierCascades.
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


#if !defined(_CASCADE_H__INCLUDED_)
#define _CASCADE_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Classifiers.h"

//namespace {  // cubicles
  
typedef vector<CStrongClassifier> CSClsfVector;
typedef vector<CWeakClassifier> CWeakClsfVector;
typedef vector<CSClsfVector> CSClsfMatrix;

typedef vector<string> CStringVector;

/////////////////////////////////////////////////////////////////////////////
//
// class CClassifierCascade
//

class CClassifierCascade {
 private:
  enum CascadeType {
    CASCADE_TYPE_SEQUENTIAL = 0,
    CASCADE_TYPE_FAN        = 1,
    CASCADE_TYPE_TREE       = 3
  };
 public:
  CClassifierCascade();
  CClassifierCascade(const CClassifierCascade& frm);
  CClassifierCascade(int template_width, int template_height,
		     double image_area_ratio);
  ~CClassifierCascade();

  CClassifierCascade& operator=(const CClassifierCascade& frm);

  CStrongClassifier& AddStrongClassifier();
  CStrongClassifier& AddStrongClassifier(int branch, double fpr, double dr);
  bool Evaluate(const CIntegralImage& image, 
                CStringVector& matches) const;
#pragma warning (disable: 4786)
  bool EvaluateDebug(const CIntegralImage& image, 
                     CStringVector& matches, ostream& os) const;
  bool Evaluate(const CIntegralImage& image,
		double mean_adjust, double stddev, int left, int top,
                CStringVector& matches) const;
#pragma warning (default: 4786)
#ifdef WITH_TRAINING
  bool Evaluate(const ExampleList::const_iterator example) const;
  void EvaluateSet(const ExampleList& examples,
		   double* false_pos_rate, double* detection_rate) const;
  void DecreaseThreshMeetDR(
           const ExampleList& examples,
	   double* false_pos_rate,
	   double* detection_rate, double min_detection_rate, 
	   int adjust_clsf, int num_steps);
  void AdjustThreshMeetDR(
           const ExampleList& examples,
	   double* false_pos_rate, double max_false_pos_rate,
	   double* detection_rate, double min_detection_rate, 
	   int adjust_clsf, int num_steps);
  void AdjustThreshMeetFPR(
           const ExampleList& examples,
	   double* false_pos_rate, double max_false_pos_rate,
	   double* detection_rate,
	   int adjust_clsf, int num_steps);
  void EvaluateThreshs(const CIntegralImage& image,
		       double mean_adjust, double stddev, int left, int top,
		       CIntMatrix& numMatches, 
                       const CDoubleVector& threshs) const;
#endif // WITH_TRAINING

  void ScaleFeaturesEvenly(double scale_x, double scale_y,
        int scaled_template_width, int scaled_template_height) const;
  //  void ParseFrom(istream& is);
  void ParseFrom(const string& filename);
  int RemoveLastStrongClassifier(); // returns number of remaining ones
  int RemoveLastWeakClassifier();
  bool empty() const { return m_classifiers.empty(); }

  // member access
  int GetTemplateWidth() const { return m_template_width; }
  int GetTemplateHeight() const { return m_template_height; }
  int GetNumStrongClassifiers(int branch=-1) const;
  double GetTotalFalsePositiveRate() const
    { return m_total_false_positive_rate; }
  double GetFalsePositiveRate(int clsf) const;
  double GetLastDetectionRate() const { return m_last_detection_rate; }
  double GetDetectionRate(int clsf) const;
  double GetImageAreaRatio() const { return m_image_area_ratio; }
  bool GetExhausted() const { return m_trainset_exhausted; }
  CStrongClassifier& GetStrongClassifier(int num)
    { return m_classifiers[num]; }
  const CStrongClassifier& GetStrongClassifier(int num) const
    { return m_classifiers[num]; }
  void SetFalsePositiveRate(int clsf, double fpr);
  void SetDetectionRate(int clsf, double dr);
  void SetExhausted(bool exhausted) { m_trainset_exhausted = exhausted; }
  CStringVector GetNames() const; // return a copy

  ostream& output(ostream& os) const;

protected:
  /*
  void RealParseFrom(istream& is);
  void ParseSomeStrongClassifiers(istream& is, int offset,
                                  CSClsfVector& strongs,
                                  CDoubleVector& fprs,
                                  CDoubleVector& drs);
*/
 private:
  string                    m_name;
  CascadeType               m_structure_type;
  CSClsfVector		    m_classifiers;
  CSClsfMatrix		    m_branch_classifiers;
  CStringVector             m_branch_names;
  int			    m_template_width, m_template_height;
  double			    m_image_area_ratio; // width over height

  // statistics
  double		    m_total_false_positive_rate;
  double		    m_last_detection_rate;
  CDoubleVector		    m_lyr_false_positive_rates;
  CDoubleVector		    m_lyr_detection_rates;
  CDoubleVector		    m_branch_false_positive_rates;
  CDoubleVector		    m_branch_detection_rates;
  CDoubleMatrix		    m_branch_lyr_false_positive_rates;
  CDoubleMatrix		    m_branch_lyr_detection_rates;
  bool                      m_trainset_exhausted;

  friend int yyparse();
  
};

typedef vector<CClassifierCascade> CCascadeVector;

ostream& operator<<(ostream& os, const CClassifierCascade& casc);

//}  // namespace cubicles

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(_CASCADE_H__INCLUDED_)
