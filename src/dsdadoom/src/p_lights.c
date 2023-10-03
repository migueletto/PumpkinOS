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
 *  Action routines for lighting thinkers
 *  Spawn sector based lighting effects.
 *  Handle lighting linedef types
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h" //jff 5/18/98
#include "doomdef.h"
#include "m_random.h"
#include "r_main.h"
#include "p_spec.h"
#include "p_tick.h"

#include "dsda/id_list.h"

//////////////////////////////////////////////////////////
//
// Lighting action routines, called once per tick
//
//////////////////////////////////////////////////////////

//
// T_FireFlicker()
//
// Firelight flicker action routine, called once per tick
//
// Passed a fireflicker_t structure containing light levels and timing
// Returns nothing
//
void T_FireFlicker (fireflicker_t* flick)
{
  int amount;

  if (--flick->count)
    return;

  amount = (P_Random(pr_lights)&3)*16;

  if (flick->sector->lightlevel - amount < flick->minlight)
    flick->sector->lightlevel = flick->minlight;
  else
    flick->sector->lightlevel = flick->maxlight - amount;

  flick->count = 4;
}

//
// T_LightFlash()
//
// Broken light flashing action routine, called once per tick
//
// Passed a lightflash_t structure containing light levels and timing
// Returns nothing
//
void T_LightFlash (lightflash_t* flash)
{
  if (--flash->count)
    return;

  if (flash->sector->lightlevel == flash->maxlight)
  {
    flash-> sector->lightlevel = flash->minlight;
    flash->count = (P_Random(pr_lights)&flash->mintime)+1;
  }
  else
  {
    flash-> sector->lightlevel = flash->maxlight;
    flash->count = (P_Random(pr_lights)&flash->maxtime)+1;
  }

}

//
// T_StrobeFlash()
//
// Strobe light flashing action routine, called once per tick
//
// Passed a strobe_t structure containing light levels and timing
// Returns nothing
//
void T_StrobeFlash (strobe_t*   flash)
{
  if (--flash->count)
    return;

  if (flash->sector->lightlevel == flash->minlight)
  {
    flash-> sector->lightlevel = flash->maxlight;
    flash->count = flash->brighttime;
  }
  else
  {
    flash-> sector->lightlevel = flash->minlight;
    flash->count =flash->darktime;
  }
}

//
// T_Glow()
//
// Glowing light action routine, called once per tick
//
// Passed a glow_t structure containing light levels and timing
// Returns nothing
//

void T_Glow(glow_t* g)
{
  switch(g->direction)
  {
    case -1:
      // light dims
      g->sector->lightlevel -= GLOWSPEED;
      if (g->sector->lightlevel <= g->minlight)
      {
        g->sector->lightlevel += GLOWSPEED;
        g->direction = 1;
      }
      break;

    case 1:
      // light brightens
      g->sector->lightlevel += GLOWSPEED;
      if (g->sector->lightlevel >= g->maxlight)
      {
        g->sector->lightlevel -= GLOWSPEED;
        g->direction = -1;
      }
      break;
  }
}

//////////////////////////////////////////////////////////
//
// Sector lighting type spawners
//
// After the map has been loaded, each sector is scanned
// for specials that spawn thinkers
//
//////////////////////////////////////////////////////////

//
// P_SpawnFireFlicker()
//
// Spawns a fire flicker lighting thinker
//
// Passed the sector that spawned the thinker
// Returns nothing
//
void P_SpawnFireFlicker (sector_t*  sector)
{
  fireflicker_t*  flick;

  P_ClearNonGeneralizedSectorSpecial(sector);

  flick = Z_MallocLevel ( sizeof(*flick));

  memset(flick, 0, sizeof(*flick));
  P_AddThinker (&flick->thinker);

  flick->thinker.function = T_FireFlicker;
  flick->sector = sector;
  sector->lightingdata = flick;
  flick->maxlight = sector->lightlevel;
  flick->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel)+16;
  flick->count = 4;
}

