
/*
 * @(#)plex.c
 *
 * Copyright 1997-1999, Wes Cherry   (mailto:wesc@technosis.com)
 *           2000-2003, Aaron Ardiri (mailto:aaron@ardiri.com)
 * All rights reserved.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;  either version 2, or (at your option)
 * any version.
 *
 * This program is distributed in the hope that it will be useful,  but
 * WITHOUT  ANY  WARRANTY;   without  even   the  implied  warranty  of 
 * MERCHANTABILITY  or FITNESS FOR A  PARTICULAR  PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You  should have  received a  copy of the GNU General Public License
 * along with this program;  if not,  please write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Revisions:
 * ==========
 *
 * pre 18-Jun-2000 <numerous developers>
 *                 creation
 *     18-Jun-2000 Aaron Ardiri
 *                 GNU GPL documentation additions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "pilrc.h"

static void *
PlexGetElementAtUnsafe(PLEX * pplex,
                       int i)
{
  return (void *)((unsigned char *)(pplex->m_rgb) + i * pplex->m_cbItem);
}

static BOOL
PlexSetMax(PLEX * pplex,
           int iMax)
{
  if (iMax != pplex->m_iMax)
  {
    pplex->m_iMax = iMax;
    pplex->m_rgb = realloc(pplex->m_rgb, pplex->m_iMax * pplex->m_cbItem);
    return pplex->m_rgb != NULL;
  }
  return fTrue;
}

BOOL
PlexInit(PLEX * pplex,
         int cbItem,
         int iMaxInit,
         int dGrow)
{
  pplex->m_iMac = 0;
  pplex->m_iMax = iMaxInit;
  pplex->m_dGrow = dGrow;
  pplex->m_cbItem = cbItem;
  pplex->m_rgb = (void *)calloc(cbItem, iMaxInit);
  return (pplex->m_rgb != NULL);
}

VOID
PlexFree(PLEX * pplex)
{
  free(pplex->m_rgb);
  pplex->m_iMac = pplex->m_iMax = 0;
  pplex->m_rgb = NULL;
}

int
PlexGetCount(PLEX * pplex)
{
  return pplex->m_iMac;
}

int
PlexGetMax(PLEX * pplex)
{
  return pplex->m_iMax;
}

void *
PlexGetElementAt(PLEX * pplex,
                 int i)
{
  Assert(PlexIsValid(pplex));
  Assert(i >= 0);
  Assert(i < pplex->m_iMac);
  return PlexGetElementAtUnsafe(pplex, i);
}

BOOL
PlexAddElement(PLEX * pplex,
               void *pb)
{
  Assert(PlexIsValid(pplex));
  if (pplex->m_iMac == pplex->m_iMax)
  {
    if (!PlexSetMax(pplex, pplex->m_iMax + pplex->m_dGrow))
      return fFalse;
  }
  memcpy(PlexGetElementAtUnsafe(pplex, pplex->m_iMac), pb, pplex->m_cbItem);
  pplex->m_iMac++;
  return fTrue;
}

BOOL
PlexIsValid(PLEX * pplex)
{
  Assert(pplex->m_iMac <= pplex->m_iMax);
  Assert(pplex->m_dGrow > 0);
  Assert(pplex->m_cbItem > 0);
  if (pplex->m_iMac > 0)
  {
    Assert(pplex->m_rgb != NULL);
  }
  return fTrue;
}
