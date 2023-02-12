
/*
 * @(#)restype.c
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
 *     18-Aug-2000 RMa
 *                 creation
 *     Jan-2001    Regis Nicolas
 *                 Merged 68K and LE32 version into one binary
 */

#include "restype.h"
#include "pilrc.h"

char *kPalmResType[kMaxNumberResType];

/**
 * Resource Type initialization based on the target type (68K/LE32).
 */
void
ResTypeInit(void)
{
  kPalmResType[0] = "tAIN";
  kPalmResType[1] = "tSTR";
  kPalmResType[2] = "tver";
  kPalmResType[3] = "tAIS";
  kPalmResType[4] = "taic";
  kPalmResType[5] = "tSTL";
  kPalmResType[14] = "MIDI";
  kPalmResType[20] = "APPL";
  kPalmResType[21] = "TRAP";
  kPalmResType[22] = "tTTL";
  kPalmResType[23] = "tLBL";
  kPalmResType[37] = "tSCH";
  kPalmResType[39] = "fnav";
  if (vfLE32)
  {
    kPalmResType[6] = "aaib";
    kPalmResType[7] = "abmp";
    kPalmResType[8] = "absb";
    kPalmResType[9] = "aalt";
    kPalmResType[10] = "akbd";
    kPalmResType[11] = "amnu";
    kPalmResType[12] = "afnt";
    kPalmResType[13] = "afti";
    kPalmResType[15] = "aclt";
    kPalmResType[16] = "acbr";
    kPalmResType[17] = "aint";
    kPalmResType[18] = "afrm";
    kPalmResType[19] = "awrd";
    kPalmResType[24] = "aslk";
    kPalmResType[25] = "acty";
    kPalmResType[26] = "afea";
    kPalmResType[27] = "akbd";
    kPalmResType[28] = "adwd";
    kPalmResType[29] = "abyt";
    kPalmResType[30] = "alcs";
    kPalmResType[31] = "abda";
    kPalmResType[32] = "aprf";
    kPalmResType[33] = "aftm";
    kPalmResType[34] = "attl";
    kPalmResType[35] = "acsl";
    kPalmResType[36] = "attb";
    kPalmResType[38] = "afnx";
  }
  else
  {
    kPalmResType[6] = "tAIB";
    kPalmResType[7] = "Tbmp";
    kPalmResType[8] = "Tbsb";
    kPalmResType[9] = "Talt";
    kPalmResType[10] = "tkbd";
    kPalmResType[11] = "MBAR";
    kPalmResType[12] = "NFNT";
    kPalmResType[13] = "fnti";
    kPalmResType[15] = "tclt";
    kPalmResType[16] = "tcbr";
    kPalmResType[17] = "tint";
    kPalmResType[18] = "tFRM";
    kPalmResType[19] = "wrdl";
    kPalmResType[24] = "silk";
    kPalmResType[25] = "cnty";
    kPalmResType[26] = "feat";
    kPalmResType[27] = "tkbd";
    kPalmResType[28] = "DLST";
    kPalmResType[29] = "BLST";
    kPalmResType[30] = "locs";
    kPalmResType[31] = "hsbd";
    kPalmResType[32] = "pref";
    kPalmResType[33] = "fntm";
    kPalmResType[34] = "ttli";
    kPalmResType[35] = "csli";
    kPalmResType[36] = "ttbl";
    kPalmResType[38] = "nfnt";
  }
}
