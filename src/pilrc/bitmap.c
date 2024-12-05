/*
 * @(#)bitmap.c
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
 *     18-Aug-2003 Ben Combee
 *                 Applied Eric Clonniger's fix for 32-bit BMPs
 *        Feb-2007 Alexander Pruss
 *                 LE32 fixes
 */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include "pilrc.h"
#include "bitmap.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_BMP
#define STBI_NO_GIF
#define STBI_NO_JPEG
#define STBI_NO_LINEAR
#include "stb_image.h"

typedef unsigned char PILRC_BYTE;                /* b */
typedef unsigned short PILRC_USHORT;             /* us */
typedef unsigned int PILRC_ULONG;                /* ul */
typedef int bool;                                /* f */

// 
// data structures
// 

/*
 * Microsoft Windows .BMP file format  
 */
// NCR: 16-feb-00 = bitmapfileheader already defined if compiling for windows
#if !defined(CW_PLUGIN) || (CWPLUGIN_HOST != CWPLUGIN_HOST_WIN32)
// define structure using USHORT types to make correct alignment
// more likely -- resolves SourceForge bug 540640

typedef struct BITMAPFILEHEADER
{
  PILRC_USHORT bfType;
  PILRC_USHORT bfSize1;
  PILRC_USHORT bfSize2;
  PILRC_USHORT bfReserved1;
  PILRC_USHORT bfReserved2;
  PILRC_USHORT bfOffBits1;
  PILRC_USHORT bfOffBits2;
} BITMAPFILEHEADER;

typedef struct BITMAPINFOHEADER
{
  PILRC_ULONG biSize;
  PILRC_ULONG biWidth;
  PILRC_ULONG biHeight;
  PILRC_USHORT biPlanes;
  PILRC_USHORT biBitCount;
  PILRC_ULONG biCompression;
  PILRC_ULONG biSizeImage;
  PILRC_ULONG biXPelsPerMeter;
  PILRC_ULONG biYPelsPerMeter;
  PILRC_ULONG biClrUsed;
  PILRC_ULONG biClrImportant;
} BITMAPINFOHEADER;

typedef struct RGBQUAD
{
  PILRC_BYTE rgbBlue;
  PILRC_BYTE rgbGreen;
  PILRC_BYTE rgbRed;
  PILRC_BYTE rgbReserved;
} RGBQUAD;

typedef struct BITMAPINFO
{
  BITMAPINFOHEADER bmiHeader;
  RGBQUAD bmiColors[1]; /* size is indefinite */
} BITMAPINFO;
#endif

#define BI_RGB         0
#define BI_RLE4        1
#define BI_RLE8        2
#define BI_BITFIELDS   3
                                                                // *INDENT-OFF*
/*
 * The 1bit-2 color system palette for Palm Computing Devices.
 */
int PalmPalette1bpp[2][3] = 
{
  { 255, 255, 255}, {   0,   0,   0 }
};

/*
 * The 2bit-4 color system palette for Palm Computing Devices.
 */
int PalmPalette2bpp[4][3] = 
{
  { 255, 255, 255}, { 192, 192, 192}, { 128, 128, 128 }, {   0,   0,   0 }
};

/*
 * The 4bit-16 color system palette for Palm Computing Devices.
 */
int PalmPalette4bpp[16][3] = 
{
  { 255, 255, 255}, { 238, 238, 238 }, { 221, 221, 221 }, { 204, 204, 204 },
  { 187, 187, 187}, { 170, 170, 170 }, { 153, 153, 153 }, { 136, 136, 136 },
  { 119, 119, 119}, { 102, 102, 102 }, {  85,  85,  85 }, {  68,  68,  68 },
  {  51,  51,  51}, {  34,  34,  34 }, {  17,  17,  17 }, {   0,   0,   0 }
};

/*
 * The 4bit-16 color system palette for Palm Computing Devices.
 */
int PalmPalette4bppColor[16][3] = 
{
  { 255, 255, 255}, { 128, 128, 128 }, { 128,   0,   0 }, { 128, 128,   0 },
  {   0, 128,   0}, {   0, 128, 128 }, {   0,   0, 128 }, { 128,   0, 128 },
  { 255,   0, 255}, { 192, 192, 192 }, { 255,   0,   0 }, { 255, 255,   0 },
  {   0, 255,   0}, {   0, 255, 255 }, {   0,   0, 255 }, {   0,   0,   0 }
};

/*
 * The 8bit-256 color system palette for Palm Computing Devices.
 */
int PalmPalette8bpp[256][3] = 
{
  { 255, 255, 255 }, { 255, 204, 255 }, { 255, 153, 255 }, { 255, 102, 255 },
  { 255,  51, 255 }, { 255,   0, 255 }, { 255, 255, 204 }, { 255, 204, 204 }, 
  { 255, 153, 204 }, { 255, 102, 204 }, { 255,  51, 204 }, { 255,   0, 204 }, 
  { 255, 255, 153 }, { 255, 204, 153 }, { 255, 153, 153 }, { 255, 102, 153 }, 
  { 255,  51, 153 }, { 255,   0, 153 }, { 204, 255, 255 }, { 204, 204, 255 },
  { 204, 153, 255 }, { 204, 102, 255 }, { 204,  51, 255 }, { 204,   0, 255 },
  { 204, 255, 204 }, { 204, 204, 204 }, { 204, 153, 204 }, { 204, 102, 204 },
  { 204,  51, 204 }, { 204,   0, 204 }, { 204, 255, 153 }, { 204, 204, 153 },
  { 204, 153, 153 }, { 204, 102, 153 }, { 204,  51, 153 }, { 204,   0, 153 },
  { 153, 255, 255 }, { 153, 204, 255 }, { 153, 153, 255 }, { 153, 102, 255 },
  { 153,  51, 255 }, { 153,   0, 255 }, { 153, 255, 204 }, { 153, 204, 204 },
  { 153, 153, 204 }, { 153, 102, 204 }, { 153,  51, 204 }, { 153,   0, 204 },
  { 153, 255, 153 }, { 153, 204, 153 }, { 153, 153, 153 }, { 153, 102, 153 },
  { 153,  51, 153 }, { 153,   0, 153 }, { 102, 255, 255 }, { 102, 204, 255 },
  { 102, 153, 255 }, { 102, 102, 255 }, { 102,  51, 255 }, { 102,   0, 255 },
  { 102, 255, 204 }, { 102, 204, 204 }, { 102, 153, 204 }, { 102, 102, 204 },
  { 102,  51, 204 }, { 102,   0, 204 }, { 102, 255, 153 }, { 102, 204, 153 },
  { 102, 153, 153 }, { 102, 102, 153 }, { 102,  51, 153 }, { 102,   0, 153 },
  {  51, 255, 255 }, {  51, 204, 255 }, {  51, 153, 255 }, {  51, 102, 255 },
  {  51,  51, 255 }, {  51,   0, 255 }, {  51, 255, 204 }, {  51, 204, 204 },
  {  51, 153, 204 }, {  51, 102, 204 }, {  51,  51, 204 }, {  51,   0, 204 },
  {  51, 255, 153 }, {  51, 204, 153 }, {  51, 153, 153 }, {  51, 102, 153 },
  {  51,  51, 153 }, {  51,   0, 153 }, {   0, 255, 255 }, {   0, 204, 255 },
  {   0, 153, 255 }, {   0, 102, 255 }, {   0,  51, 255 }, {   0,   0, 255 },
  {   0, 255, 204 }, {   0, 204, 204 }, {   0, 153, 204 }, {   0, 102, 204 },
  {   0,  51, 204 }, {   0,   0, 204 }, {   0, 255, 153 }, {   0, 204, 153 },
  {   0, 153, 153 }, {   0, 102, 153 }, {   0,  51, 153 }, {   0,   0, 153 },
  { 255, 255, 102 }, { 255, 204, 102 }, { 255, 153, 102 }, { 255, 102, 102 },
  { 255,  51, 102 }, { 255,   0, 102 }, { 255, 255,  51 }, { 255, 204,  51 },
  { 255, 153,  51 }, { 255, 102,  51 }, { 255,  51,  51 }, { 255,   0,  51 },
  { 255, 255,   0 }, { 255, 204,   0 }, { 255, 153,   0 }, { 255, 102,   0 },
  { 255,  51,   0 }, { 255,   0,   0 }, { 204, 255, 102 }, { 204, 204, 102 },
  { 204, 153, 102 }, { 204, 102, 102 }, { 204,  51, 102 }, { 204,   0, 102 },
  { 204, 255,  51 }, { 204, 204,  51 }, { 204, 153,  51 }, { 204, 102,  51 },
  { 204,  51,  51 }, { 204,   0,  51 }, { 204, 255,   0 }, { 204, 204,   0 },
  { 204, 153,   0 }, { 204, 102,   0 }, { 204,  51,   0 }, { 204,   0,   0 },
  { 153, 255, 102 }, { 153, 204, 102 }, { 153, 153, 102 }, { 153, 102, 102 },
  { 153,  51, 102 }, { 153,   0, 102 }, { 153, 255,  51 }, { 153, 204,  51 },
  { 153, 153,  51 }, { 153, 102,  51 }, { 153,  51,  51 }, { 153,   0,  51 },
  { 153, 255,   0 }, { 153, 204,   0 }, { 153, 153,   0 }, { 153, 102,   0 },
  { 153,  51,   0 }, { 153,   0,   0 }, { 102, 255, 102 }, { 102, 204, 102 },
  { 102, 153, 102 }, { 102, 102, 102 }, { 102,  51, 102 }, { 102,   0, 102 },
  { 102, 255,  51 }, { 102, 204,  51 }, { 102, 153,  51 }, { 102, 102,  51 },
  { 102,  51,  51 }, { 102,   0,  51 }, { 102, 255,   0 }, { 102, 204,   0 },
  { 102, 153,   0 }, { 102, 102,   0 }, { 102,  51,   0 }, { 102,   0,   0 },
  {  51, 255, 102 }, {  51, 204, 102 }, {  51, 153, 102 }, {  51, 102, 102 },
  {  51,  51, 102 }, {  51,   0, 102 }, {  51, 255,  51 }, {  51, 204,  51 },
  {  51, 153,  51 }, {  51, 102,  51 }, {  51,  51,  51 }, {  51,   0,  51 },
  {  51, 255,   0 }, {  51, 204,   0 }, {  51, 153,   0 }, {  51, 102,   0 },
  {  51,  51,   0 }, {  51,   0,   0 }, {   0, 255, 102 }, {   0, 204, 102 },
  {   0, 153, 102 }, {   0, 102, 102 }, {   0,  51, 102 }, {   0,   0, 102 },
  {   0, 255,  51 }, {   0, 204,  51 }, {   0, 153,  51 }, {   0, 102,  51 },
  {   0,  51,  51 }, {   0,   0,  51 }, {   0, 255,   0 }, {   0, 204,   0 },
  {   0, 153,   0 }, {   0, 102,   0 }, {   0,  51,   0 }, {  17,  17,  17 },
  {  34,  34,  34 }, {  68,  68,  68 }, {  85,  85,  85 }, { 119, 119, 119 },
  { 136, 136, 136 }, { 170, 170, 170 }, { 187, 187, 187 }, { 221, 221, 221 },
  { 238, 238, 238 }, { 192, 192, 192 }, { 128,   0,   0 }, { 128,   0, 128 },
  {   0, 128,   0 }, {   0, 128, 128 }, {   0,   0,   0 }, {   0,   0,   0 },
  {   0,   0,   0 }, {   0,   0,   0 }, {   0,   0,   0 }, {   0,   0,   0 },
  {   0,   0,   0 }, {   0,   0,   0 }, {   0,   0,   0 }, {   0,   0,   0 },
  {   0,   0,   0 }, {   0,   0,   0 }, {   0,   0,   0 }, {   0,   0,   0 },
  {   0,   0,   0 }, {   0,   0,   0 }, {   0,   0,   0 }, {   0,   0,   0 },
  {   0,   0,   0 }, {   0,   0,   0 }, {   0,   0,   0 }, {   0,   0,   0 },
  {   0,   0,   0 }, {   0,   0,   0 }, {   0,   0,   0 }, {   0,   0,   0 }
};
#define COLOR_TABLE_SIZE 1026

