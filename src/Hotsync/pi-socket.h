/*
 * $Id: pi-socket.h,v 1.72 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-socket.h: Socket-like interface to talk to handhelds
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

/** @file pi-socket.h
 *  @brief Socket-like interface to talk to handhelds
 *
 * pi-socket is the root of the way you'll talk to a device. You'll first
 * create a socket using pi_socket(), then either use pi_bind(), pi_listen()
 * and pi_accept() to accept a connection, or pi_connect() to initiate a
 * network connection.
 *
 * A socket encapsulates the various protocol layers required to talk to the
 * handheld. You can access the various protocol levels using pi_protocol().
 *
 * Each protocol layer has options you can get and set using pi_getsockopt()
 * and pi_setsockopt().
 *
 * It is possible to read and write data using pi_read() and pi_write(),
 * though this usually not necessary. Instead, you will use the functions
 * from pi-dlp.h to talk to the device. They take care of all the low-level
 * stuff.
 *
 * At any time, you can check whether a connection is still established
 * using pi_socket_connected(). After each DLP call, you can call pi_error()
 * to check the latest error code. If the error code was #PI_ERR_DLP_PALMOS,
 * you should call pi_palmos_error() to retrieve the error code returned by
 * the device itself. See the pi-dlp.h documentation for more information.
 *
 * Finally, pi_version() returns the version of the DLP protocol on the
 * device. This can be used to check whether some features are supported,
 * such as VFS calls. See pi-dlp.h for more information.
 *
 * @see pi-dlp.h
 */

#ifndef _PILOT_SOCKET_H_
#define _PILOT_SOCKET_H_

#include <unistd.h>
#include <stdint.h>

#include "pi-args.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "pi-version.h"
#include "pi-sockaddr.h"
#include "pi-buffer.h"
#include "pi-error.h"		/* For PI_ERR */

#define PI_AF_PILOT	0x00

#define PI_SOCK_STREAM	0x0010		/**< Stream socket type, for pi_socket() function */
#define PI_SOCK_RAW	0x0030		/**< Raw socket type, for pi_socket() function */
#define PI_CMD_CMP	0x01		/**< CMD command protocol type (for serial connections) */
#define PI_CMD_NET	0x02		/**< NET protocol type (for inet and USB connections) */
#define PI_CMD_SYS	0x03		/**< SYS protocol type (low-level, debugger connections) */
#define PI_MSG_PEEK	0x01		/**< Use this flag with pi_recv() to 'peek' at the incoming data (will not be removed from input buffer) */
/* deprecated (wasn't used)
#define	PI_MSG_REALLOC	0x02 */ 

/** @brief Protocol types */
enum PiProtocolTypes {
	PI_PF_DEV	= 0x01,		/**< Device-level protocol 	*/
	PI_PF_SLP	= 0x02,		/**< Serial-level protocol 	*/
	PI_PF_SYS	= 0x03,		/**< System-level protocol 	*/
	PI_PF_PADP	= 0x04,		/**< PAD-level protocol 	*/
	PI_PF_NET	= 0x05,		/**< NET-level protocol 	*/
	PI_PF_DLP	= 0x06		/**< DLP-level protocol 	*/
};

/** @brief Protocol levels for the socket's protocol queue */
enum PiOptLevels {
	PI_LEVEL_DEV,			/**< Device level 		*/
	PI_LEVEL_SLP,			/**< Serial link protocol level */
	PI_LEVEL_PADP,			/**< PADP protocol level 	*/
	PI_LEVEL_NET,			/**< NET protocol level 	*/
	PI_LEVEL_SYS,			/**< System protocol level 	*/
	PI_LEVEL_CMP,			/**< CMP protocol level 	*/
	PI_LEVEL_DLP,			/**< Desktop link protocol level*/
	PI_LEVEL_SOCK			/**< Socket level 		*/
};

