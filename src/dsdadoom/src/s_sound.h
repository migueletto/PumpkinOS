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
 *      The not so system specific sound interface.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __S_SOUND__
#define __S_SOUND__

#include "doomtype.h"
#include "p_mobj.h"
#include "r_defs.h"

#define MAX_CHANNELS 32

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init(void);

// Kills all sounds
void S_Stop(void);

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start(void);

//
// Start sound for thing at <origin>
//  using <sound_id> from sounds.h
//
void S_StartSound(void *origin, int sound_id);
void S_LoopSound(void *origin, int sfx_id, int timeout);

void S_StartSectorSound(sector_t *sector, int sfx_id);
void S_LoopSectorSound(sector_t *sector, int sfx_id, int timeout);

void S_StartMobjSound(mobj_t *mobj, int sfx_id);
void S_StartVoidSound(int sfx_id);
void S_StartLineSound(line_t *line, degenmobj_t *soundorg, int sfx_id);

// Will start a sound at a given volume.
void S_StartSoundAtVolume(void *origin, int sound_id, int volume, int loop_timeout);

// killough 4/25/98: mask used to indicate sound origin is player item pickup
#define PICKUP_SOUND (0x8000)

// Stop sound for thing at <origin>
void S_StopSound(void* origin);

void S_StopSoundLoops(void);

extern int full_sounds;
void S_UnlinkSound(void *origin);

// Start music using <music_id> from sounds.h
void S_StartMusic(int music_id);

// Start music using <music_id> from sounds.h, and set whether looping
void S_ChangeMusic(int music_id, int looping);
void S_ChangeMusInfoMusic(int lumpnum, int looping);
dboolean S_ChangeMusicByName(const char *name, dboolean looping);
void S_RestartMusic(void);

// Stops the music fer sure.
void S_StopMusic(void);

// Stop and resume music, during game PAUSE.
void S_PauseSound(void);
void S_ResumeSound(void);

//
// Updates music & sounds
//
void S_UpdateSounds(void);

// machine-independent sound params
extern int default_numChannels;
extern int numChannels;

//jff 3/17/98 holds last IDMUS number, or -1
extern int idmusnum;

// heretic

#include "doomtype.h"

void S_SetSoundCurve(dboolean fullprocess);
void S_StartAmbientSound(void *origin, int sound_id, int volume);

// hexen

void S_StartSongName(const char *songLump, dboolean loop);
dboolean S_GetSoundPlayingInfo(void * mobj, int sound_id);
int S_GetSoundID(const char *name);

#endif
