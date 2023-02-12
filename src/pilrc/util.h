/*
 * @(#)util.h
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

#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>
#include <stdarg.h>
#include "std.h"

/*
 * RMa add : Debug mode define 
 */
#ifdef _DEBUG
#define		HEXOUT		1
#endif

#ifdef __GNUC__
#define PRINTF_FORMATTING  __attribute__ ((format(printf, 1, 2)))
#else
#define PRINTF_FORMATTING
#endif

/* Error() and ErrorLine() write a message to stderr and halt translation;
   WarningLine() writes to stderr and returns.  ErrorLine() and WarningLine()
   add an indication of the current source file and line number to the message
   given via the printf format etc arguments.  */

VOID Error(const char *szFormat, ...)  PRINTF_FORMATTING;
VOID ErrorLine(const char *szFormat, ...)    PRINTF_FORMATTING;
VOID WarningLine(const char *szFormat, ...)  PRINTF_FORMATTING;

/* The functions above are actually wrappers around this general purpose
   diagnostic function.  Generally there's no need to call this directly,
   but in any case: fError distinguishes between error/warning, and if
   position or position->szFilename is NULL then it's a non-localised
   diagnostic, as for Error().  */

typedef struct FILELINE
{
  char *szFilename;
  FILE *fh;
  int line;
}
FILELINE;

VOID Diagnostic(BOOL fError, const FILELINE *position,
                const char *szFormat, va_list *args);

/*lint -function(exit,Error) */
/*lint -function(exit,ErrorLine) */
/*lint -function(exit,Diagnostic) */

BOOL FSzEqI(const char *sz1,
            const char *sz2);
int WMin(int w1,
         int w2);
int WMax(int w1,
         int w2);

VOID EmitB(BYTE b);
VOID EmitW(unsigned short w);
VOID EmitL(unsigned long l);

VOID intstrncpy(p_int * dst,
                const char *src,
                int n);

/* Returns a pointer (which should be freed by the caller) to memory allocated
   for a string constructed according to the somewhat printf-style formatted
   arguments.  The following sequences in szFormat are expanded:
	%s  inserts the corresponding char* argument;
	%e  considers the corresponding char* argument to be an extension
	    to be removed if the output string so far ends thus;
	/   inserts / or \ as appropriate if the output string so far is
	    non-empty and doesn't already end in a directory separator.  */
char *MakeFilename(const char *szFormat, ...);

/* Returns a pointer (which should be freed by the caller) to memory allocated
   for a temporary filename (as returned by tmpnam()) string.  Don't count on
   rename() succeeding with this file: it's quite likely to be on a different
   filesystem from the one you'd like it to be on.  */
char *MakeTempFilename(void);

VOID OpenOutput(char *szBase,
                int id);
VOID CloseOutput(void);
FILE *getOpenedOutputFile(void);

VOID SetOutFileDir(const char *sz);

VOID OpenResDBFile(const char *szFile);
VOID CloseResDBFile(void);

VOID OpenResFile(const char *szFile);
VOID CloseResFile(void);
VOID DumpBytes(void *pv,
               int cb);
VOID PadBoundary(void);
VOID PadWordBoundary(void);
int IbOut(void);
void AddAccessPath(const char *path);
void FreeAccessPathsList(void);

/* Returns a pointer (which should be freed by the caller) to the full path
   of the file found, and a file handle (which should be closed by the caller)
   in returnFile.  */
char *FindAndOpenFile(const char *szIn, const char *mode, FILE **returnFile);

void InitDependsList(void);
void AddEntryToDependsList(const char *filename);
void OutputDependsList(FILE *dependsFile, const char *szDependsTarget);
void FreeDependsList(void);

extern char rgbZero[];

#ifdef CW_PLUGIN
// XXX ncr
#undef feof
#define feof(f)    MyFeof(f)
#endif

#endif
