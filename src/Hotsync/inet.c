/*
 * $Id: inet.c,v 1.61 2006/10/12 14:21:22 desrod Exp $
 *
 * inet.c: Interface layer to TCP/IP NetSync connections
 *
 * Copyright (c) 1997, Kenneth Albanowski
 * Copyright (c) 1999, Tilo Christ
 * Copyright (c) 1999, John Franks
 * Copyright (c) 2004, 2005 Florent Pillet
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
 * -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>

#include "pi-debug.h"
#include "pi-source.h"
#include "pi-inet.h"
#include "pi-cmp.h"
#include "pi-net.h"

#include "sys.h"
#include "debug.h"

/* Declare prototypes */
static void pi_inet_device_free (pi_device_t *dev);
static pi_protocol_t* pi_inet_protocol (pi_device_t *dev);
static pi_protocol_t* pi_inet_protocol_dup (pi_protocol_t *prot);
static void pi_inet_protocol_free (pi_protocol_t *prot);
static int pi_inet_close(pi_socket_t *ps);
static int pi_inet_connect(pi_socket_t *ps, struct sockaddr *addr, size_t addrlen);
static int pi_inet_bind(pi_socket_t *ps, struct sockaddr *addr, size_t addrlen);
static int pi_inet_listen(pi_socket_t *ps, int backlog);
static int pi_inet_accept(pi_socket_t *ps, struct sockaddr *addr, size_t *addrlen);
static ssize_t pi_inet_read(pi_socket_t *ps, pi_buffer_t *msg, size_t len, int flags);
static ssize_t pi_inet_write(pi_socket_t *ps, const unsigned char *msg, size_t len, int flags);
static int pi_inet_getsockopt(pi_socket_t *ps, int level, int option_name, void *option_value, size_t *option_len);
static int pi_inet_setsockopt(pi_socket_t *ps, int level, int option_name, const void *option_value, size_t *option_len);
static int pi_inet_flush(pi_socket_t *ps, int flags);

extern int pi_socket_init(pi_socket_t *ps);

pi_device_t*
pi_inet_device (int type) 
{
	pi_device_t *dev = NULL;
	pi_inet_data_t *data = NULL;

	dev = (pi_device_t *)malloc (sizeof (pi_device_t));
	if (dev != NULL) {
		data = (pi_inet_data_t *)malloc (sizeof (pi_inet_data_t));
		if (data == NULL) {
			free(dev);
			dev = NULL;
		}
	}

	if (dev != NULL && data != NULL) {
		dev->free 	= pi_inet_device_free;
		dev->protocol 	= pi_inet_protocol;	
		dev->bind 	= pi_inet_bind;
		dev->listen 	= pi_inet_listen;
		dev->accept 	= pi_inet_accept;
		dev->connect 	= pi_inet_connect;
		dev->close 	= pi_inet_close;

		data->timeout 	= 0;
		data->rx_bytes 	= 0;
		data->rx_errors	= 0;
		data->tx_bytes 	= 0;
		data->tx_errors	= 0;
		dev->data 	= data;
	}

	return dev;
}

static void
pi_inet_device_free (pi_device_t *dev)
{
	ASSERT (dev != NULL);
	if (dev != NULL) {
		if (dev->data != NULL)
			free(dev->data);
		free(dev);
	}
}

static pi_protocol_t*
pi_inet_protocol (pi_device_t *dev)
{	
	pi_protocol_t *prot;
	//pi_inet_data_t *data;

	ASSERT (dev != NULL);
	
	//data = dev->data;
	
	prot = (pi_protocol_t *)malloc (sizeof (pi_protocol_t));

	if (prot != NULL) {
		prot->level 		= PI_LEVEL_DEV;
		prot->dup 		= pi_inet_protocol_dup;
		prot->free 		= pi_inet_protocol_free;
		prot->read 		= pi_inet_read;
		prot->write 		= pi_inet_write;
		prot->flush		= pi_inet_flush;
		prot->getsockopt 	= pi_inet_getsockopt;
		prot->setsockopt 	= pi_inet_setsockopt;
		prot->data = NULL;
	}
	
	return prot;
}

