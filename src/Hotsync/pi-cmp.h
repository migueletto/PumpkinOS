/*
 * $Id: pi-cmp.h,v 1.22 2006/10/17 13:24:06 desrod Exp $
 *
 * pi-cmp.h - Palm Connection Management Protocol interface
 *
 * This latches early in the sync to determine serial sync speeds
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

#ifndef _PILOT_CMP_H_
#define _PILOT_CMP_H_

/** @file pi-cmp.h
 *  @brief Palm Connection Management Protocol definitions
 *
 */

#include "pi-args.h"
#include "pi-buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PI_CMP_HEADER_LEN 10
#define PI_CMP_MTU        10

#define PI_CMP_OFFSET_TYPE 0
#define PI_CMP_OFFSET_FLGS 1
#define PI_CMP_OFFSET_VERS 2
#define PI_CMP_OFFSET_RESV 4
#define PI_CMP_OFFSET_BAUD 6

#define PI_CMP_TYPE_WAKE 0x01
#define PI_CMP_TYPE_INIT 0x02
#define PI_CMP_TYPE_ABRT 0x03
#define PI_CMP_TYPE_EXTN 0x04			/**< Type for extended CMP packets */

#define PI_CMP_VERS_1_0 0x0100L
#define PI_CMP_VERS_1_1 0x0101L
#define PI_CMP_VERS_1_2 0x0102L
#define PI_CMP_VERS_1_3	0x0103L

#define PI_CMP_VERSION	PI_CMP_VERS_1_2

/* CMP packet flag values */
#define	CMP_FL_CHANGE_BAUD_RATE		0x80	/**< Want to switch speeds */
#define	CMP_FL_ONE_MINUTE_TIMEOUT 	0x40	/**< Use a 1 minute timeout before dropping link */
#define	CMP_FL_TWO_MINUTE_TIMEOUT 	0x20	/**< Use a 2 minute timeout before dropping ling */
#define	CMP_FL_LONG_PACKET_SUPPORT 	0x10	/**< long PADP packet format is supported */

	struct pi_cmp_data {
		unsigned char type;
		unsigned char flags;
		unsigned int version;
		int baudrate;
	};

	extern pi_protocol_t *cmp_protocol
	    PI_ARGS((void));

	extern int cmp_rx_handshake
	  PI_ARGS((pi_socket_t *ps, int establishrate, int establishhighrate));

	extern int cmp_tx_handshake
	  PI_ARGS((pi_socket_t *ps));

	extern ssize_t cmp_tx
	  PI_ARGS((pi_socket_t *ps, PI_CONST unsigned char *buf,
		size_t len, int flags));

	extern ssize_t cmp_rx
	  PI_ARGS((pi_socket_t *ps, pi_buffer_t *msg,
		size_t expect, int flags));

	extern int cmp_init
	  PI_ARGS((pi_socket_t *ps, int baudrate));

	extern int cmp_abort
	  PI_ARGS((pi_socket_t *ps, int reason));

	extern int cmp_wakeup
	  PI_ARGS((pi_socket_t *ps, int maxbaud));

	extern void cmp_dump
	  PI_ARGS((PI_CONST unsigned char *cmp, int rxtx));

#ifdef __cplusplus
}
#endif
#endif				/* _PILOT_CMP_H_ */
