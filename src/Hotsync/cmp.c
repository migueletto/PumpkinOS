/*
 * $Id: cmp.c,v 1.41 2006/11/07 21:13:24 adridg Exp $
 *
 * cmp.c:  Pilot Connection Management Protocol
 *
 * Copyright (c) 1996, Kenneth Albanowski.
 * Copyright (c) 1999, Tilo Christ
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

#include <stdio.h>

#include "pi-debug.h"
#include "pi-source.h"
#include "pi-socket.h"
#include "pi-padp.h"
#include "pi-cmp.h"
#include "pi-error.h"

/* Declare prototypes */
static int cmp_flush(pi_socket_t *ps, int flags);
static int cmp_getsockopt(pi_socket_t *ps, int level, int option_name, 
			  void *option_value, size_t *option_len);
static int cmp_setsockopt(pi_socket_t *ps, int level, int option_name, 
			  const void *option_value, size_t *option_len);

static pi_protocol_t *cmp_protocol_dup (pi_protocol_t *prot);
static void cmp_protocol_free (pi_protocol_t *prot);

/* Protocol Functions */
/***********************************************************************
 *
 * Function:    cmp_protocol_dup
 *
 * Summary:     clones an existing pi_protocol struct 
 *
 * Parameters:  pi_protocol_t*
 *
 * Returns:     pi_protocol_t* or NULL if operation failed
 *
 ***********************************************************************/
static pi_protocol_t*
cmp_protocol_dup (pi_protocol_t *prot)
{
	pi_protocol_t *new_prot;

	struct	pi_cmp_data	 *data,
				 *new_data;
	
	new_prot = (pi_protocol_t *)malloc (sizeof (pi_protocol_t));
	new_data = (struct pi_cmp_data *)malloc (sizeof (struct pi_cmp_data));

	if ( (new_prot != NULL) && (new_data != NULL) ) {
		new_prot->level 	= prot->level;
		new_prot->dup 		= prot->dup;
		new_prot->free 		= prot->free;
		new_prot->read 		= prot->read;
		new_prot->write 	= prot->write;
		new_prot->flush		= prot->flush;
		new_prot->getsockopt 	= prot->getsockopt;
		new_prot->setsockopt 	= prot->setsockopt;

		data = (struct pi_cmp_data *)prot->data;
		new_data->type 		= data->type;
		new_data->flags 	= data->flags;
		new_data->version 	= data->version;
		new_data->baudrate 	= data->baudrate;

		new_prot->data 		= new_data;

	} else if (new_prot != NULL) {
		free(new_prot);
		new_prot = NULL;
	} else if (new_data != NULL) {
		free(new_data);
		new_data = NULL;
	}

	return new_prot;
}


/***********************************************************************
 *
 * Function:    cmp_protocol_free
 *
 * Summary:     frees an existing pi_protocol struct
 *
 * Parameters:  pi_protocol_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
static void
cmp_protocol_free (pi_protocol_t *prot)
{
	if (prot != NULL) {
		if (prot->data != NULL)
			free(prot->data);
		free(prot);
	}
}


/***********************************************************************
 *
 * Function:    cmp_protocol
 *
 * Summary:     creates and inits pi_protocol struct instance
 *
 * Parameters:  void
 *
 * Returns:     pi_protocol_t* or NULL if operation failed
 *
 ***********************************************************************/
pi_protocol_t*
cmp_protocol (void)
{
	pi_protocol_t *prot;
	struct 	pi_cmp_data *data;

	prot = (pi_protocol_t *)malloc (sizeof (pi_protocol_t));	
	data = (struct pi_cmp_data *)malloc (sizeof (struct pi_cmp_data));

	if (prot != NULL && data != NULL) {
		prot->level 		= PI_LEVEL_CMP;
		prot->dup 		= cmp_protocol_dup;
		prot->free 		= cmp_protocol_free;
		prot->read 		= cmp_rx;
		prot->write 		= cmp_tx;
		prot->flush		= cmp_flush;
		prot->getsockopt 	= cmp_getsockopt;
		prot->setsockopt 	= cmp_setsockopt;

		data->type 	= 0;
		data->flags 	= 0;
		data->version 	= 0;
		data->baudrate 	= 0;

		prot->data 	= data;

	} else if (prot != NULL) {
		free(prot);
		prot = NULL;
	} else if (data != NULL) {
		free(data);
		data = NULL;
	}

	return prot;
}


/***********************************************************************
 *
 * Function:    cmp_rx_handshake
 *
 * Summary:     establishes RX handshake
 *
 * Parameters:  pi_socket_t*, baudrate, hirate enable
 *
 * Returns:     0 for success, negative otherwise
 *
 ***********************************************************************/
