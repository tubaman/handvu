/* A Bison parser, made by GNU Bison 1.875c.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     CLASSIFIER_CASCADE = 258,
     TYPE_SEQUENTIAL = 259,
     TYPE_FAN = 260,
     TYPE_TREE = 261,
     RATIO = 262,
     FPR = 263,
     DR = 264,
     SUCCESSFUL = 265,
     EXHAUSTED = 266,
     STRONG = 267,
     CLASSIFIER = 268,
     CLASSIFIERS = 269,
     UPCASE_STRONG = 270,
     WEAK = 271,
     NULL_ID = 272,
     THRESHOLD = 273,
     LEFT_RIGHT = 274,
     UP_DOWN = 275,
     LEFT_CENTER_RIGHT = 276,
     SEVEN_COLUMNS = 277,
     DIAG = 278,
     LEFT_RIGHT_SAME = 279,
     UP_DOWN_SAME = 280,
     LEFT_CENTER_RIGHT_SAME = 281,
     SEVEN_COLUMNS_SAME = 282,
     SEVEN_COLUMNS_SIMILAR = 283,
     DIAG_SAME = 284,
     DIAG_SIMILAR = 285,
     FOUR_BOXES = 286,
     FOUR_BOXES_SAME = 287,
     DOUBLEQUOTE = 288,
     COMMA = 289,
     X_BY = 290,
     SLASH = 291,
     DOT = 292,
     COLON = 293,
     OPEN_PAREN = 294,
     CLOSE_PAREN = 295,
     OPEN_BRACKET = 296,
     CLOSE_BRACKET = 297,
     ASTERIX = 298,
     LT = 299,
     GTEQ = 300,
     MINUS = 301,
     BRANCH = 302,
     BRANCHES = 303,
     COMMON = 304,
     INT_NUMBER = 305,
     FP_NUMBER = 306,
     NAME = 307
   };
#endif
#define CLASSIFIER_CASCADE 258
#define TYPE_SEQUENTIAL 259
#define TYPE_FAN 260
#define TYPE_TREE 261
#define RATIO 262
#define FPR 263
#define DR 264
#define SUCCESSFUL 265
#define EXHAUSTED 266
#define STRONG 267
#define CLASSIFIER 268
#define CLASSIFIERS 269
#define UPCASE_STRONG 270
#define WEAK 271
#define NULL_ID 272
#define THRESHOLD 273
#define LEFT_RIGHT 274
#define UP_DOWN 275
#define LEFT_CENTER_RIGHT 276
#define SEVEN_COLUMNS 277
#define DIAG 278
#define LEFT_RIGHT_SAME 279
#define UP_DOWN_SAME 280
#define LEFT_CENTER_RIGHT_SAME 281
#define SEVEN_COLUMNS_SAME 282
#define SEVEN_COLUMNS_SIMILAR 283
#define DIAG_SAME 284
#define DIAG_SIMILAR 285
#define FOUR_BOXES 286
#define FOUR_BOXES_SAME 287
#define DOUBLEQUOTE 288
#define COMMA 289
#define X_BY 290
#define SLASH 291
#define DOT 292
#define COLON 293
#define OPEN_PAREN 294
#define CLOSE_PAREN 295
#define OPEN_BRACKET 296
#define CLOSE_BRACKET 297
#define ASTERIX 298
#define LT 299
#define GTEQ 300
#define MINUS 301
#define BRANCH 302
#define BRANCHES 303
#define COMMON 304
#define INT_NUMBER 305
#define FP_NUMBER 306
#define NAME 307




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 130 "CascadeFileParser.yy"
typedef union YYSTYPE {
  int ival;
  double fval;
  char* sval;
  bool bval;
  CIntegralFeature* pIF;
  CRect* pRect;
  CWeakClassifier* pWC;
  CClassifierCascade* pCasc;
} YYSTYPE;
/* Line 1275 of yacc.c.  */
#line 152 "CascadeFileParser.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



