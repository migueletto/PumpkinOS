/*
 * pi-bluetooth.h
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
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. *
 */
#ifndef _PILOT_BLUETOOTH_H_
#define _PILOT_BLUETOOTH_H_

#include "pi-args.h"
#include "pi-buffer.h"

#ifdef HAVE_BLUEZ
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PI_BLUETOOTH_DEV     1

struct pi_bluetooth_impl
{
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

struct pi_bluetooth_data
{
	struct pi_bluetooth_impl impl;

	/* Device name passed from the front-ends */
	char *device;

	/* Which channel we're listening on, the handle
	 * for the SDP record as well as the SDP session */
	int channel;
#ifdef HAVE_BLUEZ
	uint32_t handle;
	sdp_session_t *sess;
#endif

	/* Time out */
	int timeout;
};

extern pi_device_t *pi_bluetooth_device
	PI_ARGS((int type));

extern void pi_bluetooth_impl_init
    PI_ARGS((struct pi_bluetooth_impl *impl));

#ifdef __cplusplus
}
#endif
#endif
