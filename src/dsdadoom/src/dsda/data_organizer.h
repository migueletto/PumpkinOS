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
//	DSDA Data Organizer
//

#ifndef __DSDA_DATA_ORGANIZER__
#define __DSDA_DATA_ORGANIZER__

char* dsda_DetectDirectory(const char* env_key, int arg_id);
void dsda_InitDataDir(void);
char* dsda_DataDir(void);
const char* dsda_DataRoot(void);

#endif
