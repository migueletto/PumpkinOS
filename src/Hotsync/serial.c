/*
 * $Id: serial.c,v 1.72 2006/10/12 14:21:22 desrod Exp $
 *
 * serial.c: Interface layer to serial HotSync connections
 *
 * Copyright (c) 1996, 1997, D. Jeff Dionne & Kenneth Albanowski
 * Copyright (c) 1999, Tilo Christ
 * Copyright (c) 2005, Florent Pillet
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
 *
 * -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
//#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>		/* Needed for Redhat 6.x machines */
#include <unistd.h>

#include "pi-debug.h"
#include "pi-source.h"
#include "pi-socket.h"
#include "pi-serial.h"
#include "pi-net.h"
#include "pi-cmp.h"
#include "pi-error.h"
#include "pi-util.h"

#include "sys.h"
#include "debug.h"

#ifdef OS2
#include <sys/select.h>
#endif

/* Declare prototypes */
static int pi_serial_connect(pi_socket_t *ps, struct sockaddr *addr, 
			size_t addrlen);
static int pi_serial_bind(pi_socket_t *ps, struct sockaddr *addr,
			size_t addrlen);
static int pi_serial_listen(pi_socket_t *ps, int backlog);
static int pi_serial_accept(pi_socket_t *ps, struct sockaddr *addr,
			size_t *addrlen);
static int pi_serial_getsockopt(pi_socket_t *ps, int level,
			int option_name, void *option_value,
			size_t *option_len);
static int pi_serial_setsockopt(pi_socket_t *ps, int level,
			int option_name, const void *option_value,
			size_t *option_len);
static int pi_serial_close(pi_socket_t *ps);

extern int pi_socket_init(pi_socket_t *ps);


/* Protocol Functions */
/***********************************************************************
 *
 * Function:    pi_serial_protocol_dup
 *
 * Summary:     clones an existing pi_protocol struct
 *
 * Parameters:  pi_protocol*
 *
 * Returns:     pi_protocol_t* or NULL if operation failed
 *
 ***********************************************************************/
static pi_protocol_t*
pi_serial_protocol_dup (pi_protocol_t *prot)
{
	pi_protocol_t *new_prot;

	ASSERT (prot != NULL);
	
	new_prot = (pi_protocol_t *) malloc(sizeof (pi_protocol_t));

	if (new_prot != NULL) {
		new_prot->level 	= prot->level;
		new_prot->dup 		= prot->dup;
		new_prot->free 		= prot->free;
		new_prot->read 		= prot->read;
		new_prot->write 	= prot->write;
		new_prot->flush		= prot->flush;
		new_prot->getsockopt 	= prot->getsockopt;
		new_prot->setsockopt 	= prot->setsockopt;
		new_prot->data 		= NULL;
	}

	return new_prot;
}


/***********************************************************************
 *
 * Function:    pi_serial_protocol_free
 *
 * Summary:     frees an existing pi_protocol struct
 *
 * Parameters:  pi_protocol*
 *
 * Returns:     void
 *
 ***********************************************************************/
static void
pi_serial_protocol_free (pi_protocol_t *prot)
{
	ASSERT (prot != NULL);

	if (prot != NULL)
		free(prot);
}


/***********************************************************************
 *
 * Function:    pi_serial_protocol
 *
 * Summary:     creates and inits pi_protocol struct instance
 *
 * Parameters:  pi_device_t*
 *
 * Returns:     pi_protocol_t* or NULL if operation failed
 *
 ***********************************************************************/
static pi_protocol_t*
pi_serial_protocol (pi_device_t *dev)
{	
	pi_protocol_t *prot;
	struct pi_serial_data *data;

	ASSERT (dev != NULL);
	
	prot = (pi_protocol_t *) malloc(sizeof (pi_protocol_t));

	data = (struct pi_serial_data *)(dev->data);

	if (prot != NULL) {
		prot->level 		= PI_LEVEL_DEV;
		prot->dup 		= pi_serial_protocol_dup;
		prot->free 		= pi_serial_protocol_free;
		prot->read 		= data->impl.read;
		prot->write 		= data->impl.write;
		prot->flush		= data->impl.flush;
		prot->getsockopt 	= pi_serial_getsockopt;
		prot->setsockopt 	= pi_serial_setsockopt;
		prot->data 		= NULL;
	}
	
	return prot;
}


