
/*
 * @(#)plex.h
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

typedef struct _plex
{
  int m_iMac;
  int m_iMax;
  int m_dGrow;
  int m_cbItem;
  void *m_rgb;
}
PLEX;

BOOL PlexInit(PLEX * pplex,
              int cbItem,
              int iMaxInit,
              int dGrow);
VOID PlexFree(PLEX * pplex);
int PlexGetCount(PLEX * pplex);
int PlexGetMax(PLEX * pplex);
void *PlexGetElementAt(PLEX * pplex,
                       int i);
BOOL PlexAddElement(PLEX * pplex,
                    void *pb);
BOOL PlexIsValid(PLEX * pplex);

#define DEFPL(NAME) typedef PLEX NAME;
