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
 *  Items: key cards, artifacts, weapon, ammunition.
 *
 *-----------------------------------------------------------------------------*/


#ifndef __D_ITEMS__
#define __D_ITEMS__

#include "doomdef.h"

//
// Internal weapon flags
//
#define WIF_ENABLEAPS 0x00000001 // [XA] enable "ammo per shot" field for native Doom weapon codepointers

// haleyjd 09/11/07: weapon flags
//
#define WPF_NOFLAG         0x00000000 // no flag
#define WPF_NOTHRUST       0x00000001 // doesn't thrust Mobj's
#define WPF_SILENT         0x00000002 // weapon is silent
#define WPF_NOAUTOFIRE     0x00000004 // weapon won't autofire in A_WeaponReady
#define WPF_FLEEMELEE      0x00000008 // monsters consider it a melee weapon
#define WPF_AUTOSWITCHFROM 0x00000010 // can be switched away from when ammo is picked up
#define WPF_NOAUTOSWITCHTO   0x00000020 // cannot be switched to when ammo is picked up

/* Weapon info: sprite frames, ammunition use. */
typedef struct
{
  ammotype_t  ammo;
  int         upstate;
  int         downstate;
  int         readystate;
  int         atkstate;
  int         holdatkstate;
  int         flashstate;
  int         ammopershot;
  int         intflags;
  int         flags;
} weaponinfo_t;

extern weaponinfo_t doom_weaponinfo[NUMWEAPONS+2];

// heretic

extern weaponinfo_t wpnlev1info[NUMWEAPONS];
extern weaponinfo_t wpnlev2info[NUMWEAPONS];

// hexen

extern weaponinfo_t hexen_weaponinfo[HEXEN_NUMWEAPONS][NUMCLASSES];

// dynamically selected in global.c

extern weaponinfo_t* weaponinfo;

#endif
