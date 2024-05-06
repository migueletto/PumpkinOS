/*
 * $Id: unixserial.c,v 1.53 2006/10/12 14:21:23 desrod Exp $
 *
 * unixserial.c: tty line interface code for Pilot serial comms under UNIX
 *
 * Copyright (c) 1996, 1997, D. Jeff Dionne & Kenneth Albanowski.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>		/* Needed for Redhat 6.x machines */
#include <fcntl.h>
#include <string.h>

#include "pi-debug.h"
#include "pi-source.h"
#include "pi-serial.h"
#include "pi-error.h"

/* if this is running on a NeXT system... */
#ifdef NeXT
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/file.h>
#endif

#ifdef HAVE_SYS_IOCTL_COMPAT_H
#include <sys/ioctl_compat.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifndef SGTTY

#ifndef HAVE_CFMAKERAW
#define cfmakeraw(ptr) (ptr)->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR\
					 |IGNCR|ICRNL|IXON);\
                       (ptr)->c_oflag &= ~OPOST;\
                       (ptr)->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);\
                       (ptr)->c_cflag &= ~(CSIZE|PARENB);\
                       (ptr)->c_cflag |= CS8
#endif

#ifndef HAVE_CFSETSPEED
#if defined(HAVE_CFSETISPEED) && defined(HAVE_CFSETOSPEED)
#define cfsetspeed(t,speed) \
  (cfsetispeed(t,speed) || cfsetospeed(t,speed))
#else
static int cfsetspeed(struct termios *t, int speed)
{
#ifdef HAVE_TERMIOS_CSPEED
	t->c_ispeed = speed;
	t->c_ospeed = speed;
#else
	t->c_cflag |= speed;
#endif
	return 0;
}
#endif
#endif

#endif /* SGTTY */

#ifndef O_NONBLOCK
# define O_NONBLOCK 0
#endif

/* Linux versions "before 2.1.8 or so" fail to flush hardware FIFO on port
   close */
#ifdef linux
# ifndef LINUX_VERSION_CODE
#  include <linux/version.h>
# endif
# ifndef LINUX_VERSION_CODE
#  define sleeping_beauty
# else
#  if (LINUX_VERSION_CODE < 0x020108)
#   define sleeping_beauty
#  endif
# endif
#endif

/* Unspecified NetBSD versions fail to flush hardware FIFO on port close */
#if defined(__NetBSD__) || defined (__OpenBSD__)
# define sleeping_beauty
#endif

/* Unspecified BSD/OS versions fail to flush hardware FIFO on port close */
#ifdef __bsdi__
# define sleeping_beauty
#endif

/* SGI IRIX fails to flush hardware FIFO on port close */
#ifdef __sgi
# define sleeping_beauty
#endif

/* Declare prototypes */
static int s_open(pi_socket_t *ps, struct pi_sockaddr *addr,
	size_t addrlen);
static int s_close(pi_socket_t *ps);
static int s_changebaud(pi_socket_t *ps);
static ssize_t s_write(pi_socket_t *ps, const unsigned char *buf,
	size_t len, int flags);
static ssize_t s_read(pi_socket_t *ps, pi_buffer_t *buf, size_t len,
	int flags);
static int s_poll(pi_socket_t *ps, int timeout);

static speed_t calcrate(int baudrate);
void pi_serial_impl_init (struct pi_serial_impl *impl);
static int s_flush(pi_socket_t *ps, int flags);

#ifdef sleeping_beauty
static void s_delay(time_t sec, suseconds_t usec);
#endif


/***********************************************************************
 *
 * Function:    s_open
 *
 * Summary:     Open the serial port and establish a connection for
 *		unix
 *
 * Parameters:	pi_socket_t*, pi_socket_taddr*, size_t
 *
 * Returns:     The file descriptor or negative on error
 *
 ***********************************************************************/
