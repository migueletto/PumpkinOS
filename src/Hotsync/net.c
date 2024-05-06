/*
 * $Id: net.c,v 1.45 2006/10/12 14:21:22 desrod Exp $
 *
 * net.c: Protocol for NetSync connections
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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "pi-debug.h"
#include "pi-source.h"
#include "pi-net.h"
#include "pi-error.h"

#define PI_NET_TIMEOUT 30*1000

static int net_flush(pi_socket_t *ps, int flags);
static int net_getsockopt(pi_socket_t *ps, int level, int option_name, 
			  void *option_value, size_t *option_len);
static int net_setsockopt(pi_socket_t *ps, int level, int option_name, 
			  const void *option_value, size_t *option_len);

/***********************************************************************
 *
 * Function:    net_protocol_dup
 *
 * Summary:     clones an existing pi_protocol struct
 *
 * Parameters:  pi_protocol_t*
 *
 * Returns:     pi_protocol_t* or NULL if operation failed
 *
 ***********************************************************************/
static pi_protocol_t
*net_protocol_dup (pi_protocol_t *prot)
{
	pi_protocol_t *new_prot = NULL;
	pi_net_data_t	*data = NULL,
			*new_data = NULL;

	ASSERT(prot != NULL);
	
	new_prot = (pi_protocol_t *)malloc (sizeof (pi_protocol_t));
	if (new_prot != NULL) {
		new_data = (pi_net_data_t *)malloc (sizeof (pi_net_data_t));
		if (new_data == NULL) {
			free(new_prot);
			new_prot = NULL;
		}
	}

	if (new_prot != NULL && new_data != NULL) {
		new_prot->level 	= prot->level;
		new_prot->dup 		= prot->dup;
		new_prot->free 		= prot->free;
		new_prot->read 		= prot->read;
		new_prot->write 	= prot->write;
		new_prot->flush		= prot->flush;
		new_prot->getsockopt 	= prot->getsockopt;
		new_prot->setsockopt 	= prot->setsockopt;

		data 			= (pi_net_data_t *)prot->data;
		new_data->type 		= data->type;
		new_data->split_writes	= data->split_writes;
		new_data->write_chunksize	= data->write_chunksize;
		new_data->txid 		= data->txid;
		new_prot->data 		= new_data;
	}

	return new_prot;
}


/***********************************************************************
 *
 * Function:    net_protocol_free
 *
 * Summary:     frees an existing pi_protocol struct
 *
 * Parameters:  pi_protocol*
 *
 * Returns:     void
 *
 ***********************************************************************/
static
void net_protocol_free (pi_protocol_t *prot)
{
	ASSERT (prot != NULL);

	if (prot != NULL) {
		if (prot->data != NULL)
			free(prot->data);
		free(prot);
	}
}


/***********************************************************************
 *
 * Function:    net_protocol
 *
 * Summary:     creates and inits pi_protocol struct instance
 *
 * Parameters:  void
 *
 * Returns:     pi_protocol_t* or NULL if operation failed
 *
 ***********************************************************************/
pi_protocol_t
*net_protocol (void)
{
	pi_protocol_t *prot = NULL;
	pi_net_data_t *data = NULL;

	prot = (pi_protocol_t *)malloc (sizeof (pi_protocol_t));	
	if (prot != NULL) {
		data = (pi_net_data_t *)malloc (sizeof (pi_net_data_t));
		if (data == NULL) {
			free(prot);
			prot = NULL;
		}
	}

	if (prot != NULL && data != NULL) {
		prot->level 		= PI_LEVEL_NET;
		prot->dup 		= net_protocol_dup;
		prot->free 		= net_protocol_free;
		prot->read 		= net_rx;
		prot->write 		= net_tx;
		prot->flush		= net_flush;
		prot->getsockopt 	= net_getsockopt;
		prot->setsockopt 	= net_setsockopt;

		data->type 		= PI_NET_TYPE_DATA;
		data->split_writes	= 1;	    /* write packet header and data separately */
		data->write_chunksize	= 4096;	    /* and push data in 4k chunks. Required for some USB devices */
		data->txid 		= 0x00;
		prot->data 		= data;
	}

	return prot;
}


/***********************************************************************
 *
 * Function:    net_rx_handshake
 *
 * Summary:     RX handshake
 *
 * Parameters:  pi_socket_t*
 *
 * Returns:     0 for success, negative otherwise
 *
 ***********************************************************************/
