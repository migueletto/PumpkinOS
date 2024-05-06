/*
 * $Id: debug.c,v 1.16 2006/10/12 14:21:22 desrod Exp $
 *
 * debug.c:  Pilot-Link debug configuration and debug logging
 *
 * Copyright (c) 1996, Anonymous
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
//#include <ctype.h>

#include "pi-debug.h"
#include "pi-threadsafe.h"

#include "sys.h"
#include "debug.h"

static int debug_types = PI_DBG_NONE;
static int debug_level = PI_DBG_LVL_NONE;

/***********************************************************************
 *
 * Function:    pi_debug_get_types
 *
 * Summary:     fetches the current debug types configuration
 *
 * Parameters:  void
 *
 * Returns:     debug_types
 *
 ***********************************************************************/
int
pi_debug_get_types (void)
{
	return debug_types;
}


/***********************************************************************
 *
 * Function:    pi_debug_set_types
 *
 * Summary:     sets the debug_types configuration
 *
 * Parameters:  types
 *
 * Returns:     void
 *
 ***********************************************************************/
void
pi_debug_set_types (int types)
{
	debug_types = types;
}


/***********************************************************************
 *
 * Function:    pi_debug_get_types
 *
 * Summary:     fetches the current debug level configuration
 *
 * Parameters:  void
 *
 * Returns:     debug_level
 *
 ***********************************************************************/
int
pi_debug_get_level (void)
{
	return debug_level;
}


/***********************************************************************
 *
 * Function:    pi_debug_set_level
 *
 * Summary:     sets the debug_level configuration
 *
 * Parameters:  level
 *
 * Returns:     void
 *
 ***********************************************************************/
void
pi_debug_set_level (int level)
{
	debug_level = level;
}

/***********************************************************************
 *
 * Function:    pi_debug_set_file
 *
 * Summary:     sets the debug log file configuration
 *
 * Parameters:  char* to logfile name
 *
 * Returns:     void
 *
 ***********************************************************************/
void
pi_debug_set_file (const char *path) 
{
}


/***********************************************************************
 *
 * Function:    pi_log
 *
 * Summary:     logs a debug message
 *
 * Parameters:  type, level, format, ...
 *
 * Returns:     void
 *
 ***********************************************************************/
void
pi_log (int type, int level, const char *format, ...)
{
	va_list ap;
    int l;

	//if (!(debug_types & type) && type != PI_DBG_ALL)
		//return;
	
	//if (debug_level < level)
		//return;

    switch (level) {
      case PI_DBG_LVL_ERR:   l = DEBUG_ERROR; break;
      case PI_DBG_LVL_WARN:  l = DEBUG_INFO;  break;
      case PI_DBG_LVL_INFO:  l = DEBUG_INFO;  break;
      case PI_DBG_LVL_DEBUG: l = DEBUG_TRACE; break;
      case PI_DBG_LVL_NONE:  l = DEBUG_TRACE; break;
      default: l = -1;
    }

    if (l != -1) {
	  va_start(ap, format);
      debugva(l, SYS_DEBUG, format, ap);
	  va_end(ap);
    }
}

void
pi_dumpline(const char *buf, size_t len, unsigned int addr)
{
	unsigned int i;
	int offset;
	char line[256];

	offset = sprintf(line, "  %.4x  ", addr);

	for (i = 0; i < 16; i++) {
		if (i < len)
			offset += sprintf(line+offset, "%.2x ",
			       0xff & (unsigned int) buf[i]);
		else {
			strcpy(line+offset, "   ");
			offset += 3;
		}
	}

	strcpy(line+offset, "  ");
	offset += 2;

	for (i = 0; i < len; i++) {
		if (buf[i] == '%') {
			/* since we're going through pi_log, we need to
			 * properly escape % characters
			 */
			line[offset++] = '%';
			line[offset++] = '%';
		} else if (/*isprint((int)buf[i]) &&*/ buf[i] >= 32 && buf[i] <= 126)
			line[offset++] = buf[i];
		else
			line[offset++] = '.';
	}

	strcpy(line+offset,"\n");
	LOG((PI_DBG_ALL, PI_DBG_LVL_NONE, line));
}

void
dumpline(const char *buf, size_t len, unsigned int addr)
{
	/* this function will be removed in 0.13. Use pi_dumpline() instead. */
	pi_dumpline(buf, len, addr);
}

void
pi_dumpdata(const char *buf, size_t len)
{
	unsigned int i;

	for (i = 0; i < len; i += 16)
		pi_dumpline(buf + i, ((len - i) > 16) ? 16 : len - i, i);
}

void
dumpdata(const char *buf, size_t len)
{
	/* this function will be removed in 0.13. Use pi_dumpdata() instead */
	pi_dumpdata(buf, len);
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
