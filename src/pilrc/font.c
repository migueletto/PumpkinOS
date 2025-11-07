/*
 * @(#)font.c
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

// #include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "pilrc.h"

/*
 * Header definitions and token types 
 */

#define h_fontType 0                             /* font type */
#define h_firstChar 1                            /* ASCII code of first character */
#define h_lastChar 2                             /* ASCII code of last character */
#define h_maxWidth 3                             /* maximum character width */
#define h_kernMax 4                              /* negative of maximum character kern */
#define h_nDescent 5                             /* negative of descent */
#define h_fRectWidth 6                           /* width of font rectangle */
#define h_fRectHeight 7                          /* height of font rectangle */
#define h_owTLoc 8                               /* offset to offset/width table */
#define h_ascent 9                               /* ascent */
#define h_descent 10                             /* descent */
#define h_leading 11                             /* leading */
#define h_rowWords 12                            /* row width of bit image in words (16bits) */

#define h_version 13                             /* 1 = PalmNewFontVersion */
#define h_densityCount 14                        /* num of glygh density present */

#define g_glyph 20                               /* new glyph token */
#define g_offset 21                              /* offset token */
#define g_width 22                               /* width token */
#define g_bitmap 99                              /* bitmap data */


#define kArraySize 258

/*
 * some foreign font handing function 
 */
int (*pfnChkCode) (const unsigned char *cp,
                   int *pdx);

/*
 * Some globals to keep track of things for error reporting 
 */

static FILELINE fl;
int vfontType;

/*
 * Global to hold the font widths and offsets 
 */

typedef struct FontCharInfoType
{
  char offset;
  char width;
} FontCharInfoType;

FontCharInfoType *fntOW[256];
unsigned int fntH[256];

static int
IsDBCSNone(const unsigned char *cp,
           int *pdxChar)
{
  return 0;
}

/*-----------------------------------------------------------------------------
|	IsBIG5
|	
|		Check the double byte char is BIG5 coded
-------------------------------------------------------------DLIN------------*/
static int
IsBIG5(const unsigned char *cp,
       int *pdxChar)
{

  /*
   * BIG-5 code rule
   * first  byte range 0x81..0xfe             (high byte)
   * second byte range 0x40..0x7e, 0xa1..0xfe (low  byte)
   * 
   * default symbols range 0xa140..0xa3bf
   * often used word range 0xa440..0xc67e
   * seldom                0xc940..0xf9d5
   * 
   * full typed 0..9       0xa2af..0xa2b8
   * A..Z       0xa2cf..0xa2e8
   * a..z       0xa2e9..0xa343
   * sound symbol b..u     0xa374..0xa3ba
   * sound level .1234     0xa3bb..0xa3bf
   * 
   * full typed space      0xa140
   * cama       0xa141
   * 
   */
  if ((*cp >= 0x81 && *cp <= 0xfe)
      && ((*(cp + 1) >= 0x40 && *(cp + 1) <= 0x7e)
          || (*(cp + 1) >= 0xa1 && *(cp + 1) <= 0xfe)))
  {
    *pdxChar = 13;
    return 2;
  }
  return 0;

}

/*-----------------------------------------------------------------------------
|	IsJapanese
|	
|		Check the double byte char is Japanese coded
-------------------------------------------------------------DLIN------------*/
static int
IsJapanese(const unsigned char *cp,
           int *pdxChar)
{

  if ((*cp >= 0xa1 && *cp <= 0xdf))
  {
    *pdxChar = 5;
    return 1;
  }
  else if ((((*cp >= 0x81) && (*cp <= 0x9f)) ||
            ((*cp >= 0xe0) && (*cp <= 0xef))) &&
           (((*(cp + 1) >= 0x40) && (*(cp + 1) <= 0x7e)) ||
            ((*(cp + 1) >= 0x80) && (*(cp + 1) <= 0xfc))))
  {
    *pdxChar = 9;                                /* not sure about this */
    return 2;
  }
  return 0;

}

/*-----------------------------------------------------------------------------
| IsKoreanHanme
|	
|	Check the double byte char is Korean coded(Large Font, for HanMe)
-----------------------------------------------------------------------------*/
static int
IsKoreanHanme(const unsigned char *cp,
              int *pdxChar)
{

  /*
   * Korean code rule (by JaeMok Jeong)
   * first  byte range 0xb0..0xc8             (high byte)
   * second byte range 0xa1..0xfe             (low  byte)
   */
  if ((*cp >= 0xb0 && *cp <= 0xc8)
      && (*(cp + 1) >= 0xa1 && *(cp + 1) <= 0xfe))
  {
    *pdxChar = 11;                               /* not sure about this, hanme font width */
    return 2;
  }

  return 0;
}

/*-----------------------------------------------------------------------------
| IsKoreanHantip
|	
|	Check the double byte char is Korean coded(Small Font, for Hantiip)
-----------------------------------------------------------------------------*/
static int
IsKoreanHantip(const unsigned char *cp,
               int *pdxChar)
{

  /*
   * Korean code rule (by JaeMok Jeong)
   * first  byte range 0xb0..0xc8             (high byte)
   * second byte range 0xa1..0xfe             (low  byte)
   */
  if ((*cp >= 0xb0 && *cp <= 0xc8)
      && (*(cp + 1) >= 0xa1 && *(cp + 1) <= 0xfe))
  {
    *pdxChar = 8;                                /* hantip font width = 8 */
    return 2;
  }
  return 0;
}

static void
DiagnosticX(BOOL error, const char *szFormat, ...)
{
  va_list args;
  va_start(args, szFormat);
  Diagnostic(error, &fl, szFormat, &args);
  va_end(args);
}