int
net_rx_handshake(pi_socket_t *ps)
{
	static const unsigned char msg1[] =	/* 50 bytes */
		"\x12\x01\x00\x00\x00\x00\x00\x00\x00\x20\x00\x00\x00"
		"\x24\xff\xff\xff\xff\x3c\x00\x3c\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\xc0\xa8\xa5\x1f\x04\x27\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
	static const unsigned char msg2[] =	/* 46 bytes */
		"\x13\x01\x00\x00\x00\x00\x00\x00\x00\x20\x00\x00\x00"
		"\x20\xff\xff\xff\xff\x00\x3c\x00\x3c\x00\x00\x00\x00"
		"\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00";
	pi_buffer_t *buffer;
	int err;

	buffer = pi_buffer_new (256);
	if (buffer == NULL) {
		errno = ENOMEM;
		return pi_set_error(ps->sd, PI_ERR_GENERIC_MEMORY);
	}

	if ((err = net_rx(ps, buffer, 256, 0)) >= 0  &&
		(err = net_tx(ps, msg1, 50, 0)) >= 0	&&
		(err = net_rx(ps, buffer, 50, 0)) >= 0  &&
		(err = net_tx(ps, msg2, 46, 0)) >= 0	&&
		(err = net_rx(ps, buffer, 8, 0)) >= 0)
	{
		pi_buffer_free (buffer);
		return 0;
	}

	pi_buffer_free (buffer);
	return err;
}


/***********************************************************************
 *
 * Function:    net_tx_handshake
 *
 * Summary:     TX handshake
 *
 * Parameters:  pi_socket_t*
 *
 * Returns:     0 for success, negative otherwise
 *
 ***********************************************************************/
int
net_tx_handshake(pi_socket_t *ps)
{
	static const unsigned char msg1[] =	/* 22 bytes */
		"\x90\x01\x00\x00\x00\x00\x00\x00\x00\x20\x00\x00\x00"
		"\x08\x01\x00\x00\x00\x00\x00\x00\x00";
	static const unsigned char msg2[] =	/* 50 bytes */
		"\x92\x01\x00\x00\x00\x00\x00\x00\x00\x20\x00\x00\x00"
		"\x24\xff\xff\xff\xff\x00\x3c\x00\x3c\x40\x00\x00\x00"
		"\x01\x00\x00\x00\xc0\xa8\xa5\x1e\x04\x01\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
	static const unsigned char msg3[] =	/* 8 bytes */
		"\x93\x00\x00\x00\x00\x00\x00\x00";
	pi_buffer_t *buffer;
	int err;

	buffer = pi_buffer_new (256);
	if (buffer == NULL) {
		errno = ENOMEM;
		return pi_set_error(ps->sd, PI_ERR_GENERIC_MEMORY);
	}
	
	if ((err = net_tx(ps, msg1, 22, 0)) >= 0	&&
		(err = net_rx(ps, buffer, 256, 0)) >= 0  &&
		(err = net_tx(ps, msg2, 50, 0)) >= 0	&&
		(err = net_rx(ps, buffer, 256, 0)) >= 0  &&
		(err = net_tx(ps, msg3, 8, 0)) >= 0)
	{
		pi_buffer_free (buffer);
		return 0;
	}

	pi_buffer_free (buffer);
	return err;
}

/***********************************************************************
 *
 * Function:    net_flush
 *
 * Summary:     Flush input and output buffers
 *
 * Parameters:  pi_socket_t*, flags
 *
 * Returns:     A negative number on error, 0 otherwise
 *
 ***********************************************************************/