int
s_open(pi_socket_t *ps, struct pi_sockaddr *addr, size_t addrlen)
{
	int 	fd, 
		i;
	char 	*tty 	= addr->pi_device;

	struct pi_serial_data *data =
		(struct pi_serial_data *)ps->device->data;
	
#ifndef SGTTY
	struct termios tcn;
#else
	struct sgttyb tcn;
#endif
	if ((fd = open(tty, O_RDWR | O_NONBLOCK)) < 0) {
		ps->last_error = PI_ERR_GENERIC_SYSTEM;
		return PI_ERR_GENERIC_SYSTEM;	/* errno already set */
	}

	if (!isatty(fd)) {
		close(fd);
		errno = EINVAL;
		ps->last_error = PI_ERR_GENERIC_SYSTEM;
		return PI_ERR_GENERIC_SYSTEM;
	}

#ifndef SGTTY
	/* Set the tty to raw and to the correct speed */
	tcgetattr(fd, &tcn);

	data->tco 	= tcn;
	tcn.c_oflag 	= 0;
	tcn.c_iflag 	= IGNBRK | IGNPAR;
	tcn.c_cflag 	= CREAD | CLOCAL | CS8;

	cfsetspeed(&tcn, calcrate(data->rate));

	tcn.c_lflag = NOFLSH;

	cfmakeraw(&tcn);

	for (i = 0; i < 16; i++)
		tcn.c_cc[i] = 0;

	tcn.c_cc[VMIN] 	= 1;
	tcn.c_cc[VTIME] = 0;

	tcsetattr(fd, TCSANOW, &tcn);
#else
	/* Set the tty to raw and to the correct speed */
	ioctl(fd, TIOCGETP, &tcn);

	data->tco = tcn;

	tcn.sg_flags = RAW;
	tcn.sg_ispeed = calcrate(data->rate);
	tcn.sg_ospeed = calcrate(data->rate);

	ioctl(fd, TIOCSETN, &tcn);
#endif

	if ((i = fcntl(fd, F_GETFL, 0)) != -1) {
		i &= ~O_NONBLOCK;
		fcntl(fd, F_SETFL, i);
	}

	if ((i = pi_socket_setsd(ps, fd)) < 0)
		return i;

	return fd;
}


/***********************************************************************
 *
 * Function:    s_close
 *
 * Summary:     Close the open socket/file descriptor
 *
 * Parameters:	pi_socket_t*
 *
 * Returns:     0 for success, negative otherwise
 *
 ***********************************************************************/
static int
s_close(pi_socket_t *ps)
{
#ifdef sleeping_beauty
	s_delay(2, 0);
#endif

#if 0		/* previous test would never allow this code to execute */
 #ifndef SGTTY
	tcsetattr(ps->sd, TCSADRAIN, &data->tco);
 #else
	ioctl(ps->sd, TIOCSETP, &data->tco);
 #endif
#endif
	
	LOG((PI_DBG_DEV, PI_DBG_LVL_INFO,
		"DEV CLOSE unixserial fd: %d\n", ps->sd));

	return close(ps->sd);
}


/***********************************************************************
 *
 * Function:    s_poll
 *
 * Summary:     poll the open socket/file descriptor
 *
 * Parameters:	pi_socket_t*, timeout in milliseconds
 *
 * Returns:     0 for success, negative otherwise
 *
 ***********************************************************************/
static int
s_poll(pi_socket_t *ps, int timeout)
{
	struct 	pi_serial_data *data =
		 (struct pi_serial_data *)ps->device->data;
	struct 	timeval t;
	fd_set 	ready;

	FD_ZERO(&ready);
	FD_SET(ps->sd, &ready);

	/* If timeout == 0, wait forever for packet, otherwise wait till
	   timeout milliseconds */
	if (timeout == 0)
		select(ps->sd + 1, &ready, 0, 0, 0);
	else {
		t.tv_sec 	= timeout / 1000;
		t.tv_usec 	= (timeout % 1000) * 1000;
		if (select(ps->sd + 1, &ready, 0, 0, &t) == 0)
			return pi_set_error(ps->sd, PI_ERR_SOCK_TIMEOUT);
	}

	if (!FD_ISSET(ps->sd, &ready)) {
		/* otherwise throw out any current packet and return */
		LOG((PI_DBG_DEV, PI_DBG_LVL_WARN,
			"DEV POLL unixserial timeout\n"));
		data->rx_errors++;
		errno = ETIMEDOUT;
		return pi_set_error(ps->sd, PI_ERR_SOCK_TIMEOUT);
	}
	LOG((PI_DBG_DEV, PI_DBG_LVL_DEBUG,
		"DEV POLL unixserial found data on fd: %d\n", ps->sd));

	return 0;
}