/*
 * Report an error with line number and filename 
 */

static void
ErrorX(const char *s)
{
  DiagnosticX(fTrue, "%s", s);
}

/*
 * Report a warning with line number and filename 
 */

static void
WarningX(const char *s)
{
  DiagnosticX(fFalse, "%s", s);
}

/*
 * Parse out a line and return it's token and value 
 */

static void
ParseLine(char *s,
          int *token,
          int *value)
{
  size_t i, len = strlen(s);
  char *s1;

  *token = g_bitmap;
  *value = 0;

  /*
   * first check for normal "token value" line 
   */
  for (i = 0; i < len; i++)
  {
    s[i] = toupper(s[i]);
    if (s[i] == ' ' || s[i] == 9)
    {
      s[i] = 0;
      if (!strcmp(s, "FONTTYPE"))
        *token = h_fontType;
      if (!strcmp(s, "MAXWIDTH"))
        *token = h_maxWidth;
      if (!strcmp(s, "KERNMAX"))
        *token = h_kernMax;
      if (!strcmp(s, "NDESCENT"))
        *token = h_nDescent;
      if (!strcmp(s, "FRECTWIDTH"))
        *token = h_fRectWidth;
      if (!strcmp(s, "FRECTHEIGHT"))
        *token = h_fRectHeight;
      if (!strcmp(s, "ASCENT"))
        *token = h_ascent;
      if (!strcmp(s, "DESCENT"))
        *token = h_descent;
      if (!strcmp(s, "LEADING"))
        *token = h_leading;
      if (!strcmp(s, "FIRSTCHAR"))
        *token = h_firstChar;
      if (!strcmp(s, "LASTCHAR"))
        *token = h_lastChar;
      if (!strcmp(s, "GLYPH"))
        *token = g_glyph;
      if (!strcmp(s, "OFFSET"))
        *token = g_offset;
      if (!strcmp(s, "WIDTH"))
        *token = g_width;

      if (*token == g_bitmap)
        ErrorX("Invalid Token");
      break;

    }
  }

  /*
   * determe the value 
   */
  if (*token != g_bitmap)
  {
    i++;
    if ((len - i) == 3 && s[i] == 39 && s[i + 2] == 39)
      *value = s[i + 1];
    else
    {
      *value = strtol(s + i, &s1, 10);
      if (s1[0] != 0)
        ErrorX("Invalid Value");
    }
  }

}

/*
 * Ugly routine to call after every glyph 
 */

static void
CloseGlyph(p_int *header,
           int width,
           int *row,
           int *col,
           int autoWidth,
           int autoRectWidth)
{
  if (width > header[h_maxWidth])
  {
    if (autoWidth)
      header[h_maxWidth] = width;
    else
      WarningX("Width exceeds maxWidth definition");
  }

  if (width > header[h_fRectWidth])
  {
    if (autoRectWidth)
      header[h_fRectWidth] = width;
    else
      WarningX("Width exceeds fRectWidth definition");
  }

  if (!header[h_fRectHeight])
    header[h_fRectHeight] = *row;
  if (!(*row) || header[h_fRectHeight] != *row)
    ErrorX("Invalid height on previous glyph");

  *row = 0;
  *col += width;
}

/*
 * The main function of this module 
 */
/*
 * I changed this a little bit to correctly handle fonts with missing char provided.
 * A font is not required to provide all the 256 chars and can even have hole.
 * When a character not defined is displayed, PalmOS uses the "missingChar".
 * The missing char is an additional set after the end of the font.
 * You can specify it (you should actually) in a Font file using GLYPH -1.
 * This should be the last one in the file.
 * Thus you can control what is displayed if something using your font tries to
 * display an unsupported character.
 * 
 * I changed PilRC to correctly handle font definition files containing or not the missing char.
 * 
 * Additionally, the columns array must contain n+1 items including the col past the last char
 * used to compute the size of the last char by some tools. I also fixed that.
 * Regis Nicolas (regis.nicolas@corp.palm.com)
 */

