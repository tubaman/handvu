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
  * $Id: CascadeFileParser.yy,v 1.5 2004/10/09 17:04:53 matz Exp $
**/

// CascadeFileParser takes care of parsing integral features,
// weak classifiers, strong classifiers, and classifier cascades
// from file input.  Compile with bison.
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

%{
#include "Cascade.h"
#include "Classifiers.h"
#include "IntegralFeatures.h"
#include "Exceptions.h"
//#include <stdio.h>

// internal variables:
bool parse_entire_cascade;
int image_width, image_height;
CStrongClassifier* curr_strong;
CClassifierCascade* pCascade;
CWeakClassifier* parsed_weak;
int curr_line;
int curr_branch;
string yyIDstring;

void nextline(void)
{
  curr_line++;
}

void yyerror(const char *str)
//void yyerror(YYLTYPE *llocp, const char *str)
{
  char* buf = new char[strlen(str)+1024];
  sprintf(buf, "parsing error (line %d): %s",
          curr_line, str);
  string safe(buf);
  delete buf;
  throw ITException(safe);
//  printf("error: %s\n", str);
}

int yyparse(void);
int yylex(void);  

extern FILE* yyin;

CClassifierCascade* parse_cascade(FILE* fp)
{
  // init variables
  pCascade = NULL;
  curr_strong = NULL;
  parsed_weak = NULL;
  image_width = image_height = -1;
  parse_entire_cascade = true;
  curr_line = 1;
  yyin = fp;
  //  yyout = fopen("C:\\tmp\\dump.txt", "a+");

  // parse the file!
  int result = yyparse();

  // fflush(yyout);
  // fclose(yyout);

  if (result) {
    delete pCascade;
    throw ITException("error parsing");
  }

  return pCascade;
}

%}


%union {
  int ival;
  double fval;
  char* sval;
  bool bval;
  CIntegralFeature* pIF;
  CRect* pRect;
  CWeakClassifier* pWC;
  CClassifierCascade* pCasc;
}

//%error-verbose

%start ROOT

%token CLASSIFIER_CASCADE
%token TYPE_SEQUENTIAL
%token TYPE_FAN
%token TYPE_TREE
%token RATIO
%token FPR
%token DR
%token SUCCESSFUL
%token EXHAUSTED
%token STRONG
%token CLASSIFIER
%token CLASSIFIERS
%token UPCASE_STRONG
%token WEAK
%token NULL_ID
%token THRESHOLD
%token LEFT_RIGHT
%token UP_DOWN
%token LEFT_CENTER_RIGHT
%token SEVEN_COLUMNS
%token DIAG
%token LEFT_RIGHT_SAME
%token UP_DOWN_SAME
%token LEFT_CENTER_RIGHT_SAME
%token SEVEN_COLUMNS_SAME
%token SEVEN_COLUMNS_SIMILAR
%token DIAG_SAME
%token DIAG_SIMILAR
%token FOUR_BOXES
%token FOUR_BOXES_SAME

%token DOUBLEQUOTE
%token COMMA
%token X_BY
%token SLASH
%token DOT
%token COLON
%token OPEN_PAREN
%token CLOSE_PAREN
%token OPEN_BRACKET
%token CLOSE_BRACKET
%token ASTERIX
%token LT
%token GTEQ
%token MINUS
%token BRANCH
%token BRANCHES
%token COMMON

%token <ival> INT_NUMBER
%token <fval> FP_NUMBER
%token <sval> NAME

%type <pIF> integral_feature
%type <pWC> weak_classifier
%type <pRect> quadruple
%type <bval> comparison_lt trainset_exhausted
%type <pCasc> cascade
%type <fval> fp_number
%type <ival> strong_classifier_package_header strong_classifier_header;

%%

ROOT:
  cascade {}
| weak_classifier {}
;

cascade:
  cascade_header_p1
    cascade_header_p2_sequential
    strong_classifier_package
  {
  }
| cascade_header_p1
    cascade_header_p2_fan
    branch_seq
  {
  }
| cascade_header_p1
    cascade_header_p2_tree
  {
  }
;

cascade_header_p1:
  CLASSIFIER_CASCADE DOUBLEQUOTE NAME DOUBLEQUOTE COMMA
  {
    if (!parse_entire_cascade) {
      throw ITException("wrong parsing mode set");
    }
    pCascade = new CClassifierCascade();
    pCascade->m_name = yyIDstring;
//    pCascade->m_name = $3;
    curr_branch = -1;
  }
;

cascade_header_p2_sequential:
  TYPE_SEQUENTIAL COMMA INT_NUMBER X_BY INT_NUMBER COMMA
    RATIO fp_number OPEN_PAREN FPR COLON fp_number COMMA
    DR COLON fp_number COMMA trainset_exhausted CLOSE_PAREN
  {
    image_width = $3;
    image_height = $5;
    pCascade->m_template_width = $3;
    pCascade->m_template_height = $5;
    pCascade->m_image_area_ratio = $8;
    pCascade->m_structure_type = CClassifierCascade::CASCADE_TYPE_SEQUENTIAL;
    pCascade->m_total_false_positive_rate = $12;
    pCascade->m_last_detection_rate = $16;
    pCascade->m_trainset_exhausted = $18;
  }
