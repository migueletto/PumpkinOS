/*
 * @(#)lex.h
 *
 * Copyright 1997-1999, Wes Cherry   (mailto:wesc@technosis.com)
 *           2000-2004, Aaron Ardiri (mailto:aaron@ardiri.com)
 * All rights reserved.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;  either version 2, or (at your option)
 * any version.
 *
 * This program is distributed in the hope that it will be useful,  but
 * WITHOUT  ANY  WARRANTY;   without  even   the  implied  warranty  of 
 * MERCHANTABILITY  or FITNESS FOR A  PARTICULAR  PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You  should have  received a  copy of the GNU General Public License
 * along with this program;  if not,  please write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Revisions:
 * ==========
 *
 * pre 18-Jun-2000 <numerous developers>
 *                 creation
 *     18-Jun-2000 Aaron Ardiri
 *                 GNU GPL documentation additions
 */

#ifndef __lex__
#define __lex__

typedef int LT;

#define ltNil     -1

/*
 * must be same as nt's 
 */
#define ltConst    0
#define ltId       1
#define ltPlus     2
#define ltMinus    3
#define ltMult     4
#define ltDiv      5
#define ltMod      6

#define ltLT       7
#define ltGT       8
#define ltEQ       9
#define ltLTE      10
#define ltGTE      11
#define ltNE       12
#define ltCaret	   13
#define ltAssign   14
#define ltBSlash   15
#define ltPound    16
#define ltDoubleSlash 17
#define ltSemi     18
#define ltAt       19
#define ltPipe     20

#define ltLParen   42
#define ltRParen   43
#define ltLBracket 44
#define ltRBracket 45
#define ltLBrace   46
#define ltRBrace   47
#define ltComma    48
#define ltQuestion 49
#define ltColon    50
#define ltStr      51
#define ltCComment 52
#define ltEndCComment 53
#define ltPoint    54

typedef int VAL;

typedef enum LEXSIZE
{
  lsUnspecified,
  lsByte,
  lsWord,
  lsLong
}
LEXSIZE;


#define cchIdMax 4096

/*
 * LEXeme 
 */
typedef struct _lex
{
  LT lt;
  char szId[cchIdMax];
  VAL val;
  LEXSIZE size;
}
LEX;

/*
 * Lex function prototypes 
 */
BOOL FInitLexer(const char *pch,
                BOOL fReportErrors);
const char *PchLexerBuffer(void);
BOOL FGetLex(LEX * plex,
             BOOL fInComment);
VOID PrintLex(LEX * plex);

#endif                                           /* __lex__ */