void
DumpFont(const char *pchFileName,
         int fntNo)
{
  char s[kArraySize], *s1, *bitmap[kArraySize];
  unsigned short int coltable[kArraySize];
  size_t x;
  int token, value;
  p_int header[14];
  int curChar = -1;
  int autoWidth = 1;
  int autoRectWidth = 1;
  int row = 0;
  int col = 0;
  int width = 0;
  int missingChar = 0;
  unsigned short bit[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };

  memset(header, 0, sizeof(header));
  memset(bitmap, 0, sizeof(bitmap));

  fl.szFilename = FindAndOpenFile(pchFileName, "rt", &fl.fh);
  fl.line = 0;

  if (fntOW[fntNo])
    free(fntOW[fntNo]);
  if (!(fntOW[fntNo] = malloc(kArraySize * sizeof(FontCharInfoType))))
    Error("Out of memory");
  memset(fntOW[fntNo], -1, kArraySize * sizeof(FontCharInfoType));

  while (fgets(s, sizeof(s), fl.fh))
  {
    fl.line++;

    /*
     * Remove leading and trailing whitespace 
     */
    for (s1 = s; s1[0] == ' ' || s1[0] == 9; s1++) ;
    for (x = strlen(s1); x; x--)
    {
      if (s1[x - 1] == ' ' || s1[x - 1] == 9 || s1[x - 1] == 10
          || s1[x - 1] == 13)
        s1[x - 1] = 0;
      else
        break;
    }

    if (s1[0] && (s[0] != '/' || s[1] != '/'))
    {                                            /* skip blank lines and comment lines */
      ParseLine(s1, &token, &value);
      if (token <= h_leading)
      {
        if (curChar >= 0)
          ErrorX("Header must precede glyphs");
        header[token] = value;
        if (token == h_maxWidth)
          autoWidth = 0;
        if (token == h_fRectWidth)
          autoRectWidth = 0;
        if (token == h_firstChar)
          WarningX("FIRSTCHAR will be overridden based on data");
        if (token == h_lastChar)
          WarningX("LASTCHAR will be overridden based on data");
      }

      /*
       * Do new char processing 
       */
      if (token == g_glyph)
      {
        if (curChar >= 0)
          CloseGlyph(header, width, &row, &col, autoWidth, autoRectWidth);

        if (value == -1)
        {
          /*
           * Handle missing char 
           */
          curChar++;
          fntOW[fntNo][curChar].offset = 0;
          coltable[curChar] = col;
          missingChar = 1;
          continue;
        }
        if (value < 0 || value > 255)
          ErrorX("Glyph number out of range");
        if (value <= curChar)
          ErrorX("Glyph out of sequence");
        if (missingChar != 0)
          ErrorX("GLYPH -1 MUST be the last one in the font definition file");

        for (x = curChar + 1; x <= (size_t) value; x++)
          coltable[x] = col;
        if (curChar < 0)
          header[h_firstChar] = value;
        curChar = value;
        header[h_lastChar] = value;
        fntOW[fntNo][curChar].offset = 0;

        continue;
      }

      if (token == g_offset)
      {
        if (curChar < 0)
          ErrorX("Unexpected OFFSET token");
        fntOW[fntNo][curChar].offset = value;
        continue;
      }

      if (token == g_width)
      {
        if (curChar < 0)
          ErrorX("Unexpected WIDTH token");
        fntOW[fntNo][curChar].width = value;
        continue;
      }

      if (token == g_bitmap)
      {
        if (row == 0)
        {
          width = strlen(s1);
          if (fntOW[fntNo][curChar].width == (char)-1)
            fntOW[fntNo][curChar].width = width;
        }
        else if (width != (int)strlen(s1))
          ErrorX("Invalid width");

        if (!bitmap[row])
        {
          if (!(bitmap[row] = malloc(1024)))
            Error("Out of memory");
          memset(bitmap[row], 0, 1024);

        }

        for (x = 0; x < (size_t) width; x++)
        {
          if (s1[x] != '-' && s1[x] != '.')
          {
            bitmap[row][(col + x) >> 3] |= bit[(col + x) & 0x7];
          }
        }
        if (++row > 255)
          ErrorX("Too many rows");
        continue;
      }

    }

  }
  if (!feof(fl.fh))
    Error("Error reading file: %s", fl.szFilename);

  fclose(fl.fh);

  /*
   * Write the data 
   */

  if (curChar < 0)
    ErrorX("No glyphs");
  if (!missingChar)
  {
    coltable[curChar + 1] = coltable[curChar] + fntOW[fntNo][curChar].width;
    curChar++;
    fntOW[fntNo][curChar].offset = 0;
    fntOW[fntNo][curChar].width = 0;
    /*
     * Always output it 
     */
    missingChar = 1;
  }
  /*
   * Add the extra column info used to compute the last char width (even if it is the missing char) 
   */
  coltable[curChar + 1] = coltable[curChar] + fntOW[fntNo][curChar].width;
  CloseGlyph(header, width, &row, &col, autoWidth, autoRectWidth);
  fntH[fntNo] = header[h_fRectHeight];

  header[h_rowWords] = (col + 15) / 16;
  header[h_owTLoc] =
    header[h_rowWords] * header[h_fRectHeight] + 7 + header[h_lastChar] -
    header[h_firstChar] + missingChar;

#ifdef DEBUG_FONT
  fprintf(stderr, "\nfontType    = %04X (%d)\n" \
                  "firstChar   = %d\n" \
                  "lastChar    = %d\n" \
                  "maxWidth    = %d\n" \
                  "kernMax     = %d\n" \
                  "nDescent    = %d\n" \
                  "fRectWidth  = %d\n" \
                  "fRectHeight = %d\n" \
                  "owTLoc      = %d\n" \
                  "ascent      = %d\n" \
                  "descent     = %d\n" \
                  "leading     = %d\n" \
                  "rowWords    = %d\n\n", \
      header[h_fontType], header[h_fontType], header[h_firstChar], header[h_lastChar],
      header[h_maxWidth], header[h_kernMax], header[h_nDescent], header[h_fRectWidth],
      header[h_fRectHeight], header[h_owTLoc], header[h_ascent], header[h_descent],
      header[h_leading], header[h_rowWords]);
#endif

  // dump font header
  CbEmitStruct(header, szRCFONT, NULL, fTrue);

  // dump Glyph
  for (x = 0; x < (size_t) header[h_fRectHeight]; x++)
  {
    DumpBytes(bitmap[x], header[h_rowWords] * 2);
    free(bitmap[x]);
  }

  // dump Columns
  for (x = header[h_firstChar];
       x <= (size_t) header[h_lastChar] + 1 + missingChar; x++)
    EmitW(coltable[x]);

  // dump CharInfoTag
  DumpBytes(&fntOW[fntNo][header[h_firstChar]],
            (header[h_lastChar] - header[h_firstChar] + 1 + missingChar) * 2);

/*
  for (x = 0; x < header[h_lastChar] - header[h_firstChar] + 1 + missingChar; x++) {
    uint16_t w = fntOW[fntNo][header[h_firstChar] + x].offset;
    w = (w << 8) | fntOW[fntNo][header[h_firstChar] + x].width;
    EmitW(w);
  }
*/

  free(fl.szFilename);
}


