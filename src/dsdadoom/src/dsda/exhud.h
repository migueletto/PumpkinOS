//
// Copyright(C) 2021 by Ryan Krafnick
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
//	DSDA Extended HUD
//

#ifndef __DSDA_EXHUD__
#define __DSDA_EXHUD__

void dsda_InitExHud(void);
void dsda_UpdateExHud(void);
void dsda_DrawExHud(void);
void dsda_DrawExIntermission(void);
void dsda_ToggleRenderStats(void);
void dsda_RefreshExHudFPS(void);
void dsda_RefreshExHudMinimap(void);
void dsda_RefreshExHudLevelSplits(void);
void dsda_RefreshExHudCoordinateDisplay(void);
void dsda_RefreshExHudCommandDisplay(void);
void dsda_RefreshMapCoordinates(void);
void dsda_RefreshMapTotals(void);
void dsda_RefreshMapTime(void);
void dsda_RefreshMapTitle(void);

#endif