;

cascade_header_p2_fan:
  TYPE_FAN COMMA INT_NUMBER X_BY INT_NUMBER COMMA
    RATIO fp_number OPEN_PAREN FPR COLON fp_number COMMA
    DR COLON fp_number COMMA trainset_exhausted CLOSE_PAREN
    INT_NUMBER BRANCHES DOT
  {
    image_width = $3;
    image_height = $5;
    pCascade->m_template_width = $3;
    pCascade->m_template_height = $5;
    pCascade->m_image_area_ratio = $8;
    pCascade->m_structure_type = CClassifierCascade::CASCADE_TYPE_FAN;
    pCascade->m_total_false_positive_rate = $12;
    pCascade->m_last_detection_rate = $16;
    pCascade->m_trainset_exhausted = $18;
    int num_branches = $20;
    pCascade->m_branch_classifiers.resize(num_branches);
    pCascade->m_branch_names.resize(num_branches);
    pCascade->m_branch_false_positive_rates.resize(num_branches);
    pCascade->m_branch_detection_rates.resize(num_branches);
    pCascade->m_branch_lyr_false_positive_rates.resize(num_branches);
    pCascade->m_branch_lyr_detection_rates.resize(num_branches);
  }
;

cascade_header_p2_tree:
  TYPE_TREE
  {
    pCascade->m_structure_type = CClassifierCascade::CASCADE_TYPE_TREE;
    throw ITException("cascade type tree not supported yet");
  }
;

trainset_exhausted:
  SUCCESSFUL { $$ = false; }
| EXHAUSTED  { $$ = true; }
;

branch_seq:
  branch_header strong_classifier_package
| branch_seq branch_header strong_classifier_package
;

branch_header:
  COMMON BRANCH
  {
  }
| BRANCH INT_NUMBER DOUBLEQUOTE NAME DOUBLEQUOTE
    OPEN_PAREN FPR COLON fp_number COMMA DR COLON fp_number CLOSE_PAREN
  {
    curr_branch = $2;
    if (curr_branch<0 || curr_branch>=(int)pCascade->m_branch_classifiers.size()) {
      throw ITException("branch number out of range");
    }
    pCascade->m_branch_names[$2] = yyIDstring; // $4;
    pCascade->m_branch_false_positive_rates[$2] = $9;
    pCascade->m_branch_detection_rates[$2] = $13;
  }
;

strong_classifier_package:
  strong_classifier_package_header
    strong_classifier_seq
  {
    int declared_num_strong_clsf = $1;
    if (declared_num_strong_clsf
          !=pCascade->GetNumStrongClassifiers(curr_branch))
    {
      char buf[1024];
      sprintf(buf, "instead of %d strong classifiers as declared, "
                   "%d were found (branch %d)",
              declared_num_strong_clsf,
              pCascade->GetNumStrongClassifiers(curr_branch),
              curr_branch);
      throw ITException(buf);
    }
  }
;

strong_classifier_package_header:
  INT_NUMBER STRONG CLASSIFIERS DOT
  {
    $$ = $1;
  }
;

strong_classifier_seq:
  strong_classifier_seq strong_classifier
| strong_classifier
;

strong_classifier:
  strong_classifier_header
    weighted_weak_classifier_seq
  {
    int delared_num_weak = $1;
    if (delared_num_weak!=curr_strong->GetNumWeakClassifiers()) {
      char buf[1024];
      sprintf(buf, "declared %d weak classifiers, found %d",
              delared_num_weak, curr_strong->GetNumWeakClassifiers());
      throw ITException(buf);
    }
  }
;

strong_classifier_header:
  UPCASE_STRONG CLASSIFIER INT_NUMBER OPEN_PAREN
    FPR COLON fp_number COMMA DR COLON fp_number CLOSE_PAREN COLON
    INT_NUMBER WEAK CLASSIFIERS COMMA THRESHOLD fp_number COLON
  {
    int declared_id = $3;
    int branch_offset = 0;
    if (curr_branch!=-1) {
       branch_offset = pCascade->GetNumStrongClassifiers();
    }
    if (declared_id-branch_offset
          !=pCascade->GetNumStrongClassifiers(curr_branch))
    {
      char buf[1024];
      sprintf(buf, "expected strong classifier number %d, found number %d (branch %d)",
              pCascade->GetNumStrongClassifiers(), $3, curr_branch);
      throw ITException(buf);
    }
    curr_strong = &pCascade->AddStrongClassifier(curr_branch, $7, $11);
    curr_strong->SetAlphasThreshold($19);
    $$ = $14;
  }
;

weighted_weak_classifier_seq:
  weighted_weak_classifier_seq weighted_weak_classifier
| weighted_weak_classifier
;

