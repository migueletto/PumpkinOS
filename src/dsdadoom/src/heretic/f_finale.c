//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// F_finale.c

#include "doomstat.h"
#include "w_wad.h"
#include "v_video.h"
#include "s_sound.h"
#include "sounds.h"

#include "dsda/palette.h"

#include "heretic/def.h"
#include "heretic/dstrings.h"

#include "heretic/f_finale.h"

static int finalestage;                // 0 = text, 1 = art screen
static int finalecount;

#define TEXTSPEED       3
#define TEXTWAIT        250

static const char *finaletext;
static const char *finaleflat;

static int FontABaseLump;

/*
=======================
=
= F_StartFinale
=
=======================
*/

void Heretic_F_StartFinale(void)
{
  gameaction = ga_nothing;
  gamestate = GS_FINALE;
  automap_active = false;

  switch (gameepisode)
  {
    case 1:
      finaleflat = "FLOOR25";
      finaletext = HERETIC_E1TEXT;
      break;
    case 2:
      finaleflat = "FLATHUH1";
      finaletext = HERETIC_E2TEXT;
      break;
    case 3:
      finaleflat = "FLTWAWA2";
      finaletext = HERETIC_E3TEXT;
      break;
    case 4:
      finaleflat = "FLOOR28";
      finaletext = HERETIC_E4TEXT;
      break;
    case 5:
      finaleflat = "FLOOR08";
      finaletext = HERETIC_E5TEXT;
      break;
  }

  finalestage = 0;
  finalecount = 0;
  FontABaseLump = W_GetNumForName("FONTA_S") + 1;
  S_ChangeMusic(heretic_mus_cptd, true);
}

dboolean F_BlockingInput(void)
{
  return finalestage == 1 && gameepisode == 2;
}

dboolean Heretic_F_Responder(event_t * event)
{
  if (event->type != ev_keydown)
  {
    return false;
  }

  if (F_BlockingInput())
  {                           // we're showing the water pic, make any key kick to demo mode
    V_SetPlayPal(playpal_default);
    finalestage++;
    return true;
  }
  return false;
}

/*
=======================
=
= F_Ticker
=
=======================
*/

void Heretic_F_Ticker(void)
{
  finalecount++;
  if (!finalestage && finalecount > strlen(finaletext) * TEXTSPEED + TEXTWAIT)
  {
    finalecount = 0;
    if (!finalestage)
    {
      finalestage = 1;
    }
  }
}

/*
=======================
=
= F_TextWrite
=
=======================
*/

void Heretic_F_TextWrite(void)
{
  int count;
  const char *ch;
  int c;
  int cx, cy;
  int lump;
  int width;

  // e6y: wide-res
  V_ClearBorder();

  //
  // erase the entire screen to a tiled background
  //
  V_DrawBackground(finaleflat, 0);

  //
  // draw some of the text onto the screen
  //
  cx = 20;
  cy = 5;
  ch = finaletext;

  count = (finalecount - 10) / TEXTSPEED;
  if (count < 0)
    count = 0;
  for (; count; count--)
  {
    c = *ch++;
    if (!c)
      break;
    if (c == '\n')
    {
      cx = 20;
      cy += 9;
      continue;
    }

    c = toupper(c);
    if (c < 33)
    {
      cx += 5;
      continue;
    }

    lump = FontABaseLump + c - 33;
    width = R_NumPatchWidth(lump);
    if (cx + width > SCREENWIDTH)
      break;
    V_DrawNumPatch(cx, cy, 0, lump, CR_DEFAULT, VPT_STRETCH);
    cx += width;
  }
}

/*
==================
=
= F_DemonScroll
=
==================
*/

void F_DemonScroll(void)
{
  static int yval = 0;
  static int nextscroll = 0;

  if (finalecount < 70)
  {
    V_DrawRawScreen("FINAL1");
    nextscroll = finalecount;
  }
  else if (yval < 200)
  {
    V_DrawRawScreenSection("FINAL2", (200 - yval) * 320, 0, yval);
    V_DrawRawScreenSection("FINAL1", 0, yval, 200 - yval);
    if (finalecount >= nextscroll)
    {
      yval++;
      nextscroll = finalecount + 3;
    }
  }
  else
  {                           //else, we'll just sit here and wait, for now
    V_DrawRawScreen("FINAL2");
  }
}

/*
==================
=
= F_DrawUnderwater
=
==================
*/

void F_DrawUnderwater(void)
{
  switch (finalestage)
  {
    case 1:
      V_SetPlayPal(playpal_heretic_e2end);
      V_DrawRawScreen("E2END");

      break;
    case 2:
      V_DrawRawScreen("TITLE");
  }
}

/*
=======================
=
= F_Drawer
=
=======================
*/

void Heretic_F_Drawer(void)
{
  if (!finalestage)
    Heretic_F_TextWrite();
  else
  {
    switch (gameepisode)
    {
      case 1:
        if (gamemode == shareware)
        {
          V_DrawRawScreen("ORDER");
        }
        else
        {
          V_DrawRawScreen("CREDIT");
        }
        break;
      case 2:
        F_DrawUnderwater();
        break;
      case 3:
        F_DemonScroll();
        break;
      case 4:            // Just show credits screen for extended episodes
      case 5:
        V_DrawRawScreen("CREDIT");
        break;
    }
  }
}