static pi_protocol_t*
pi_inet_protocol_dup (pi_protocol_t *prot)
{
	pi_protocol_t *new_prot;

	ASSERT (prot != NULL);

	new_prot = (pi_protocol_t *)malloc (sizeof (pi_protocol_t));

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

static void
pi_inet_protocol_free (pi_protocol_t *prot)
{
	ASSERT (prot != NULL);
	if (prot != NULL)
		free(prot);
}

static int
pi_inet_bind(pi_socket_t *ps, struct sockaddr *addr, size_t addrlen)
{
	int 	opt,
		sd,
		err;
	size_t	optlen;
	struct 	pi_sockaddr *paddr = (struct pi_sockaddr *) addr;
	struct 	sockaddr_in serv_addr;
	char 	*device = paddr->pi_device, 
		*port	= NULL;

	/* Figure out the addresses to allow */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	if (strlen(device) > 1 && strncmp(device, "any", 3)) {
		serv_addr.sin_addr.s_addr = inet_addr(device);
		if (serv_addr.sin_addr.s_addr == (in_addr_t)-1) {
			struct hostent *hostent = gethostbyname(device);

			if (!hostent)
				return pi_set_error(ps->sd, PI_ERR_GENERIC_SYSTEM);

			memcpy((char *) &serv_addr.sin_addr.s_addr,
				   hostent->h_addr, (size_t)hostent->h_length);
		}
	} else {
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	if ((port = strchr(device, ':')) != NULL) {
		serv_addr.sin_port = htons(atoi(++port));
	} else {
		serv_addr.sin_port = htons(14238);
	}

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd < 0) {
		LOG((PI_DBG_DEV, PI_DBG_LVL_ERR, 
			"DEV BIND Inet: Unable to create socket\n"));
		return pi_set_error(ps->sd, PI_ERR_GENERIC_SYSTEM);
	}	
debug(1, "XXX", "pi_inet_bind fd %d -> %d", ps->sd, sd);
	if ((err = pi_socket_setsd (ps, sd)) < 0)
		return err;

	opt = 1;
	optlen = sizeof(opt);

	if (setsockopt(ps->sd, SOL_SOCKET, SO_REUSEADDR, (void *) &opt,
			(int)optlen) < 0) {
		return pi_set_error(ps->sd, PI_ERR_GENERIC_SYSTEM);
	}

	if (bind(ps->sd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		return pi_set_error(ps->sd, PI_ERR_GENERIC_SYSTEM);

	LOG((PI_DBG_DEV, PI_DBG_LVL_INFO,
		"DEV BIND Inet Bound to %s\n", device));

	ps->raddr 	= malloc(addrlen);
	memcpy(ps->raddr, addr, addrlen);
	ps->raddrlen 	= addrlen;
	ps->laddr 	= malloc(addrlen);
	memcpy(ps->laddr, addr, addrlen);
	ps->laddrlen 	= addrlen;

	return 0;
}

static int
pi_inet_connect(pi_socket_t *ps, struct sockaddr *addr, size_t addrlen)
{
	int 	sd,
		err;

	struct 	pi_sockaddr *paddr = (struct pi_sockaddr *) addr;
	struct 	sockaddr_in serv_addr;
	char 	*device = paddr->pi_device;
	
	/* Figure out the addresses to allow */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	if (strlen(device) > 1) {
		serv_addr.sin_addr.s_addr = inet_addr(device);
		if (serv_addr.sin_addr.s_addr == (in_addr_t)-1) {
			struct hostent *hostent = gethostbyname(device);
		
			if (!hostent) {
				LOG((PI_DBG_DEV, PI_DBG_LVL_ERR, 
					"DEV CONNECT Inet: Unable"
					" to determine host\n"));
				return pi_set_error(ps->sd, PI_ERR_GENERIC_SYSTEM);
			}
			
			memcpy((char *) &serv_addr.sin_addr.s_addr,
				   hostent->h_addr, (size_t)hostent->h_length);
		}
	} else {
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	serv_addr.sin_port = htons(14238);

	sd = socket(AF_INET, SOCK_STREAM, 0);

	if (sd < 0) {
		LOG((PI_DBG_DEV, PI_DBG_LVL_ERR, 
			"DEV CONNECT Inet: Unable to create socket\n"));
		return pi_set_error(ps->sd, PI_ERR_GENERIC_SYSTEM);
	}

	if ((err = pi_socket_setsd (ps, sd)) < 0)
		return err;

	if (connect (ps->sd, (struct sockaddr *) &serv_addr,
			 sizeof(serv_addr)) < 0) {
		LOG((PI_DBG_DEV, PI_DBG_LVL_ERR, 
			"DEV CONNECT Inet: Unable to connect\n"));
		return pi_set_error(ps->sd, PI_ERR_GENERIC_SYSTEM);
	}

	ps->raddr 	= malloc(addrlen);
	memcpy(ps->raddr, addr, addrlen);
	ps->raddrlen 	= addrlen;
	ps->laddr 	= malloc(addrlen);
	memcpy(ps->laddr, addr, addrlen);
	ps->laddrlen 	= addrlen;

	switch (ps->cmd) {
		case PI_CMD_CMP:
			if ((err = cmp_tx_handshake(ps)) < 0)
				goto fail;
			break;
		case PI_CMD_NET:
			if ((err = net_tx_handshake(ps)) < 0)
				goto fail;
			break;
	}
	ps->state = PI_SOCK_CONN_INIT;
	ps->command = 0;

	LOG((PI_DBG_DEV, PI_DBG_LVL_INFO, "DEV CONNECT Inet: Connected\n"));
	return 0;

fail:
	return err;
}

static int
pi_inet_listen(pi_socket_t *ps, int backlog)
{
	int 	result;
	
	result = listen(ps->sd, backlog);
	if (result == 0)
		ps->state = PI_SOCK_LISTEN;

	return result;
}

static int
pi_inet_accept(pi_socket_t *ps, struct sockaddr *addr, size_t *addrlen)
{
	int	sd,
		err,
		split = 0,
		chunksize = 0;
	size_t	len,
		size;
	pl_socklen_t l = 0;
	unsigned char cmp_flags;
	struct 	timeval t;
	fd_set 	ready;
    int r;
	
	if (addrlen)
		l = *addrlen;
    if (ps->accept_to < 0) {
 	    sd = accept(ps->sd, addr, &l);
    } else {
	    FD_ZERO(&ready);
	    FD_SET(ps->sd, &ready);
		t.tv_sec 	= ps->accept_to / 1000;
		t.tv_usec 	= (ps->accept_to % 1000) * 1000;
		if ((r = select(ps->sd + 1, &ready, 0, 0, &t)) == -1) {
            sd = -1;
        } else if (r == 1) {
 	        sd = accept(ps->sd, addr, &l);
        } else {
            // timeout
		    pi_set_error(ps->sd, -1);
		    err = PI_ERR_SOCK_TIMEOUT;
		    goto fail;
        }
    }
	if (sd < 0) {
		pi_set_error(ps->sd, sd);
		err = PI_ERR_GENERIC_SYSTEM;
		goto fail;
	}
	if (addrlen) *addrlen = l;

	pi_socket_setsd(ps, sd);
	pi_socket_init(ps);

	switch (ps->cmd) {
		case PI_CMD_CMP:
			if ((err = cmp_rx_handshake(ps, 57600, 0)) < 0)
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

			break;
		case PI_CMD_NET:
			/* network: make sure we don't split writes. set socket option
			 * on both the command and non-command instances of the protocol
			 */
			len = sizeof (split);
			pi_setsockopt(ps->sd, PI_LEVEL_NET, PI_NET_SPLIT_WRITES,
				&split, &len);
			len = sizeof (chunksize);
			pi_setsockopt(ps->sd, PI_LEVEL_NET, PI_NET_WRITE_CHUNKSIZE,
				&chunksize, &len);

			ps->command ^= 1;
			len = sizeof (split);
			pi_setsockopt(ps->sd, PI_LEVEL_NET, PI_NET_SPLIT_WRITES,
				&split, &len);
			len = sizeof (chunksize);
			pi_setsockopt(ps->sd, PI_LEVEL_NET, PI_NET_WRITE_CHUNKSIZE,
				&chunksize, &len);
			ps->command ^= 1;

			if ((err = net_rx_handshake(ps)) < 0)
				goto fail;
			break;
	}

	ps->state 	= PI_SOCK_CONN_ACCEPT;
	ps->command 	= 0;
	ps->dlprecord = 0;

	LOG((PI_DBG_DEV, PI_DBG_LVL_INFO, "DEV INET ACCEPT accepted\n"));

	return ps->sd;

 fail:
	return err;
}

static int
pi_inet_close(pi_socket_t *ps)
{
	if (ps->sd) {
		close(ps->sd);
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

static int
pi_inet_flush(pi_socket_t *ps, int flags)
{
	char buf[256];
	int fl;

	if (flags & PI_FLUSH_INPUT) {
		if ((fl = fcntl(ps->sd, F_GETFL, 0)) != -1) {
			fcntl(ps->sd, F_SETFL, fl | O_NONBLOCK);
			while (recv(ps->sd, buf, sizeof(buf), 0) > 0)
				;
			fcntl(ps->sd, F_SETFL, fl);
		}
	}
	return 0;
}

static ssize_t
pi_inet_write(pi_socket_t *ps, const unsigned char *msg, size_t len, int flags)
{
	int 	total,
		nwrote;
	pi_inet_data_t *data = (pi_inet_data_t *)ps->device->data;
	struct 	timeval t;
	fd_set 	ready;

	FD_ZERO(&ready);
	FD_SET(ps->sd, &ready);

	total = len;
	while (total > 0) {
		if (data->timeout == 0) {
			if (select(ps->sd + 1, 0, &ready, 0, 0) < 0
				&& errno == EINTR)
				continue;
		} else {
			t.tv_sec 	= data->timeout / 1000;
			t.tv_usec 	= (data->timeout % 1000) * 1000;
			if (select(ps->sd + 1, 0, &ready, 0, &t) == 0)
				return pi_set_error(ps->sd, PI_ERR_SOCK_TIMEOUT);
		}
		if (!FD_ISSET(ps->sd, &ready)) {
			ps->state = PI_SOCK_CONN_BREAK;
			return pi_set_error(ps->sd, PI_ERR_SOCK_DISCONNECTED);
		}

		nwrote = write(ps->sd, msg, len);
		if (nwrote < 0) {
			/* test errno to properly set the socket error */
			if (errno == EPIPE || errno == EBADF) {
				ps->state = PI_SOCK_CONN_BREAK;
				return pi_set_error(ps->sd, PI_ERR_SOCK_DISCONNECTED);
			}
			return pi_set_error(ps->sd, PI_ERR_SOCK_IO);
		}

		total -= nwrote;
	}
	data->tx_bytes += len;

	LOG((PI_DBG_DEV, PI_DBG_LVL_INFO, "DEV TX Inet Bytes: %d\n", len));

	return len;
}

static ssize_t
pi_inet_read(pi_socket_t *ps, pi_buffer_t *msg, size_t len, int flags)
{
	int 	r, 
		fl 	= 0;
	pi_inet_data_t *data = (pi_inet_data_t *)ps->device->data;
	fd_set 	ready;
	struct 	timeval t;

	if (pi_buffer_expect (msg, len) == NULL) {
		errno = ENOMEM;
		return pi_set_error(ps->sd, PI_ERR_GENERIC_MEMORY);
	}

	if (flags == PI_MSG_PEEK)
		fl = MSG_PEEK;
	
	FD_ZERO(&ready);
	FD_SET(ps->sd, &ready);

	/* If timeout == 0, wait forever for packet, otherwise wait till
	   timeout milliseconds */
	if (data->timeout == 0)
		select(ps->sd + 1, &ready, 0, 0, 0);
	else {
		t.tv_sec 	= data->timeout / 1000;
		t.tv_usec 	= (data->timeout % 1000) * 1000;
		if (select(ps->sd + 1, &ready, 0, 0, &t) == 0)
			return pi_set_error(ps->sd, PI_ERR_SOCK_TIMEOUT);
	}

	/* If data is available in time, read it */
	if (FD_ISSET(ps->sd, &ready)) {
		r = recv(ps->sd, msg->data + msg->used, len, fl);
		if (r < 0) {
			if (errno == EPIPE || errno == EBADF) {
				ps->state = PI_SOCK_CONN_BREAK;
				return pi_set_error(ps->sd, PI_ERR_SOCK_DISCONNECTED);
			}
			return pi_set_error(ps->sd, PI_ERR_SOCK_IO);
		}

		data->rx_bytes += r;
		msg->used += r;

		LOG((PI_DBG_DEV, PI_DBG_LVL_INFO, "DEV RX Inet Bytes: %d\n", r));
		return r;
	}

	/* otherwise throw out any current packet and return */
	LOG((PI_DBG_DEV, PI_DBG_LVL_WARN, "DEV RX Inet timeout\n"));
	data->rx_errors++;
	return 0;
}

static int
pi_inet_getsockopt(pi_socket_t *ps, int level, int option_name, 
		   void *option_value, size_t *option_len)
{
	pi_inet_data_t *data = (pi_inet_data_t *)ps->device->data;

	switch (option_name) {
		case PI_DEV_TIMEOUT:
			if (*option_len != sizeof (data->timeout)) {
				errno = EINVAL;
				return pi_set_error(ps->sd, PI_ERR_GENERIC_ARGUMENT);
			}
			memcpy (option_value, &data->timeout,
				sizeof (data->timeout));
			*option_len = sizeof (data->timeout);
			break;
	}

	return 0;
}

static int
pi_inet_setsockopt(pi_socket_t *ps, int level, int option_name, 
		   const void *option_value, size_t *option_len)
{
	pi_inet_data_t *data = (pi_inet_data_t *)ps->device->data;

	switch (option_name) {
		case PI_DEV_TIMEOUT:
			if (*option_len != sizeof (data->timeout)) {
				errno = EINVAL;
				return pi_set_error(ps->sd, PI_ERR_GENERIC_ARGUMENT);
			}
			memcpy (&data->timeout, option_value,
				sizeof (data->timeout));
			break;
	}

	return 0;
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
