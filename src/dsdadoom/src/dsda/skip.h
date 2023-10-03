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
//	DSDA Skip Mode
//

#include "doomtype.h"

dboolean dsda_SkipMode(void);
void dsda_EnterSkipMode(void);
void dsda_ExitSkipMode(void);
void dsda_ToggleSkipMode(void);
void dsda_SkipToNextMap(void);
void dsda_SkipToEndOfMap(void);
void dsda_SkipToLogicTic(int tic);
void dsda_EvaluateSkipModeGTicker(void);
void dsda_EvaluateSkipModeInitNew(void);
void dsda_EvaluateSkipModeBuildTiccmd(void);
void dsda_EvaluateSkipModeDoCompleted(void);
void dsda_EvaluateSkipModeDoTeleportNewMap(void);
void dsda_EvaluateSkipModeDoWorldDone(void);
void dsda_EvaluateSkipModeCheckDemoStatus(void);
void dsda_HandleSkip(void);