/* Device Functions */
/***********************************************************************
 *
 * Function:    pi_serial_device_free
 *
 * Summary:     frees an existing pi_device struct
 *
 * Parameters:  pi_device_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
static void
pi_serial_device_free (pi_device_t *dev) 
{
	ASSERT (dev != NULL);

	free(dev->data);
	free(dev);
}


/***********************************************************************
 *
 * Function:    pi_serial_device
 *
 * Summary:     creates and inits pi_device struct instance 
 *
 * Parameters:  device type
 *
 * Returns:     pi_device_t* or NULL if operation failed
 *
 ***********************************************************************/
pi_device_t*
pi_serial_device (int type) 
{
	pi_device_t *dev;
	struct 	pi_serial_data *data;
	
	dev = (pi_device_t *) malloc(sizeof (pi_device_t));
	if (dev == NULL)
		return NULL;

	data = (struct pi_serial_data *) malloc(sizeof (struct pi_serial_data));
	if (data == NULL) {
		free(dev);
		return NULL;
	}

	dev->free 	= pi_serial_device_free;
	dev->protocol 	= pi_serial_protocol;	
	dev->bind 	= pi_serial_bind;
	dev->listen 	= pi_serial_listen;
	dev->accept 	= pi_serial_accept;
	dev->connect 	= pi_serial_connect;
	dev->close 	= pi_serial_close;

	switch (type) {
		case PI_SERIAL_DEV:
			pi_serial_impl_init (&data->impl);
			break;
		case PI_SRM_DEV:
			pi_srm_impl_init (&data->impl);
			break;
		default:
			pi_serial_impl_init (&data->impl);
			break;
	}
	
	data->buf_size 		= 0;
	data->rate 		= -1;
	data->establishrate 	= -1;
	data->establishhighrate = -1;
	data->timeout 		= 0;
	data->rx_bytes 		= 0;
	data->rx_errors 	= 0;
	data->tx_bytes 		= 0;
	data->tx_errors 	= 0;

	dev->data 		= data;

	return dev;
}


/***********************************************************************
 *
 * Function:    pi_serial_connect
 *
 * Summary:     Connect socket to a given address
 *
 * Parameters:  pi_socket*, sockaddr*, size_t
 *
 * Returns:     A negative number on error, 0 otherwise
 *
 ***********************************************************************/
static int
pi_serial_connect(pi_socket_t *ps, struct sockaddr *addr,
	size_t addrlen)
{
	struct 	pi_serial_data *data =
		(struct pi_serial_data *)ps->device->data;
	struct 	pi_sockaddr *pa = (struct pi_sockaddr *) addr;
	int err;

	if (ps->type == PI_SOCK_STREAM) {
		if (ps->protocol == PI_PF_SYS) {
			data->establishrate = data->rate = 57600;
		} else {
			if (data->establishrate == -1)
				get_pilot_rate(&data->establishrate, &data->establishhighrate);

			/* Mandatory CMP connection rate */
			data->rate = 9600;
		}
	} else if (ps->type == PI_SOCK_RAW) {
		/* Mandatory SysPkt connection rate */
		data->establishrate = data->rate = 57600;
	}

	if ((err = data->impl.open(ps, pa, addrlen)) < 0)
		return err;	/* errno already set */

	ps->raddr 	= malloc(addrlen);
	memcpy(ps->raddr, addr, addrlen);
	ps->raddrlen 	= addrlen;
	ps->laddr 	= malloc(addrlen);
	memcpy(ps->laddr, addr, addrlen);
	ps->laddrlen 	= addrlen;

	if (ps->type == PI_SOCK_STREAM) {
		size_t 	size;
		switch (ps->cmd) {
			case PI_CMD_CMP:
				if (cmp_tx_handshake(ps) < 0)
					goto fail;

				size = sizeof(data->rate);
				pi_getsockopt(ps->sd, PI_LEVEL_CMP, PI_CMP_BAUD,
						&data->rate, &size);

				if ((err = data->impl.changebaud(ps)) < 0)
					goto fail;
				break;
				
			case PI_CMD_NET:
				if ((err = data->impl.changebaud(ps)) < 0)
					goto fail;
				break;
				
			case PI_CMD_SYS:
				if ((err = data->impl.changebaud(ps)) < 0)
					goto fail;
				break;
		}
	}
	ps->state = PI_SOCK_CONN_INIT;
	ps->command = 0;
	return 0;

fail:
	return err;
}


