/*
 * $Id: pi-file.h,v 1.29 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-file.h: Pilot File Interface Library
 *
 * This is free software, licensed under the GNU Library Public License V2.
 * See the file COPYING.LIB for details.
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

/** @file pi-file.h
 *  @brief Database file install, retrieve and management interface
 *
 * The pi-file layer is a convenience management library that provides for
 * easy access, creation, install and retrieve of database files (PRC, PDB,
 * PQA)
 *
 * Palm databases on the local machines can be created with pi_file_create()
 * or opened read-only using pi_file_open(). Several functions are provided
 * to access resources and records. Caller must make sure to use the
 * appropriate functions, depending on the type of the database (i.e. only
 * use the record read/write functions on data databases, only use the
 * resource read/write functions on resource databases).
 *
 * A set of utility functions are provided to facilitate installing and
 * retrieving databases on the devide. pi_file_install() will perform all
 * the steps required to install a database on a device. pi_file_merge() can
 * be used to update an existing database or add new records/resources to
 * it.  pi_file_retrieve() will read a database from the device.
 */

#ifndef _PILOT_FILE_H_
#define _PILOT_FILE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "pi-dlp.h"		/* For DBInfo */

/** @brief Structure describing a record or resource entry in a database file */
typedef struct pi_file_entry {
	int 	offset;			/**< Offset in the on-disk file */
	int	size;			/**< Size of the resource or record */
	int	resource_id;		/**< For resources, resource ID */
	int	attrs;			/**< Record attributes */
	uint32_t type;		/**< For resdources, resource type */
	recordid_t uid;			/**< For records, record unique ID */
} pi_file_entry_t;

typedef struct pi_file {
	int 	err;
	int	for_writing;		/**< Non-zero if the file was opened with pi_file_create() */
	int	app_info_size;		/**< Size of the appInfo block */
	int	sort_info_size;		/**< Size of the sortInfo block */
	int	next_record_list_id;
	int	resource_flag;
	int	ent_hdr_size;		/**< Size of the header for each resource/record (depends on whether the file is a resource or record file) */
	int	num_entries;		/**< Number of actual entries in the entries memory block */
	int	num_entries_allocated;	/**< Number of entries allocated in the entries memory block */
	int	rbuf_size;		/**< Size of the internal read buffer */
	FILE 	*f;			/**< Actual on-disk file */
	pi_buffer_t *tmpbuf;		/**< Temporary buffer for databases opened with pi_file_create() */
	char 	*file_name;		/**< Access path */
	void 	*app_info;		/**< Pointer to the appInfo block or NULL */
	void	*sort_info;		/**< Pointer to the sortInfo block or NULL */
	void	*rbuf;			/**< Read buffer, used internally */
	uint32_t unique_id_seed;	/**< Database file's unique ID seed as read from an existing file */
	struct 	DBInfo info;		/**< Database information and attributes */
	struct 	pi_file_entry *entries;	/**< Array of records / resources */
} pi_file_t;

/** @brief Transfer progress callback structure
 *
 * The progress callback structure is prepared by the client application and
 * passed to pi_file_install(), pi_file_merge() and pi_file_retrieve()
 * functions. It allows the client application to be notified during a file
 * transfer (i.e. to update a progress indicator) and gives it a chance to
 * cancel transfers.
 */
typedef struct {
	int type;				/**< Transfer type (see #piProgressType enum) */
	int transferred_bytes;			/**< Current transferred bytes */
	void *userinfo;				/**< Provided by and passed back to client (not used for now, will be in a future release) */
	union {
		struct {
			pi_file_t *pf;		/**< May be NULL */
			struct DBSizeInfo size;	/**< Size information */
			int transferred_records;/**< Number of records or resources */
		} db;

		struct {
			char *path;		/**< VFS file path */
			int32_t total_bytes;	/**< File size in bytes */
		} vfs;
	} data;
} pi_progress_t;

/** @brief Progress callback function definition
 *
 * Callback should return either #PI_TRANSFER_STOP or
 * #PI_TRANSFER_CONTINUE
 */
typedef int (*progress_func)(int socket, pi_progress_t *progress);

/** @brief Transfer progress types for the @a type member of pi_progress_t */
enum piProgressType {
	PI_PROGRESS_SEND_DB = 1,		/**< Sending a database */
	PI_PROGRESS_RECEIVE_DB,			/**< Receiving a database */
	PI_PROGRESS_SEND_VFS,			/**< Sending a VFS file */
	PI_PROGRESS_RECEIVE_VFS			/**< Receiving a VFS file */
};


#define PI_TRANSFER_STOP	0		/**< Returned by progress callback to stop the transfer */
#define	PI_TRANSFER_CONTINUE	1		/**< Returned by progress callback to continue the transfer */

