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
 *  Intermission screens.
 *
 *-----------------------------------------------------------------------------
 */

#include "doomstat.h"
#include "m_random.h"
#include "w_wad.h"
#include "g_game.h"
#include "r_main.h"
#include "v_video.h"
#include "wi_stuff.h"
#include "s_sound.h"
#include "sounds.h"
#include "lprintf.h"  // jff 08/03/98 - declaration of lprintf
#include "r_draw.h"
#include "hu_stuff.h"

#include "dsda/exhud.h"
#include "dsda/font.h"
#include "dsda/mapinfo.h"

#include "heretic/in_lude.h"
#include "hexen/in_lude.h"

// Ty 03/17/98: flag that new par times have been loaded in d_deh
extern dboolean deh_pars;

//
// Data needed to add patches to full screen intermission pics.
// Patches are statistics messages, and animations.
// Loads of by-pixel layout and placement, offsets etc.
//

//
// Different vetween registered DOOM (1994) and
//  Ultimate DOOM - Final edition (retail, 1995?).
// This is supposedly ignored for commercial
//  release (aka DOOM II), which had 34 maps
//  in one episode. So there.
#define NUMEPISODES 4
#define NUMMAPS     9


// Not used
// in tics
//U #define PAUSELEN    (TICRATE*2)
//U #define SCORESTEP    100
//U #define ANIMPERIOD    32
// pixel distance from "(YOU)" to "PLAYER N"
//U #define STARDIST  10
//U #define WK 1


// GLOBAL LOCATIONS
#define WI_TITLEY      2
#define WI_SPACINGY   33

// SINGLE-PLAYER STUFF
#define SP_STATSX     50
#define SP_STATSY     50

#define SP_TIMEX      8
// proff/nicolas 09/20/98 -- changed for hi-res
#define SP_TIMEY      160
//#define SP_TIMEY      (SCREENHEIGHT-32)


// NET GAME STUFF
#define NG_STATSY     50
#define NG_STATSX     (32 + V_NamePatchWidth(star)/2 + 32*!dofrags)

#define NG_SPACINGX   64


// Used to display the frags matrix at endgame
// DEATHMATCH STUFF
#define DM_MATRIXX    42
#define DM_MATRIXY    68

#define DM_SPACINGX   40

#define DM_TOTALSX   269

#define DM_KILLERSX   10
#define DM_KILLERSY  100
#define DM_VICTIMSX    5
#define DM_VICTIMSY   50


// These animation variables, structures, etc. are used for the
// DOOM/Ultimate DOOM intermission screen animations.  This is
// totally different from any sprite or texture/flat animations
typedef enum
{
  ANIM_ALWAYS,   // determined by patch entry
  ANIM_RANDOM,   // occasional
  ANIM_LEVEL     // continuous
} animenum_t;

typedef struct
{
  int   x;       // x/y coordinate pair structure
  int   y;
} point_t;


//
// Animation.
//
typedef struct
{
  animenum_t  type;

  // period in tics between animations
  int   period;

  // number of animation frames
  int   nanims;

  // location of animation
  point_t loc;

  // ALWAYS: n/a,
  // RANDOM: period deviation (<256),
  // LEVEL: level
  int   data1;

  // ALWAYS: n/a,
  // RANDOM: random base period,
  // LEVEL: n/a
  int   data2;

  /* actual graphics for frames of animations
   * cphipps - const
   */
  patchnum_t p[3];

  // following must be initialized to zero before use!

  // next value of bcnt (used in conjunction with period)
  int   nexttic;

  // last drawn animation frame
  int   lastdrawn;

  // next frame number to animate
  int   ctr;

  // used by RANDOM and LEVEL when animating
  int   state;
} wi_anim_t;


static point_t lnodes[NUMEPISODES][NUMMAPS] =
{
  // Episode 0 World Map
  {
    { 185, 164 }, // location of level 0 (CJ)
    { 148, 143 }, // location of level 1 (CJ)
    { 69, 122 },  // location of level 2 (CJ)
    { 209, 102 }, // location of level 3 (CJ)
    { 116, 89 },  // location of level 4 (CJ)
    { 166, 55 },  // location of level 5 (CJ)
    { 71, 56 },   // location of level 6 (CJ)
    { 135, 29 },  // location of level 7 (CJ)
    { 71, 24 }    // location of level 8 (CJ)
  },

  // Episode 1 World Map should go here
  {
    { 254, 25 },  // location of level 0 (CJ)
    { 97, 50 },   // location of level 1 (CJ)
    { 188, 64 },  // location of level 2 (CJ)
    { 128, 78 },  // location of level 3 (CJ)
    { 214, 92 },  // location of level 4 (CJ)
    { 133, 130 }, // location of level 5 (CJ)
    { 208, 136 }, // location of level 6 (CJ)
    { 148, 140 }, // location of level 7 (CJ)
    { 235, 158 }  // location of level 8 (CJ)
  },

  // Episode 2 World Map should go here
  {
    { 156, 168 }, // location of level 0 (CJ)
    { 48, 154 },  // location of level 1 (CJ)
    { 174, 95 },  // location of level 2 (CJ)
    { 265, 75 },  // location of level 3 (CJ)
    { 130, 48 },  // location of level 4 (CJ)
    { 279, 23 },  // location of level 5 (CJ)
    { 198, 48 },  // location of level 6 (CJ)
    { 140, 25 },  // location of level 7 (CJ)
    { 281, 136 }  // location of level 8 (CJ)
  }
};


//
// Animation locations for episode 0 (1).
// Using patches saves a lot of space,
//  as they replace 320x200 full screen frames.
//
static wi_anim_t epsd0animinfo[] =
{
  { ANIM_ALWAYS, TICRATE/3, 3, { 224, 104 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 184, 160 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 112, 136 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 72, 112 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 88, 96 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 64, 48 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 192, 40 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 136, 16 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 80, 16 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 64, 24 } }
};

static wi_anim_t epsd1animinfo[] =
{
  { ANIM_LEVEL,  TICRATE/3, 1, { 128, 136 }, 1 },
  { ANIM_LEVEL,  TICRATE/3, 1, { 128, 136 }, 2 },
  { ANIM_LEVEL,  TICRATE/3, 1, { 128, 136 }, 3 },
  { ANIM_LEVEL,  TICRATE/3, 1, { 128, 136 }, 4 },
  { ANIM_LEVEL,  TICRATE/3, 1, { 128, 136 }, 5 },
  { ANIM_LEVEL,  TICRATE/3, 1, { 128, 136 }, 6 },
  { ANIM_LEVEL,  TICRATE/3, 1, { 128, 136 }, 7 },
  { ANIM_LEVEL,  TICRATE/3, 3, { 192, 144 }, 8 },
  { ANIM_LEVEL,  TICRATE/3, 1, { 128, 136 }, 8 }
};

static wi_anim_t epsd2animinfo[] =
{
  { ANIM_ALWAYS, TICRATE/3, 3, { 104, 168 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 40, 136 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 160, 96 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 104, 80 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 120, 32 } },
  { ANIM_ALWAYS, TICRATE/4, 3, { 40, 0 } }
};

static int NUMANIMS[NUMEPISODES] =
{
  sizeof(epsd0animinfo)/sizeof(wi_anim_t),
  sizeof(epsd1animinfo)/sizeof(wi_anim_t),
  sizeof(epsd2animinfo)/sizeof(wi_anim_t)
};

