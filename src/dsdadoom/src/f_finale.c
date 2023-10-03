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
 *      Game completion, final screen animation.
 *
 *-----------------------------------------------------------------------------
 */

#include "doomstat.h"
#include "d_event.h"
#include "g_game.h"
#include "lprintf.h"
#include "v_video.h"
#include "w_wad.h"
#include "s_sound.h"
#include "sounds.h"
#include "d_deh.h"  // Ty 03/22/98 - externalizations

#include "heretic/f_finale.h"
#include "hexen/f_finale.h"

#include "dsda/font.h"
#include "dsda/mapinfo.h"

#include "f_finale.h" // CPhipps - hmm...

// defines for the end mission display text                     // phares

#define TEXTSPEED    3     // original value                    // phares
#define TEXTWAIT     250   // original value                    // phares
#define NEWTEXTSPEED 0.01f // new value                         // phares
#define NEWTEXTWAIT  1000  // new value                         // phares

// Stage of animation:
//  0 = text, 1 = art screen, 2 = character cast
int finalestage;
int finalecount;
const char*   finaletext;
const char*   finaleflat;
const char*   finalepatch;

// defines for the end mission display text                     // phares

// CPhipps - removed the old finale screen text message strings;
// they were commented out for ages already
// Ty 03/22/98 - ... the new s_WHATEVER extern variables are used
// in the code below instead.

void    F_CastTicker (void);
dboolean F_CastResponder (event_t *ev);
void    F_CastDrawer (void);

void WI_checkForAccelerate(void);    // killough 3/28/98: used to
extern int acceleratestage;          // accelerate intermission screens
int midstage;                 // whether we're in "mid-stage"

//
// F_StartFinale
//
void F_StartFinale (void)
{
  int mnum;
  int muslump;

  if (heretic) return Heretic_F_StartFinale();
  if (hexen) return Hexen_F_StartFinale();

  gameaction = ga_nothing;
  gamestate = GS_FINALE;
  automap_active = false;

  // killough 3/28/98: clear accelerative text flags
  acceleratestage = midstage = 0;

  finaletext = NULL;
  finaleflat = NULL;
  finalepatch = NULL;

  dsda_InterMusic(&mnum, &muslump);

  if (muslump >= 0)
  {
    S_ChangeMusInfoMusic(muslump, true);
  }
  else
  {
    S_ChangeMusic(mnum, true);
  }

  // Okay - IWAD dependend stuff.
  // This has been changed severly, and
  //  some stuff might have changed in the process.
  switch ( gamemode )
  {
    // DOOM 1 - E1, E3 or E4, but each nine missions
    case shareware:
    case registered:
    case retail:
    {
      switch (gameepisode)
      {
        case 1:
             finaleflat = bgflatE1; // Ty 03/30/98 - new externalized bg flats
             finaletext = s_E1TEXT; // Ty 03/23/98 - Was e1text variable.
             break;
        case 2:
             finaleflat = bgflatE2;
             finaletext = s_E2TEXT; // Ty 03/23/98 - Same stuff for each
             break;
        case 3:
             finaleflat = bgflatE3;
             finaletext = s_E3TEXT;
             break;
        case 4:
             finaleflat = bgflatE4;
             finaletext = s_E4TEXT;
             break;
        default:
             // Ouch.
             break;
      }
      break;
    }

    // DOOM II and missions packs with E1, M34
    case commercial:
    {
      // Ty 08/27/98 - added the gamemission logic
      switch (gamemap)
      {
        case 6:
             finaleflat = bgflat06;
             finaletext = (gamemission==pack_tnt)  ? s_T1TEXT :
                          (gamemission==pack_plut) ? s_P1TEXT : s_C1TEXT;
             break;
        case 11:
             finaleflat = bgflat11;
             finaletext = (gamemission==pack_tnt)  ? s_T2TEXT :
                          (gamemission==pack_plut) ? s_P2TEXT : s_C2TEXT;
             break;
        case 20:
             finaleflat = bgflat20;
             finaletext = (gamemission==pack_tnt)  ? s_T3TEXT :
                          (gamemission==pack_plut) ? s_P3TEXT : s_C3TEXT;
             break;
        case 30:
             finaleflat = bgflat30;
             finaletext = (gamemission==pack_tnt)  ? s_T4TEXT :
                          (gamemission==pack_plut) ? s_P4TEXT : s_C4TEXT;
             break;
        case 15:
             finaleflat = bgflat15;
             finaletext = (gamemission==pack_tnt)  ? s_T5TEXT :
                          (gamemission==pack_plut) ? s_P5TEXT : s_C5TEXT;
             break;
        case 31:
             finaleflat = bgflat31;
             finaletext = (gamemission==pack_tnt)  ? s_T6TEXT :
                          (gamemission==pack_plut) ? s_P6TEXT : s_C6TEXT;
             break;
        default:
             // Ouch.
             break;
      }
      if (gamemission == pack_nerve && gamemap == 8)
      {
        finaleflat = bgflat06;
        finaletext = s_C6TEXT;
      }
      break;
      // Ty 08/27/98 - end gamemission logic
    }

    // Indeterminate.
    default:  // Ty 03/30/98 - not externalized
         finaleflat = "F_SKY1"; // Not used anywhere else.
         finaletext = s_C1TEXT;  // FIXME - other text, music?
         break;
  }

  if (dsda_FinaleShortcut()) {
    switch (gamemode) {
      case shareware:
      case registered:
      case retail:
        switch (gameepisode) {
          case 1:
              finaleflat = bgflatE1;
              finaletext = s_E1TEXT;
              break;
          case 2:
              finaleflat = bgflatE2;
              finaletext = s_E2TEXT;
              break;
          case 3:
              finaleflat = bgflatE3;
              finaletext = s_E3TEXT;
              break;
          case 4:
          default:
              finaleflat = bgflatE4;
              finaletext = s_E4TEXT;
              break;
        }
        break;
      case commercial:
        if (gamemission == pack_nerve) {
          finaleflat = bgflat06;
          finaletext = s_C6TEXT;
        }
        else {
          finaleflat = bgflat30;
          finaletext = (gamemission == pack_tnt)  ? s_T4TEXT :
                       (gamemission == pack_plut) ? s_P4TEXT : s_C4TEXT;
        }
        break;
      default:
        break;
    }
  }

  dsda_StartFinale();

  finalestage = 0;
  finalecount = 0;
}



