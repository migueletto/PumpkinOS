/*
 * $Id: sys.c,v 1.17 2006/10/12 14:21:22 desrod Exp $
 *
 * sys.c:  Pilot System Protocol
 *
 * (c) 1996, Kenneth Albanowski.
 * Derived from padp.c.
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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "pi-debug.h"
#include "pi-source.h"
#include "pi-slp.h"
#include "pi-sys.h"
#include "pi-error.h"

/* Declare function prototypes */
static int sys_flush(pi_socket_t *ps, int flags);
static int sys_getsockopt(pi_socket_t *ps, int level, int option_name, 
			  void *option_value, size_t *option_len);
static int sys_setsockopt(pi_socket_t *ps, int level, int option_name, 
			  const void *option_value, size_t *option_len);


/***********************************************************************
 *
 * Function:    sys_protocol_dup
 *
 * Summary:     clones an existing pi_protocol struct
 *
 * Parameters:  pi_protocol*
 *
 * Returns:     pi_protocol_t* or NULL if operation failed
 *
 ***********************************************************************/
static pi_protocol_t *
sys_protocol_dup (pi_protocol_t *prot)
{
	pi_protocol_t *new_prot = NULL;
	pi_sys_data_t	*data = NULL,
			*new_data = NULL;
	
	new_prot = (pi_protocol_t *)malloc (sizeof (pi_protocol_t));
	if (new_prot != NULL) {
		new_data = (pi_sys_data_t *)malloc (sizeof (pi_sys_data_t));
		if (new_data == NULL) {
			free(new_prot);
			new_prot = NULL;
		}
	}

	if (new_prot != NULL && new_data != NULL) {	
		new_prot->level	= prot->level;
		new_prot->dup 	= prot->dup;
		new_prot->free 	= prot->free;
		new_prot->read 	= prot->read;
		new_prot->write = prot->write;
		new_prot->flush = prot->flush;
		new_prot->getsockopt	= prot->getsockopt;
		new_prot->setsockopt 	= prot->setsockopt;

		data 	= (pi_sys_data_t *)prot->data;
		new_data->txid 	= data->txid;
		new_prot->data 	= new_data;
	}

	return new_prot;
}


/***********************************************************************
 *
 * Function:    sys_protocol_free
 *
 * Summary:     frees an existing pi_protocol struct
 *
 * Parameters:  pi_protocol*
 *
 * Returns:     void
 *
 ***********************************************************************/
static void
sys_protocol_free (pi_protocol_t *prot)
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
 * Function:    sys_protocol
 *
 * Summary:     creates and inits pi_protocol struct instance
 *
 * Parameters:  void
 *
 * Returns:     pi_protocol_t* or NULL if operation failed
 *
 ***********************************************************************/
pi_protocol_t *
sys_protocol (void)
{
	pi_protocol_t *prot = NULL;
	pi_sys_data_t *data = NULL;

	prot 	= (pi_protocol_t *)malloc (sizeof (pi_protocol_t));	
	if (prot != NULL) {
		data = (pi_sys_data_t *)malloc (sizeof (pi_sys_data_t));
		if (data == NULL) {
			free(prot);
			prot = NULL;
		}
	}

	if (prot != NULL && data != NULL) {
		prot->level 	= PI_LEVEL_SYS;
		prot->dup 	= sys_protocol_dup;
		prot->free 	= sys_protocol_free;
		prot->read 	= sys_rx;
		prot->write 	= sys_tx;
		prot->flush	= sys_flush;
		prot->getsockopt = sys_getsockopt;
		prot->setsockopt = sys_setsockopt;

		data->txid 	= 0x00;
		prot->data 	= data;
	}

	return prot;
}


/***********************************************************************
 *
 * Function:    sys_tx
 *
 * Summary:     Send a system message
 *
 * Parameters:  pi_socket_t*, char* to buffer, buffer length, flags
 *
 * Returns:     0 if success, nonzero otherwise
 *
 ***********************************************************************/