#define LE_BITMAP_VERSION_MASK 0x80

/*
 * The 4bit-16 color user palette for Palm Computing Devices.
 */
int UserPalette4bpp[16][3];

/*
 * The 8bit-256 color user palette for Palm Computing Devices.
 */
int UserPalette8bpp[256][3];
                                                                // *INDENT-ON*

// 
// local function prototypes
// 

                                                                // *INDENT-OFF*
static PILRC_ULONG LLoadX86(PILRC_ULONG w);
static PILRC_USHORT WLoadX86(PILRC_USHORT w);

static int BMP_RGBToColorIndex(int, int, int, int[][3], int);
static int BMP_GetBits1bpp(BITMAPINFO *, int, PILRC_BYTE *,
                           int, int, int, int *, int *, int *, int *);
static int BMP_GetBits4bpp(BITMAPINFO *, int, PILRC_BYTE *,
                           int, int, int, int *, int *, int *, int *);
static int BMP_GetBits8bpp(BITMAPINFO *, int, PILRC_BYTE *,
                           int, int, int, int *, int *, int *, int *);
static int BMP_GetBits15bpp(BITMAPINFO *, int, PILRC_BYTE *,
                           int, int, int, int *, int *, int *, int *);
static int BMP_GetBits16bpp(BITMAPINFO *, int, PILRC_BYTE *,
                            int, int, int, int *, int *, int *, int *);
static int BMP_GetBits24bpp(BITMAPINFO *, int, PILRC_BYTE *,
                            int, int, int, int *, int *, int *, int *);
static int BMP_GetBits32bpp(BITMAPINFO *, int, PILRC_BYTE *,
                            int, int, int, int *, int *, int *, int *);
static int BMP_GetBits32bppPng(BITMAPINFO *, int, PILRC_BYTE *,
                            int, int, int, int *, int *, int *, int *);

static void BMP_SetBits1bpp(int, PILRC_BYTE *, int, int, int);
static void BMP_SetBits2bpp(int, PILRC_BYTE *, int, int, int, int);
static void BMP_SetBits4bpp(int, PILRC_BYTE *, int, int, int, int);
static void BMP_SetBits8bpp(int, PILRC_BYTE *, int, int, int, int);
static void BMP_SetBits16bpp(int, PILRC_BYTE *, int, int, int, int);
static void BMP_SetBits24bpp(int, PILRC_BYTE *, int, int, int, int);
static void BMP_SetBits32bpp(int, PILRC_BYTE *, int, int, int, int);

static void BMP_ConvertWindowsBitmap(RCBITMAP *, PILRC_BYTE *, long, int, BOOL, int *, int, int);
static void BMP_ConvertTextBitmap(RCBITMAP *, PILRC_BYTE *, int);
static void BMP_ConvertX11Bitmap(RCBITMAP *, PILRC_BYTE *, int);
static void BMP_ConvertPNMBitmap(RCBITMAP *, PILRC_BYTE *, int, int, BOOL, int *);
static void BMP_CompressBitmap(RCBITMAP *, int, int, BOOL, BOOL);
static void BMP_CompressDumpBitmap(RCBITMAP *, int, int, BOOL, BOOL, BOOL, BOOL, int, int *);
static void BMP_InvalidExtension(const char *);

static void BMP_FillBitmapV3Header(RCBITMAP *, RCBITMAP_V3 *, int *, int);
                                                                // *INDENT-ON*

// 
// code
// 

/**
 * Set the 4bpp user palette.
 *
 * @param p palette
 * @param nColors the number of colors
 */
void
SetUserPalette4bpp(int p[][3],
                   int nColors)
{
  int c;

#ifdef BITMAP_CALL_LOG
  if (!vfQuiet)
  {
    printf("SetUserPalette4bpp(");
    for (c = 0; c < nColors; c++)
      printf("%d:%d:%d, ", p[c][0], p[c][1], p[c][2]);
    printf("%d)\n", nColors);
  }
#endif

  nColors = (nColors > 16) ? 16 : nColors;

  // copy colors
  for (c = 0; c < nColors; c++)
  {
    UserPalette4bpp[c][0] = p[c][0];
    UserPalette4bpp[c][1] = p[c][1];
    UserPalette4bpp[c][2] = p[c][2];
  }

  // fill remainder with blacks
  for (c = nColors; c < 16; c++)
  {
    UserPalette4bpp[c][0] = 0;
    UserPalette4bpp[c][1] = 0;
    UserPalette4bpp[c][2] = 0;
  }
}

/**
 * Set the 4bpp user palette to the system palette
 */
void
SetUserPalette4bppToDefault4bpp()
{
#ifdef BITMAP_CALL_LOG
  if (!vfQuiet)
    printf("SetUserPalette4bppToDefault4bpp(): ");
#endif

  SetUserPalette4bpp(PalmPalette4bppColor, 16);
}

/**
 * Set the 8bpp user palette.
 *
 * @param p palette
 * @param nColors the number of colors
 */
void
SetUserPalette8bpp(int p[][3],
                   int nColors)
{
  int c;

#ifdef BITMAP_CALL_LOG
  if (!vfQuiet)
  {
    printf("SetUserPalette8bpp(");
    for (c = 0; c < nColors; c++)
      printf("%d:%d:%d, ", p[c][0], p[c][1], p[c][2]);
    printf("%d)\n", nColors);
  }
#endif

  nColors = (nColors > 256) ? 256 : nColors;

  // copy colors
  for (c = 0; c < nColors; c++)
  {
    UserPalette8bpp[c][0] = p[c][0];
    UserPalette8bpp[c][1] = p[c][1];
    UserPalette8bpp[c][2] = p[c][2];
  }

  // fill remainder with blacks
  for (c = nColors; c < 256; c++)
  {
    UserPalette8bpp[c][0] = 0;
    UserPalette8bpp[c][1] = 0;
    UserPalette8bpp[c][2] = 0;
  }
}

/**
 * Set the 8bpp user palette to the system palette
 */
void
SetUserPalette8bppToDefault8bpp()
{
#ifdef BITMAP_CALL_LOG
  if (!vfQuiet)
    printf("SetUserPalette8bppToDefault8bpp(): ");
#endif

  SetUserPalette8bpp(PalmPalette8bpp, 256);
}

/**
 * Convert a RGB triplet to a index within the PalmPalette256[][] table.
 *
 * @param r the red RGB value
 * @param g the green RGB value
 * @param b the blue RGB value
 * @param palette a pointer to a color pallete array
 * @param paletteSize the number of items in the palette array (triples)
 * @return the index of the RGB triplet in the palette table.
 */
static int
BMP_RGBToColorIndex(int r,
                    int g,
                    int b,
                    int palette[][3],
                    int paletteSize)
{
  int index, lowValue, i, *diffArray;

  // generate the color "differences" for all colors in the palette
  diffArray = (int *)malloc(paletteSize * sizeof(int));
  for (i = 0; i < paletteSize; i++)
  {
    diffArray[i] = ((palette[i][0] - r) * (palette[i][0] - r)) +
      ((palette[i][1] - g) * (palette[i][1] - g)) +
      ((palette[i][2] - b) * (palette[i][2] - b));
  }

  // find the palette index that has the smallest color "difference"

  // OLD ALGORITHM
  // 
  // index = 0;
  // lowValue = diffArray[0];
  // for (i=1; i<paletteSize; i++) {

  // NEW ALGORITHM
  // 
  // by Scott Ludwig - ensures (0,0,0) in 8bpp color is index 255

  index = paletteSize - 1;
  lowValue = diffArray[index];
  for (i = index - 1; i >= 0; i--)
  {
    if (diffArray[i] < lowValue)
    {
      lowValue = diffArray[i];
      index = i;
    }
  }

  // clean up
  free(diffArray);

  return index;
}

/**
 * Convert an unsigned long from x86 to Palm Computing Platform format.
 *
 * @param ul the unsigned long to convert
 * @return the unsigned long in Palm Computing Platform format.
 */
static PILRC_ULONG
LLoadX86(PILRC_ULONG ul)
{
  PILRC_BYTE *pb;
  PILRC_ULONG ulOut;

  pb = (PILRC_BYTE *) & ul;
  ulOut =
    (PILRC_ULONG) ((*(pb + 3) << 24) | (*(pb + 2) << 16) |
                   (*(pb + 1) << 8) | (*pb));

  return ulOut;
}

/**
 * Convert an unsigned short from x86 to Palm Computing Platform format.
 *
 * @param ul the unsigned short to convert
 * @return the unsigned short in Palm Computing Platform format.
 */
static PILRC_USHORT
WLoadX86(PILRC_USHORT w)
{
  PILRC_BYTE *pb;
  PILRC_USHORT wOut;

  pb = (PILRC_BYTE *) & w;
  wOut = (PILRC_USHORT) ((*(pb + 1) << 8) | (*pb));

  return wOut;
}

/**
 * Get the number of bytes each bitmap row requires.
 *
 * @param cx         the width of the bitmap.
 * @param cbpp       the number of bits per byte.
 * @param cBitsAlign the os-dependant byte alignment definition.
 * @return the number of bytes each bitmap row requires.
 */
static int
BMP_CbRow(int cx,
          int cbpp,
          int cBitsAlign)
{
  return ((cx * cbpp + (cBitsAlign - 1)) & ~(cBitsAlign - 1)) >> 3;
}

/**
 * Get a single bit from a 1bpp bitmap 
 *
 * @param pbmi       bitmap information
 * @param cx         the width of the bitmap.
 * @param pb         a reference to the bitmap resource. 
 * @param x          the x-coordinate of the pixel to process.
 * @param y          the y-coordinate of the pixel to process.
 * @param cBitsAlign the os-dependant byte alignment definition.
 * @param a          alpha channel of pixel
 * @param r          red channel of pixel
 * @param g          green channel of pixel
 * @param b          blue channel of pixel
 * @return zero if the bit not set, non-zero otherwise.
 */
static int
BMP_GetBits1bpp(BITMAPINFO * pbmi,
                int cx,
                PILRC_BYTE * pb,
                int x,
                int y,
                int cBitsAlign,
                int *a,
                int *r,
                int *g,
                int *b)
{
  int cbRow;
  int w;

  cbRow = BMP_CbRow(cx, 1, cBitsAlign);
  pb += cbRow * y + (x >> 3);

  w = (*pb & (0x01 << (7 - (x & 7)))) ? 1 : 0;

  // return the values we need
  *a = 0;
  *r = pbmi->bmiColors[w].rgbRed;
  *g = pbmi->bmiColors[w].rgbGreen;
  *b = pbmi->bmiColors[w].rgbBlue;

  return w;
}

/**
 * Set a single bit in a 1bpp bitmap 
 *
 * @param cx         the width of the bitmap.
 * @param pb         a reference to the bitmap resource. 
 * @param x          the x-coordinate of the pixel to process.
 * @param y          the y-coordinate of the pixel to process.
 * @param cBitsAlign the os-dependant byte alignment definition.
 */
static void
BMP_SetBits1bpp(int cx,
                PILRC_BYTE * pb,
                int x,
                int y,
                int cBitsAlign)
{
  int cbRow;

  cbRow = BMP_CbRow(cx, 1, cBitsAlign);
  pb += cbRow * y + (x >> 3);
  if (vfLE32)
    *pb |= (0x01 << (x & 7));
  else
    *pb |= (0x01 << (7 - (x & 7)));
}