dboolean F_Responder (event_t *event)
{
  if (heretic) return Heretic_F_Responder(event);
  if (hexen) return Hexen_F_Responder(event);

  if (finalestage == 2)
    return F_CastResponder (event);

  return false;
}

// Get_TextSpeed() returns the value of the text display speed  // phares
// Rewritten to allow user-directed acceleration -- killough 3/28/98

float Get_TextSpeed(void)
{
  return midstage ? NEWTEXTSPEED : (midstage=acceleratestage) ?
    acceleratestage=0, NEWTEXTSPEED : TEXTSPEED;
}


//
// F_Ticker
//
// killough 3/28/98: almost totally rewritten, to use
// player-directed acceleration instead of constant delays.
// Now the player can accelerate the text display by using
// the fire/use keys while it is being printed. The delay
// automatically responds to the user, and gives enough
// time to read.
//
// killough 5/10/98: add back v1.9 demo compatibility
//

static dboolean F_ShowCast(void)
{
  return gamemap == 30 ||
         (gamemission == pack_nerve && allow_incompatibility && gamemap == 8) ||
         dsda_FinaleShortcut();
}

void F_Ticker(void)
{
  int i;

  if (heretic) return Heretic_F_Ticker();
  if (hexen) return Hexen_F_Ticker();

  if (dsda_FTicker())
  {
    return;
  }

  if (!demo_compatibility)
    WI_checkForAccelerate();  // killough 3/28/98: check for acceleration
  else
    if (gamemode == commercial && finalecount > 50) // check for skipping
      for (i = 0; i < g_maxplayers; i++)
        if (players[i].cmd.buttons)
          goto next_level;      // go on to the next level

  // advance animation
  finalecount++;

  if (finalestage == 2)
    F_CastTicker();

  if (!finalestage)
    {
      float speed = demo_compatibility ? TEXTSPEED : Get_TextSpeed();
      /* killough 2/28/98: changed to allow acceleration */
      if (finalecount > strlen(finaletext)*speed +
          (midstage ? NEWTEXTWAIT : TEXTWAIT) ||
          (midstage && acceleratestage)) {
        if (gamemode != commercial)       // Doom 1 / Ultimate Doom episode end
          {                               // with enough time, it's automatic
            if (gameepisode == 3)
              F_StartScroll(NULL, NULL, NULL, true);
            else
              F_StartPostFinale();
          }
        else   // you must press a button to continue in Doom 2
          if (!demo_compatibility && midstage)
            {
            next_level:
              if (F_ShowCast())
                F_StartCast(NULL, NULL, true); // cast of Doom 2 characters
              else
                gameaction = ga_worlddone;  // next level, e.g. MAP07
            }
      }
    }
}

