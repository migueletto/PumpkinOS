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
 *      Created by a sound utility.
 *      Kept as a sample, DOOM2 sounds.
 *
 *-----------------------------------------------------------------------------*/

// killough 5/3/98: reformatted

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomtype.h"
#include "sounds.h"

//
// Information about all the music
//

musicinfo_t doom_S_music[] = {
  { 0 },
  { "e1m1", 0 },
  { "e1m2", 0 },
  { "e1m3", 0 },
  { "e1m4", 0 },
  { "e1m5", 0 },
  { "e1m6", 0 },
  { "e1m7", 0 },
  { "e1m8", 0 },
  { "e1m9", 0 },
  { "e2m1", 0 },
  { "e2m2", 0 },
  { "e2m3", 0 },
  { "e2m4", 0 },
  { "e2m5", 0 },
  { "e2m6", 0 },
  { "e2m7", 0 },
  { "e2m8", 0 },
  { "e2m9", 0 },
  { "e3m1", 0 },
  { "e3m2", 0 },
  { "e3m3", 0 },
  { "e3m4", 0 },
  { "e3m5", 0 },
  { "e3m6", 0 },
  { "e3m7", 0 },
  { "e3m8", 0 },
  { "e3m9", 0 },
  { "inter", 0 },
  { "intro", 0 },
  { "bunny", 0 },
  { "victor", 0 },
  { "introa", 0 },
  { "runnin", 0 },
  { "stalks", 0 },
  { "countd", 0 },
  { "betwee", 0 },
  { "doom", 0 },
  { "the_da", 0 },
  { "shawn", 0 },
  { "ddtblu", 0 },
  { "in_cit", 0 },
  { "dead", 0 },
  { "stlks2", 0 },
  { "theda2", 0 },
  { "doom2", 0 },
  { "ddtbl2", 0 },
  { "runni2", 0 },
  { "dead2", 0 },
  { "stlks3", 0 },
  { "romero", 0 },
  { "shawn2", 0 },
  { "messag", 0 },
  { "count2", 0 },
  { "ddtbl3", 0 },
  { "ampie", 0 },
  { "theda3", 0 },
  { "adrian", 0 },
  { "messg2", 0 },
  { "romer2", 0 },
  { "tense", 0 },
  { "shawn3", 0 },
  { "openin", 0 },
  { "evil", 0 },
  { "ultima", 0 },
  { "read_m", 0 },
  { "dm2ttl", 0 },
  { "dm2int", 0 },

  // custom music from MUSINFO lump
  { "musinfo", 0 }
};


//
// Information about all the sfx
//