/**
 * Set bits in a 2bpp bitmap 
 *
 * @param cx         the width of the bitmap.
 * @param pb         a reference to the bitmap resource. 
 * @param x          the x-coordinate of the pixel to process.
 * @param y          the y-coordinate of the pixel to process.
 * @param bits       the bits to set (0..3).
 * @param cBitsAlign the os-dependant byte alignment definition.
 */
static void
BMP_SetBits2bpp(int cx,
                PILRC_BYTE * pb,
                int x,
                int y,
                int bits,
                int cBitsAlign)
{
  int cbRow;

  cbRow = BMP_CbRow(cx, 2, cBitsAlign);
  pb += cbRow * y + (x >> 2);
  *pb |= (bits << ((3 - (x & 3)) * 2));
}

/**
 * Get bits from a 4bpp bitmap 
 *
 * @param pbmi       bitmap information
 * @param cx         the width of the bitmap.
 * @param pb         a reference to the bitmap resource. 
 * @param x          the x-coordinate of the pixel to process.
 * @param y          the y-coordinate of the pixel to process.
 * @param cBitsAlign the os-dependant byte alignment definition.
 * @param a          alpha channel of pixel
 * @param r          red channel of pixel
 * @param g          green channel of pixel
 * @param b          blue channel of pixel
 * @return zero if the bit not set, non-zero otherwise.
 * @return the bit representation for the (x,y) pixel.
 */
static int
BMP_GetBits4bpp(BITMAPINFO * pbmi,
                int cx,
                PILRC_BYTE * pb,
                int x,
                int y,
                int cBitsAlign,
                int *a,
                int *r,
                int *g,
                int *b)
{
  int cbRow;
  int w;

  cbRow = BMP_CbRow(cx, 4, cBitsAlign);
  pb += cbRow * y + (x >> 1);

  w = ((x & 1) != 0) ? (*pb & 0x0f) : ((*pb & 0xf0) >> 4);

  // return the values we need
  *a = 0;
  *r = pbmi->bmiColors[w].rgbRed;
  *g = pbmi->bmiColors[w].rgbGreen;
  *b = pbmi->bmiColors[w].rgbBlue;

  return w;
}

/**
 * Set bits in a 4bpp bitmap 
 *
 * @param cx         the width of the bitmap.
 * @param pb         a reference to the bitmap resource. 
 * @param x          the x-coordinate of the pixel to process.
 * @param y          the y-coordinate of the pixel to process.
 * @param bits       the bits to set (0..15).
 * @param cBitsAlign the os-dependant byte alignment definition.
 */
static void
BMP_SetBits4bpp(int cx,
                PILRC_BYTE * pb,
                int x,
                int y,
                int bits,
                int cBitsAlign)
{
  int cbRow;

  cbRow = BMP_CbRow(cx, 4, cBitsAlign);
  pb += cbRow * y + (x >> 1);
  *pb |= ((x & 1) != 0) ? bits : (bits << 4);
}

/**
 * Get bits from a 8bpp bitmap 
 *
 * @param pbmi       bitmap information
 * @param cx         the width of the bitmap.
 * @param pb         a reference to the bitmap resource. 
 * @param x          the x-coordinate of the pixel to process.
 * @param y          the y-coordinate of the pixel to process.
 * @param cBitsAlign the os-dependant byte alignment definition.
 * @param a          alpha channel of pixel
 * @param r          red channel of pixel
 * @param g          green channel of pixel
 * @param b          blue channel of pixel
 * @return the bit representation for the (x,y) pixel.
 */
static int
BMP_GetBits8bpp(BITMAPINFO * pbmi,
                int cx,
                PILRC_BYTE * pb,
                int x,
                int y,
                int cBitsAlign,
                int *a,
                int *r,
                int *g,
                int *b)
{
  int cbRow;
  int w;

  cbRow = BMP_CbRow(cx, 8, cBitsAlign);
  pb += cbRow * y + x;

  w = *pb;

  // return the values we need
  *a = 0;
  *r = pbmi->bmiColors[w].rgbRed;
  *g = pbmi->bmiColors[w].rgbGreen;
  *b = pbmi->bmiColors[w].rgbBlue;

  return w;
}

/**
 * Set bits in a 8bpp bitmap 
 *
 * @param cx         the width of the bitmap.
 * @param pb         a reference to the bitmap resource. 
 * @param x          the x-coordinate of the pixel to process.
 * @param y          the y-coordinate of the pixel to process.
 * @param bits       the bits to set (0..255).
 * @param cBitsAlign the os-dependant byte alignment definition.
 */
static void
BMP_SetBits8bpp(int cx,
                PILRC_BYTE * pb,
                int x,
                int y,
                int bits,
                int cBitsAlign)
{
  int cbRow;

  cbRow = BMP_CbRow(cx, 8, cBitsAlign);
  pb += cbRow * y + x;
  *pb = bits;
}

/**
 * Set bits in a 16bpp bitmap 
 *
 * @param cx         the width of the bitmap.
 * @param pb         a reference to the bitmap resource. 
 * @param x          the x-coordinate of the pixel to process.
 * @param y          the y-coordinate of the pixel to process.
 * @param bits       the bits to set (0..65535).
 * @param cBitsAlign the os-dependant byte alignment definition.
 */
static void
BMP_SetBits16bpp(int cx,
                 PILRC_BYTE * pb,
                 int x,
                 int y,
                 int bits,
                 int cBitsAlign)
{
  int cbRow;

  cbRow = BMP_CbRow(cx, 16, cBitsAlign);
  pb += cbRow * y + (x * 2);
  *pb = (PILRC_BYTE) ((bits & 0xFF00) >> 8);     // 5-6-5 (r-g-b) bit
  // layout
  *(pb + 1) = (PILRC_BYTE) (bits & 0xFF);
}

/**
 * Set bits in a 24bpp bitmap 
 *
 * @param cx         the width of the bitmap.
 * @param pb         a reference to the bitmap resource. 
 * @param x          the x-coordinate of the pixel to process.
 * @param y          the y-coordinate of the pixel to process.
 * @param bits       the bits to set (0..65535).
 * @param cBitsAlign the os-dependant byte alignment definition.
 */
static void
BMP_SetBits24bpp(int cx,
                 PILRC_BYTE * pb,
                 int x,
                 int y,
                 int bits,
                 int cBitsAlign)
{
  int cbRow;

  cbRow = BMP_CbRow(cx, 24, cBitsAlign);
  pb += cbRow * y + (x * 3);
  pb[0] = (PILRC_BYTE) ((bits & 0xFF0000) >> 16); // red
  pb[1] = (PILRC_BYTE) ((bits & 0x00FF00) >> 8);  // green
  pb[2] = (PILRC_BYTE) (bits & 0xFF);             // blue
}

/**
 * Set bits in a 32bpp bitmap 
 *
 * @param cx         the width of the bitmap.
 * @param pb         a reference to the bitmap resource. 
 * @param x          the x-coordinate of the pixel to process.
 * @param y          the y-coordinate of the pixel to process.
 * @param bits       the bits to set (0..65535).
 * @param cBitsAlign the os-dependant byte alignment definition.
 */
static void
BMP_SetBits32bpp(int cx,
                 PILRC_BYTE * pb,
                 int x,
                 int y,
                 int bits,
                 int cBitsAlign)
{
  int cbRow;

  cbRow = BMP_CbRow(cx, 32, cBitsAlign);
  pb += cbRow * y + (x * 4);
  pb[0] = (PILRC_BYTE) ((bits & 0xFF000000) >> 24); // alpha
  pb[1] = (PILRC_BYTE) ((bits & 0x00FF0000) >> 16); // red
  pb[2] = (PILRC_BYTE) ((bits & 0x0000FF00) >> 8);  // green
  pb[3] = (PILRC_BYTE) (bits & 0xFF);               // blue
}

/**
 * Get bits from a 15bpp bitmap 
 *
 * @param pbmi       bitmap information
 * @param cx         the width of the bitmap.
 * @param pb         a reference to the bitmap resource. 
 * @param x          the x-coordinate of the pixel to process.
 * @param y          the y-coordinate of the pixel to process.
 * @param cBitsAlign the os-dependant byte alignment definition.
 * @param a          alpha channel of pixel
 * @param r          red channel of pixel
 * @param g          green channel of pixel
 * @param b          blue channel of pixel
 * @return the bit representation for the (x,y) pixel.
 */
static int
BMP_GetBits15bpp(BITMAPINFO * pbmi,
                 int cx,
                 PILRC_BYTE * pb,
                 int x,
                 int y,
                 int cBitsAlign,
                 int *a,
                 int *r,
                 int *g,
                 int *b)
{
  int cbRow;
  int w;

  cbRow = BMP_CbRow(cx, 16, cBitsAlign);
  pb += cbRow * y + (x * 2);

  // get the pixel
  w = (pb[1] << 8) | pb[0];

  // return the values we need
  *a = 0;
  *r = (((w & 0x7C00) >> 7) | ((w & 0x1C00) >> 10));
  *g = (((w & 0x03E0) >> 2) | ((w & 0x00e0) >> 5));
  *b = (((w & 0x001F) << 3) | (w & 0x0007));
  return -1;                                     // no index, direct color
}

/**
 * Get bits from a 16bpp bitmap 
 *
 * @param pbmi       bitmap information
 * @param cx         the width of the bitmap.
 * @param pb         a reference to the bitmap resource. 
 * @param x          the x-coordinate of the pixel to process.
 * @param y          the y-coordinate of the pixel to process.
 * @param cBitsAlign the os-dependant byte alignment definition.
 * @param a          alpha channel of pixel
 * @param r          red channel of pixel
 * @param g          green channel of pixel
 * @param b          blue channel of pixel
 * @return the bit representation for the (x,y) pixel.
 */
static int
BMP_GetBits16bpp(BITMAPINFO * pbmi,
                 int cx,
                 PILRC_BYTE * pb,
                 int x,
                 int y,
                 int cBitsAlign,
                 int *a,
                 int *r,
                 int *g,
                 int *b)
{
  int cbRow;
  int w;

  cbRow = BMP_CbRow(cx, 16, cBitsAlign);
  pb += cbRow * y + (x * 2);

  // get the pixel
  w = (pb[1] << 8) | pb[0];

  // return the values we need
  *a = 0;
  *r = (((w & 0xF800) >> 8) | ((w & 0x3800) >> 11));
  *g = (((w & 0x07E0) >> 3) | ((w & 0x0060) >> 5));
  *b = (((w & 0x001F) << 3) | (w & 0x0007));     // MiR 1st July 2002 was  *b = (((w & 0x001F) >> 3) | (w & 0x0007));
  // which shifts the blue bits the wrong way resulting in color corruption
  // in Bitmap displayed on Palm OS
  // This should be mirror operation of shift carried out in "BMP_ConvertWindowsBitmap()"

  return -1;                                     // no index, direct color
}

/**
 * Get bits from a 24bpp bitmap 
 *
 * @param pbmi       bitmap information
 * @param cx         the width of the bitmap.
 * @param pb         a reference to the bitmap resource. 
 * @param x          the x-coordinate of the pixel to process.
 * @param y          the y-coordinate of the pixel to process.
 * @param cBitsAlign the os-dependant byte alignment definition.
 * @param a          alpha channel of pixel
 * @param r          red channel of pixel
 * @param g          green channel of pixel
 * @param b          blue channel of pixel
 * @return the bit representation for the (x,y) pixel.
 */
static int
BMP_GetBits24bpp(BITMAPINFO * pbmi,
                 int cx,
                 PILRC_BYTE * pb,
                 int x,
                 int y,
                 int cBitsAlign,
                 int *a,
                 int *r,
                 int *g,
                 int *b)
{
  int cbRow;

  cbRow = BMP_CbRow(cx, 24, cBitsAlign);
  pb += cbRow * y + (x * 3);

  // return the values we need
  *b = pb[0];
  *g = pb[1];
  *r = pb[2];
  *a = 0;

  return -1;                                     // no index, direct color
}

