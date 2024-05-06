/*
 * $Id: pi-todo.h,v 1.21 2006/11/22 22:52:25 adridg Exp $
 *
 * pi-todo.h: Palm ToDo application support (Classic)
 * see pi-tasks.h for Palm Tasks application support
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

#ifndef _PILOT_TODO_H_
#define _PILOT_TODO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include "pi-appinfo.h"
#include "pi-buffer.h"

	typedef enum {
		todo_v1,
	} todoType;

	typedef struct ToDo {
		int indefinite;
		struct tm due;
		int priority;
		int complete;
		char *description;
		char *note;
	} ToDo_t;

	typedef struct ToDoAppInfo {
		todoType type;
		struct CategoryAppInfo category;
		int dirty;
		int sortByPriority;
	} ToDoAppInfo_t;

	extern void free_ToDo PI_ARGS((ToDo_t *));
	extern int unpack_ToDo
	    PI_ARGS((ToDo_t *, const pi_buffer_t *record, todoType type));
	extern int pack_ToDo
	    PI_ARGS((const ToDo_t *, pi_buffer_t *record, todoType type));
	extern int unpack_ToDoAppInfo
	    PI_ARGS((ToDoAppInfo_t *, const unsigned char *record, size_t len));
	extern int pack_ToDoAppInfo
	    PI_ARGS((const ToDoAppInfo_t *, unsigned char *record, size_t len));

#ifdef __cplusplus
  };
#endif

#endif				/* _PILOT_TODO_H_ */