/***********************************************************************
 *
 * Function:    pi_serial_bind
 *
 * Summary:     Bind address to a local socket
 *
 * Parameters:  pi_socket*, sockaddr*, size_t
 *
 * Returns:     A negative number on error, 0 otherwise
 *
 ***********************************************************************/
static int
pi_serial_bind(pi_socket_t *ps, struct sockaddr *addr, size_t addrlen)
{
	struct 	pi_serial_data *data =
			(struct pi_serial_data *)ps->device->data;
	struct 	pi_sockaddr *pa = (struct pi_sockaddr *) addr;
	int err; //, count = 0;

debug(1, "XXX", "pi_serial_bind");
	if (ps->type == PI_SOCK_STREAM) {
		if (data->establishrate == -1)
			get_pilot_rate(&data->establishrate, &data->establishhighrate);

		/* Mandatory CMP connection rate */
		data->rate = 9600;
debug(1, "XXX", "pi_serial_bind stream");
	} else if (ps->type == PI_SOCK_RAW) {
		/* Mandatory SysPkt connection rate */
		data->establishrate = data->rate = 57600;
debug(1, "XXX", "pi_serial_bind raw");
	}

//begin:
debug(1, "XXX", "pi_serial_bind open...");
	if ((err = data->impl.open(ps, pa, addrlen)) < 0) {
		//int 	save_errno = errno;
debug(1, "XXX", "pi_serial_bind open error");
#ifdef MAXPATHLEN
		char	realport[MAXPATHLEN];
#else
 # ifdef PATH_MAX
		char	realport[PATH_MAX];
 # else
		char	realport[4096];
 # endif /* PATH_MAX */
#endif /* MAXPATHLEN */

		realpath(pa->pi_device, realport);
		//errno = save_errno;

		LOG((PI_DBG_DEV, PI_DBG_LVL_ERR,
		    "Error opening device %s\n",
		    pa->pi_device));
#if 0
		if (errno == ENOENT) {
			LOG((PI_DBG_DEV, PI_DBG_LVL_ERR,
					" The device %s does not exist..\n",
					pa->pi_device));
			LOG((PI_DBG_DEV, PI_DBG_LVL_ERR,
					" Possible solution:\n\n\tmknod %s c "
					"<major> <minor>\n\n", pa->pi_device));
		} else if (errno == EACCES) {
			LOG((PI_DBG_DEV, PI_DBG_LVL_ERR,
					"   Please check the "
					"permissions on %s..\n", realport));
			LOG((PI_DBG_DEV, PI_DBG_LVL_ERR,
					" Possible solution:\n\n\tchmod 0666 "
					"%s\n\n", realport));
		} else if (errno == ENODEV) {
			while (count <= 5) {
				if (isatty(fileno(stdout))) {
					LOG((PI_DBG_DEV, PI_DBG_LVL_ERR,
							"\r   Port not connected,"
							" sleeping for 2 seconds, "));
					LOG((PI_DBG_DEV, PI_DBG_LVL_ERR,
							"%d retries..",
							5-count));
				}
				sleep(2);
				count++;
				goto begin;
			}
			LOG((PI_DBG_DEV, PI_DBG_LVL_ERR,
					"\n\n   Device not found on %s, \
					Did you hit HotSync?\n\n", realport));	
		} else if (errno == EISDIR) {
			LOG((PI_DBG_DEV, PI_DBG_LVL_ERR,
					" The port specified must"
					" contain a device name, and %s was"
					" a directory.\n"
					"   Please change that to reference a"
					" real device, and try"
					" again\n\n", pa->pi_device));
		}
#endif
		return err;
	}
	ps->raddr 	= malloc(addrlen);
	memcpy(ps->raddr, addr, addrlen);
	ps->raddrlen 	= addrlen;
	ps->laddr 	= malloc(addrlen);
	memcpy(ps->laddr, addr, addrlen);
	ps->laddrlen 	= addrlen;

	return 0;
}