/*
 * The main function of this module
 *
 *
 */

void
DumpFontFamily( int fntNo, int version, unsigned int densityCount, FNTFAMDEF * fontFamilyEntries)
{
  char s[kArraySize], *s1, *bitmap[kArraySize];
  unsigned short int coltable[kArraySize];
  size_t x;
  int token, value;
  p_int header[15];
  unsigned short bit[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };
  unsigned int counter = 0;
  FNTFAMDEF * tmpFontFamilyEntries = fontFamilyEntries;
  const FNTFAMDEF *firstEntry = fontFamilyEntries;
  unsigned int aGlyphBitsOffset = 0;

  FontCharInfoType *singleOW = NULL;
  unsigned int singleH = 0;

  if (densityCount < 2)
    ErrorLine("FontFamily must have 2 fonts with density 72 and 144.");

  do
  {
    int curChar = -1;
    int autoWidth = 1;
    int autoRectWidth = 1;
    int row = 0;
    int col = 0;
    int width = 0;
    int missingChar = 0;
    int headerSize = 0;

    memset(header, 0, sizeof(header));
    memset(bitmap, 0, sizeof(bitmap));

    fl.szFilename = FindAndOpenFile(fontFamilyEntries->pchFileName, "rt",
                                    &fl.fh);
    fl.line = 0;

    if (fntOW[fntNo])
      free(fntOW[fntNo]);
    if (!(fntOW[fntNo] = malloc(kArraySize * sizeof(FontCharInfoType))))
      Error("Out of memory");

    memset(fntOW[fntNo], -1, kArraySize * sizeof(FontCharInfoType));

    while (fgets(s, sizeof(s), fl.fh))
    {
      fl.line++;

      /*
       * Remove leading and trailing whitespace 
       */
      for (s1 = s; s1[0] == ' ' || s1[0] == 9; s1++) ;
      for (x = strlen(s1); x; x--)
      {
        if (s1[x - 1] == ' ' || s1[x - 1] == 9 || s1[x - 1] == 10
            || s1[x - 1] == 13)
          s1[x - 1] = 0;
        else
          break;
      }

      if (s1[0] && (s[0] != 47 || s[1] != 47))
      {
        /* skip blank lines and comment lines */
        ParseLine(s1, &token, &value);
        if (token <= h_leading)
        {
          if (curChar >= 0)
            ErrorX("Header must precede glyphs");
          header[token] = value;
          if (token == h_maxWidth)
            autoWidth = 0;
          if (token == h_fRectWidth)
            autoRectWidth = 0;
        }

        /*
         * Do new char processing 
         */
        if (token == g_glyph)
        {
          if (curChar >= 0)
            CloseGlyph(header, width, &row, &col, autoWidth, autoRectWidth);

          if (value == -1)
          {
            /*
             * Handle missing char 
             */
            curChar++;
            fntOW[fntNo][curChar].offset = 0;
            coltable[curChar] = col;
            missingChar = 1;
            continue;
          }

          if (value < 0 || value > 255)
            ErrorX("Glyph number out of range");
          if (value <= curChar)
            ErrorX("Glyph out of sequence");
          if (missingChar != 0)
            ErrorX("GLYPH -1 MUST be the last one in the font definition file");

          for (x = curChar + 1; x <= (size_t) value; x++)
            coltable[x] = col;
          if (curChar < 0)
            header[h_firstChar] = value;
          curChar = value;
          header[h_lastChar] = value;
          fntOW[fntNo][curChar].offset = 0;
          continue;
        }
 
        if (token == g_offset)
        {
          if (curChar < 0)
            ErrorX("Unexpected OFFSET token");
          fntOW[fntNo][curChar].offset = value;
          continue;
        }

        if (token == g_width)
        {
          if (curChar < 0)
            ErrorX("Unexpected WIDTH token");
          fntOW[fntNo][curChar].width = value;
          if (singleOW && value * firstEntry->density != singleOW[curChar].width * fontFamilyEntries->density)
            WarningLine("Widths of '%c' (%d) glyphs not in proportion across different densities", (isprint(curChar))? curChar : '?', curChar);
          continue;
        }

        if (token == g_bitmap)
        {
          if (row == 0)
          {
            width = strlen(s1);
            if (fntOW[fntNo][curChar].width == (char)-1)
            {
              fntOW[fntNo][curChar].width = width;
              if (singleOW && width * firstEntry->density != singleOW[curChar].width * fontFamilyEntries->density)
                WarningLine("Widths of '%c' (%d) glyphs not in proportion across different densities", (isprint(curChar))? curChar : '?', curChar);
            }
          }
          else if (width != (int)strlen(s1))
            ErrorX("Invalid width");

          if (!bitmap[row])
          {
            if (!(bitmap[row] = malloc(1024)))
              Error("Out of memory");
            memset(bitmap[row], 0, 1024);
          }

          for (x = 0; x < (size_t) width; x++)
            if (s1[x] != '-' && s1[x] != '.')
              bitmap[row][(col + x) >> 3] |= bit[(col + x) & 0x7];

          if (++row > 255)
            ErrorX("Too many rows");
          continue;
        }
      }
    }

    if (!feof(fl.fh))
      Error("Error reading file: %s", fl.szFilename);

    fclose(fl.fh);

    /*
     * Write the data 
     */
    if (curChar < 0)
      ErrorX("No glyphs");

    if (!missingChar)
    {
      coltable[curChar + 1] = coltable[curChar] + fntOW[fntNo][curChar].width;
      curChar++;
      fntOW[fntNo][curChar].offset = 0;
      fntOW[fntNo][curChar].width = 0;
      /*
       * Always output it 
       */
      missingChar = 1;
    }

    /*
     * Add the extra column info used to compute the last char width
     * (even if it is the missing char) 
     */

    coltable[curChar + 1] = coltable[curChar] + fntOW[fntNo][curChar].width;
    CloseGlyph(header, width, &row, &col, autoWidth, autoRectWidth);
    fntH[fntNo] = header[h_fRectHeight];
    if (singleOW &&
        fntH[fntNo]*firstEntry->density != singleH*fontFamilyEntries->density)
      WarningLine("Font heights not in proportion across different densities");

    header[h_rowWords] = (col + 15) / 16;
/*
    header[h_owTLoc] =
        header[h_rowWords] * header[h_fRectHeight] + 7 + header[h_lastChar] -
        header[h_firstChar] + missingChar;
*/
    if (vfLE32)
      header[h_owTLoc] = (densityCount * 8) +		// sizeof( RCFONTDENSITYBA32TYPE )= 8
          header[h_lastChar] - header[h_firstChar] + missingChar + 2;
    else
      header[h_owTLoc] = (densityCount * 6) +		// sizeof( RCFONTDENSITYBA16TYPE )= 6
          header[h_lastChar] - header[h_firstChar] + missingChar + 3;

    header[h_version] = version;
    header[h_densityCount] = densityCount;
    header[h_fontType] += 0x200;		// set the fntExtendedFormatMask, high density support

#ifdef DEBUG_FONT
    fprintf(stderr, "\nfontType    = %04X (%d)\n" \
                    "firstChar   = %d\n" \
                    "lastChar    = %d\n" \
                    "maxWidth    = %d\n" \
                    "kernMax     = %d\n" \
                    "nDescent    = %d\n" \
                    "fRectWidth  = %d\n" \
                    "fRectHeight = %d\n" \
                    "owTLoc      = %d\n" \
                    "ascent      = %d\n" \
                    "descent     = %d\n" \
                    "leading     = %d\n" \
                    "rowWords    = %d\n" \
                    "version     = %d\n" \
                    "densityCount= %d\n\n", \
        header[h_fontType], header[h_fontType], header[h_firstChar], header[h_lastChar],
        header[h_maxWidth], header[h_kernMax], header[h_nDescent], header[h_fRectWidth],
        header[h_fRectHeight], header[h_owTLoc], header[h_ascent], header[h_descent],
        header[h_leading], header[h_rowWords], header[h_version], header[h_densityCount]);
#endif

    if (!counter)    // Prepare and dump header
    {
      // dump font family header
      headerSize = CbEmitStruct(header, szRCFONT2, NULL, fTrue);
      if (vfLE32)
      {
        EmitW(0x0000);    // padding, must be set to 0
        headerSize += 2 + (densityCount * 8); // sizeof( RCFONTDENSITYBA32TYPE )= 8
      }
      else
        headerSize += (densityCount * 6);		// sizeof( RCFONTDENSITYBA16TYPE )= 6

      // dump FontDensityType[densityCount]
      for (x = 0; x < densityCount; x++)
      {
        RCFONTDENSITYTYPE aTest;

        SETBAFIELD(aTest, density, tmpFontFamilyEntries->density);
        if (!x)
          aGlyphBitsOffset = headerSize + 2 +
            (((header[h_lastChar] + 1 + missingChar) - (header[h_firstChar] - 1)) * 2) +
            ((header[h_lastChar] - header[h_firstChar] + 1 + missingChar) * 2);
        else
          aGlyphBitsOffset += header[h_fRectHeight] * (header[h_rowWords] * 2);
        SETBAFIELD(aTest, glyphBitsOffset, aGlyphBitsOffset);

        if (vfLE32)
          CbEmitStruct(&(aTest.s32), szRCFONTDENSITYTYPE, NULL, fTrue);
        else
          CbEmitStruct(&(aTest.s16), szRCFONTDENSITYTYPE, NULL, fTrue);
        tmpFontFamilyEntries++;
      }

      // dump Columns
      for (x = header[h_firstChar];
           x <= (size_t) header[h_lastChar] + 1 + missingChar; x++)
        EmitW(coltable[x]);

      // dump CharInfoTag
      DumpBytes(&fntOW[fntNo][header[h_firstChar]],
          (header[h_lastChar] - header[h_firstChar] + 1 + missingChar) * 2);
/*
      for (x = 0; x < header[h_lastChar] - header[h_firstChar] + 1 + missingChar; x++) {
        uint16_t w = fntOW[fntNo][header[h_firstChar] + x].offset;
        w = (w << 8) | fntOW[fntNo][header[h_firstChar] + x].width;
        EmitW(w);
      }
*/

      EmitW(0xFFFF);

      // detach the single density metrics so they won't be overwritten
      singleOW = fntOW[fntNo];
      fntOW[fntNo] = NULL;
      singleH = fntH[fntNo];
    }
 
    // dump Glyph
    for (x = 0; x < (size_t) header[h_fRectHeight]; x++)
    {
      DumpBytes(bitmap[x], header[h_rowWords] * 2);
      if ((fontFamilyEntries->density != 72) && (header[h_rowWords] & 0x0001))
        EmitW(0x0000);
      free(bitmap[x]);
    }

    free(fl.szFilename);

    // next entry
    fontFamilyEntries++;
  }
  while (counter++ < densityCount - 1);

  // add unknown value
  if (vfLE32)
  {
    EmitL( 0);
    EmitL(0xF2000000);
  }

  // Record the single density metrics for future reference
  free(fntOW[fntNo]);
  fntOW[fntNo] = singleOW;
  fntH[fntNo] = singleH;
}