static wi_anim_t *anims[NUMEPISODES] =
{
  epsd0animinfo,
  epsd1animinfo,
  epsd2animinfo
};


//
// GENERAL DATA
//

//
// Locally used stuff.
//
#define FB 0


// States for single-player
#define SP_KILLS    0
#define SP_ITEMS    2
#define SP_SECRET   4
#define SP_FRAGS    6
#define SP_TIME     8
#define SP_PAR      ST_TIME

#define SP_PAUSE    1

// in seconds
#define SHOWNEXTLOCDELAY  4
//#define SHOWLASTLOCDELAY  SHOWNEXTLOCDELAY


// used to accelerate or skip a stage
int   acceleratestage;           // killough 3/28/98: made global

// wbs->pnum
static int    me;

 // specifies current state
static stateenum_t  state;

// contains information passed into intermission
static wbstartstruct_t* wbs;

static wbplayerstruct_t* plrs;  // wbs->plyr[]

// used for general timing
static int    cnt;

// used for timing of background animation
static int    bcnt;

// signals to refresh everything for one frame
static int    firstrefresh;

static int    cnt_time;
static int    cnt_total_time;
static int    cnt_par;
static int    cnt_pause;

//
//  GRAPHICS
//

// You Are Here graphic
static const char* yah[3] = { "WIURH0", "WIURH1", 0 };

// splat
static const char* splat[2] = {"WISPLAT", 0};

// %, : graphics
static const char percent[] = {"WIPCNT"};
static const char colon[] = {"WICOLON"};

// 0-9 graphic
static patchnum_t num[10];

// minus sign
static const char wiminus[] = {"WIMINUS"};

// "Finished!" graphics
static const char finished[] = {"WIF"};

// "Entering" graphic
static const char entering[] = {"WIENTER"};

// "secret"
static const char sp_secret[] = {"WISCRT2"};

// "Kills", "Scrt", "Items", "Frags"
static const char kills[] = {"WIOSTK"};
static const char secret[] = {"WIOSTS"};
static const char items[] = {"WIOSTI"};
static const char frags[] = {"WIFRGS"};

// Time sucks.
static const char time1[] = {"WITIME"};
static const char par[] = {"WIPAR"};
static const char sucks[] = {"WISUCKS"};

// "killers", "victims"
static const char killers[] = {"WIKILRS"};
static const char victims[] = {"WIVCTMS"};

// "Total", your face, your dead face
static const char total[] = {"WIMSTT"};
static const char star[] = {"STFST01"};
static const char bstar[] = {"STFDEAD0"};

// "red P[1..g_maxplayers]"
static const char facebackp[] = {"STPB0"};

static const char *exitpic, *enterpic;

//
// CODE
//

static void WI_endDeathmatchStats(void);
static void WI_endNetgameStats(void);
#define WI_endStats WI_endNetgameStats

/* ====================================================================
 * WI_levelNameLump
 * Purpore: Returns the name of the graphic lump containing the name of
 *          the given level.
 * Args:    Episode and level, and buffer (must by 9 chars) to write to
 * Returns: void
 */
void WI_levelNameLump(int epis, int map, char* buf)
{
  if (gamemode == commercial) {
    sprintf(buf, "CWILV%2.2d", map);
  } else {
    sprintf(buf, "WILV%d%d", epis, map);
  }
}

static int WI_killPercent(int i)
{
  return wbs->maxkills ? (plrs[i].skills * 100) / wbs->maxkills
                       : MAX(1, plrs[i].skills) * 100;
}

static int WI_killLimit(int i)
{
  return wbs->maxkills ? (plrs[i].skills * 100) / wbs->maxkills
                       : (plrs[i].skills * 100);

}

static int WI_itemPercent(int i)
{
  return wbs->maxitems ? (plrs[i].sitems * 100) / wbs->maxitems : 100;
}

static int WI_itemLimit(int i)
{
  return wbs->maxitems ? (plrs[i].sitems * 100) / wbs->maxitems
                       : (plrs[i].sitems * 100);

}

static int WI_secretPercent(int i)
{
  return wbs->maxsecret ? (plrs[i].skills * 100) / wbs->maxkills : 100;
}

static int WI_secretLimit(int i)
{
  return wbs->maxsecret ? (plrs[i].ssecret * 100) / wbs->maxsecret
                        : compatibility_level < lxdoom_1_compatibility ? 0 : 100;
}

// ====================================================================
// WI_slamBackground
// Purpose: Put the full-screen background up prior to patches
// Args:    none
// Returns: void
//
static void WI_slamBackground(void)
{
  char  name[9];  // limited to 8 characters

  if (state != StatCount && enterpic) strcpy(name, enterpic);
  else if (exitpic) strcpy(name, exitpic);
  else if (gamemode == commercial || wbs->epsd < 0 || (gamemode == retail && wbs->epsd >= 3))
    strcpy(name, "INTERPIC");
  else
    snprintf(name, sizeof(name), "WIMAP%d", wbs->epsd);

  // e6y: wide-res
  V_ClearBorder();

  // background
  V_DrawNamePatch(0, 0, FB, name, CR_DEFAULT, VPT_STRETCH);
}

#define SPACEWIDTH 4

static void WI_DrawString(int cx, int cy, const char* ch)
{
  int   w;
  int   c;
  const char *cc = ch;
  int width = 0;

  // center the text.
  while (*cc) {
    c = *cc++;         // get next char
    c = toupper(c) - HU_FONTSTART;
    if (c < 0 || c> HU_FONTSIZE)
    {
      width += SPACEWIDTH;    // space
      continue;
    }
    width += hud_font.font[c].width;
  }
  cx -= width / 2;
  if (cx < 0) cx = 0;


  while (*ch) {
    c = *ch++;         // get next char
    c = toupper(c) - HU_FONTSTART;
    if (c < 0 || c> HU_FONTSIZE)
    {
      cx += SPACEWIDTH;    // space
      continue;
    }
    w = hud_font.font[c].width;
    if (cx + w > 320)
      break;

    V_DrawNumPatch(cx, cy, 0, hud_font.font[c].lumpnum, CR_GRAY, VPT_STRETCH | VPT_TRANS);
    cx += w;
  }
}

// ====================================================================
// WI_drawLF
// Purpose: Draw the "Finished" level name before showing stats
// Args:    none
// Returns: void
//
const char *lf_levelname;
const char *lf_levelpic;
const char *lf_author;

void WI_drawLF(void)
{
  int y = WI_TITLEY;
  char lname[9];

  dsda_PrepareFinished();

  if (lf_levelname)
  {
    // The level defines a new name but no texture for the name.
    WI_DrawString(160, y, lf_levelname);

    y += (5 * hud_font.height / 4);
  }
  else
  {
    // draw <LevelName>
    /* cph - get the graphic lump name and use it */
    if (lf_levelpic)
      strcpy(lname, lf_levelpic);
    else
      WI_levelNameLump(wbs->epsd, wbs->last, lname);

    if (!W_LumpNameExists(lname))
      return;

    // CPhipps - patch drawing updated
    V_DrawNamePatch((320 - V_NamePatchWidth(lname)) / 2, y,
      FB, lname, CR_DEFAULT, VPT_STRETCH);

    y += (5 * V_NamePatchHeight(lname)) / 4;
  }

  if (lf_author)
  {
    WI_DrawString(160, y, lf_author);

    y += (5 * hud_font.height / 4);
  }

  // CPhipps - patch drawing updated
  // draw "Finished!"
  V_DrawNamePatch((320 - V_NamePatchWidth(finished))/2, y,
     FB, finished, CR_DEFAULT, VPT_STRETCH);
}