/** @brief Device level socket options (use pi_getsockopt() and pi_setsockopt()) */
enum PiOptDevice {
	PI_DEV_RATE,
	PI_DEV_ESTRATE,
	PI_DEV_HIGHRATE,
	PI_DEV_TIMEOUT
};

/** @brief Serial link protocol socket options (use pi_getsockopt() and pi_setsockopt()) */
enum PiOptSLP {
	PI_SLP_DEST,
	PI_SLP_LASTDEST,
	PI_SLP_SRC,
	PI_SLP_LASTSRC,
	PI_SLP_TYPE,
	PI_SLP_LASTTYPE,
	PI_SLP_TXID,
	PI_SLP_LASTTXID
};

/** @brief PADP protocol socket options (use pi_getsockopt() and pi_setsockopt()) */
enum PiOptPADP {
	PI_PADP_TYPE,
	PI_PADP_LASTTYPE,
	PI_PADP_FREEZE_TXID,		/**< if set, don't increment txid when receiving a packet. Mainly used by dlp_VFSFileRead() */
	PI_PADP_USE_LONG_FORMAT		/**< if set, use the long packet size format when transmitting */
};

/** @brief CMP protocol socket options (use pi_getsockopt() and pi_setsockopt()) */
enum PiOptCMP {
	PI_CMP_TYPE,
	PI_CMP_FLAGS,
	PI_CMP_VERS,
	PI_CMP_BAUD
};

/** @brief NET protocol socket options (use pi_getsockopt() and pi_setsockopt()) */
enum PiOptNet {
	PI_NET_TYPE,
	PI_NET_SPLIT_WRITES,		/**< if set, write separately the NET header and data */
	PI_NET_WRITE_CHUNKSIZE		/**< size of data chunks if PI_NET_SPLIT_WRITES is set. 0 for no chunking of data */
};

/** @brief Socket level options (use pi_getsockopt() and pi_setsockopt()) */
enum PiOptSock {
	PI_SOCK_STATE,			/**< Socket state (listening, closed, etc.) */
	PI_SOCK_HONOR_RX_TIMEOUT	/**< Set to 1 to honor timeouts when waiting for data. Set to 0 to disable timeout (i.e. during dlp_CallApplication) */
};

struct	pi_protocol;			/* forward declaration */

/** @brief Definition of a socket */
typedef struct pi_socket {
	int sd;				/**< Socket descriptor to pass to other functions */

	int type;			/**< Socket type (i.e #PI_SOCK_STREAM) */
	int protocol;			/**< Protocol (usually #PI_PF_DLP) */
	int cmd;

	struct sockaddr *laddr;		/**< Socket local address */
	size_t laddrlen;		/**< Local address length */
	struct sockaddr *raddr;		/**< Socket remote address */
	size_t raddrlen;		/**< Remote address length */

	struct pi_protocol **protocol_queue;	/**< Ptr to the protocol queue */
	int queue_len;			/**< Protocol queue length */
	struct pi_protocol **cmd_queue;	/**< Ptr to the command queue */
	int cmd_len;			/**< Command queue length */
	struct pi_device *device;	/**< Low-level device we're talking to */

	int state;			/**< Current socket state (initially #PI_SOCK_CLOSE). Use pi_setsockopt() with #PI_SOCK_STATE to set the state. */
	int honor_rx_to;		/**< Honor packet reception timeouts. Set most to 1 of the time to have timeout management on incoming packets. Can be disabled when needed using pi_setsockopt() with #PI_SOCK_HONOR_RX_TIMEOUT. This is used, for example, to disable timeouts in dlp_CallApplication() so that lengthy tasks don't return an error. */
	int command;			/**< true when socket in command state  */
	int accept_to;			/**< timeout value for call to accept() */
	int dlprecord;			/**< Index used for some DLP functions */

	int dlpversion;			/**< version of the DLP protocol running on the device */
	uint32_t maxrecsize;	/**< max record size on the device */

	int last_error;			/**< error code returned by the last dlp_* command */
	int palmos_error;		/**< Palm OS error code returned by the last transaction with the handheld */
} pi_socket_t;