static void
SetBuiltinFont(int fntNo,
               unsigned char *rgbWidths,
               int ichStart,
               int cch)
{
  int ich;

  Assert(ichStart + cch <= 256);
  if (fntOW[fntNo] == NULL)
  {
    if (!(fntOW[fntNo] = malloc(256 * sizeof(FontCharInfoType))))
      Error("Out of memory");
    memset(fntOW[fntNo], -1, 256 * sizeof(FontCharInfoType));
  }
  for (ich = ichStart; ich < ichStart + cch; ich++)
  {
    fntOW[fntNo][ich].width = rgbWidths[ich - ichStart];
  }
}

/*
 * Call to set up memory with standard font data 
 */

void
InitFontMem(int fontType)
{

  static unsigned char font0[] = {
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 2, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 11, 5, 8, 8,
    6, 5, 5, 5, 5, 5, 5, 5,
    2, 2, 4, 8, 6, 8, 7, 2,
    4, 4, 6, 6, 3, 4, 2, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 2, 3, 6, 6, 6, 5,
    8, 5, 5, 5, 6, 4, 4, 6,
    6, 2, 4, 6, 5, 8, 6, 7,
    5, 7, 5, 5, 6, 6, 6, 8,
    6, 6, 6, 3, 5, 3, 6, 4,
    3, 5, 5, 4, 5, 5, 4, 5,
    5, 2, 3, 5, 2, 8, 5, 5,
    5, 5, 4, 4, 4, 5, 5, 6,
    6, 6, 4, 4, 2, 4, 7, 5,
    5, 5, 3, 8, 5, 6, 6, 6,
    4, 11, 5, 4, 8, 10, 10, 10,
    10, 3, 3, 5, 5, 4, 4, 7,
    7, 10, 4, 4, 8, 5, 5, 6,
    2, 2, 6, 6, 8, 6, 2, 5,
    4, 8, 5, 6, 6, 4, 8, 6,
    5, 6, 4, 4, 3, 5, 6, 2,
    4, 2, 5, 6, 8, 8, 8, 5,
    5, 5, 5, 5, 5, 5, 7, 5,
    4, 4, 4, 4, 3, 2, 3, 3,
    7, 6, 7, 7, 7, 7, 7, 5,
    8, 6, 6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5, 5, 8, 4,
    5, 5, 5, 5, 2, 2, 3, 3,
    5, 5, 5, 5, 5, 5, 5, 6,
    7, 5, 5, 5, 5, 6, 5, 6
  };

  static unsigned char font1[] = {
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 2, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 11, 5, 10, 8,
    6, 6, 5, 5, 5, 5, 5, 5,
    2, 3, 6, 10, 6, 13, 9, 3,
    5, 5, 6, 6, 3, 5, 3, 6,
    6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 3, 3, 6, 6, 6, 6,
    10, 7, 7, 6, 7, 5, 5, 8,
    8, 3, 5, 7, 6, 10, 7, 8,
    7, 8, 8, 6, 7, 7, 8, 11,
    7, 7, 7, 4, 6, 4, 6, 5,
    4, 6, 6, 5, 6, 6, 5, 6,
    6, 3, 4, 6, 3, 9, 6, 6,
    6, 6, 5, 5, 6, 6, 6, 9,
    6, 6, 5, 5, 3, 5, 7, 5,
    6, 5, 3, 9, 5, 6, 5, 5,
    4, 17, 6, 5, 10, 10, 10, 10,
    10, 3, 3, 5, 5, 4, 4, 6,
    7, 10, 5, 5, 10, 5, 5, 7,
    2, 3, 6, 7, 8, 7, 3, 6,
    4, 8, 6, 8, 6, 5, 8, 6,
    5, 6, 4, 4, 4, 6, 7, 2,
    4, 2, 6, 8, 9, 9, 9, 6,
    7, 7, 7, 7, 7, 7, 9, 6,
    5, 5, 5, 5, 3, 3, 3, 3,
    8, 7, 8, 8, 8, 8, 8, 6,
    8, 7, 7, 7, 7, 7, 7, 8,
    6, 6, 6, 6, 6, 6, 9, 5,
    6, 6, 6, 6, 3, 3, 3, 3,
    6, 6, 6, 6, 6, 6, 6, 7,
    8, 6, 6, 6, 6, 6, 6, 6
  };

  static unsigned char font2[] = {

    5, 5, 5, 5, 5, 5, 5, 5,
    5, 4, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 11, 5, 10, 8,
    10, 7, 5, 5, 5, 5, 5, 5,
    4, 2, 4, 9, 6, 11, 8, 2,
    4, 4, 6, 8, 3, 4, 2, 5,
    7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 2, 3, 7, 7, 7, 5,
    11, 9, 6, 7, 7, 6, 6, 8,
    7, 3, 4, 7, 5, 10, 7, 8,
    6, 8, 6, 5, 6, 7, 7, 11,
    7, 6, 5, 3, 5, 3, 6, 6,
    3, 6, 7, 6, 7, 7, 4, 7,
    6, 3, 3, 6, 2, 10, 7, 7,
    7, 7, 4, 5, 4, 7, 6, 10,
    6, 7, 6, 4, 2, 4, 7, 5,
    7, 5, 3, 7, 5, 10, 6, 6,
    4, 13, 5, 4, 9, 10, 10, 10,
    10, 3, 3, 5, 5, 5, 6, 12,
    7, 11, 5, 4, 12, 5, 5, 8,
    4, 2, 6, 6, 8, 8, 2, 6,
    4, 10, 5, 7, 7, 4, 10, 4,
    5, 6, 5, 4, 3, 6, 6, 2,
    4, 3, 5, 7, 8, 8, 8, 5,
    9, 9, 9, 9, 9, 9, 9, 7,
    6, 6, 6, 6, 3, 2, 3, 3,
    8, 7, 8, 8, 8, 8, 8, 6,
    8, 7, 7, 7, 7, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 11, 6,
    7, 7, 7, 7, 3, 2, 3, 3,
    6, 7, 7, 7, 7, 7, 7, 6,
    7, 7, 7, 7, 7, 7, 6, 7
  };

  static unsigned char font3[] = {

    5, 5, 5, 11, 10, 10, 10, 11,
    11, 7, 4, 6, 6, 8, 8, 8,
    8, 8, 8, 7, 7, 7, 6, 10,
    10, 10, 10, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5
  };

  static unsigned char font4[] = {

    12, 12, 8, 8, 12, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5
  };

  static unsigned char font5[] = {
    5, 11, 11, 11, 11, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5
  };

  static unsigned char font6[] = {
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 4, 6, 4, 7,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9
  };

  static unsigned char font7[] = {
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 2, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 11, 5, 8, 8,
    9, 6, 5, 5, 5, 5, 5, 5,
    2, 3, 6, 8, 7, 11, 9, 3,
    4, 4, 8, 7, 3, 5, 3, 5,
    6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 3, 4, 7, 6, 7, 6,
    10, 9, 7, 7, 7, 6, 6, 7,
    7, 3, 7, 7, 6, 9, 8, 7,
    7, 8, 7, 7, 7, 7, 8, 10,
    8, 7, 6, 4, 5, 4, 8, 8,
    4, 6, 6, 6, 6, 6, 5, 6,
    6, 3, 4, 6, 3, 9, 6, 6,
    6, 6, 5, 6, 5, 6, 7, 9,
    6, 6, 5, 7, 3, 7, 8, 5,
    6, 5, 3, 7, 6, 9, 6, 6,
    6, 12, 7, 4, 9, 10, 10, 10,
    10, 3, 3, 6, 6, 8, 7, 9,
    7, 8, 6, 4, 9, 5, 5, 7,
    1, 3, 6, 7, 8, 7, 2, 5,
    6, 8, 4, 7, 7, 3, 8, 8,
    5, 7, 4, 4, 4, 7, 8, 3,
    4, 3, 4, 7, 8, 8, 8, 6,
    9, 9, 9, 9, 9, 9, 11, 7,
    6, 6, 6, 6, 4, 4, 5, 4,
    8, 8, 7, 7, 7, 7, 7, 6,
    7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 9, 6,
    6, 6, 6, 6, 4, 4, 5, 4,
    6, 6, 6, 6, 6, 6, 6, 7,
    6, 6, 6, 6, 6, 6, 6, 6
  };

  /*
   * starts at 0xe0 
   */
  static unsigned char font0Hebrew[] = {
    6, 6, 5, 6, 6, 3, 4, 6,
    6, 3, 6, 5, 6, 6, 6, 3,
    4, 6, 6, 6, 6, 6, 6, 6,
    6, 7, 7, 2, 2, 2, 2, 2
  };

  static unsigned char font1Hebrew[] = {
    7, 7, 6, 7, 7, 3, 5, 7,
    8, 3, 6, 6, 6, 7, 8, 3,
    5, 7, 8, 7, 7, 6, 6, 7,
    6, 9, 8, 2, 2, 2, 2, 2
  };

  static unsigned char font2Hebrew[] = {
    7, 7, 6, 7, 7, 4, 6, 7,
    7, 4, 7, 7, 7, 7, 7, 4,
    4, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 2, 2, 2, 2, 2
  };

  /*
   * 0xA8, 0xB8, 0xC0-0xFF
   */

  static unsigned char font0Cyrillic[] = {
    4, 5,
    5, 5, 5, 5, 7, 4, 8, 5,
    5, 5, 6, 6, 8, 6, 7, 5,
    5, 6, 6, 5, 8, 6, 6, 5,
    6, 7, 6, 7, 5, 5, 7, 5,
    5, 5, 5, 4, 6, 5, 8, 5,
    5, 5, 5, 5, 6, 5, 5, 5,
    5, 4, 6, 6, 6, 6, 6, 5,
    6, 7, 6, 7, 5, 5, 7, 5
  };

  static unsigned char font1Cyrillic[] = {
    5, 6,
    7, 6, 6, 6, 8, 5, 9, 6,
    7, 7, 7, 8,10, 8, 8, 8,
    7, 7, 7, 6,11, 7, 8, 8,
    9,10, 7, 9, 6, 8, 9, 7,
    6, 6, 6, 5, 8, 6, 9, 6,
    6, 6, 7, 7, 9, 6, 6, 6,
    6, 5, 5, 6, 8, 6, 7, 6,
    9,10, 7, 9, 6, 6, 9, 6
  };

  static unsigned char font2Cyrillic[] = {
    6, 7,
    9, 6, 6, 6, 8, 6, 8, 6,
    6, 6, 7, 7,10, 6, 8, 6,
    6, 7, 6, 6, 8, 7, 7, 6,
    8, 9, 7, 8, 6, 6, 9, 6,
    6, 6, 6, 5, 7, 7, 6, 6,
    7, 7, 6, 6, 8, 6, 7, 7,
    7, 6, 6, 7, 8, 6, 7, 6,
    8, 9, 7, 8, 6, 6, 8, 6
  };

  static unsigned char font7Cyrillic[] = {
    6, 6,
    9, 7, 7, 7, 9, 6, 11,6,
    8, 8, 7, 8, 9, 7, 7, 7,
    7, 7, 7, 6, 11,8, 7, 7,
    9,10, 8,10, 7, 6,10, 7,
    6, 6, 6, 5, 6, 6, 8, 6,
    6, 6, 6, 7,11, 6, 6, 6,
    6, 6, 7, 6, 9, 6, 7, 6,
    9,10, 7, 9, 6, 6, 9, 6
  };

  /*
   * BUG! font7Hebrew? 
   */

  memset(fntOW, 0, sizeof(fntOW));
  memset(fntH, 0, sizeof(fntH));

  vfontType = fontType;
  pfnChkCode = IsDBCSNone;

  SetBuiltinFont(0, font0, 0, sizeof(font0));
  SetBuiltinFont(1, font1, 0, sizeof(font1));
  SetBuiltinFont(2, font2, 0, sizeof(font2));
  SetBuiltinFont(3, font3, 0, sizeof(font3));
  SetBuiltinFont(4, font4, 0, sizeof(font4));
  SetBuiltinFont(5, font5, 0, sizeof(font5));
  SetBuiltinFont(6, font6, 0, sizeof(font6));
  SetBuiltinFont(7, font7, 0, sizeof(font7));

  fntH[0] = 11;
  fntH[1] = 11;
  fntH[2] = 14;
  fntH[3] = 10;
  fntH[4] = 11;
  fntH[5] = 8;
  fntH[6] = 19;
  fntH[7] = 15;

  switch (fontType)
  {
    case fontChineseBig5:
      pfnChkCode = IsBIG5;
      break;
    case fontChineseGB:
      Error("GB character widths not yet implemented!");
      break;
    case fontJapanese:
      pfnChkCode = IsJapanese;
      break;
    case fontKoreanHanme:
      pfnChkCode = IsKoreanHanme;
      break;
    case fontKoreanHantip:
      pfnChkCode = IsKoreanHantip;
      break;
    case fontHebrew:
      SetBuiltinFont(0, font0Hebrew, 0xe0, sizeof(font0Hebrew));
      SetBuiltinFont(1, font1Hebrew, 0xe0, sizeof(font1Hebrew));
      SetBuiltinFont(2, font2Hebrew, 0xe0, sizeof(font2Hebrew));
      break;

    case fontCyrillic:

      SetBuiltinFont(0, font0Cyrillic,   0xA8, 1 );
      SetBuiltinFont(0, font0Cyrillic+1, 0xB8, 1 );
      SetBuiltinFont(0, font0Cyrillic+2, 0xC0, sizeof(font0Cyrillic)-2);

      SetBuiltinFont(1, font1Cyrillic,   0xA8, 1 );
      SetBuiltinFont(1, font1Cyrillic+1, 0xB8, 1 );
      SetBuiltinFont(1, font1Cyrillic+2, 0xC0, sizeof(font1Cyrillic)-2);

      SetBuiltinFont(2, font2Cyrillic,   0xA8, 1 );
      SetBuiltinFont(2, font2Cyrillic+1, 0xB8, 1 );
      SetBuiltinFont(2, font2Cyrillic+2, 0xC0, sizeof(font2Cyrillic)-2);

      SetBuiltinFont(7, font7Cyrillic,   0xA8, 1 );
      SetBuiltinFont(7, font7Cyrillic+1, 0xB8, 1 );
      SetBuiltinFont(7, font7Cyrillic+2, 0xC0, sizeof(font7Cyrillic)-2);

      break;
  }

}