/***********************************************************************
 *
 * Function:    pi_serial_listen
 *
 * Summary:     Prepare for incoming connections
 *
 * Parameters:  pi_socket*, backlog
 *
 * Returns:     0 for success, negative otherwise
 *
 ***********************************************************************/
static int pi_serial_listen(pi_socket_t *ps, int backlog)
{
	int 	result;
	struct 	pi_serial_data *data =
		(struct pi_serial_data *)ps->device->data;
	
debug(1, "XXX", "pi_serial_listen");
	/* ps->rate has been set by bind */
	result = data->impl.changebaud(ps);
debug(1, "XXX", "pi_serial_listen changebaud: %d", result);
	if (result == 0)
		ps->state = PI_SOCK_LISTEN;
	
	return result;
}

/***********************************************************************
 *
 * Function:    pi_serial_accept
 *
 * Summary:     Accept an incoming connection
 *
 * Parameters:  pi_socket*, sockaddr*
 *
 * Returns:     Nothing
 *
 ***********************************************************************/
static int
pi_serial_accept(pi_socket_t *ps, struct sockaddr *addr,
	size_t *addrlen)
{
	struct 	pi_serial_data *data =
		(struct pi_serial_data *)ps->device->data;
	size_t 	size;
	int	err;

	/* Wait for data */
#ifdef linux
	if (ps->accept_to) {
		/* shield against losing the first packet */
		int result = data->impl.poll(ps, 1000);
		LOG((PI_DBG_DEV, PI_DBG_LVL_DEBUG, "%s: %d, poll result: %d.\n", __FILE__, __LINE__, result));

		if (result < 0) {
			unsigned char buf[] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 };
			data->impl.write(ps, buf, sizeof (buf), 1000);
		}
	}
#endif
	if ((err = data->impl.poll(ps, ps->accept_to * 1000)) < 0)
		goto fail;

	data->timeout = ps->accept_to * 1000;

	pi_socket_init(ps);
	if (ps->type == PI_SOCK_STREAM) {
		struct timeval tv;
		unsigned char cmp_flags;

		switch (ps->cmd) {
			case PI_CMD_CMP:
				if ((err = cmp_rx_handshake(ps, data->establishrate, data->establishhighrate)) < 0)
					goto fail;
				
				/* propagate the long packet format flag to both command and non-command stacks */
				size = sizeof(cmp_flags);
				pi_getsockopt(ps->sd, PI_LEVEL_CMP, PI_CMP_FLAGS, &cmp_flags, &size);	
				if (cmp_flags & CMP_FL_LONG_PACKET_SUPPORT) {
					int use_long_format = 1;
					size = sizeof(int);
					pi_setsockopt(ps->sd, PI_LEVEL_PADP, PI_PADP_USE_LONG_FORMAT,
						      &use_long_format, &size);
					ps->command ^= 1;
					pi_setsockopt(ps->sd, PI_LEVEL_PADP, PI_PADP_USE_LONG_FORMAT,
						      &use_long_format, &size);
					ps->command ^= 1;
				}
					
					/* We always reconfigure our port, no matter what */
				size = sizeof(data->rate);
				pi_getsockopt(ps->sd, PI_LEVEL_CMP, PI_CMP_BAUD, &data->rate, &size);
				if ((err = data->impl.changebaud(ps)) < 0)
					goto fail;
					
				/* Palm device needs some time to reconfigure its port */
				tv.tv_sec 	= 0;
				tv.tv_usec 	= 50000;
				select(0, 0, 0, 0, &tv);
				break;

			case PI_CMD_NET:
				/* serial/network: make sure we don't split writes. set socket option
				 * on both the command and non-command instances of the protocol
				 */
#ifdef MACOSX
				/* We need to turn fragmentation OFF to improve Bluetooth performance
				 * but this code is also used by USB on Linux and Freebsd
				 * therefore, only compile it when running OS X
				 */
				{
					int split = 0;
					size_t chunksize = 0;

					size = sizeof (split);
					pi_setsockopt(ps->sd, PI_LEVEL_NET, PI_NET_SPLIT_WRITES,
						&split, &size);
					size = sizeof (chunksize);
					pi_setsockopt(ps->sd, PI_LEVEL_NET, PI_NET_WRITE_CHUNKSIZE,
						&chunksize, &size);

					ps->command ^= 1;
					size = sizeof (split);
					pi_setsockopt(ps->sd, PI_LEVEL_NET, PI_NET_SPLIT_WRITES,
						&split, &size);
					size = sizeof (chunksize);
					pi_setsockopt(ps->sd, PI_LEVEL_NET, PI_NET_WRITE_CHUNKSIZE,
						&chunksize, &size);
					ps->command ^= 1;
				}
#endif
				if ((err = net_rx_handshake(ps)) < 0)
					goto fail;
				break;
		}
		ps->dlprecord = 0;
	}

	data->timeout = 0;
	ps->command = 0;
	ps->state = PI_SOCK_CONN_ACCEPT;

	return ps->sd;

