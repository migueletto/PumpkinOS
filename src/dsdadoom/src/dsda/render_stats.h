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
//	DSDA Render Stats
//

#ifndef __RENDER_STATS__
#define __RENDER_STATS__

typedef struct {
  int visplanes;
  int drawsegs;
  int vissprites;
} dsda_render_stats_t;

void dsda_BeginRenderStats(void);
void dsda_RecordVisSprite(void);
void dsda_RecordVisSprites(int n);
void dsda_RecordVisPlane(void);
void dsda_RecordVisPlanes(int n);
void dsda_RecordDrawSeg(void);
void dsda_RecordDrawSegs(int n);
void dsda_UpdateRenderStats(void);

#endif