//
// P_SpawnLightFlash()
//
// Spawns a broken light flash lighting thinker
//
// Passed the sector that spawned the thinker
// Returns nothing
//
void P_SpawnLightFlash (sector_t* sector)
{
  lightflash_t* flash;

  P_ClearNonGeneralizedSectorSpecial(sector);

  flash = Z_MallocLevel ( sizeof(*flash));

  memset(flash, 0, sizeof(*flash));
  P_AddThinker (&flash->thinker);

  flash->thinker.function = T_LightFlash;
  flash->sector = sector;
  sector->lightingdata = flash;
  flash->maxlight = sector->lightlevel;

  flash->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel);
  flash->maxtime = 64;
  flash->mintime = 7;
  flash->count = (P_Random(pr_lights)&flash->maxtime)+1;
}

//
// P_SpawnStrobeFlash
//
// Spawns a blinking light thinker
//
// Passed the sector that spawned the thinker, speed of blinking
// and whether blinking is to by syncrhonous with other sectors
//
// Returns nothing
//
void P_SpawnStrobeFlash
( sector_t* sector,
  int   fastOrSlow,
  int   inSync )
{
  strobe_t* flash;

  flash = Z_MallocLevel ( sizeof(*flash));

  memset(flash, 0, sizeof(*flash));
  P_AddThinker (&flash->thinker);

  flash->sector = sector;
  sector->lightingdata = flash;
  flash->darktime = fastOrSlow;
  flash->brighttime = STROBEBRIGHT;
  flash->thinker.function = T_StrobeFlash;
  flash->maxlight = sector->lightlevel;
  flash->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);

  if (flash->minlight == flash->maxlight)
    flash->minlight = 0;

  P_ClearNonGeneralizedSectorSpecial(sector);

  if (!inSync)
    flash->count = (P_Random(pr_lights)&7)+1;
  else
    flash->count = 1;
}

//
// P_SpawnGlowingLight()
//
// Spawns a glowing light (smooth oscillation from min to max) thinker
//
// Passed the sector that spawned the thinker
// Returns nothing
//
void P_SpawnGlowingLight(sector_t*  sector)
{
  glow_t* g;

  g = Z_MallocLevel( sizeof(*g));

  memset(g, 0, sizeof(*g));
  P_AddThinker(&g->thinker);

  g->sector = sector;
  sector->lightingdata = g;
  g->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel);
  g->maxlight = sector->lightlevel;
  g->thinker.function = T_Glow;
  g->direction = -1;

  P_ClearNonGeneralizedSectorSpecial(sector);
}

//////////////////////////////////////////////////////////
//
// Linedef lighting function handlers
//
//////////////////////////////////////////////////////////

//
// EV_StartLightStrobing()
//
// Start strobing lights (usually from a trigger)
//
// Passed the line that activated the strobing
// Returns true
//
// jff 2/12/98 added int return value, fixed return
//
int EV_StartLightStrobing(line_t* line)
{
  const int *id_p;
  sector_t* sec;

  // start lights strobing in all sectors tagged same as line
  FIND_SECTORS(id_p, line->tag)
  {
    sec = &sectors[*id_p];

    // Original code never stored lighting data,
    //   so this only stops a light appearing in old complevels,
    //   and only when there is a floor or ceiling thinker.
    if (demo_compatibility && P_PlaneActive(sec))
      continue;

    P_SpawnStrobeFlash (sec,SLOWDARK, 0);
  }
  return 1;
}

//
// EV_TurnTagLightsOff()
//
// Turn line's tagged sector's lights to min adjacent neighbor level
//
// Passed the line that activated the lights being turned off
// Returns true
//
// jff 2/12/98 added int return value, fixed return
//
int EV_TurnTagLightsOff(line_t* line)
{
  const int *id_p;

  // search sectors for those with same tag as activating line

  // killough 10/98: replaced inefficient search with fast search
  FIND_SECTORS(id_p, line->tag)
  {
    sector_t *sector = sectors + *id_p, *tsec;
    int i, min = sector->lightlevel;
    // find min neighbor light level
    for (i = 0;i < sector->linecount; i++)
      if ((tsec = getNextSector(sector->lines[i], sector)) &&
        tsec->lightlevel < min)
    min = tsec->lightlevel;
    sector->lightlevel = min;
  }
  return 1;
}

