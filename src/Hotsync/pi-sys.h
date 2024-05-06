/*
 * $Id: pi-sys.h,v 1.9 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-sys.h
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

#ifndef _PILOT_SYS_H
#define _PILOT_SYS_H

#include "pi-args.h"
#include "pi-buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PI_SYS_HEADER_LEN  0

	typedef struct pi_sys_data 
	{
		unsigned char txid;
	} pi_sys_data_t;

	extern pi_protocol_t *sys_protocol
	    PI_ARGS((void));

	extern ssize_t sys_tx
	  PI_ARGS((pi_socket_t *ps, PI_CONST unsigned char *buf,
		 size_t len, int flags));
	extern ssize_t sys_rx
	  PI_ARGS((pi_socket_t *ps, pi_buffer_t *buf,
		 size_t len, int flags));

	extern void sys_dump_header
	    PI_ARGS((PI_CONST unsigned char *data, int rxtx));
	extern void sys_dump
	    PI_ARGS((PI_CONST unsigned char *data, size_t len));

#ifdef __cplusplus
}
#endif
#endif				/*_PILOT_SYSPKT_H_*/
