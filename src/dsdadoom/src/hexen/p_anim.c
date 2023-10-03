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

#include "doomdef.h"
#include "doomstat.h"
#include "m_random.h"
#include "s_sound.h"
#include "sounds.h"
#include "r_defs.h"
#include "r_state.h"
#include "sc_man.h"
#include "lprintf.h"
#include "w_wad.h"
#include "p_setup.h"

#include "dsda/map_format.h"
#include "dsda/mapinfo.h"

#include "p_anim.h"

#define ANIM_SCRIPT_NAME "ANIMDEFS"
#define MAX_FRAME_DEFS 96
#define ANIM_FLAT 0
#define ANIM_TEXTURE 1
#define SCI_FLAT    "flat"
#define SCI_TEXTURE "texture"
#define SCI_PIC     "pic"
#define SCI_TICS    "tics"
#define SCI_RAND    "rand"

#define LIGHTNING_SPECIAL 	198
#define LIGHTNING_SPECIAL2 	199
#define SKYCHANGE_SPECIAL 	200

typedef struct
{
    int index;
    int tics;
} frameDef_t;

static void P_LightningFlash(void);

extern fixed_t Sky1ColumnOffset;
extern fixed_t Sky2ColumnOffset;
extern dboolean DoubleSky;
extern short numlinespecials;
extern line_t *linespeciallist[];

fixed_t Sky1ScrollDelta;
fixed_t Sky2ScrollDelta;

animDef_t AnimDefs[MAX_ANIM_DEFS];
static frameDef_t FrameDefs[MAX_FRAME_DEFS];
static int AnimDefCount;
static dboolean LevelHasLightning;
int NextLightningFlash;
int LightningFlash;
static int *LightningLightLevels;

void P_AnimateCompatibleSurfaces(void)
{
  // nothing in doom
}

void P_AnimateHereticSurfaces(void)
{
  int i;
  line_t *line;

  // Update scrolling texture offsets
  for (i = 0; i < numlinespecials; i++)
  {
      line = linespeciallist[i];
      switch (line->special)
      {
          case 48:           // Effect_Scroll_Left
              sides[line->sidenum[0]].textureoffset += FRACUNIT;
              break;
          case 99:           // Effect_Scroll_Right
              sides[line->sidenum[0]].textureoffset -= FRACUNIT;
              break;
      }
  }
}

void P_AnimateZDoomSurfaces(void)
{
  // MAP_FORMAT_TODO: P_AnimateZDoomSurfaces
  // The linespeciallist stuff isn't relevant (using doom scrollers)
  // AnimDef stuff will come later
  // Skies / lightning as well
}

void P_AnimateHexenSurfaces(void)
{
    int i;
    animDef_t *ad;
    line_t *line;

    // Animate flats and textures
    for (i = 0; i < AnimDefCount; i++)
    {
        ad = &AnimDefs[i];
        ad->tics--;
        if (ad->tics == 0)
        {
            if (ad->currentFrameDef == ad->endFrameDef)
            {
                ad->currentFrameDef = ad->startFrameDef;
            }
            else
            {
                ad->currentFrameDef++;
            }
            ad->tics = FrameDefs[ad->currentFrameDef].tics;
            if (ad->tics > 255)
            {                   // Random tics
                ad->tics = (ad->tics >> 16)
                    + P_Random(pr_hexen) % ((ad->tics & 0xff00) >> 8);
            }
            if (ad->type == ANIM_FLAT)
            {
                flattranslation[ad->index] =
                    FrameDefs[ad->currentFrameDef].index;
            }
            else
            {                   // Texture
                texturetranslation[ad->index] =
                    FrameDefs[ad->currentFrameDef].index;
            }
        }
    }

    // Update scrolling textures
    for (i = 0; i < numlinespecials; i++)
    {
        line = linespeciallist[i];
        switch (line->special)
        {
            case 100:          // Scroll_Texture_Left
                sides[line->sidenum[0]].textureoffset += line->special_args[0] << 10;
                break;
            case 101:          // Scroll_Texture_Right
                sides[line->sidenum[0]].textureoffset -= line->special_args[0] << 10;
                break;
            case 102:          // Scroll_Texture_Up
                sides[line->sidenum[0]].rowoffset += line->special_args[0] << 10;
                break;
            case 103:          // Scroll_Texture_Down
                sides[line->sidenum[0]].rowoffset -= line->special_args[0] << 10;
                break;
        }
    }

    // Update sky column offsets
    Sky1ColumnOffset += Sky1ScrollDelta;
    Sky2ColumnOffset += Sky2ScrollDelta;

    if (LevelHasLightning)
    {
        if (!NextLightningFlash || LightningFlash)
        {
            P_LightningFlash();
        }
        else
        {
            NextLightningFlash--;
        }
    }
}