//
// EV_LightTurnOn()
//
// Turn sectors tagged to line lights on to specified or max neighbor level
//
// Passed the activating line, and a level to set the light to
// If level passed is 0, the maximum neighbor lighting is used
// Returns true
//
// jff 2/12/98 added int return value, fixed return
//
int EV_LightTurnOn(line_t *line, int bright)
{
  const int *id_p;

  // search all sectors for ones with same tag as activating line

  // killough 10/98: replace inefficient search with fast search
  FIND_SECTORS(id_p, line->tag)
  {
    sector_t *temp, *sector = sectors+*id_p;
    int j, tbright = bright; //jff 5/17/98 search for maximum PER sector

    // bright = 0 means to search for highest light level surrounding sector

    if (!bright)
      for (j = 0;j < sector->linecount; j++)
        if ((temp = getNextSector(sector->lines[j],sector)) &&
            temp->lightlevel > tbright)
    tbright = temp->lightlevel;

    sector->lightlevel = tbright;

    //jff 5/17/98 unless compatibility optioned
    //then maximum near ANY tagged sector
    if (comp[comp_model])
      bright = tbright;
  }
  return 1;
}

/* killough 10/98:
 *
 * EV_LightTurnOnPartway()
 *
 * Turn sectors tagged to line lights on to specified or max neighbor level
 *
 * Passed the activating line, and a light level fraction between 0 and 1.
 * Sets the light to min on 0, max on 1, and interpolates in-between.
 * Used for doors with gradual lighting effects.
 *
 * Returns true
 */

int EV_LightTurnOnPartway(line_t *line, fixed_t level)
{
  const int *id_p;

  if (level < 0)          // clip at extremes
    level = 0;
  if (level > FRACUNIT)
    level = FRACUNIT;

  // search all sectors for ones with same tag as activating line
  FIND_SECTORS(id_p, line->tag)
  {
    sector_t *temp, *sector = sectors+*id_p;
    int j, bright = 0, min = sector->lightlevel;

    for (j = 0; j < sector->linecount; j++)
      if ((temp = getNextSector(sector->lines[j],sector)))
      {
        if (temp->lightlevel > bright)
          bright = temp->lightlevel;
        if (temp->lightlevel < min)
          min = temp->lightlevel;
      }

    sector->lightlevel =   // Set level in-between extremes
      (level * bright + (FRACUNIT-level) * min) >> FRACBITS;
  }
  return 1;
}

void EV_LightChange(int tag, short change)
{
  const int *id_p;

  FIND_SECTORS(id_p, tag)
    sectors[*id_p].lightlevel += change;
}

void EV_LightSet(int tag, short level)
{
  const int *id_p;

  FIND_SECTORS(id_p, tag)
    sectors[*id_p].lightlevel = level;
}

void EV_LightSetMinNeighbor(int tag)
{
  const int *id_p;

  FIND_SECTORS(id_p, tag)
  {
    int i;
    short level;
    sector_t *temp, *sector;

    sector = &sectors[*id_p];
    level = sector->lightlevel;

    for (i = 0; i < sector->linecount; i++)
      if ((temp = getNextSector(sector->lines[i], sector)) && temp->lightlevel < level)
        level = temp->lightlevel;

    sector->lightlevel = level;
  }
}

void EV_LightSetMaxNeighbor(int tag)
{
  const int *id_p;

  FIND_SECTORS(id_p, tag)
  {
    int i;
    short level;
    sector_t *temp, *sector;

    sector = &sectors[*id_p];
    level = 0;

    for (i = 0; i < sector->linecount; i++)
      if ((temp = getNextSector(sector->lines[i], sector)) && temp->lightlevel > level)
        level = temp->lightlevel;

    sector->lightlevel = level;
  }
}

void T_ZDoom_Glow(zdoom_glow_t *g)
{
  if (g->tics++ >= g->maxtics)
  {
    if (g->oneshot)
    {
      g->sector->lightlevel = g->endlevel;
      g->sector->lightingdata = NULL;
      P_RemoveThinker(&g->thinker);
      return;
    }
    else
    {
      short temp = g->startlevel;
      g->startlevel = g->endlevel;
      g->endlevel = temp;
      g->tics -= g->maxtics;
    }
  }

  g->sector->lightlevel = g->tics * (g->endlevel - g->startlevel) / g->maxtics + g->startlevel;
}

