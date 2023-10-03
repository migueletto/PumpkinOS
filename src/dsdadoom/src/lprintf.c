/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  Provides a logical console output routine that allows what is
 *  output to console normally and when output is redirected to
 *  be controlled..
 *
 *-----------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef _MSC_VER
#include <io.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "doomtype.h"
#include "lprintf.h"
#include "i_main.h"
#include "i_system.h"
#include "e6y.h"//e6y
#include "i_capture.h"

#include "host.h"
#include "debug.h"

#include "dsda/args.h"

static dboolean disable_message_box;

int cons_stdout_mask = LO_INFO;
int cons_stderr_mask = LO_WARN | LO_ERROR;

/* cphipps - enlarged message buffer and made non-static
 * We still have to be careful here, this function can be called after exit
 */
#define MAX_MESSAGE_SIZE 2048

int lprintf(OutputLevels pri, const char *s, ...)
{
  int r=0;
  char msg[MAX_MESSAGE_SIZE];
  int lvl=pri;

  va_list v;
  va_start(v,s);
  vsnprintf(msg,sizeof(msg),s,v);    /* print message in buffer  */
  va_end(v);

#if 0
#ifdef _WIN32
  // do not crash with unicode dirs
  if (fileno(stdout) != -1)
#endif
  if (lvl & cons_stdout_mask)
    r = fprintf(stdout,"%s",msg);

#ifdef _WIN32
  // do not crash with unicode dirs
  if (fileno(stderr) != -1)
#endif
  if (lvl & cons_stderr_mask)
    r = fprintf(stderr,"%s",msg);
#endif

  switch (pri) {
    case LO_INFO: lvl = DEBUG_INFO; break;
    case LO_WARN: lvl = DEBUG_ERROR; break;
    case LO_ERROR: lvl = DEBUG_ERROR; break;
    case LO_DEBUG: lvl = DEBUG_TRACE; break;
    default: lvl = DEBUG_INFO;
  }
  DG_debug(lvl, "%s", msg);

  return r;
}

void I_EnableVerboseLogging(void)
{
  cons_stdout_mask = LO_INFO | LO_DEBUG;
}

void I_DisableAllLogging(void)
{
  cons_stdout_mask = 0;
  cons_stderr_mask = 0;
}

void I_DisableMessageBoxes(void)
{
  disable_message_box = true;
}

/*
 * I_Error
 *
 * cphipps - moved out of i_* headers, to minimise source files that depend on
 * the low-level headers. All this does is print the error, then call the
 * low-level safe exit function.
 * killough 3/20/98: add const
 */

void I_Error(const char *error, ...)
{
  char errmsg[MAX_MESSAGE_SIZE];
  va_list argptr;
  va_start(argptr,error);
  vsnprintf(errmsg,sizeof(errmsg),error,argptr);
  va_end(argptr);
  lprintf(LO_ERROR, "%s\n", errmsg);
#ifdef _WIN32
  if (!disable_message_box && !dsda_Flag(dsda_arg_nodraw) && !capturing_video) {
    I_MessageBox(errmsg, PRB_MB_OK);
  }
#endif
  I_SafeExit(-1);
}

void I_Warn(const char *error, ...)
{
  char errmsg[MAX_MESSAGE_SIZE];
  va_list argptr;
  va_start(argptr, error);
  vsnprintf(errmsg, sizeof(errmsg), error, argptr);
  va_end(argptr);
  lprintf(LO_WARN, "%s\n", errmsg);
#ifdef _WIN32
  if (!dsda_Flag(dsda_arg_nodraw) && !capturing_video) {
    I_MessageBox(errmsg, PRB_MB_OK);
  }
#endif
}