//
// F_TextWrite
//
// This program displays the background and text at end-mission     // phares
// text time. It draws both repeatedly so that other displays,      //   |
// like the main menu, can be drawn over it dynamically and         //   V
// erased dynamically. The TEXTSPEED constant is changed into
// the Get_TextSpeed function so that the speed of writing the      //   ^
// text can be increased, and there's still time to read what's     //   |
// written.                                                         // phares
// CPhipps - reformatted

#include "hu_stuff.h"

void F_TextWrite (void)
{
  if (finalepatch)
  {
    V_ClearBorder();
    V_DrawNamePatch(0, 0, 0, finalepatch, CR_DEFAULT, VPT_STRETCH);
  }
  else
    V_DrawBackground(finaleflat, 0);

  { // draw some of the text onto the screen
    int         cx = 10;
    int         cy = 10;
    const char* ch = finaletext; // CPhipps - const
    int         count = (int)((float)(finalecount - 10)/Get_TextSpeed()); // phares
    int         w;

    if (count < 0)
      count = 0;

    for ( ; count ; count-- ) {
      int       c = *ch++;

      if (!c)
        break;

      if (c == '\n') {
        cx = 10;
        cy += 11;
        continue;
      }

      c = toupper(c) - HU_FONTSTART;
      if (c < 0 || c> HU_FONTSIZE) {
        cx += 4;
        continue;
      }

      w = hud_font.font[c].width;
      if (cx+w > SCREENWIDTH)
        break;

      // CPhipps - patch drawing updated
      V_DrawNumPatch(cx, cy, 0, hud_font.font[c].lumpnum, CR_DEFAULT, VPT_STRETCH);
      cx+=w;
    }
  }
}

//
// Final DOOM 2 animation
// Casting by id Software.
//   in order of appearance
//
typedef struct
{
  const char **name; // CPhipps - const**
  mobjtype_t   type;
} castinfo_t;

static const castinfo_t castorder_d2[] = {
  { &s_CC_ZOMBIE,  MT_POSSESSED },
  { &s_CC_SHOTGUN, MT_SHOTGUY },
  { &s_CC_HEAVY,   MT_CHAINGUY },
  { &s_CC_IMP,     MT_TROOP },
  { &s_CC_DEMON,   MT_SERGEANT },
  { &s_CC_LOST,    MT_SKULL },
  { &s_CC_CACO,    MT_HEAD },
  { &s_CC_HELL,    MT_KNIGHT },
  { &s_CC_BARON,   MT_BRUISER },
  { &s_CC_ARACH,   MT_BABY },
  { &s_CC_PAIN,    MT_PAIN },
  { &s_CC_REVEN,   MT_UNDEAD },
  { &s_CC_MANCU,   MT_FATSO },
  { &s_CC_ARCH,    MT_VILE },
  { &s_CC_SPIDER,  MT_SPIDER },
  { &s_CC_CYBER,   MT_CYBORG },
  { &s_CC_HERO,    MT_PLAYER },
  { NULL,          0 }
};

static const castinfo_t castorder_d1[] = {
  { &s_CC_ZOMBIE,  MT_POSSESSED },
  { &s_CC_SHOTGUN, MT_SHOTGUY },
  { &s_CC_IMP,     MT_TROOP },
  { &s_CC_DEMON,   MT_SERGEANT },
  { &s_CC_LOST,    MT_SKULL },
  { &s_CC_CACO,    MT_HEAD },
  { &s_CC_BARON,   MT_BRUISER },
  { &s_CC_SPIDER,  MT_SPIDER },
  { &s_CC_CYBER,   MT_CYBORG },
  { &s_CC_HERO,    MT_PLAYER },
  { NULL,          0 }
};

