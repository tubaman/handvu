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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



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




/* Copy the first part of user declarations.  */
#line 62 "CascadeFileParser.yy"

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



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

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
/* Line 191 of yacc.c.  */
#line 258 "CascadeFileParser.cc"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 270 "CascadeFileParser.cc"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   define YYSTACK_ALLOC alloca
#  endif
# else
#  if defined (alloca) || defined (_ALLOCA_H)
#   define YYSTACK_ALLOC alloca
#  else
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  24
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   311

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  53
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  22
/* YYNRULES -- Number of rules. */
#define YYNRULES  46
/* YYNRULES -- Number of states. */
#define YYNSTATES  313

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   307

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    11,    15,    18,    24,    44,
      67,    69,    71,    73,    76,    80,    83,    98,   101,   106,
     109,   111,   114,   135,   138,   140,   144,   153,   155,   157,
     159,   170,   181,   194,   215,   228,   239,   250,   263,   284,
     305,   318,   331,   340,   349,   359,   361
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      54,     0,    -1,    55,    -1,    70,    -1,    56,    57,    63,
      -1,    56,    58,    61,    -1,    56,    59,    -1,     3,    33,
      52,    33,    34,    -1,     4,    34,    50,    35,    50,    34,
       7,    74,    39,     8,    38,    74,    34,     9,    38,    74,
      34,    60,    40,    -1,     5,    34,    50,    35,    50,    34,
       7,    74,    39,     8,    38,    74,    34,     9,    38,    74,
      34,    60,    40,    50,    48,    37,    -1,     6,    -1,    10,
      -1,    11,    -1,    62,    63,    -1,    61,    62,    63,    -1,
      49,    47,    -1,    47,    50,    33,    52,    33,    39,     8,
      38,    74,    34,     9,    38,    74,    40,    -1,    64,    65,
      -1,    50,    12,    14,    37,    -1,    65,    66,    -1,    66,
      -1,    67,    68,    -1,    15,    13,    50,    39,     8,    38,
      74,    34,     9,    38,    74,    40,    38,    50,    16,    14,
      34,    18,    74,    38,    -1,    68,    69,    -1,    69,    -1,
      74,    43,    70,    -1,    41,    72,    42,    71,    74,    39,
      74,    40,    -1,    44,    -1,    45,    -1,    17,    -1,    19,
      50,    34,    50,    35,    50,    34,    50,    34,    50,    -1,
      20,    50,    34,    50,    34,    50,    35,    50,    34,    50,
      -1,    21,    50,    34,    50,    35,    50,    34,    50,    36,
      50,    34,    50,    -1,    22,    50,    34,    50,    35,    50,
      34,    50,    34,    50,    34,    50,    34,    50,    34,    50,
      34,    50,    34,    50,    -1,    23,    50,    34,    50,    34,
      50,    35,    50,    34,    50,    34,    50,    -1,    24,    50,
      34,    50,    35,    50,    34,    50,    34,    50,    -1,    25,
      50,    34,    50,    34,    50,    35,    50,    34,    50,    -1,
      26,    50,    34,    50,    35,    50,    34,    50,    36,    50,
      34,    50,    -1,    27,    50,    34,    50,    35,    50,    34,
      50,    34,    50,    34,    50,    34,    50,    34,    50,    34,
      50,    34,    50,    -1,    28,    50,    34,    50,    35,    50,
      34,    50,    34,    50,    34,    50,    34,    50,    34,    50,
      34,    50,    34,    50,    -1,    29,    50,    34,    50,    34,
      50,    35,    50,    34,    50,    34,    50,    -1,    30,    50,
      34,    50,    34,    50,    35,    50,    34,    50,    34,    50,
      -1,    31,    73,    34,    73,    34,    73,    34,    73,    -1,
      32,    73,    34,    73,    34,    73,    34,    73,    -1,    39,
      50,    34,    50,    34,    50,    34,    50,    40,    -1,    51,
      -1,    50,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   209,   209,   210,   214,   219,   224,   231,   244,   261,
     286,   294,   295,   299,   300,   304,   307,   321,   340,   347,
     348,   352,   366,   390,   391,   395,   403,   415,   416,   420,
     422,   428,   434,   440,   447,   453,   459,   465,   471,   479,
     487,   493,   499,   510,   523,   529,   530
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "CLASSIFIER_CASCADE", "TYPE_SEQUENTIAL",
  "TYPE_FAN", "TYPE_TREE", "RATIO", "FPR", "DR", "SUCCESSFUL", "EXHAUSTED",
  "STRONG", "CLASSIFIER", "CLASSIFIERS", "UPCASE_STRONG", "WEAK",
  "NULL_ID", "THRESHOLD", "LEFT_RIGHT", "UP_DOWN", "LEFT_CENTER_RIGHT",
  "SEVEN_COLUMNS", "DIAG", "LEFT_RIGHT_SAME", "UP_DOWN_SAME",
  "LEFT_CENTER_RIGHT_SAME", "SEVEN_COLUMNS_SAME", "SEVEN_COLUMNS_SIMILAR",
  "DIAG_SAME", "DIAG_SIMILAR", "FOUR_BOXES", "FOUR_BOXES_SAME",
  "DOUBLEQUOTE", "COMMA", "X_BY", "SLASH", "DOT", "COLON", "OPEN_PAREN",
  "CLOSE_PAREN", "OPEN_BRACKET", "CLOSE_BRACKET", "ASTERIX", "LT", "GTEQ",
  "MINUS", "BRANCH", "BRANCHES", "COMMON", "INT_NUMBER", "FP_NUMBER",
  "NAME", "$accept", "ROOT", "cascade", "cascade_header_p1",
  "cascade_header_p2_sequential", "cascade_header_p2_fan",
  "cascade_header_p2_tree", "trainset_exhausted", "branch_seq",
  "branch_header", "strong_classifier_package",
  "strong_classifier_package_header", "strong_classifier_seq",
  "strong_classifier", "strong_classifier_header",
  "weighted_weak_classifier_seq", "weighted_weak_classifier",
  "weak_classifier", "comparison_lt", "integral_feature", "quadruple",
  "fp_number", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    53,    54,    54,    55,    55,    55,    56,    57,    58,
      59,    60,    60,    61,    61,    62,    62,    63,    64,    65,
      65,    66,    67,    68,    68,    69,    70,    71,    71,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    73,    74,    74
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     3,     3,     2,     5,    19,    22,
       1,     1,     1,     2,     3,     2,    14,     2,     4,     2,
       1,     2,    20,     2,     1,     3,     8,     1,     1,     1,
      10,    10,    12,    20,    12,    10,    10,    12,    20,    20,
      12,    12,     8,     8,     9,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,     0,     0,     2,     0,     3,     0,    29,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     1,     0,     0,    10,     0,     0,
       6,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     4,     0,     0,     0,     5,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    27,    28,     0,     0,     0,     0,     0,
      17,    20,     0,     0,    15,     0,    13,     7,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    46,    45,     0,     0,     0,     0,     0,
      19,    21,    24,     0,     0,    14,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    18,     0,    23,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    25,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    26,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    42,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    30,    31,     0,     0,
       0,    35,    36,     0,     0,     0,     0,     0,    44,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    32,     0,    34,    37,     0,
       0,    40,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     8,     0,     0,    33,    38,    39,     0,     0,     0,
       0,     9,    22
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     3,     4,     5,    28,    29,    30,   295,    55,    56,
      51,    52,    80,    81,    82,   111,   112,     6,    75,    23,
      45,   113
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -208
static const short yypact[] =
{
      -1,   -28,   -13,    24,  -208,    16,  -208,   -19,  -208,   -16,
     -15,   -14,   -12,   -11,    -9,    -8,    -7,    -6,    -5,    -4,
      -3,    -2,    -2,     6,  -208,    15,    18,  -208,     3,   -24,
    -208,    21,    23,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,     5,    36,    37,   -18,     8,    22,
      61,  -208,    59,    38,    39,   -24,     3,    41,    40,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    53,    54,
      51,    -2,    -2,  -208,  -208,   -22,    52,    56,    62,    64,
      59,  -208,   -22,    72,  -208,     3,  -208,  -208,    71,    55,
      75,    76,    73,    77,    79,    80,    81,    82,    84,    85,
      70,    87,    89,  -208,  -208,    86,    74,    78,    90,    83,
    -208,   -22,  -208,    88,    91,  -208,    92,    94,    95,    96,
      97,    98,    99,   100,   101,   104,   105,   106,   102,    -2,
      -2,   -22,   103,   107,  -208,    93,  -208,   116,   125,   126,
     124,   127,   128,   129,   131,   133,   135,   136,   137,   138,
     139,   113,   141,   142,   132,   119,   122,   130,  -208,   140,
     134,   143,   144,   145,   146,   147,   148,   151,   152,   153,
     154,   155,   149,    -2,    -2,  -208,   -22,   -22,   168,   169,
     156,   157,   150,   158,   173,   174,   175,   176,   177,   179,
     180,   181,   160,  -208,  -208,   178,   182,   -22,   184,   166,
     170,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   183,   210,   211,   195,   -22,  -208,  -208,   196,   197,
     198,  -208,  -208,   200,   212,   213,   214,   215,  -208,   207,
     216,    69,   217,   202,   203,   205,   206,   208,   209,   218,
     219,   -22,   -22,   222,   121,  -208,   223,  -208,  -208,   227,
     228,  -208,  -208,   229,   230,   -22,   232,   221,   224,   225,
     172,   241,   226,   -22,   231,   233,   238,   235,   239,   240,
     236,   234,   237,   242,   -22,   -22,   243,  -208,   245,   246,
     247,   248,   249,    63,   244,   250,   251,    20,    20,    66,
     252,   254,   255,  -208,  -208,   256,   257,   261,   253,   258,
     259,  -208,   260,    65,  -208,  -208,  -208,   263,   -22,   262,
     264,  -208,  -208
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -208,  -208,  -208,  -208,  -208,  -208,  -208,  -207,  -208,   123,
     -53,  -208,  -208,     2,  -208,  -208,   -27,   -23,  -208,  -208,
     -21,   -75
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned short yytable[] =
{
     105,    46,     1,    86,     8,     7,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      25,    26,    27,    53,    24,    54,    73,    74,   103,   104,
     293,   294,   115,    31,    32,    33,    34,    44,    35,    36,
       2,    37,    38,    39,    40,    41,    42,    43,    47,    48,
     101,   102,    49,    50,    57,    70,   154,    58,    76,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      71,    72,    77,    78,    79,    87,   108,   109,   243,   289,
     297,   296,   110,   308,   136,   100,    84,   106,    83,   117,
      88,   107,    89,    90,    91,    92,    93,    94,    95,    96,
      97,   195,   196,    98,    99,   114,   116,   120,   152,   153,
     118,   119,   121,   122,   158,   123,   124,   125,   126,   127,
     128,   129,   214,   130,   132,   131,   176,   134,   133,   177,
     256,   137,   157,   135,     0,     0,   151,   155,   178,     0,
     232,   156,   139,   138,   140,   141,   142,   143,   144,   145,
     146,   147,   193,   194,   148,   149,   150,     2,   159,   161,
     160,   162,   163,   172,   164,   165,   253,   254,   166,   167,
     168,   169,   175,   170,   171,   173,   174,   198,    85,   179,
     262,   267,     0,   192,   180,     0,   201,     0,   270,     0,
     199,   200,   202,   181,   182,   183,   184,   185,   186,   281,
     282,   187,   188,   189,   190,   191,   197,   203,   204,   205,
     211,   207,   206,   208,   209,   210,   216,   212,   229,   230,
     217,   213,   215,   228,     0,     0,     0,     0,     0,   231,
     233,   234,   235,   310,   236,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   241,   237,   238,   239,   240,
     268,   244,   245,   246,   242,   247,   248,   257,   249,   250,
     255,   258,   259,   260,   261,   271,   269,   272,   251,   252,
     263,   264,   273,   274,   265,   266,   277,   275,   276,   284,
     285,   286,   287,   288,   278,     0,   298,   279,   299,   300,
       0,     0,   280,   283,   290,   303,   301,   302,     0,   311,
     291,   292,   312,   304,     0,     0,     0,     0,   305,   306,
     307,   309
};

static const short yycheck[] =
{
      75,    22,     3,    56,    17,    33,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
       4,     5,     6,    47,     0,    49,    44,    45,    50,    51,
      10,    11,    85,    52,    50,    50,    50,    39,    50,    50,
      41,    50,    50,    50,    50,    50,    50,    50,    42,    34,
      71,    72,    34,    50,    33,    50,   131,    34,    50,    34,
      34,    34,    34,    34,    34,    34,    34,    34,    34,    34,
      34,    34,    50,    12,    15,    34,    14,    13,     9,    16,
      14,   288,    80,    18,   111,    34,    47,    35,    50,    34,
      50,    35,    50,    50,    50,    50,    50,    50,    50,    50,
      50,   176,   177,    50,    50,    33,    35,    34,   129,   130,
      35,    35,    35,    34,   137,    35,    35,    35,    34,    34,
      50,    34,   197,    34,    50,    39,     7,    37,    50,     7,
       9,    43,    39,    50,    -1,    -1,    34,    34,     8,    -1,
     215,    34,    50,    52,    50,    50,    50,    50,    50,    50,
      50,    50,   173,   174,    50,    50,    50,    41,    33,    35,
      34,    34,    34,    50,    35,    34,   241,   242,    35,    34,
      34,    34,    40,    35,    35,    34,    34,     8,    55,    39,
     255,     9,    -1,    34,    50,    -1,    36,    -1,   263,    -1,
      34,    34,    34,    50,    50,    50,    50,    50,    50,   274,
     275,    50,    50,    50,    50,    50,    38,    34,    34,    34,
      50,    34,    36,    34,    34,    34,    50,    39,     8,     8,
      50,    39,    38,    40,    -1,    -1,    -1,    -1,    -1,    34,
      34,    34,    34,   308,    34,    50,    50,    50,    50,    50,
      50,    50,    50,    50,    50,    38,    34,    34,    34,    34,
       9,    34,    50,    50,    38,    50,    50,    34,    50,    50,
      38,    34,    34,    34,    34,    34,    40,    34,    50,    50,
      38,    50,    34,    38,    50,    50,    40,    38,    38,    34,
      34,    34,    34,    34,    50,    -1,    34,    50,    34,    34,
      -1,    -1,    50,    50,    50,    34,    40,    40,    -1,    37,
      50,    50,    38,    50,    -1,    -1,    -1,    -1,    50,    50,
      50,    48
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,    41,    54,    55,    56,    70,    33,    17,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    72,     0,     4,     5,     6,    57,    58,
      59,    52,    50,    50,    50,    50,    50,    50,    50,    50,
      50,    50,    50,    50,    39,    73,    73,    42,    34,    34,
      50,    63,    64,    47,    49,    61,    62,    33,    34,    34,
      34,    34,    34,    34,    34,    34,    34,    34,    34,    34,
      50,    34,    34,    44,    45,    71,    50,    50,    12,    15,
      65,    66,    67,    50,    47,    62,    63,    34,    50,    50,
      50,    50,    50,    50,    50,    50,    50,    50,    50,    50,
      34,    73,    73,    50,    51,    74,    35,    35,    14,    13,
      66,    68,    69,    74,    33,    63,    35,    34,    35,    35,
      34,    35,    34,    35,    35,    35,    34,    34,    50,    34,
      34,    39,    50,    50,    37,    50,    69,    43,    52,    50,
      50,    50,    50,    50,    50,    50,    50,    50,    50,    50,
      50,    34,    73,    73,    74,    34,    34,    39,    70,    33,
      34,    35,    34,    34,    35,    34,    35,    34,    34,    34,
      35,    35,    50,    34,    34,    40,     7,     7,     8,    39,
      50,    50,    50,    50,    50,    50,    50,    50,    50,    50,
      50,    50,    34,    73,    73,    74,    74,    38,     8,    34,
      34,    36,    34,    34,    34,    34,    36,    34,    34,    34,
      34,    50,    39,    39,    74,    38,    50,    50,    50,    50,
      50,    50,    50,    50,    50,    50,    50,    50,    40,     8,
       8,    34,    74,    34,    34,    34,    34,    34,    34,    34,
      34,    38,    38,     9,    34,    50,    50,    50,    50,    50,
      50,    50,    50,    74,    74,    38,     9,    34,    34,    34,
      34,    34,    74,    38,    50,    50,    50,     9,     9,    40,
      74,    34,    34,    34,    38,    38,    38,    40,    50,    50,
      50,    74,    74,    50,    34,    34,    34,    34,    34,    16,
      50,    50,    50,    10,    11,    60,    60,    14,    34,    34,
      34,    40,    40,    34,    50,    50,    50,    50,    18,    48,
      74,    37,    38
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)		\
   ((Current).first_line   = (Rhs)[1].first_line,	\
    (Current).first_column = (Rhs)[1].first_column,	\
    (Current).last_line    = (Rhs)[N].last_line,	\
    (Current).last_column  = (Rhs)[N].last_column)
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if defined (YYMAXDEPTH) && YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 209 "CascadeFileParser.yy"
    {}
    break;

  case 3:
#line 210 "CascadeFileParser.yy"
    {}
    break;

  case 4:
#line 217 "CascadeFileParser.yy"
    {
  }
    break;

  case 5:
#line 222 "CascadeFileParser.yy"
    {
  }
    break;

  case 6:
#line 226 "CascadeFileParser.yy"
    {
  }
    break;

  case 7:
#line 232 "CascadeFileParser.yy"
    {
    if (!parse_entire_cascade) {
      throw ITException("wrong parsing mode set");
    }
    pCascade = new CClassifierCascade();
    pCascade->m_name = yyIDstring;
//    pCascade->m_name = $3;
    curr_branch = -1;
  }
    break;

  case 8:
#line 247 "CascadeFileParser.yy"
    {
    image_width = yyvsp[-16].ival;
    image_height = yyvsp[-14].ival;
    pCascade->m_template_width = yyvsp[-16].ival;
    pCascade->m_template_height = yyvsp[-14].ival;
    pCascade->m_image_area_ratio = yyvsp[-11].fval;
    pCascade->m_structure_type = CClassifierCascade::CASCADE_TYPE_SEQUENTIAL;
    pCascade->m_total_false_positive_rate = yyvsp[-7].fval;
    pCascade->m_last_detection_rate = yyvsp[-3].fval;
    pCascade->m_trainset_exhausted = yyvsp[-1].bval;
  }
    break;

  case 9:
#line 265 "CascadeFileParser.yy"
    {
    image_width = yyvsp[-19].ival;
    image_height = yyvsp[-17].ival;
    pCascade->m_template_width = yyvsp[-19].ival;
    pCascade->m_template_height = yyvsp[-17].ival;
    pCascade->m_image_area_ratio = yyvsp[-14].fval;
    pCascade->m_structure_type = CClassifierCascade::CASCADE_TYPE_FAN;
    pCascade->m_total_false_positive_rate = yyvsp[-10].fval;
    pCascade->m_last_detection_rate = yyvsp[-6].fval;
    pCascade->m_trainset_exhausted = yyvsp[-4].bval;
    int num_branches = yyvsp[-2].ival;
    pCascade->m_branch_classifiers.resize(num_branches);
    pCascade->m_branch_names.resize(num_branches);
    pCascade->m_branch_false_positive_rates.resize(num_branches);
    pCascade->m_branch_detection_rates.resize(num_branches);
    pCascade->m_branch_lyr_false_positive_rates.resize(num_branches);
    pCascade->m_branch_lyr_detection_rates.resize(num_branches);
  }
    break;

  case 10:
#line 287 "CascadeFileParser.yy"
    {
    pCascade->m_structure_type = CClassifierCascade::CASCADE_TYPE_TREE;
    throw ITException("cascade type tree not supported yet");
  }
    break;

  case 11:
#line 294 "CascadeFileParser.yy"
    { yyval.bval = false; }
    break;

  case 12:
#line 295 "CascadeFileParser.yy"
    { yyval.bval = true; }
    break;

  case 15:
#line 305 "CascadeFileParser.yy"
    {
  }
    break;

  case 16:
#line 309 "CascadeFileParser.yy"
    {
    curr_branch = yyvsp[-12].ival;
    if (curr_branch<0 || curr_branch>=(int)pCascade->m_branch_classifiers.size()) {
      throw ITException("branch number out of range");
    }
    pCascade->m_branch_names[yyvsp[-12].ival] = yyIDstring; // $4;
    pCascade->m_branch_false_positive_rates[yyvsp[-12].ival] = yyvsp[-5].fval;
    pCascade->m_branch_detection_rates[yyvsp[-12].ival] = yyvsp[-1].fval;
  }
    break;

  case 17:
#line 323 "CascadeFileParser.yy"
    {
    int declared_num_strong_clsf = yyvsp[-1].ival;
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
    break;

  case 18:
#line 341 "CascadeFileParser.yy"
    {
    yyval.ival = yyvsp[-3].ival;
  }
    break;

  case 21:
#line 354 "CascadeFileParser.yy"
    {
    int delared_num_weak = yyvsp[-1].ival;
    if (delared_num_weak!=curr_strong->GetNumWeakClassifiers()) {
      char buf[1024];
      sprintf(buf, "declared %d weak classifiers, found %d",
              delared_num_weak, curr_strong->GetNumWeakClassifiers());
      throw ITException(buf);
    }
  }
    break;

  case 22:
#line 369 "CascadeFileParser.yy"
    {
    int declared_id = yyvsp[-17].ival;
    int branch_offset = 0;
    if (curr_branch!=-1) {
       branch_offset = pCascade->GetNumStrongClassifiers();
    }
    if (declared_id-branch_offset
          !=pCascade->GetNumStrongClassifiers(curr_branch))
    {
      char buf[1024];
      sprintf(buf, "expected strong classifier number %d, found number %d (branch %d)",
              pCascade->GetNumStrongClassifiers(), yyvsp[-17].ival, curr_branch);
      throw ITException(buf);
    }
    curr_strong = &pCascade->AddStrongClassifier(curr_branch, yyvsp[-13].fval, yyvsp[-9].fval);
    curr_strong->SetAlphasThreshold(yyvsp[-1].fval);
    yyval.ival = yyvsp[-6].ival;
  }
    break;

  case 25:
#line 396 "CascadeFileParser.yy"
    {
    curr_strong->AddWeakClassifier(yyvsp[0].pWC, yyvsp[-2].fval);
    delete yyvsp[0].pWC;
  }
    break;

  case 26:
#line 405 "CascadeFileParser.yy"
    {
    if (parse_entire_cascade) {
      yyval.pWC = new CWeakClassifier(yyvsp[-6].pIF, yyvsp[-4].bval, yyvsp[-3].fval, yyvsp[-1].fval);
    } else {
      parsed_weak = new CWeakClassifier(yyvsp[-6].pIF, yyvsp[-4].bval, yyvsp[-3].fval, yyvsp[-1].fval);
    }
  }
    break;

  case 27:
#line 415 "CascadeFileParser.yy"
    { yyval.bval = true; }
    break;

  case 28:
#line 416 "CascadeFileParser.yy"
    { yyval.bval = false; }
    break;

  case 29:
#line 420 "CascadeFileParser.yy"
    { yyval.pIF = NULL; }
    break;

  case 30:
#line 424 "CascadeFileParser.yy"
    { yyval.pIF = new CLeftRightIF(image_width, image_height,
                          yyvsp[-8].ival, yyvsp[-6].ival, yyvsp[-4].ival, yyvsp[-2].ival, yyvsp[0].ival);
  }
    break;

  case 31:
#line 430 "CascadeFileParser.yy"
    { yyval.pIF = new CUpDownIF(image_width, image_height,
                       yyvsp[-8].ival, yyvsp[-6].ival, yyvsp[-4].ival, yyvsp[-2].ival, yyvsp[0].ival);
  }
    break;

  case 32:
#line 436 "CascadeFileParser.yy"
    { yyval.pIF = new CLeftCenterRightIF(image_width, image_height,
                                yyvsp[-10].ival, yyvsp[-8].ival, yyvsp[-6].ival, yyvsp[-4].ival, yyvsp[-2].ival, yyvsp[0].ival);
  }
    break;

  case 33:
#line 443 "CascadeFileParser.yy"
    { yyval.pIF = new CSevenColumnsIF(image_width, image_height,
                             yyvsp[-18].ival, yyvsp[-16].ival, yyvsp[-14].ival, yyvsp[-12].ival, yyvsp[-10].ival, yyvsp[-8].ival, yyvsp[-6].ival, yyvsp[-4].ival, yyvsp[-2].ival, yyvsp[0].ival);
  }
    break;

  case 34:
#line 449 "CascadeFileParser.yy"
    { yyval.pIF = new CDiagIF(image_width, image_height,
                     yyvsp[-10].ival, yyvsp[-8].ival, yyvsp[-6].ival, yyvsp[-4].ival, yyvsp[-2].ival, yyvsp[0].ival);
  }
    break;

  case 35:
#line 455 "CascadeFileParser.yy"
    { yyval.pIF = new CLeftRightSameIF(image_width, image_height,
                              yyvsp[-8].ival, yyvsp[-6].ival, yyvsp[-4].ival, yyvsp[-2].ival, yyvsp[0].ival);
  }
    break;

  case 36:
#line 461 "CascadeFileParser.yy"
    { yyval.pIF = new CUpDownSameIF(image_width, image_height,
                           yyvsp[-8].ival, yyvsp[-6].ival, yyvsp[-4].ival, yyvsp[-2].ival, yyvsp[0].ival);
  }
    break;

  case 37:
#line 467 "CascadeFileParser.yy"
    { yyval.pIF = new CLeftCenterRightSameIF(image_width, image_height,
                                    yyvsp[-10].ival, yyvsp[-8].ival, yyvsp[-6].ival, yyvsp[-4].ival, yyvsp[-2].ival, yyvsp[0].ival);
  }
    break;

  case 38:
#line 474 "CascadeFileParser.yy"
    { yyval.pIF = new CSevenColumnsSameIF(image_width, image_height,
                                 yyvsp[-18].ival, yyvsp[-16].ival,
                                 yyvsp[-14].ival, yyvsp[-12].ival, yyvsp[-10].ival, yyvsp[-8].ival, yyvsp[-6].ival, yyvsp[-4].ival, yyvsp[-2].ival, yyvsp[0].ival);
  }
    break;

  case 39:
#line 482 "CascadeFileParser.yy"
    { yyval.pIF = new CSevenColumnsSimilarIF(image_width, image_height,
                                    yyvsp[-18].ival, yyvsp[-16].ival,
                                    yyvsp[-14].ival, yyvsp[-12].ival, yyvsp[-10].ival, yyvsp[-8].ival, yyvsp[-6].ival, yyvsp[-4].ival, yyvsp[-2].ival, yyvsp[0].ival);
  }
    break;

  case 40:
#line 489 "CascadeFileParser.yy"
    { yyval.pIF = new CDiagSameIF(image_width, image_height,
                         yyvsp[-10].ival, yyvsp[-8].ival, yyvsp[-6].ival, yyvsp[-4].ival, yyvsp[-2].ival, yyvsp[0].ival);
  }
    break;

  case 41:
#line 495 "CascadeFileParser.yy"
    { yyval.pIF = new CDiagSimilarIF(image_width, image_height,
                            yyvsp[-10].ival, yyvsp[-8].ival, yyvsp[-6].ival, yyvsp[-4].ival, yyvsp[-2].ival, yyvsp[0].ival);
  }
    break;

  case 42:
#line 501 "CascadeFileParser.yy"
    {
    yyval.pIF = new CFourBoxesIF(image_width, image_height,
                          yyvsp[-6].pRect, yyvsp[-4].pRect, yyvsp[-2].pRect, yyvsp[0].pRect);
    delete yyvsp[-6].pRect;
    delete yyvsp[-4].pRect;
    delete yyvsp[-2].pRect;
    delete yyvsp[0].pRect;
  }
    break;

  case 43:
#line 512 "CascadeFileParser.yy"
    {
    yyval.pIF = new CFourBoxesSameIF(image_width, image_height,
                              yyvsp[-6].pRect, yyvsp[-4].pRect, yyvsp[-2].pRect, yyvsp[0].pRect);
    delete yyvsp[-6].pRect;
    delete yyvsp[-4].pRect;
    delete yyvsp[-2].pRect;
    delete yyvsp[0].pRect;
  }
    break;

  case 44:
#line 525 "CascadeFileParser.yy"
    { yyval.pRect = new CRect(yyvsp[-7].ival, yyvsp[-5].ival, yyvsp[-3].ival, yyvsp[-1].ival); }
    break;

  case 45:
#line 529 "CascadeFileParser.yy"
    { yyval.fval = yyvsp[0].fval; }
    break;

  case 46:
#line 530 "CascadeFileParser.yy"
    { yyval.fval = (double) yyvsp[0].ival; }
    break;


    }

/* Line 1000 of yacc.c.  */
#line 1710 "CascadeFileParser.cc"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {
		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
		 yydestruct (yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
	  yydestruct (yytoken, &yylval);
	  yychar = YYEMPTY;

	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}