int
cmp_rx_handshake(pi_socket_t *ps, int establishrate,
	int establishhighrate) 
{
	pi_protocol_t *prot;
	struct 	pi_cmp_data *data;
	pi_buffer_t *buf;
	int bytes;

	prot = pi_protocol(ps->sd, PI_LEVEL_CMP);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	data = (struct pi_cmp_data *)prot->data;

	/* Read the cmp packet */
	buf = pi_buffer_new (PI_CMP_HEADER_LEN);
	if (buf == NULL) {
		//errno = ENOMEM;
		return pi_set_error(ps->sd, PI_ERR_GENERIC_MEMORY);
	}
	
	bytes = cmp_rx(ps, buf, PI_CMP_HEADER_LEN, 0);

	pi_buffer_free (buf);
	if (bytes < 0)
		return bytes;

	if ((data->version & 0xFF00) == 0x0100) {
		if (establishrate != -1) {
			if (establishrate > data->baudrate) {
				if (establishhighrate) {
					LOG((PI_DBG_CMP, PI_DBG_LVL_INFO, 
					    "CMP Establishing higher rate %ul (%ul)\n",
					    establishrate, data->baudrate));
					data->baudrate = establishrate;
				}
			} else {
				data->baudrate = establishrate;
			}
		}
		
		if ((bytes = cmp_init(ps, data->baudrate)) < 0)
			return bytes;
	} else {
		/* 0x80 means the comm version wasn't compatible */
		LOG((PI_DBG_CMP, PI_DBG_LVL_ERR, "CMP Incompatible Version\n"));
		cmp_abort(ps, 0x80);
		//errno = ECONNREFUSED;
		return pi_set_error(ps->sd, PI_ERR_PROT_INCOMPATIBLE);
	}

	return 0;
}


/***********************************************************************
 *
 * Function:    cmp_tx_handshake
 *
 * Summary:     establishes TX handshake
 *
 * Parameters:  pi_socket_t*
 *
 * Returns:     >= 0 for success, negative otherwise
 *
 ***********************************************************************/
int
cmp_tx_handshake(pi_socket_t *ps) 
{
	pi_protocol_t *prot;
	struct 	pi_cmp_data *data;
	int result;

	prot = pi_protocol(ps->sd, PI_LEVEL_CMP);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	data = (struct pi_cmp_data *)prot->data;

	if ((result = cmp_wakeup(ps, 38400)) < 0)	/* Assume box can't go over 38400 */
		return result;

	if ((result = cmp_rx(ps, NULL, 0, 0)) < 0)
		return result;							/* failed to read, errno already set */

	switch (data->type) {
		case PI_CMP_TYPE_INIT:
			return 0;
		case PI_CMP_TYPE_ABRT:
			LOG((PI_DBG_CMP, PI_DBG_LVL_NONE,
				"CMP Aborted by other end\n"));
			//errno = -EIO;
			return pi_set_error(ps->sd, PI_ERR_PROT_ABORTED);
	}

	return PI_ERR_PROT_INCOMPATIBLE;

}


/***********************************************************************
 *
 * Function:    cmp_tx
 *
 * Summary:     Transmit CMP Packets
 *
 * Parameters:  pi_socket_t*, char* to buf, buf length, flags
 *
 * Returns:     A negative number on error, 0 otherwise
 *
 ***********************************************************************/