/** @brief Internal sockets chained list */
typedef struct pi_socket_list
{
	pi_socket_t *ps;
	struct pi_socket_list *next;
} pi_socket_list_t;

/** @name Socket management */
/*@{*/
	/** @brief Create a new socket
	 *
	 * Call this function to allocate a new socket which you will later
	 * bind to a specific port
	 *
	 * @param domain Not used. Set to 0.
	 * @param type Socket type (#PI_SOCK_STREAM or #PI_SOCK_RAW)
	 * @param protocol Protocol to use (usually #PI_PF_DLP for #PI_SOCK_STREAM sockets)
	 * @return Socket ID
	 */
	extern int pi_socket PI_ARGS((int domain, int type, int protocol));

	/** @brief Assign a new socket descriptor
	 *
	 * Assign a new socket descriptor to the socket. On platforms that
	 * support it, this function reuses the socket's existing descriptor
	 * after closing it first. In all cases (whether the sd changed or
	 * not), you don't have to close the new @a pi_sd you passed.
	 *
	 * @param ps Socket structure
	 * @param pi_sd New socket descriptor
	 * @return The socket structure's new socket descriptor value or negative on error
	 */
	extern int pi_socket_setsd PI_ARGS((pi_socket_t *ps, int pi_sd));

	/** @brief Get socket name
	 *
	 * Structure needs to have its @a laddr member initialized and valid
	 *
	 * @param pi_sd Socket descriptor
	 * @param remote_addr Will receive the local name
	 * @param namelen On input, the size allocated to receive the name. On output, the actual name length
	 * @return 0 on success, negative on error
	 */
	extern int pi_getsockname
	    PI_ARGS((int pi_sd, struct sockaddr * remote_addr, size_t *namelen));

	/** @brief Get a socket's remote address
	 *
	 * @param pi_sd Socket descriptor
	 * @param remote_addr Will receive the remote address/name
	 * @param namelen On input, maximum name/address length. On output, actual length
	 * @return 0 on success, negative on error.
	 */
	extern int pi_getsockpeer
	    PI_ARGS((int pi_sd, struct sockaddr * remote_addr, size_t *namelen));

	/** @brief Get a socket option
	 *
	 * You can get socket options for various levels of the protocol
	 * stack.  See the options list in #socket.h
	 *
	 * @param pi_sd	Socket descriptor
	 * @param level Protocol level (see #PiOptLevels enum)
	 * @param option_name Option "name" (i.e. #PI_DEV_TIMEOUT at #PI_LEVEL_DEV level)
	 * @param option_value Pointer to the option value
	 * @param option_len Len of the pointed option_value.
	 * @return Negative code on error
	 */
	extern int pi_getsockopt
	    PI_ARGS((int pi_sd, int level, int option_name,
		     void *option_value, size_t *option_len));

	/** @brief Set a socket option
	 *
	 * You can set socket options for various levels of the protocol
	 * stack.  See the options list in #socket.h
	 *
	 * @param pi_sd	Socket descriptor
	 * @param level Protocol level (see #PiOptLevels enum)
	 * @param option_name Option "name" (i.e. #PI_DEV_TIMEOUT at #PI_LEVEL_DEV level)
	 * @param option_value Pointer to the option value
	 * @param option_len Len of the pointed option_value.
	 * @return Negative code on error
	 */
	extern int pi_setsockopt
	    PI_ARGS((int pi_sd, int level, int option_name, 
		     const void *option_value, size_t *option_len));

	/** @brief Retrieve the protocol structure for the given level
	 *
	 * You should rarely need to use this function. It allows retrieving
	 * the protocol structure for any protocol in a socket's protocol
	 * stack
	 *
	 * @param pi_sd Socket descriptor
	 * @param level Protocol level (see #PiOptLevels enum)
	 * @return Protocol structure pointer or NULL if not found
	 */
	extern struct pi_protocol *pi_protocol
	    PI_ARGS((int pi_sd, int level));

	/** @brief Browse the protocol stack
	 *
	 * You should rarely need to use this function. It allows retrieving
	 * the next protocol in the stack, up from lower levels to upper
	 * levels. A protocol stack always has a PI_LEVEL_DEV at bottom, so
	 * you can use pi_protocol() to retrieve the lowest stack level,
	 * then repeatedly call pi_protocol_next() to get the next protocol
	 * in the chain
	 *
	 * @param pi_sd Socket descriptor
	 * @param level Level from which you want to get the next protocol (see #PiOptLevels enum)
	 * @return Protocol structure ptr, or NULL if not found
	 */
	extern struct pi_protocol *pi_protocol_next
	    PI_ARGS((int pi_sd, int level));	
