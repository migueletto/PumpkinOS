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
 *  Something to do with weapon sprite frames. Don't ask me.
 *
 *-----------------------------------------------------------------------------
 */

// We are referring to sprite numbers.
#include "doomtype.h"
#include "info.h"

#include "d_items.h"


//
// PSPRITE ACTIONS for waepons.
// This struct controls the weapon animations.
//
// Each entry is:
//  ammo/amunition type
//  upstate
//  downstate
//  readystate
//  atkstate, i.e. attack/fire/hit frame
//  flashstate, muzzle flash
//
weaponinfo_t doom_weaponinfo[NUMWEAPONS+2] =
{
  {
    // fist
    am_noammo,
    S_PUNCHUP,
    S_PUNCHDOWN,
    S_PUNCH,
    S_PUNCH1,
    S_NULL,
    S_NULL,
    1,
    0,
    WPF_FLEEMELEE | WPF_AUTOSWITCHFROM | WPF_NOAUTOSWITCHTO
  },
  {
    // pistol
    am_clip,
    S_PISTOLUP,
    S_PISTOLDOWN,
    S_PISTOL,
    S_PISTOL1,
    S_NULL,
    S_PISTOLFLASH,
    1,
    0,
    WPF_AUTOSWITCHFROM
  },
  {
    // shotgun
    am_shell,
    S_SGUNUP,
    S_SGUNDOWN,
    S_SGUN,
    S_SGUN1,
    S_NULL,
    S_SGUNFLASH1,
    1,
    0,
    WPF_NOFLAG
  },
  {
    // chaingun
    am_clip,
    S_CHAINUP,
    S_CHAINDOWN,
    S_CHAIN,
    S_CHAIN1,
    S_NULL,
    S_CHAINFLASH1,
    1,
    0,
    WPF_NOFLAG
  },
  {
    // missile launcher
    am_misl,
    S_MISSILEUP,
    S_MISSILEDOWN,
    S_MISSILE,
    S_MISSILE1,
    S_NULL,
    S_MISSILEFLASH1,
    1,
    0,
    WPF_NOAUTOFIRE
  },
  {
    // plasma rifle
    am_cell,
    S_PLASMAUP,
    S_PLASMADOWN,
    S_PLASMA,
    S_PLASMA1,
    S_NULL,
    S_PLASMAFLASH1,
    1,
    0,
    WPF_NOFLAG
  },
  {
    // bfg 9000
    am_cell,
    S_BFGUP,
    S_BFGDOWN,
    S_BFG,
    S_BFG1,
    S_NULL,
    S_BFGFLASH1,
    40,
    0,
    WPF_NOAUTOFIRE
  },
  {
    // chainsaw
    am_noammo,
    S_SAWUP,
    S_SAWDOWN,
    S_SAW,
    S_SAW1,
    S_NULL,
    S_NULL,
    1,
    0,
    WPF_NOTHRUST | WPF_FLEEMELEE | WPF_NOAUTOSWITCHTO
  },
  {
    // super shotgun
    am_shell,
    S_DSGUNUP,
    S_DSGUNDOWN,
    S_DSGUN,
    S_DSGUN1,
    S_NULL,
    S_DSGUNFLASH1,
    2,
    0,
    WPF_NOFLAG
  },

  // dseg03:00082D90                 weaponinfo_t <5, 46h, 45h, 43h, 47h, 0>
  // dseg03:00082D90                 weaponinfo_t <1, 22h, 21h, 20h, 23h, 2Fh>
  // dseg03:00082E68 animdefs        dd 0                    ; istexture
  // dseg03:00082E68                 db 'N', 'U', 'K', 'A', 'G', 'E', '3', 2 dup(0); endname
  // dseg03:00082E68                 db 'N', 'U', 'K', 'A', 'G', 'E', '1', 2 dup(0); startname
  // dseg03:00082E68                 dd 8                    ; speed
  // dseg03:00082E68                 dd 0                    ; istexture
  {
    // ololo weapon
    0,
    S_NULL, // states are not used for emulation of weaponinfo overrun
    S_NULL,
    S_NULL,
    S_NULL,
    S_NULL,
    S_NULL,
    0,
    0,
    WPF_NOFLAG
  },
  {
    // preved medved weapon
    0,
    S_NULL,
    S_NULL,
    S_NULL,
    S_NULL,
    S_NULL,
    S_NULL,
    0,
    0,
    WPF_NOFLAG
  },
};