// ====================================================================
// WI_drawEL
// Purpose: Draw introductory "Entering" and level name
// Args:    none
// Returns: void
//
const char *el_levelname;
const char *el_levelpic;
const char *el_author;

void WI_drawEL(void)
{
  int y = WI_TITLEY;
  char lname[9];

  // draw "Entering"
  // CPhipps - patch drawing updated
  V_DrawNamePatch((320 - V_NamePatchWidth(entering)) / 2,
    y, FB, entering, CR_DEFAULT, VPT_STRETCH);

  y += (5 * V_NamePatchHeight(entering)) / 4;

  dsda_PrepareEntering();

  if (el_levelname)
  {
    // The level defines a new name but no texture for the name.
    WI_DrawString(160, y, el_levelname);

    y += (5 * hud_font.height / 4);
  }
  else
  {
    /* cph - get the graphic lump name */
    if (el_levelpic)
      strcpy(lname, el_levelpic);
    else
      WI_levelNameLump(wbs->nextep, wbs->next, lname);

    if (!W_LumpNameExists(lname))
      return;

    // CPhipps - patch drawing updated
    // draw level
    V_DrawNamePatch((320 - V_NamePatchWidth(lname)) / 2, y, FB,
      lname, CR_DEFAULT, VPT_STRETCH);

    y += (5 * V_NamePatchHeight(lname)) / 4;
  }

  if (el_author)
  {
    WI_DrawString(160, y, el_author);

    y += (5 * hud_font.height / 4);
  }
}

/* ====================================================================
 * WI_drawOnLnode
 * Purpose: Draw patches at a location based on episode/map
 * Args:    n   -- index to map# within episode
 *          c[] -- array of names of patches to be drawn
 * Returns: void
 */
void
WI_drawOnLnode  // draw stuff at a location by episode/map#
( int   n,
  const char* const c[] )
{
  int   i;
  dboolean fits = false;

  i = 0;
  do
  {
    int            left;
    int            top;
    int            right;
    int            bottom;
    const rpatch_t* patch = R_PatchByName(c[i]);

    left = lnodes[wbs->epsd][n].x - patch->leftoffset;
    top = lnodes[wbs->epsd][n].y - patch->topoffset;
    right = left + patch->width;
    bottom = top + patch->height;

    if (left >= 0
       && right < 320
       && top >= 0
       && bottom < 200)
    {
      fits = true;
    }
    else
    {
      i++;
    }
  } while (!fits && i!=2 && c[i]);

  if (fits && i<2)
  {
    // CPhipps - patch drawing updated
    V_DrawNamePatch(lnodes[wbs->epsd][n].x, lnodes[wbs->epsd][n].y,
       FB, c[i], CR_DEFAULT, VPT_STRETCH);
  }
  else
  {
    // DEBUG
    //jff 8/3/98 use logical output routine
    lprintf(LO_DEBUG,"Could not place patch on level %d\n", n+1);
  }
}


// ====================================================================
// WI_initAnimatedBack
// Purpose: Initialize pointers and styles for background animation
// Args:    none
// Returns: void
//
void WI_initAnimatedBack(int entering)
{
  int   i;
  wi_anim_t* a;

  if (exitpic) return;
  if (enterpic && entering) return;

  if (gamemode == commercial)  // no animation for DOOM2
    return;

  if (wbs->epsd < 0 || wbs->epsd > 2)
    return;


  for (i=0;i<NUMANIMS[wbs->epsd];i++)
  {
    a = &anims[wbs->epsd][i];

    // init variables
    a->ctr = -1;

    // specify the next time to draw it
    if (a->type == ANIM_ALWAYS)
      a->nexttic = bcnt + 1 + (M_Random()%a->period);
    else
      if (a->type == ANIM_RANDOM)
        a->nexttic = bcnt + 1 + a->data2+(M_Random()%a->data1);
      else
        if (a->type == ANIM_LEVEL)
          a->nexttic = bcnt + 1;
  }
}


// ====================================================================
// WI_updateAnimatedBack
// Purpose: Figure out what animation we do on this iteration
// Args:    none
// Returns: void
//
void WI_updateAnimatedBack(void)
{
  int   i;
  wi_anim_t* a;

  if (exitpic) return;
  if (enterpic && state != StatCount) return;

  if (gamemode == commercial)
    return;

  if (wbs->epsd < 0 || wbs->epsd > 2)
    return;

  for (i=0;i<NUMANIMS[wbs->epsd];i++)
  {
    a = &anims[wbs->epsd][i];

    if (bcnt == a->nexttic)
    {
      switch (a->type)
      {
        case ANIM_ALWAYS:
             if (++a->ctr >= a->nanims) a->ctr = 0;
             a->nexttic = bcnt + a->period;
             break;

        case ANIM_RANDOM:
             a->ctr++;
             if (a->ctr == a->nanims)
             {
               a->ctr = -1;
               a->nexttic = bcnt+a->data2+(M_Random()%a->data1);
             }
             else
               a->nexttic = bcnt + a->period;
             break;

        case ANIM_LEVEL:
             // gawd-awful hack for level anims
             if (!(state == StatCount && i == 7)
                && wbs->next == a->data1)
             {
               a->ctr++;
               if (a->ctr == a->nanims) a->ctr--;
               a->nexttic = bcnt + a->period;
             }
             break;
      }
    }
  }
}


// ====================================================================
// WI_drawAnimatedBack
// Purpose: Actually do the animation (whew!)
// Args:    none
// Returns: void
//
void WI_drawAnimatedBack(void)
{
  int     i;
  wi_anim_t*   a;

  if (exitpic) return;
  if (enterpic && state != StatCount) return;

  if (gamemode==commercial) //jff 4/25/98 Someone forgot commercial an enum
    return;

  if (wbs->epsd < 0 || wbs->epsd > 2)
    return;

  for (i=0 ; i<NUMANIMS[wbs->epsd] ; i++)
  {
    a = &anims[wbs->epsd][i];

    if (a->ctr >= 0)
      // CPhipps - patch drawing updated
      V_DrawNumPatch(a->loc.x, a->loc.y, FB, a->p[a->ctr].lumpnum, CR_DEFAULT, VPT_STRETCH);
  }
}