/**
 * Get bits from a 32bpp bitmap 
 *
 * @param pbmi       bitmap information
 * @param cx         the width of the bitmap.
 * @param pb         a reference to the bitmap resource. 
 * @param x          the x-coordinate of the pixel to process.
 * @param y          the y-coordinate of the pixel to process.
 * @param cBitsAlign the os-dependant byte alignment definition.
 * @param a          alpha channel of pixel
 * @param r          red channel of pixel
 * @param g          green channel of pixel
 * @param b          blue channel of pixel
 * @return the bit representation for the (x,y) pixel.
 */
static int
BMP_GetBits32bpp(BITMAPINFO * pbmi,
                 int cx,
                 PILRC_BYTE * pb,
                 int x,
                 int y,
                 int cBitsAlign,
                 int *a,
                 int *r,
                 int *g,
                 int *b)
{
  int cbRow;

  cbRow = BMP_CbRow(cx, 32, cBitsAlign);
  pb += cbRow * y + (x * 4);

  // return the values we need
  *b = pb[0];
  *g = pb[1];
  *r = pb[2];
  *a = pb[3];

  return -1;                                     // no index, direct color
}

static int
BMP_GetBits32bppPng(BITMAPINFO * pbmi,
                    int cx,
                    PILRC_BYTE * pb,
                    int x,
                    int y,
                    int cBitsAlign,
                    int *a,
                    int *r,
                    int *g,
                    int *b)
{
  int cbRow;

  cbRow = BMP_CbRow(cx, 32, cBitsAlign);
  pb += cbRow * y + (x * 4);

  // return the values we need
  *r = pb[0];
  *g = pb[1];
  *b = pb[2];
  *a = pb[3];

  return -1;                                     // no index, direct color
}

/**
 * Convert a Microsoft Windows BMP file to Palm Computing resource data.
 *
 * @param rcbmp      a reference to the Palm Computing resource data.
 * @param pbResData  a reference to the bitmap resource. 
 * @param bitmaptype the type of bitmap (B+W, Grey, Grey16 or Color)?
 * @param colortable does a color table need to be generated? 
 * @param transparencyData anything we need to know about transparency
 */
static void
BMP_ConvertWindowsBitmap(RCBITMAP * rcbmp,
                         PILRC_BYTE * pbResData,
                         long size,
                         int bitmaptype,
                         BOOL colortable,
                         int *transparencyData,
                         int density, int png)
{
  PILRC_BYTE *pbSrc;
  int i, x, y, dx, dy, colorDat, icomp;
  int cbRow, cbHeader, cbits, cbitsPel, numClrs;
  BITMAPINFO *pbmi;
  BITMAPINFOHEADER bmi;
  int (*getBits) (BITMAPINFO *,
                  int,
                  PILRC_BYTE *,
                  int,
                  int,
                  int,
                  int *,
                  int *,
                  int *,
                  int *) = NULL;
  int dstPalette[256][3] = { {0, 0, 0} };
  int dstPaletteSize = 0;

  if (png) {
    if (bitmaptype != rwBitmapColor32k)
      ErrorLine("Pilrc supports only 32-bits Bitmaps for PNG files ");

    pbSrc = stbi_load_from_memory(pbResData, size, &dx, &dy, &icomp, 4);

    if (pbSrc == NULL)
      ErrorLine("Pilrc failed to load PNG image ");
    if (icomp != 4)
      ErrorLine("Pilrc supports only 32-bits PNG images ");

    cbits = 32;
    numClrs = 0;
    cbitsPel = -1;
    colorDat = 0;
  } else {
    pbmi = (BITMAPINFO *) (pbResData + sizeof(BITMAPFILEHEADER));
    memcpy(&bmi, pbmi, sizeof(BITMAPINFOHEADER));

    cbHeader = LLoadX86(bmi.biSize);
    dx = LLoadX86(bmi.biWidth);
    dy = LLoadX86(bmi.biHeight);
    cbits = WLoadX86(bmi.biBitCount);
    numClrs = LLoadX86(bmi.biClrUsed);
    if (numClrs == 0)
      numClrs = 1 << cbits;                        // MSPaint DONT set this
    if (numClrs > 256)
      numClrs = 0;                                 // direct color FIX
    pbSrc = ((PILRC_BYTE *) pbmi) + cbHeader + (sizeof(RGBQUAD) * numClrs);
    cbitsPel = -1;
    colorDat = 0;

    if ((bmi.biCompression == BI_RLE4) || (bmi.biCompression == BI_RLE8))
      ErrorLine("Pilrc does not support compressed '.bmp' files ");
  }

  // check the format of the bitmap
  switch (cbits)
  {
    case 1:
      getBits = BMP_GetBits1bpp;
      break;

    case 4:
      getBits = BMP_GetBits4bpp;
      break;

    case 8:
      getBits = BMP_GetBits8bpp;
      break;

    case 15:
      // fall through
    case 16:
      if (bmi.biCompression == BI_BITFIELDS)
      {
        if (*((int *)pbSrc + 1) == 0x3E00)       //rgb 5-5-5
          getBits = BMP_GetBits15bpp;
        else                                     //rgb 5-6-5
      getBits = BMP_GetBits16bpp;
        pbSrc += 12;                             // MiR 1st July 2002, for 16bpp bitmaps the bitmap body data 
        // starts at pbmi + cbHeader + 12
      }
      else
        getBits = BMP_GetBits15bpp;              // by default 5-5-5
      break;

    case 24:
      getBits = BMP_GetBits24bpp;
      break;

    case 32:
      if (png) {
        getBits = BMP_GetBits32bppPng;
      } else {
        if (bmi.biCompression == BI_BITFIELDS)
        {
          pbSrc += 12;                             // MiR 1st July 2002, for 16bpp bitmaps the bitmap body data 
          // starts at pbmi + cbHeader + 12
          WarningLine
            ("32 bits bitmap don't use default color mask. It's not supported by PilRC.");
        }
        getBits = BMP_GetBits32bpp;
      }
      break;

    default:
      ErrorLine
        ("Bitmap not monochrome, 16, 256, 16bit, 24bit or 32bit color");
      break;
  }

  // configure the palmOS bitmap settings
  switch (bitmaptype)
  {
    case rwBitmap:
      cbitsPel = 1;
      dstPaletteSize = 2;
      for (i = 0; i < dstPaletteSize; i++)
      {
        dstPalette[i][0] = PalmPalette1bpp[i][0];
        dstPalette[i][1] = PalmPalette1bpp[i][1];
        dstPalette[i][2] = PalmPalette1bpp[i][2];
      }
      break;

    case rwBitmapGrey:
      cbitsPel = 2;
      dstPaletteSize = 4;
      for (i = 0; i < dstPaletteSize; i++)
      {
        dstPalette[i][0] = PalmPalette2bpp[i][0];
        dstPalette[i][1] = PalmPalette2bpp[i][1];
        dstPalette[i][2] = PalmPalette2bpp[i][2];
      }
      break;

    case rwBitmapGrey16:
      cbitsPel = 4;
      dstPaletteSize = 16;
      for (i = 0; i < dstPaletteSize; i++)
      {
        dstPalette[i][0] = PalmPalette4bpp[i][0];
        dstPalette[i][1] = PalmPalette4bpp[i][1];
        dstPalette[i][2] = PalmPalette4bpp[i][2];
      }
      break;

    case rwBitmapColor16:
      cbitsPel = 4;
      dstPaletteSize = 16;
      for (i = 0; i < dstPaletteSize; i++)
      {
        dstPalette[i][0] = UserPalette4bpp[i][0];
        dstPalette[i][1] = UserPalette4bpp[i][1];
        dstPalette[i][2] = UserPalette4bpp[i][2];
      }
      break;

    case rwBitmapColor256:
      cbitsPel = 8;
      dstPaletteSize = 256;
      for (i = 0; i < dstPaletteSize; i++)
      {
        dstPalette[i][0] = UserPalette8bpp[i][0];
        dstPalette[i][1] = UserPalette8bpp[i][1];
        dstPalette[i][2] = UserPalette8bpp[i][2];
      }
      if (colortable)
        colorDat = COLOR_TABLE_SIZE;
      break;

    case rwBitmapColor16k:
      cbitsPel = 16;
      colortable = fFalse;
      if (density == kSingleDensity)
        colorDat = 8;                            // direct color structure & transparency
      break;

    case rwBitmapColor24k:
      cbitsPel = 24;
      colortable = fFalse;
      if (density == kSingleDensity)
        colorDat = 8;                            // direct color structure & transparency
      break;

    case rwBitmapColor32k:
      cbitsPel = 32;
      colortable = fFalse;
      if (density == kSingleDensity)
        colorDat = 8;                            // direct color structure & transparency
      break;

    default:
      Assert(fFalse);
      break;
  }

  // allocate memory for image data (word aligned)
  cbRow = ((dx * cbitsPel + 15) & ~15) >> 3;
  rcbmp->cbDst = (cbRow * dy) + colorDat;
  rcbmp->pbBits = (unsigned char *)malloc(rcbmp->cbDst);
  memset(rcbmp->pbBits, 0, rcbmp->cbDst);

  // configure the bitmap header
  switch (bitmaptype)
  {
    case rwBitmap:
    case rwBitmapGrey:
    case rwBitmapGrey16:
    case rwBitmapColor16:

      rcbmp->pixelsize = cbitsPel;
      rcbmp->version = 1;
      if (vfLE32)
        rcbmp->version |= LE_BITMAP_VERSION_MASK;

      // do we need to consider transparency?
      switch (transparencyData[0])
      {
        case rwTransparency:
          // rcbmp->ff |= 0x2000;
          rcbmp->flags.hasTransparency = fTrue;

          rcbmp->transparentIndex =
            BMP_RGBToColorIndex(transparencyData[1],
                                transparencyData[2],
                                transparencyData[3],
                                dstPalette, dstPaletteSize);
          break;

        case rwTransparencyIndex:
          // rcbmp->ff |= 0x2000;
          rcbmp->flags.hasTransparency = fTrue;

          rcbmp->transparentIndex = transparencyData[1];
          break;

        default:
          break;
      }
      break;

    case rwBitmapColor256:
      rcbmp->pixelsize = cbitsPel;
      rcbmp->version = 2;
      if (vfLE32)
        rcbmp->version |= LE_BITMAP_VERSION_MASK;

      // do we need to store a color table?
      if (colortable)
      {

        PILRC_BYTE *tmpPtr;
        int i;

        // rcbmp->ff |= 0x4000; 
        rcbmp->flags.hasColorTable = fTrue;

        tmpPtr = rcbmp->pbBits;
        if ( vfLE32 ) {
            /* 0x100 in 32-bit littleendian */
            *tmpPtr++ = 0x00;
            *tmpPtr++ = 0x01;
            *tmpPtr++ = 0x00;
            *tmpPtr++ = 0x00;
        }
        else {
            /* 0x100 in 16-bit bigendian */
            *tmpPtr++ = 0x01;
            *tmpPtr++ = 0x00;
        }

        // extract the color table (the number we have)
        for (i = 0; i < numClrs; i++)
        {
          *tmpPtr++ = i;
          *tmpPtr++ = pbmi->bmiColors[i].rgbRed;
          *tmpPtr++ = pbmi->bmiColors[i].rgbGreen;
          *tmpPtr++ = pbmi->bmiColors[i].rgbBlue;
        }

        // fill in remaining colors with black
        for (; i < 256; i++)
        {
          *tmpPtr++ = i;
          *tmpPtr++ = 0;
          *tmpPtr++ = 0;
          *tmpPtr++ = 0;
        }
      }

      // do we need to consider transparency?
      switch (transparencyData[0])
      {
        case rwTransparency:
          // rcbmp->ff |= 0x2000;
          rcbmp->flags.hasTransparency = fTrue;

          rcbmp->transparentIndex =
            BMP_RGBToColorIndex(transparencyData[1],
                                transparencyData[2],
                                transparencyData[3],
                                dstPalette, dstPaletteSize);
          break;

        case rwTransparencyIndex:
          // rcbmp->ff |= 0x2000;
          rcbmp->flags.hasTransparency = fTrue;

          rcbmp->transparentIndex = transparencyData[1];
          break;

        default:
          break;
      }
      break;

    case rwBitmapColor16k:
      rcbmp->pixelsize = cbitsPel;
      rcbmp->version = 2;
      if (vfLE32)
        rcbmp->version |= LE_BITMAP_VERSION_MASK;

      // rcbmp->ff |= 0x0400; 
      rcbmp->flags.directColor = fTrue;
      // do we need to consider transparency?
      if (density == kSingleDensity)
      {
        PILRC_BYTE *tmpPtr;

        tmpPtr = rcbmp->pbBits;
        *tmpPtr++ = 5;                           // 5 red bits
        *tmpPtr++ = 6;                           // 6 green bits
        *tmpPtr++ = 5;                           // 5 blue bits
        *tmpPtr++ = 0;                           // skip over reserved

        switch (transparencyData[0])
        {
          case rwTransparency:
            // rcbmp->ff |= 0x2000;
            rcbmp->flags.hasTransparency = fTrue;

            *tmpPtr++ = 0;
            *tmpPtr++ = transparencyData[1];
            *tmpPtr++ = transparencyData[2];
            *tmpPtr++ = transparencyData[3];     // set the transparent color
            break;

          default:
            break;
        }
      }
      else if (transparencyData[0] == rwTransparency)
        rcbmp->flags.hasTransparency = fTrue;
      break;

    case rwBitmapColor24k:
    case rwBitmapColor32k:
      rcbmp->pixelsize = cbitsPel;
      rcbmp->version = 2;
      if (vfLE32)
        rcbmp->version |= LE_BITMAP_VERSION_MASK;
      // rcbmp->ff |= 0x0400; 

      rcbmp->flags.directColor = fTrue;
      // do we need to consider transparency?
      if (density == kSingleDensity)
      {
        PILRC_BYTE *tmpPtr;

        tmpPtr = rcbmp->pbBits;
        *tmpPtr++ = 8;                           // 8 red bits
        *tmpPtr++ = 8;                           // 8 green bits
        *tmpPtr++ = 8;                           // 8 blue bits
        *tmpPtr++ = 0;                           // skip over reserved

        switch (transparencyData[0])
        {
          case rwTransparency:
            // rcbmp->ff |= 0x2000;
            rcbmp->flags.hasTransparency = fTrue;

            *tmpPtr++ = 0;
            *tmpPtr++ = transparencyData[1];
            *tmpPtr++ = transparencyData[2];
            *tmpPtr++ = transparencyData[3];     // set the transparent color
            break;

          default:
            break;
        }
      }
      else if (transparencyData[0] == rwTransparency)
        rcbmp->flags.hasTransparency = fTrue;
      break;

    default:
      break;
  }

  // convert from .bmp to binary format
  for (y = 0; y < dy; y++)
  {
    for (x = 0; x < dx; x++)
    {

      //int yT = (dy > 0) ? dy - y - 1 : y;
      int yT = (png) ? y : dy - y - 1;
      int w, a, r, g, b;

      // whats the (r,g,b) tupile at the index?
      w = getBits(pbmi, dx, pbSrc, x, yT, 32, &a, &r, &g, &b);

      // what type of bitmap are we dealing with?
      switch (bitmaptype)
      {
        case rwBitmap:
          {
            int v = BMP_RGBToColorIndex(r, g, b,
                                        dstPalette,
                                        dstPaletteSize);

            // if needed, set the bit
            if (v == 1)
              BMP_SetBits1bpp(dx, rcbmp->pbBits, x, y, 16);
          }
          break;

        case rwBitmapGrey:
          {
            int v = BMP_RGBToColorIndex(r, g, b,
                                        dstPalette,
                                        dstPaletteSize);

            BMP_SetBits2bpp(dx, rcbmp->pbBits, x, y, v, 16);
          }
          break;

        case rwBitmapGrey16:
        case rwBitmapColor16:
          {
            int v = BMP_RGBToColorIndex(r, g, b,
                                        dstPalette,
                                        dstPaletteSize);

            BMP_SetBits4bpp(dx, rcbmp->pbBits, x, y, v, 16);
          }
          break;

        case rwBitmapColor256:
          {
            int v = BMP_RGBToColorIndex(r, g, b,
                                        dstPalette,
                                        dstPaletteSize);

            // if we need a color table, use original bitmap data
            if (colortable)
              BMP_SetBits8bpp(dx, (rcbmp->pbBits + colorDat), x, y, w, 16);

            // no color table? use mapping
            else
              BMP_SetBits8bpp(dx, (rcbmp->pbBits + colorDat), x, y, v, 16);
          }
          break;

        case rwBitmapColor16k:
          {
            int pixel = ((((int)r & 0xF8) << 8) |       // 1111100000000000 
                         (((int)g & 0xFC) << 3) |       // 0000011111100000
                         (((int)b & 0xF8) >> 3));       // 0000000000011111

            BMP_SetBits16bpp(dx, (rcbmp->pbBits + colorDat), x, y, pixel, 16);
          }
          break;

        case rwBitmapColor24k:
          {
            int pixel = ((r << 16) | (g << 8) | b);

            BMP_SetBits24bpp(dx, (rcbmp->pbBits + colorDat), x, y, pixel, 16);
          }
          break;

        case rwBitmapColor32k:
          {
            int pixel = ((a << 24) | (r << 16) | (g << 8) | b);

            BMP_SetBits32bpp(dx, (rcbmp->pbBits + colorDat), x, y, pixel, 16);
          }
          break;

        default:
          break;
      }
    }
  }

  // store width/height information in bitmap
  rcbmp->cx = (int)dx;
  rcbmp->cy = (int)dy;
  rcbmp->cbRow = (int)cbRow;

  if (png && pbSrc) stbi_image_free(pbSrc);
}