/*@}*/

/** @name Connection management */
/*@{*/
	/** @brief Checks whether a connection is established
	 *
	 * If the socket wasn't found, returns 0 and @a errno is set to
	 * ESRCH.
	 *
	 * @param pi_sd Socket descriptor
	 * @return != 0 if a connection is established
	 */
	extern int pi_socket_connected
		PI_ARGS((int pi_sd));

	/** @brief Connect to a remote server
	 *
	 * Connect to a remote server.
	 *
	 * @param pi_sd Socket descriptor
	 * @param port Port string (see pi_bind() description)
	 * @return Negative on error
	 */
	extern PI_ERR pi_connect
	    PI_ARGS((int pi_sd, const char *port));

	/** @brief Bind the socket to a specific port
	 *
	 * Call this function after creating a new socket with pi_socket()
	 * to bind the socket to a specific port. Recognized port prefixes
	 * are: "serial:", "usb:" and "net:". On Unix platforms, you need to
	 * indicate the /dev entry to bind serial: and usb: to.
	 *
	 * @param pi_sd Socket descriptor
	 * @param port Port string as described above
	 * @return Negative error code on error
	 */
	extern PI_ERR pi_bind
	    PI_ARGS((int pi_sd, const char *port));

	extern PI_ERR pi_listen PI_ARGS((int pi_sd, int backlog));

	/** @brief Wait for a handheld
	 *
	 * This function calls pi_accept_to() with a timeout of 0 (wait
	 * forever). If an error occurs, the socket is closed.
	 *
	 * @param pi_sd Socket descriptor
	 * @param remote_addr Unused. Pass NULL.
	 * @param namelen Unused. Pass NULL.
	 * @return Negative error code on error, returns 0 once a device connects
	 */
	extern PI_ERR pi_accept
	    PI_ARGS((int pi_sd, struct sockaddr * remote_addr,
		     size_t *namelen));

	/** @brief Wait for a handheld
	 *
	 * Wait for a device to connect on the port the socket has been
	 * bound to (using pi_bind()). If an error or timeout occurs, the
	 * socket is closed.
	 *
	 * @param pi_sd Socket descriptor
	 * @param remote_addr Unused. Pass NULL.
	 * @param namelen Unused. Pass NULL.
	 * @param timeout Number of seconds to wait. Pass 0 to wait forever.
	 * @return Negative error code on error, returns 0 once a device connects
	 */
	extern PI_ERR pi_accept_to
	    PI_ARGS((int pi_sd, struct sockaddr * remote_addr, size_t *namelen,
		     int timeout));

	/** @brief Close a socket
	 *
	 * This function closes a socket and disposes of all the internal
	 * structures. If a device is currently connected to this socket,
	 * the connection is interrupted.
	 *
	 * @param pi_sd Socket descriptor
	 * @return Negative error code on error
	 */
	extern int pi_close PI_ARGS((int pi_sd));
/*@}*/