void P_AnimateSurfaces(void)
{
  map_format.animate_surfaces();
}

static void P_LightningFlash(void)
{
    int i;
    sector_t *tempSec;
    int *tempLight;
    dboolean foundSec;
    int flashLight;

    if (LightningFlash)
    {
        LightningFlash--;
        if (LightningFlash)
        {
            tempLight = LightningLightLevels;
            tempSec = sectors;
            for (i = 0; i < numsectors; i++, tempSec++)
            {
                if (tempSec->ceilingpic == skyflatnum
                    || tempSec->special == LIGHTNING_SPECIAL
                    || tempSec->special == LIGHTNING_SPECIAL2)
                {
                    if (*tempLight < tempSec->lightlevel - 4)
                    {
                        tempSec->lightlevel -= 4;
                    }
                    tempLight++;
                }
            }
        }
        else
        {                       // remove the alternate lightning flash special
            tempLight = LightningLightLevels;
            tempSec = sectors;
            for (i = 0; i < numsectors; i++, tempSec++)
            {
                if (tempSec->ceilingpic == skyflatnum
                    || tempSec->special == LIGHTNING_SPECIAL
                    || tempSec->special == LIGHTNING_SPECIAL2)
                {
                    tempSec->lightlevel = *tempLight;
                    tempLight++;
                }
            }
            Sky1Texture = dsda_Sky1Texture();
        }
        return;
    }
    LightningFlash = (P_Random(pr_hexen) & 7) + 8;
    flashLight = 200 + (P_Random(pr_hexen) & 31);
    tempSec = sectors;
    tempLight = LightningLightLevels;
    foundSec = false;
    for (i = 0; i < numsectors; i++, tempSec++)
    {
        if (tempSec->ceilingpic == skyflatnum
            || tempSec->special == LIGHTNING_SPECIAL
            || tempSec->special == LIGHTNING_SPECIAL2)
        {
            *tempLight = tempSec->lightlevel;
            if (tempSec->special == LIGHTNING_SPECIAL)
            {
                tempSec->lightlevel += 64;
                if (tempSec->lightlevel > flashLight)
                {
                    tempSec->lightlevel = flashLight;
                }
            }
            else if (tempSec->special == LIGHTNING_SPECIAL2)
            {
                tempSec->lightlevel += 32;
                if (tempSec->lightlevel > flashLight)
                {
                    tempSec->lightlevel = flashLight;
                }
            }
            else
            {
                tempSec->lightlevel = flashLight;
            }
            if (tempSec->lightlevel < *tempLight)
            {
                tempSec->lightlevel = *tempLight;
            }
            tempLight++;
            foundSec = true;
        }
    }
    if (foundSec)
    {
        Sky1Texture = dsda_Sky2Texture();     // set alternate sky
        S_StartVoidSound(hexen_sfx_thunder_crash);
    }
    // Calculate the next lighting flash
    if (!NextLightningFlash)
    {
        if (P_Random(pr_hexen) < 50)
        {                       // Immediate Quick flash
            NextLightningFlash = (P_Random(pr_hexen) & 15) + 16;
        }
        else
        {
            if (P_Random(pr_hexen) < 128 && !(leveltime & 32))
            {
                NextLightningFlash = ((P_Random(pr_hexen) & 7) + 2) * 35;
            }
            else
            {
                NextLightningFlash = ((P_Random(pr_hexen) & 15) + 5) * 35;
            }
        }
    }
}

