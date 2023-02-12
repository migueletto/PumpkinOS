/*
 * @(#)lex.c
 *
 * Copyright 1997-1999, Wes Cherry   (mailto:wesc@technosis.com)
 *           2000-2005, Aaron Ardiri (mailto:aaron@ardiri.com)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define NOPILOTINC
#include "pilrc.h"

static int wBaseCur = 10;

/* The lexer state needs to be saved and restored when the input stream
   changes (due to #include, for example).  */
#define pchLex (vIn.pchLex)

static int commentDepth;

BOOL
FInitLexer(const char *pch,
           BOOL fMarkErrors)
{
  pchLex = pch;
  return fTrue;
}

static BOOL
FSkipWhite(void)
{
  if (pchLex == NULL)
    return fFalse;
  while (*pchLex == ' ' || *pchLex == '\t' || *pchLex == '\n'
         || *pchLex == '\r')
    pchLex++;
  return (*pchLex != '\000');
}

const char *
PchLexerBuffer(void)
{
  FSkipWhite();
  return pchLex;
}

/***    Allow 12345678LU, for instance (should be only in .h or .hpp?!?) ***/
static void
AllowLUAtEndOfConstant(int ch)
{
  /* ISO C allows one of {u,U} optionally followed by one of {l,ll,L,LL},
     or vice versa.  We allow anything matching [uUlL]*, which is much less
     strict.  This is okay because we'll ignore this integer-suffix anyway,
     and any runaway parsing will quickly be stopped by whitespace.  */

  while (tolower(ch) == 'u' || tolower(ch) == 'l')
    ch = *pchLex++;
}

static BOOL
FParseHex(LEX * plex,
          int ch)
{
	ch = tolower(ch);
	if ((ch == '0') && ((*(pchLex) == 'x') || (*(pchLex) == 'X')))
	{
		pchLex++;
		ch = *pchLex++;
	}
	if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f'))
	{
		plex->lt = ltConst;
		plex->val = 0;
		while ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f'))
		{
			plex->val *= 16;

			if ((ch >= '0' && ch <= '9'))
				plex->val += ch - '0';
			else
				plex->val += ch - 'a' + 10;
			ch = *pchLex++;
			ch = tolower(ch);
		}

		plex->size = lsUnspecified;
		if (ch == '.')
		{
			if (*pchLex == 'b' || *pchLex == 'B')
			{
				plex->size = lsByte;
				pchLex += 2;
			}
			else if (*pchLex == 'w' || *pchLex == 'W')
			{
				plex->size = lsWord;
				pchLex += 2;
			}
			else if (*pchLex == 'l' || *pchLex == 'L')
			{
				plex->size = lsLong;
				pchLex += 2;
			}
		}

		AllowLUAtEndOfConstant(ch);
		return fTrue;
	}
	return fFalse;
}

static BOOL
FParseConst(LEX * plex,
            int ch)
{
  char *pchStore;

  pchStore = plex->szId;
  Assert(wBaseCur == 10);
  if ((ch >= '0' && ch <= '9') || ch == '.')
  {
    plex->lt = ltConst;
    plex->val = 0;
    if ((ch == '0') && ((*(pchLex) == 'x') || (*(pchLex) == 'X')))
    {
      *pchStore++ = *pchLex++;
      *pchStore = (char)ch;
      ch = *pchLex++;
      return FParseHex(plex, ch);
    }

    while (ch >= '0' && ch <= '9')
    {
      plex->val *= 10;
      plex->val += ch - '0';
      *pchStore++ = (char)ch;
      ch = *pchLex++;
    }

    plex->size = lsUnspecified;
    if (ch == '.')
    {
      if (*pchLex == 'b' || *pchLex == 'B')
      {
       plex->size = lsByte;
       pchLex += 2;
      }
      else if (*pchLex == 'w' || *pchLex == 'W')
      {
       plex->size = lsWord;
       pchLex += 2;
      }
      else if (*pchLex == 'l' || *pchLex == 'L')
      {
       plex->size = lsLong;
       pchLex += 2;
      }
    }
  }
  else if (ch == '\'')
  {
    int cc;

    plex->lt = ltConst;
    plex->val = 0;
    *pchStore++ = (char)ch;
    for (cc = 0; cc < 4; ++cc)
    {
      ch = (BYTE) * pchLex++;
      *pchStore++ = (char)ch;

            /***    printf("char=[%c]\n", ch); ***/
      if (ch == '\'')
        break;

      if (ch < ' ')
        ErrorLine("Unknown character in '' constant: %c", ch);

      plex->val *= 256;
      plex->val += ch;                           /* high-byte first as a guess */
    }

        /***    Compensate for when we got a full 4 characters ***/
    if (ch != '\'')
      ch = (BYTE) * pchLex++;

    ++pchLex;                                    /* compensate for later -- by caller */
    if (ch != '\'')
      ErrorLine("Unknown '' constant terminator: %c", ch);
  }
  else
  {
    *pchStore = 0;
    return fFalse;
  }

    /***    Note: 'pchLex' is now one past the character that's in 'ch' - the next character to parse ***/
  AllowLUAtEndOfConstant(ch);

  *pchStore = 0;
  return fTrue;
}