/** @name Low-level data transfers */
/*@{*/
	/** @brief Send data on the given socket
	 *
	 * Perform a synchronous write on the given socket. Writes are
	 * performed through the protocol stack. Therefore, the data you
	 * send will be properly encapsulated in a packet conforming to the
	 * connected protocol (i.e. NET protocol if you're talking to a
	 * network or USB device). Usually, you won't send data directly,
	 * but rather use the dlp_XXX functions to talk to the device.
	 *
	 * @param pi_sd Socket descriptor
	 * @param msg Ptr to the data to send
	 * @param len Size of the data to send
	 * @param flags No write flag defined at this time
	 * @return Number of bytes sent. Negative on error.
	 */
	extern int pi_send
	    PI_ARGS((int pi_sd, PI_CONST void *msg, size_t len, int flags));

	/** @brief Wait for incoming data from the device
	 *
	 * Wait for data sent by the device. Note that this function goes
	 * through the protocol stack, therefore it waits for well-formed
	 * packets and decodes them to extract the data. Usually, you won't
	 * use this function directly. Instead, you'll use the dlp_XXX
	 * functions to talk to the device. Remember that you need to pass a
	 * valid pi_buffer_t (for example one allocated with
	 * pi_buffer_new()).
	 *
	 * @param pi_sd Socket descriptor
	 * @param msg Ptr to a valid pi_buffer_t buffer that will contain the received data
	 * @param len Size of the data we want to read
	 * @param flags Read flags. Use #PI_MSG_PEEK to leave data in the input buffer.
	 * @return Number of bytes read. Negative on error.
	 */
	extern ssize_t pi_recv
	    PI_ARGS((int pi_sd, pi_buffer_t *msg, size_t len, int flags));

	/** @brief Wait for incoming data from the device
	 *
	 * Alias for the pi_recv() function.
	 *
	 * @param pi_sd Socket descriptor
	 * @param msg Ptr to a valid pi_buffer_t buffer that will contain the received data
	 * @param len Size of the data we want to read
	 * @return Number of bytes read. Negative on error.
	 */
	extern ssize_t pi_read PI_ARGS((int pi_sd, pi_buffer_t *msg, size_t len));

	/** @brief Write data on the given socket
	 *
	 * Alias for the pi_send() function.
	 *
	 * @param pi_sd Socket descriptor
	 * @param databuf Ptr to the data to send
	 * @param datasize Size of the data to send
	 * @return Number of bytes sent. Negative on error.
	 */
	extern ssize_t pi_write PI_ARGS((int pi_sd, PI_CONST void *databuf, size_t datasize));

	/** @brief Flush input and/or output bytes
	 *
	 * Flush incoming and/or outgoing data. Most device implementations
	 * currently only support flushing the bytes in the incoming data
	 * buffer, as most writes are synchronous.
	 *
	 * @param pi_sd Socket descriptor
	 * @param flags Mask with valus #PI_FLUSH_INPUT, #PI_FLUSH_OUTPUT.
	 * @return Negative on error
	 */
	extern void pi_flush PI_ARGS((int pi_sd, int flags));
/*@}*/