/***********************************************************************
 *
 * Function:    s_write
 *
 * Summary:     Write to the open socket/file descriptor
 *
 * Parameters:	pi_socket_t*, unsigned char* to buf, buf length
 *
 * Returns:     number of bytes written or negative on error
 *
 ***********************************************************************/
static ssize_t
s_write(pi_socket_t *ps, const unsigned char *buf, size_t len,
	int flags)
{
	ssize_t	total,
		nwrote;
	struct 	pi_serial_data *data =
		(struct pi_serial_data *)ps->device->data;
	struct 	timeval t;
	fd_set 	ready;

	FD_ZERO(&ready);
	FD_SET(ps->sd, &ready);

	total = len;
	while (total > 0) {
		if (data->timeout == 0)
			select(ps->sd + 1, 0, &ready, 0, 0);
		else {
			t.tv_sec 	= data->timeout / 1000;
			t.tv_usec 	= (data->timeout % 1000) * 1000;
			if (select(ps->sd + 1, 0, &ready, 0, &t) == 0)
				return pi_set_error(ps->sd, PI_ERR_SOCK_TIMEOUT);
		}

		if (!FD_ISSET(ps->sd, &ready))
			return pi_set_error(ps->sd, PI_ERR_SOCK_TIMEOUT);

		nwrote = write(ps->sd, buf, len);
		if (nwrote < 0) {
			if (errno == EPIPE || errno == EBADF) {
				ps->state = PI_SOCK_CONN_BREAK;
				return pi_set_error(ps->sd, PI_ERR_SOCK_DISCONNECTED);
			}
			return pi_set_error(ps->sd, PI_ERR_SOCK_IO);
		}
		total -= nwrote;
	}
	data->tx_bytes += len;

	/* hack to slow things down so that the Visor will work */
	usleep(10 + len);

	LOG((PI_DBG_DEV, PI_DBG_LVL_DEBUG,
		"DEV TX unixserial wrote %d bytes\n", len));

	return len;
}


/***********************************************************************
 *
 * Function:    s_read_buf
 *
 * Summary:     read from data buffer
 *
 * Parameters:	pi_socket_t*, pi_buffer_t* to buf, length to get
 *
 * Returns:     number of bytes read
 *
 ***********************************************************************/
static size_t
s_read_buf (pi_socket_t *ps, pi_buffer_t *buf, size_t len, int flags) 
{
	struct 	pi_serial_data *data =
		(struct pi_serial_data *)ps->device->data;
	size_t	rbuf = data->buf_size;

	if (rbuf > len)
		rbuf = len;

	if (pi_buffer_append (buf, data->buf, rbuf) == NULL) {
		errno = ENOMEM;
		return pi_set_error(ps->sd, PI_ERR_GENERIC_MEMORY);
	}

	if (flags != PI_MSG_PEEK) {
		data->buf_size -= rbuf;
		if (data->buf_size > 0)
			memmove(data->buf, &data->buf[rbuf], data->buf_size);
	}

	LOG((PI_DBG_DEV, PI_DBG_LVL_DEBUG,
		"DEV RX unixserial read %d bytes from read-ahead buffer\n", rbuf));

	return rbuf;
}

/***********************************************************************
 *
 * Function:    s_read
 *
 * Summary:     Read incoming data from the socket/file descriptor
 *
 * Parameters:	pi_socket_t*, pi_buffer_t* to buf, expect length, flags
 *
 * Returns:     number of bytes read or negative on error
 *
 ***********************************************************************/