/*
 * Skip a newline 
 */
static int
BMP_SkipNewLine(PILRC_BYTE * data,
                int size,
                int i)
{
  while (i < size && (data[i] != '\n' && data[i] != '\r'))
  {
    ++i;
  }
  if (i + 1 < size && data[i] == '\r' && data[i + 1] == '\n')
  {
    i += 2;
  }
  else if (i < size)
  {
    ++i;
  }
  return i;
}

/**
 * Convert a .pbitm file to Palm Computing resource data.
 *
 * @param rcbmp      a reference to the Palm Computing resource data.
 * @param data       a reference to the bitmap resource. 
 * @param size       the size the bitmap resource.
 */
static void
BMP_ConvertTextBitmap(RCBITMAP * rcbmp,
                      PILRC_BYTE * data,
                      int size)
{
  int i, x, y;

  // determine the width and height of the image
  rcbmp->cx = 0;
  rcbmp->cy = 0;
  for (i = 0; i < size; i = BMP_SkipNewLine(data, size, i))
  {
    int j = i;

    while ((j < size) && (data[j] != '\n') && (data[j] != '\r'))
      j++;
    if (rcbmp->cx < (j - i))
      rcbmp->cx = j - i;
    ++rcbmp->cy;
  }

  // allocate image buffer
  rcbmp->cbRow = ((rcbmp->cx + 15) & ~15) / 8;
  rcbmp->cbDst = rcbmp->cbRow * rcbmp->cy;
  rcbmp->pixelsize = 1;
  rcbmp->version = 1;
  if (vfLE32)
    rcbmp->version |= LE_BITMAP_VERSION_MASK;
  rcbmp->pbBits = malloc(rcbmp->cbDst);
  memset(rcbmp->pbBits, 0, rcbmp->cbDst);

  // convert the image
  x = 0;
  y = 0;
  for (i = 0; i < size;)
  {
    if ((data[i] == '\n') || (data[i] == '\r'))
    {
      x = 0;
      y++;
      i = BMP_SkipNewLine(data, size, i);
    }
    else
    {
      if ((data[i] != ' ') && (data[i] != '-'))
        rcbmp->pbBits[(y * rcbmp->cbRow) + (x >> 3)] |= (1 << (7 - (x & 7)));

      x++;
      i++;
    }
  }
}

/**
 * Convert a .xbm file to Palm Computing resource data.
 *
 * @param rcbmp      a reference to the Palm Computing resource data.
 * @param data       a reference to the bitmap resource. 
 * @param size       the size the bitmap resource.
 */
static void
BMP_ConvertX11Bitmap(RCBITMAP * rcbmp,
                     PILRC_BYTE * data,
                     int size)
{
  static PILRC_BYTE rev[] =
    { 0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15 };

  char name_and_type[80], *type;
  int i, value, pos;

  // read X11 bitmap header
  for (i = 0; i < size; i = BMP_SkipNewLine(data, size, i))
  {

    int result[2];

    // try and get the width and height
    result[0] = sscanf((const char *)data + i,
                       "#define %79s %d", name_and_type, &value);
    if (result[0] == 2)
    {

      type = strrchr(name_and_type, '_');
      type = (type == NULL) ? name_and_type : type + 1;
      if (strcmp(type, "width") == 0)
        rcbmp->cx = value;
      else if (strcmp(type, "height") == 0)
        rcbmp->cy = value;
    }
    // look for the data token (is there data)?
    result[0] = sscanf((const char *)data + i,
                       "static unsigned char %s = {", name_and_type);
    result[1] = sscanf((const char *)data + i,
                       "static char %s = {", name_and_type);
    if (result[0] || result[1])
    {

      type = strrchr(name_and_type, '_');
      type = (type == NULL) ? name_and_type : type + 1;
      if (strcmp(type, "bits[]") == 0 && rcbmp->cx > 0 && rcbmp->cy > 0)
      {
        rcbmp->cbRow = ((rcbmp->cx + 15) & ~15) / 8;
        rcbmp->cbDst = rcbmp->cbRow * rcbmp->cy;
        rcbmp->pbBits = malloc(rcbmp->cbDst);

        break;                                   // get out of the loop
      }
    }
  }

  if (rcbmp->pbBits == NULL)
    ErrorLine("Invalid X11 bitmap");

  // read the X11 bitmap data
  memset(rcbmp->pbBits, 0, rcbmp->cbDst);
  value = 0;
  pos = 0;
  for (i = BMP_SkipNewLine(data, size, i); i < size; i++)
  {

    PILRC_BYTE c = tolower(data[i]);

    // termination of a byte?
    if ((c == ',') || (c == '}'))
    {
      if (pos < rcbmp->cbDst)
      {
        rcbmp->pbBits[pos++] = (unsigned char)value;
        if (((rcbmp->cx % 16) > 0) && ((rcbmp->cx % 16) < 9) &&
            ((pos % rcbmp->cbRow) == (rcbmp->cbRow - 1)))
          pos++;
      }
      value = 0;
    }
    else
      // numerical data? - process accordingly (hexagonal)
    if ((c >= '0') && (c <= '9'))
      value = (value >> 4) + (rev[c - '0'] << 4);
    else if ((c >= 'a') && (c <= 'f'))
      value = (value >> 4) + (rev[c - 'a' + 10] << 4);
  }
}

struct rgb
{
  int r, g, b;
};

struct foreign_reader
{
  void (*start_row) (struct foreign_reader * self,
                     int y);
  void (*read_pixel) (struct foreign_reader * self,
                      struct rgb * color);
  int maxval;
  PILRC_BYTE *pb, *pblim;
  void *userdata;
};

static void
default_start_row(struct foreign_reader *self,
                  int y)
{
}

static void
WriteGreyTbmp(RCBITMAP * rcbmp,
              struct foreign_reader *reader)
{
  BOOL warnColorLost = fFalse;
  int depth = rcbmp->pixelsize;
  unsigned long outmaxval = (1UL << depth) - 1;
  unsigned char *outp = rcbmp->pbBits;
  int *index = malloc((reader->maxval + 1) * sizeof(int));
  int i, x, y, ningreys;

