/*
 * $Id: pi-args.h,v 1.7 2006/10/17 13:24:06 desrod Exp $
 *
 * pi-args.h: Macros for prototype definitions
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

#ifndef __PILOT_ARGS_H__
#define __PILOT_ARGS_H__

/** @file pi-args.h
 *  @brief Macros for prototype definitions
 *
 */
#if ((defined(__STDC__) || defined(SABER)) && !defined(NO_PROTOTYPE)) || defined(__cplusplus) || defined(USE_PROTOTYPE) || defined(CAN_PROTOTYPE)
#   define PI_ARGS(x)       x
#   define PI_CONST const
#else
#   define PI_ARGS(x)       ()
#   define PI_CONST
#endif

#endif