// ====================================================================
// WI_drawNum
// Purpose: Draws a number.  If digits > 0, then use that many digits
//          minimum, otherwise only use as many as necessary
// Args:    x, y   -- location
//          n      -- the number to be drawn
//          digits -- number of digits minimum or zero
// Returns: new x position after drawing (note we are going to the left)
// CPhipps - static
static int WI_drawNum (int x, int y, int n, int digits)
{
  int   fontwidth = num[0].width;
  int   neg;
  int   temp;

  if (digits < 0)
  {
    if (!n)
    {
      // make variable-length zeros 1 digit long
      digits = 1;
    }
    else
    {
      // figure out # of digits in #
      digits = 0;
      temp = n;

      while (temp)
      {
        temp /= 10;
        digits++;
      }
    }
  }

  neg = n < 0;
  if (neg)
    n = -n;

  // if non-number, do not draw it
  if (n == 1994)
    return 0;

  // draw the new number
  while (digits--)
  {
    x -= fontwidth;
    // CPhipps - patch drawing updated
    V_DrawNumPatch(x, y, FB, num[ n % 10 ].lumpnum, CR_DEFAULT, VPT_STRETCH);
    n /= 10;
  }

  // draw a minus sign if necessary
  if (neg)
    // CPhipps - patch drawing updated
    V_DrawNamePatch(x-=8, y, FB, wiminus, CR_DEFAULT, VPT_STRETCH);

  return x;
}


// ====================================================================
// WI_drawPercent
// Purpose: Draws a percentage, really just a call to WI_drawNum
//          after putting a percent sign out there
// Args:    x, y   -- location
//          p      -- the percentage value to be drawn, no negatives
// Returns: void
// CPhipps - static
static void WI_drawPercent(int x, int y, int p)
{
  if (p < 0)
    return;

  // CPhipps - patch drawing updated
  V_DrawNamePatch(x, y, FB, percent, CR_DEFAULT, VPT_STRETCH);
  WI_drawNum(x, y, p, -1);
}


// ====================================================================
// WI_drawTime
// Purpose: Draws the level completion time or par time, or "Sucks"
//          if 1 hour or more
// Args:    x, y   -- location
//          t      -- the time value to be drawn
// Returns: void
//
// CPhipps - static
//         - largely rewritten to display hours and use slightly better algorithm

static void WI_drawTime(int x, int y, int t)
{
  int   n;

  if (t<0)
    return;

  if (t < 100*60*60)
    for(;;) {
      n = t % 60;
      t /= 60;
      x = WI_drawNum(x, y, n, (t || n>9) ? 2 : 1) - V_NamePatchWidth(colon);

      // draw
      if (t)
  // CPhipps - patch drawing updated
        V_DrawNamePatch(x, y, FB, colon, CR_DEFAULT, VPT_STRETCH);
      else break;
    }
  else // "sucks" (maybe should be "addicted", even I've never had a 100 hour game ;)
    V_DrawNamePatch(x - V_NamePatchWidth(sucks),
        y, FB, sucks, CR_DEFAULT, VPT_STRETCH);
}


// ====================================================================
// WI_End
// Purpose: Unloads data structures (inverse of WI_Start)
// Args:    none
// Returns: void
//
void WI_End(void)
{
  if (heretic) return;

  if (deathmatch)
    WI_endDeathmatchStats();
  else if (netgame)
    WI_endNetgameStats();
  else
    WI_endStats();
}


// ====================================================================
// WI_initNoState
// Purpose: Clear state, ready for end of level activity
// Args:    none
// Returns: void
//
void WI_initNoState(void)
{
  state = NoState;
  acceleratestage = 0;
  cnt = 10;
}


// ====================================================================
// WI_drawTimeStats
// Purpose: Put the times on the screen
// Args:    time, total time, par time, in seconds
// Returns: void
//
// cph - pulled from WI_drawStats below

static void WI_drawTimeStats(int cnt_time, int cnt_total_time, int cnt_par)
{
  V_DrawNamePatch(SP_TIMEX, SP_TIMEY, FB, time1, CR_DEFAULT, VPT_STRETCH);
  WI_drawTime(320/2 - SP_TIMEX, SP_TIMEY, cnt_time);

  V_DrawNamePatch(SP_TIMEX, (SP_TIMEY+200)/2, FB, total, CR_DEFAULT, VPT_STRETCH);
  WI_drawTime(320/2 - SP_TIMEX, (SP_TIMEY+200)/2, cnt_total_time);

  // Ty 04/11/98: redid logic: should skip only if with pwad but
  // without deh patch
  // killough 2/22/98: skip drawing par times on pwads
  // Ty 03/17/98: unless pars changed with deh patch

  if (
    !(modifiedgame && !wbs->modified_partime)
    || (gamemission == pack_nerve && allow_incompatibility)
  )
  {
    if (wbs->epsd < 4 || wbs->modified_partime)
    {
      int display_count;

      display_count =
        wbs->fake_partime && cnt_par >= 0 ? wbs->fake_partime / TICRATE : cnt_par;

      V_DrawNamePatch(320/2 + SP_TIMEX, SP_TIMEY, FB, par, CR_DEFAULT, VPT_STRETCH);
      WI_drawTime(320 - SP_TIMEX, SP_TIMEY, display_count);
    }
  }

  dsda_DrawExIntermission();
}

// ====================================================================
// WI_updateNoState
// Purpose: Cycle until end of level activity is done
// Args:    none
// Returns: void
//
void WI_updateNoState(void)
{

  WI_updateAnimatedBack();

  if (!--cnt)
    G_WorldDone();
}

static dboolean    snl_pointeron = false;


// ====================================================================
// WI_initShowNextLoc
// Purpose: Prepare to show the next level's location
// Args:    none
// Returns: void
//
void WI_initShowNextLoc(void)
{
  int behaviour;

  dsda_ShowNextLocBehaviour(&behaviour);

  if (behaviour & WI_SHOW_NEXT_DONE)
  {
    G_WorldDone();
    return;
  }

  if (behaviour & WI_SHOW_NEXT_LOC)
  {
    state = ShowNextLoc;
  }

  if (behaviour & WI_SHOW_NEXT_EPISODAL)
  {
    // episode change possible
    if (wbs->epsd != wbs->nextep)
    {
      void WI_loadData(void);

      wbs->epsd = wbs->nextep;
      wbs->last = wbs->next - 1;
      WI_loadData();
    }
  }

  acceleratestage = 0;

  // e6y: That was pretty easy - only a HEX editor and luck
  // There is no more desync on ddt-tas.zip\e4tux231.lmp
  // --------- tasdoom.idb ---------
  // .text:00031194 loc_31194:      ; CODE XREF: WI_updateStats+3A9j
  // .text:00031194                 mov     ds:state, 1
  // .text:0003119E                 mov     ds:acceleratestage, 0
  // .text:000311A8                 mov     ds:cnt, 3Ch
  // nowhere no hide
  if (compatibility_level == tasdoom_compatibility)
    cnt = 60;
  else
    cnt = SHOWNEXTLOCDELAY * TICRATE;

  WI_initAnimatedBack(true);
}


// ====================================================================
// WI_updateShowNextLoc
// Purpose: Prepare to show the next level's location
// Args:    none
// Returns: void
//
void WI_updateShowNextLoc(void)
{
  WI_updateAnimatedBack();

  if (!--cnt || acceleratestage)
    WI_initNoState();
  else
    snl_pointeron = (cnt & 31) < 20;
}


