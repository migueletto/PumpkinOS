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
//	DSDA Extended Demo
//

#ifndef __DSDA_EXDEMO__
#define __DSDA_EXDEMO__

#include "doomtype.h"

int dsda_IsExDemoSigned(void);
void dsda_MergeExDemoFeatures(void);
void dsda_LoadExDemo(const char* filename);
int dsda_CopyExDemo(const byte** buffer, int* length);
void dsda_WriteExDemoFooter(void);

#endif