fail:
	return err;
}


/***********************************************************************
 *
 * Function:    pi_serial_getsockopt
 *
 * Summary:     get options on socket
 *
 * Parameters:  pi_socket*, level, option name, option value, option length
 *
 * Returns:     0 for success, negative otherwise
 *
 ***********************************************************************/
static int
pi_serial_getsockopt(pi_socket_t *ps, int level, int option_name, 
		     void *option_value, size_t *option_len)
{
	struct pi_serial_data *data =
		(struct pi_serial_data *)ps->device->data;

	switch (option_name) {
		case PI_DEV_RATE:
			if (*option_len != sizeof (data->rate))
				goto error;
			memcpy (option_value, &data->rate, sizeof (data->rate));
			break;

		case PI_DEV_ESTRATE:
			if (*option_len != sizeof (data->establishrate))
				goto error;
			memcpy (option_value, &data->establishrate, sizeof (data->establishrate));
			break;

		case PI_DEV_HIGHRATE:
			if (*option_len != sizeof (data->establishhighrate))
				goto error;
			memcpy (option_value, &data->establishhighrate, sizeof (data->establishhighrate));
			break;

		case PI_DEV_TIMEOUT:
			if (*option_len != sizeof (data->timeout))
				goto error;
			memcpy (option_value, &data->timeout, sizeof (data->timeout));
			break;
	}

	return 0;
	
error:
	//errno = EINVAL;
	return pi_set_error(ps->sd, PI_ERR_GENERIC_ARGUMENT);
}


/***********************************************************************
 *
 * Function:    pi_serial_setsockopt
 *
 * Summary:     set options on socket
 *
 * Parameters:  pi_socket*, level, option name, option value, option length
 *
 * Returns:     0 for success, negative otherwise
 *
 ***********************************************************************/
static int
pi_serial_setsockopt(pi_socket_t *ps, int level, int option_name, 
		     const void *option_value, size_t *option_len)
{
	struct pi_serial_data *data =
		(struct pi_serial_data *)ps->device->data;

	/* FIXME: can't change stuff if already connected */
	switch (option_name) {
		case PI_DEV_ESTRATE:
			if (*option_len != sizeof (data->establishrate))
				goto error;
			memcpy (&data->establishrate, option_value, sizeof (data->establishrate));
			break;

		case PI_DEV_HIGHRATE:
			if (*option_len != sizeof (data->establishhighrate))
				goto error;
			memcpy (&data->establishhighrate, option_value, sizeof (data->establishhighrate));
			break;

		case PI_DEV_TIMEOUT:
			if (*option_len != sizeof (data->timeout))
				goto error;
			memcpy (&data->timeout, option_value, sizeof (data->timeout));
			break;
	}

	return 0;
	
 error:
	//errno = EINVAL;
	return pi_set_error(ps->sd, PI_ERR_GENERIC_ARGUMENT);
}


/***********************************************************************
 *
 * Function:    pi_serial_close
 *
 * Summary:     Close a connection, destroy the socket
 *
 * Parameters:  pi_socket*
 *
 * Returns:     always 0 for success
 *
 ***********************************************************************/
static int pi_serial_close(pi_socket_t *ps)
{
	struct pi_serial_data *data =
		(struct pi_serial_data *)ps->device->data;

	if (ps->sd) {
		data->impl.close (ps);
		ps->sd = 0;
	}

	if (ps->laddr) {
		free(ps->laddr);
		ps->laddr = NULL;
	}

	if (ps->raddr) {
		free(ps->raddr);
		ps->raddr = NULL;
	}

	return 0;
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
