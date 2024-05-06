/*
 * $Id: slp.c,v 1.56 2006/10/13 09:52:13 fpillet Exp $
 *
 * slp.c:  Pilot SLP protocol
 *
 * (c) 1996, D. Jeff Dionne.
 * Much of this code adapted from Brian J. Swetland <swetland@uiuc.edu>
 * Additional work Copyright (c) 2005, Florent Pillet
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
#include <stdlib.h>
//#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

#include "pi-debug.h"
#include "pi-source.h"
#include "pi-serial.h"
#include "pi-slp.h"
#include "pi-error.h"

/* Declare function prototypes */
static int slp_flush(pi_socket_t *ps, int flags);
static int slp_getsockopt(pi_socket_t *ps, int level, int option_name, 
			  void *option_value, size_t *option_len);
static int slp_setsockopt(pi_socket_t *ps, int level, int option_name, 
			  const void *option_value, size_t *option_len);


/***********************************************************************
 *
 * Function:    slp_protocol_dup
 *
 * Summary:     clones an existing pi_protocol struct
 *
 * Parameters:  pi_protocol_t*
 *
 * Returns:     pi_protocol_t* or NULL if operation failed
 *
 ***********************************************************************/
static pi_protocol_t*
slp_protocol_dup (pi_protocol_t *prot)
{
	pi_protocol_t *new_prot;

	struct	pi_slp_data	*data,
				*new_data;
	
	new_prot = (pi_protocol_t *)malloc (sizeof (pi_protocol_t));
	new_data = (struct pi_slp_data *)malloc (sizeof (struct pi_slp_data));

	if (new_prot != NULL && new_data != NULL) {
		new_prot->level	= prot->level;
		new_prot->dup 	= prot->dup;
		new_prot->free 	= prot->free;
		new_prot->read 	= prot->read;
		new_prot->write	= prot->write;
		new_prot->flush = prot->flush;
		new_prot->getsockopt = prot->getsockopt;
		new_prot->setsockopt = prot->setsockopt;

		data = (struct pi_slp_data *)prot->data;
	
		new_data->dest 	= data->dest;
		new_data->last_dest = data->last_dest;	
		new_data->src 	= data->src;
		new_data->last_src = data->last_src;
		new_data->type 	= data->type;
		new_data->last_type = data->last_type;
		new_data->txid 	= data->txid;
		new_data->last_txid = data->last_txid;

		new_prot->data 	= new_data;

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
 * Function:    slp_protocol_free
 *
 * Summary:     frees an existing pi_protocol struct
 *
 * Parameters:  pi_protocol_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
static void
slp_protocol_free (pi_protocol_t *prot)
{
	if (prot != NULL) {
		if (prot->data != NULL)
			free(prot->data);
		free(prot);
	}
}


/***********************************************************************
 *
 * Function:    slp_protocol
 *
 * Summary:     creates a pi_protocol struct instance
 *
 * Parameters:  pi_protocol_t*
 *
 * Returns:     pi_protocol_t* or NULL if operation failed
 *
 ***********************************************************************/
pi_protocol_t*
slp_protocol (void)
{
	pi_protocol_t *prot;
	struct 	pi_slp_data *data;

	prot 	= (pi_protocol_t *)malloc (sizeof (pi_protocol_t));
	data 	= (struct pi_slp_data *)malloc (sizeof (struct pi_slp_data));

	if (prot != NULL && data != NULL) {
		prot->level = PI_LEVEL_SLP;
		prot->dup = slp_protocol_dup;
		prot->free = slp_protocol_free;
		prot->read = slp_rx;
		prot->write = slp_tx;
		prot->flush = slp_flush;
		prot->getsockopt = slp_getsockopt;
		prot->setsockopt = slp_setsockopt;

		data->dest = PI_SLP_SOCK_DLP;
		data->last_dest	= -1;	
		data->src = PI_SLP_SOCK_DLP;
		data->last_src 	= -1;
		data->type = PI_SLP_TYPE_PADP;
		data->last_type	= -1;
		data->txid = 0xfe;
		data->last_txid	= 0xff;
		prot->data = data;

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
 * Function:    slp_tx
 *
 * Summary:     Build and queue up an SLP packet to be transmitted
 *
 * Parameters:  None
 *
 * Returns:     A negative number on error, 0 otherwise
 *
 ***********************************************************************/
ssize_t
slp_tx(pi_socket_t *ps, const unsigned char *buf, size_t len, int flags)
{
	int 	bytes;
	pi_protocol_t	*prot,
			*next;
	struct 	pi_slp_data *data;
	struct 	slp *slp;
	unsigned char *slp_buf;
	unsigned int	i,
			n;

	prot = pi_protocol(ps->sd, PI_LEVEL_SLP);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	data = (struct pi_slp_data *)prot->data;
	next = pi_protocol_next(ps->sd, PI_LEVEL_SLP);
	if (next == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	slp_buf = (unsigned char *) malloc (PI_SLP_HEADER_LEN +
		PI_SLP_MTU + PI_SLP_FOOTER_LEN);
	if (slp_buf == NULL)
		return pi_set_error(ps->sd, PI_ERR_GENERIC_MEMORY);

	slp = (struct slp *) slp_buf;

	/* Header values */
	slp->_be 	= 0xbe;
	slp->_ef 	= 0xef;
	slp->_ed 	= 0xed;
	slp->dest 	= data->dest;
	slp->src 	= data->src;
	slp->type 	= data->type;
	set_short(&slp->dlen, len);
	slp->id_ 	= data->txid;

	for (n = i = 0; i < 9; i++)
		n += slp_buf[i];
	slp->csum = 0xff & n;

	/* Copy in the packet data */
	memcpy (slp_buf + PI_SLP_HEADER_LEN, buf, len);

	/* CRC value */
	set_short(&slp_buf[PI_SLP_HEADER_LEN + len],
		crc16(slp_buf, (int)(PI_SLP_HEADER_LEN + len)));

	/* Write out the data */
	bytes = next->write(ps, slp_buf,
		PI_SLP_HEADER_LEN + len + PI_SLP_FOOTER_LEN, flags);

	if (bytes >= 0) {
		CHECK(PI_DBG_SLP, PI_DBG_LVL_INFO, slp_dump_header(slp_buf, 1));
		CHECK(PI_DBG_SLP, PI_DBG_LVL_DEBUG, slp_dump(slp_buf));
	}
	
	free (slp_buf);

	return bytes;
}

/* Sigh.  SLP is a really broken protocol.  It has no proper framing, so it
   makes a proper "device driver" layer impossible.  There ought to be a
   layer below SLP that reads frames off the wire and passes them up. 
   Instead, all we can do is have the device driver give us bytes and SLP has
   to keep a pile of status info while it builds frames for itself.  So
   here's the code that does that. */

/***********************************************************************
 *
 * Function:    slp_rx
 *
 * Summary:     Accept SLP packets on the wire
 *
 * Parameters:  None
 *
 * Returns:     packet length or negative on error
 *
 ***********************************************************************/
ssize_t
slp_rx(pi_socket_t *ps, pi_buffer_t *buf, size_t len, int flags)
{
	int 	i,
		computed_crc,
		received_crc,
		b1, 
		b2, 
		b3,	
		state,
		expect 		= 0,
		packet_len,
		bytes;
	unsigned char
		header_checksum;
	pi_protocol_t	*prot,
			*next;
	pi_buffer_t *slp_buf;
	struct 	pi_slp_data *data;

	LOG((PI_DBG_SLP, PI_DBG_LVL_DEBUG, "SLP RX len=%d flags=0x%04x\n",
		len, flags));

	prot = pi_protocol(ps->sd, PI_LEVEL_SLP);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	data = (struct pi_slp_data *)prot->data;
	next = pi_protocol_next(ps->sd, PI_LEVEL_SLP);
	if (next == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	slp_buf = pi_buffer_new (PI_SLP_HEADER_LEN + PI_SLP_MTU + PI_SLP_FOOTER_LEN);
	if (slp_buf == NULL) {
		//errno = ENOMEM;
		return pi_set_error(ps->sd, PI_ERR_GENERIC_MEMORY);
	}

	state 		= 0;
	packet_len	= 0;

	for (;;) {
		switch (state) {		
		case 0:
			expect = 3;
			state++;
			break;

		case 1:
			b1 = slp_buf->data[PI_SLP_OFFSET_SIG1];
			b2 = slp_buf->data[PI_SLP_OFFSET_SIG2];
			b3 = slp_buf->data[PI_SLP_OFFSET_SIG3];
			if (b1 == PI_SLP_SIG_BYTE1 &&
			    b2 == PI_SLP_SIG_BYTE2 &&
			    b3 == PI_SLP_SIG_BYTE3) {
				state++;
				expect = PI_SLP_HEADER_LEN - 3;
			} else {
				slp_buf->data[PI_SLP_OFFSET_SIG1] = slp_buf->data[PI_SLP_OFFSET_SIG2];
				slp_buf->data[PI_SLP_OFFSET_SIG2] = slp_buf->data[PI_SLP_OFFSET_SIG3];
				expect = 1;
				slp_buf->used = 2;
				LOG((PI_DBG_SLP, PI_DBG_LVL_WARN,
					"SLP RX Unexpected signature"
					" 0x%.2x 0x%.2x 0x%.2x\n",
					b1, b2, b3));
			}
			break;

		case 2:
			/* Addition check sum for header */
			for (header_checksum = i = 0; i < 9; i++)
				header_checksum += slp_buf->data[i];

			/* read in the whole SLP header. */
			if (header_checksum == slp_buf->data[PI_SLP_OFFSET_SUM]) {
				state++;
				packet_len = get_short(&slp_buf->data[PI_SLP_OFFSET_SIZE]);
				if (packet_len > (int)len) {
					LOG((PI_DBG_SLP, PI_DBG_LVL_ERR,
						"SLP RX Packet size exceed buffer\n"));
					pi_buffer_free (slp_buf);
					return pi_set_error(ps->sd, PI_ERR_PROT_BADPACKET);
				}
				expect = packet_len;
			} else {
				LOG((PI_DBG_SLP, PI_DBG_LVL_WARN,
					"SLP RX Header checksum failed for header:\n"));
				pi_dumpdata((const char *)slp_buf->data, PI_SLP_HEADER_LEN);
				pi_buffer_free (slp_buf);
				return 0;
			}
			break;

		case 3:
			state++;
			expect = PI_SLP_FOOTER_LEN;
			break;

		case 4:
			/* that should be the whole packet. */
			computed_crc = crc16(slp_buf->data, PI_SLP_HEADER_LEN + packet_len);
			received_crc = get_short(&slp_buf->data[PI_SLP_HEADER_LEN + packet_len]);
			if (get_byte(&slp_buf->data[PI_SLP_OFFSET_TYPE]) == PI_SLP_TYPE_LOOP) {
				/* Adjust because every tenth loopback
				 * packet has a bogus check sum */
				if (computed_crc != received_crc)
					computed_crc |= 0x00e0;
			}
			if (computed_crc != received_crc) {
				LOG((PI_DBG_SLP, PI_DBG_LVL_ERR,
				    "SLP RX packet crc failed: "
				    "computed=0x%.4x received=0x%.4x\n",
				    computed_crc, received_crc));
				pi_buffer_free (slp_buf);
				return 0;
			}
			
			/* Track the info so getsockopt will work */
			data->last_dest = get_byte(&slp_buf->data[PI_SLP_OFFSET_DEST]);
			data->last_src 	= get_byte(&slp_buf->data[PI_SLP_OFFSET_SRC]);
			data->last_type = get_byte(&slp_buf->data[PI_SLP_OFFSET_TYPE]);
			data->last_txid = get_byte(&slp_buf->data[PI_SLP_OFFSET_TXID]);

			CHECK(PI_DBG_SLP, PI_DBG_LVL_INFO, slp_dump_header(slp_buf->data, 0));
			CHECK(PI_DBG_SLP, PI_DBG_LVL_DEBUG, slp_dump(slp_buf->data));

			if (pi_buffer_append (buf, &slp_buf->data[PI_SLP_HEADER_LEN], packet_len) == NULL) {
				//errno = ENOMEM;
				return pi_set_error(ps->sd, PI_ERR_GENERIC_MEMORY);
			}
			pi_buffer_free (slp_buf);
			return packet_len;

		default:
			break;
		}
		
		do {
			bytes = next->read(ps, slp_buf, (size_t)expect, flags);
			if (bytes < 0) {
				LOG((PI_DBG_SLP, PI_DBG_LVL_ERR,
				    "SLP RX Read Error %d\n",
				    bytes));
				pi_buffer_free (slp_buf);
				return bytes;
			}
			expect -= bytes;
		} while (expect > 0);
	}
}

/***********************************************************************
 *
 * Function:    slp_flush
 *
 * Summary:     Flush input and output buffers
 *
 * Parameters:  pi_socket_t*, flags
 *
 * Returns:     A negative number on error, 0 otherwise
 *
 ***********************************************************************/
static int
slp_flush(pi_socket_t *ps, int flags)
{
	pi_protocol_t	*prot,
			*next;

	prot = pi_protocol(ps->sd, PI_LEVEL_SLP);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	next = pi_protocol_next(ps->sd, PI_LEVEL_SLP);
	if (next == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	return next->flush(ps, flags);
}

/***********************************************************************
 *
 * Function:    slp_getsockopt
 *
 * Summary:     get options on socket
 *
 * Parameters:  pi_socket*, level, option name, option value, option length
 *
 * Returns:     0 for success, negative otherwise
 *
 ***********************************************************************/
static int
slp_getsockopt(pi_socket_t *ps, int level, int option_name, 
       void *option_value, size_t *option_len)
{
	pi_protocol_t *prot;
	struct 	pi_slp_data *data;

	prot = pi_protocol(ps->sd, PI_LEVEL_SLP);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);

	data = (struct pi_slp_data *)prot->data;

	switch (option_name) {
		case PI_SLP_DEST:
			if (*option_len < sizeof (data->dest))
				goto error;
			memcpy (option_value, &data->dest, sizeof (data->dest));
			*option_len = sizeof (data->dest);
			break;
		case PI_SLP_LASTDEST:
			if (*option_len < sizeof (data->dest))
				goto error;
			memcpy (option_value, &data->last_dest,
					sizeof (data->last_dest));
			*option_len = sizeof (data->last_dest);
			break;
		case PI_SLP_SRC:
			if (*option_len < sizeof (data->src))
				goto error;
			memcpy (option_value, &data->src, 
					sizeof (data->src));
			*option_len = sizeof (data->src);
			break;
		case PI_SLP_LASTSRC:
			if (*option_len < sizeof (data->last_src))
				goto error;
			memcpy (option_value, &data->last_src, 
					sizeof (data->last_src));
			*option_len = sizeof (data->last_src);
			break;
		case PI_SLP_TYPE:
			if (*option_len < sizeof (data->type))
				goto error;
			memcpy (option_value, &data->type,
					sizeof (data->type));
			*option_len = sizeof (data->type);
			break;
		case PI_SLP_LASTTYPE:
			if (*option_len < sizeof (data->last_type))
				goto error;
			memcpy (option_value, &data->last_type,
					sizeof (data->last_type));
			*option_len = sizeof (data->last_type);
			break;
		case PI_SLP_TXID:
			if (*option_len < sizeof (data->txid))
				goto error;
			memcpy (option_value, &data->txid,
					sizeof (data->txid));
			*option_len = sizeof (data->txid);
			break;
		case PI_SLP_LASTTXID:
			if (*option_len < sizeof (data->last_txid))
				goto error;
			memcpy (option_value, &data->last_txid,
					sizeof (data->last_txid));
			*option_len = sizeof (data->last_txid);
			break;
	}
	
	return 0;
	
 error:
	//errno = EINVAL;
	return pi_set_error(ps->sd, PI_ERR_GENERIC_ARGUMENT);
}


/***********************************************************************
 *
 * Function:    slp_setsockopt
 *
 * Summary:     set options on socket
 *
 * Parameters:  pi_socket*, level, option name, option value, option length
 *
 * Returns:     0 for success, negative otherwise
 *
 ***********************************************************************/
static int
slp_setsockopt(pi_socket_t *ps, int level, int option_name, 
       const void *option_value, size_t *option_len)
{
	pi_protocol_t *prot;
	struct 	pi_slp_data *data;

	prot = pi_protocol(ps->sd, PI_LEVEL_SLP);
	if (prot == NULL)
		return pi_set_error(ps->sd, PI_ERR_SOCK_INVALID);
	data = (struct pi_slp_data *)prot->data;

	switch (option_name) {
		case PI_SLP_DEST:
			if (*option_len != sizeof (data->dest))
				goto error;
			memcpy (&data->dest, option_value,
					sizeof (data->dest));
			*option_len = sizeof (data->dest);
			break;
		case PI_SLP_SRC:
			if (*option_len != sizeof (data->src))
				goto error;
			memcpy (&data->src, option_value,
					sizeof (data->src));
			*option_len = sizeof (data->src);
			break;
		case PI_SLP_TYPE:
			if (*option_len != sizeof (data->type))
				goto error;
			memcpy (&data->type, option_value,
					sizeof (data->type));
			*option_len = sizeof (data->type);
			break;
		case PI_SLP_TXID:
			if (*option_len != sizeof (data->txid))
				goto error;
			memcpy (&data->txid, option_value,
					sizeof (data->txid));
			*option_len = sizeof (data->txid);
			break;
	}
	
	return 0;
	
 error:
	//errno = EINVAL;
	return pi_set_error(ps->sd, PI_ERR_GENERIC_ARGUMENT);
}


/***********************************************************************
 *
 * Function:    slp_dump_header
 *
 * Summary:     Dump the contents of the SPL frame header
 *
 * Parameters:  char* to data buffer, RXTX flag
 *
 * Returns:     void
 *
 ***********************************************************************/
void
slp_dump_header(const unsigned char *data, int rxtx)
{	
	LOG((PI_DBG_SLP, PI_DBG_LVL_NONE,
	    "SLP %s %d->%d type=%d txid=0x%.2x len=0x%.4x checksum=0x%.2x\n",
	    rxtx ? "TX" : "RX",
	    get_byte(&data[PI_SLP_OFFSET_DEST]),
	    get_byte(&data[PI_SLP_OFFSET_SRC]),
	    get_byte(&data[PI_SLP_OFFSET_TYPE]),
	    get_byte(&data[PI_SLP_OFFSET_TXID]),
	    get_short(&data[PI_SLP_OFFSET_SIZE]),
	    get_byte(&data[PI_SLP_OFFSET_SUM])));
}


/***********************************************************************
 *
 * Function:    slp_dump
 *
 * Summary:     Dump the contents of the SPL frame
 *
 * Parameters:  char* to data buffer
 *
 * Returns:     void
 *
 ***********************************************************************/
void
slp_dump(const unsigned char *data)
{
	pi_dumpdata((char *)&data[PI_SLP_HEADER_LEN], get_short(&data[PI_SLP_OFFSET_SIZE]));
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
