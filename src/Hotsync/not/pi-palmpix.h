/*
 * $Id: pi-palmpix.h,v 1.5 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-palmpix.h: Kodak PalmPix Camera support 
 * (Kodak didn't like that we did this, tee hee!)
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
 
#ifndef PILOT_PALMPIX_H
#define PILOT_PALMPIX_H

#include "pi-args.h"

#ifdef __cplusplus
extern "C" 
{
#endif

#define PalmPix_Creator (makelong ("COCO"))
#define PalmPix_DB "ArchImage"

/* flags values */
#define PALMPIX_COLOUR_CORRECTION    1
#define PALMPIX_HISTOGRAM_STRETCH    2

struct PalmPixState {
   /* This callback should read record #RECNO into BUFFER and BUFSIZE, and
      return 0 when successful, just like pi_file_read_record().  */
   int (*getrecord) PI_ARGS ((struct PalmPixState *self, int recno,
   void **buffer, size_t *bufsize));

   /* This will be filled in by pixName.  */
   char pixname[33];

   /* After unpack_PalmPix, this will be the last record index which is part
      of the current picture.  */
   int highest_recno;
   
   /* Set these to some permutation of 0,1,2 before using pixPixmap.  */
   int offset_r, offset_g, offset_b;

   /* This specifies the png or ppm output */
   int output_type;
   
   /* This will be filled in by pixPixmap.  */
   unsigned char *pixmap;

   /* The output brightness adjustment */
   int bias;
   
   /* This controls colour correction and histogram stretch */
   int flags;
};
   
enum {
   pixChannelGR, pixChannelR, pixChannelB, pixChannelGB
};
   
struct PalmPixHeader {
   int w, h, resolution, zoom, num;
   int year, month, day, hour, min, sec;
   int numRec, thumbLen;
   int chansize[4];
};
   
enum {
   pixName = 0x01, pixThumbnail = 0x02, pixPixmap = 0x04
};
   
/* picture output types */
#define PALMPIX_OUT_PPM    1
#define PALMPIX_OUT_PNG    2
   
/* Returns the number of bytes from the buffer that were consumed, or 0 on
   error (generally the record not in fact being a PalmPixHeader).  */
extern int unpack_PalmPixHeader
     PI_ARGS ((struct PalmPixHeader *h, const unsigned char *p, int len));
   
extern int unpack_PalmPix
     PI_ARGS ((struct PalmPixState *state,
	       const struct PalmPixHeader *h, int recno, int wanted));
   
extern int free_PalmPix_data
     PI_ARGS ((struct PalmPixState *state));
   
#ifdef __cplusplus 
}
#endif

#endif
