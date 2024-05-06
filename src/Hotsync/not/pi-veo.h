/*
 * $Id: pi-veo.h,v 1.4 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-veo.h: Veo camera device support
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

#ifndef _PILOT_VEO_H_
#define _PILOT_VEO_H_

#include "pi-appinfo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VeoAppInfo {
	int 	dirty,
	sortByPriority;
	struct 	CategoryAppInfo category;
} VeoAppInfo_t;

/* Actions */
#define VEO_ACTION_OUTPUT      0x01
#define VEO_ACTION_LIST        0x02
#define VEO_ACTION_OUTPUT_ONE  0x04
   
/* Output type */
#define VEO_OUT_PPM       0x01
#define VEO_OUT_PNG       0x02

typedef struct Veo {
   unsigned char   res1[1];

   /* 0 = high, 1 = med, 2 = low this must mean something to the desktop
      conduit side, because it doesn't change anything on the Palm */
   unsigned char   quality;

   /* 0 = 640x480, 1 = 320x240 */
   unsigned char   resolution;
   unsigned char   res2[12];
   unsigned long   picnum;
   unsigned short  day, month, year;
   
   /* These are not in the Palm db header. They're used by the decoder */
   unsigned short  width, height;
   int  sd, db;
   char            name[32];
} Veo_t;

void free_Veo(Veo_t *v );
int unpack_Veo(Veo_t *v, unsigned char *buffer, size_t len);
int unpack_VeoAppInfo(VeoAppInfo_t *vai, unsigned char *record, size_t len);
int pack_Veo(Veo_t *v, unsigned char *buffer, size_t len);
int pack_VeoAppInfo(VeoAppInfo_t *vai, unsigned char *record, size_t len);

#ifdef __cplusplus
}
#endif				/*__cplusplus*/

#endif
