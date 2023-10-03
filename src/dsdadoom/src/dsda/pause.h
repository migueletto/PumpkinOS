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
//	DSDA Pause Mode
//

#include "doomtype.h"

#define PAUSE_COMMAND   1
#define PAUSE_PLAYBACK  2
#define PAUSE_BUILDMODE 4

dboolean dsda_Paused(void);
dboolean dsda_PausedViaMenu(void);
dboolean dsda_PausedOutsideDemo(void);
dboolean dsda_CameraPaused(void);
dboolean dsda_PauseMode(int mode);
void dsda_RemovePauseMode(int mode);
void dsda_ApplyPauseMode(int mode);
void dsda_TogglePauseMode(int mode);
void dsda_ResetPauseMode(void);
int dsda_MaskPause(void);
void dsda_UnmaskPause(int mask);