int
net_flush(pi_socket_t *ps, int flags)
{
	pi_protocol_t	*prot,
			*next;

	prot = pi_protocol(ps->sd, PI_LEVEL_NET);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	next = pi_protocol_next(ps->sd, PI_LEVEL_NET);
	if (next == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	return next->flush(ps, flags);
}

/***********************************************************************
 *
 * Function:    net_tx
 *
 * Summary:     Transmit NET Packets
 *
 * Parameters:  pi_socket_t*, char* to buf, buf length, flags
 *
 * Returns:     A negative number on error, 0 otherwise
 *
 ***********************************************************************/
ssize_t
net_tx(pi_socket_t *ps, const unsigned char *msg, size_t len, int flags)
{
	int 	bytes,
			offset,
			remain,
			tosend;
	pi_protocol_t	*prot,
			*next;
	pi_net_data_t *data;
	unsigned char *buf;

	prot = pi_protocol(ps->sd, PI_LEVEL_NET);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);
	data = (pi_net_data_t *)prot->data;

	next = pi_protocol_next(ps->sd, PI_LEVEL_NET);
	if (next == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	/* Create the header */
	buf = (unsigned char *) malloc(PI_NET_HEADER_LEN + len);
	if (buf == NULL)
		return pi_set_error(ps->sd, PI_ERR_GENERIC_MEMORY);
	buf[PI_NET_OFFSET_TYPE] = data->type;
	if (data->type == PI_NET_TYPE_TCKL)
		buf[PI_NET_OFFSET_TXID] = 0xff;
	else
		buf[PI_NET_OFFSET_TXID] = data->txid;
	set_long(&buf[PI_NET_OFFSET_SIZE], len);
	memcpy(&buf[PI_NET_HEADER_LEN], msg, len);

	/* Write the header and body, possibly in one write, or in two,
	 * or in more, depending on the current options. Crucial options
	 * here are `split_writes' and `write_chunksize' in this protocol's
	 * data (use net_setsockopt() to set them).
	 */
	if (data->split_writes)
	{
		/* Bugfix for USB send problems. If connected over
		 * USB, do the following:
		 * - send the 6 bytes of header first
		 * - split the rest of data into chunks if the write_chunksize opt is set
		 * This is what Palm Desktop does on Windows for the Zire 72
		 * (uses split writes and 4k chunks)
		 * -- FP
		 */
		bytes = next->write(ps, buf, PI_NET_HEADER_LEN, flags);
		if (bytes < PI_NET_HEADER_LEN)
		{
			free(buf);
			return bytes;
		}
		offset = PI_NET_HEADER_LEN;
		remain = len;
	}
	else
	{
		offset = 0;
		remain = PI_NET_HEADER_LEN + len;
	}

	while (remain > 0)
	{
		if (data->write_chunksize)
			tosend = (remain > data->write_chunksize) ? data->write_chunksize : remain;
		else
			tosend = remain;

		bytes = next->write(ps, &buf[offset], tosend, flags);
		if (bytes < tosend)
		{
			free(buf);
			return bytes;
		}
		remain -= bytes;
		offset += bytes;
	}

	CHECK(PI_DBG_NET, PI_DBG_LVL_INFO, net_dump_header(buf, 1, ps->sd));
	CHECK(PI_DBG_NET, PI_DBG_LVL_DEBUG, pi_dumpdata((char *)msg, len));
	
	free(buf);
	return len;
}

/***********************************************************************
 *
 * Function:    net_rx
 *
 * Summary:     Receive NET Packets
 *
 * Parameters:  ps	--> socket to read from
 *		msg	<-- malloc()ed buffer containing the data
 *		len	--> unused
 *		flags   --> unused
 *
 * Returns:     A negative number on error, 0 on timeout, otherwise the
 *              length of the received packet.
 *
 ***********************************************************************/
ssize_t
net_rx(pi_socket_t *ps, pi_buffer_t *msg, size_t len, int flags)
{
	int 	bytes, 
		total_bytes, 
		packet_len,
		timeout,
		honor_rx_timeout;
	size_t	size;
	pi_protocol_t	*prot,
			*next;
	pi_buffer_t *header;
	pi_net_data_t *data;

	prot = pi_protocol(ps->sd, PI_LEVEL_NET);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);
	
	data = (pi_net_data_t *)prot->data;
	next = pi_protocol_next(ps->sd, PI_LEVEL_NET);
	if (next == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	size = sizeof(honor_rx_timeout);
	pi_getsockopt(ps->sd, PI_LEVEL_SOCK, PI_SOCK_HONOR_RX_TIMEOUT,
		&honor_rx_timeout, &size);

	timeout = honor_rx_timeout ? PI_NET_TIMEOUT : 0;
	size = sizeof(timeout);
	pi_setsockopt(ps->sd, PI_LEVEL_DEV, PI_DEV_TIMEOUT, 
		      &timeout, &size);

	header = pi_buffer_new (PI_NET_HEADER_LEN);
	if (header == NULL) {
		errno = ENOMEM;
		return pi_set_error(ps->sd, PI_ERR_GENERIC_MEMORY);
	}

	/* loop until we find a non-tickle packet (if the other end
	   sends us a tickle, we would receive it prior to getting
	   the expected reply to one of our commands, so we need
	   to make sure tickle packets don't get in the way) */
	total_bytes = 0;
	while (!total_bytes) {
		if (data->txid == 0) {	
			/* Peek to see if it is a headerless packet */
			bytes = next->read(ps, header, 1, flags);
			if (bytes <= 0) {
				pi_buffer_free (header);
				return bytes;
			}
			
			LOG ((PI_DBG_NET, PI_DBG_LVL_INFO,
				  "NET RX (%i): Checking for headerless packet %d\n",
				  ps->sd, header->data[0]));
			
			if (header->data[0] == 0x90) {
				/* Cause the header bytes to be skipped */
				LOG ((PI_DBG_NET, PI_DBG_LVL_INFO,
					  "NET RX (%i): Headerless packet\n",
					  ps->sd));
				total_bytes = PI_NET_HEADER_LEN;
				header->data[PI_NET_OFFSET_TYPE] = PI_NET_TYPE_DATA;
				header->data[PI_NET_OFFSET_TXID] = 0x01;
				set_long (&header->data[PI_NET_OFFSET_SIZE], 21);
				break;
			} else {
				total_bytes += bytes;
			}
		}
		
		/* bytes in what's left of the header */
		while (total_bytes < PI_NET_HEADER_LEN) {
			bytes = next->read(ps, header,
					(size_t)(PI_NET_HEADER_LEN - total_bytes), flags);
			if (bytes <= 0) {
				pi_buffer_free (header);
				return bytes;
			}
			total_bytes += bytes;
		}
		
		packet_len = get_long(&header->data[PI_NET_OFFSET_SIZE]);
		data->type = header->data[PI_NET_OFFSET_TYPE];

		switch (data->type) {
			case PI_NET_TYPE_TCKL:
				if (packet_len != 0) {
					LOG ((PI_DBG_NET, PI_DBG_LVL_ERR,
						"NET RX (%i): tickle packet with non-zero length\n",
						ps->sd));
					pi_buffer_free(header);
					return pi_set_error(ps->sd, PI_ERR_PROT_BADPACKET);
				}
				/* valid tickle packet; continue reading. */
				LOG((PI_DBG_NET, PI_DBG_LVL_DEBUG,
					"NET RX (%i): received tickle packet\n",
					ps->sd));
				total_bytes = 0;
				header->used = 0;
				break;
			
			case PI_NET_TYPE_DATA:
				/* move on to reading the rest of the packet */
				break;

			default:
				LOG ((PI_DBG_NET, PI_DBG_LVL_ERR,
					"NET RX (%i): Unknown packet type\n",
					ps->sd));
				CHECK(PI_DBG_NET, PI_DBG_LVL_INFO, pi_dumpdata((char *)header->data, PI_NET_HEADER_LEN));
				pi_buffer_free(header);
				return pi_set_error(ps->sd, PI_ERR_PROT_BADPACKET);
		}
	}

	total_bytes = 0;
	packet_len = get_long(&header->data[PI_NET_OFFSET_SIZE]);

	/* shield against absurd packet lengths */
	if (packet_len < 0 || packet_len > 0x100000L) {
		/* we see an invalid packet */
		next->flush(ps, PI_FLUSH_INPUT);
		LOG ((PI_DBG_NET, PI_DBG_LVL_ERR, "NET RX (%i): Invalid packet length (%ld)\n", ps->sd, packet_len));
		pi_buffer_free(header);
		return pi_set_error(ps->sd, PI_ERR_PROT_BADPACKET);
	}

	/* read the actual packet data */
	while (total_bytes < packet_len) {
		bytes = next->read(ps, msg,
			(size_t)(packet_len - total_bytes), flags);
		if (bytes < 0) {
			pi_buffer_free (header);
			return bytes;
		}
		total_bytes += bytes;
	}

	CHECK(PI_DBG_NET, PI_DBG_LVL_INFO, net_dump_header(header->data, 0, ps->sd));
	CHECK(PI_DBG_NET, PI_DBG_LVL_DEBUG, net_dump(header->data, msg->data));

	/* Update the transaction id */
	if (ps->state == PI_SOCK_CONN_INIT || ps->command == 1)
		data->txid = header->data[PI_NET_OFFSET_TXID];
	else {
		data->txid++;
		if (data->txid == 0xff)
			data->txid = 1;
	}

	pi_buffer_free (header);
	return packet_len;
}

/***********************************************************************
 *
 * Function:    net_getsockopt
 *
 * Summary:     get options on socket
 *
 * Parameters:  pi_socket*, level, option name, option value, option length
 *
 * Returns:     0 for success, negative otherwise
 *
 ***********************************************************************/
static int
net_getsockopt(pi_socket_t *ps, int level, int option_name, 
	       void *option_value, size_t *option_len)
{
	pi_protocol_t *prot;
	pi_net_data_t *data;

	prot = pi_protocol(ps->sd, PI_LEVEL_NET);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	data = (pi_net_data_t *)prot->data;

	switch (option_name) {
		case PI_NET_TYPE:
			if (*option_len != sizeof (data->type)) {
				errno = EINVAL;
				return pi_set_error(ps->sd, PI_ERR_GENERIC_ARGUMENT);
			}
			memcpy (option_value, &data->type,
				sizeof (data->type));
			*option_len = sizeof (data->type);
			break;
	}

	return 0;
}


/***********************************************************************
 *
 * Function:    net_setsockopt
 *
 * Summary:     set options on socket
 *
 * Parameters:  pi_socket*, level, option name, option value, option length
 *
 * Returns:     0 for success, negative otherwise
 *
 ***********************************************************************/
static int
net_setsockopt(pi_socket_t *ps, int level, int option_name, 
	       const void *option_value, size_t *option_len)
{
	pi_protocol_t *prot;
	pi_net_data_t *data;

	prot = pi_protocol(ps->sd, PI_LEVEL_NET);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	data = (pi_net_data_t *)prot->data;

	switch (option_name) {
		case PI_NET_TYPE:
			if (*option_len != sizeof (data->type)) {
				errno = EINVAL;
				return pi_set_error(ps->sd, PI_ERR_GENERIC_ARGUMENT);
			}
			memcpy (&data->type, option_value,
				sizeof (data->type));
			break;
		
		/* this option, when set to != 0, instructs NET to separately
		 * write the NET header and the data block. Data can be further
		 * sent in chunks by also setting PI_NET_WRITE_CHUNKSIZE below.
		 */
		case PI_NET_SPLIT_WRITES:
			if (*option_len != sizeof (data->split_writes)) {
				errno = EINVAL;
				return pi_set_error(ps->sd, PI_ERR_GENERIC_ARGUMENT);
			}
			memcpy (&data->split_writes, option_value,
				sizeof(data->split_writes));
			break;

		/* this option, when set to != 0, instructs NET to write the
		 * packet data in chunks of the given maximum size. If
		 * PI_NET_SPLIT_WRITES is not set, and this option is set, we
		 * chunk the whole write (including the NET header)
		 */
		case PI_NET_WRITE_CHUNKSIZE:
			if (*option_len != sizeof (data->write_chunksize)) {
				errno = EINVAL;
				return pi_set_error(ps->sd, PI_ERR_GENERIC_ARGUMENT);
			}
			memcpy (&data->write_chunksize, option_value,
				sizeof(data->write_chunksize));
			break;
	}

	return 0;
}

/***********************************************************************
 *
 * Function:    net_dump_header
 *
 * Summary:     Dump the NET packet header
 *
 * Parameters:  char* to net packet, TX boolean
 *
 * Returns:     void
 *
 ***********************************************************************/
void
net_dump_header(unsigned char *data, int rxtx, int sd)
{
	LOG((PI_DBG_NET, PI_DBG_LVL_NONE,
	    "NET %s sd=%i type=%d txid=0x%.2x len=0x%.4x\n",
	    rxtx ? "TX" : "RX",
	    sd,
	    get_byte(&data[PI_NET_OFFSET_TYPE]),
	    get_byte(&data[PI_NET_OFFSET_TXID]),
	    get_long(&data[PI_NET_OFFSET_SIZE])));
}


/***********************************************************************
 *
 * Function:    net_dump
 *
 * Summary:     Dump the NET packet
 *
 * Parameters:  char* to net packet
 *
 * Returns:     void
 *
 ***********************************************************************/
void
net_dump(unsigned char *header, unsigned char *data)
{
	size_t 	size;

	size = get_long(&header[PI_NET_OFFSET_SIZE]);
	pi_dumpdata((char *)data, size);
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