static int
ChParseOctal(int ch)
{
  int chVal, cnt;

  chVal = 0;

        /***    Functionality change: 2.01b     bar: only three octal chars! ***/
  for (cnt = 3; cnt-- && ((ch >= '0') && (ch <= '7'));)
  {
    chVal *= 8;
    chVal += ch - '0';
    ch = *pchLex++;
  }
  pchLex--;                                      /* back off to the non-octal digit */
  return chVal;
}

#ifndef _within
#define _within(n, l, h)    (((n) >= (l)) && ((n) <= (h)))
#endif

static int
hexize(int c)
{

/***   convert hex digit to binary                                                 ***/
  if (_within(c, '0', '9'))
    return (c - '0');
  if (_within(c, 'A', 'F'))
    return (c - ('A' - 10));
  if (_within(c, 'a', 'f'))
    return (c - ('a' - 10));

  return (-1);
}

static int
ChParseHex(int ch)
{
  int chVal;

  chVal = 0;
  for (;;)
  {
    int n;

    ch = *pchLex;
    n = hexize(ch);
    if (n < 0)
      break;
    n += (chVal * 16);
    if (n >= 256)
      break;                                     /* Java's \Uxxxx might take more digits (Unicode alert!!!!) */
    ++pchLex;
    chVal = n;
  }

  return chVal;
}

static BOOL
FParseId(LEX * plex,
         int ch)
{
  LEX lex;

  if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
  {
    /*
     * Identifier 
     */
    int cch;

    lex.lt = ltId;
    cch = 0;
    do
    {

      /*
       * if (ch != '"') 
       */
      {
        lex.szId[cch] = (char)ch;                /* gratuitous cast - Unicode alert!!!! */
        cch++;
      }
      ch = *pchLex++;
      if (cch == cchIdMax - 1)
      {
        ErrorLine("Identifier too long");
        break;
      }
    }
    while ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
           || (ch >= '0' && ch <= '9') || ch == '_');
    lex.szId[cch] = '\000';

    *plex = lex;

    plex->size = lsUnspecified;
    if (ch == '.')
    {
      if (*pchLex == 'b' || *pchLex == 'B')
      {
       plex->size = lsByte;
       pchLex += 2;
      }
      else if (*pchLex == 'w' || *pchLex == 'W')
      {
       plex->size = lsWord;
       pchLex += 2;
      }
      else if (*pchLex == 'l' || *pchLex == 'L')
      {
       plex->size = lsLong;
       pchLex += 2;
      }
    }

    return fTrue;
  }
  return fFalse;
}

/***    Skip to end of C comment, if there is an end - else eat the input ***/
static BOOL
FSkipToEndOfCComment(void)
{
  while (*pchLex)
  {
    if ((pchLex[0] == '/') && (pchLex[1] == '*'))
    {
      WarningLine("nested comment");
      commentDepth++;
      pchLex += 2;
      return (fFalse);
    }

    if ((pchLex[0] == '*') && (pchLex[1] == '/'))
    {
      commentDepth--;
      pchLex += 2;
      if (commentDepth)
        return (fFalse);                         /* end of nested comment */
      else
        return (fTrue);                          /* true end of comment */
    }
    pchLex++;
  }

  return (fFalse);
}

#define SLT(ch, ltArg) case ch: lex.lt = ltArg; break;

