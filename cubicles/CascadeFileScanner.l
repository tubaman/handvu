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
  * $Id: CascadeFileScanner.l,v 1.7 2004/10/22 03:26:00 matz Exp $
**

// CascadeFileScanner takes care of scanning integral features,
// weak classifiers, strong classifiers, and classifier cascades
// from file input.  Compile with flex.
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
*/

%{
#include "cubicles.hpp"
#include "IntegralFeatures.h"
#include "Classifiers.h"
#include "Cascade.h"
#include "CascadeFileParser.h"

void yyerror(const char *str);
//void yyerror(YYLTYPE *llocp, const char *str);

extern "C" int mywrap(void)
{
  /*
  FILE* fp = fopen("c:\\tmp\\dump3.txt", "a+");
  fprintf(fp, "wrap called\n");
  fflush(fp);
  fclose(fp);
  */
  return 1;
}

extern string yyIDstring;
 
void nextline(void);
 
#define yywrap mywrap

#define YY_NEVER_INTERACTIVE 1
#define YYERROR_VERBOSE 1

%}


/* definitions */

DIGIT [0-9]
ID [a-wyzA-WYZ_][a-zA-Z0-9_]*|"x"[a-zA-Z_][a-zA-Z0-9_]*
SIMPLE_FP ([+|-]?{DIGIT}+"."{DIGIT}*)|([+|-]?{DIGIT}*"."{DIGIT}+)
EXPONENT_FP [+|-]?{DIGIT}*"."{DIGIT}+[eE][-|+]?{DIGIT}+

%%

"ClassifierCascade" { return CLASSIFIER_CASCADE; }
"sequential" { return TYPE_SEQUENTIAL; }
"fan" { return TYPE_FAN; }
"tree" { return TYPE_TREE; }
"ratio" { return RATIO; }
"fpr" { return FPR; }
"dr" { return DR; }
"successful" { return SUCCESSFUL; }
"exhausted" { return EXHAUSTED; }
"strong" { return STRONG; }
"classifiers" { return CLASSIFIERS; }
"classifier" { return CLASSIFIER; }
"STRONG" { return UPCASE_STRONG; }
"weak" { return WEAK; }
"threshold" { return THRESHOLD; }
"NULL" { return NULL_ID; }
"LeftRight" { return LEFT_RIGHT; }
"UpDown" { return UP_DOWN; }
"LeftCenterRight" { return LEFT_CENTER_RIGHT; }
"SevenColumns" { return SEVEN_COLUMNS; }
"Diag" { return DIAG; }
"LeftRightSame" { return LEFT_RIGHT_SAME; }
"UpDownSame" { return UP_DOWN_SAME; }
"LeftCenterRightSame" { return LEFT_CENTER_RIGHT_SAME; }
"SevenColumnsSame" { return SEVEN_COLUMNS_SAME; }
"SevenColumnsSimilar" { return SEVEN_COLUMNS_SIMILAR; }
"DiagSame" { return DIAG_SAME; }
"DiagSimilar" { return DIAG_SIMILAR; }
"FourBoxes" { return FOUR_BOXES; }
"FourBoxesSame" { return FOUR_BOXES_SAME; }
"branches" { return BRANCHES; }
"COMMON" { return COMMON; }
"BRANCH" { return BRANCH; }


"\"" { return DOUBLEQUOTE; }
"," { return COMMA; }
"x" { return X_BY; }
":" { return COLON; }
"(" { return OPEN_PAREN; }
")" { return CLOSE_PAREN; }
"[" { return OPEN_BRACKET; }
"]" { return CLOSE_BRACKET; }
"*" { return ASTERIX; }
"<" { return LT; }
">=" { return GTEQ; }
"/" { return SLASH; }

{SIMPLE_FP}|{EXPONENT_FP} { yylval.fval = atof(yytext); return FP_NUMBER; }
-?{DIGIT}+ { yylval.ival = atoi(yytext); return INT_NUMBER; }
{ID} { yyIDstring = string(yytext); yylval.sval = yytext; return NAME; }

"." { return DOT; }
"-" { return MINUS; }

\  { ; }
\t { ; }
\r\n { nextline(); }
\n { nextline(); }

"#".*\r\n { nextline(); } /*comment*/
"#".*\n { nextline(); } /*comment*/
. { char buf[32]; sprintf(buf, "illegal token: ' '"); buf[16]=yytext[0]; yyerror(buf); }

%%