static const castinfo_t *castorder = castorder_d2;

static int castnum;
static int casttics;
static state_t* caststate;
static dboolean castdeath;
static int castframes;
static int castonmelee;
static dboolean castattacking;
static const char *castbackground;

//
// F_StartCast
//

static void F_StartCastMusic(const char* music, dboolean loop_music)
{
  if (music)
  {
    if (!S_ChangeMusicByName(music, loop_music))
      lprintf(LO_WARN, "Finale cast music not found: %s\n", music);
  }
  else if (gamemode == commercial)
  {
    S_ChangeMusic(mus_evil, loop_music);
  }
  else
  {
    lprintf(LO_WARN, "Finale cast music unspecified\n");
    S_StopMusic();
  }
}

void F_StartCast (const char* background, const char* music, dboolean loop_music)
{
  castorder = (gamemode == commercial ? castorder_d2 : castorder_d1);
  castbackground = (background ? background : bgcastcall);

  wipegamestate = -1;         // force a screen wipe
  castnum = 0;
  caststate = &states[mobjinfo[castorder[castnum].type].seestate];
  casttics = caststate->tics;
  castdeath = false;
  finalestage = 2;
  castframes = 0;
  castonmelee = 0;
  castattacking = false;

  F_StartCastMusic(music, loop_music);
}

//
// F_CastTicker
//
void F_CastTicker (void)
{
  int st;
  int sfx;

  if (--casttics > 0)
    return;                 // not time to change state yet

  if (caststate->tics == -1 || caststate->nextstate == S_NULL)
  {
    // switch from deathstate to next monster
    castnum++;
    castdeath = false;
    if (castorder[castnum].name == NULL)
      castnum = 0;
    if (mobjinfo[castorder[castnum].type].seesound)
      S_StartVoidSound(mobjinfo[castorder[castnum].type].seesound);
    caststate = &states[mobjinfo[castorder[castnum].type].seestate];
    castframes = 0;
  }
  else
  {
    // just advance to next state in animation
    if (caststate == &states[S_PLAY_ATK1])
      goto stopattack;    // Oh, gross hack!
    st = caststate->nextstate;
    caststate = &states[st];
    castframes++;

    // sound hacks....
    switch (st)
    {
      case S_PLAY_ATK1:     sfx = sfx_dshtgn; break;
      case S_POSS_ATK2:     sfx = sfx_pistol; break;
      case S_SPOS_ATK2:     sfx = sfx_shotgn; break;
      case S_VILE_ATK2:     sfx = sfx_vilatk; break;
      case S_SKEL_FIST2:    sfx = sfx_skeswg; break;
      case S_SKEL_FIST4:    sfx = sfx_skepch; break;
      case S_SKEL_MISS2:    sfx = sfx_skeatk; break;
      case S_FATT_ATK8:
      case S_FATT_ATK5:
      case S_FATT_ATK2:     sfx = sfx_firsht; break;
      case S_CPOS_ATK2:
      case S_CPOS_ATK3:
      case S_CPOS_ATK4:     sfx = sfx_shotgn; break;
      case S_TROO_ATK3:     sfx = sfx_claw; break;
      case S_SARG_ATK2:     sfx = sfx_sgtatk; break;
      case S_BOSS_ATK2:
      case S_BOS2_ATK2:
      case S_HEAD_ATK2:     sfx = sfx_firsht; break;
      case S_SKULL_ATK2:    sfx = sfx_sklatk; break;
      case S_SPID_ATK2:
      case S_SPID_ATK3:     sfx = sfx_shotgn; break;
      case S_BSPI_ATK2:     sfx = sfx_plasma; break;
      case S_CYBER_ATK2:
      case S_CYBER_ATK4:
      case S_CYBER_ATK6:    sfx = sfx_rlaunc; break;
      case S_PAIN_ATK3:     sfx = sfx_sklatk; break;
      default: sfx = 0; break;
    }

    if (sfx)
      S_StartVoidSound(sfx);
  }

  if (castframes == 12)
  {
    // go into attack frame
    castattacking = true;
    if (castonmelee)
      caststate=&states[mobjinfo[castorder[castnum].type].meleestate];
    else
      caststate=&states[mobjinfo[castorder[castnum].type].missilestate];
    castonmelee ^= 1;
    if (caststate == &states[S_NULL])
    {
      if (castonmelee)
        caststate=
          &states[mobjinfo[castorder[castnum].type].meleestate];
      else
        caststate=
          &states[mobjinfo[castorder[castnum].type].missilestate];
    }
  }

  if (castattacking)
  {
    if (castframes == 24
       ||  caststate == &states[mobjinfo[castorder[castnum].type].seestate] )
    {
      stopattack:
      castattacking = false;
      castframes = 0;
      caststate = &states[mobjinfo[castorder[castnum].type].seestate];
    }
  }

  casttics = caststate->tics;
  if (casttics == -1)
      casttics = 15;
}


