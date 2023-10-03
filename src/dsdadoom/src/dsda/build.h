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
//	DSDA Build Mode
//

#include "d_event.h"
#include "d_ticcmd.h"
#include "tables.h"

dboolean dsda_AllowBuilding(void);
dboolean dsda_BuildMode(void);
void dsda_QueueBuildCommands(ticcmd_t* cmds, int depth);
dboolean dsda_BuildPlayback(void);
void dsda_CopyBuildCmd(ticcmd_t* cmd);
void dsda_ReadBuildCmd(ticcmd_t* cmd);
void dsda_EnterBuildMode(void);
void dsda_RefreshBuildMode(void);
dboolean dsda_BuildResponder(event_t *ev);
void dsda_ToggleBuildTurbo(void);
dboolean dsda_AdvanceFrame(void);
dboolean dsda_BuildMF(int x);
dboolean dsda_BuildMB(int x);
dboolean dsda_BuildSR(int x);
dboolean dsda_BuildSL(int x);
dboolean dsda_BuildTR(int x);
dboolean dsda_BuildTL(int x);
dboolean dsda_BuildFU(int x);
dboolean dsda_BuildFD(int x);
dboolean dsda_BuildFC(void);
dboolean dsda_BuildLU(int x);
dboolean dsda_BuildLD(int x);
dboolean dsda_BuildLC(void);
dboolean dsda_BuildUA(int x);
