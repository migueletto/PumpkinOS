/*
 * $Id: pi-source.h,v 1.42 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-source.h
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

#ifndef _PILOT_SOURCE_H_
#define _PILOT_SOURCE_H_

# include <sys/ioctl.h>
# include <sys/time.h>
# include <sys/errno.h>
# include <time.h>
# include <fcntl.h>
# include <unistd.h>
# include <string.h>
# include <stdlib.h>
# include <dirent.h>
# include <errno.h>
# include <assert.h>

#ifdef NeXT
# include <sys/types.h>
# include <sys/socket.h>
#endif

#ifdef __EMX__
# define OS2
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/param.h>		/* for htonl .. */
# define ENOMSG 150
# define strcasecmp stricmp
# define strncasecmp strnicmp

# define TTYPrompt "com#"
# define RETSIGTYPE void
# define HAVE_SIGACTION
# define HAVE_DUP2
# define HAVE_SYS_SELECT_H
# define HAVE_STRDUP
#else
#endif

#ifdef SGTTY
# include <sgtty.h>
#else
# include <termios.h>
#endif

#ifndef PI_DEPRECATED
#if __GNUC__ - 0 > 3 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 >= 2)
# define PI_DEPRECATED __attribute__ ((deprecated))
#else
# define PI_DEPRECATED
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "pi-socket.h"
#include "pi-macros.h"
#include "pi-buffer.h"

#define PI_SOCK_LISTEN       0x01       /* Listener 			*/
#define PI_SOCK_CONN_ACCEPT  0x02       /* Connected by accepting 	*/
#define PI_SOCK_CONN_INIT    0x04       /* Connected by initiating 	*/
#define PI_SOCK_CONN_BREAK   0x08       /* Connected but broken 	*/
#define PI_SOCK_CONN_END     0x10       /* Connected but end 		*/
#define PI_SOCK_CLOSE        0x20       /* Closed 			*/

#define PI_FLUSH_INPUT       0x01       /* for flush() 			*/
#define	PI_FLUSH_OUTPUT      0x02       /* for flush() 			*/

	typedef struct pi_protocol {
		int level;
		struct pi_protocol *(*dup)
			PI_ARGS((struct pi_protocol *));
		void (*free)
			PI_ARGS((struct pi_protocol *));
		ssize_t	(*read)
			PI_ARGS((pi_socket_t *ps, pi_buffer_t *buf,
				size_t expect, int flags));
		ssize_t	(*write)
			PI_ARGS((pi_socket_t *ps, PI_CONST unsigned char *buf,
				size_t len, int flags));
		int (*flush)
			PI_ARGS((pi_socket_t *ps, int flags));
	 	int (*getsockopt)
			PI_ARGS((pi_socket_t *ps, int level,
				int option_name, void *option_value,
					size_t *option_len));
		int (*setsockopt)
			PI_ARGS((pi_socket_t *ps, int level,
				int option_name, const void *option_value,
					size_t *option_len));
		void *data;
	} pi_protocol_t;

	typedef struct pi_device {
		void (*free)
			PI_ARGS((struct pi_device *dev));
		struct pi_protocol *(*protocol)
			PI_ARGS((struct pi_device *dev));
		int (*bind)
			PI_ARGS((pi_socket_t *ps,
				struct sockaddr *addr, size_t addrlen));
		int (*listen)
			PI_ARGS((pi_socket_t *ps, int backlog));
		int (*accept)
			PI_ARGS((pi_socket_t *ps, struct sockaddr *addr,
				size_t *addrlen));
		int (*connect)
			PI_ARGS((pi_socket_t *ps, struct sockaddr *addr,
				size_t addrlen));
		int (*close)
			PI_ARGS((pi_socket_t *ps));
		void *data;
	} pi_device_t;
	
	/* internal functions */
	extern pi_socket_list_t *pi_socket_recognize PI_ARGS((pi_socket_t *));
	extern pi_socket_t *find_pi_socket PI_ARGS((int sd));
	extern int crc16 PI_ARGS((unsigned char *ptr, int count));
	extern char *printlong PI_ARGS((uint32_t val));
	extern uint32_t makelong PI_ARGS((char *c));

	/* provide compatibility for old code. Code should now use
	   pi_dumpline() and pi_dumpdata() */

	extern void dumpline
	    PI_ARGS((PI_CONST char *buf, size_t len, unsigned int addr)) PI_DEPRECATED;

	extern void dumpdata
	    PI_ARGS((PI_CONST char *buf, size_t len)) PI_DEPRECATED;


#ifdef __cplusplus
}
#endif
#endif				/* _PILOT_SOURCE_H_ */