//
// F_CastResponder
//

dboolean F_CastResponder (event_t* ev)
{
  if (ev->type != ev_keydown)
    return false;

  if (castdeath)
    return true;                    // already in dying frames

  // go into death frame
  castdeath = true;
  caststate = &states[mobjinfo[castorder[castnum].type].deathstate];
  casttics = caststate->tics;
  castframes = 0;
  castattacking = false;
  if (mobjinfo[castorder[castnum].type].deathsound)
    S_StartVoidSound(mobjinfo[castorder[castnum].type].deathsound);

  return true;
}


static void F_CastPrint (const char* text) // CPhipps - static, const char*
{
  const char* ch; // CPhipps - const
  int         c;
  int         cx;
  int         w;
  int         width;

  // find width
  ch = text;
  width = 0;

  while (ch)
  {
    c = *ch++;
    if (!c)
      break;
    c = toupper(c) - HU_FONTSTART;
    if (c < 0 || c> HU_FONTSIZE)
    {
      width += 4;
      continue;
    }

    w = hud_font.font[c].width;
    width += w;
  }

  // draw it
  cx = 160-width/2;
  ch = text;
  while (ch)
  {
    c = *ch++;
    if (!c)
      break;
    c = toupper(c) - HU_FONTSTART;
    if (c < 0 || c> HU_FONTSIZE)
    {
      cx += 4;
      continue;
    }

    w = hud_font.font[c].width;
    // CPhipps - patch drawing updated
    V_DrawNumPatch(cx, 180, 0, hud_font.font[c].lumpnum, CR_DEFAULT, VPT_STRETCH);
    cx+=w;
  }
}


//
// F_CastDrawer
//

void F_CastDrawer (void)
{
  spritedef_t*        sprdef;
  spriteframe_t*      sprframe;
  int                 lump;
  dboolean             flip;

  // e6y: wide-res
  V_ClearBorder();
  // erase the entire screen to a background
  // CPhipps - patch drawing updated
  V_DrawNamePatch(0,0,0, castbackground, CR_DEFAULT, VPT_STRETCH); // Ty 03/30/98 bg texture extern

  F_CastPrint (*(castorder[castnum].name));

  // draw the current frame in the middle of the screen
  sprdef = &sprites[caststate->sprite];
  sprframe = &sprdef->spriteframes[ caststate->frame & FF_FRAMEMASK];
  lump = sprframe->lump[0];
  flip = (dboolean)(sprframe->flip & 1);

  // CPhipps - patch drawing updated
  V_DrawNumPatch(160, 170, 0, lump+firstspritelump, CR_DEFAULT,
     VPT_STRETCH | (flip ? VPT_FLIP : 0));
}

//
// F_BunnyScroll
//
static const char* pfub1 = "PFUB1";
static const char* pfub2 = "PFUB2";

static const char* scrollpic1;
static const char* scrollpic2;

static void F_StartScrollMusic(const char* music, dboolean loop_music)
{
  if (music) {
    if (!S_ChangeMusicByName(music, loop_music))
      lprintf(LO_WARN, "Finale scroll music not found: %s\n", music);
  }
  else if (gamemode != commercial)
    S_ChangeMusic(mus_bunny, loop_music);
  else {
    lprintf(LO_WARN, "Finale scroll music unspecified\n");
    S_StopMusic();
  }
}