/** @name Opening and closing files */
/*@{*/
	/** @brief Open a database for read-only access
	 *
	 * Don't dispose of the returned structure directly.
	 * Use pi_file_close() instead.
	 *
	 * @param name The access path to the database to open on the local machine
	 * @return An initialized pi_file_t structure or NULL.
	 */
	extern pi_file_t *pi_file_open
		PI_ARGS((const char *name));

	/** @brief Create a new database file
	 *
	 * A new database file is created on the local machine.
	 *
	 * @param name Access path of the new file to create
	 * @param INPUT	Characteristics of the database to create
	 * @return A new pi_file_t structure. Use pi_file_close() to write data and close file.
	 */
	extern pi_file_t *pi_file_create
	    PI_ARGS((const char *name, const struct DBInfo *INPUT));

	/** @brief Closes a local file
	 *
	 * If the file had been opened with pi_file_create, all
	 * modifications are written to disk before the file is closed
	 *
	 * @param pf	The pi_file_t structure is being disposed of by this function
	 * @return An error code (see file pi-error.h)
	 */
	extern int pi_file_close PI_ARGS((pi_file_t *pf));
/*@}*/

/** @name Reading from open files */
/*@{*/
	/** @brief Returns database specification
	 *
	 * @param pf	An open file
	 * @return DBInfo structure describing the file
	 */
	extern void pi_file_get_info
	    PI_ARGS((const pi_file_t *pf, struct DBInfo *OUTPUT));

	/** @brief Returns the file's appInfo block
	 *
	 * The returned data is not a copy of the pi_file_t's appInfo
	 * structures. Don't dispose or modify it.
	 *
	 * @param pf An open file
	 * @param datap On return, ptr to appInfo data or NULL
	 * @param sizep On return, size of the appInfoBlock
	 */
	extern void pi_file_get_app_info
	    PI_ARGS((pi_file_t *pf, void **datap, size_t *sizep));

	/** @brief Returns the file's sortInfo block
	 *
	 * The returned data is not a copy of the pi_file_t's sortInfo
	 * block: do not dispose of it!
	 *
	 * @param pf An open file
	 * @param datap On return, ptr to sortInfo data or NULL
	 * @param sizep On return, size of the sortInfoBlock
	 */
	extern void pi_file_get_sort_info
	    PI_ARGS((pi_file_t *pf, void **dadtap, size_t *sizep));

	/** @brief Read a resource by index
	 *
	 * If it exists, the returned data points directly into the file
	 * structures. Don't dispose or modify it.
	 *
	 * @param pf An open file
	 * @param resindex The resource index
	 * @param bufp On return, pointer to the resource data block
	 * @param sizep If not NULL, size of the resource data
	 * @param restype If not NULL, resource type
	 * @param resid If not NULL, resource
	 * @return Negative error code on error
	 */
	extern int pi_file_read_resource
	    PI_ARGS((pi_file_t *pf, int resindex, void **bufp, size_t *sizep,
		     uint32_t *restype, int *resid));

	/** @brief Read a resource by type and ID
	 *
	 * If it exists, the returned data points directly into the file
	 * structures.
	 *
	 * @param pf An open file
	 * @param restype Resource type
	 * @param resid Resource ID
	 * @param bufp On return, pointer to the resource data or NULL
	 * @param sizep If not NULL, the size of the resource data
	 * @param resindex If not NULL, on return contains the resource index
	 * @return Negative error code on error
	 */
	extern int pi_file_read_resource_by_type_id
	    PI_ARGS((pi_file_t *pf, uint32_t restype, int resid,
		     void **bufp, size_t *sizep, int *resindex));

	/** @brief Checks whether a resource type/id exists in an open file
	 *
	 * @param pf An open file
	 * @param restype Resource type to check
	 * @param resid Resource ID to check
	 * @return Non-zero if a resource with same type and id exists
	 */
	extern int pi_file_type_id_used
	    PI_ARGS((const pi_file_t *pf, uint32_t restype, int resid));

	/** @brief Checks whether a record with the given UID exists
	 *
	 * @param pf An open file
	 * @param uid The record UID to look for
	 * @return Non-zero if a record with the same UID exists
	 */
	extern int pi_file_id_used
	    PI_ARGS((const pi_file_t *pf, recordid_t uid));

	/** @brief Read a record by index
	 *
	 * If it exists, the returned data points directly into the file
	 * structures. Don't dispose or modify it.
	 *
	 * @param pf An open file
	 * @param recindex Record index
	 * @param bufp On return, pointer to the resource data or NULL
	 * @param sizep If not NULL, the size of the resource data
	 * @param recattrs If not NULL, the record attributes
	 * @param category If not NULL, the record category
	 * @param recuid If not NULL, the record unique ID
	 * @return Negative error code on error
	 */
	extern int pi_file_read_record
	    PI_ARGS((pi_file_t *pf, int recindex, void **bufp, size_t *sizep,
		     int *recattrs, int *category, recordid_t *recuid));

	/** @brief Read a record by unique ID
	 *
	 * If it exists, the returned data points directly into the file
	 * structures. Don't dispose or modify it.
	 *
	 * @param pf An open file
	 * @param recuid The record unique ID
	 * @param bufp On return, pointer to the resource data or NULL
	 * @param sizep If not NULL, the size of the resource data
	 * @param recindex If not NULL, the record index
	 * @param recattrs If not NULL, the record attributes
	 * @param category If not NULL, the record category
	 * @return Negative error code on error
	 */
	extern int pi_file_read_record_by_id
	    PI_ARGS((pi_file_t *pf, recordid_t recuid, void **bufp,
		     size_t *sizep, int *recindex, int *recattrs, int *category));