ssize_t
sys_tx(pi_socket_t *ps, const unsigned char *buf, size_t len, int flags)
{
	pi_protocol_t	*prot,
			*next;

	pi_sys_data_t *data;

	int 	type,
		socket;

	size_t	size;

	prot = pi_protocol(ps->sd, PI_LEVEL_SYS);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	data = (pi_sys_data_t *)prot->data;

	next = pi_protocol_next(ps->sd, PI_LEVEL_SYS);
	if (next == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	if (!data->txid || data->txid == 0xff)
		data->txid = 0x11;	/* some random # */
	data->txid++;
	if (!data->txid || data->txid == 0xff)
		data->txid = 0x11;	/* some random # */

	type 	= PI_SLP_TYPE_RDCP;

	/* Fix me, allow socket type */
	socket 	= PI_SLP_SOCK_CON;
	size 	= sizeof(type);
	pi_setsockopt(ps->sd, PI_LEVEL_SLP, PI_SLP_TYPE, 
		      &type, &size);
	pi_setsockopt(ps->sd, PI_LEVEL_SLP, PI_SLP_DEST, 
		      &socket, &size);
	pi_setsockopt(ps->sd, PI_LEVEL_SLP, PI_SLP_SRC, 
		      &socket, &size);
	size = sizeof(data->txid);
	pi_setsockopt(ps->sd, PI_LEVEL_SLP, PI_SLP_TXID, 
		      &data->txid, &size);
	
	len = next->write(ps, buf, len, flags);
	if (len >= 0) {
		CHECK(PI_DBG_SYS, PI_DBG_LVL_INFO, sys_dump_header(buf, 1));
		CHECK(PI_DBG_SYS, PI_DBG_LVL_DEBUG, sys_dump(buf, len));
	}

	return len;
}


/***********************************************************************
 *
 * Function:    sys_rx
 *
 * Summary:     Receive system message
 *
 * Parameters:  pi_socket_t*, char* to buffer, buffer length, flags
 *
 * Returns:     Length of read or negative on error
 *
 ***********************************************************************/
ssize_t
sys_rx(pi_socket_t *ps, pi_buffer_t *buf, size_t len, int flags)
{
	pi_protocol_t	*next,
			*prot;

	//pi_sys_data_t *data;
	size_t 	data_len;

	prot = pi_protocol(ps->sd, PI_LEVEL_SYS);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	//data = (pi_sys_data_t *)prot->data;
	next = pi_protocol_next(ps->sd, PI_LEVEL_SYS);
	if (next == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	data_len = next->read(ps, buf, len, flags);

	CHECK(PI_DBG_SYS, PI_DBG_LVL_INFO, sys_dump_header(buf->data, 0));
	CHECK(PI_DBG_SYS, PI_DBG_LVL_DEBUG, sys_dump(buf->data, data_len));

	return data_len;
}

/***********************************************************************
 *
 * Function:    sys_flush
 *
 * Summary:     Flush input and output buffers
 *
 * Parameters:  pi_socket_t*, flags
 *
 * Returns:     A negative number on error, 0 otherwise
 *
 ***********************************************************************/
static int
sys_flush(pi_socket_t *ps, int flags)
{
	pi_protocol_t	*prot,
			*next;

	prot = pi_protocol(ps->sd, PI_LEVEL_SYS);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	next = pi_protocol_next(ps->sd, PI_LEVEL_SYS);
	if (next == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	return next->flush(ps, flags);
}

/***********************************************************************
 *
 * Function:    sys_getsockopt
 *
 * Summary:     get options on socket
 *
 * Parameters:  pi_socket*, level, option name, option value, option length
 *
 * Returns:     0 for success
 *
 ***********************************************************************/
static int
sys_getsockopt(pi_socket_t *ps, int level, int option_name, 
	       void *option_value, size_t *option_len)
{
	return 0;
}


/***********************************************************************
 *
 * Function:    sys_setsockopt
 *
 * Summary:     get options on socket
 *
 * Parameters:  pi_socket*, level, option name, option value, option length
 *
 * Returns:     0 for success
 *
 ***********************************************************************/
static int
sys_setsockopt(pi_socket_t *ps, int level, int option_name, 
	       const void *option_value, size_t *option_len)
{
	return 0;
}


/***********************************************************************
 *
 * Function:    sys_dump_header
 *
 * Summary:     Dump SYS packet header
 *
 * Parameters:  char* to data, RXTX boolean
 *
 * Returns:     void
 *
 ***********************************************************************/
void
sys_dump_header(const unsigned char *data, int rxtx)
{
	LOG((PI_DBG_SYS, PI_DBG_LVL_NONE,
	    "SYS %s\n", rxtx ? "TX" : "RX"));
}


/***********************************************************************
 *
 * Function:    sys_dump
 *
 * Summary:     Dump SYS packet
 *
 * Parameters:  char* to data, length
 *
 * Returns:     void
 *
 ***********************************************************************/
void
sys_dump(const unsigned char *data, size_t len)
{
	pi_dumpdata((char *)&data[PI_SYS_HEADER_LEN], len);
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */

