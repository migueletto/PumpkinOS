
/*
 * @(#)font.h
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

#ifndef __FONT_H__
#define __FONT_H__

// font types
#define fontDefault      0
#define fontHebrew       1
#define fontJapanese     2
#define fontChineseBig5  3
#define fontChineseGB    4
#define fontKoreanHanme  5
#define fontKoreanHantip 6
#define fontCyrillic     7

typedef struct FNTFAMDEF
{
  char *pchFileName;
  int density;
}
FNTFAMDEF;

extern int (*pfnChkCode) (const unsigned char *cp,
                          int *pdx);

extern void DumpFont(const char *,
                     int);

extern void DumpFontFamily(int fntNo,
                           int version,
                           unsigned int densityCount,
                           FNTFAMDEF * fontFamilyEntries);

extern void InitFontMem(int);
extern void FreeFontMem(void);

extern int DxCalcRgdx(unsigned char *,
                      int,
                      int *);
extern int DxCalcExtent(unsigned char *,
                        int);
extern int DxChar(int,
                  int);
extern int DyFont(int);
extern int GetFontType(void);

#endif
