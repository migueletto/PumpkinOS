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

// MN_menu.h

#ifndef __HERETIC_MN_MENU__
#define __HERETIC_MN_MENU__

void MN_Init(void);
void MN_Ticker(void);
void MN_Drawer(void);
void MN_DrawMainMenu(void);
void MN_DrawOptions(void);
void MN_DrawSetup(void);
void MN_DrawMouse(void);
void MN_DrawSound(void);
void MN_DrawLoad(void);
void MN_DrawSave(void);
void MN_DrawPause(void);
void MN_DrawMessage(const char* messageString);
void MN_DrawSlider(int x, int y, int width, int slot);
void MN_DrTextA(const char *text, int x, int y);
int MN_TextAHeight(const char *text);
int MN_TextAWidth(const char *text);
void MN_DrTextB(const char *text, int x, int y);
int MN_TextBWidth(const char *text);

// hexen

void MN_DrawEpisode(void);
void MN_UpdateClass(int choice);
void MN_DrTextAYellow(const char *text, int x, int y);
void MN_DrawSkillMenu(void);

#endif