static void P_SpawnZDoomLightGlow(sector_t *sec, short startlevel, short endlevel,
                                  short maxtics, dboolean oneshot)
{
  zdoom_glow_t *g;

  g = Z_MallocLevel(sizeof(*g));

  memset(g, 0, sizeof(*g));
  P_AddThinker(&g->thinker);
  g->thinker.function = T_ZDoom_Glow;

  g->sector = sec;
  sec->lightingdata = g;
  g->startlevel = startlevel;
  g->endlevel = endlevel;
  g->tics = -1;
  g->maxtics = maxtics;
  g->oneshot = oneshot;
}

void EV_StartLightFading(int tag, short level, short tics)
{
  const int *id_p;

  FIND_SECTORS(id_p, tag)
  {
    sector_t *sec = &sectors[*id_p];

    if (sec->lightingdata || sec->lightlevel == level)
      continue;

    if (tics)
    {
      P_SpawnZDoomLightGlow(sec, sec->lightlevel, level, tics, true);
    }
    else
    {
      sec->lightlevel = level;
    }
  }
}

void EV_StartLightGlowing(int tag, short upper, short lower, short tics)
{
  const int *id_p;

  if (tics == 0)
    return;

  if (upper < lower)
  {
    short temp = upper;
    upper = lower;
    lower = temp;
  }

  FIND_SECTORS(id_p, tag)
  {
    sector_t *sec = &sectors[*id_p];

    if (sec->lightingdata)
      continue;

    P_SpawnZDoomLightGlow(sec, upper, lower, tics, false);
  }
}

void T_ZDoom_Flicker(zdoom_flicker_t *g)
{
  if (g->count)
  {
    g->count--;
  }
  else if (g->sector->lightlevel == g->upper)
  {
    g->sector->lightlevel = g->lower;
    g->count = (P_Random(pr_lights) & 7) + 1;
  }
  else
  {
    g->sector->lightlevel = g->upper;
    g->count = (P_Random(pr_lights) & 31) + 1;
  }
}

static void P_SpawnZDoomLightFlicker(sector_t *sec, short upper, short lower)
{
  zdoom_flicker_t *g;

  g = Z_MallocLevel(sizeof(*g));

  memset(g, 0, sizeof(*g));
  P_AddThinker(&g->thinker);
  g->thinker.function = T_ZDoom_Flicker;

  g->sector = sec;
  sec->lightingdata = g;
  g->upper = upper;
  g->lower = lower;
  g->count = (P_Random(pr_lights) & 64) + 1;
}

void EV_StartLightFlickering(int tag, short upper, short lower)
{
  const int *id_p;

  FIND_SECTORS(id_p, tag)
  {
    sector_t *sec = &sectors[*id_p];

    if (sec->lightingdata)
      continue;

    P_SpawnZDoomLightFlicker(sec, upper, lower);
  }
}

static void P_SpawnZDoomLightStrobe(sector_t *sector, int upper, int lower,
                             int brighttime, int darktime, int count)
{
  strobe_t* g;

  g = Z_MallocLevel ( sizeof(*g));

  memset(g, 0, sizeof(*g));
  P_AddThinker (&g->thinker);

  g->sector = sector;
  sector->lightingdata = g;
  g->darktime = darktime;
  g->brighttime = brighttime;
  g->thinker.function = T_StrobeFlash;
  g->maxlight = upper;
  g->minlight = lower;

  if (g->minlight == g->maxlight)
    g->minlight = 0;

  g->count = count;
}

static void P_SpawnZDoomLightStrobeDoom(sector_t* sector, int brighttime, int darktime)
{
  int count = (P_Random(pr_lights) & 7) + 1;

  P_SpawnZDoomLightStrobe(
    sector, sector->lightlevel, P_FindMinSurroundingLight(sector, sector->lightlevel),
    brighttime, darktime, count
  );
}

void EV_StartZDoomLightStrobing(int tag, int upper, int lower, int brighttime, int darktime)
{
  const int *id_p;

  FIND_SECTORS(id_p, tag)
  {
    sector_t *sec = &sectors[*id_p];

    if (sec->lightingdata)
      continue;

    P_SpawnZDoomLightStrobe(sec, upper, lower, brighttime, darktime, 1);
  }
}

