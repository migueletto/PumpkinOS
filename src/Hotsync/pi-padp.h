/*
 * $Id: pi-padp.h,v 1.20 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-padp.h: 
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

#ifndef _PILOT_PADP_H_
#define _PILOT_PADP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pi-socket.h"

#define PI_PADP_HEADER_LEN	4
#define PI_PADP_MTU		1024

#define PI_PADP_OFFSET_TYPE	0
#define PI_PADP_OFFSET_FLGS	1
#define PI_PADP_OFFSET_SIZE	2

#define padData			0x01
#define padWake			0x101
#define padAck			0x02
#define padTickle		0x04
#define padAbort		0x08	/* PalmOS 2.0 only */

#define PADP_FL_FIRST		0x80	/**< Flag indicating that this is the first fragment in a PADP packet */
#define PADP_FL_LAST		0x40	/**< Flag indicating that this is the last fragment in a PADP packet */
#define PADP_FL_MEMERROR	0x20	/**< Flag denoting a memory error on the device */
#define	PADP_FL_LONG		0x10	/**< If set, the PADP packet size is stored on a long */

	typedef struct padp {
		unsigned char type;
		unsigned char flags;
		int size;
	} padp_t;

	typedef struct pi_padp_data 
	{
		int type;
		int last_type;
		int freeze_txid;	/**< see #PI_PADP_FREEZE_TXID sockopt */
		int use_long_format;	/**< set to != 0 if we want to transmit packets using the long size format */

		unsigned char txid;
		unsigned next_txid;

		unsigned char last_ack_txid;
		struct padp last_ack_padp;
	} pi_padp_data_t;


	extern pi_protocol_t *padp_protocol
	    PI_ARGS((void));

	extern ssize_t padp_tx
	    PI_ARGS((pi_socket_t *ps, PI_CONST unsigned char *buf,
			size_t len, int flags));

	extern ssize_t padp_rx
	    PI_ARGS((pi_socket_t *ps, pi_buffer_t *buf, size_t expect,
			int flags));

	extern void padp_dump_header
	    PI_ARGS((PI_CONST unsigned char *data, int rxtx));
	extern void padp_dump
	    PI_ARGS((PI_CONST unsigned char *data));

#ifdef __cplusplus
}
#endif
#endif				/* _PILOT_PADP_H_ */

