//
// Copyright(C) 2022 by Ryan Krafnick
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
// DESCRIPTION:
//	DSDA Spawn Numbers
//

#include "info.h"

#include "spawn_number.h"

#define SPAWN_NUMBER_MAX 155

int doom_spawn_numbers[SPAWN_NUMBER_MAX] = {
  [0] = MT_NULL,
  [1] = MT_SHOTGUY, // shotgun guy
  [2] = MT_CHAINGUY, // chaingun guy
  [3] = MT_BRUISER, // baron
  [4] = MT_POSSESSED, // zombieman
  [5] = MT_TROOP, // imp
  [6] = MT_BABY, // arachnotron
  [7] = MT_SPIDER, // spider mastermind
  [8] = MT_SERGEANT, // demon
  [9] = MT_SHADOWS, // spectre
  [10] = MT_TROOPSHOT, // imp fireball
  [11] = MT_CLIP, // ammo clip
  [12] = MT_MISC22, // shotgun shells
  MT_NULL, // unused 13
  MT_NULL, // unused 14
  MT_NULL, // unused 15
  MT_NULL, // unused 16
  MT_NULL, // unused 17
  MT_NULL, // unused 18
  [19] = MT_HEAD, // cacodemon
  [20] = MT_UNDEAD, // revenant
  [21] = MT_NULL, // bridge (x)
  [22] = MT_MISC3, // armor bonus
  [23] = MT_MISC10, // stimpack
  [24] = MT_MISC11, // medkit
  [25] = MT_MISC12, // soul sphere
  MT_NULL, // unused 26
  [27] = MT_SHOTGUN, // shotgun
  [28] = MT_CHAINGUN, // chaingun
  [29] = MT_MISC27, // rocket launcher
  [30] = MT_MISC28, // plasma rifle
  [31] = MT_MISC25, // BFG
  [32] = MT_MISC26, // chainsaw
  [33] = MT_SUPERSHOTGUN, // ssg
  MT_NULL, // unused 34
  MT_NULL, // unused 35
  MT_NULL, // unused 36
  MT_NULL, // unused 37
  MT_NULL, // unused 38
  MT_NULL, // unused 39
  MT_NULL, // unused 40
  MT_NULL, // unused 41
  MT_NULL, // unused 42
  MT_NULL, // unused 43
  MT_NULL, // unused 44
  MT_NULL, // unused 45
  MT_NULL, // unused 46
  MT_NULL, // unused 47
  MT_NULL, // unused 48
  MT_NULL, // unused 49
  MT_NULL, // unused 50
  [51] = MT_PLASMA, // plasma
  MT_NULL, // unused 52
  [53] = MT_TRACER, // rev rocket
  MT_NULL, // unused 54
  MT_NULL, // unused 55
  MT_NULL, // unused 56
  MT_NULL, // unused 57
  MT_NULL, // unused 58
  MT_NULL, // unused 59
  MT_NULL, // unused 60
  MT_NULL, // unused 61
  MT_NULL, // unused 62
  MT_NULL, // unused 63
  MT_NULL, // unused 64
  MT_NULL, // unused 65
  MT_NULL, // unused 66
  MT_NULL, // unused 67
  [68] = MT_MISC0, // green armor
  [69] = MT_MISC1, // blue armor
  MT_NULL, // unused 70
  MT_NULL, // unused 71
  MT_NULL, // unused 72
  MT_NULL, // unused 73
  MT_NULL, // unused 74
  [75] = MT_MISC20, // cell
  MT_NULL, // unused 76
  MT_NULL, // unused 77
  MT_NULL, // unused 78
  MT_NULL, // unused 79
  MT_NULL, // unused 80
  MT_NULL, // unused 81
  MT_NULL, // unused 82
  MT_NULL, // unused 83
  MT_NULL, // unused 84
  [85] = MT_MISC4, // blue keycard
  [86] = MT_MISC5, // red keycard
  [87] = MT_MISC6, // yellow keycard
  [88] = MT_MISC7, // yellow skull key
  [89] = MT_MISC8, // red skull key
  [90] = MT_MISC9, // blue skull key
  MT_NULL, // unused 91
  MT_NULL, // unused 92
  MT_NULL, // unused 93
  MT_NULL, // unused 94
  MT_NULL, // unused 95
  MT_NULL, // unused 96
  MT_NULL, // unused 97
  [98] = MT_FIRE, // archvile fire
  MT_NULL, // unused 99
  [100] = MT_NULL, // stealth baron (x)
  [101] = MT_NULL, // stealth hell knight (x)
  [102] = MT_NULL, // stealth zombieman (x)
  [103] = MT_NULL, // stealth shutgun guy (x)
  MT_NULL, // unused 104
  MT_NULL, // unused 105
  MT_NULL, // unused 106
  MT_NULL, // unused 107
  MT_NULL, // unused 108
  MT_NULL, // unused 109
  [110] = MT_SKULL, // lost soul
  [111] = MT_VILE, // archvile
  [112] = MT_FATSO, // mancubus
  [113] = MT_KNIGHT, // hell knight
  [114] = MT_CYBORG, // cyberdemon
  [115] = MT_PAIN, // pain elemental
  [116] = MT_WOLFSS, // wolf ss
  [117] = MT_NULL, // stealth arachnotron (x)
  [118] = MT_NULL, // stealth archvile (x)
  [119] = MT_NULL, // stealth cacodemon (x)
  [120] = MT_NULL, // stealth chaingun guy (x)
  [121] = MT_NULL, // stealth demon (x)
  [122] = MT_NULL, // stealth imp (x)
  [123] = MT_NULL, // stealth mancubus (x)
  [124] = MT_NULL, // stealth revenant (x)
  [125] = MT_BARREL, // barrel
  [126] = MT_HEADSHOT, // cacodemon fireball
  [127] = MT_ROCKET, // rocket
  [128] = MT_BFG, // bfg shot
  [129] = MT_ARACHPLAZ, // arachnotron shot
  MT_NULL, // unused 130
  [131] = MT_PUFF, // bullet puff
  [132] = MT_MEGA, // megasphere
  [133] = MT_INV, // invulnerability
  [134] = MT_MISC13, // berserk
  [135] = MT_INS, // partial invisibility
  [136] = MT_MISC14, // radiation suit
  [137] = MT_MISC15, // computer map
  [138] = MT_MISC16, // light amp goggles
  [139] = MT_MISC17, // ammo box
  [140] = MT_MISC18, // rocket (ammo)
  [141] = MT_MISC19, // box of rockets
  [142] = MT_MISC21, // cell pack
  [143] = MT_MISC23, // box of shells
  [144] = MT_MISC24, // backpack
  [145] = MT_MISC68, // guts
  [146] = MT_MISC71, // pool of blood
  [147] = MT_MISC84, // pool of blood 1
  [148] = MT_MISC85, // pool of blood 2
  [149] = MT_MISC77, // flaming barrel
  [150] = MT_MISC86, // brain
  [151] = MT_NULL, // scripted marine (x)
  [152] = MT_MISC2, // health bonus
  [153] = MT_FATSHOT, // mancubus shot
  [154] = MT_BRUISERSHOT, // baron fireball
};

int dsda_ThingTypeFromSpawnNumber(int spawn_number) {
  if (spawn_number < 0 || spawn_number >= SPAWN_NUMBER_MAX)
    return MT_NULL;

  return doom_spawn_numbers[spawn_number];
}