void EV_StartZDoomLightStrobingDoom(int tag, int brighttime, int darktime)
{
  const int *id_p;

  FIND_SECTORS(id_p, tag)
  {
    sector_t *sec = &sectors[*id_p];

    if (sec->lightingdata)
      continue;

    P_SpawnZDoomLightStrobeDoom(sec, brighttime, darktime);
  }
}

void EV_StopLightEffect(int tag)
{
  const int *id_p;

  FIND_SECTORS(id_p, tag)
  {
    sector_t *sec = &sectors[*id_p];

    if (sec->lightingdata)
    {
      P_RemoveThinker((thinker_t *) sec->lightingdata);
      sec->lightingdata = NULL;
    }
  }
}

// hexen

void T_Light(light_t * light)
{
    if (light->count)
    {
        light->count--;
        return;
    }
    switch (light->type)
    {
        case LITE_FADE:
            light->sector->lightlevel =
                ((light->sector->lightlevel << FRACBITS) +
                 light->value2) >> FRACBITS;
            if (light->tics2 == 1)
            {
                if (light->sector->lightlevel >= light->value1)
                {
                    light->sector->lightlevel = light->value1;
                    P_RemoveThinker(&light->thinker);
                }
            }
            else if (light->sector->lightlevel <= light->value1)
            {
                light->sector->lightlevel = light->value1;
                P_RemoveThinker(&light->thinker);
            }
            break;
        case LITE_GLOW:
            light->sector->lightlevel =
                ((light->sector->lightlevel << FRACBITS) +
                 light->tics1) >> FRACBITS;
            if (light->tics2 == 1)
            {
                if (light->sector->lightlevel >= light->value1)
                {
                    light->sector->lightlevel = light->value1;
                    light->tics1 = -light->tics1;
                    light->tics2 = -1;  // reverse direction
                }
            }
            else if (light->sector->lightlevel <= light->value2)
            {
                light->sector->lightlevel = light->value2;
                light->tics1 = -light->tics1;
                light->tics2 = 1;       // reverse direction
            }
            break;
        case LITE_FLICKER:
            if (light->sector->lightlevel == light->value1)
            {
                light->sector->lightlevel = light->value2;
                light->count = (P_Random(pr_hexen) & 7) + 1;
            }
            else
            {
                light->sector->lightlevel = light->value1;
                light->count = (P_Random(pr_hexen) & 31) + 1;
            }
            break;
        case LITE_STROBE:
            if (light->sector->lightlevel == light->value1)
            {
                light->sector->lightlevel = light->value2;
                light->count = light->tics2;
            }
            else
            {
                light->sector->lightlevel = light->value1;
                light->count = light->tics1;
            }
            break;
        default:
            break;
    }
}

dboolean EV_SpawnLight(line_t * line, byte * arg, lighttype_t type)
{
    light_t *light;
    sector_t *sec;
    const int *id_p;
    int arg1, arg2, arg3, arg4;
    dboolean think;
    dboolean rtn;

    arg1 = arg[1];
    arg2 = arg[2];
    arg3 = arg[3];
    arg4 = arg[4];

    rtn = false;
    FIND_SECTORS(id_p, arg[0])
    {
        think = false;
        sec = &sectors[*id_p];

        light = (light_t *) Z_MallocLevel(sizeof(light_t));
        light->type = type;
        light->sector = sec;
        light->count = 0;
        rtn = true;
        switch (type)
        {
            case LITE_RAISEBYVALUE:
                sec->lightlevel += arg1;
                if (sec->lightlevel > 255)
                {
                    sec->lightlevel = 255;
                }
                break;
            case LITE_LOWERBYVALUE:
                sec->lightlevel -= arg1;
                if (sec->lightlevel < 0)
                {
                    sec->lightlevel = 0;
                }
                break;
            case LITE_CHANGETOVALUE:
                sec->lightlevel = arg1;
                if (sec->lightlevel < 0)
                {
                    sec->lightlevel = 0;
                }
                else if (sec->lightlevel > 255)
                {
                    sec->lightlevel = 255;
                }
                break;
            case LITE_FADE:
                think = true;
                light->value1 = arg1;   // destination lightlevel
                light->value2 = FixedDiv((arg1 - sec->lightlevel) << FRACBITS, arg2 << FRACBITS);       // delta lightlevel
                if (sec->lightlevel <= arg1)
                {
                    light->tics2 = 1;   // get brighter
                }
                else
                {
                    light->tics2 = -1;
                }
                break;
            case LITE_GLOW:
                think = true;
                light->value1 = arg1;   // upper lightlevel
                light->value2 = arg2;   // lower lightlevel
                light->tics1 = FixedDiv((arg1 - sec->lightlevel) << FRACBITS, arg3 << FRACBITS);        // lightlevel delta
                if (sec->lightlevel <= arg1)
                {
                    light->tics2 = 1;   // get brighter
                }
                else
                {
                    light->tics2 = -1;
                }
                break;
            case LITE_FLICKER:
                think = true;
                light->value1 = arg1;   // upper lightlevel
                light->value2 = arg2;   // lower lightlevel
                sec->lightlevel = light->value1;
                light->count = (P_Random(pr_hexen) & 64) + 1;
                break;
            case LITE_STROBE:
                think = true;
                light->value1 = arg1;   // upper lightlevel
                light->value2 = arg2;   // lower lightlevel
                light->tics1 = arg3;    // upper tics
                light->tics2 = arg4;    // lower tics
                light->count = arg3;
                sec->lightlevel = light->value1;
                break;
            default:
                rtn = false;
                break;
        }
        if (think)
        {
            P_AddThinker(&light->thinker);
            light->thinker.function = T_Light;
        }
        else
        {
            Z_Free(light);
        }
    }
    return rtn;
}