sfxinfo_t doom_S_sfx[] = {
  // S_sfx[0] needs to be a dummy for odd reasons.
  { "none", 0, 0, -1, -1, 0, 0, 0, "" },
  { "pistol", 64, 0, -1, -1, 0, 0, 0, "" },
  { "shotgn", 64, 0, -1, -1, 0, 0, 0, "" },
  { "sgcock", 64, 0, -1, -1, 0, 0, 0, "" },
  { "dshtgn", 64, 0, -1, -1, 0, 0, 0, "" },
  { "dbopn", 64, 0, -1, -1, 0, 0, 0, "" },
  { "dbcls", 64, 0, -1, -1, 0, 0, 0, "" },
  { "dbload", 64, 0, -1, -1, 0, 0, 0, "" },
  { "plasma", 64, 0, -1, -1, 0, 0, 0, "" },
  { "bfg", 64, 0, -1, -1, 0, 0, 0, "" },
  { "sawup", 64, 0, -1, -1, 0, 0, 0, "" },
  { "sawidl", 118, 0, -1, -1, 0, 0, 0, "" },
  { "sawful", 64, 0, -1, -1, 0, 0, 0, "" },
  { "sawhit", 64, 0, -1, -1, 0, 0, 0, "" },
  { "rlaunc", 64, 0, -1, -1, 0, 0, 0, "" },
  { "rxplod", 70, 0, -1, -1, 0, 0, 0, "" },
  { "firsht", 70, 0, -1, -1, 0, 0, 0, "" },
  { "firxpl", 70, 0, -1, -1, 0, 0, 0, "" },
  { "pstart", 100, 0, -1, -1, 0, 0, 0, "" },
  { "pstop", 100, 0, -1, -1, 0, 0, 0, "" },
  { "doropn", 100, 0, -1, -1, 0, 0, 0, "" },
  { "dorcls", 100, 0, -1, -1, 0, 0, 0, "" },
  { "stnmov", 119, 0, -1, -1, 0, 0, 0, "" },
  { "swtchn", 78, 0, -1, -1, 0, 0, 0, "" },
  { "swtchx", 78, 0, -1, -1, 0, 0, 0, "" },
  { "plpain", 96, 0, -1, -1, 0, 0, 0, "" },
  { "dmpain", 96, 0, -1, -1, 0, 0, 0, "" },
  { "popain", 96, 0, -1, -1, 0, 0, 0, "" },
  { "vipain", 96, 0, -1, -1, 0, 0, 0, "" },
  { "mnpain", 96, 0, -1, -1, 0, 0, 0, "" },
  { "pepain", 96, 0, -1, -1, 0, 0, 0, "" },
  { "slop", 78, 0, -1, -1, 0, 0, 0, "" },
  { "itemup", 78, 0, -1, -1, 0, 0, 0, "" },
  { "wpnup", 78, 0, -1, -1, 0, 0, 0, "" },
  { "oof", 96, 0, -1, -1, 0, 0, 0, "" },
  { "telept", 32, 0, -1, -1, 0, 0, 0, "" },
  { "posit1", 98, 0, -1, -1, 0, 0, 0, "" },
  { "posit2", 98, 0, -1, -1, 0, 0, 0, "" },
  { "posit3", 98, 0, -1, -1, 0, 0, 0, "" },
  { "bgsit1", 98, 0, -1, -1, 0, 0, 0, "" },
  { "bgsit2", 98, 0, -1, -1, 0, 0, 0, "" },
  { "sgtsit", 98, 0, -1, -1, 0, 0, 0, "" },
  { "cacsit", 98, 0, -1, -1, 0, 0, 0, "" },
  { "brssit", 94, 0, -1, -1, 0, 0, 0, "" },
  { "cybsit", 92, 0, -1, -1, 0, 0, 0, "" },
  { "spisit", 90, 0, -1, -1, 0, 0, 0, "" },
  { "bspsit", 90, 0, -1, -1, 0, 0, 0, "" },
  { "kntsit", 90, 0, -1, -1, 0, 0, 0, "" },
  { "vilsit", 90, 0, -1, -1, 0, 0, 0, "" },
  { "mansit", 90, 0, -1, -1, 0, 0, 0, "" },
  { "pesit", 90, 0, -1, -1, 0, 0, 0, "" },
  { "sklatk", 70, 0, -1, -1, 0, 0, 0, "" },
  { "sgtatk", 70, 0, -1, -1, 0, 0, 0, "" },
  { "skepch", 70, 0, -1, -1, 0, 0, 0, "" },
  { "vilatk", 70, 0, -1, -1, 0, 0, 0, "" },
  { "claw", 70, 0, -1, -1, 0, 0, 0, "" },
  { "skeswg", 70, 0, -1, -1, 0, 0, 0, "" },
  { "pldeth", 32, 0, -1, -1, 0, 0, 0, "" },
  { "pdiehi", 32, 0, -1, -1, 0, 0, 0, "" },
  { "podth1", 70, 0, -1, -1, 0, 0, 0, "" },
  { "podth2", 70, 0, -1, -1, 0, 0, 0, "" },
  { "podth3", 70, 0, -1, -1, 0, 0, 0, "" },
  { "bgdth1", 70, 0, -1, -1, 0, 0, 0, "" },
  { "bgdth2", 70, 0, -1, -1, 0, 0, 0, "" },
  { "sgtdth", 70, 0, -1, -1, 0, 0, 0, "" },
  { "cacdth", 70, 0, -1, -1, 0, 0, 0, "" },
  { "skldth", 70, 0, -1, -1, 0, 0, 0, "" },
  { "brsdth", 32, 0, -1, -1, 0, 0, 0, "" },
  { "cybdth", 32, 0, -1, -1, 0, 0, 0, "" },
  { "spidth", 32, 0, -1, -1, 0, 0, 0, "" },
  { "bspdth", 32, 0, -1, -1, 0, 0, 0, "" },
  { "vildth", 32, 0, -1, -1, 0, 0, 0, "" },
  { "kntdth", 32, 0, -1, -1, 0, 0, 0, "" },
  { "pedth", 32, 0, -1, -1, 0, 0, 0, "" },
  { "skedth", 32, 0, -1, -1, 0, 0, 0, "" },
  { "posact", 120, 0, -1, -1, 0, 0, 0, "" },
  { "bgact", 120, 0, -1, -1, 0, 0, 0, "" },
  { "dmact", 120, 0, -1, -1, 0, 0, 0, "" },
  { "bspact", 100, 0, -1, -1, 0, 0, 0, "" },
  { "bspwlk", 100, 0, -1, -1, 0, 0, 0, "" },
  { "vilact", 100, 0, -1, -1, 0, 0, 0, "" },
  { "noway", 78, 0, -1, -1, 0, 0, 0, "" },
  { "barexp", 60, 0, -1, -1, 0, 0, 0, "" },
  { "punch", 64, 0, -1, -1, 0, 0, 0, "" },
  { "hoof", 70, 0, -1, -1, 0, 0, 0, "" },
  { "metal", 70, 0, -1, -1, 0, 0, 0, "" },
  { "chgun", 64, &doom_S_sfx[sfx_pistol], 150, 0, 0, 0, 0, "" },
  { "tink", 60, 0, -1, -1, 0, 0, 0, "" },
  { "bdopn", 100, 0, -1, -1, 0, 0, 0, "" },
  { "bdcls", 100, 0, -1, -1, 0, 0, 0, "" },
  { "itmbk", 100, 0, -1, -1, 0, 0, 0, "" },
  { "flame", 32, 0, -1, -1, 0, 0, 0, "" },
  { "flamst", 32, 0, -1, -1, 0, 0, 0, "" },
  { "getpow", 60, 0, -1, -1, 0, 0, 0, "" },
  { "bospit", 70, 0, -1, -1, 0, 0, 0, "" },
  { "boscub", 70, 0, -1, -1, 0, 0, 0, "" },
  { "bossit", 70, 0, -1, -1, 0, 0, 0, "" },
  { "bospn", 70, 0, -1, -1, 0, 0, 0, "" },
  { "bosdth", 70, 0, -1, -1, 0, 0, 0, "" },
  { "manatk", 70, 0, -1, -1, 0, 0, 0, "" },
  { "mandth", 70, 0, -1, -1, 0, 0, 0, "" },
  { "sssit", 70, 0, -1, -1, 0, 0, 0, "" },
  { "ssdth", 70, 0, -1, -1, 0, 0, 0, "" },
  { "keenpn", 70, 0, -1, -1, 0, 0, 0, "" },
  { "keendt", 70, 0, -1, -1, 0, 0, 0, "" },
  { "skeact", 70, 0, -1, -1, 0, 0, 0, "" },
  { "skesit", 70, 0, -1, -1, 0, 0, 0, "" },
  { "skeatk", 70, 0, -1, -1, 0, 0, 0, "" },
  { "radio", 60, 0, -1, -1, 0, 0, 0, "" },

  // killough 11/98: dog sounds
  { "dgsit", 98, 0, -1, -1, 0, 0, 0, "" },
  { "dgatk", 70, 0, -1, -1, 0, 0, 0, "" },
  { "dgact", 120, 0, -1, -1, 0, 0, 0, "" },
  { "dgdth", 70, 0, -1, -1, 0, 0, 0, "" },
  { "dgpain", 96, 0, -1, -1, 0, 0, 0, "" },

  //e6y
  { "secret", 60, 0, -1, -1, 0, 0, 0, "" },
  { "gibdth", 60, 0, -1, -1, 0, 0, 0, "" },
  // Everything from here up to 500 is reserved for future use.

  // Free slots for DEHEXTRA. Priorities should be overridden by user.
  // There is a gap present to accomodate Eternity Engine - see their commit
  // @ https://github.com/team-eternity/eternity/commit/b8fb8f71 - which  means
  // I must use desginated initializers, or else supply an exact number of dummy
  // entries to pad it out. Not sure which would be uglier to maintain. -SH
  [500] = { "fre000", 127, 0, -1, -1, 0, 0, 0, "" },
  [501] = { "fre001", 127, 0, -1, -1, 0, 0, 0, "" },
  [502] = { "fre002", 127, 0, -1, -1, 0, 0, 0, "" },
  [503] = { "fre003", 127, 0, -1, -1, 0, 0, 0, "" },
  [504] = { "fre004", 127, 0, -1, -1, 0, 0, 0, "" },
  [505] = { "fre005", 127, 0, -1, -1, 0, 0, 0, "" },
  [506] = { "fre006", 127, 0, -1, -1, 0, 0, 0, "" },
  [507] = { "fre007", 127, 0, -1, -1, 0, 0, 0, "" },
  [508] = { "fre008", 127, 0, -1, -1, 0, 0, 0, "" },
  [509] = { "fre009", 127, 0, -1, -1, 0, 0, 0, "" },
  [510] = { "fre010", 127, 0, -1, -1, 0, 0, 0, "" },
  [511] = { "fre011", 127, 0, -1, -1, 0, 0, 0, "" },
  [512] = { "fre012", 127, 0, -1, -1, 0, 0, 0, "" },
  [513] = { "fre013", 127, 0, -1, -1, 0, 0, 0, "" },
  [514] = { "fre014", 127, 0, -1, -1, 0, 0, 0, "" },
  [515] = { "fre015", 127, 0, -1, -1, 0, 0, 0, "" },
  [516] = { "fre016", 127, 0, -1, -1, 0, 0, 0, "" },
  [517] = { "fre017", 127, 0, -1, -1, 0, 0, 0, "" },
  [518] = { "fre018", 127, 0, -1, -1, 0, 0, 0, "" },
  [519] = { "fre019", 127, 0, -1, -1, 0, 0, 0, "" },
  [520] = { "fre020", 127, 0, -1, -1, 0, 0, 0, "" },
  [521] = { "fre021", 127, 0, -1, -1, 0, 0, 0, "" },
  [522] = { "fre022", 127, 0, -1, -1, 0, 0, 0, "" },
  [523] = { "fre023", 127, 0, -1, -1, 0, 0, 0, "" },
  [524] = { "fre024", 127, 0, -1, -1, 0, 0, 0, "" },
  [525] = { "fre025", 127, 0, -1, -1, 0, 0, 0, "" },
  [526] = { "fre026", 127, 0, -1, -1, 0, 0, 0, "" },
  [527] = { "fre027", 127, 0, -1, -1, 0, 0, 0, "" },
  [528] = { "fre028", 127, 0, -1, -1, 0, 0, 0, "" },
  [529] = { "fre029", 127, 0, -1, -1, 0, 0, 0, "" },
  [530] = { "fre030", 127, 0, -1, -1, 0, 0, 0, "" },
  [531] = { "fre031", 127, 0, -1, -1, 0, 0, 0, "" },
  [532] = { "fre032", 127, 0, -1, -1, 0, 0, 0, "" },
  [533] = { "fre033", 127, 0, -1, -1, 0, 0, 0, "" },
  [534] = { "fre034", 127, 0, -1, -1, 0, 0, 0, "" },
  [535] = { "fre035", 127, 0, -1, -1, 0, 0, 0, "" },
  [536] = { "fre036", 127, 0, -1, -1, 0, 0, 0, "" },
  [537] = { "fre037", 127, 0, -1, -1, 0, 0, 0, "" },
  [538] = { "fre038", 127, 0, -1, -1, 0, 0, 0, "" },
  [539] = { "fre039", 127, 0, -1, -1, 0, 0, 0, "" },
  [540] = { "fre040", 127, 0, -1, -1, 0, 0, 0, "" },
  [541] = { "fre041", 127, 0, -1, -1, 0, 0, 0, "" },
  [542] = { "fre042", 127, 0, -1, -1, 0, 0, 0, "" },
  [543] = { "fre043", 127, 0, -1, -1, 0, 0, 0, "" },
  [544] = { "fre044", 127, 0, -1, -1, 0, 0, 0, "" },
  [545] = { "fre045", 127, 0, -1, -1, 0, 0, 0, "" },
  [546] = { "fre046", 127, 0, -1, -1, 0, 0, 0, "" },
  [547] = { "fre047", 127, 0, -1, -1, 0, 0, 0, "" },
  [548] = { "fre048", 127, 0, -1, -1, 0, 0, 0, "" },
  [549] = { "fre049", 127, 0, -1, -1, 0, 0, 0, "" },
  [550] = { "fre050", 127, 0, -1, -1, 0, 0, 0, "" },
  [551] = { "fre051", 127, 0, -1, -1, 0, 0, 0, "" },
  [552] = { "fre052", 127, 0, -1, -1, 0, 0, 0, "" },
  [553] = { "fre053", 127, 0, -1, -1, 0, 0, 0, "" },
  [554] = { "fre054", 127, 0, -1, -1, 0, 0, 0, "" },
  [555] = { "fre055", 127, 0, -1, -1, 0, 0, 0, "" },
  [556] = { "fre056", 127, 0, -1, -1, 0, 0, 0, "" },
  [557] = { "fre057", 127, 0, -1, -1, 0, 0, 0, "" },
  [558] = { "fre058", 127, 0, -1, -1, 0, 0, 0, "" },
  [559] = { "fre059", 127, 0, -1, -1, 0, 0, 0, "" },
  [560] = { "fre060", 127, 0, -1, -1, 0, 0, 0, "" },
  [561] = { "fre061", 127, 0, -1, -1, 0, 0, 0, "" },
  [562] = { "fre062", 127, 0, -1, -1, 0, 0, 0, "" },
  [563] = { "fre063", 127, 0, -1, -1, 0, 0, 0, "" },
  [564] = { "fre064", 127, 0, -1, -1, 0, 0, 0, "" },
  [565] = { "fre065", 127, 0, -1, -1, 0, 0, 0, "" },
  [566] = { "fre066", 127, 0, -1, -1, 0, 0, 0, "" },
  [567] = { "fre067", 127, 0, -1, -1, 0, 0, 0, "" },
  [568] = { "fre068", 127, 0, -1, -1, 0, 0, 0, "" },
  [569] = { "fre069", 127, 0, -1, -1, 0, 0, 0, "" },
  [570] = { "fre070", 127, 0, -1, -1, 0, 0, 0, "" },
  [571] = { "fre071", 127, 0, -1, -1, 0, 0, 0, "" },
  [572] = { "fre072", 127, 0, -1, -1, 0, 0, 0, "" },
  [573] = { "fre073", 127, 0, -1, -1, 0, 0, 0, "" },
  [574] = { "fre074", 127, 0, -1, -1, 0, 0, 0, "" },
  [575] = { "fre075", 127, 0, -1, -1, 0, 0, 0, "" },
  [576] = { "fre076", 127, 0, -1, -1, 0, 0, 0, "" },
  [577] = { "fre077", 127, 0, -1, -1, 0, 0, 0, "" },
  [578] = { "fre078", 127, 0, -1, -1, 0, 0, 0, "" },
  [579] = { "fre079", 127, 0, -1, -1, 0, 0, 0, "" },
  [580] = { "fre080", 127, 0, -1, -1, 0, 0, 0, "" },
  [581] = { "fre081", 127, 0, -1, -1, 0, 0, 0, "" },
  [582] = { "fre082", 127, 0, -1, -1, 0, 0, 0, "" },
  [583] = { "fre083", 127, 0, -1, -1, 0, 0, 0, "" },
  [584] = { "fre084", 127, 0, -1, -1, 0, 0, 0, "" },
  [585] = { "fre085", 127, 0, -1, -1, 0, 0, 0, "" },
  [586] = { "fre086", 127, 0, -1, -1, 0, 0, 0, "" },
  [587] = { "fre087", 127, 0, -1, -1, 0, 0, 0, "" },
  [588] = { "fre088", 127, 0, -1, -1, 0, 0, 0, "" },
  [589] = { "fre089", 127, 0, -1, -1, 0, 0, 0, "" },
  [590] = { "fre090", 127, 0, -1, -1, 0, 0, 0, "" },
  [591] = { "fre091", 127, 0, -1, -1, 0, 0, 0, "" },
  [592] = { "fre092", 127, 0, -1, -1, 0, 0, 0, "" },
  [593] = { "fre093", 127, 0, -1, -1, 0, 0, 0, "" },
  [594] = { "fre094", 127, 0, -1, -1, 0, 0, 0, "" },
  [595] = { "fre095", 127, 0, -1, -1, 0, 0, 0, "" },
  [596] = { "fre096", 127, 0, -1, -1, 0, 0, 0, "" },
  [597] = { "fre097", 127, 0, -1, -1, 0, 0, 0, "" },
  [598] = { "fre098", 127, 0, -1, -1, 0, 0, 0, "" },
  [599] = { "fre099", 127, 0, -1, -1, 0, 0, 0, "" },
  [600] = { "fre100", 127, 0, -1, -1, 0, 0, 0, "" },
  [601] = { "fre101", 127, 0, -1, -1, 0, 0, 0, "" },
  [602] = { "fre102", 127, 0, -1, -1, 0, 0, 0, "" },
  [603] = { "fre103", 127, 0, -1, -1, 0, 0, 0, "" },
  [604] = { "fre104", 127, 0, -1, -1, 0, 0, 0, "" },
  [605] = { "fre105", 127, 0, -1, -1, 0, 0, 0, "" },
  [606] = { "fre106", 127, 0, -1, -1, 0, 0, 0, "" },
  [607] = { "fre107", 127, 0, -1, -1, 0, 0, 0, "" },
  [608] = { "fre108", 127, 0, -1, -1, 0, 0, 0, "" },
  [609] = { "fre109", 127, 0, -1, -1, 0, 0, 0, "" },
  [610] = { "fre110", 127, 0, -1, -1, 0, 0, 0, "" },
  [611] = { "fre111", 127, 0, -1, -1, 0, 0, 0, "" },
  [612] = { "fre112", 127, 0, -1, -1, 0, 0, 0, "" },
  [613] = { "fre113", 127, 0, -1, -1, 0, 0, 0, "" },
  [614] = { "fre114", 127, 0, -1, -1, 0, 0, 0, "" },
  [615] = { "fre115", 127, 0, -1, -1, 0, 0, 0, "" },
  [616] = { "fre116", 127, 0, -1, -1, 0, 0, 0, "" },
  [617] = { "fre117", 127, 0, -1, -1, 0, 0, 0, "" },
  [618] = { "fre118", 127, 0, -1, -1, 0, 0, 0, "" },
  [619] = { "fre119", 127, 0, -1, -1, 0, 0, 0, "" },
  [620] = { "fre120", 127, 0, -1, -1, 0, 0, 0, "" },
  [621] = { "fre121", 127, 0, -1, -1, 0, 0, 0, "" },
  [622] = { "fre122", 127, 0, -1, -1, 0, 0, 0, "" },
  [623] = { "fre123", 127, 0, -1, -1, 0, 0, 0, "" },
  [624] = { "fre124", 127, 0, -1, -1, 0, 0, 0, "" },
  [625] = { "fre125", 127, 0, -1, -1, 0, 0, 0, "" },
  [626] = { "fre126", 127, 0, -1, -1, 0, 0, 0, "" },
  [627] = { "fre127", 127, 0, -1, -1, 0, 0, 0, "" },
  [628] = { "fre128", 127, 0, -1, -1, 0, 0, 0, "" },
  [629] = { "fre129", 127, 0, -1, -1, 0, 0, 0, "" },
  [630] = { "fre130", 127, 0, -1, -1, 0, 0, 0, "" },
  [631] = { "fre131", 127, 0, -1, -1, 0, 0, 0, "" },
  [632] = { "fre132", 127, 0, -1, -1, 0, 0, 0, "" },
  [633] = { "fre133", 127, 0, -1, -1, 0, 0, 0, "" },
  [634] = { "fre134", 127, 0, -1, -1, 0, 0, 0, "" },
  [635] = { "fre135", 127, 0, -1, -1, 0, 0, 0, "" },
  [636] = { "fre136", 127, 0, -1, -1, 0, 0, 0, "" },
  [637] = { "fre137", 127, 0, -1, -1, 0, 0, 0, "" },
  [638] = { "fre138", 127, 0, -1, -1, 0, 0, 0, "" },
  [639] = { "fre139", 127, 0, -1, -1, 0, 0, 0, "" },
  [640] = { "fre140", 127, 0, -1, -1, 0, 0, 0, "" },
  [641] = { "fre141", 127, 0, -1, -1, 0, 0, 0, "" },
  [642] = { "fre142", 127, 0, -1, -1, 0, 0, 0, "" },
  [643] = { "fre143", 127, 0, -1, -1, 0, 0, 0, "" },
  [644] = { "fre144", 127, 0, -1, -1, 0, 0, 0, "" },
  [645] = { "fre145", 127, 0, -1, -1, 0, 0, 0, "" },
  [646] = { "fre146", 127, 0, -1, -1, 0, 0, 0, "" },
  [647] = { "fre147", 127, 0, -1, -1, 0, 0, 0, "" },
  [648] = { "fre148", 127, 0, -1, -1, 0, 0, 0, "" },
  [649] = { "fre149", 127, 0, -1, -1, 0, 0, 0, "" },
  [650] = { "fre150", 127, 0, -1, -1, 0, 0, 0, "" },
  [651] = { "fre151", 127, 0, -1, -1, 0, 0, 0, "" },
  [652] = { "fre152", 127, 0, -1, -1, 0, 0, 0, "" },
  [653] = { "fre153", 127, 0, -1, -1, 0, 0, 0, "" },
  [654] = { "fre154", 127, 0, -1, -1, 0, 0, 0, "" },
  [655] = { "fre155", 127, 0, -1, -1, 0, 0, 0, "" },
  [656] = { "fre156", 127, 0, -1, -1, 0, 0, 0, "" },
  [657] = { "fre157", 127, 0, -1, -1, 0, 0, 0, "" },
  [658] = { "fre158", 127, 0, -1, -1, 0, 0, 0, "" },
  [659] = { "fre159", 127, 0, -1, -1, 0, 0, 0, "" },
  [660] = { "fre160", 127, 0, -1, -1, 0, 0, 0, "" },
  [661] = { "fre161", 127, 0, -1, -1, 0, 0, 0, "" },
  [662] = { "fre162", 127, 0, -1, -1, 0, 0, 0, "" },
  [663] = { "fre163", 127, 0, -1, -1, 0, 0, 0, "" },
  [664] = { "fre164", 127, 0, -1, -1, 0, 0, 0, "" },
  [665] = { "fre165", 127, 0, -1, -1, 0, 0, 0, "" },
  [666] = { "fre166", 127, 0, -1, -1, 0, 0, 0, "" },
  [667] = { "fre167", 127, 0, -1, -1, 0, 0, 0, "" },
  [668] = { "fre168", 127, 0, -1, -1, 0, 0, 0, "" },
  [669] = { "fre169", 127, 0, -1, -1, 0, 0, 0, "" },
  [670] = { "fre170", 127, 0, -1, -1, 0, 0, 0, "" },
  [671] = { "fre171", 127, 0, -1, -1, 0, 0, 0, "" },
  [672] = { "fre172", 127, 0, -1, -1, 0, 0, 0, "" },
  [673] = { "fre173", 127, 0, -1, -1, 0, 0, 0, "" },
  [674] = { "fre174", 127, 0, -1, -1, 0, 0, 0, "" },
  [675] = { "fre175", 127, 0, -1, -1, 0, 0, 0, "" },
  [676] = { "fre176", 127, 0, -1, -1, 0, 0, 0, "" },
  [677] = { "fre177", 127, 0, -1, -1, 0, 0, 0, "" },
  [678] = { "fre178", 127, 0, -1, -1, 0, 0, 0, "" },
  [679] = { "fre179", 127, 0, -1, -1, 0, 0, 0, "" },
  [680] = { "fre180", 127, 0, -1, -1, 0, 0, 0, "" },
  [681] = { "fre181", 127, 0, -1, -1, 0, 0, 0, "" },
  [682] = { "fre182", 127, 0, -1, -1, 0, 0, 0, "" },
  [683] = { "fre183", 127, 0, -1, -1, 0, 0, 0, "" },
  [684] = { "fre184", 127, 0, -1, -1, 0, 0, 0, "" },
  [685] = { "fre185", 127, 0, -1, -1, 0, 0, 0, "" },
  [686] = { "fre186", 127, 0, -1, -1, 0, 0, 0, "" },
  [687] = { "fre187", 127, 0, -1, -1, 0, 0, 0, "" },
  [688] = { "fre188", 127, 0, -1, -1, 0, 0, 0, "" },
  [689] = { "fre189", 127, 0, -1, -1, 0, 0, 0, "" },
  [690] = { "fre190", 127, 0, -1, -1, 0, 0, 0, "" },
  [691] = { "fre191", 127, 0, -1, -1, 0, 0, 0, "" },
  [692] = { "fre192", 127, 0, -1, -1, 0, 0, 0, "" },
  [693] = { "fre193", 127, 0, -1, -1, 0, 0, 0, "" },
  [694] = { "fre194", 127, 0, -1, -1, 0, 0, 0, "" },
  [695] = { "fre195", 127, 0, -1, -1, 0, 0, 0, "" },
  [696] = { "fre196", 127, 0, -1, -1, 0, 0, 0, "" },
  [697] = { "fre197", 127, 0, -1, -1, 0, 0, 0, "" },
  [698] = { "fre198", 127, 0, -1, -1, 0, 0, 0, "" },
  [699] = { "fre199", 127, 0, -1, -1, 0, 0, 0, "" },
};