  for (i = 0; i <= reader->maxval; i++)
    index[i] = -1;

  for (y = 0; y < rcbmp->cy; y++)
  {
    unsigned int outword = 0;
    int outlen = 0;

    reader->start_row(reader, y);

    for (x = 0; x < rcbmp->cx; x++)
    {
      struct rgb c;
      unsigned long grey;

      reader->read_pixel(reader, &c);
      if (c.r == c.g && c.g == c.b)
        grey = c.r;
      else
      {
        warnColorLost = fTrue;
        /*
         * From the colorspace-FAQ.  There's _some_ chance I'm
         * applying the right formula.  It's been a long time
         * since graphics class...  
         */
        grey = (299L * c.r + 587L * c.g + 114L * c.b) / 1000L;
      }

      if (grey > (unsigned long)reader->maxval)
        grey = reader->maxval;

      if (index[grey] == -1)
      {
        unsigned long outgrey;

        outgrey = (grey * outmaxval + reader->maxval / 2) / reader->maxval;
        index[grey] = (outgrey <= outmaxval) ? outmaxval - outgrey : 0;
      }

      outword <<= depth;
      outword |= index[grey];
      outlen += depth;
      if (outlen >= 16)
      {
        *outp++ = outword >> 8;
        *outp++ = outword & 0xff;
        outword = 0;
        outlen = 0;
      }
    }

    if (outlen > 0)
    {
      outword <<= 16 - outlen;
      *outp++ = outword >> 8;
      *outp++ = outword & 0xff;
    }
  }

  if (warnColorLost)
    WarningLine("Some colors saturated in index converted to grey");

  ningreys = 0;
  for (i = 0; i <= reader->maxval; i++)
    if (index[i] != -1)
      ningreys++;

  free(index);

  if (ningreys > (int)(outmaxval + 1))
    WarningLine("%d input grey levels converted to only %ld",
                ningreys, outmaxval + 1);
}

static int
WriteIndexedColorTbmp(RCBITMAP * rcbmp,
                      struct foreign_reader *reader,
                      const int *transparencyData,
                      struct rgb *colortable)
{
#define N 937
  struct hash_entry
  {
    struct rgb key;
    unsigned char index;
  } table[N];

  unsigned char *outp = rcbmp->pbBits;
  int x, y, h, nentries, ninputcolors/*, noutputcolors*/;

  nentries = 0;
  for (h = 0; h < N; h++)
    table[h].key.r = -1;

  ninputcolors = 0;

  for (y = 0; y < rcbmp->cy; y++)
  {
    reader->start_row(reader, y);

    for (x = 0; x < rcbmp->cx; x++)
    {
      struct rgb c;
      unsigned char index;

      reader->read_pixel(reader, &c);

      /*
       * This hash function has no basis in theory...  
       */
      for (h = (((c.r + 37) ^ (c.g << 2)) + c.b) % N;; h = (h + 1) % N)
        if (table[h].key.r == -1)
        {
          struct rgb cs;

          cs.r = (c.r * 255UL) / reader->maxval;
          cs.g = (c.g * 255UL) / reader->maxval;
          cs.b = (c.b * 255UL) / reader->maxval;

          if (colortable)
          {
            if (ninputcolors < 256)
            {
              colortable[ninputcolors] = cs;
              index = ninputcolors;
            }
            else
              index = 0;                         /* FIXME!!!! */
          }
          else
            index = BMP_RGBToColorIndex(cs.r, cs.g, cs.b,
                                        UserPalette8bpp, 256);

	  if (transparencyData[0] == rwTransparency
	      && cs.r == transparencyData[1]
	      && cs.g == transparencyData[2]
	      && cs.b == transparencyData[3])
	  {
	    rcbmp->flags.hasTransparency = fTrue;
	    rcbmp->transparentIndex = index;
	  }

          ninputcolors++;

          /*
           * Only add the new color if the table isn't already
           * over full. This'll slow down for input images with
           * lots of distinct colors, but it won't be infinitely 
           * slow as it would be if we let the table get
           * completely full.  
           */
          if (nentries < N / 2)
          {
            nentries++;
            table[h].key.r = c.r;
            table[h].key.g = c.g;
            table[h].key.b = c.b;
            table[h].index = index;
          }
          break;
        }
        else if (table[h].key.r == c.r && table[h].key.g == c.g
                 && table[h].key.b == c.b)
        {
          index = table[h].index;
          break;
        }

      *outp++ = index;
    }

    if (rcbmp->cbRow > rcbmp->cx)
      *outp++ = 0;
  }

  //noutputcolors = (colortable) ? ninputcolors : 0;

  if (ninputcolors > 256)
  {
    WarningLine("%d input colors converted to only 256", ninputcolors);
    ninputcolors = 256;
  }

  return ninputcolors;
}

/**
 * Write the body of a Palm Computing bitmap resource data (family member).
 *
 * @param rcbmp      a reference to the Palm Computing resource data.
 * @param width      width of the bitmap
 * @param height     height of the bitmap
 * @param bitmaptype the type of bitmap (B+W, Grey, Grey16 or Color)?
 * @param colortable does a color table need to be generated?
 * @param transparencyData anything we need to know about transparency
 * @param reader     callbacks and state variables to read the input file
 */
static void
WriteTbmp(RCBITMAP * rcbmp,
          int width,
          int height,
          int bitmaptype,
          BOOL colortable,
          const int *transparencyData,
          struct foreign_reader *reader)
{
  int depth = 0;

  switch (bitmaptype)
  {
    case rwBitmap:
      depth = 1;
      break;

    case rwBitmapGrey:
      depth = 2;
      break;

    case rwBitmapGrey16:
      depth = 4;
      break;

    case rwBitmapColor256:
      depth = 8;
      break;

    case rwBitmapColor16k:
    case rwBitmapColor24k:
    case rwBitmapColor32k:

      // 
      // UNSUPPORTED RIGHT NOW
      // 

      break;

    default:
      Assert(fFalse);
      break;
  }

  rcbmp->cx = width;
  rcbmp->cy = height;
  rcbmp->pixelsize = depth;
  rcbmp->cbRow = ((width * depth + 15) & ~15) >> 3;
  rcbmp->cbDst = rcbmp->cbRow * height;
  rcbmp->pbBits = malloc(rcbmp->cbDst);
  rcbmp->version = (depth >= 8) ? 2 : 1;
  if (vfLE32)
    rcbmp->version |= LE_BITMAP_VERSION_MASK;

  if (transparencyData[0] == rwTransparencyIndex)
  {
    rcbmp->flags.hasTransparency = fTrue;
    rcbmp->transparentIndex = transparencyData[1];
  }

  /*
   * The iterate-over-the-pixels code of these two ought to be unified
   * into one function, but because the color conversions inside the
   * loops are so different, it seems easier to duplicate the loops.  
   */

  if (rcbmp->pixelsize < 8)
    WriteGreyTbmp(rcbmp, reader);
  else
  {
    if (colortable)
    {
      struct rgb table[256];
      int nentries = WriteIndexedColorTbmp(rcbmp, reader, transparencyData, table);

      /*
       * Now bolt the color table onto the front of the output bits. 
       */

      unsigned char *newBits = malloc(2 + 4 * nentries + rcbmp->cbDst);

      if (newBits)
      {
        unsigned char *bits = newBits;
        int i;

        *bits++ = (nentries & 0xff00) >> 8;
        *bits++ = nentries & 0x00ff;

        for (i = 0; i < nentries; i++)
        {
          *bits++ = i;
          *bits++ = table[i].r;
          *bits++ = table[i].g;
          *bits++ = table[i].b;
        }

        memcpy(bits, rcbmp->pbBits, rcbmp->cbDst);
        free(rcbmp->pbBits);

        // rcbmp->ff |= 0x4000;
        rcbmp->flags.hasColorTable = fTrue;

        rcbmp->pbBits = newBits;
        rcbmp->cbDst += 2 + 4 * nentries;
      }
    }
    else
      (void)WriteIndexedColorTbmp(rcbmp, reader, transparencyData, NULL);
  }
}

static void
SkipPNMWhitespace(struct foreign_reader *r)
{
  while (r->pb < r->pblim)
    if (isspace(*r->pb))
      r->pb++;
    else if (*r->pb == '#')
      while (r->pb < r->pblim && *r->pb != '\n' && *r->pb != '\r')
        r->pb++;
    else
      break;
}

static int
ReadPNMInt(struct foreign_reader *r)
{
  int n = 0;

  SkipPNMWhitespace(r);
  for (; r->pb < r->pblim && isdigit(*r->pb); r->pb++)
    n = 10 * n + *r->pb - '0';

  return n;
}

/*
 * A lot of these depend on PILRC_BYTE being unsigned.  Further, if we run
 * out of input (i.e., pb >= pblim) these won't infinite loop or anything,
 * but they probably will return random colors.  But running out of input
 * indicates that the input is corrupted, so this doesn't really matter.  
 */

static void
ReadP1(struct foreign_reader *r,
       struct rgb *c)
{
  SkipPNMWhitespace(r);
  while (r->pb < r->pblim)
    switch (*r->pb++)
    {
      case '0':
        /*
         * Yes, PBM color levels really are reversed compared to PGMs! 
         */
        c->r = c->g = c->b = 1;
        return;

      case '1':
        c->r = c->g = c->b = 0;
        return;

      default:
        break;
    }
}

struct userdata_P4
{
  PILRC_BYTE byte, mask;
};

static void
StartRowP4(struct foreign_reader *r,
           int y)
{
  struct userdata_P4 *p4 = r->userdata;

  p4->mask = 0;
}

static void
ReadP4(struct foreign_reader *r,
       struct rgb *c)
{
  struct userdata_P4 *p4 = r->userdata;

  if (p4->mask == 0)
  {
    if (r->pb < r->pblim)
      p4->byte = *r->pb++;
    p4->mask = 0x80;
  }

  c->r = c->g = c->b = (p4->byte & p4->mask) ? 0 : 1;
  p4->mask >>= 1;
}

static void
ReadP2(struct foreign_reader *r,
       struct rgb *c)
{
  c->r = c->g = c->b = ReadPNMInt(r);
}

static void
ReadP5(struct foreign_reader *r,
       struct rgb *c)
{
  if (r->pb < r->pblim)
    c->r = c->g = c->b = *r->pb++;
}

static void
ReadP3(struct foreign_reader *r,
       struct rgb *c)
{
  c->r = ReadPNMInt(r);
  c->g = ReadPNMInt(r);
  c->b = ReadPNMInt(r);
}

static void
ReadP6(struct foreign_reader *r,
       struct rgb *c)
{
  if (r->pb + 2 < r->pblim)
  {
    c->r = *r->pb++;
    c->g = *r->pb++;
    c->b = *r->pb++;
  }
}

/**
 * Convert a PBM/PGM/PPM file to Palm Computing resource data.
 *
 * @param rcbmp      a reference to the Palm Computing resource data.
 * @param pbResData  a reference to the bitmap resource.
 * @param cb         the size the bitmap resource.
 * @param bitmaptype the type of bitmap (B+W, Grey, Grey16 or Color)?
 * @param colortable does a color table need to be generated?
 * @param transparencyData anything we need to know about transparency
 */