/** @name Error codes management */
/*@{*/
	/** @brief Return the last error after a low-level or DLP call
	 *
	 * If the socket wasn't found, @a errno is set to ESRCH and the
	 * function returns #PI_ERR_SOCK_INVALID.
	 *
	 * @param pi_sd Socket descriptor
	 * @return Error code or 0 if no error or #PI_ERR_SOCK_INVALID is socket was not found
	 */
	extern int pi_error
		PI_ARGS((int pi_sd));

	/** @brief Set the last error code
	 *
	 * If the socket wasn't found, @a errno is set to ESRCH.  If the
	 * error code is #PI_ERR_GENERIC_MEMORY, @a errno is set to ENOMEM.
	 *
	 * @param pi_sd Socket descriptor
	 * @param error_code Error code to set
	 * @return The error code
	 */
	extern int pi_set_error
		PI_ARGS((int pi_sd, int error_code));

	/** @brief Get the last Palm OS error code the device returned to us
	 *
	 * After a DLP transaction, if you got a #PI_ERR_DLP_PALMOS error,
	 * you should call this function to obtain the error code returned
	 * by the device. It may be either a standard Palm OS error code, or
	 * one of the DLP errors (see #dlpErrors enum) If the socket wasn't
	 * found, @a errno is set to ESRCH and the function returns
	 * #PI_ERR_SOCK_INVALID.
	 *
	 * @param pi_sd Socket descriptor
	 * @return The Palm OS error code or #PI_ERR_SOCK_INVALID if socket was not found
	 */
	extern int pi_palmos_error
		PI_ARGS((int pi_sd));

	/** @brief Set the last Palm OS error code
	 *
	 * If the socket wasn't found, @a errno is set to ESRCH.
	 *
	 * @param pi_sd Socket descriptor
	 * @param error_code Error code to set
	 * @return The error code
	 */
	extern int pi_set_palmos_error
		PI_ARGS((int pi_sd, int error_code));

	/** @brief Clear both the last error code and the last Palm OS error code
	 *
	 * If the socket wasn't found, @a errno is set to ESRCH.
	 *
	 * @param sd Socket descriptor
	 */
	extern void pi_reset_errors
		PI_ARGS((int sd));
/*@}*/

/** @name Miscellaneous functions */
/*@{*/
	/** @brief Return the version of the DLP protocol supported by the device
	 *
	 * Once connected to a handheld, you can call this function to
	 * obtain the version of the DLP protocol it supports. See pi-dlp.h
	 * for information about the various DLP versions.
	 *
	 * @param pi_sd Socket descriptor
	 * @return DLP version or #PI_ERR_SOCK_INVALID if socket was not found
	 */
	extern PI_ERR pi_version PI_ARGS((int pi_sd));
	
	/** @brief Return the maximum size of a database record that can be transferred
	 *
	 * Use this function to obtain the maximum size a database record
	 * can be when transferring it to the device. On-device records may
	 * be larger than what is currently supported by the version of the
	 * DLP protocol that runs on the device. On devices with an
	 * implementation of DLP < 1.4, you'll get 0xFFFF meaning that you
	 * can't transfer records larger than 64k.
	 *
	 * If the socket wasn't found, returns 0 and errno is set to ESRCH.
	 *
	 * @param pi_sd Socket descriptor
	 * @return Maximum record transfer size
	 */
	extern uint32_t pi_maxrecsize PI_ARGS((int pi_sd));

	/** @brief Tickle a stream connection to keep it alive
	 *
	 * Call pi_tickle() at regular intervals to keep the connection
	 * alive.  If you're not sending any command to the device, some
	 * devices will automatically disconnect after some time. Calling
	 * pi_tickle() does keep the connection opened, which can be
	 * necessary if you are writing a conduit that performs lengthy
	 * tasks like retrieving data from the Internet.
	 *
	 * @param pi_sd Socket descriptor
	 * @return An error code if an error occured (see pi-error.h)
	 */
	extern PI_ERR pi_tickle PI_ARGS((int pi_sd));

	/** @brief Set a watchdog that will call pi_tickle() at regular intervals
	 *
	 * The watchdog uses the unix SIGALRM to fire an alarm at regular
	 * intervals. If the socket is still connected when the alarm fires,
	 * pi_tickle() is called to keep the connection alive.
	 *
	 * @param pi_sd Socket descriptor
	 * @param interval Time interval in seconds between alarms
	 * @return 0, or #PI_ERR_SOCK_INVALID if the socket wasn't found
	 */
	extern int pi_watchdog PI_ARGS((int pi_sd, int interval));
/*@}*/

#ifdef __cplusplus
}
#endif
#endif				/* _PILOT_SOCKET_H_ */