/*
 * Call to free the memory 
 */

void
FreeFontMem()
{
  int i;

  for (i = 0; i < 256; i++)
    if (fntOW[i])
      free(fntOW[i]);

}

/*-----------------------------------------------------------------------------
|	DxCalcRgdx
|	
|		Get extent of pilot string, placing each char width in rgdx
-------------------------------------------------------------WESC------------*/
int
DxCalcRgdx(unsigned char *sz,
           int ifnt,
           int *rgdx)
{
  unsigned char *pch;
  int dx;

  if (sz == NULL)
    return 0;
  dx = 0;

  for (pch = sz; *pch != 0; pch++)
  {
    if (rgdx != NULL)
      rgdx[pch - sz] = DxChar(*pch, ifnt);
    dx += DxChar(*pch, ifnt);
  }
  return dx;
}

/*-----------------------------------------------------------------------------
|	DxCalcExtent
|	
|		Calc extent of string -- BUG! only works for font 0 (and bold)
-------------------------------------------------------------WESC------------*/
int
DxCalcExtent(unsigned char *sz,
             int ifnt)
{
  unsigned char *pch;
  int dx;

  if (sz == NULL)
    return 0;
  dx = 0;

  for (pch = sz; *pch != 0; pch++)
  {
    int dxChar;
    int cch;

    cch = (*pfnChkCode) (pch, &dxChar);
    if (cch > 0)
    {                                            /* Daniel Lin 99/04/07 */
      dx += dxChar;
      pch += cch - 1;                            /* double byte chars */
    }
    else
      dx += DxChar(*pch, ifnt);
  }
  return dx;
}

int
DxChar(int ch,
       int ifnt)
{
  int dx;

  if (ifnt < 0 || ifnt >= 255)
    ErrorLine("Invalid FontID");
  if (!fntOW[ifnt])
    ErrorLine("Font not defined");
  dx = fntOW[ifnt][ch].width;
  if (dx == -1)
    dx = 0;
  return dx;
}

int
DyFont(int ifnt)
{
  if (ifnt < 0 || ifnt >= 255)
    ErrorLine("Invalid FontID");
  return fntH[ifnt];
}

int
GetFontType()
{
  return vfontType;
}
