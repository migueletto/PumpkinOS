/*
 * $Id: pi-slp.h,v 1.21 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-slp.h: Palm Serial Link Protocol support
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

#ifndef _PILOT_PADP_SLP_H_
#define _PILOT_PADP_SLP_H_

#include "pi-args.h"
#include "pi-buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PI_SLP_HEADER_LEN	10
#define PI_SLP_FOOTER_LEN	2
#define PI_SLP_MTU		0xffff

#define PI_SLP_SIG_BYTE1 0xbe
#define PI_SLP_SIG_BYTE2 0xef
#define PI_SLP_SIG_BYTE3 0xed

#define PI_SLP_OFFSET_SIG1 0
#define PI_SLP_OFFSET_SIG2 1
#define PI_SLP_OFFSET_SIG3 2
#define PI_SLP_OFFSET_DEST 3
#define PI_SLP_OFFSET_SRC  4
#define PI_SLP_OFFSET_TYPE 5
#define PI_SLP_OFFSET_SIZE 6
#define PI_SLP_OFFSET_TXID 8
#define PI_SLP_OFFSET_SUM  9

#define PI_SLP_SOCK_DBG  0x00
#define PI_SLP_SOCK_CON  0x01
#define PI_SLP_SOCK_RUI  0x02
#define PI_SLP_SOCK_DLP  0x03

#define PI_SLP_TYPE_RDCP 0x00
#define PI_SLP_TYPE_PADP 0x02
#define PI_SLP_TYPE_LOOP 0x03

	struct pi_slp_data 
	{
		int dest;
		int last_dest;
		int src;
		int last_src;
		
		int type;
		int last_type;
		
		unsigned char txid;
		unsigned char last_txid;
	};
	
	struct slp {
		unsigned char _be;
		unsigned char _ef;
		unsigned char _ed;
		unsigned char dest;
		unsigned char src;
		unsigned char type;
		unsigned short dlen;
		unsigned char id_;
		unsigned char csum;
	};

	extern pi_protocol_t *slp_protocol
	    PI_ARGS((void));

	extern ssize_t slp_tx
	    PI_ARGS((pi_socket_t * ps, PI_CONST unsigned char *buf, size_t len, int flags));
	extern ssize_t slp_rx
	    PI_ARGS((pi_socket_t *ps, pi_buffer_t *buf, size_t expect, int flags));

	extern void slp_dump_header
	    PI_ARGS((PI_CONST unsigned char *data, int rxtx));
	extern void slp_dump
	    PI_ARGS((PI_CONST unsigned char *data));

#ifdef __cplusplus
}
#endif
#endif				/* _PILOT_PADP_SLP_H_ */