// ====================================================================
// WI_drawShowNextLoc
// Purpose: Show the next level's location on animated backgrounds
// Args:    none
// Returns: void
//
void WI_drawShowNextLoc(void)
{
  int   i;
  int   last;

  if (dsda_SkipDrawShowNextLoc())
  {
    return;
  }

  WI_slamBackground();

  // draw animated background
  WI_drawAnimatedBack();

  // custom interpic.
  if (exitpic || (enterpic && state != StatCount))
  {
    WI_drawEL();
    return;
  }

  if ( gamemode != commercial)
  {
    if (wbs->epsd < 0 || wbs->epsd > 2)
    {
      WI_drawEL();  // "Entering..." if not E1 or E2
      return;
    }

    last = (wbs->last == 8) ? wbs->next - 1 : wbs->last;

    // draw a splat on taken cities.
    for (i=0 ; i<=last ; i++)
      WI_drawOnLnode(i, splat);

    // splat the secret level?
    if (wbs->didsecret)
      WI_drawOnLnode(8, splat);

    // draw flashing ptr
    if (snl_pointeron)
      WI_drawOnLnode(wbs->next, yah);
  }

  if (gamemission == pack_nerve && allow_incompatibility && wbs->last == 7)
    return; // MAP08 end game

  // draws which level you are entering..
  if ( (gamemode != commercial)
     || wbs->next != 30)  // check for MAP30 end game
  WI_drawEL();
}

// ====================================================================
// WI_drawNoState
// Purpose: Draw the pointer and next location
// Args:    none
// Returns: void
//
void WI_drawNoState(void)
{
  snl_pointeron = true;
  WI_drawShowNextLoc();
}

// ====================================================================
// WI_fragSum
// Purpose: Calculate frags for this player based on the current totals
//          of all the other players.  Subtract self-frags.
// Args:    playernum -- the player to be calculated
// Returns: the total frags for this player
//
int WI_fragSum(int playernum)
{
  int   i;
  int   frags = 0;

  for (i = 0; i < g_maxplayers; i++)
  {
    if (playeringame[i]  // is this player playing?
       && i!=playernum) // and it's not the player we're calculating
    {
      frags += plrs[playernum].frags[i];
    }
  }


  // JDC hack - negative frags.
  frags -= plrs[playernum].frags[playernum];

  return frags;
}

static int          dm_state;
// CPhipps - short, dynamically allocated
static short int  **dm_frags;  // frags matrix
static short int   *dm_totals;  // totals by player

// ====================================================================
// WI_initDeathmatchStats
// Purpose: Set up to display DM stats at end of level.  Calculate
//          frags for all players.
// Args:    none
// Returns: void
//
void WI_initDeathmatchStats(void)
{
  int   i; // looping variables

  // CPhipps - allocate data structures needed
  dm_frags  = Z_Calloc(g_maxplayers, sizeof(*dm_frags));
  dm_totals = Z_Calloc(g_maxplayers, sizeof(*dm_totals));

  state = StatCount;  // We're doing stats
  acceleratestage = 0;
  dm_state = 1;  // count how many times we've done a complete stat

  cnt_pause = TICRATE;

  for (i = 0 ;i < g_maxplayers; i++)
  {
    if (playeringame[i])
    {
      // CPhipps - allocate frags line
      dm_frags[i] = Z_Calloc(g_maxplayers, sizeof(**dm_frags)); // set all counts to zero

      dm_totals[i] = 0;
    }
  }
  WI_initAnimatedBack(false);
}

// ====================================================================
// CPhipps - WI_endDeathmatchStats
// Purpose: Deallocate dynamically allocated DM stats data
// Args:    none
// Returns: void
//

void WI_endDeathmatchStats(void)
{
  int i;
  for (i = 0; i < g_maxplayers; i++)
    Z_Free(dm_frags[i]);

  Z_Free(dm_frags); Z_Free(dm_totals);
}

// ====================================================================
// WI_updateDeathmatchStats
// Purpose: Advance Deathmatch stats screen animation.  Calculate
//          frags for all players.  Lots of noise and drama around
//          the presentation.
// Args:    none
// Returns: void
//
void WI_updateDeathmatchStats(void)
{
  int   i;
  int   j;

  dboolean stillticking;

  WI_updateAnimatedBack();

  if (acceleratestage && dm_state != 4)  // still ticking
  {
    acceleratestage = 0;

    for (i = 0; i < g_maxplayers; i++)
    {
      if (playeringame[i])
      {
        for (j = 0; j < g_maxplayers; j++)
          if (playeringame[j])
            dm_frags[i][j] = plrs[i].frags[j];

        dm_totals[i] = WI_fragSum(i);
      }
    }


    S_StartVoidSound(sfx_barexp);  // bang
    dm_state = 4;  // we're done with all 4 (or all we have to do)
  }


  if (dm_state == 2)
  {
    if (!(bcnt&3))
      S_StartVoidSound(sfx_pistol);  // noise while counting

    stillticking = false;

    for (i = 0; i < g_maxplayers; i++)
    {
      if (playeringame[i])
      {
        for (j = 0; j < g_maxplayers; j++)
        {
          if (playeringame[j]
             && dm_frags[i][j] != plrs[i].frags[j])
          {
            if (plrs[i].frags[j] < 0)
              dm_frags[i][j]--;
            else
              dm_frags[i][j]++;

            if (dm_frags[i][j] > 999) // Ty 03/17/98 3-digit frag count
              dm_frags[i][j] = 999;

            if (dm_frags[i][j] < -999)
              dm_frags[i][j] = -999;

            stillticking = true;
          }
        }
        dm_totals[i] = WI_fragSum(i);

        if (dm_totals[i] > 999)
          dm_totals[i] = 999;

        if (dm_totals[i] < -999)
          dm_totals[i] = -999;  // Ty 03/17/98 end 3-digit frag count
      }
    }

    if (!stillticking)
    {
      S_StartVoidSound(sfx_barexp);
      dm_state++;
    }
  }
  else if (dm_state == 4)
  {
    if (acceleratestage)
    {
      S_StartVoidSound(sfx_slop);

      if ( gamemode == commercial)
        WI_initNoState();
      else
        WI_initShowNextLoc();
    }
  }
  else if (dm_state & 1)
  {
    if (!--cnt_pause)
    {
      dm_state++;
      cnt_pause = TICRATE;
    }
  }
}


