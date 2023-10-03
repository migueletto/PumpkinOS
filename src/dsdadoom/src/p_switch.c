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
 *  Switches, buttons. Two-state animation. Exits.
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_spec.h"
#include "g_game.h"
#include "s_sound.h"
#include "sounds.h"
#include "lprintf.h"
#include "e6y.h"//e6y

#include "dsda.h"
#include "dsda/map_format.h"

//==================================================================
//
//      CHANGE THE TEXTURE OF A WALL SWITCH TO ITS OPPOSITE
//
//==================================================================
const switchlist_t heretic_alphSwitchList[] = {
  { "SW1OFF", "SW1ON", 1 },
  { "SW2OFF", "SW2ON", 1 },
  { "",        "",     0 }
};

const switchlist_t hexen_alphSwitchList[] = {
    {"SW_1_UP", "SW_1_DN", hexen_sfx_switch1},
    {"SW_2_UP", "SW_2_DN", hexen_sfx_switch1},
    {"VALVE1", "VALVE2", hexen_sfx_valve_turn},
    {"SW51_OFF", "SW51_ON", hexen_sfx_switch2},
    {"SW52_OFF", "SW52_ON", hexen_sfx_switch2},
    {"SW53_UP", "SW53_DN", hexen_sfx_rope_pull},
    {"PUZZLE5", "PUZZLE9", hexen_sfx_switch1},
    {"PUZZLE6", "PUZZLE10", hexen_sfx_switch1},
    {"PUZZLE7", "PUZZLE11", hexen_sfx_switch1},
    {"PUZZLE8", "PUZZLE12", hexen_sfx_switch1},
    {"\0", "\0", 0}
};

// killough 2/8/98: Remove switch limit

static int *switchlist;                           // killough
static int max_numswitches;                       // killough
static int numswitches;                           // killough

button_t  buttonlist[MAXBUTTONS];

const switchlist_t *alphSwitchList;         //jff 3/23/98 pointer to switch table

//
// P_InitSwitchList()
//
// Only called at game initialization in order to list the set of switches
// and buttons known to the engine. This enables their texture to change
// when activated, and in the case of buttons, change back after a timeout.
//
// This routine modified to read its data from a predefined lump or
// PWAD lump called SWITCHES rather than a static table in this module to
// allow wad designers to insert or modify switches.
//
// Lump format is an array of byte packed switchlist_t structures, terminated
// by a structure with episode == -0. The lump can be generated from a
// text source file using SWANTBLS.EXE, distributed with the BOOM utils.
// The standard list of switches and animations is contained in the example
// source text file DEFSWANI.DAT also in the BOOM util distribution.
//
// Rewritten by Lee Killough to remove limit 2/8/98
//
void P_InitSwitchList(void)
{
  int lump = -1;
  int i, index = 0;
  int episode = (gamemode == registered || gamemode==retail) ?
                 2 : gamemode == commercial ? 3 : 1;

  // MAP_FORMAT_TODO: switch list?
  if (heretic)
  {
    alphSwitchList = heretic_alphSwitchList;
  }
  else if (hexen)
  {
    alphSwitchList = hexen_alphSwitchList;
  }
  else
  {
    lump = W_GetNumForName("SWITCHES"); // cph - new wad lump handling

    //jff 3/23/98 read the switch table from a predefined lump
    alphSwitchList = (const switchlist_t *)W_LumpByNum(lump);
  }

  for (i=0;;i++)
  {
    if (index+1 >= max_numswitches)
      switchlist = Z_Realloc(switchlist, sizeof *switchlist *
          (max_numswitches = max_numswitches ? max_numswitches*2 : 8));

    // hexen overrides the episode field with a sound index
    if (hexen || LittleShort(alphSwitchList[i].episode) <= episode) //jff 5/11/98 endianess
    {
      int texture1, texture2;

      if (!LittleShort(alphSwitchList[i].episode))
        break;

      // Ignore switches referencing unknown texture names, instead of exiting.
      // Warn if either one is missing, but only add if both are valid.
      texture1 = R_CheckTextureNumForName(alphSwitchList[i].name1);
      if (texture1 == -1)
        lprintf(LO_WARN, "P_InitSwitchList: unknown texture %s\n",
            alphSwitchList[i].name1);

      texture2 = R_CheckTextureNumForName(alphSwitchList[i].name2);
      if (texture2 == -1)
        lprintf(LO_WARN, "P_InitSwitchList: unknown texture %s\n",
            alphSwitchList[i].name2);

      if (texture1 != -1 && texture2 != -1) {
        switchlist[index++] = texture1;
        switchlist[index++] = texture2;
      }
    }
  }

  numswitches = index / 2;
  switchlist[index] = -1;
}

//
// P_StartButton()
//
// Start a button (retriggerable switch) counting down till it turns off.
//
// Passed the linedef the button is on, which texture on the sidedef contains
// the button, the texture number of the button, and the time the button is
// to remain active in gametics.
// No return.
//
static void P_StartButton
( line_t*       line,
  bwhere_e      w,
  int           texture,
  int           time )
{
  int           i;

  if (!raven)
    // See if button is already pressed
    for (i = 0;i < MAXBUTTONS;i++)
      if (buttonlist[i].btimer && buttonlist[i].line == line)
        return;

  for (i = 0;i < MAXBUTTONS;i++)
    if (!buttonlist[i].btimer)    // use first unused element of list
    {
      buttonlist[i].line = line;
      buttonlist[i].where = w;
      buttonlist[i].btexture = texture;
      buttonlist[i].btimer = time;
      /* use sound origin of line itself - no need to compatibility-wrap
       * as the popout code gets it wrong whatever its value */
      buttonlist[i].soundorg = &line->soundorg;
      return;
    }

  I_Error("P_StartButton: no button slots left!");
}

