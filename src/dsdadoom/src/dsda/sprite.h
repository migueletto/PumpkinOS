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
//	DSDA Sprite
//

#ifndef __DSDA_SPRITE__
#define __DSDA_SPRITE__

int dsda_GetDehSpriteIndex(const char* key);
int dsda_GetOriginalSpriteIndex(const char* key);
void dsda_InitializeSprites(const char** source, int count);
void dsda_FreeDehSprites(void);

#endif