ssize_t
cmp_tx(pi_socket_t *ps, const unsigned char *buf, size_t len, int flags)
{
	int 	bytes,
		type;
	size_t	size;
	pi_protocol_t	*prot,
			*next;
	struct 	pi_cmp_data *data;
	unsigned char cmp_buf[PI_CMP_HEADER_LEN];

	prot = pi_protocol(ps->sd, PI_LEVEL_CMP);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	data = (struct pi_cmp_data *)prot->data;
	next = pi_protocol_next(ps->sd, PI_LEVEL_CMP);
	if (next == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	type = padData;
	size = sizeof(type);
	pi_setsockopt(ps->sd, PI_LEVEL_PADP, PI_PADP_TYPE, &type, &size);

	set_byte(&cmp_buf[PI_CMP_OFFSET_TYPE], data->type);
	set_byte(&cmp_buf[PI_CMP_OFFSET_FLGS], data->flags);
	set_short(&cmp_buf[PI_CMP_OFFSET_VERS], data->version > PI_CMP_VERSION ? PI_CMP_VERSION : data->version);
	set_short(&cmp_buf[PI_CMP_OFFSET_RESV], 0);
	set_long(&cmp_buf[PI_CMP_OFFSET_BAUD], data->baudrate);

	CHECK(PI_DBG_CMP, PI_DBG_LVL_INFO, cmp_dump(cmp_buf, 1));

	bytes = next->write(ps, cmp_buf, PI_CMP_HEADER_LEN, flags);
	if (bytes < 10)
		return (bytes < 0) ? bytes : pi_set_error(ps->sd, PI_ERR_PROT_ABORTED);

	return 0;
}

/***********************************************************************
 *
 * Function:    cmp_rx
 *
 * Summary:     Receive CMP packets
 *
 * Parameters:  None
 *
 * Returns:     A negative number on error, 0 otherwise
 *
 ***********************************************************************/
ssize_t
cmp_rx(pi_socket_t *ps, pi_buffer_t *msg, size_t len, int flags)
{
	int 	bytes;
	pi_protocol_t	*prot,
			*next;
	struct 	pi_cmp_data *data;

	LOG((PI_DBG_CMP, PI_DBG_LVL_DEBUG, "CMP RX len=%d flags=0x%02x\n",
		len, flags));

	prot = pi_protocol(ps->sd, PI_LEVEL_CMP);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	data = (struct pi_cmp_data *)prot->data;
	next = pi_protocol_next(ps->sd, PI_LEVEL_CMP);
	if (next == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	bytes = next->read(ps, msg, len, flags);
	if (bytes < 10)
		return pi_set_error(ps->sd, (bytes < 0) ? bytes : PI_ERR_PROT_ABORTED);

	CHECK(PI_DBG_CMP, PI_DBG_LVL_INFO, cmp_dump(msg->data, 0));

	data->type 	= get_byte(&msg->data[PI_CMP_OFFSET_TYPE]);
	data->flags 	= get_byte(&msg->data[PI_CMP_OFFSET_FLGS]);
	data->version 	= get_short(&msg->data[PI_CMP_OFFSET_VERS]);
	data->baudrate 	= get_long(&msg->data[PI_CMP_OFFSET_BAUD]);

	return 0;
}

/***********************************************************************
 *
 * Function:    cmp_flush
 *
 * Summary:     Flush input and output buffers
 *
 * Parameters:  pi_socket_t*, flags
 *
 * Returns:     A negative number on error, 0 otherwise
 *
 ***********************************************************************/
static int
cmp_flush(pi_socket_t *ps, int flags)
{
	pi_protocol_t	*prot,
			*next;

	prot = pi_protocol(ps->sd, PI_LEVEL_CMP);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	next = pi_protocol_next(ps->sd, PI_LEVEL_CMP);
	if (next == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	return next->flush(ps, flags);
}

/***********************************************************************
 *
 * Function:    cmp_init
 *
 * Summary:     Initialize the socket for CMP transmission
 *
 * Parameters:  pi_socket_t*, baudrate
 *
 * Returns:     Number of packets transmitted
 *
 ***********************************************************************/
int
cmp_init(pi_socket_t *ps, int baudrate)
{
	pi_protocol_t *prot;
	struct 	pi_cmp_data *data;

	prot = pi_protocol(ps->sd, PI_LEVEL_CMP);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);
	data = (struct pi_cmp_data *)prot->data;

	data->type = PI_CMP_TYPE_INIT;
	data->flags = CMP_FL_LONG_PACKET_SUPPORT;
	if (baudrate != 9600)
		data->flags = CMP_FL_CHANGE_BAUD_RATE;
	data->baudrate = baudrate;

	return cmp_tx(ps, NULL, 0, 0);
}

/***********************************************************************
 *
 * Function:    cmp_abort
 *
 * Summary:     Abort a CMP session in progress
 *
 * Parameters:  pi_socket_t*
 *
 * Returns:     Number of PADP packets transmitted or negative on error
 *
 ***********************************************************************/
int
cmp_abort(pi_socket_t *ps, int reason)
{
	pi_protocol_t *prot;
	struct 	pi_cmp_data *data;
	
	prot = pi_protocol(ps->sd, PI_LEVEL_CMP);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	data = (struct pi_cmp_data *)prot->data;
	data->type = PI_CMP_TYPE_ABRT;
	data->flags = reason;

	LOG((PI_DBG_CMP, PI_DBG_LVL_NONE, "CMP ABORT\n"));

	return cmp_tx (ps, NULL, 0, 0);
}

/***********************************************************************
 *
 * Function:    cmp_wakeup
 *
 * Summary:     Wakeup the CMP listener process
 *
 * Parameters:  pi_socket_t*
 *
 * Returns:     Number of PADP packets transmitted or negative on error
 *
 ***********************************************************************/
int
cmp_wakeup(pi_socket_t *ps, int maxbaud)
{
	pi_protocol_t *prot;
	struct 	pi_cmp_data *data;
	
	prot = pi_protocol(ps->sd, PI_LEVEL_CMP);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	data = (struct pi_cmp_data *)prot->data;
	data->type 	= PI_CMP_TYPE_WAKE;
	data->flags 	= 0;
	data->version 	= PI_CMP_VERSION;
	data->baudrate 	= maxbaud;

	return cmp_tx(ps, NULL, 0, 0);
}


/***********************************************************************
 *
 * Function:    cmp_getsockopt
 *
 * Summary:     get options on socket
 *
 * Parameters:  pi_socket*, level, option name, option value, option length
 *
 * Returns:     0 for success, negative otherwise
 *
 ***********************************************************************/
static int
cmp_getsockopt(pi_socket_t *ps, int level, int option_name, 
	       void *option_value, size_t *option_len)
{
	pi_protocol_t *prot;
	struct 	pi_cmp_data *data;

	(void) level;

	prot = pi_protocol(ps->sd, PI_LEVEL_CMP);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);
	data = (struct pi_cmp_data *)prot->data;

	switch (option_name) {
		case PI_CMP_TYPE:
			if (*option_len != sizeof (data->type))
				goto error;
			memcpy (option_value, &data->type,
				sizeof (data->type));
			*option_len = sizeof (data->type);
			break;

		case PI_CMP_FLAGS:
			if (*option_len != sizeof (data->flags))
				goto error;
			memcpy (option_value, &data->flags,
				sizeof (data->flags));
			*option_len = sizeof (data->flags);
			break;

		case PI_CMP_VERS:
			if (*option_len != sizeof (data->version))
				goto error;
			memcpy (option_value, &data->version,
				sizeof (data->version));
			*option_len = sizeof (data->version);
			break;

		case PI_CMP_BAUD:
			if (*option_len != sizeof (data->baudrate))
				goto error;
			memcpy (option_value, &data->baudrate,
				sizeof (data->baudrate));
			*option_len = sizeof (data->baudrate);
			break;
	}

	return 0;
	
 error:
	//errno = EINVAL;
	return pi_set_error(ps->sd, PI_ERR_GENERIC_ARGUMENT);
}


/***********************************************************************
 *
 * Function:    cmp_setsockopt
 *
 * Summary:     set options on socket
 *
 * Parameters:  pi_socket*, level, option name, option value, option length
 *
 * Returns:     0 for success, negative otherwise
 *
 ***********************************************************************/
static int
cmp_setsockopt(pi_socket_t *ps, int level, int option_name, 
	       const void *option_value, size_t *option_len)
{
	pi_protocol_t *prot;
	struct 	pi_padp_data *data;

	(void) level;

	prot = pi_protocol(ps->sd, PI_LEVEL_PADP);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);
	data = (struct pi_padp_data *)prot->data;

	if (option_name == PI_PADP_TYPE) {
		if (*option_len != sizeof (data->type))
			goto error;
		memcpy (&data->type, option_value,
			sizeof (data->type));
		*option_len = sizeof (data->type);
	}

	return 0;
	
 error:
	//errno = EINVAL;
	return pi_set_error(ps->sd, PI_ERR_GENERIC_ARGUMENT);
}