//
// P_ChangeSwitchTexture()
//
// Function that changes switch wall texture on activation.
//
// Passed the line which the switch is on, and whether its retriggerable.
// If not retriggerable, this function clears the line special to insure that
//
// No return
//
void P_ChangeSwitchTexture
( line_t*       line,
  int           useAgain )
{
  /* Rearranged a bit to avoid too much code duplication */
  degenmobj_t *soundorg;
  int     i, sound;
  short   *texture, *ttop, *tmid, *tbot;
  bwhere_e position;

  ttop = &sides[line->sidenum[0]].toptexture;
  tmid = &sides[line->sidenum[0]].midtexture;
  tbot = &sides[line->sidenum[0]].bottomtexture;

  /* don't zero line->special until after exit switch test */
  if (!hexen && !useAgain)
    line->special = 0;

  /* search for a texture to change */
  texture = NULL; position = 0;
  for (i = 0; i < numswitches * 2; i++) { /* this could be more efficient... */
    if (switchlist[i] == *ttop) {
      texture = ttop; position = top; break;
    } else if (switchlist[i] == *tmid) {
      texture = tmid; position = middle; break;
    } else if (switchlist[i] == *tbot) {
      texture = tbot; position = bottom; break;
    }
  }
  if (texture == NULL)
    return; /* no switch texture was found to change */
  *texture = switchlist[i^1];

  if (hexen)
  {
    // hexen has sound id in episode field
    sound = alphSwitchList[i / 2].episode;
    soundorg = &line->frontsector->soundorg;
  }
  else
  {
    sound = g_sfx_swtchn;
    /* use the sound origin of the linedef (its midpoint)
     * unless in a compatibility mode */
    soundorg = &line->soundorg;
    if (comp[comp_sound] || compatibility_level < prboom_6_compatibility) {
      /* usually NULL, unless there is another button already pressed in,
       * in which case it's the sound origin of that button press... */
      soundorg = buttonlist->soundorg;
    }
  }

  S_StartLineSound(line, soundorg, sound);

  if (useAgain)
    P_StartButton(line, position, switchlist[i], BUTTONTIME);
}

static dboolean P_IsSwitchTexture(short texture)
{
  int i;

  for (i = 0; i < numswitches * 2; ++i)
    if (switchlist[i] == texture)
      return true;

  return false;
}

dboolean P_CheckSwitchRange(line_t *line, mobj_t *mo, int sideno)
{
  side_t *side;
  sector_t *front;
  dboolean can_hit_top, can_hit_bottom;
  dboolean found_switch;

  // Is it possible to use a side that doesn't exist?
  if (line->sidenum[sideno] == NO_INDEX)
  {
    return true;
  }

  side = &sides[line->sidenum[sideno]];
  front = side->sector;

  if (mo->z + mo->height <= front->floorheight || mo->z >= front->ceilingheight)
  {
    return false;
  }

  // one-sided
  if (line->sidenum[1] == NO_INDEX)
  {
    return true;
  }

  P_LineOpening(line, NULL);

  // acts like one-sided
  if (line_opening.range <= 0)
  {
    return true;
  }

  can_hit_top = (front->ceilingheight > line_opening.top) &&
                (mo->z + mo->height > line_opening.top && mo->z < front->ceilingheight);

  can_hit_bottom = (front->floorheight < line_opening.bottom) &&
                   (mo->z + mo->height > front->floorheight && mo->z < line_opening.bottom);

  found_switch = false;

  if (side->toptexture && P_IsSwitchTexture(side->toptexture))
  {
    found_switch = true;

    if (can_hit_top)
    {
      return true;
    }
  }

  if (side->bottomtexture && P_IsSwitchTexture(side->bottomtexture))
  {
    found_switch = true;

    if (can_hit_bottom)
    {
      return true;
    }
  }

  if (side->midtexture && P_IsSwitchTexture(side->midtexture))
  {
    fixed_t top, bottom;

    found_switch = true;

    if (P_GetMidTexturePosition(line, sideno, &top, &bottom))
    {
      if (front->ceilingheight > bottom && front->floorheight < top)
      {
        if (mo->z + mo->height > bottom && mo->z < top)
        {
          return true;
        }
      }
    }
  }

  return !found_switch && (can_hit_top || can_hit_bottom);
}

//
// GetPairForSwitchTexture
//
int GetPairForSwitchTexture(side_t *side)
{
  int i;
  short *texture, *ttop, *tmid, *tbot;

  ttop = &side->toptexture;
  tmid = &side->midtexture;
  tbot = &side->bottomtexture;

  /* search for a texture to change */
  texture = NULL;
  for (i = 0;i < numswitches*2;i++) {
    if (switchlist[i] == *ttop) {
      texture = ttop; break;
    } else if (switchlist[i] == *tmid) {
      texture = tmid; break;
    } else if (switchlist[i] == *tbot) {
      texture = tbot; break;
    }
  }

  if (texture == NULL)
    return -1;

  return switchlist[i^1];
}

