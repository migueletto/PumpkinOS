//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
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

#ifndef __HEXEN_P_ANIM__
#define __HEXEN_P_ANIM__

#define MAX_ANIM_DEFS 20

typedef struct
{
    int type;
    int index;
    int tics;
    int currentFrameDef;
    int startFrameDef;
    int endFrameDef;
} animDef_t;

extern animDef_t AnimDefs[MAX_ANIM_DEFS];

extern int NextLightningFlash;
extern int LightningFlash;

void P_AnimateSurfaces(void);
void P_ForceLightning(void);
void P_InitLightning(void);
void P_InitFTAnims(void);

#endif
