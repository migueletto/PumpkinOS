//
// Copyright(C) 2020 by Ryan Krafnick
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
//	DSDA Console
//

#ifndef __DSDA_CONSOLE__
#define __DSDA_CONSOLE__

#include "doomtype.h"
#include "m_menu.h"

#define CONSOLE_SCRIPT_COUNT 10

extern menu_t dsda_ConsoleDef;

int dsda_ConsoleHeight(void);
dboolean dsda_OpenConsole(void);
void dsda_UpdateConsoleText(char* text);
void dsda_UpdateConsole(int action);
void dsda_ExecuteConsoleScript(int i);
void dsda_InterpretConsoleCommands(const char* str, dboolean noise, dboolean raise_errors);

#endif
