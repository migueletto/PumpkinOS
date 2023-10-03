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
// F_finale.h

#ifndef __HERETIC_F_FINALE__
#define __HERETIC_F_FINALE__

#include "d_event.h"

dboolean Heretic_F_Responder(event_t * event);
void Heretic_F_Drawer(void);
void Heretic_F_Ticker(void);
void Heretic_F_StartFinale(void);

#endif