// ====================================================================
// WI_drawDeathmatchStats
// Purpose: Draw the stats on the screen in a matrix
// Args:    none
// Returns: void
//
// proff/nicolas 09/20/98 -- changed for hi-res
// CPhipps - patch drawing updated
void WI_drawDeathmatchStats(void)
{
  int   i;
  int   j;
  int   x;
  int   y;
  int   w;

  int   halfface = V_NamePatchWidth(facebackp)/2;

  WI_slamBackground();

  // draw animated background
  WI_drawAnimatedBack();
  WI_drawLF();

  // draw stat titles (top line)
  V_DrawNamePatch(DM_TOTALSX-V_NamePatchWidth(total)/2,
     DM_MATRIXY-WI_SPACINGY+10, FB, total, CR_DEFAULT, VPT_STRETCH);

  V_DrawNamePatch(DM_KILLERSX, DM_KILLERSY, FB, killers, CR_DEFAULT, VPT_STRETCH);
  V_DrawNamePatch(DM_VICTIMSX, DM_VICTIMSY, FB, victims, CR_DEFAULT, VPT_STRETCH);

  // draw P?
  x = DM_MATRIXX + DM_SPACINGX;
  y = DM_MATRIXY;

  for (i = 0; i < g_maxplayers; i++)
  {
    if (playeringame[i]) {
      //int trans = playernumtotrans[i];
      V_DrawNamePatch(x-halfface, DM_MATRIXY - WI_SPACINGY,
         FB, facebackp, i ? CR_LIMIT+i : CR_DEFAULT,
         VPT_STRETCH | (i ? VPT_TRANS : 0));
      V_DrawNamePatch(DM_MATRIXX-halfface, y,
         FB, facebackp, i ? CR_LIMIT+i : CR_DEFAULT,
         VPT_STRETCH | (i ? VPT_TRANS : 0));

      if (i == me)
      {
        V_DrawNamePatch(x-halfface, DM_MATRIXY - WI_SPACINGY,
           FB, bstar, CR_DEFAULT, VPT_STRETCH);
        V_DrawNamePatch(DM_MATRIXX-halfface, y,
           FB, star, CR_DEFAULT, VPT_STRETCH);
      }
    }
    x += DM_SPACINGX;
    y += WI_SPACINGY;
  }

  // draw stats
  y = DM_MATRIXY+10;
  w = num[0].width;

  for (i = 0; i < g_maxplayers; i++)
  {
    x = DM_MATRIXX + DM_SPACINGX;

    if (playeringame[i])
    {
      for (j = 0; j < g_maxplayers; j++)
      {
        if (playeringame[j])
          WI_drawNum(x+w, y, dm_frags[i][j], 2);

        x += DM_SPACINGX;
      }
      WI_drawNum(DM_TOTALSX+w, y, dm_totals[i], 2);
    }
    y += WI_SPACINGY;
  }
}


//
// Note: The term "Netgame" means a coop game
//

static int *cnt_kills;
static int *cnt_items;
static int *cnt_secret;
static int *cnt_frags;
static int    dofrags;
static int    ng_state;

// ====================================================================
// CPhipps - WI_endNetgameStats
// Purpose: Clean up coop game stats
// Args:    none
// Returns: void
//
static void WI_endNetgameStats(void)
{
  Z_Free(cnt_frags); cnt_frags = NULL;
  Z_Free(cnt_secret); cnt_secret = NULL;
  Z_Free(cnt_items); cnt_items = NULL;
  Z_Free(cnt_kills); cnt_kills = NULL;
}

// ====================================================================
// WI_initNetgameStats
// Purpose: Prepare for coop game stats
// Args:    none
// Returns: void
//
void WI_initNetgameStats(void)
{
  int i;

  state = StatCount;
  acceleratestage = 0;
  ng_state = 1;

  cnt_pause = TICRATE;

  // CPhipps - allocate these dynamically, blank with calloc
  cnt_kills = Z_Calloc(g_maxplayers, sizeof(*cnt_kills));
  cnt_items = Z_Calloc(g_maxplayers, sizeof(*cnt_items));
  cnt_secret= Z_Calloc(g_maxplayers, sizeof(*cnt_secret));
  cnt_frags = Z_Calloc(g_maxplayers, sizeof(*cnt_frags));

  for (i = 0; i < g_maxplayers; i++)
    if (playeringame[i])
      dofrags += WI_fragSum(i);

  dofrags = !!dofrags; // set to true or false - did we have frags?

  WI_initAnimatedBack(false);
}


// ====================================================================
// WI_updateNetgameStats
// Purpose: Calculate coop stats as we display them with noise and fury
// Args:    none
// Returns: void
// Comment: This stuff sure is complicated for what it does
//
void WI_updateNetgameStats(void)
{
  int   i;
  int   fsum;

  dboolean stillticking;

  WI_updateAnimatedBack();

  if (acceleratestage && ng_state != 10)
  {
    acceleratestage = 0;

    for (i = 0; i < g_maxplayers; i++)
    {
      if (!playeringame[i])
        continue;

      cnt_kills[i] = WI_killPercent(i);
      cnt_items[i] = WI_itemPercent(i);
      cnt_secret[i] = WI_secretPercent(i);

      if (dofrags)
        cnt_frags[i] = WI_fragSum(i);  // we had frags
    }
    S_StartVoidSound(sfx_barexp);  // bang
    ng_state = 10;
  }

  if (ng_state == 2)
  {
    if (!(bcnt&3))
      S_StartVoidSound(sfx_pistol);  // pop

    stillticking = false;

    for (i = 0; i < g_maxplayers; i++)
    {
      if (!playeringame[i])
        continue;

      cnt_kills[i] += 2;

      if (cnt_kills[i] >= WI_killLimit(i))
        cnt_kills[i] = WI_killPercent(i);
      else
        stillticking = true; // still got stuff to tally
    }

    if (!stillticking)
    {
      S_StartVoidSound(sfx_barexp);
      ng_state++;
    }
  }
  else if (ng_state == 4)
  {
    if (!(bcnt&3))
      S_StartVoidSound(sfx_pistol);

    stillticking = false;

    for (i = 0; i < g_maxplayers; i++)
    {
      if (!playeringame[i])
        continue;

      cnt_items[i] += 2;
      if (cnt_items[i] >= WI_itemLimit(i))
        cnt_items[i] = WI_itemPercent(i);
      else
        stillticking = true;
    }

    if (!stillticking)
    {
      S_StartVoidSound(sfx_barexp);
      ng_state++;
    }
  }
  else if (ng_state == 6)
  {
    if (!(bcnt&3))
      S_StartVoidSound(sfx_pistol);

    stillticking = false;

    for (i = 0; i < g_maxplayers; i++)
    {
      if (!playeringame[i])
        continue;

      cnt_secret[i] += 2;

      if (cnt_secret[i] >= WI_secretLimit(i))
        cnt_secret[i] = WI_secretPercent(i);
      else
        stillticking = true;
    }

    if (!stillticking)
    {
      S_StartVoidSound(sfx_barexp);
      ng_state += 1 + 2*!dofrags;
    }
  }
  else if (ng_state == 8)
  {
    if (!(bcnt&3))
      S_StartVoidSound(sfx_pistol);

    stillticking = false;

    for (i = 0; i < g_maxplayers; i++)
    {
      if (!playeringame[i])
        continue;

      cnt_frags[i] += 1;

      if (cnt_frags[i] >= (fsum = WI_fragSum(i)))
        cnt_frags[i] = fsum;
      else
        stillticking = true;
    }

    if (!stillticking)
    {
      S_StartVoidSound(sfx_pldeth);
      ng_state++;
    }
  }
  else if (ng_state == 10)
  {
    if (acceleratestage)
    {
      S_StartVoidSound(sfx_sgcock);
      if ( gamemode == commercial )
        WI_initNoState();
      else
        WI_initShowNextLoc();
    }
  }
  else if (ng_state & 1)
  {
    if (!--cnt_pause)
    {
      ng_state++;
      cnt_pause = TICRATE;
    }
  }
}