static int PhaseTable[64] = {
    128, 112, 96, 80, 64, 48, 32, 32,
    16, 16, 16, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 16, 16, 16,
    32, 32, 48, 64, 80, 96, 112, 128
};

void T_Phase(phase_t * phase)
{
    phase->index = (phase->index + 1) & 63;
    phase->sector->lightlevel = phase->base + PhaseTable[phase->index];
}

void P_SpawnPhasedLight(sector_t * sector, int base, int index)
{
    phase_t *phase;

    phase = Z_MallocLevel(sizeof(*phase));
    P_AddThinker(&phase->thinker);
    phase->sector = sector;
    sector->lightingdata = phase;
    if (index == -1)
    {                           // sector->lightlevel as the index
        phase->index = sector->lightlevel & 63;
    }
    else
    {
        phase->index = index & 63;
    }
    phase->base = base & 255;
    sector->lightlevel = phase->base + PhaseTable[phase->index];
    phase->thinker.function = T_Phase;

    P_ClearNonGeneralizedSectorSpecial(sector);
}

void P_SpawnLightSequence(sector_t * sector, int indexStep)
{
    sector_t *sec;
    sector_t *nextSec;
    sector_t *tempSec;
    int seqSpecial;
    int i;
    int count;
    fixed_t index;
    fixed_t indexDelta;
    int base;

    seqSpecial = LIGHT_SEQUENCE;        // look for Light_Sequence, first
    sec = sector;
    count = 1;
    do
    {
        nextSec = NULL;
        sec->special = LIGHT_SEQUENCE_START;    // make sure that the search doesn't back up.
        for (i = 0; i < sec->linecount; i++)
        {
            tempSec = getNextSector(sec->lines[i], sec);
            if (!tempSec)
            {
                continue;
            }
            if (tempSec->special == seqSpecial)
            {
                if (seqSpecial == LIGHT_SEQUENCE)
                {
                    seqSpecial = LIGHT_SEQUENCE_ALT;
                }
                else
                {
                    seqSpecial = LIGHT_SEQUENCE;
                }
                nextSec = tempSec;
                count++;
            }
        }
        sec = nextSec;
    }
    while (sec);

    sec = sector;
    count *= indexStep;
    index = 0;
    indexDelta = FixedDiv(64 * FRACUNIT, count * FRACUNIT);
    base = sector->lightlevel;
    do
    {
        nextSec = NULL;
        if (sec->lightlevel)
        {
            base = sec->lightlevel;
        }
        P_SpawnPhasedLight(sec, base, index >> FRACBITS);
        index += indexDelta;
        for (i = 0; i < sec->linecount; i++)
        {
            tempSec = getNextSector(sec->lines[i], sec);
            if (!tempSec)
            {
                continue;
            }
            if (tempSec->special == LIGHT_SEQUENCE_START)
            {
                nextSec = tempSec;
            }
        }
        sec = nextSec;
    }
    while (sec);
}