weighted_weak_classifier:
  fp_number ASTERIX weak_classifier
  {
    curr_strong->AddWeakClassifier($3, $1);
    delete $3;
  }
;

weak_classifier:
  OPEN_BRACKET integral_feature CLOSE_BRACKET
    comparison_lt fp_number OPEN_PAREN fp_number CLOSE_PAREN
  {
    if (parse_entire_cascade) {
      $$ = new CWeakClassifier($2, $4, $5, $7);
    } else {
      parsed_weak = new CWeakClassifier($2, $4, $5, $7);
    }
  }
;

comparison_lt:
  LT { $$ = true; }
| GTEQ { $$ = false; }
;

integral_feature:
  NULL_ID { $$ = NULL; }
|
  LEFT_RIGHT INT_NUMBER COMMA INT_NUMBER X_BY
    INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER
  { $$ = new CLeftRightIF(image_width, image_height,
                          $2, $4, $6, $8, $10);
  }
|
  UP_DOWN INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER X_BY
    INT_NUMBER COMMA INT_NUMBER
  { $$ = new CUpDownIF(image_width, image_height,
                       $2, $4, $6, $8, $10);
  }
|
  LEFT_CENTER_RIGHT INT_NUMBER COMMA INT_NUMBER X_BY
    INT_NUMBER COMMA INT_NUMBER SLASH INT_NUMBER COMMA INT_NUMBER
  { $$ = new CLeftCenterRightIF(image_width, image_height,
                                $2, $4, $6, $8, $10, $12);
  }
|
  SEVEN_COLUMNS INT_NUMBER COMMA INT_NUMBER X_BY
    INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER COMMA
    INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER
  { $$ = new CSevenColumnsIF(image_width, image_height,
                             $2, $4, $6, $8, $10, $12, $14, $16, $18, $20);
  }
|
  DIAG INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER X_BY
    INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER
  { $$ = new CDiagIF(image_width, image_height,
                     $2, $4, $6, $8, $10, $12);
  }
|
  LEFT_RIGHT_SAME INT_NUMBER COMMA INT_NUMBER X_BY
    INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER
  { $$ = new CLeftRightSameIF(image_width, image_height,
                              $2, $4, $6, $8, $10);
  }
|
  UP_DOWN_SAME INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER X_BY
    INT_NUMBER COMMA INT_NUMBER
  { $$ = new CUpDownSameIF(image_width, image_height,
                           $2, $4, $6, $8, $10);
  }
|
  LEFT_CENTER_RIGHT_SAME INT_NUMBER COMMA INT_NUMBER X_BY
    INT_NUMBER COMMA INT_NUMBER SLASH INT_NUMBER COMMA INT_NUMBER
  { $$ = new CLeftCenterRightSameIF(image_width, image_height,
                                    $2, $4, $6, $8, $10, $12);
  }
|
  SEVEN_COLUMNS_SAME INT_NUMBER COMMA INT_NUMBER X_BY
    INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER COMMA
    INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER
  { $$ = new CSevenColumnsSameIF(image_width, image_height,
                                 $2, $4,
                                 $6, $8, $10, $12, $14, $16, $18, $20);
  }
|
  SEVEN_COLUMNS_SIMILAR INT_NUMBER COMMA INT_NUMBER X_BY
    INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER COMMA
    INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER
  { $$ = new CSevenColumnsSimilarIF(image_width, image_height,
                                    $2, $4,
                                    $6, $8, $10, $12, $14, $16, $18, $20);
  }
|
  DIAG_SAME INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER X_BY
    INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER
  { $$ = new CDiagSameIF(image_width, image_height,
                         $2, $4, $6, $8, $10, $12);
  }
|
  DIAG_SIMILAR INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER X_BY
    INT_NUMBER COMMA INT_NUMBER COMMA INT_NUMBER
  { $$ = new CDiagSimilarIF(image_width, image_height,
                            $2, $4, $6, $8, $10, $12);
  }
|
  FOUR_BOXES quadruple COMMA quadruple COMMA
    quadruple COMMA quadruple
  {
    $$ = new CFourBoxesIF(image_width, image_height,
                          $2, $4, $6, $8);
    delete $2;
    delete $4;
    delete $6;
    delete $8;
  }
|
  FOUR_BOXES_SAME quadruple COMMA quadruple COMMA
    quadruple COMMA quadruple
  {
    $$ = new CFourBoxesSameIF(image_width, image_height,
                              $2, $4, $6, $8);
    delete $2;
    delete $4;
    delete $6;
    delete $8;
  }
;

quadruple:
  OPEN_PAREN INT_NUMBER COMMA INT_NUMBER COMMA
    INT_NUMBER COMMA INT_NUMBER CLOSE_PAREN
  { $$ = new CRect($2, $4, $6, $8); }
;

fp_number:
  FP_NUMBER { $$ = $1; }
| INT_NUMBER { $$ = (double) $1; }
;