static ssize_t
s_read(pi_socket_t *ps, pi_buffer_t *buf, size_t len, int flags)
{
	ssize_t rbuf = 0,
		bytes;
	struct 	pi_serial_data *data =
		(struct pi_serial_data *)ps->device->data;
	struct 	timeval t;
	fd_set 	ready;

	/* check whether we have at least partial data in store */
	if (data->buf_size) {
		rbuf = s_read_buf(ps, buf, len, flags);
		if (rbuf < 0)
			return rbuf;
		len -= rbuf;
		if (len == 0)
			return rbuf;
	}

	/* If timeout == 0, wait forever for packet, otherwise wait till
	   timeout milliseconds */
	FD_ZERO(&ready);
	FD_SET(ps->sd, &ready);
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
		if (flags == PI_MSG_PEEK && len > 256)
			len = 256;

		if (pi_buffer_expect (buf, len) == NULL) {
			errno = ENOMEM;
			return pi_set_error(ps->sd, PI_ERR_GENERIC_MEMORY);
		}

		bytes = read(ps->sd, &buf->data[buf->used], len);

		if (bytes > 0) {
			if (flags == PI_MSG_PEEK) {
				memcpy(data->buf + data->buf_size, buf->data + buf->used, bytes);
				data->buf_size += bytes;
			}
			buf->used += bytes;
			data->rx_bytes += bytes;
			rbuf += bytes;

			LOG((PI_DBG_DEV, PI_DBG_LVL_DEBUG,
				"DEV RX unixserial read %d bytes\n", bytes));
		} else if (bytes < 0) {
			rbuf = bytes;
		}
	} else {
		LOG((PI_DBG_DEV, PI_DBG_LVL_WARN,
			"DEV RX unixserial timeout\n"));
		data->rx_errors++;
		errno = ETIMEDOUT;
		return pi_set_error(ps->sd, PI_ERR_SOCK_TIMEOUT);
	}

	return rbuf;
}

/***********************************************************************
 *
 * Function:    s_flush
 *
 * Summary:	Flush incoming and/or outgoing data from the socket/file
 *		descriptor
 *
 * Parameters:	ps is of type pi_socket that contains the sd member which is
 *              the file descriptor that the data in buf will be read. It
 *              also contains the read buffer.
 *
 *		flags is of type int and can be a combination of
 *		PI_FLUSH_INPUT and PI_FLUSH_OUTPUT
 *
 * Returns:	0
 *
 ***********************************************************************/
static int
s_flush(pi_socket_t *ps, int flags)
{
	int fl;
	char buf[256];
	struct pi_serial_data *data = (struct pi_serial_data *) ps->device->data;

	if (flags & PI_FLUSH_INPUT) {
		/* clear internal buffer */
		data->buf_size = 0;

		/* flush pending data (we assume the socket is in blocking mode) */
		if ((fl = fcntl(ps->sd, F_GETFL, 0)) != -1)
		{
			fcntl(ps->sd, F_SETFL, fl | O_NONBLOCK);
			while (recv(ps->sd, buf, sizeof(buf), 0) > 0)
				;
			fcntl(ps->sd, F_SETFL, fl);
		}

		LOG((PI_DBG_DEV, PI_DBG_LVL_DEBUG,
			"DEV FLUSH unixserial flushed input buffer\n"));
	}
	return 0;
}

#ifdef sleeping_beauty
/***********************************************************************
 *
 * Function:    s_delay
 *
 * Summary:     Delay for a given period of time
 *
 * Parameters:	seconds, microseconds
 *
 * Returns:     Nothing
 *
 ***********************************************************************/
static void
s_delay(time_t sec, suseconds_t usec)
{
	struct 	timeval tv;

	tv.tv_sec 	= sec;
	tv.tv_usec 	= usec;

	select(0, 0, 0, 0, &tv);
}
#endif


/***********************************************************************
 *
 * Function:    s_changebaud
 *
 * Summary:     Change the speed of the socket
 *
 * Parameters:	pi_socket_t*
 *
 * Returns:     0 on success, negative otherwise
 *
 ***********************************************************************/
