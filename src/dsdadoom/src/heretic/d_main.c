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

// D_main.c

#include "sounds.h"
#include "g_game.h"
#include "d_main.h"

#include "heretic/dstrings.h"

static void Heretic_D_DrawTitle(const char *_x)
{
  D_SetPage("TITLE", 210, heretic_mus_titl);
}

static void Heretic_D_DrawTitle2(const char *_x)
{
  D_SetPage("TITLE", 140, 0);
}

static void Heretic_D_DrawCredits(const char *_x)
{
  D_SetPage("CREDIT", 200, 0);
}

static void Heretic_D_DrawOrder(const char *_x)
{
  D_SetPage("ORDER", 200, 0);
}

const demostate_t heretic_demostates[][4] =
{
  {
    { Heretic_D_DrawTitle, NULL },
    { Heretic_D_DrawTitle, NULL },
    { Heretic_D_DrawTitle, NULL },
    { Heretic_D_DrawTitle, NULL },
  },

  {
    { Heretic_D_DrawTitle2, NULL },
    { Heretic_D_DrawTitle2, NULL },
    { Heretic_D_DrawTitle2, NULL },
    { Heretic_D_DrawTitle2, NULL },
  },

  {
    { G_DeferedPlayDemo, "demo1" },
    { G_DeferedPlayDemo, "demo1" },
    { G_DeferedPlayDemo, "demo1" },
    { G_DeferedPlayDemo, "demo1" },
  },

  {
    { Heretic_D_DrawCredits, NULL },
    { Heretic_D_DrawCredits, NULL },
    { Heretic_D_DrawCredits, NULL },
    { Heretic_D_DrawCredits, NULL },
  },

  {
    { G_DeferedPlayDemo, "demo2" },
    { G_DeferedPlayDemo, "demo2" },
    { G_DeferedPlayDemo, "demo2" },
    { G_DeferedPlayDemo, "demo2" },
  },

  {
    { Heretic_D_DrawOrder,   NULL },
    { Heretic_D_DrawCredits, NULL },
    { Heretic_D_DrawCredits, NULL },
    { Heretic_D_DrawCredits, NULL },
  },

  {
    { G_DeferedPlayDemo, "demo3" },
    { G_DeferedPlayDemo, "demo3" },
    { G_DeferedPlayDemo, "demo3" },
    { G_DeferedPlayDemo, "demo3" },
  },

  {
    {NULL},
    {NULL},
    {NULL},
    {NULL},
  }
};
