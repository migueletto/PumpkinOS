/*
 * $Id: pi-serial.h,v 1.30 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-serial.h: Palm serial protocol support 
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
 
#ifndef _PILOT_SERIAL_H_
#define _PILOT_SERIAL_H_

#include "pi-args.h"
#include "pi-buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PI_SERIAL_DEV     1
#define PI_SRM_DEV        2

	struct pi_serial_impl {
		int (*open) PI_ARGS((pi_socket_t *ps,
			struct pi_sockaddr *addr, size_t addrlen));
		int (*close) PI_ARGS((pi_socket_t *ps));
		int (*changebaud) PI_ARGS((pi_socket_t *ps));
		ssize_t (*write) PI_ARGS((pi_socket_t *ps,
			PI_CONST unsigned char *buf, size_t len, int flags));
		ssize_t (*read) PI_ARGS((pi_socket_t *ps,
			pi_buffer_t *buf, size_t expect, int flags));
		int (*flush) PI_ARGS((pi_socket_t *ps, int flags));
		int (*poll) PI_ARGS((pi_socket_t *ps, int timeout));
	};

	struct pi_serial_data {
		struct pi_serial_impl impl;

		unsigned char buf[256];
		size_t buf_size;
		
		/* IO options */		
#ifndef OS2
# ifndef SGTTY
		struct termios tco;
# else
		struct sgttyb tco;
# endif
#endif

		/* Baud rate info */
		int rate;		/**< Current port baud rate */
		int establishrate;	/**< Baud rate to use after link is established. If -1, will use the max speed advertised by the device */

		int establishhighrate;	/**< Boolean: try to establish rate higher than the device publishes*/

		/* Time out */
		int timeout;
		
		/* Statistics */
		int rx_bytes;
		int rx_errors;

		int tx_bytes;
		int tx_errors;

		uint16_t portId;
	};

	extern pi_device_t *pi_serial_device
            PI_ARGS((int type));

	extern void pi_serial_impl_init
	    PI_ARGS((struct pi_serial_impl *impl));

	extern void pi_srm_impl_init
	    PI_ARGS((struct pi_serial_impl *impl));

#ifdef __cplusplus
}
#endif
#endif