static int
s_changebaud(pi_socket_t *ps)
{
	struct 	pi_serial_data *data =
		(struct pi_serial_data *)ps->device->data;
#ifndef SGTTY
	struct 	termios tcn;

	LOG((PI_DBG_DEV, PI_DBG_LVL_DEBUG,
		"DEV SPEED unixserial switch to %d bps\n", (int)data->rate));

#ifdef sleeping_beauty
	s_delay(0, 200000);
#endif
	/* Set the tty to the new speed */
	if (tcgetattr(ps->sd, &tcn))
		return pi_set_error(ps->sd, PI_ERR_GENERIC_SYSTEM);

	tcn.c_cflag 	= CREAD | CLOCAL | CS8;
	cfsetspeed(&tcn, calcrate(data->rate));

	if (tcsetattr(ps->sd, TCSADRAIN, &tcn))
		return pi_set_error(ps->sd, PI_ERR_GENERIC_SYSTEM);

#else
	struct sgttyb tcn;

	if (ioctl(ps->sd, TIOCGETP, &tcn))
		return pi_set_error(ps->sd, PI_ERR_GENERIC_SYSTEM);

	tcn.sg_ispeed 	= calcrate(data->rate);
	tcn.sg_ospeed 	= calcrate(data->rate);

	if (ioctl(ps->sd, TIOCSETN, &tcn))
		return pi_set_error(ps->sd, PI_ERR_GENERIC_SYSTEM);
#endif

#ifdef sleeping_beauty
	s_delay(0, 200000);
#endif
	return 0;
}


/***********************************************************************
 *
 * Function:    pi_serial_impl_init
 *
 * Summary:     initialize function pointers for serial I/O operations
 *
 * Parameters:	struct pi_serial_impl*
 *
 * Returns:     void
 *
 ***********************************************************************/
void
pi_serial_impl_init (struct pi_serial_impl *impl)
{
	impl->open 		= s_open;
	impl->close 		= s_close;
	impl->changebaud 	= s_changebaud;
	impl->write 		= s_write;
	impl->read 		= s_read;
	impl->flush		= s_flush;
	impl->poll 		= s_poll;
}


/***********************************************************************
 *
 * Function:    calcrate
 *
 * Summary:     validates the selected baudrate
 *
 * Paramters:	buadrate
 *
 * Returns:     POSIX defined baudrate constant or terminates the process
 *		if the requested baudrate is not supported.
 *
 ***********************************************************************/
static speed_t
calcrate(int baudrate)
{
#ifdef B50
	if (baudrate == 50)
		return B50;
#endif
#ifdef B75
	if (baudrate == 75)
		return B75;
#endif
#ifdef B110
	if (baudrate == 110)
		return B110;
#endif
#ifdef B134
	if (baudrate == 134)
		return B134;
#endif
#ifdef B150
	if (baudrate == 150)
		return B150;
#endif
#ifdef B200
	if (baudrate == 200)
		return B200;
#endif
#ifdef B300
	if (baudrate == 300)
		return B300;
#endif
#ifdef B600
	if (baudrate == 600)
		return B600;
#endif
#ifdef B1200
	if (baudrate == 1200)
		return B1200;
#endif
#ifdef B1800
	if (baudrate == 1800)
		return B1800;
#endif
#ifdef B2400
	if (baudrate == 2400)
		return B2400;
#endif
#ifdef B4800
	if (baudrate == 4800)
		return B4800;
#endif
#ifdef B9600
	if (baudrate == 9600)
		return B9600;
#endif
#ifdef B19200
	else if (baudrate == 19200)
		return B19200;
#endif
#ifdef B38400
	else if (baudrate == 38400)
		return B38400;
#endif
#ifdef B57600
	else if (baudrate == 57600)
		return B57600;
#endif
#ifdef B76800
	else if (baudrate == 76800)
		return B76800;
#endif
#ifdef B115200
	else if (baudrate == 115200)
		return B115200;
#endif
#ifdef B230400
	else if (baudrate == 230400)
		return B230400;
#endif
#ifdef B460800
	else if (baudrate == 460800)
		return B460800;
#endif

	LOG((PI_DBG_DEV, PI_DBG_LVL_ERR,
		"DEV Serial CHANGEBAUD Unable to set baud rate %d\n",
		baudrate));
	abort();	/* invalid baud rate */
	return 0;
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
