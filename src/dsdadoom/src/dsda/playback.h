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
//	DSDA Playback
//

#include "doomtype.h"

#define PLAYBACK_NORMAL      0
#define PLAYBACK_JOIN_ON_END 1

void dsda_RestartPlayback(void);
dboolean dsda_JumpToLogicTic(int tic);
dboolean dsda_JumpToLogicTicFrom(int tic, int from_tic);
void dsda_ExecutePlaybackOptions(void);
const char* dsda_ParsePlaybackOptions(void);
const char* dsda_PlaybackName(void);
void dsda_ClearPlaybackStream(void);
void dsda_InitDemoPlayback(void);
void dsda_AttachPlaybackStream(const byte* demo_p, int length, int behaviour);
int dsda_PlaybackTics(void);
void dsda_StorePlaybackPosition(void);
void dsda_RestorePlaybackPosition(void);
void dsda_JoinDemo(ticcmd_t* cmd);
void dsda_TryPlaybackOneTick(ticcmd_t* cmd);