// ====================================================================
// WI_drawNetgameStats
// Purpose: Put the coop stats on the screen
// Args:    none
// Returns: void
//
// proff/nicolas 09/20/98 -- changed for hi-res
// CPhipps - patch drawing updated
void WI_drawNetgameStats(void)
{
  int   i;
  int   x;
  int   y;
  int   pwidth = V_NamePatchWidth(percent);
  int   fwidth = V_NamePatchWidth(facebackp);

  WI_slamBackground();

  // draw animated background
  WI_drawAnimatedBack();

  WI_drawLF();

  // draw stat titles (top line)
  V_DrawNamePatch(NG_STATSX+NG_SPACINGX-V_NamePatchWidth(kills),
     NG_STATSY, FB, kills, CR_DEFAULT, VPT_STRETCH);

  V_DrawNamePatch(NG_STATSX+2*NG_SPACINGX-V_NamePatchWidth(items),
     NG_STATSY, FB, items, CR_DEFAULT, VPT_STRETCH);

  V_DrawNamePatch(NG_STATSX+3*NG_SPACINGX-V_NamePatchWidth(secret),
     NG_STATSY, FB, secret, CR_DEFAULT, VPT_STRETCH);

  if (dofrags)
    V_DrawNamePatch(NG_STATSX+4*NG_SPACINGX-V_NamePatchWidth(frags),
       NG_STATSY, FB, frags, CR_DEFAULT, VPT_STRETCH);

  // draw stats
  y = NG_STATSY + V_NamePatchHeight(kills);

  for (i = 0; i < g_maxplayers; i++)
  {
    //int trans = playernumtotrans[i];
    if (!playeringame[i])
      continue;

    x = NG_STATSX;
    V_DrawNamePatch(x-fwidth, y, FB, facebackp,
       i ? CR_LIMIT+i : CR_DEFAULT,
       VPT_STRETCH | (i ? VPT_TRANS : 0));

    if (i == me)
      V_DrawNamePatch(x-fwidth, y, FB, star, CR_DEFAULT, VPT_STRETCH);

    x += NG_SPACINGX;
    if (cnt_kills)
      WI_drawPercent(x-pwidth, y+10, cnt_kills[i]);
    x += NG_SPACINGX;
    if (cnt_items)
      WI_drawPercent(x-pwidth, y+10, cnt_items[i]);
    x += NG_SPACINGX;
    if (cnt_secret)
      WI_drawPercent(x-pwidth, y+10, cnt_secret[i]);
    x += NG_SPACINGX;

    if (dofrags && cnt_frags)
      WI_drawNum(x, y+10, cnt_frags[i], -1);

    y += WI_SPACINGY;
  }

  if (y <= SP_TIMEY)
    // cph - show times in coop on the entering screen
    WI_drawTimeStats(plrs[me].stime / TICRATE, wbs->totaltimes / TICRATE, wbs->partime / TICRATE);
}

static int  sp_state;

// ====================================================================
// WI_initStats
// Purpose: Get ready for single player stats
// Args:    none
// Returns: void
// Comment: Seems like we could do all these stats in a more generic
//          set of routines that weren't duplicated for dm, coop, sp
//
void WI_initStats(void)
{
  state = StatCount;
  acceleratestage = 0;
  sp_state = 1;

  // CPhipps - allocate (awful code, I know, but saves changing it all) and initialise
  *(cnt_kills = Z_Malloc(sizeof(*cnt_kills))) =
  *(cnt_items = Z_Malloc(sizeof(*cnt_items))) =
  *(cnt_secret= Z_Malloc(sizeof(*cnt_secret))) = -1;
  cnt_time = cnt_par = cnt_total_time = -1;
  cnt_pause = TICRATE;

  WI_initAnimatedBack(false);
}

// ====================================================================
// WI_updateStats
// Purpose: Calculate solo stats
// Args:    none
// Returns: void
//
void WI_updateStats(void)
{
  //e6y
  static dboolean play_early_explosion = true;

  WI_updateAnimatedBack();

  if (acceleratestage && sp_state != 10)
  {
    acceleratestage = 0;
    cnt_kills[0] = WI_killPercent(me);
    cnt_items[0] = WI_itemPercent(me);
    cnt_secret[0] = WI_secretPercent(me);

    cnt_total_time = wbs->totaltimes / TICRATE;
    cnt_time = plrs[me].stime / TICRATE;
    cnt_par = wbs->partime / TICRATE;
    S_StartVoidSound(sfx_barexp);
    sp_state = 10;
  }

  if (sp_state == 2)
  {
    cnt_kills[0] += 2;

    if (!(bcnt&3))
      S_StartVoidSound(sfx_pistol);

    if (cnt_kills[0] >= WI_killLimit(me))
    {
      cnt_kills[0] = WI_killPercent(me);
      S_StartVoidSound(sfx_barexp);
      sp_state++;
    }
  }
  else if (sp_state == 4)
  {
    cnt_items[0] += 2;

    if (!(bcnt&3))
      S_StartVoidSound(sfx_pistol);

    if (cnt_items[0] >= WI_itemLimit(me))
    {
      cnt_items[0] = WI_itemPercent(me);
      S_StartVoidSound(sfx_barexp);
      sp_state++;
    }
  }
  else if (sp_state == 6)
  {
    cnt_secret[0] += 2;

    if (!(bcnt&3))
      S_StartVoidSound(sfx_pistol);

    if (cnt_secret[0] >= WI_secretLimit(me))
    {
      cnt_secret[0] = WI_secretPercent(me);
      S_StartVoidSound(sfx_barexp);
      sp_state++;
    }
  }
  else if (sp_state == 8)
  {
    if (!(bcnt&3) && play_early_explosion) //e6y: do not play count sound after explosion sound
      S_StartVoidSound(sfx_pistol);

    cnt_time += 3;

    if (cnt_time >= plrs[me].stime / TICRATE)
      cnt_time = plrs[me].stime / TICRATE;

    cnt_total_time += 3;

    if (cnt_total_time >= wbs->totaltimes / TICRATE)
      cnt_total_time = wbs->totaltimes / TICRATE;

    cnt_par += 3;

    // e6y
    // if par time is hidden (if modifiedgame is true)
    // the game should play explosion sound immediately after
    // the counter will reach level time instead of par time
    if (modifiedgame && play_early_explosion)
    {
      if ((cnt_time >= plrs[me].stime / TICRATE) && (compatibility_level < lxdoom_1_compatibility || cnt_total_time >= wbs->totaltimes / TICRATE))
      {
        // for ExM8 levels if the player won't have pressed <Use>
        if (compatibility_level < lxdoom_1_compatibility)
          cnt_total_time = wbs->totaltimes / TICRATE;

        S_StartVoidSound(sfx_barexp);
        play_early_explosion = false; // do not play it twice or more
      }
    }

    if (cnt_par >= wbs->partime / TICRATE)
    {
      cnt_par = wbs->partime / TICRATE;

      if ((cnt_time >= plrs[me].stime / TICRATE) && (compatibility_level < lxdoom_1_compatibility || cnt_total_time >= wbs->totaltimes / TICRATE))
      {
        //e6y: for ExM8 levels
        if (compatibility_level < lxdoom_1_compatibility)
          cnt_total_time = wbs->totaltimes / TICRATE;

        if (!modifiedgame) //e6y: do not play explosion sound if it was already played
          S_StartVoidSound(sfx_barexp);
        sp_state++;
      }
    }
  }
  else if (sp_state == 10)
  {
    if (acceleratestage)
    {
      S_StartVoidSound(sfx_sgcock);

      if (gamemode == commercial)
        WI_initNoState();
      else
        WI_initShowNextLoc();
    }
  }
  else if (sp_state & 1)
  {
    play_early_explosion = true; //e6y
    if (!--cnt_pause)
    {
      sp_state++;
      cnt_pause = TICRATE;
    }
  }
}