//
// P_UseSpecialLine
//
//
// Called when a thing uses (pushes) a special line.
// Only the front sides of lines are usable.
// Dispatches to the appropriate linedef function handler.
//
// Passed the thing using the line, the line being used, and the side used
// Returns true if a thinker was created
//
dboolean
P_UseSpecialLine
( mobj_t*       thing,
  line_t*       line,
  int           side,
  dboolean		bossaction)
{
  dsda_WatchLineActivation(line, thing);

  if (map_format.hexen)
  {
    if (side)
    {
      if (line->activation & SPAC_USEBACK)
      {
        return P_ActivateLine(line, thing, side, SPAC_USEBACK);
      }

      return false;
    }

    return P_ActivateLine(line, thing, side, SPAC_USE);
  }

  // e6y
  // b.m. side test was broken in boom201
  if ((demoplayback ? (demover != 201) : (compatibility_level != boom_201_compatibility)))
    if (side) //jff 6/1/98 fix inadvertent deletion of side test
      return false;

  if (heretic) return Heretic_P_UseSpecialLine(thing, line, side, bossaction);

  //jff 02/04/98 add check here for generalized floor/ceil mover
  if (!demo_compatibility)
  {
    // pointer to line function is NULL by default, set non-null if
    // line special is push or switch generalized linedef type
    int (*linefunc)(line_t *line)=NULL;

    // check each range of generalized linedefs
    if ((unsigned)line->special >= GenEnd)
    {
      // Out of range for GenFloors
    }
    else if ((unsigned)line->special >= GenFloorBase)
    {
      if (!thing->player && !bossaction)
        if ((line->special & FloorChange) || !(line->special & FloorModel))
          return false; // FloorModel is "Allow Monsters" if FloorChange is 0
      if (!line->tag && ((line->special&6)!=6)) //jff 2/27/98 all non-manual
        return false;                         // generalized types require tag
      linefunc = EV_DoGenFloor;
    }
    else if ((unsigned)line->special >= GenCeilingBase)
    {
      if (!thing->player && !bossaction)
        if ((line->special & CeilingChange) || !(line->special & CeilingModel))
          return false;   // CeilingModel is "Allow Monsters" if CeilingChange is 0
      if (!line->tag && ((line->special&6)!=6)) //jff 2/27/98 all non-manual
        return false;                         // generalized types require tag
      linefunc = EV_DoGenCeiling;
    }
    else if ((unsigned)line->special >= GenDoorBase)
    {
      if (!thing->player && !bossaction)
      {
        if (!(line->special & DoorMonster))
          return false;   // monsters disallowed from this door
        if (line->flags & ML_SECRET) // they can't open secret doors either
          return false;
      }
      if (!line->tag && ((line->special&6)!=6)) //jff 3/2/98 all non-manual
        return false;                         // generalized types require tag
      linefunc = EV_DoGenDoor;
    }
    else if ((unsigned)line->special >= GenLockedBase)
    {
      if (!thing->player || bossaction)
        return false;   // monsters disallowed from unlocking doors
      if (!P_CanUnlockGenDoor(line,thing->player))
        return false;
      if (!line->tag && ((line->special&6)!=6)) //jff 2/27/98 all non-manual
        return false;                         // generalized types require tag

      linefunc = EV_DoGenLockedDoor;
    }
    else if ((unsigned)line->special >= GenLiftBase)
    {
      if (!thing->player && !bossaction)
        if (!(line->special & LiftMonster))
          return false; // monsters disallowed
      if (!line->tag && ((line->special&6)!=6)) //jff 2/27/98 all non-manual
        return false;                         // generalized types require tag
      linefunc = EV_DoGenLift;
    }
    else if ((unsigned)line->special >= GenStairsBase)
    {
      if (!thing->player && !bossaction)
        if (!(line->special & StairMonster))
          return false; // monsters disallowed
      if (!line->tag && ((line->special&6)!=6)) //jff 2/27/98 all non-manual
        return false;                         // generalized types require tag
      linefunc = EV_DoGenStairs;
    }
    else if ((unsigned)line->special >= GenCrusherBase)
    {
      if (!thing->player && !bossaction)
        if (!(line->special & CrusherMonster))
          return false; // monsters disallowed
      if (!line->tag && ((line->special&6)!=6)) //jff 2/27/98 all non-manual
        return false;                         // generalized types require tag
      linefunc = EV_DoGenCrusher;
    }

    if (linefunc)
      switch((line->special & TriggerType) >> TriggerTypeShift)
      {
        case PushOnce:
          if (!side)
            if (linefunc(line))
              line->special = 0;
          return true;
        case PushMany:
          if (!side)
            linefunc(line);
          return true;
        case SwitchOnce:
          if (linefunc(line))
            P_ChangeSwitchTexture(line,0);
          return true;
        case SwitchMany:
          if (linefunc(line))
            P_ChangeSwitchTexture(line,1);
          return true;
        default:  // if not a switch/push type, do nothing here
          return false;
      }
  }

  // Switches that other things can activate.
  if (!thing->player && !bossaction)
  {
    // never open secret doors
    if (line->flags & ML_SECRET)
      return false;

    switch(line->special)
    {
      case 1:         // MANUAL DOOR RAISE
      case 32:        // MANUAL BLUE
      case 33:        // MANUAL RED
      case 34:        // MANUAL YELLOW
      //jff 3/5/98 add ability to use teleporters for monsters
      case 195:       // switch teleporters
      case 174:
      case 210:       // silent switch teleporters
      case 209:
        break;

      default:
        return false;
        break;
    }
  }

  if (bossaction)
  {
    switch(line->special)
    {
		// 0-tag specials, locked switches and teleporters need to be blocked for boss actions.
      case 1:         // MANUAL DOOR RAISE
      case 32:        // MANUAL BLUE
      case 33:        // MANUAL RED
      case 34:        // MANUAL YELLOW
	  case 117:       // Blazing door raise
	  case 118:       // Blazing door open
	  case 133:		  // BlzOpenDoor BLUE
	  case 135:		  // BlzOpenDoor RED
	  case 137:		  // BlzOpenDoor YEL

	  case 99:		  // BlzOpenDoor BLUE
	  case 134:		  // BlzOpenDoor RED
	  case 136:		  // BlzOpenDoor YELLOW

					  //jff 3/5/98 add ability to use teleporters for monsters
      case 195:       // switch teleporters
      case 174:
      case 210:       // silent switch teleporters
      case 209:
        return false;
        break;
    }
  }

  if (!P_CheckTag(line))  //jff 2/27/98 disallow zero tag on some types
    return false;

  // Dispatch to handler according to linedef type
  switch (line->special)
  {
    // Manual doors, push type with no tag
    case 1:             // Vertical Door
    case 26:            // Blue Door/Locked
    case 27:            // Yellow Door /Locked
    case 28:            // Red Door /Locked

    case 31:            // Manual door open
    case 32:            // Blue locked door open
    case 33:            // Red locked door open
    case 34:            // Yellow locked door open

    case 117:           // Blazing door raise
    case 118:           // Blazing door open
      EV_VerticalDoor (line, thing);
      break;

    // Switches (non-retriggerable)
    case 7:
      // Build Stairs
      if (EV_BuildStairs(line,build8))
        P_ChangeSwitchTexture(line,0);
      break;

    case 9:
      // Change Donut
      if (EV_DoDonut(line))
        P_ChangeSwitchTexture(line,0);
      break;

    case 11:
      /* Exit level
       * killough 10/98: prevent zombies from exiting levels
       */
      if (!bossaction && thing->player && thing->player->health <= 0 && !comp[comp_zombie])
      {
        S_StartMobjSound(thing, sfx_noway);
        return false;
      }

      P_ChangeSwitchTexture(line,0);
      G_ExitLevel(0);
      break;

    case 14:
      // Raise Floor 32 and change texture
      if (EV_DoPlat(line,raiseAndChange,32))
        P_ChangeSwitchTexture(line,0);
      break;

    case 15:
      // Raise Floor 24 and change texture
      if (EV_DoPlat(line,raiseAndChange,24))
        P_ChangeSwitchTexture(line,0);
      break;

    case 18:
      // Raise Floor to next highest floor
      if (EV_DoFloor(line, raiseFloorToNearest))
        P_ChangeSwitchTexture(line,0);
      break;

    case 20:
      // Raise Plat next highest floor and change texture
      if (EV_DoPlat(line,raiseToNearestAndChange,0))
        P_ChangeSwitchTexture(line,0);
      break;

    case 21:
      // PlatDownWaitUpStay
      if (EV_DoPlat(line,downWaitUpStay,0))
        P_ChangeSwitchTexture(line,0);
      break;

    case 23:
      // Lower Floor to Lowest
      if (EV_DoFloor(line,lowerFloorToLowest))
        P_ChangeSwitchTexture(line,0);
      break;

    case 29:
      // Raise Door
      if (EV_DoDoor(line,normal))
        P_ChangeSwitchTexture(line,0);
      break;

    case 41:
      // Lower Ceiling to Floor
      if (EV_DoCeiling(line,lowerToFloor))
        P_ChangeSwitchTexture(line,0);
      break;

    case 71:
      // Turbo Lower Floor
      if (EV_DoFloor(line,turboLower))
        P_ChangeSwitchTexture(line,0);
      break;

    case 49:
      // Ceiling Crush And Raise
      if (EV_DoCeiling(line,crushAndRaise))
        P_ChangeSwitchTexture(line,0);
      break;

    case 50:
      // Close Door
      if (EV_DoDoor(line,closeDoor))
        P_ChangeSwitchTexture(line,0);
      break;

    case 51:
      /* Secret EXIT
       * killough 10/98: prevent zombies from exiting levels
       */
      if (!bossaction && thing->player && thing->player->health <= 0 && !comp[comp_zombie])
      {
        S_StartMobjSound(thing, sfx_noway);
        return false;
      }

      P_ChangeSwitchTexture(line,0);
      G_SecretExitLevel(0);
      break;

    case 55:
      // Raise Floor Crush
      if (EV_DoFloor(line,raiseFloorCrush))
        P_ChangeSwitchTexture(line,0);
      break;

    case 101:
      // Raise Floor
      if (EV_DoFloor(line,raiseFloor))
        P_ChangeSwitchTexture(line,0);
      break;

    case 102:
      // Lower Floor to Surrounding floor height
      if (EV_DoFloor(line,lowerFloor))
        P_ChangeSwitchTexture(line,0);
      break;

    case 103:
      // Open Door
      if (EV_DoDoor(line,openDoor))
        P_ChangeSwitchTexture(line,0);
      break;

    case 111:
      // Blazing Door Raise (faster than TURBO!)
      if (EV_DoDoor (line,blazeRaise))
        P_ChangeSwitchTexture(line,0);
      break;

    case 112:
      // Blazing Door Open (faster than TURBO!)
      if (EV_DoDoor (line,blazeOpen))
        P_ChangeSwitchTexture(line,0);
      break;

    case 113:
      // Blazing Door Close (faster than TURBO!)
      if (EV_DoDoor (line,blazeClose))
        P_ChangeSwitchTexture(line,0);
      break;

    case 122:
      // Blazing PlatDownWaitUpStay
      if (EV_DoPlat(line,blazeDWUS,0))
        P_ChangeSwitchTexture(line,0);
      break;

    case 127:
      // Build Stairs Turbo 16
      if (EV_BuildStairs(line,turbo16))
        P_ChangeSwitchTexture(line,0);
      break;

    case 131:
      // Raise Floor Turbo
      if (EV_DoFloor(line,raiseFloorTurbo))
        P_ChangeSwitchTexture(line,0);
      break;

    case 133:
      // BlzOpenDoor BLUE
    case 135:
      // BlzOpenDoor RED
    case 137:
      // BlzOpenDoor YELLOW
      if (EV_DoLockedDoor (line,blazeOpen,thing))
        P_ChangeSwitchTexture(line,0);
      break;

    case 140:
      // Raise Floor 512
      if (EV_DoFloor(line,raiseFloor512))
        P_ChangeSwitchTexture(line,0);
      break;

      // killough 1/31/98: factored out compatibility check;
      // added inner switch, relaxed check to demo_compatibility

    default:
      if (!demo_compatibility)
        switch (line->special)
        {
          //jff 1/29/98 added linedef types to fill all functions out so that
          // all possess SR, S1, WR, W1 types

          case 158:
            // Raise Floor to shortest lower texture
            // 158 S1  EV_DoFloor(raiseToTexture), CSW(0)
            if (EV_DoFloor(line,raiseToTexture))
              P_ChangeSwitchTexture(line,0);
            break;

          case 159:
            // Raise Floor to shortest lower texture
            // 159 S1  EV_DoFloor(lowerAndChange)
            if (EV_DoFloor(line,lowerAndChange))
              P_ChangeSwitchTexture(line,0);
            break;

          case 160:
            // Raise Floor 24 and change
            // 160 S1  EV_DoFloor(raiseFloor24AndChange)
            if (EV_DoFloor(line,raiseFloor24AndChange))
              P_ChangeSwitchTexture(line,0);
            break;

          case 161:
            // Raise Floor 24
            // 161 S1  EV_DoFloor(raiseFloor24)
            if (EV_DoFloor(line,raiseFloor24))
              P_ChangeSwitchTexture(line,0);
            break;

          case 162:
            // Moving floor min n to max n
            // 162 S1  EV_DoPlat(perpetualRaise,0)
            if (EV_DoPlat(line,perpetualRaise,0))
              P_ChangeSwitchTexture(line,0);
            break;

          case 163:
            // Stop Moving floor
            // 163 S1  EV_DoPlat(perpetualRaise,0)
            EV_StopPlat(line);
            P_ChangeSwitchTexture(line,0);
            break;

          case 164:
            // Start fast crusher
            // 164 S1  EV_DoCeiling(fastCrushAndRaise)
            if (EV_DoCeiling(line,fastCrushAndRaise))
              P_ChangeSwitchTexture(line,0);
            break;

          case 165:
            // Start slow silent crusher
            // 165 S1  EV_DoCeiling(silentCrushAndRaise)
            if (EV_DoCeiling(line,silentCrushAndRaise))
              P_ChangeSwitchTexture(line,0);
            break;

          case 166:
            // Raise ceiling, Lower floor
            // 166 S1 EV_DoCeiling(raiseToHighest), EV_DoFloor(lowerFloortoLowest)
            if (EV_DoCeiling(line, raiseToHighest) ||
                EV_DoFloor(line, lowerFloorToLowest))
              P_ChangeSwitchTexture(line,0);
            break;

          case 167:
            // Lower floor and Crush
            // 167 S1 EV_DoCeiling(lowerAndCrush)
            if (EV_DoCeiling(line, lowerAndCrush))
              P_ChangeSwitchTexture(line,0);
            break;

          case 168:
            // Stop crusher
            // 168 S1 EV_CeilingCrushStop()
            if (EV_CeilingCrushStop(line))
              P_ChangeSwitchTexture(line,0);
            break;

          case 169:
            // Lights to brightest neighbor sector
            // 169 S1  EV_LightTurnOn(0)
            EV_LightTurnOn(line,0);
            P_ChangeSwitchTexture(line,0);
            break;

          case 170:
            // Lights to near dark
            // 170 S1  EV_LightTurnOn(35)
            EV_LightTurnOn(line,35);
            P_ChangeSwitchTexture(line,0);
            break;

          case 171:
            // Lights on full
            // 171 S1  EV_LightTurnOn(255)
            EV_LightTurnOn(line,255);
            P_ChangeSwitchTexture(line,0);
            break;

          case 172:
            // Start Lights Strobing
            // 172 S1  EV_StartLightStrobing()
            EV_StartLightStrobing(line);
            P_ChangeSwitchTexture(line,0);
            break;

          case 173:
            // Lights to Dimmest Near
            // 173 S1  EV_TurnTagLightsOff()
            EV_TurnTagLightsOff(line);
            P_ChangeSwitchTexture(line,0);
            break;

          case 174:
            // Teleport
            // 174 S1  Teleport(side,thing)
            if (map_format.ev_teleport(0, line->tag,line,side,thing,TELF_VANILLA))
              P_ChangeSwitchTexture(line,0);
            break;

          case 175:
            // Close Door, Open in 30 secs
            // 175 S1  EV_DoDoor(close30ThenOpen)
            if (EV_DoDoor(line,close30ThenOpen))
              P_ChangeSwitchTexture(line,0);
            break;

          case 189: //jff 3/15/98 create texture change no motion type
            // Texture Change Only (Trigger)
            // 189 S1 Change Texture/Type Only
            if (EV_DoChange(line,trigChangeOnly,line->tag))
              P_ChangeSwitchTexture(line,0);
            break;

          case 203:
            // Lower ceiling to lowest surrounding ceiling
            // 203 S1 EV_DoCeiling(lowerToLowest)
            if (EV_DoCeiling(line,lowerToLowest))
              P_ChangeSwitchTexture(line,0);
            break;

          case 204:
            // Lower ceiling to highest surrounding floor
            // 204 S1 EV_DoCeiling(lowerToMaxFloor)
            if (EV_DoCeiling(line,lowerToMaxFloor))
              P_ChangeSwitchTexture(line,0);
            break;

          case 209:
            // killough 1/31/98: silent teleporter
            //jff 209 S1 SilentTeleport
            if (map_format.ev_teleport(0, line->tag, line, side, thing, TELF_SILENT))
              P_ChangeSwitchTexture(line,0);
            break;

          case 241: //jff 3/15/98 create texture change no motion type
            // Texture Change Only (Numeric)
            // 241 S1 Change Texture/Type Only
            if (EV_DoChange(line,numChangeOnly,line->tag))
              P_ChangeSwitchTexture(line,0);
            break;

          case 221:
            // Lower floor to next lowest floor
            // 221 S1 Lower Floor To Nearest Floor
            if (EV_DoFloor(line,lowerFloorToNearest))
              P_ChangeSwitchTexture(line,0);
            break;

          case 229:
            // Raise elevator next floor
            // 229 S1 Raise Elevator next floor
            if (EV_DoElevator(line,elevateUp))
              P_ChangeSwitchTexture(line,0);
            break;

          case 233:
            // Lower elevator next floor
            // 233 S1 Lower Elevator next floor
            if (EV_DoElevator(line,elevateDown))
              P_ChangeSwitchTexture(line,0);
            break;

          case 237:
            // Elevator to current floor
            // 237 S1 Elevator to current floor
            if (EV_DoElevator(line,elevateCurrent))
              P_ChangeSwitchTexture(line,0);
            break;


          // jff 1/29/98 end of added S1 linedef types

          //jff 1/29/98 added linedef types to fill all functions out so that
          // all possess SR, S1, WR, W1 types

          case 78: //jff 3/15/98 create texture change no motion type
            // Texture Change Only (Numeric)
            // 78 SR Change Texture/Type Only
            if (EV_DoChange(line,numChangeOnly,line->tag))
              P_ChangeSwitchTexture(line,1);
            break;

          case 176:
            // Raise Floor to shortest lower texture
            // 176 SR  EV_DoFloor(raiseToTexture), CSW(1)
            if (EV_DoFloor(line,raiseToTexture))
              P_ChangeSwitchTexture(line,1);
            break;

          case 177:
            // Raise Floor to shortest lower texture
            // 177 SR  EV_DoFloor(lowerAndChange)
            if (EV_DoFloor(line,lowerAndChange))
              P_ChangeSwitchTexture(line,1);
            break;

          case 178:
            // Raise Floor 512
            // 178 SR  EV_DoFloor(raiseFloor512)
            if (EV_DoFloor(line,raiseFloor512))
              P_ChangeSwitchTexture(line,1);
            break;

          case 179:
            // Raise Floor 24 and change
            // 179 SR  EV_DoFloor(raiseFloor24AndChange)
            if (EV_DoFloor(line,raiseFloor24AndChange))
              P_ChangeSwitchTexture(line,1);
            break;

          case 180:
            // Raise Floor 24
            // 180 SR  EV_DoFloor(raiseFloor24)
            if (EV_DoFloor(line,raiseFloor24))
              P_ChangeSwitchTexture(line,1);
            break;

          case 181:
            // Moving floor min n to max n
            // 181 SR  EV_DoPlat(perpetualRaise,0)

            EV_DoPlat(line,perpetualRaise,0);
            P_ChangeSwitchTexture(line,1);
            break;

          case 182:
            // Stop Moving floor
            // 182 SR  EV_DoPlat(perpetualRaise,0)
            EV_StopPlat(line);
            P_ChangeSwitchTexture(line,1);
            break;

          case 183:
            // Start fast crusher
            // 183 SR  EV_DoCeiling(fastCrushAndRaise)
            if (EV_DoCeiling(line,fastCrushAndRaise))
              P_ChangeSwitchTexture(line,1);
            break;

          case 184:
            // Start slow crusher
            // 184 SR  EV_DoCeiling(crushAndRaise)
            if (EV_DoCeiling(line,crushAndRaise))
              P_ChangeSwitchTexture(line,1);
            break;

          case 185:
            // Start slow silent crusher
            // 185 SR  EV_DoCeiling(silentCrushAndRaise)
            if (EV_DoCeiling(line,silentCrushAndRaise))
              P_ChangeSwitchTexture(line,1);
            break;

          case 186:
            // Raise ceiling, Lower floor
            // 186 SR EV_DoCeiling(raiseToHighest), EV_DoFloor(lowerFloortoLowest)
            if (EV_DoCeiling(line, raiseToHighest) ||
                EV_DoFloor(line, lowerFloorToLowest))
              P_ChangeSwitchTexture(line,1);
            break;

          case 187:
            // Lower floor and Crush
            // 187 SR EV_DoCeiling(lowerAndCrush)
            if (EV_DoCeiling(line, lowerAndCrush))
              P_ChangeSwitchTexture(line,1);
            break;

          case 188:
            // Stop crusher
            // 188 SR EV_CeilingCrushStop()
            if (EV_CeilingCrushStop(line))
              P_ChangeSwitchTexture(line,1);
            break;

          case 190: //jff 3/15/98 create texture change no motion type
            // Texture Change Only (Trigger)
            // 190 SR Change Texture/Type Only
            if (EV_DoChange(line,trigChangeOnly,line->tag))
              P_ChangeSwitchTexture(line,1);
            break;

          case 191:
            // Lower Pillar, Raise Donut
            // 191 SR  EV_DoDonut()
            if (EV_DoDonut(line))
              P_ChangeSwitchTexture(line,1);
            break;

          case 192:
            // Lights to brightest neighbor sector
            // 192 SR  EV_LightTurnOn(0)
            EV_LightTurnOn(line,0);
            P_ChangeSwitchTexture(line,1);
            break;

          case 193:
            // Start Lights Strobing
            // 193 SR  EV_StartLightStrobing()
            EV_StartLightStrobing(line);
            P_ChangeSwitchTexture(line,1);
            break;

          case 194:
            // Lights to Dimmest Near
            // 194 SR  EV_TurnTagLightsOff()
            EV_TurnTagLightsOff(line);
            P_ChangeSwitchTexture(line,1);
            break;

          case 195:
            // Teleport
            // 195 SR  Teleport(side,thing)
            if (map_format.ev_teleport(0, line->tag,line,side,thing,TELF_VANILLA))
              P_ChangeSwitchTexture(line,1);
            break;

          case 196:
            // Close Door, Open in 30 secs
            // 196 SR  EV_DoDoor(close30ThenOpen)
            if (EV_DoDoor(line,close30ThenOpen))
              P_ChangeSwitchTexture(line,1);
            break;

          case 205:
            // Lower ceiling to lowest surrounding ceiling
            // 205 SR EV_DoCeiling(lowerToLowest)
            if (EV_DoCeiling(line,lowerToLowest))
              P_ChangeSwitchTexture(line,1);
            break;

          case 206:
            // Lower ceiling to highest surrounding floor
            // 206 SR EV_DoCeiling(lowerToMaxFloor)
            if (EV_DoCeiling(line,lowerToMaxFloor))
              P_ChangeSwitchTexture(line,1);
            break;

          case 210:
            // killough 1/31/98: silent teleporter
            //jff 210 SR SilentTeleport
            if (map_format.ev_teleport(0, line->tag, line, side, thing, TELF_SILENT))
              P_ChangeSwitchTexture(line,1);
            break;

          case 211: //jff 3/14/98 create instant toggle floor type
            // Toggle Floor Between C and F Instantly
            // 211 SR Toggle Floor Instant
            if (EV_DoPlat(line,toggleUpDn,0))
              P_ChangeSwitchTexture(line,1);
            break;

          case 222:
            // Lower floor to next lowest floor
            // 222 SR Lower Floor To Nearest Floor
            if (EV_DoFloor(line,lowerFloorToNearest))
              P_ChangeSwitchTexture(line,1);
            break;

          case 230:
            // Raise elevator next floor
            // 230 SR Raise Elevator next floor
            if (EV_DoElevator(line,elevateUp))
              P_ChangeSwitchTexture(line,1);
            break;

          case 234:
            // Lower elevator next floor
            // 234 SR Lower Elevator next floor
            if (EV_DoElevator(line,elevateDown))
              P_ChangeSwitchTexture(line,1);
            break;

          case 238:
            // Elevator to current floor
            // 238 SR Elevator to current floor
            if (EV_DoElevator(line,elevateCurrent))
              P_ChangeSwitchTexture(line,1);
            break;

          case 258:
            // Build stairs, step 8
            // 258 SR EV_BuildStairs(build8)
            if (EV_BuildStairs(line,build8))
              P_ChangeSwitchTexture(line,1);
            break;

          case 259:
            // Build stairs, step 16
            // 259 SR EV_BuildStairs(turbo16)
            if (EV_BuildStairs(line,turbo16))
              P_ChangeSwitchTexture(line,1);
            break;

          // 1/29/98 jff end of added SR linedef types

        }
      break;

    // Buttons (retriggerable switches)
    case 42:
      // Close Door
      if (EV_DoDoor(line,closeDoor))
        P_ChangeSwitchTexture(line,1);
      break;

    case 43:
      // Lower Ceiling to Floor
      if (EV_DoCeiling(line,lowerToFloor))
        P_ChangeSwitchTexture(line,1);
      break;

    case 45:
      // Lower Floor to Surrounding floor height
      if (EV_DoFloor(line,lowerFloor))
        P_ChangeSwitchTexture(line,1);
      break;

    case 60:
      // Lower Floor to Lowest
      if (EV_DoFloor(line,lowerFloorToLowest))
        P_ChangeSwitchTexture(line,1);
      break;

    case 61:
      // Open Door
      if (EV_DoDoor(line,openDoor))
        P_ChangeSwitchTexture(line,1);
      break;

    case 62:
      // PlatDownWaitUpStay
      if (EV_DoPlat(line,downWaitUpStay,1))
        P_ChangeSwitchTexture(line,1);
      break;

    case 63:
      // Raise Door
      if (EV_DoDoor(line,normal))
        P_ChangeSwitchTexture(line,1);
      break;

    case 64:
      // Raise Floor to ceiling
      if (EV_DoFloor(line,raiseFloor))
        P_ChangeSwitchTexture(line,1);
      break;

    case 66:
      // Raise Floor 24 and change texture
      if (EV_DoPlat(line,raiseAndChange,24))
        P_ChangeSwitchTexture(line,1);
      break;

    case 67:
      // Raise Floor 32 and change texture
      if (EV_DoPlat(line,raiseAndChange,32))
        P_ChangeSwitchTexture(line,1);
      break;

    case 65:
      // Raise Floor Crush
      if (EV_DoFloor(line,raiseFloorCrush))
        P_ChangeSwitchTexture(line,1);
      break;

    case 68:
      // Raise Plat to next highest floor and change texture
      if (EV_DoPlat(line,raiseToNearestAndChange,0))
        P_ChangeSwitchTexture(line,1);
      break;

    case 69:
      // Raise Floor to next highest floor
      if (EV_DoFloor(line, raiseFloorToNearest))
        P_ChangeSwitchTexture(line,1);
      break;

    case 70:
      // Turbo Lower Floor
      if (EV_DoFloor(line,turboLower))
        P_ChangeSwitchTexture(line,1);
      break;

    case 114:
      // Blazing Door Raise (faster than TURBO!)
      if (EV_DoDoor (line,blazeRaise))
        P_ChangeSwitchTexture(line,1);
      break;

    case 115:
      // Blazing Door Open (faster than TURBO!)
      if (EV_DoDoor (line,blazeOpen))
        P_ChangeSwitchTexture(line,1);
      break;

    case 116:
      // Blazing Door Close (faster than TURBO!)
      if (EV_DoDoor (line,blazeClose))
        P_ChangeSwitchTexture(line,1);
      break;

    case 123:
      // Blazing PlatDownWaitUpStay
      if (EV_DoPlat(line,blazeDWUS,0))
        P_ChangeSwitchTexture(line,1);
      break;

    case 132:
      // Raise Floor Turbo
      if (EV_DoFloor(line,raiseFloorTurbo))
        P_ChangeSwitchTexture(line,1);
      break;

    case 99:
      // BlzOpenDoor BLUE
    case 134:
      // BlzOpenDoor RED
    case 136:
      // BlzOpenDoor YELLOW
      if (EV_DoLockedDoor (line,blazeOpen,thing))
        P_ChangeSwitchTexture(line,1);
      break;

    case 138:
      // Light Turn On
      EV_LightTurnOn(line,255);
      P_ChangeSwitchTexture(line,1);
      break;

    case 139:
      // Light Turn Off
      EV_LightTurnOn(line,35);
      P_ChangeSwitchTexture(line,1);
      break;
  }
  return true;
}

