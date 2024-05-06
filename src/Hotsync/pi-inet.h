/*
 * $Id: pi-inet.h,v 1.10 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-inet.h: Network macro definitions
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
 
#ifndef _PILOT_INET_H_
#define _PILOT_INET_H_

#include "pi-args.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PI_NET_DEV     1

	typedef struct pi_inet_data {
		/* Time out */
		int timeout;
		
		/* Statistics */
		int rx_bytes;
		int rx_errors;

		int tx_bytes;
		int tx_errors;
	} pi_inet_data_t;

	extern pi_device_t *pi_inet_device
            PI_ARGS((int type));

#ifdef __cplusplus
}
#endif
#endif