BOOL
FGetLex(LEX * plex,
        BOOL fInComment)
{
  int ch;
  LEX lex;
  char *pchStore;

  lex.lt = plex->lt = ltNil;
  if (!FSkipWhite())
    return fFalse;

  if (fInComment)
  {
    if (!FSkipToEndOfCComment())
      lex.lt = ltCComment;                       /* keep going with it */
    else
      lex.lt = ltEndCComment;                    /* ok, the comment is over */
  }
  else
  {
    pchStore = lex.szId;
    ch = *pchStore++ = *pchLex++;

    *pchStore = 0;
    switch (ch)
    {
        /*
         * BUG! could use a lookup table... 
         */
        /*
         * TODO logical operators 
         */
        SLT(0xA0, ltConst)                       /* RMa add to support TRAP special case */
      SLT('.', ltPoint) SLT('+', ltPlus) SLT('-', ltMinus) SLT('*', ltMult) SLT('%', ltMod) SLT('(', ltLParen) SLT(')', ltRParen) SLT('[', ltLBracket) SLT(']', ltRBracket) SLT('{', ltLBrace) SLT('}', ltRBrace) SLT(',', ltComma) SLT('?', ltQuestion) SLT(':', ltColon) SLT('^', ltCaret) SLT('\\', ltBSlash) SLT('#', ltPound) SLT('@', ltAt) SLT(';', ltSemi) SLT('|', ltPipe) case '/':
        if (*pchLex == '/')
        {
          *pchStore++ = *pchLex++;
          *pchStore = 0;
          lex.lt = ltDoubleSlash;
        }
        else if (*pchLex == '*')
        {
          commentDepth = 1;
          pchLex++;
          if (!FSkipToEndOfCComment())
            lex.lt = ltCComment;
          else
            lex.lt = ltEndCComment;              /* return place holder token */
        }
        else
          lex.lt = ltDiv;
        break;
      case '<':
        if (*pchLex == '=')
        {
          *pchStore++ = *pchLex++;
          *pchStore = 0;
          lex.lt = ltLTE;
        }
        else if (*pchLex == '>')
        {
          *pchStore++ = *pchLex++;
          *pchStore = 0;
          lex.lt = ltNE;
        }
        else
          lex.lt = ltLT;
        break;
      case '>':
        if (*pchLex == '=')
        {
          *pchStore++ = *pchLex++;
          *pchStore = 0;
          lex.lt = ltGTE;
        }
        else
          lex.lt = ltGT;
        break;
      case '=':
        if (*pchLex == '=')
        {
          *pchStore++ = *pchLex++;
          *pchStore = 0;
          lex.lt = ltEQ;
        }
        else
          lex.lt = ltAssign;
        break;
      case '"':
        lex.lt = ltStr;
        pchStore = lex.szId;
        while (*pchLex != '"')
        {
          int n, tmp;

          n = (*pfnChkCode) ((const unsigned char *)pchLex, &tmp);
          if (n >= 1)
          {
            while (n-- > 0)
              *pchStore++ = *pchLex++;
          }
          else if (*pchLex == '\\')
          {
            int ch;

            pchLex++;
            ch = *pchLex++;
            switch (ch)
            {
              case 'a':
                ch = '\a';
                break;

              case 'b':
                ch = '\b';
                break;

              case 'f':
                ch = '\f';
                break;

              case 'n':
                ch = '\n';
                break;

              case 'r':
                ch = '\r';
                break;

              case 't':
                ch = '\t';
                break;

              case 'v':
                ch = '\v';
                break;

              case '0':
              case '1':
              case '2':
              case '3':
              case '4':
              case '5':
              case '6':
              case '7':
                ch = ChParseOctal(ch);
                break;

              case 'x':
              case 'X':
                ch = ChParseHex(ch);
                break;

              case '\0':                        /* handle slash at the end of the line  */
                ch = '\\';
                break;

              case 'e':                         /* ESC, thank you Richard Brinkerhoff   */
                ch = '\033';
                break;

              case 'z':                         /* special control z'er                 */
                ch = 'z' & 0x1f;
                break;

              case '_':                         /* special ignore - turns to nothing    */
                ch = '\0';
                break;
            }

                                                /***    This program does not handle nulls in strings ***/
            if (ch)
              *pchStore++ = (char)ch;            /* gratuitous cast - Unicode alert!!!! */
          }
          else
            *pchStore++ = *pchLex++;
          if (pchStore - lex.szId == cchIdMax - 1)
          {
            ErrorLine("String too long");
            break;
          }
          if (*pchLex == 0)
          {
            ErrorLine("Unterminated string");
            break;
          }
        }
        pchLex++;
        *pchStore = 0;
        break;
      default:
        if (FParseConst(&lex, ch) || FParseId(&lex, ch))
        {
          /*
           * do nuthin...code is easier to read this way 
           */
        }
        else
        {
          ErrorLine("Unknown character: '%c'", ch);
        }
        pchLex--;
        break;
    }
  }

  *plex = lex;
  return lex.lt != ltNil;
}

#define SPLT(lt, sz) case lt: printf(sz); break;

VOID
PrintLex(LEX * plex)
{
  plex = plex;
#ifdef FOO
  switch (plex->lt)
  {
    case ltConst:
      printf("%d ", plex->val);
      break;
    case ltId:
      printf("%s ", plex->szId);
      break;
      SPLT(ltPoint, ".");
      SPLT(ltPlus, "+");
      SPLT(ltMinus, "-");
      SPLT(ltMult, "*");
      SPLT(ltDiv, "/");
      SPLT(ltMod, "%");
      SPLT(ltLParen, "(");
      SPLT(ltRParen, ")");
      SPLT(ltLBracket, "[");
      SPLT(ltRBracket, "]");
      SPLT(ltLBrace, "{");
      SPLT(ltRBrace, "}");
      SPLT(ltComma, ",");
      SPLT(ltLT, "<");
      SPLT(ltGT, ">");
      SPLT(ltLTE, "<=");
      SPLT(ltGTE, ">=");
      SPLT(ltNE, "<>");
      SPLT(ltEQ, "==");
      SPLT(ltAssign, "=");
      SPLT(ltQuestion, "?");
      SPLT(ltColon, ":");
      SPLT(ltCaret, "^");
  }
#endif
}

/*
 * eof 
 */