static void
BMP_ConvertPNMBitmap(RCBITMAP * rcbmp,
                     PILRC_BYTE * pb,
                     int cb,
                     int bitmaptype,
                     BOOL colortable,
                     int *transparencyData)
{
  static void (*read_func[]) (struct foreign_reader *,
                              struct rgb *) =
  {
  NULL, ReadP1, ReadP2, ReadP3, ReadP4, ReadP5, ReadP6};

  struct foreign_reader pnm;
  struct userdata_P4 dataP4;
  int type, width, height;

  type = (cb >= 2 && pb[0] == 'P') ? pb[1] - '0' : -1;

  if (type < 1 || type > 6)
    ErrorLine("Not a PBM/PNM/PGM/PPM file.");

  pnm.pb = pb;
  pnm.pblim = pb + cb;

  pnm.pb += 2;
  width = ReadPNMInt(&pnm);
  height = ReadPNMInt(&pnm);

  pnm.maxval = (type == 1 || type == 4) ? 1 : ReadPNMInt(&pnm);

  if (type >= 4)
  {
    /*
     * Skip up to one character of whitespace in RAWBITS files.  
     */
    if (pnm.pb < pnm.pblim && isspace(*pnm.pb))
      pnm.pb++;
  }

  pnm.read_pixel = read_func[type];
  if (type == 4)
  {
    pnm.start_row = StartRowP4;
    pnm.userdata = &dataP4;
  }
  else
    pnm.start_row = default_start_row;

  WriteTbmp(rcbmp, width, height, bitmaptype, colortable, transparencyData, &pnm);

  if (transparencyData[0] && !rcbmp->flags.hasTransparency)
    WarningLine("Transparency not detected in PBM/PNM/PGM/PPM file");
}

/**
 * Scanline Compress a Bitmap (Tbmp or tAIB) resource.
 * 
 * @param rcbmp      a reference to the Palm Computing resource data.
 * @param compress   compression style?
 * @param colortable does a color table need to be generated?
 * @param directColor is the bitmap > 8bpp? (direct color mode)
 */
static void
BMP_CompressBitmap(RCBITMAP * rcbmp,
                   int compress,
                   int bmpVersion,
                   BOOL colortable,
                   BOOL directColor)
{
  unsigned char *bits;
  int size, msize, i, j, k, flag;
  int firstByteToWriteLen;

  // determine how much memory is required for compression (hopefully
  // less)
  size = 2;                                      // compressed bmp size in UInt16
  if (bmpVersion > 2)
    size += 2;                                   // compressed bmp size in UInt32
  if (colortable)
    size += COLOR_TABLE_SIZE;
  if ((bmpVersion < 3) && (directColor))
    size += 8;

  msize = size + ((rcbmp->cbRow + ((rcbmp->cbRow + 7) / 8)) * rcbmp->cy);

  // allocat memory and clear
  bits = (unsigned char *)malloc(msize * sizeof(unsigned char));
  memset(bits, 0, msize * sizeof(unsigned char));

  // prevent transparency data from being compressed in 16bpp
  if ((bmpVersion < 3) && (directColor))
    rcbmp->pbBits += 8;

  if (colortable)                                // don't compress color table
    rcbmp->pbBits += COLOR_TABLE_SIZE;

  // do the scanline compression (at least, attempt it)
  for (i = 0; i < rcbmp->cy; i++)
  {
    flag = 0;
    for (j = 0; j < rcbmp->cbRow; j++)
    {
      if ((i == 0) ||
          (rcbmp->pbBits[(i * rcbmp->cbRow) + j] !=
           rcbmp->pbBits[((i - 1) * rcbmp->cbRow) + j]))
      {
        flag |= (0x80 >> (j & 7));
      }
      if (((j & 7) == 7) || (j == (rcbmp->cbRow - 1)))
      {
        bits[size++] = (unsigned char)flag;
        for (k = (j & ~7); k <= j; ++k)
        {
          if (((flag <<= 1) & 0x100) != 0)
            bits[size++] = rcbmp->pbBits[i * rcbmp->cbRow + k];
        }
        flag = 0;
      }
    }
  }

  if (colortable)
    rcbmp->pbBits -= COLOR_TABLE_SIZE;

  // fix pointer back for 16bpp (we offset it previously)
  if ((bmpVersion < 3) && (directColor))
    rcbmp->pbBits -= 8;

  // if we must compress, or if it was worth it, save!
  if (compress == rwForceCompress || size < rcbmp->cbDst)
  {
    // do we have a color table?
    if ((colortable) && (!directColor))
    {
      int i;

      // copy the color table (dont forget it!)
      for (i = 0; i < COLOR_TABLE_SIZE; i++)
        bits[i] = rcbmp->pbBits[i];

      firstByteToWriteLen = COLOR_TABLE_SIZE;
    }
    // direct color info?
    else if ((bmpVersion < 3) && (directColor))
    {
      int i;

      // copy the direct color info (dont forget it!)
      for (i = 0; i < 8; i++)
        bits[i] = rcbmp->pbBits[i];

      firstByteToWriteLen = 8;
    }
    else
    {
      firstByteToWriteLen = 0;
    }

    if (bmpVersion > 2)
    {                                            /* 32 bits */
      bits[firstByteToWriteLen] = (unsigned char)((size & 0xff000000) >> 24);
      bits[firstByteToWriteLen + 1] =
        (unsigned char)((size & 0x00ff0000) >> 16);
      bits[firstByteToWriteLen + 2] =
        (unsigned char)((size & 0x0000ff00) >> 8);
      bits[firstByteToWriteLen + 3] = (unsigned char)(size & 0x000000ff);
    }
    else
    {
      if (size > 0xFFFF)
        ErrorLine("bmp compressed too big to enter in version 2 format...");

      bits[firstByteToWriteLen] = (unsigned char)((size & 0xff00) >> 8);
      bits[firstByteToWriteLen + 1] = (unsigned char)(size & 0x00ff);
    }

    // change the data chunk to the newly compressed data
    free(rcbmp->pbBits);

    // rcbmp->ff |= 0x8000;
    rcbmp->flags.compressed = fTrue;

    rcbmp->pbBits = bits;
    rcbmp->cbDst = size;
  }
  else
    free(bits);
}

                                                                // *INDENT-OFF*
