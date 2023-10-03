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
 *      Random number LUT.
 *
 * 1/19/98 killough: Rewrote random number generator for better randomness,
 * while at the same time maintaining demo sync and backward compatibility.
 *
 * 2/16/98 killough: Made each RNG local to each control-equivalent block,
 * to reduce the chances of demo sync problems.
 *
 *-----------------------------------------------------------------------------*/


#include "doomstat.h"
#include "m_random.h"
#include "lprintf.h"
#include "tables.h"

//
// M_Random
// Returns a 0-255 number
//
static const unsigned char doom_rndtable[256] = { // 1/19/98 killough -- made const
    0,   8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66 ,
    74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36 ,
    95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188 ,
    52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224 ,
    149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242 ,
    145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0 ,
    175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235 ,
    25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113 ,
    94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75 ,
    136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196 ,
    135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113 ,
    80, 250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241 ,
    24, 223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224 ,
    145, 224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95 ,
    28, 139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226 ,
    71,  17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36 ,
    17,  46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106 ,
    197, 242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136 ,
    120, 163, 236, 249
};

static const unsigned char hexen_rndtable[256] = {
    201, 1, 243, 19, 18, 42, 183, 203, 101, 123, 154, 137, 34, 118, 10, 216,
    135, 246, 0, 107, 133, 229, 35, 113, 177, 211, 110, 17, 139, 84, 251, 235,
    182, 166, 161, 230, 143, 91, 24, 81, 22, 94, 7, 51, 232, 104, 122, 248,
    175, 138, 127, 171, 222, 213, 44, 16, 9, 33, 88, 102, 170, 150, 136, 114,
    62, 3, 142, 237, 6, 252, 249, 56, 74, 30, 13, 21, 180, 199, 32, 132,
    187, 234, 78, 210, 46, 131, 197, 8, 206, 244, 73, 4, 236, 178, 195, 70,
    121, 97, 167, 217, 103, 40, 247, 186, 105, 39, 95, 163, 99, 149, 253, 29,
    119, 83, 254, 26, 202, 65, 130, 155, 60, 64, 184, 106, 221, 93, 164, 196,
    112, 108, 179, 141, 54, 109, 11, 126, 75, 165, 191, 227, 87, 225, 156, 15,
    98, 162, 116, 79, 169, 140, 190, 205, 168, 194, 41, 250, 27, 20, 14, 241,
    50, 214, 72, 192, 220, 233, 67, 148, 96, 185, 176, 181, 215, 207, 172, 85,
    89, 90, 209, 128, 124, 2, 55, 173, 66, 152, 47, 129, 59, 43, 159, 240,
    239, 12, 189, 212, 144, 28, 200, 77, 219, 198, 134, 228, 45, 92, 125, 151,
    5, 53, 255, 52, 68, 245, 160, 158, 61, 86, 58, 82, 117, 37, 242, 145,
    69, 188, 115, 76, 63, 100, 49, 111, 153, 80, 38, 57, 174, 224, 71, 231,
    23, 25, 48, 218, 120, 147, 208, 36, 226, 223, 193, 238, 157, 204, 146, 31
};

static const unsigned char *rndtable = doom_rndtable;

rng_t rng;     // the random number state

unsigned int rngseed = 1993;   // killough 3/26/98: The seed

int (P_Random)(pr_class_t pr_class)
{
  // killough 2/16/98:  We always update both sets of random number
  // generators, to ensure repeatability if the demo_compatibility
  // flag is changed while the program is running. Changing the
  // demo_compatibility flag does not change the sequences generated,
  // only which one is selected from.
  //
  // All of this RNG stuff is tricky as far as demo sync goes --
  // it's like playing with explosives :) Lee

  int compat = pr_class == pr_misc ?
    (rng.prndindex = (rng.prndindex + 1) & 255) :
    (rng. rndindex = (rng. rndindex + 1) & 255) ;

  unsigned long boom;

  // killough 3/31/98:
  // If demo sync insurance is not requested, use
  // much more unstable method by putting everything
  // except pr_misc into pr_all_in_one

  if (pr_class != pr_misc && !demo_insurance)      // killough 3/31/98
    pr_class = pr_all_in_one;

  boom = rng.seed[pr_class];

  // killough 3/26/98: add pr_class*2 to addend

  rng.seed[pr_class] = boom * 1664525ul + 221297ul + pr_class*2;

  if (demo_compatibility)
    return rndtable[compat];

  boom >>= 20;

  /* killough 3/30/98: use gametic-levelstarttic to shuffle RNG
   * killough 3/31/98: but only if demo insurance requested,
   * since it's unnecessary for random shuffling otherwise
   * killough 9/29/98: but use basetic now instead of levelstarttic
   * cph - DEMOSYNC - this change makes MBF demos work,
   *       but does it break Boom ones?
   */

  if (demo_insurance)
    boom += boom_logictic * 7;

  return boom & 255;
}

// Initialize all the seeds
//
// This initialization method is critical to maintaining demo sync.
// Each seed is initialized according to its class, so if new classes
// are added they must be added to end of pr_class_t list. killough
//

void M_ClearRandom (void)
{
  int i;
  unsigned int seed = rngseed*2+1;     // add 3/26/98: add rngseed
  for (i=0; i<NUMPRCLASS; i++)         // go through each pr_class and set
    rng.seed[i] = seed *= 69069ul;     // each starting seed differently
  rng.prndindex = rng.rndindex = 0;    // clear two compatibility indices
}

// [XA] Common random formulas used by codepointers

//
// P_RandomHitscanAngle
// Outputs a random angle between (-spread, spread), as an int ('cause it can be negative).
//   spread: Maximum angle (degrees, in fixed point -- not BAM!)
//
int P_RandomHitscanAngle(pr_class_t pr_class, fixed_t spread)
{
  int t;
  int64_t spread_bam;

  // FixedToAngle doesn't work for negative numbers,
  // so for convenience take just the absolute value.
  spread_bam = (spread < 0 ? FixedToAngle(-spread) : FixedToAngle(spread));
  t = P_Random(pr_class);
  return (int)((spread_bam * (t - P_Random(pr_class))) / 255);
}

//
// P_RandomHitscanSlope
// Outputs a random angle between (-spread, spread), converted to values used for slope
//   spread: Maximum vertical angle (degrees, in fixed point -- not BAM!)
//
int P_RandomHitscanSlope(pr_class_t pr_class, fixed_t spread)
{
  int angle;

  angle = P_RandomHitscanAngle(pr_class, spread);

  // clamp it, yo
  if (angle > ANG90)
    return finetangent[0];
  else if (-angle > ANG90)
    return finetangent[FINEANGLES/2 - 1];
  else
    return finetangent[(ANG90 - angle) >> ANGLETOFINESHIFT];
}

// heretic

int P_SubRandom (void)
{
    int r = P_Random(pr_heretic);
    return r - P_Random(pr_heretic);
}

// hexen

void P_UseHexenRNG(void)
{
  rndtable = hexen_rndtable;
}