// heretic

dboolean Heretic_P_UseSpecialLine(mobj_t * thing, line_t * line, int side, dboolean bossaction)
{
    // This condition never reached in heretic
    if (side || bossaction) return false;

    //
    //      Switches that other things can activate
    //
    if (!thing->player)
    {
        if (line->flags & ML_SECRET)
            return false;       // never open secret doors
        switch (line->special)
        {
            case 1:            // MANUAL DOOR RAISE
            case 32:           // MANUAL BLUE
            case 33:           // MANUAL RED
            case 34:           // MANUAL YELLOW
                break;
            default:
                return false;
        }
    }

    //
    // do something
    //
    switch (line->special)
    {
            //===============================================
            //      MANUALS
            //===============================================
        case 1:                // Vertical Door
        case 26:               // Blue Door/Locked
        case 27:               // Yellow Door /Locked
        case 28:               // Red Door /Locked

        case 31:               // Manual door open
        case 32:               // Blue locked door open
        case 33:               // Red locked door open
        case 34:               // Yellow locked door open
            EV_VerticalDoor(line, thing);
            break;
            //===============================================
            //      SWITCHES
            //===============================================
        case 7:                // Switch_Build_Stairs (8 pixel steps)
            if (EV_BuildStairs(line, heretic_build8))
            {
                P_ChangeSwitchTexture(line, 0);
            }
            break;
        case 107:              // Switch_Build_Stairs_16 (16 pixel steps)
            if (EV_BuildStairs(line, heretic_turbo16))
            {
                P_ChangeSwitchTexture(line, 0);
            }
            break;
        case 9:                // Change Donut
            if (EV_DoDonut(line))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 11:               // Exit level
            G_ExitLevel(0);
            P_ChangeSwitchTexture(line, 0);
            break;
        case 14:               // Raise Floor 32 and change texture
            if (EV_DoPlat(line, raiseAndChange, 32))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 15:               // Raise Floor 24 and change texture
            if (EV_DoPlat(line, raiseAndChange, 24))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 18:               // Raise Floor to next highest floor
            if (EV_DoFloor(line, raiseFloorToNearest))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 20:               // Raise Plat next highest floor and change texture
            if (EV_DoPlat(line, raiseToNearestAndChange, 0))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 21:               // PlatDownWaitUpStay
            if (EV_DoPlat(line, downWaitUpStay, 0))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 23:               // Lower Floor to Lowest
            if (EV_DoFloor(line, lowerFloorToLowest))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 29:               // Raise Door
            if (EV_DoDoor(line, vld_normal))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 41:               // Lower Ceiling to Floor
            if (EV_DoCeiling(line, lowerToFloor))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 71:               // Turbo Lower Floor
            if (EV_DoFloor(line, turboLower))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 49:               // Lower Ceiling And Crush
            if (EV_DoCeiling(line, lowerAndCrush))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 50:               // Close Door
            if (EV_DoDoor(line, vld_close))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 51:               // Secret EXIT
            G_SecretExitLevel(0);
            P_ChangeSwitchTexture(line, 0);
            break;
        case 55:               // Raise Floor Crush
            if (EV_DoFloor(line, raiseFloorCrush))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 101:              // Raise Floor
            if (EV_DoFloor(line, raiseFloor))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 102:              // Lower Floor to Surrounding floor height
            if (EV_DoFloor(line, lowerFloor))
                P_ChangeSwitchTexture(line, 0);
            break;
        case 103:              // Open Door
            if (EV_DoDoor(line, vld_open))
                P_ChangeSwitchTexture(line, 0);
            break;
            //===============================================
            //      BUTTONS
            //===============================================
        case 42:               // Close Door
            if (EV_DoDoor(line, vld_close))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 43:               // Lower Ceiling to Floor
            if (EV_DoCeiling(line, lowerToFloor))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 45:               // Lower Floor to Surrounding floor height
            if (EV_DoFloor(line, lowerFloor))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 60:               // Lower Floor to Lowest
            if (EV_DoFloor(line, lowerFloorToLowest))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 61:               // Open Door
            if (EV_DoDoor(line, vld_open))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 62:               // PlatDownWaitUpStay
            if (EV_DoPlat(line, downWaitUpStay, 1))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 63:               // Raise Door
            if (EV_DoDoor(line, vld_normal))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 64:               // Raise Floor to ceiling
            if (EV_DoFloor(line, raiseFloor))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 66:               // Raise Floor 24 and change texture
            if (EV_DoPlat(line, raiseAndChange, 24))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 67:               // Raise Floor 32 and change texture
            if (EV_DoPlat(line, raiseAndChange, 32))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 65:               // Raise Floor Crush
            if (EV_DoFloor(line, raiseFloorCrush))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 68:               // Raise Plat to next highest floor and change texture
            if (EV_DoPlat(line, raiseToNearestAndChange, 0))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 69:               // Raise Floor to next highest floor
            if (EV_DoFloor(line, raiseFloorToNearest))
                P_ChangeSwitchTexture(line, 1);
            break;
        case 70:               // Turbo Lower Floor
            if (EV_DoFloor(line, turboLower))
                P_ChangeSwitchTexture(line, 1);
            break;
    }

    return true;
}