static const unsigned short crctt_16[256] = 
{
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
  0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
  0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
  0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
  0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
  0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
  0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
  0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
  0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
  0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
  0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
  0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
  0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
  0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
  0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
  0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
  0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
  0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
  0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
  0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
  0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
  0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
  0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
  0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
  0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
  0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
  0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
  0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
  0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
  0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
                                                                // *INDENT-ON*

/**
 * Calculate the 16-bit CRC of a data block using the table
 *	  lookup method.
 *
 *	@param bufP			pointer to the data buffer;
 *	@param count		the number of bytes in the buffer;
 *	@param crc			the seed crc value;
 *
 * RETURNS:				a 16-bit CRC for the data buffer.
 *
 * 03-mar-2001			RMa add on
 *
 */
static unsigned short
Crc16CalcBlock(const void *bufP,
               unsigned short count,
               unsigned short crc)
{
  register const unsigned char *byteP = bufP;
  const unsigned short *crctt = crctt_16;   // CRC translation table

  //
  // Calculate the 16 bit CRC using the table lookup method.
  //
  if (count)
  {
    do
    {
      crc = (crc << 8) ^ crctt[(unsigned char)((crc >> 8) ^ *byteP++)];
    } while (--count);
  }

  return (crc & 0xffff);
}

/**
 * Fill new header with common value and add new value
 *
 *	@param rcbmp		pointer to the source data buffer;
 *	@param rcbmpv3		pointer to the destination data buffer;
 *	@param transparencyData    pointer to transparency data
 *	@param density		bit density of new V3 bitmap
 *
 * 25-apr-2002			RMa add on
 *
 */
static void
BMP_FillBitmapV3Header(RCBITMAP * rcbmp,
                       RCBITMAP_V3 * rcbmpv3,
                       int *transparencyData,
                       int density)
{
  if ((rcbmp) && (rcbmpv3))
  {
    rcbmpv3->cx = rcbmp->cx;
    rcbmpv3->cy = rcbmp->cy;
    rcbmpv3->cbRow = rcbmp->cbRow;
    rcbmpv3->flags = rcbmp->flags;
    rcbmpv3->pixelsize = rcbmp->pixelsize;
    if (vfLE32)
      rcbmpv3->version = 0x83;
    else
      rcbmpv3->version = 0x03;
    rcbmpv3->size = 0x18;

    if (vfLE32)
    {
      if (rcbmp->pixelsize <= 8)
        /* Palm/Sony devices don't support pixelFormatIndexedLE,
           so do pixelFormatIndexed--which is the same, since
           there is no LE/BE difference for 8-bits! */
        rcbmpv3->pixelFormat = pixelFormatIndexed; 
      else
        rcbmpv3->pixelFormat = pixelFormat565LE;
    }
    else
    {
      if (rcbmp->pixelsize <= 8)
        rcbmpv3->pixelFormat = pixelFormatIndexed;
      else
        rcbmpv3->pixelFormat = pixelFormat565;
    }

    rcbmpv3->compressionType = rcbmp->compressionType;
    rcbmpv3->density = density;
    rcbmpv3->transparentValue = 0;
    if (rcbmp->flags.hasTransparency)
    {
      if (rcbmp->pixelsize <= 8)
    rcbmpv3->transparentValue = rcbmp->transparentIndex;
      else if (rcbmp->pixelsize == 16)
      {
        // RMa bug fix in 16k no palette, no index just the 5-6-5 color intead.
        rcbmpv3->transparentValue = (((transparencyData[1] & 0xF8) << 8) |      // 1111100000000000 
                                     ((transparencyData[2] & 0xFC) << 3) |      // 0000011111100000
                                     ((transparencyData[3] & 0xF8) >> 3));      // 0000000000011111
      }
      else
      {
        rcbmpv3->transparentValue = 0;
        rcbmpv3->transparentValue = (transparencyData[1] << 16) & 0x00FF0000;
        rcbmpv3->transparentValue |= (transparencyData[2] << 8) & 0x0000FF00;
        rcbmpv3->transparentValue |= (transparencyData[3] << 0) & 0x000000FF;
      }
    }
    rcbmpv3->nextDepthOffset = rcbmp->nextDepthOffset;
    rcbmpv3->cbDst = rcbmp->cbDst;
    rcbmpv3->pbBits = rcbmp->pbBits;
  }
}

/**
 * Compress and Dump a single Bitmap (Tbmp or tAIB) resource.
 * 
 * @param rcbmp      a reference to the Bitmap resource
 * @param isIcon     an icon? 0 = bitmap, non zero = icon resource ID
 * @param compress   compression style?
 * @param colortable does a color table need to be generated?
 * @param directColor is the bitmap > 8bpp? (direct color mode)
 * @param multibit   should this bitmap be prepared for multibit? 
 * @param bootscreen	should this bitmap be prepared for size & crc header add on ?
 * @param density    does this bitmap is a new version
 */
static void
BMP_CompressDumpBitmap(RCBITMAP * rcbmp,
                       int isIcon,
                       int compress,
                       BOOL colortable,
                       BOOL directColor,
                       BOOL multibit,
                       BOOL bootScreen,
                       int density,
                       int *transparencyData)
{
  int stdIconSize_x;
  int stdIconSize_y;
  int stdSmallIconSize_x;
  int stdSmallIconSize_y;

  if (density == kSingleDensity)
  {
	stdIconSize_x = 32;
	stdIconSize_y = 22;
	stdSmallIconSize_x = 15;
	stdSmallIconSize_y = 9;
  }
  else if (density == kOneAndOneHalfDensity)
  {
	stdIconSize_x = 48;
	stdIconSize_y = 33;
	stdSmallIconSize_x = 22;
	stdSmallIconSize_y = 13;
  }
  else if (density == kDoubleDensity)
  {
	stdIconSize_x = 64;
	stdIconSize_y = 44;
	stdSmallIconSize_x = 30;
	stdSmallIconSize_y = 18;
  }
  else
  {
	ErrorLine("Unsupported density value");
	stdIconSize_x = stdIconSize_y = 0;
	stdSmallIconSize_x = stdSmallIconSize_y = 0;
  }

  // anything specific with icons here?
  switch (isIcon)
  {
    case 1000:
      if (!vfAllowBadIconSizes &&
          ((rcbmp->cx != stdIconSize_x) || (rcbmp->cy != stdIconSize_x)) &&
          ((rcbmp->cx != stdIconSize_x) || (rcbmp->cy != stdIconSize_y)) &&
          ((rcbmp->cx != stdIconSize_y) || (rcbmp->cy != stdIconSize_y)))
        WarningLine("Icon resource not %dx%d, %dx%d or %dx%d (preferred)",
                    stdIconSize_x, stdIconSize_x,
                    stdIconSize_x, stdIconSize_y,
                    stdIconSize_y, stdIconSize_y);
      break;

    case 1001:
      if (!vfAllowBadIconSizes &&
          (rcbmp->cx != stdSmallIconSize_x) &&
          (rcbmp->cy != stdSmallIconSize_y))
        WarningLine("Small icon resource not %dx%d",
                    stdSmallIconSize_x, stdSmallIconSize_y);
      break;

    default:
      // using non-standard icon resource :)) *tut-tut* :)
      break;
  }

  // do we need to do some compression?
  // NOTE: compression of 16, 24 and 32bpp DONT work right now
  if ((compress == rwAutoCompress) || (compress == rwForceCompress))
  {
    BMP_CompressBitmap(rcbmp, compress, (density == kSingleDensity) ? 2 : 3, colortable,
                       directColor);
  }

  // is this single resource part of a multibit bitmap family?
  if (multibit)
  {
    if (density != kSingleDensity)
    {
      // determine the next depth offset (# byte)
      if (rcbmp->cbDst & 3)
        rcbmp->nextDepthOffset = 0x18 + rcbmp->cbDst + 4 - (rcbmp->cbDst & 3);
      else
        rcbmp->nextDepthOffset = 0x18 + rcbmp->cbDst;
    }
    else
      // determine the next depth offset (# dwords)
      rcbmp->nextDepthOffset = 4 + (rcbmp->cbDst >> 2); // 16
    // bytes - 
    // header

    // if we need to, round up to the nearest dword and get more
    // memory
    if (density == kSingleDensity)
      if ((rcbmp->cbDst % 4) != 0)
      {
        int i, oldSize = rcbmp->cbDst;

        rcbmp->nextDepthOffset++;
        rcbmp->cbDst = (rcbmp->nextDepthOffset - 4) << 2;
        rcbmp->pbBits = (PILRC_BYTE *) realloc(rcbmp->pbBits, rcbmp->cbDst);

        // need to clear extra memory that was allocated?
        for (i = oldSize; i < rcbmp->cbDst; i++)
          rcbmp->pbBits[i] = 0x00;               // clear it
      }
  }

  if (bootScreen)                                /* RMa add on header is a size and a crc */
  {
    FILE *outputFile = getOpenedOutputFile();
    int currentPosInFile = ftell(outputFile);
    int newPosInFile;
    char *pData;
    int dataSize;
    int bootScreenHeaderSize = (vfLE32) ? 8 : 6;
    unsigned short crc;
    int test;
    int error;

    EmitL(0);
    EmitW(0);
    if (vfLE32)
      EmitW(0);

    // dump the bitmap header and data
    if (density != kSingleDensity)
    {
      RCBITMAP_V3 rcbmpv3;

      BMP_FillBitmapV3Header(rcbmp, &rcbmpv3, transparencyData, density);
      CbEmitStruct(&rcbmpv3, szRCBITMAP_V3, NULL, fTrue);
    }
    else
    {
      CbEmitStruct(rcbmp, szRCBITMAP, NULL, fTrue);     /* write structure tbmp */
    }
    DumpBytes(rcbmp->pbBits, rcbmp->cbDst);      /* write data tbmp */
    if (density != kSingleDensity)
    {
      BOOL savedValue = vfLE32;

      vfLE32 = fTrue;
      PadBoundary();
      vfLE32 = savedValue;
    }
    else
      PadBoundary();
    newPosInFile = ftell(outputFile);

    dataSize = newPosInFile - (currentPosInFile + bootScreenHeaderSize);
    pData = malloc(dataSize);
    error = fseek(outputFile, currentPosInFile + bootScreenHeaderSize, SEEK_SET);       /* jump after the header */
    if (!error)
    {
      if ((test = fread(pData, 1, dataSize, outputFile)) != dataSize)
      {
        fprintf(stderr, "ERROR: Read %lu bytes, expected %lu\n",
          (unsigned long)test, (unsigned long)dataSize);
        abort();
      }
      crc = Crc16CalcBlock(pData, (unsigned short)dataSize, 0);

      error = fseek(outputFile, currentPosInFile, SEEK_SET);    /* jump before the header */
      EmitL(dataSize);                           /* write header size of data */
      if (vfLE32)                                /* write header crc of data */
        EmitL((unsigned int)crc);
      else
        EmitW(crc);

      error = fseek(outputFile, newPosInFile, SEEK_SET);        /* jump afer header + bmp */
    }
    free(pData);
  }
  else
  {
    // dump the bitmap header and data
    if (density != kSingleDensity)
    {
      RCBITMAP_V3 rcbmpv3;
      BOOL savedValue = vfLE32;

      BMP_FillBitmapV3Header(rcbmp, &rcbmpv3, transparencyData, density);
      CbEmitStruct(&rcbmpv3, szRCBITMAP_V3, NULL, fTrue);
    DumpBytes(rcbmp->pbBits, rcbmp->cbDst);

      vfLE32 = fTrue;
      PadBoundary();
      vfLE32 = savedValue;
    }
    else
    {
      CbEmitStruct(rcbmp, szRCBITMAP, NULL, fTrue);
      DumpBytes(rcbmp->pbBits, rcbmp->cbDst);
      PadBoundary();                             // RMa add: BUG correction 
  }

  }
  // clean up
  free(rcbmp->pbBits);
}

/**
 * The fileName for the resource cannot be processed.
 *
 * @param fileName the source file name
 */
static void
BMP_InvalidExtension(const char *fileName)
{
  ErrorLine("Bitmap file extension not recognized for file %s"
            " (Supported extensions: .BMP, .pbitm, .xbm and .pbm/.ppm/.pnm)",
            fileName);
}

/**
 * Dump a single Bitmap (Tbmp or tAIB or tbsb) resource.
 * 
 * @param fileName   the source file name  
 * @param isIcon     an icon? 0 = bitmap, non zero = icon resource ID
 * @param compress   compression style?
 * @param bitmaptype the type of bitmap (B+W, Grey, Grey16 or Color)?
 * @param colortable does a color table need to be generated?
 * @param transparencyData anything we need to know about transparency
 * @param multibit   should this bitmap be prepared for multibit? 
 * @param bootscreen	should this bitmap be prepared for size & crc header add on ?
 * @param density    does this bitmap is a new version
 */
void DumpBitmap(const char *fileName,
           int isIcon,
           int compress,
           int bitmaptype,
           BOOL colortable,
           int *transparencyData,
           BOOL multibit,
           BOOL bootScreen,
           int density)
{
  PILRC_BYTE *pBMPData;
  const char *pchExt;
  char *pathName;
  FILE *pFile;
  long size;
  RCBITMAP rcbmp;
  BOOL directColor;

#if BITMAP_CALL_LOG
  if (!vfQuiet)
  {
    printf("DumpBitmap(\"%s\", ", fileName);
    if (isIcon)
      printf("icon #%d, ", isIcon);
    else
      printf("not-icon, ");
    switch (compress)
    {
      case 0:			printf("no(0)-compress, ");	break;
      case rwNoCompress:	printf("no-compress, ");	break;
      case rwAutoCompress:	printf("compress, ");		break;
      case rwForceCompress:	printf("force-compress, ");	break;
      default:
	printf("compress %d?, ", compress);
	break;
    }
    switch (bitmaptype)
    {
      case rwBitmap:		printf("bmp,");		break;
      case rwBitmapGrey:	printf("bmpGrey,");	break;
      case rwBitmapGrey16:	printf("bmpGrey16,");	break;
      case rwBitmapColor16:	printf("bmpColor16,");	break;
      case rwBitmapColor256:	printf("bmpColor256,");	break;
      case rwBitmapColor16k:	printf("bmpColor16k,");	break;
      case rwBitmapColor24k:	printf("bmpColor24k,");	break;
      case rwBitmapColor32k:	printf("bmpColor32k,");	break;
      default:
	printf("bmp %d?,", bitmaptype);
	break;
    }
    printf("\n           %sctbl, ", colortable? "" : "no-");
    switch (transparencyData[0])
    {
      case 0:
	printf("no-transp, ");
	break;
      case rwTransparency:
	printf("transp %d:%d:%d, ",
	       transparencyData[1], transparencyData[2], transparencyData[3]);
	break;
      case rwTransparencyIndex:
	printf("transp #%d, ", transparencyData[1]);
	break;
      default:
	printf("transp %d?, ", transparencyData[0]);
	break;
    }
    printf("%smultibit, %sbs, %d)\n",
	   multibit? "" : "not-", bootScreen? "" : "not-", density);
  }
#endif

  // is it a direct color bitmap? 
  directColor = ((bitmaptype == rwBitmapColor16k) ||
                 (bitmaptype == rwBitmapColor24k) ||
                 (bitmaptype == rwBitmapColor32k));

  // determine the size of the resource to load
  pathName = FindAndOpenFile(fileName, "rb", &pFile);
  if (pFile == NULL)
    ErrorLine("Could not find Resource %s.", fileName);

  fseek(pFile, 0, SEEK_END);
  size = ftell(pFile);
  rewind(pFile);

  // alocate memory and load the resource in
  pBMPData = malloc(size);
  if (pBMPData == NULL)
    ErrorLine("Resource %s too big to fit in memory", pathName);
  fread(pBMPData, 1, size, pFile);
  fclose(pFile);

  // obtain the file extension
  pchExt = strrchr(pathName, '.');
  if ((strlen(pathName) < 5) || (pchExt == NULL))
  {
    BMP_InvalidExtension(pathName);
  }
  pchExt++;

  // convert the resource into a binary resource
  memset(&rcbmp, 0, sizeof(RCBITMAP));
  if (FSzEqI(pchExt, "bmp"))
  {
    BMP_ConvertWindowsBitmap(&rcbmp, pBMPData, size, bitmaptype, colortable,
                             transparencyData, density, 0);
    BMP_CompressDumpBitmap(&rcbmp, isIcon, compress, colortable,
                           directColor, multibit, bootScreen, density,
                             transparencyData);
  }
  else if (FSzEqI(pchExt, "png"))
  {
    BMP_ConvertWindowsBitmap(&rcbmp, pBMPData, size, bitmaptype, colortable,
                             transparencyData, density, 1);
    BMP_CompressDumpBitmap(&rcbmp, isIcon, compress, colortable,
                           directColor, multibit, bootScreen, density,
                             transparencyData);
  }
  else if (FSzEqI(pchExt, "pbitm"))
  {
    BMP_ConvertTextBitmap(&rcbmp, pBMPData, size);
    BMP_CompressDumpBitmap(&rcbmp, isIcon, compress, fFalse,
                           directColor, multibit, bootScreen, density,
                           transparencyData);
  }
  else if (FSzEqI(pchExt, "xbm"))
  {
    BMP_ConvertX11Bitmap(&rcbmp, pBMPData, size);
    BMP_CompressDumpBitmap(&rcbmp, isIcon, compress, fFalse,
                           directColor, multibit, bootScreen, density,
                           transparencyData);
  }
  else if (FSzEqI(pchExt, "pbm") || FSzEqI(pchExt, "pgm")
           || FSzEqI(pchExt, "ppm") || FSzEqI(pchExt, "pnm"))
  {
    BMP_ConvertPNMBitmap(&rcbmp, pBMPData, size, bitmaptype, colortable,
                         transparencyData);
    BMP_CompressDumpBitmap(&rcbmp, isIcon, compress, fFalse,
                           directColor, multibit, bootScreen, density,
                           transparencyData);
  }
  else
  {
    BMP_InvalidExtension(pathName);
  }

  // clean up
  free(pBMPData);
  free(pathName);
}