#define DISAMBIGUATED_SFX(id, tag) { "", 0, &doom_S_sfx[id], 0, 0, 0, 0, 0, tag }

sfxinfo_t doom_disambiguated_sfx[] = {
  DISAMBIGUATED_SFX(sfx_pistol, "weapons/pistol"),
  DISAMBIGUATED_SFX(sfx_pistol, "grunt/attack"),
  DISAMBIGUATED_SFX(sfx_pistol, "menu/choose"),
  DISAMBIGUATED_SFX(sfx_pistol, "intermission/tick"),

  DISAMBIGUATED_SFX(sfx_shotgn, "weapons/shotgf"),
  DISAMBIGUATED_SFX(sfx_shotgn, "shotguy/attack"),
  DISAMBIGUATED_SFX(sfx_shotgn, "chainguy/attack"),
  DISAMBIGUATED_SFX(sfx_shotgn, "spider/attack"),
  DISAMBIGUATED_SFX(sfx_shotgn, "wolfss/attack"),

  DISAMBIGUATED_SFX(sfx_sgcock, "weapons/shotgr"),
  DISAMBIGUATED_SFX(sfx_sgcock, "intermission/paststats"),
  DISAMBIGUATED_SFX(sfx_sgcock, "intermission/pastcoopstats"),

  DISAMBIGUATED_SFX(sfx_dshtgn, "weapons/sshotf"),

  DISAMBIGUATED_SFX(sfx_dbopn, "weapons/sshoto"),

  DISAMBIGUATED_SFX(sfx_dbcls, "weapons/sshotc"),

  DISAMBIGUATED_SFX(sfx_dbload, "weapons/sshotl"),

  DISAMBIGUATED_SFX(sfx_plasma, "weapons/plasmaf"),
  DISAMBIGUATED_SFX(sfx_plasma, "baby/attack"),

  DISAMBIGUATED_SFX(sfx_bfg, "weapons/bfgf"),

  DISAMBIGUATED_SFX(sfx_sawup, "weapons/sawup"),

  DISAMBIGUATED_SFX(sfx_sawidl, "weapons/sawidle"),

  DISAMBIGUATED_SFX(sfx_sawful, "weapons/sawfull"),

  DISAMBIGUATED_SFX(sfx_sawhit, "weapons/sawhit"),

  DISAMBIGUATED_SFX(sfx_rlaunc, "weapons/rocklf"),

  DISAMBIGUATED_SFX(sfx_rxplod, "weapons/bfgx"),

  DISAMBIGUATED_SFX(sfx_firsht, "baron/attack"),
  DISAMBIGUATED_SFX(sfx_firsht, "fatso/attack"),
  DISAMBIGUATED_SFX(sfx_firsht, "imp/attack"),
  DISAMBIGUATED_SFX(sfx_firsht, "caco/attack"),

  DISAMBIGUATED_SFX(sfx_firxpl, "weapons/plasmax"),
  DISAMBIGUATED_SFX(sfx_firxpl, "fatso/shotx"),
  DISAMBIGUATED_SFX(sfx_firxpl, "imp/shotx"),
  DISAMBIGUATED_SFX(sfx_firxpl, "caco/shotx"),
  DISAMBIGUATED_SFX(sfx_firxpl, "baron/shotx"),
  DISAMBIGUATED_SFX(sfx_firxpl, "skull/death"),
  DISAMBIGUATED_SFX(sfx_firxpl, "baby/shotx"),
  DISAMBIGUATED_SFX(sfx_firxpl, "brain/cubeboom"),

  DISAMBIGUATED_SFX(sfx_pstart, "plats/pt1_strt"),

  DISAMBIGUATED_SFX(sfx_pstop, "plats/pt1_stop"),
  DISAMBIGUATED_SFX(sfx_pstop, "menu/cursor"),

  DISAMBIGUATED_SFX(sfx_doropn, "doors/dr1_open"),

  DISAMBIGUATED_SFX(sfx_dorcls, "doors/dr1_clos"),

  DISAMBIGUATED_SFX(sfx_stnmov, "plats/pt1_mid"),
  DISAMBIGUATED_SFX(sfx_stnmov, "menu/change"),

  DISAMBIGUATED_SFX(sfx_swtchn, "switches/normbutn"),
  DISAMBIGUATED_SFX(sfx_swtchn, "menu/activate"),
  DISAMBIGUATED_SFX(sfx_swtchn, "menu/backup"),
  DISAMBIGUATED_SFX(sfx_swtchn, "menu/prompt"),

  DISAMBIGUATED_SFX(sfx_swtchx, "switches/exitbutn"),
  DISAMBIGUATED_SFX(sfx_swtchx, "menu/dismiss"),
  DISAMBIGUATED_SFX(sfx_swtchx, "menu/clear"),

  DISAMBIGUATED_SFX(sfx_plpain, "*pain100"),
  DISAMBIGUATED_SFX(sfx_plpain, "*pain75"),
  DISAMBIGUATED_SFX(sfx_plpain, "*pain50"),
  DISAMBIGUATED_SFX(sfx_plpain, "*pain25"),

  DISAMBIGUATED_SFX(sfx_dmpain, "demon/pain"),
  DISAMBIGUATED_SFX(sfx_dmpain, "spectre/pain"),
  DISAMBIGUATED_SFX(sfx_dmpain, "caco/pain"),
  DISAMBIGUATED_SFX(sfx_dmpain, "baron/pain"),
  DISAMBIGUATED_SFX(sfx_dmpain, "knight/pain"),
  DISAMBIGUATED_SFX(sfx_dmpain, "skull/pain"),
  DISAMBIGUATED_SFX(sfx_dmpain, "spider/pain"),
  DISAMBIGUATED_SFX(sfx_dmpain, "baby/pain"),
  DISAMBIGUATED_SFX(sfx_dmpain, "cyber/pain"),

  DISAMBIGUATED_SFX(sfx_popain, "grunt/pain"),
  DISAMBIGUATED_SFX(sfx_popain, "shotguy/pain"),
  DISAMBIGUATED_SFX(sfx_popain, "skeleton/pain"),
  DISAMBIGUATED_SFX(sfx_popain, "chainguy/pain"),
  DISAMBIGUATED_SFX(sfx_popain, "imp/pain"),
  DISAMBIGUATED_SFX(sfx_popain, "wolfss/pain"),

  DISAMBIGUATED_SFX(sfx_vipain, "vile/pain"),

  DISAMBIGUATED_SFX(sfx_mnpain, "fatso/pain"),

  DISAMBIGUATED_SFX(sfx_pepain, "pain/pain"),

  DISAMBIGUATED_SFX(sfx_slop, "*gibbed"),
  DISAMBIGUATED_SFX(sfx_slop, "misc/gibbed"),
  DISAMBIGUATED_SFX(sfx_slop, "vile/raise"),
  DISAMBIGUATED_SFX(sfx_slop, "intermission/pastdmstats"),

  DISAMBIGUATED_SFX(sfx_itemup, "misc/i_pkup"),
  DISAMBIGUATED_SFX(sfx_itemup, "misc/k_pkup"),
  DISAMBIGUATED_SFX(sfx_itemup, "misc/health_pkup"),
  DISAMBIGUATED_SFX(sfx_itemup, "misc/armor_pkup"),
  DISAMBIGUATED_SFX(sfx_itemup, "misc/ammo_pkup"),

  DISAMBIGUATED_SFX(sfx_wpnup, "misc/w_pkup"),

  DISAMBIGUATED_SFX(sfx_oof, "*grunt"),
  DISAMBIGUATED_SFX(sfx_oof, "*land"),
  DISAMBIGUATED_SFX(sfx_oof, "menu/invalid"),

  DISAMBIGUATED_SFX(sfx_telept, "misc/teleport"),
  DISAMBIGUATED_SFX(sfx_telept, "brain/spawn"),

  DISAMBIGUATED_SFX(sfx_posit1, "grunt/sight1"),
  DISAMBIGUATED_SFX(sfx_posit1, "shotguy/sight1"),
  DISAMBIGUATED_SFX(sfx_posit1, "chainguy/sight1"),

  DISAMBIGUATED_SFX(sfx_posit2, "grunt/sight2"),
  DISAMBIGUATED_SFX(sfx_posit2, "shotguy/sight2"),
  DISAMBIGUATED_SFX(sfx_posit2, "chainguy/sight2"),

  DISAMBIGUATED_SFX(sfx_posit3, "grunt/sight3"),
  DISAMBIGUATED_SFX(sfx_posit3, "shotguy/sight3"),
  DISAMBIGUATED_SFX(sfx_posit3, "chainguy/sight3"),

  DISAMBIGUATED_SFX(sfx_bgsit1, "imp/sight1"),

  DISAMBIGUATED_SFX(sfx_bgsit2, "imp/sight2"),

  DISAMBIGUATED_SFX(sfx_sgtsit, "demon/sight"),
  DISAMBIGUATED_SFX(sfx_sgtsit, "spectre/sight"),

  DISAMBIGUATED_SFX(sfx_cacsit, "caco/sight"),

  DISAMBIGUATED_SFX(sfx_brssit, "baron/sight"),

  DISAMBIGUATED_SFX(sfx_cybsit, "cyber/sight"),

  DISAMBIGUATED_SFX(sfx_spisit, "spider/sight"),

  DISAMBIGUATED_SFX(sfx_bspsit, "baby/sight"),

  DISAMBIGUATED_SFX(sfx_kntsit, "knight/sight"),

  DISAMBIGUATED_SFX(sfx_vilsit, "vile/sight"),

  DISAMBIGUATED_SFX(sfx_mansit, "fatso/sight"),

  DISAMBIGUATED_SFX(sfx_pesit, "pain/sight"),

  DISAMBIGUATED_SFX(sfx_sklatk, "skull/melee"),

  DISAMBIGUATED_SFX(sfx_sgtatk, "demon/melee"),
  DISAMBIGUATED_SFX(sfx_sgtatk, "spectre/melee"),

  DISAMBIGUATED_SFX(sfx_skepch, "skeleton/melee"),

  DISAMBIGUATED_SFX(sfx_vilatk, "vile/start"),

  DISAMBIGUATED_SFX(sfx_claw, "imp/melee"),
  DISAMBIGUATED_SFX(sfx_claw, "baron/melee"),

  DISAMBIGUATED_SFX(sfx_skeswg, "skeleton/swing"),

  DISAMBIGUATED_SFX(sfx_pldeth, "*death"),
  DISAMBIGUATED_SFX(sfx_pldeth, "intermission/cooptotal"),

  DISAMBIGUATED_SFX(sfx_pdiehi, "*xdeath"),

  DISAMBIGUATED_SFX(sfx_podth1, "grunt/death1"),
  DISAMBIGUATED_SFX(sfx_podth1, "shotguy/death1"),
  DISAMBIGUATED_SFX(sfx_podth1, "chainguy/death1"),

  DISAMBIGUATED_SFX(sfx_podth2, "grunt/death2"),
  DISAMBIGUATED_SFX(sfx_podth2, "shotguy/death2"),
  DISAMBIGUATED_SFX(sfx_podth2, "chainguy/death2"),

  DISAMBIGUATED_SFX(sfx_podth3, "grunt/death3"),
  DISAMBIGUATED_SFX(sfx_podth3, "shotguy/death3"),
  DISAMBIGUATED_SFX(sfx_podth3, "chainguy/death3"),

  DISAMBIGUATED_SFX(sfx_bgdth1, "imp/death1"),

  DISAMBIGUATED_SFX(sfx_bgdth2, "imp/death2"),

  DISAMBIGUATED_SFX(sfx_sgtdth, "demon/death"),
  DISAMBIGUATED_SFX(sfx_sgtdth, "spectre/death"),

  DISAMBIGUATED_SFX(sfx_cacdth, "caco/death"),

  DISAMBIGUATED_SFX(sfx_skldth, "misc/unused"),

  DISAMBIGUATED_SFX(sfx_brsdth, "baron/death"),

  DISAMBIGUATED_SFX(sfx_cybdth, "cyber/death"),

  DISAMBIGUATED_SFX(sfx_spidth, "spider/death"),

  DISAMBIGUATED_SFX(sfx_bspdth, "baby/death"),

  DISAMBIGUATED_SFX(sfx_vildth, "vile/death"),

  DISAMBIGUATED_SFX(sfx_kntdth, "knight/death"),

  DISAMBIGUATED_SFX(sfx_pedth, "pain/death"),

  DISAMBIGUATED_SFX(sfx_skedth, "skeleton/death"),

  DISAMBIGUATED_SFX(sfx_posact, "grunt/active"),
  DISAMBIGUATED_SFX(sfx_posact, "shotguy/active"),
  DISAMBIGUATED_SFX(sfx_posact, "fatso/active"),
  DISAMBIGUATED_SFX(sfx_posact, "chainguy/active"),
  DISAMBIGUATED_SFX(sfx_posact, "wolfss/active"),

  DISAMBIGUATED_SFX(sfx_bgact, "imp/active"),

  DISAMBIGUATED_SFX(sfx_dmact, "demon/active"),
  DISAMBIGUATED_SFX(sfx_dmact, "spectre/active"),
  DISAMBIGUATED_SFX(sfx_dmact, "caco/active"),
  DISAMBIGUATED_SFX(sfx_dmact, "baron/active"),
  DISAMBIGUATED_SFX(sfx_dmact, "knight/active"),
  DISAMBIGUATED_SFX(sfx_dmact, "skull/active"),
  DISAMBIGUATED_SFX(sfx_dmact, "spider/active"),
  DISAMBIGUATED_SFX(sfx_dmact, "cyber/active"),
  DISAMBIGUATED_SFX(sfx_dmact, "pain/active"),

  DISAMBIGUATED_SFX(sfx_bspact, "baby/active"),

  DISAMBIGUATED_SFX(sfx_bspwlk, "baby/walk"),

  DISAMBIGUATED_SFX(sfx_vilact, "vile/active"),

  DISAMBIGUATED_SFX(sfx_noway, "*usefail"),
  DISAMBIGUATED_SFX(sfx_noway, "misc/keytry"),

  DISAMBIGUATED_SFX(sfx_barexp, "weapons/rocklx"),
  DISAMBIGUATED_SFX(sfx_barexp, "vile/stop"),
  DISAMBIGUATED_SFX(sfx_barexp, "skeleton/tracex"),
  DISAMBIGUATED_SFX(sfx_barexp, "world/barrelx"),
  DISAMBIGUATED_SFX(sfx_barexp, "misc/brainexplode"),
  DISAMBIGUATED_SFX(sfx_barexp, "intermission/nextstage"),

  DISAMBIGUATED_SFX(sfx_punch, "*fist"),

  DISAMBIGUATED_SFX(sfx_hoof, "cyber/hoof"),

  DISAMBIGUATED_SFX(sfx_metal, "spider/walk"),

  DISAMBIGUATED_SFX(sfx_chgun, "weapons/chngun"), // -> chgun -> pistol

  DISAMBIGUATED_SFX(sfx_tink, "misc/chat2"),

  DISAMBIGUATED_SFX(sfx_bdopn, "doors/dr2_open"),

  DISAMBIGUATED_SFX(sfx_bdcls, "doors/dr2_clos"),

  DISAMBIGUATED_SFX(sfx_itmbk, "misc/spawn"),

  DISAMBIGUATED_SFX(sfx_flame, "vile/firecrkl"),

  DISAMBIGUATED_SFX(sfx_flamst, "vile/firestrt"),

  DISAMBIGUATED_SFX(sfx_getpow, "misc/p_pkup"),

  DISAMBIGUATED_SFX(sfx_bospit, "brain/spit"),

  DISAMBIGUATED_SFX(sfx_boscub, "brain/cube"),

  DISAMBIGUATED_SFX(sfx_bossit, "brain/sight"),

  DISAMBIGUATED_SFX(sfx_bospn, "brain/pain"),

  DISAMBIGUATED_SFX(sfx_bosdth, "brain/death"),

  DISAMBIGUATED_SFX(sfx_manatk, "fatso/raiseguns"),

  DISAMBIGUATED_SFX(sfx_mandth, "fatso/death"),

  DISAMBIGUATED_SFX(sfx_sssit, "wolfss/sight"),

  DISAMBIGUATED_SFX(sfx_ssdth, "wolfss/death"),

  DISAMBIGUATED_SFX(sfx_keenpn, "keen/pain"),

  DISAMBIGUATED_SFX(sfx_keendt, "keen/death"),

  DISAMBIGUATED_SFX(sfx_skeact, "skeleton/active"),

  DISAMBIGUATED_SFX(sfx_skesit, "skeleton/sight"),

  DISAMBIGUATED_SFX(sfx_skeatk, "skeleton/attack"),

  DISAMBIGUATED_SFX(sfx_radio, "misc/chat"),

  DISAMBIGUATED_SFX(sfx_dgsit, "dog/sight"),

  DISAMBIGUATED_SFX(sfx_dgatk, "dog/attack"),

  DISAMBIGUATED_SFX(sfx_dgact, "dog/active"),

  DISAMBIGUATED_SFX(sfx_dgdth, "dog/death"),

  DISAMBIGUATED_SFX(sfx_dgpain, "dog/pain"),

  DISAMBIGUATED_SFX(sfx_secret, "misc/secret"),
};