void P_ForceLightning(void)
{
    NextLightningFlash = 0;
}

void P_InitLightning(void)
{
    int i;
    int secCount;

    if (!dsda_MapLightning())
    {
        LevelHasLightning = false;
        LightningFlash = 0;
        return;
    }
    LightningFlash = 0;
    secCount = 0;
    for (i = 0; i < numsectors; i++)
    {
        if (sectors[i].ceilingpic == skyflatnum
            || sectors[i].special == LIGHTNING_SPECIAL
            || sectors[i].special == LIGHTNING_SPECIAL2)
        {
            secCount++;
        }
    }
    if (secCount)
    {
        LevelHasLightning = true;
    }
    else
    {
        LevelHasLightning = false;
        return;
    }
    LightningLightLevels = (int *) Z_MallocLevel(secCount * sizeof(int));
    NextLightningFlash = ((P_Random(pr_hexen) & 15) + 5) * 35;  // don't flash at level start
}

void P_InitFTAnims(void)
{
    int base;
    int mod;
    int fd;
    animDef_t *ad;
    dboolean ignore;
    dboolean done;

    if (!map_format.animdefs) return;

    fd = 0;
    ad = AnimDefs;
    AnimDefCount = 0;
    SC_OpenLump(ANIM_SCRIPT_NAME);
    while (SC_GetString())
    {
        if (AnimDefCount == MAX_ANIM_DEFS)
        {
            I_Error("P_InitFTAnims: too many AnimDefs.");
        }
        if (SC_Compare(SCI_FLAT))
        {
            ad->type = ANIM_FLAT;
        }
        else if (SC_Compare(SCI_TEXTURE))
        {
            ad->type = ANIM_TEXTURE;
        }
        else
        {
            SC_ScriptError(NULL);
        }
        SC_MustGetString();     // Name
        ignore = false;
        if (ad->type == ANIM_FLAT)
        {
            if (!W_LumpNameExists2(sc_String, ns_flats))
            {
                ignore = true;
            }
            else
            {
                ad->index = R_FlatNumForName(sc_String);
            }
        }
        else
        {                       // Texture
            if (R_CheckTextureNumForName(sc_String) == -1)
            {
                ignore = true;
            }
            else
            {
                ad->index = R_TextureNumForName(sc_String);
            }
        }
        ad->startFrameDef = fd;
        done = false;
        while (done == false)
        {
            if (SC_GetString())
            {
                if (SC_Compare(SCI_PIC))
                {
                    if (fd == MAX_FRAME_DEFS)
                    {
                        I_Error("P_InitFTAnims: too many FrameDefs.");
                    }
                    SC_MustGetNumber();
                    if (ignore == false)
                    {
                        FrameDefs[fd].index = ad->index + sc_Number - 1;
                    }
                    SC_MustGetString();
                    if (SC_Compare(SCI_TICS))
                    {
                        SC_MustGetNumber();
                        if (ignore == false)
                        {
                            FrameDefs[fd].tics = sc_Number;
                            fd++;
                        }
                    }
                    else if (SC_Compare(SCI_RAND))
                    {
                        SC_MustGetNumber();
                        base = sc_Number;
                        SC_MustGetNumber();
                        if (ignore == false)
                        {
                            mod = sc_Number - base + 1;
                            FrameDefs[fd].tics = (base << 16) + (mod << 8);
                            fd++;
                        }
                    }
                    else
                    {
                        SC_ScriptError(NULL);
                    }
                }
                else
                {
                    SC_UnGet();
                    done = true;
                }
            }
            else
            {
                done = true;
            }
        }
        if ((ignore == false) && (fd - ad->startFrameDef < 2))
        {
            I_Error("P_InitFTAnims: AnimDef has framecount < 2.");
        }
        if (ignore == false)
        {
            ad->endFrameDef = fd - 1;
            ad->currentFrameDef = ad->endFrameDef;
            ad->tics = 1;       // Force 1st game tic to animate
            AnimDefCount++;
            ad++;
        }
    }
    SC_Close();
}