/***********************************************************************
 *
 * Function:    cmp_dump
 *
 * Summary:     Dump the CMP packet frames
 *
 * Parameters:  char* to cmp packet, TX boolean
 *
 * Returns:     void
 *
 ***********************************************************************/
void
cmp_dump(const unsigned char *cmp, int rxtx)
{
	char *type;

	(void) rxtx;

	switch (get_byte(&cmp[PI_CMP_OFFSET_TYPE])) {
		case PI_CMP_TYPE_WAKE:
			type = "WAKE";
			break;
		case PI_CMP_TYPE_INIT:
			type = "INIT";
			break;
		case PI_CMP_TYPE_ABRT:
			type = "ABRT";
			break;
		default:
			type = "UNK";
			break;
	}
	
	LOG((PI_DBG_CMP, PI_DBG_LVL_NONE,
	    "CMP %s %s Type: 0x%02x Flags: 0x%02x Version: 0x%04x Baud: %d\n",
		rxtx ? "TX" : "RX", type,
		get_byte(&cmp[PI_CMP_OFFSET_TYPE]), 
		get_byte(&cmp[PI_CMP_OFFSET_FLGS]),
		get_long(&cmp[PI_CMP_OFFSET_VERS]),
		get_long(&cmp[PI_CMP_OFFSET_BAUD])));
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