// ====================================================================
// WI_drawStats
// Purpose: Put the solo stats on the screen
// Args:    none
// Returns: void
//
// proff/nicolas 09/20/98 -- changed for hi-res
// CPhipps - patch drawing updated
void WI_drawStats(void)
{
  // line height
  int lh;

  lh = (3*num[0].height)/2;

  WI_slamBackground();

  // draw animated background
  WI_drawAnimatedBack();

  WI_drawLF();

  V_DrawNamePatch(SP_STATSX, SP_STATSY, FB, kills, CR_DEFAULT, VPT_STRETCH);
  if (cnt_kills)
    WI_drawPercent(320 - SP_STATSX, SP_STATSY, cnt_kills[0]);

  V_DrawNamePatch(SP_STATSX, SP_STATSY+lh, FB, items, CR_DEFAULT, VPT_STRETCH);
  if (cnt_items)
    WI_drawPercent(320 - SP_STATSX, SP_STATSY+lh, cnt_items[0]);

  V_DrawNamePatch(SP_STATSX, SP_STATSY+2*lh, FB, sp_secret, CR_DEFAULT, VPT_STRETCH);
  if (cnt_secret)
    WI_drawPercent(320 - SP_STATSX, SP_STATSY+2*lh, cnt_secret[0]);

  WI_drawTimeStats(cnt_time, cnt_total_time, cnt_par);
}

// ====================================================================
// WI_checkForAccelerate
// Purpose: See if the player has hit either the attack or use key
//          or mouse button.  If so we set acceleratestage to 1 and
//          all those display routines above jump right to the end.
// Args:    none
// Returns: void
//
void WI_checkForAccelerate(void)
{
  int   i;
  player_t  *player;

  // check for button presses to skip delays
  for (i = 0, player = players; i < g_maxplayers; i++, player++)
  {
    if (playeringame[i])
    {
      if (player->cmd.buttons & BT_ATTACK)
      {
        if (!player->attackdown)
          acceleratestage = 1;
        player->attackdown = true;
      }
      else
        player->attackdown = false;

      if (player->cmd.buttons & BT_USE)
      {
        if (!player->usedown)
          acceleratestage = 1;
        player->usedown = true;
      }
      else
        player->usedown = false;
    }
  }
}

// ====================================================================
// WI_Ticker
// Purpose: Do various updates every gametic, for stats, animation,
//          checking that intermission music is running, etc.
// Args:    none
// Returns: void
//
void WI_Ticker(void)
{
  if (heretic) return IN_Ticker();
  if (hexen) return Hexen_IN_Ticker();

  // counter for general background animation
  bcnt++;

  if (bcnt == 1)
  {
    int mnum;
    int muslump;

    dsda_IntermissionMusic(&mnum, &muslump);

    if (muslump >= 0)
    {
      S_ChangeMusInfoMusic(muslump, true);
    }
    else
    {
      S_ChangeMusic(mnum, true);
    }
  }

  WI_checkForAccelerate();

  switch (state)
  {
    case StatCount:
         if (deathmatch) WI_updateDeathmatchStats();
         else if (netgame) WI_updateNetgameStats();
         else WI_updateStats();
         break;

    case ShowNextLoc:
         WI_updateShowNextLoc();
         break;

    case NoState:
         WI_updateNoState();
         break;
  }
}

/* ====================================================================
 * WI_loadData
 * Purpose: Initialize intermission data such as background graphics,
 *          patches, map names, etc.
 * Args:    none
 * Returns: void
 *
 * CPhipps - modified for new wad lump handling.
 *         - no longer preload most graphics, other funcs can use
 *           them by name
 */

void WI_loadData(void)
{
  int   i;
  int   j;
  char  name[9];  // limited to 8 characters
  wi_anim_t* a;

  if (gamemode != commercial)
  {
    if (wbs->epsd < 3)
    {
      for (j=0;j<NUMANIMS[wbs->epsd];j++)
      {
        a = &anims[wbs->epsd][j];
        for (i=0;i<a->nanims;i++)
        {
          // MONDO HACK!
          if (wbs->epsd != 1 || j != 8)
          {
            // animations
            snprintf(name, sizeof(name), "WIA%d%.2d%.2d", wbs->epsd, j, i);
            R_SetPatchNum(&a->p[i], name);
          }
          else
          {
            // HACK ALERT!
            a->p[i] = anims[1][4].p[i];
          }
        }
      }
    }
  }

  for (i=0;i<10;i++)
  {
    // numbers 0-9
    sprintf(name, "WINUM%d", i);
    R_SetPatchNum(&num[i], name);
  }
}


// ====================================================================
// WI_Drawer
// Purpose: Call the appropriate stats drawing routine depending on
//          what kind of game is being played (DM, coop, solo)
// Args:    none
// Returns: void
//
void WI_Drawer (void)
{
  if (heretic) return IN_Drawer();
  if (hexen) return Hexen_IN_Drawer();

  switch (state)
  {
    case StatCount:
         if (deathmatch)
           WI_drawDeathmatchStats();
         else if (netgame)
           WI_drawNetgameStats();
         else
           WI_drawStats();
         break;

    case ShowNextLoc:
         WI_drawShowNextLoc();
         break;

    case NoState:
         WI_drawNoState();
         break;
  }
}


// ====================================================================
// WI_initVariables
// Purpose: Initialize the intermission information structure
//          Note: wbstartstruct_t is defined in d_player.h
// Args:    wbstartstruct -- pointer to the structure with the data
// Returns: void
//
void WI_initVariables(wbstartstruct_t* wbstartstruct)
{

  wbs = wbstartstruct;

#ifdef RANGECHECKING
  if (gamemode != commercial)
  {
    if ( gamemode == retail )
      RNGCHECK(wbs->epsd, 0, 3);
    else
      RNGCHECK(wbs->epsd, 0, 2);
  }
  else
  {
    RNGCHECK(wbs->last, 0, 8);
    RNGCHECK(wbs->next, 0, 8);
  }
  RNGCHECK(wbs->pnum, 0, g_maxplayers);
  RNGCHECK(wbs->pnum, 0, g_maxplayers);
#endif

  acceleratestage = 0;
  cnt = bcnt = 0;
  firstrefresh = 1;
  me = wbs->pnum;
  plrs = wbs->plyr;

  if ( gamemode != retail )
    if (wbs->epsd > 2)
      wbs->epsd -= 3;
}

// ====================================================================
// WI_Start
// Purpose: Call the various init routines
//          Note: wbstartstruct_t is defined in d_player.h
// Args:    wbstartstruct -- pointer to the structure with the
//          intermission data
// Returns: void
//
void WI_Start(wbstartstruct_t* wbstartstruct)
{
  if (heretic) return IN_Start(wbstartstruct);
  if (hexen) return Hexen_IN_Start(wbstartstruct);

  WI_initVariables(wbstartstruct);
  WI_loadData();

  exitpic = dsda_ExitPic();
  enterpic = dsda_EnterPic();

  if (deathmatch)
    WI_initDeathmatchStats();
  else if (netgame)
    WI_initNetgameStats();
  else
    WI_initStats();
}
