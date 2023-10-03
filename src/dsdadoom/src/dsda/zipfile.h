//
// Copyright(C) 2023 by Pierre Wendling
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
//	DSDA zipfile support using libzip
//

#ifndef __DSDA_ZIPFILE__
#define __DSDA_ZIPFILE__

const char* dsda_UnzipFile(const char *zipped_file_name);

void dsda_CleanZipTempDirs(void);

#endif /* __DSDA_ZIPFILE__ */