// heretic

#include "heretic/def.h"

weaponinfo_t wpnlev1info[NUMWEAPONS] = {
  {                           // Staff
    am_noammo,                 // ammo
    HERETIC_S_STAFFUP,                 // upstate
    HERETIC_S_STAFFDOWN,               // downstate
    HERETIC_S_STAFFREADY,              // readystate
    HERETIC_S_STAFFATK1_1,             // atkstate
    HERETIC_S_STAFFATK1_1,             // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    0,                                 // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Gold wand
    am_goldwand,               // ammo
    HERETIC_S_GOLDWANDUP,              // upstate
    HERETIC_S_GOLDWANDDOWN,            // downstate
    HERETIC_S_GOLDWANDREADY,           // readystate
    HERETIC_S_GOLDWANDATK1_1,          // atkstate
    HERETIC_S_GOLDWANDATK1_1,          // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_GWND_AMMO_1,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Crossbow
    am_crossbow,               // ammo
    HERETIC_S_CRBOWUP,                 // upstate
    HERETIC_S_CRBOWDOWN,               // downstate
    HERETIC_S_CRBOW1,                  // readystate
    HERETIC_S_CRBOWATK1_1,             // atkstate
    HERETIC_S_CRBOWATK1_1,             // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_CBOW_AMMO_1,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Blaster
    am_blaster,                // ammo
    HERETIC_S_BLASTERUP,               // upstate
    HERETIC_S_BLASTERDOWN,             // downstate
    HERETIC_S_BLASTERREADY,            // readystate
    HERETIC_S_BLASTERATK1_1,           // atkstate
    HERETIC_S_BLASTERATK1_3,           // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_BLSR_AMMO_1,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Skull rod
    am_skullrod,               // ammo
    HERETIC_S_HORNRODUP,               // upstate
    HERETIC_S_HORNRODDOWN,             // downstate
    HERETIC_S_HORNRODREADY,            // readystae
    HERETIC_S_HORNRODATK1_1,           // atkstate
    HERETIC_S_HORNRODATK1_1,           // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_SKRD_AMMO_1,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Phoenix rod
    am_phoenixrod,             // ammo
    HERETIC_S_PHOENIXUP,               // upstate
    HERETIC_S_PHOENIXDOWN,             // downstate
    HERETIC_S_PHOENIXREADY,            // readystate
    HERETIC_S_PHOENIXATK1_1,           // atkstate
    HERETIC_S_PHOENIXATK1_1,           // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_PHRD_AMMO_1,                   // ammopershot
    0,                                 // intflags
    WPF_NOAUTOFIRE
  },
  {                           // Mace
    am_mace,                   // ammo
    HERETIC_S_MACEUP,                  // upstate
    HERETIC_S_MACEDOWN,                // downstate
    HERETIC_S_MACEREADY,               // readystate
    HERETIC_S_MACEATK1_1,              // atkstate
    HERETIC_S_MACEATK1_2,              // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_MACE_AMMO_1,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Gauntlets
    am_noammo,                 // ammo
    HERETIC_S_GAUNTLETUP,              // upstate
    HERETIC_S_GAUNTLETDOWN,            // downstate
    HERETIC_S_GAUNTLETREADY,           // readystate
    HERETIC_S_GAUNTLETATK1_1,          // atkstate
    HERETIC_S_GAUNTLETATK1_3,          // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    0,                                 // ammopershot
    0,                                 // intflags
    WPF_NOTHRUST
  },
  {                           // Beak
    am_noammo,                 // ammo
    HERETIC_S_BEAKUP,                  // upstate
    HERETIC_S_BEAKDOWN,                // downstate
    HERETIC_S_BEAKREADY,               // readystate
    HERETIC_S_BEAKATK1_1,              // atkstate
    HERETIC_S_BEAKATK1_1,              // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    0,                                 // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  }
};

weaponinfo_t wpnlev2info[NUMWEAPONS] = {
  {                           // Staff
    am_noammo,                 // ammo
    HERETIC_S_STAFFUP2,                // upstate
    HERETIC_S_STAFFDOWN2,              // downstate
    HERETIC_S_STAFFREADY2_1,           // readystate
    HERETIC_S_STAFFATK2_1,             // atkstate
    HERETIC_S_STAFFATK2_1,             // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    0,                                 // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Gold wand
    am_goldwand,               // ammo
    HERETIC_S_GOLDWANDUP,              // upstate
    HERETIC_S_GOLDWANDDOWN,            // downstate
    HERETIC_S_GOLDWANDREADY,           // readystate
    HERETIC_S_GOLDWANDATK2_1,          // atkstate
    HERETIC_S_GOLDWANDATK2_1,          // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_GWND_AMMO_2,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Crossbow
    am_crossbow,               // ammo
    HERETIC_S_CRBOWUP,                 // upstate
    HERETIC_S_CRBOWDOWN,               // downstate
    HERETIC_S_CRBOW1,                  // readystate
    HERETIC_S_CRBOWATK2_1,             // atkstate
    HERETIC_S_CRBOWATK2_1,             // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_CBOW_AMMO_2,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Blaster
    am_blaster,                // ammo
    HERETIC_S_BLASTERUP,               // upstate
    HERETIC_S_BLASTERDOWN,             // downstate
    HERETIC_S_BLASTERREADY,            // readystate
    HERETIC_S_BLASTERATK2_1,           // atkstate
    HERETIC_S_BLASTERATK2_3,           // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_BLSR_AMMO_2,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Skull rod
    am_skullrod,               // ammo
    HERETIC_S_HORNRODUP,               // upstate
    HERETIC_S_HORNRODDOWN,             // downstate
    HERETIC_S_HORNRODREADY,            // readystae
    HERETIC_S_HORNRODATK2_1,           // atkstate
    HERETIC_S_HORNRODATK2_1,           // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_SKRD_AMMO_2,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Phoenix rod
    am_phoenixrod,             // ammo
    HERETIC_S_PHOENIXUP,               // upstate
    HERETIC_S_PHOENIXDOWN,             // downstate
    HERETIC_S_PHOENIXREADY,            // readystate
    HERETIC_S_PHOENIXATK2_1,           // atkstate
    HERETIC_S_PHOENIXATK2_2,           // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_PHRD_AMMO_2,                   // ammopershot
    0,                                 // intflags
    WPF_NOAUTOFIRE
  },
  {                           // Mace
    am_mace,                   // ammo
    HERETIC_S_MACEUP,                  // upstate
    HERETIC_S_MACEDOWN,                // downstate
    HERETIC_S_MACEREADY,               // readystate
    HERETIC_S_MACEATK2_1,              // atkstate
    HERETIC_S_MACEATK2_1,              // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_MACE_AMMO_2,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Gauntlets
    am_noammo,                 // ammo
    HERETIC_S_GAUNTLETUP2,             // upstate
    HERETIC_S_GAUNTLETDOWN2,           // downstate
    HERETIC_S_GAUNTLETREADY2_1,        // readystate
    HERETIC_S_GAUNTLETATK2_1,          // atkstate
    HERETIC_S_GAUNTLETATK2_3,          // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    0,                                 // ammopershot
    0,                                 // intflags
    WPF_NOTHRUST
  },
  {                           // Beak
    am_noammo,                 // ammo
    HERETIC_S_BEAKUP,                  // upstate
    HERETIC_S_BEAKDOWN,                // downstate
    HERETIC_S_BEAKREADY,               // readystate
    HERETIC_S_BEAKATK2_1,              // atkstate
    HERETIC_S_BEAKATK2_1,              // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    0,                                 // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  }
};

// hexen

weaponinfo_t hexen_weaponinfo[HEXEN_NUMWEAPONS][NUMCLASSES] = {
  {                           // First Weapons
    [PCLASS_FIGHTER] = {       // Fighter First Weapon - Punch
      MANA_NONE,                // mana
      HEXEN_S_PUNCHUP,                // upstate
      HEXEN_S_PUNCHDOWN,              // downstate
      HEXEN_S_PUNCHREADY,             // readystate
      HEXEN_S_PUNCHATK1_1,            // atkstate
      HEXEN_S_PUNCHATK1_1,            // holdatkstate
      HEXEN_S_NULL,                   // flashstate
      0,                              // ammopershot
      0,                              // intflags
      WPF_NOFLAG
    },
    {                          // Cleric First Weapon - Mace
      MANA_NONE,                // mana
      HEXEN_S_CMACEUP,                // upstate
      HEXEN_S_CMACEDOWN,              // downstate
      HEXEN_S_CMACEREADY,             // readystate
      HEXEN_S_CMACEATK_1,             // atkstate
      HEXEN_S_CMACEATK_1,             // holdatkstate
      HEXEN_S_NULL,                   // flashstate
      0,                              // ammopershot
      0,                              // intflags
      WPF_NOFLAG
    },
    {                          // Mage First Weapon - Wand
      MANA_NONE,
      HEXEN_S_MWANDUP,                // upstate
      HEXEN_S_MWANDDOWN,              // downstate
      HEXEN_S_MWANDREADY,             // readystate
      HEXEN_S_MWANDATK_1,             // atkstate
      HEXEN_S_MWANDATK_1,             // holdatkstate
      HEXEN_S_NULL,                   // flashstate
      0,                              // ammopershot
      0,                              // intflags
      WPF_NOFLAG
    },
    {                          // Pig - Snout
      MANA_NONE,                // mana
      HEXEN_S_SNOUTUP,                // upstate
      HEXEN_S_SNOUTDOWN,              // downstate
      HEXEN_S_SNOUTREADY,             // readystate
      HEXEN_S_SNOUTATK1,              // atkstate
      HEXEN_S_SNOUTATK1,              // holdatkstate
      HEXEN_S_NULL,                   // flashstate
      0,                              // ammopershot
      0,                              // intflags
      WPF_NOFLAG
    }
  },
  {                           // Second Weapons
    [PCLASS_FIGHTER] = {       // Fighter - Axe
      MANA_NONE,                // mana
      HEXEN_S_FAXEUP,                 // upstate
      HEXEN_S_FAXEDOWN,               // downstate
      HEXEN_S_FAXEREADY,              // readystate
      HEXEN_S_FAXEATK_1,              // atkstate
      HEXEN_S_FAXEATK_1,              // holdatkstate
      HEXEN_S_NULL,                   // flashstate
      2,                              // ammopershot
      0,                              // intflags
      WPF_NOFLAG
    },
    {                          // Cleric - Serpent Staff
      MANA_1,                   // mana
      HEXEN_S_CSTAFFUP,               // upstate
      HEXEN_S_CSTAFFDOWN,             // downstate
      HEXEN_S_CSTAFFREADY,            // readystate
      HEXEN_S_CSTAFFATK_1,            // atkstate
      HEXEN_S_CSTAFFATK_1,            // holdatkstate
      HEXEN_S_NULL,                   // flashstate
      1,                              // ammopershot
      0,                              // intflags
      WPF_NOFLAG
    },
    {                          // Mage - Cone of shards
      MANA_1,                   // mana
      HEXEN_S_CONEUP,                 // upstate
      HEXEN_S_CONEDOWN,               // downstate
      HEXEN_S_CONEREADY,              // readystate
      HEXEN_S_CONEATK1_1,             // atkstate
      HEXEN_S_CONEATK1_3,             // holdatkstate
      HEXEN_S_NULL,                   // flashstate
      3,                              // ammopershot
      0,                              // intflags
      WPF_NOFLAG
    },
    {                          // Pig - Snout
      MANA_NONE,                // mana
      HEXEN_S_SNOUTUP,                // upstate
      HEXEN_S_SNOUTDOWN,              // downstate
      HEXEN_S_SNOUTREADY,             // readystate
      HEXEN_S_SNOUTATK1,              // atkstate
      HEXEN_S_SNOUTATK1,              // holdatkstate
      HEXEN_S_NULL,                   // flashstate
      0,                              // ammopershot
      0,                              // intflags
      WPF_NOFLAG
    }
  },
  {                           // Third Weapons
    [PCLASS_FIGHTER] = {       // Fighter - Hammer
      MANA_NONE,                // mana
      HEXEN_S_FHAMMERUP,              // upstate
      HEXEN_S_FHAMMERDOWN,            // downstate
      HEXEN_S_FHAMMERREADY,           // readystate
      HEXEN_S_FHAMMERATK_1,           // atkstate
      HEXEN_S_FHAMMERATK_1,           // holdatkstate
      HEXEN_S_NULL,                   // flashstate
      3,                              // ammopershot
      0,                              // intflags
      WPF_NOFLAG
    },
    {                          // Cleric - Flame Strike
      MANA_2,                   // mana
      HEXEN_S_CFLAMEUP,               // upstate
      HEXEN_S_CFLAMEDOWN,             // downstate
      HEXEN_S_CFLAMEREADY1,           // readystate
      HEXEN_S_CFLAMEATK_1,            // atkstate
      HEXEN_S_CFLAMEATK_1,            // holdatkstate
      HEXEN_S_NULL,                   // flashstate
      4,                              // ammopershot
      0,                              // intflags
      WPF_NOFLAG
    },
    {                          // Mage - Lightning
      MANA_2,                   // mana
      HEXEN_S_MLIGHTNINGUP,           // upstate
      HEXEN_S_MLIGHTNINGDOWN,         // downstate
      HEXEN_S_MLIGHTNINGREADY,        // readystate
      HEXEN_S_MLIGHTNINGATK_1,        // atkstate
      HEXEN_S_MLIGHTNINGATK_1,        // holdatkstate
      HEXEN_S_NULL,                   // flashstate
      5,                              // ammopershot
      0,                              // intflags
      WPF_NOFLAG
    },
    {                          // Pig - Snout
      MANA_NONE,                // mana
      HEXEN_S_SNOUTUP,                // upstate
      HEXEN_S_SNOUTDOWN,              // downstate
      HEXEN_S_SNOUTREADY,             // readystate
      HEXEN_S_SNOUTATK1,              // atkstate
      HEXEN_S_SNOUTATK1,              // holdatkstate
      HEXEN_S_NULL,                   // flashstate
      0,                              // ammopershot
      0,                              // intflags
      WPF_NOFLAG
    }
  },
  {                           // Fourth Weapons
    [PCLASS_FIGHTER] = {       // Fighter - Rune Sword
      MANA_BOTH,                // mana
      HEXEN_S_FSWORDUP,               // upstate
      HEXEN_S_FSWORDDOWN,             // downstate
      HEXEN_S_FSWORDREADY,            // readystate
      HEXEN_S_FSWORDATK_1,            // atkstate
      HEXEN_S_FSWORDATK_1,            // holdatkstate
      HEXEN_S_NULL,                   // flashstate
      14,                             // ammopershot
      0,                              // intflags
      WPF_NOFLAG
    },
    {                          // Cleric - Holy Symbol
      MANA_BOTH,                // mana
      HEXEN_S_CHOLYUP,                // upstate
      HEXEN_S_CHOLYDOWN,              // downstate
      HEXEN_S_CHOLYREADY,             // readystate
      HEXEN_S_CHOLYATK_1,             // atkstate
      HEXEN_S_CHOLYATK_1,             // holdatkstate
      HEXEN_S_NULL,                   // flashstate
      18,                             // ammopershot
      0,                              // intflags
      WPF_NOFLAG
    },
    {                          // Mage - Staff
      MANA_BOTH,                // mana
      HEXEN_S_MSTAFFUP,               // upstate
      HEXEN_S_MSTAFFDOWN,             // downstate
      HEXEN_S_MSTAFFREADY,            // readystate
      HEXEN_S_MSTAFFATK_1,            // atkstate
      HEXEN_S_MSTAFFATK_1,            // holdatkstate
      HEXEN_S_NULL,                   // flashstate
      15,                             // ammopershot
      0,                              // intflags
      WPF_NOFLAG
    },
    {                          // Pig - Snout
      MANA_NONE,                // mana
      HEXEN_S_SNOUTUP,                // upstate
      HEXEN_S_SNOUTDOWN,              // downstate
      HEXEN_S_SNOUTREADY,             // readystate
      HEXEN_S_SNOUTATK1,              // atkstate
      HEXEN_S_SNOUTATK1,              // holdatkstate
      HEXEN_S_NULL,                   // flashstate
      0,                              // ammopershot
      0,                              // intflags
      WPF_NOFLAG
    }
  }
};
