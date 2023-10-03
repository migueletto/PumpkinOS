/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Lookup tables.
 *      Do not try to look them up :-).
 *      In the order of appearance:
 *
 *      int finetangent[4096]   - Tangens LUT.
 *       Should work with BAM fairly well (12 of 16bit,
 *      effectively, by shifting).
 *
 *      int finesine[10240]             - Sine lookup.
 *       Guess what, serves as cosine, too.
 *       Remarkable thing is, how to use BAMs with this?
 *
 *      int tantoangle[2049]    - ArcTan LUT,
 *        maps tan(angle) to angle fast. Gotta search.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __TABLES__
#define __TABLES__

#include "m_fixed.h"

#define FINEANGLES              8192
#define FINEMASK                (FINEANGLES-1)

// 0x100000000 to 0x2000
#define ANGLETOFINESHIFT        19

// Binary Angle Measument, BAM.
#define ANG45   0x20000000
#define ANG90   0x40000000
#define ANG135  0x60000000
#define ANG180  0x80000000
#define ANG225  0xa0000000
#define ANG270  0xc0000000
#define ANG315  0xe0000000
#define ANG1      (ANG45/45)
#define ANG60     (ANG180 / 3)
#define ANGLE_MAX 0xffffffff
#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#define SLOPERANGE 2048
#define SLOPEBITS    11
#define DBITS      (FRACBITS-SLOPEBITS)

typedef unsigned angle_t;

#define ANGLE_T_TO_PITCH_F(x) ((float) ((x) >> ANGLETOFINESHIFT) * 360.0f / FINEANGLES)
#define ANGLE_T_TO_LOOKDIR(x) ((int) -((int) (x) * M_PI / ANG1))

// lookdir range is -110 (down) to 90 (up)
// pitch is -lookdir * ang1 / pi
static const angle_t raven_angle_down_limit = (angle_t) (int) (110 * ANG1 / M_PI);
static const angle_t raven_angle_up_limit = (angle_t) (int) (-90 * ANG1 / M_PI);
#define RAVEN_PITCH_UP_LIMIT ANGLE_T_TO_PITCH_F(raven_angle_up_limit)

// Load trig tables if needed
void R_LoadTrigTables(void);

// Effective size is 10240.
extern fixed_t finesine[5*FINEANGLES/4];

// Re-use data, is just PI/2 phase shift.
static fixed_t *const finecosine = finesine + (FINEANGLES/4);

// Effective size is 4096.
extern fixed_t finetangent[FINEANGLES/2];

// Effective size is 2049;
// The +1 size is to handle the case when x==y without additional checking.

extern angle_t tantoangle[SLOPERANGE+1];

// Utility function, called by R_PointToAngle.
typedef int (*slope_div_fn)(unsigned int num, unsigned int den);
int SlopeDiv(unsigned int num, unsigned int den);
int SlopeDivEx(unsigned int num, unsigned int den);

// More utility functions, courtesy of Quasar (James Haley).
// These are straight from Eternity so demos stay in sync.
inline static angle_t FixedToAngle(fixed_t a)
{
  return (angle_t)(((uint64_t)a * ANG1) >> FRACBITS);
}

inline static fixed_t AngleToFixed(angle_t a)
{
  return (fixed_t)(((uint64_t)a << FRACBITS) / ANG1);
}

// [XA] Clamped angle->slope, for convenience
inline static fixed_t AngleToSlope(int a)
{
  if (a > ANG90)
    return finetangent[0];
  else if (-a > ANG90)
    return finetangent[FINEANGLES / 2 - 1];
  else
    return finetangent[(ANG90 - a) >> ANGLETOFINESHIFT];
}

// [XA] Ditto, using fixed-point-degrees input
inline static fixed_t DegToSlope(fixed_t a)
{
  if (a >= 0)
    return AngleToSlope(FixedToAngle(a));
  else
    return AngleToSlope(-(int)FixedToAngle(-a));
}

#endif