void F_StartScroll (const char* right, const char* left, const char* music, dboolean loop_music)
{
  wipegamestate = -1; // force a wipe
  scrollpic1 = right ? right : pfub1;
  scrollpic2 = left ? left : pfub2;
  finalecount = 0;
  finalestage = 1;

  F_StartScrollMusic(music, loop_music);
}

void F_BunnyScroll (void)
{
  char        name[10];
  int         stage;
  static int  laststage;
  static int  p1offset, p2width;

  if (finalecount == 0)
  {
    const rpatch_t *p1, *p2;
    p1 = R_PatchByName(scrollpic1);
    p2 = R_PatchByName(scrollpic2);

    p2width = p2->width;
    if (p1->width == 320)
    {
      // Unity or original PFUBs.
      p1offset = (p2width - 320) / 2;
    }
    else
    {
      // Widescreen mod PFUBs.
      p1offset = 0;
    }
  }

  {
    int scrolled = 320 - (finalecount-230)/2;
    if (scrolled <= 0) {
      V_DrawNamePatch(0, 0, 0, scrollpic2, CR_DEFAULT, VPT_STRETCH);
    } else if (scrolled >= 320) {
      V_DrawNamePatch(p1offset, 0, 0, scrollpic1, CR_DEFAULT, VPT_STRETCH);
      if (p1offset > 0)
        V_DrawNamePatch(-320, 0, 0, scrollpic2, CR_DEFAULT, VPT_STRETCH);
    } else {
      V_DrawNamePatch(p1offset + 320 - scrolled, 0, 0, scrollpic1, CR_DEFAULT, VPT_STRETCH);
      V_DrawNamePatch(-scrolled, 0, 0, scrollpic2, CR_DEFAULT, VPT_STRETCH);
    }
    if (p2width == 320)
      V_ClearBorder();
  }

  if (gamemode == commercial)
    return;

  if (finalecount < 1130)
    return;
  if (finalecount < 1180)
  {
    // CPhipps - patch drawing updated
    V_DrawNamePatch((320-13*8)/2, (200-8*8)/2,0, "END0", CR_DEFAULT, VPT_STRETCH);
    laststage = 0;
    return;
  }

  stage = (finalecount-1180) / 5;
  if (stage > 6)
    stage = 6;
  if (stage > laststage)
  {
    S_StartVoidSound(sfx_pistol);
    laststage = stage;
  }

  sprintf (name,"END%i",stage);
  // CPhipps - patch drawing updated
  V_DrawNamePatch((320-13*8)/2, (200-8*8)/2, 0, name, CR_DEFAULT, VPT_STRETCH);
}

void F_StartPostFinale (void)
{
  finalecount = 0;
  finalestage = 1;
  wipegamestate = -1; // force a wipe
}

//
// F_Drawer
//
void F_Drawer (void)
{
  if (heretic) return Heretic_F_Drawer();
  if (hexen) return Hexen_F_Drawer();

  if (dsda_FDrawer())
  {
    return;
  }

  if (finalestage == 2)
  {
    F_CastDrawer ();
    return;
  }

  if (!finalestage)
    F_TextWrite ();
  else
  {
    // e6y: wide-res
    V_ClearBorder();

    switch (gameepisode)
    {
      // CPhipps - patch drawing updated
      case 1:
           if ( gamemode == retail || gamemode == commercial )
             V_DrawNamePatch(0, 0, 0, "CREDIT", CR_DEFAULT, VPT_STRETCH);
           else
             V_DrawNamePatch(0, 0, 0, "HELP2", CR_DEFAULT, VPT_STRETCH);
           break;
      case 2:
           V_DrawNamePatch(0, 0, 0, "VICTORY2", CR_DEFAULT, VPT_STRETCH);
           break;
      case 3:
           F_BunnyScroll ();
           break;
      case 4:
           V_DrawNamePatch(0, 0, 0, "ENDPIC", CR_DEFAULT, VPT_STRETCH);
           break;
    }
  }
}