#ifndef SWIG
	extern void pi_file_get_entries
	    PI_ARGS((pi_file_t *pf, int *entries));
#endif
/*@}*/

/** @name Modifying files open for write */
/*@{*/
	/* may call these any time before close (even multiple times) */
	extern int pi_file_set_info
	    PI_ARGS((pi_file_t *pf, const struct DBInfo *infop));

	/** @brief Set the database's appInfo block
	 *
	 * The file takes ownership of the passed data block
	 *
	 * @param pf A file open for write
	 * @param data Pointer to the data or NULL to clear the appInfo block
	 * @param size Size of the new data block
	 * @return Negative code on error
	 */
	extern int pi_file_set_app_info
	    PI_ARGS((pi_file_t *pf, void *data, size_t size));

	/** @brief Set the database's sortInfo block
	 *
	 * The file takes ownership of the passed data block
	 *
	 * @param pf A file open for write
	 * @param data Pointer to the data or NULL to clear the sortInfo block
	 * @param size Size of the new data block
	 * @return Negative code on error
	 */
	extern int pi_file_set_sort_info
	    PI_ARGS((pi_file_t *pf, void *data, size_t size));

	/** @brief Add a resource to a file open for write
	 *
	 * The file takes ownership of the passed data block
	 * Function will return PI_ERR_FILE_ALREADY_EXISTS if resource with
	 * same type/id already exists
	 *
	 * @param pf A file open for write
	 * @param data The resource data
	 * @param size The resource size
	 * @param restype Resource type
	 * @param resid Resource ID
	 * @return Negative code on error.
	 */
	extern int pi_file_append_resource
	    PI_ARGS((pi_file_t *pf, void *data, size_t size,
		     uint32_t restype, int resid));

	/** @brief Add a record to a file open for write
	 *
	 * The file takes ownership of the passed data block Function will
	 * return PI_ERR_FILE_ALREADY_EXISTS if record with same unique ID
	 * already exists in the database
	 *
	 * @param pf A file open for write
	 * @param data The resource data
	 * @param size The resource size
	 * @param recattrs Record attributes
	 * @param category Record category
	 * @param recuid Record unique ID (pass 0 to have recuid automatically allocated)
	 * @return Negative code on error.
	 */
	extern int pi_file_append_record
	    PI_ARGS((pi_file_t *pf, void *buf, size_t size, int recattrs,
		     int category, recordid_t recuid));
/*@}*/

/** @name File transfer utilities */
/*@{*/
	/** @brief Retrieve a file from the handheld
	 *
	 * You must first create the local file using pi_file_create()
	 *
	 * @param pf A file open for write
	 * @param socket Socket to the connected handheld
	 * @param cardno Card number the file resides on (usually 0)
	 * @param report_progress Progress function callback or NULL (see #pi_progress_t structure)
	 * @return Negative code on error
	 */
	extern int pi_file_retrieve
	    PI_ARGS((pi_file_t *pf, int socket, int cardno,
			progress_func report_progress));

	/** @brief Install a new file on the handheld
	 *
	 * You must first open the local file with pi_file_open()
	 *
	 * @param pf An open file
	 * @param socket Socket to the connected handheld
	 * @param cardno Card number to install to (usually 0)
	 * @param report_progress Progress function callback or NULL (see #pi_progress_t structure)
	 * @return Negative code on error
	 */
	extern int pi_file_install
	    PI_ARGS((pi_file_t *pf, int socket, int cardno,
			progress_func report_progress));

	/** @brief Install a new file on the handheld or merge with an existing file
	 *
	 * The difference between this function and pi_file_install() is
	 * that if the file already exists on the handheld, pi_file_merge()
	 * will append data to the existing file. For resource files, all
	 * resources from the local file are sent to the handheld. If
	 * resources with the same type/ID exist in the handheld file, they
	 * are replaced with the new one.
	 *
	 * You must first open the local file with pi_file_open()
	 *
	 * @param pf An open file
	 * @param socket Socket to the connected handheld
	 * @param cardno Card number to install to (usually 0)
	 * @param report_progress Progress function callback or NULL (see #pi_progress_t structure)
	 * @return Negative code on error
	 */
	extern int pi_file_merge
	    PI_ARGS((pi_file_t *pf, int socket, int cardno,
			progress_func report_progress));
/*@}*/

/** @name Time utilities */
/*@{*/
	/** @brief Convert Unix time to Palm OS time
	 *
	 * @param t Unix time value
	 * @return Time value with Palm OS timebase
	 */
	extern uint32_t unix_time_to_pilot_time
	    PI_ARGS((time_t t));

	/** @brief Convert Palm OS time to Unix time
	 *
	 * @param raw_time Time value expressed in Palm OS timebase (seconds from 01-JAN-1904, 00:00)
	 * @return Unix time
	 */
	extern time_t pilot_time_to_unix_time
	    PI_ARGS((uint32_t raw_time));
/*@}*/

#ifdef __cplusplus
}
#endif
#endif
