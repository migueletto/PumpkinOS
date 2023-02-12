
/*
 * @(#)bitmap.h
 *
 * Copyright 1997-1999, Wes Cherry   (mailto:wesc@technosis.com)
 *           2000-2003, Aaron Ardiri (mailto:aaron@ardiri.com)
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

#ifndef __BITMAP_H__
#define __BITMAP_H__

/**
 * Dump a single Bitmap (Tbmp or tAIB) resource.
 * 
 * @param fileName   the source file name  
 * @param isIcon     an icon? 0 = bitmap, 1 = normal, 2 = small
 * @param compress   compression style?
 * @param bitmaptype the type of bitmap (B+W, Grey, Grey16 or Color)?
 * @param colortable does a color table need to be generated?
 * @param transparencyData anything we need to know about transparency
 * @param multibit   should this bitmap be prepared for multibit? 
 * @param bootscreen	should this bitmap be prepared for size & crc header add on ?
 */
extern void DumpBitmap(const char *fileName,
                       int isIcon,
                       int compress,
                       int bitmaptype,
                       BOOL colortable,
                       int *transparencyData,
                       BOOL multibit,
                       BOOL bootscreen,
                       int density);

/* Note that the palettes set by the following functions are only used
   when DumpBitmap() is called with a bitmaptype of rwBitmapColor16 or
   rwBitmapColor256.  */

extern void SetUserPalette4bpp(int p[][3],
                               int nColors);
extern void SetUserPalette4bppToDefault4bpp();
extern void SetUserPalette8bpp(int p[][3],
                               int nColors);
extern void SetUserPalette8bppToDefault8bpp();

#define kSingleDensity          72
#define kOneAndOneHalfDensity   108
#define kDoubleDensity          144

#endif
