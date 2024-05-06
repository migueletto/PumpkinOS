/*                              
 * $Id: dlp.c,v 1.139 2006/11/07 20:42:36 adridg Exp $
 *
 * dlp.c:  Palm DLP protocol
 *
 * Copyright (c) 1996, 1997, Kenneth Albanowski
 * Copyright (c) 1998-2003, David Desrosiers, JP Rosevear and others
 * Copyright (c) 2004, 2005, 2006, Florent Pillet
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
#include <stdarg.h>
#if 0
#ifdef HAVE_ERRNO_H
	#include <errno.h>
	#ifndef ENOMSG
		#define ENOMSG EINVAL	/* For systems that don't provide ENOMSG. Use EINVAL instead. */
	#endif
#endif
#endif
#include <sys/types.h>
#include <netinet/in.h>

#include "pi-debug.h"
#include "pi-source.h"
#include "pi-dlp.h"
#include "pi-syspkt.h"

#include "sys.h"
#include "debug.h"

#define DLP_REQUEST_DATA(req, arg, offset) &req->argv[arg]->data[offset]
#define DLP_RESPONSE_DATA(res, arg, offset) &res->argv[arg]->data[offset]

#define get_date(ptr) (dlp_ptohdate((ptr)))
#define set_date(ptr,val) (dlp_htopdate((val),(ptr)))

#define	RequireDLPVersion(sd,major,minor)	\
	if (pi_version(sd) < (((major)<<8) | (minor))) \
		return dlpErrNotSupp

/* This constant is being used during dlp_ReadResourceByType, dlp_ReadResourceByIndex, dlp_ReadRecordById, and dlp_ReadRecordByIndex
 * Scott Gruby discovered that on some devices, reading a record that has the maximum record size can lock up the device.
 * We'll read large records in two steps, getting the small amount of remaining data the second time.
 * Scott's tests showed that the value above (100 bytes) was enough of a safeguard to prevent device lockup
 */
#define	RECORD_READ_SAFEGUARD_SIZE	100

/* Define prototypes */
#ifdef PI_DEBUG
static void record_dump (uint32_t recID, unsigned int recIndex,
	int flags, int catID, const char *data, int data_len);
#endif

char *dlp_errorlist[] = {
	"No error",
	"General System error",
	"Illegal Function",
	"Out of memory",
	"Invalid parameter",
	"Not found",
	"None Open",
	"Already Open",
	"Too many Open",
	"Already Exists",
	"Cannot Open",
	"Record deleted",
	"Record busy",
	"Operation not supported",
	"-Unused-",
	"Read only",
	"Not enough space",
	"Limit exceeded",
	"Sync cancelled",
	"Bad arg wrapper",
	"Argument missing",
	"Bad argument size"
};

/* Look at "Error codes" in VFSMgr.h in the Palm SDK for their
   implementation */
char * vfs_errorlist[] = {
	"No error",
	"Buffer Overflow",
	"Generic file error",
	"File reference is invalid",
	"File still open",
	"Permission denied",
	"File or folder already exists",
	"FileEOF",
	"File not found",
	"volumereference is invalid",
	"Volume still mounted",
	"No filesystem",
	"Bad data",
	"Non-empty directory",
	"Invalid path or filename",
	"Volume full - not enough space",
	"Unimplemented",
	"Not a directory",
	"Is a directory",
	"Directory not found",
	"Name truncated"
};

/* Look at "Error codes" in ExpansionMgr.h in the Palm SDK for their
   implementation */
char * exp_errorlist[] = {
	"No error",
	"Unsupported Operation",
	"Not enough Power",
	"Card not present",
	"Invalid slotreference number",
	"Slot deallocated",
	"Card no sector read/write",
	"Card read only",
	"Card bad sector",
	"Protected sector",
	"Not open (slot driver)",
	"still open (slot driver)",
	"Unimplemented",
	"Enumeration empty",
	"Incompatible API version"
};

#ifdef DLP_TRACE
static int dlp_trace = 0;
#endif
static int dlp_version_major = PI_DLP_VERSION_MAJOR;
static int dlp_version_minor = PI_DLP_VERSION_MINOR;

#ifdef PI_DEBUG
	#define Trace(name) \
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO, "DLP sd=%d %s\n", sd, #name));
    #ifdef __GNUC__
        #define TraceX(name,format,...) \
            LOG((PI_DBG_DLP, PI_DBG_LVL_INFO, "DLP sd=%d %s " #format "\n", sd, #name, __VA_ARGS__));
    #else
        #define TraceX(name,format,...) \
            LOG((PI_DBG_DLP, PI_DBG_LVL_INFO, "DLP sd=%d %s\n", sd, #name));
    #endif
#else
	#define Trace(name)
	#define TraceX(name,format,...)
#endif

#ifdef PI_DEBUG
static void record_dump (uint32_t recID, unsigned int recIndex, int flags,
	int catID, const char *data, int data_len)
{
	LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
	    "  ID: 0x%8.8lX, Index: %u, Category: %d\n"
	    "  Flags:%s%s%s%s%s%s (0x%2.2X), and %d bytes:\n",
	    (uint32_t) recID,
	    recIndex,
		catID,
	    (flags & dlpRecAttrDeleted) ? " Deleted" : "",
	    (flags & dlpRecAttrDirty) ? " Dirty" : "",
	    (flags & dlpRecAttrBusy) ? " Busy" : "",
	    (flags & dlpRecAttrSecret) ? " Secret" : "",
	    (flags & dlpRecAttrArchived) ? " Archive" : "",
	    (!flags) ? " None" : "",
	    flags, data_len));
	pi_dumpdata(data, (size_t)data_len);
}
#endif

void
dlp_set_protocol_version(int major, int minor)
{
	dlp_version_major = major;
	dlp_version_minor = minor;
}

/***************************************************************************
 *
 * Function:	dlp_strerror
 *
 * Summary:	lookup text for dlp error
 *
 * Parameters:	error number
 *
 * Returns:     char* to error text string
 *
 ***************************************************************************/
char 
*dlp_strerror(int error)
{
	if (error < 0)
		error = -error;
	
	if ((unsigned int) error >= (sizeof(dlp_errorlist)/(sizeof(char *))))
		return "Unknown error";
	
	return dlp_errorlist[error];
}


/***************************************************************************
 *
 * Function:	dlp_arg_new
 *
 * Summary:	create a dlpArg instance
 *
 * Parameters:	id_, length of data
 *
 * Returns:     dlpArg* or NULL on failure
 *
 ***************************************************************************/
struct dlpArg
*dlp_arg_new (int argID, size_t len) 
{
	struct dlpArg *arg;
	
	arg = (struct dlpArg *)malloc(sizeof (struct dlpArg));

	if (arg != NULL) {
		arg->id_ = argID;
		arg->len = len;
		arg->data = NULL;
		if (len > 0) {
			arg->data = (char *)malloc (len);
			if (arg->data == NULL) {
				free(arg);
				arg = NULL;
			}
		}
	}	
	
	return arg;
}


/***************************************************************************
 *
 * Function:	dlp_arg_free
 *
 * Summary:	frees a dlpArg instance
 *
 * Parameters:	dlpArg*
 *
 * Returns:     void
 *
 ***************************************************************************/
void
dlp_arg_free (struct dlpArg *arg)
{
	if (arg != NULL) {
		if (arg->data != NULL)
			free (arg->data);
		free (arg);
	}
}


/***************************************************************************
 *
 * Function:	dlp_arg_len
 *
 * Summary:	computes aggregate length of data members associated with an
 *		array of dlpArg instances
 *
 * Parameters:	number of dlpArg instances, dlpArg**
 *
 * Returns:     aggregate length or -1 on error
 *
 ***************************************************************************/
int
dlp_arg_len (int argc, struct dlpArg **argv)
{
	int i, len = 0;

	for (i = 0; i < argc; i++) {
		struct dlpArg *arg = argv[i];
		
		/* FIXME: shapiro: should these be < or <= ??? */
		if (arg->len < PI_DLP_ARG_TINY_LEN &&
		    (arg->id_ & (PI_DLP_ARG_FLAG_SHORT | PI_DLP_ARG_FLAG_LONG)) == 0)
			len += 2;
		else if (arg->len < PI_DLP_ARG_SHORT_LEN &&
		         (arg->id_ & PI_DLP_ARG_FLAG_LONG) == 0)
			len += 4;
		else
			len += 6;

		len += arg->len;
	}

	return len;
}


/***************************************************************************
 *
 * Function:	dlp_request_new
 *
 * Summary:	creates a new dlpRequest instance 
 *
 * Parameters:	dlpFunction command, number of dlpArgs, lengths of dlpArgs
 *		data member
 *
 * Returns:     dlpRequest* or NULL if failure
 *
 ***************************************************************************/
struct dlpRequest*
dlp_request_new (enum dlpFunctions cmd, int argc, ...) 
{
	struct dlpRequest *req;
	va_list ap;
	int 	i,
		j;
	
	req = (struct dlpRequest *)malloc (sizeof (struct dlpRequest));

	if (req != NULL) {
		req->cmd = cmd;
		req->argc = argc;
		req->argv = NULL;

		if (argc) {
			req->argv = (struct dlpArg **) malloc (sizeof (struct dlpArg *) * argc);
			if (req->argv == NULL) {
				free(req);
				return NULL;
			}
		}
	
		va_start (ap, argc);
		for (i = 0; i < argc; i++) {
			size_t len;

			len = va_arg (ap, size_t);
			req->argv[i] = dlp_arg_new (PI_DLP_ARG_FIRST_ID + i,
				len);
			if (req->argv[i] == NULL) {
				for (j = 0; j < i; j++)
					dlp_arg_free(req->argv[j]);
				free(req->argv);
				free(req);
				req = NULL;
				break;
			}
		}
		va_end (ap);
	}
	
	return req;	
}


/***************************************************************************
 *
 * Function:	dlp_request_new_with_argid
 *
 * Summary:	creates a new dlpRequest instance with argid
 *
 * Parameters:	dlpFunction command, number of dlpArgs, argid, lengths of
 *		dlpArgs data member
 *
 * Returns:     dlpRequest* or NULL if failure
 *
 ***************************************************************************/
struct dlpRequest*
dlp_request_new_with_argid (enum dlpFunctions cmd, int argid, int argc, ...)
{
	struct dlpRequest *req;
	va_list ap;
	int	i,
		j;
	
	req = (struct dlpRequest *) malloc (sizeof (struct dlpRequest));

	if (req != NULL) {
		req->cmd = cmd;
		req->argc = argc;
		req->argv = NULL;

		if (argc) {
			req->argv = (struct dlpArg **) malloc (sizeof (struct dlpArg *) * argc);
			if (req->argv == NULL) {
				free(req);
				return NULL;
			}
		}

		va_start (ap, argc);
		for (i = 0; i < argc; i++) {
			size_t len;

			len = va_arg (ap, size_t);
			req->argv[i] = dlp_arg_new (argid + i, len);
			if (req->argv[i] == NULL) {
				for (j = 0; j < i; j++)
					dlp_arg_free(req->argv[j]);
				free(req->argv);
				free(req);
				req = NULL;
				break;
			}
		}
		va_end (ap);
	}

	return req;
}


/***************************************************************************
 *
 * Function:	dlp_response_new
 *
 * Summary:	creates a new dlpResponse instance 
 *
 * Parameters:	dlpFunction command, number of dlpArg instances
 *
 * Returns:     dlpResponse* or NULL if failure
 *
 ***************************************************************************/
struct dlpResponse
*dlp_response_new (enum dlpFunctions cmd, int argc) 
{
	struct dlpResponse *res;
	
	res = (struct dlpResponse *) malloc (sizeof (struct dlpResponse));

	if (res != NULL) {

		res->cmd = cmd;
		res->err = dlpErrNoError;
		res->argc = argc;
		res->argv = NULL;

		if (argc) {
			res->argv = (struct dlpArg **) malloc (sizeof (struct dlpArg *) * argc);
			if (res->argv == NULL) {
				free(res);
				return NULL;
			}
			/* zero-out argv so that in case of error during
			   response read, dlp_response_free() won't try to
			   free uninitialized ptrs */
			memset(res->argv, 0, sizeof (struct dlpArg *) * argc);
		}
	}
	
	return res;
}


/***************************************************************************
 *
 * Function:	dlp_response_read
 *
 * Summary:	reads dlp response
 *
 * Parameters:	dlpResonse**, sd
 *
 * Returns:     first dlpArg response length or -1 on error
 *
 ***************************************************************************/
ssize_t
dlp_response_read (struct dlpResponse **res, int sd)
{
	struct dlpResponse *response;
	unsigned char *buf;
	short argid;
	int i;
	ssize_t bytes;
	size_t len;
	pi_buffer_t *dlp_buf;
	
	dlp_buf = pi_buffer_new (DLP_BUF_SIZE);
	if (dlp_buf == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	bytes = pi_read (sd, dlp_buf, dlp_buf->allocated);      /* buffer will grow as needed */
	if (bytes < 0) {
		pi_buffer_free (dlp_buf);
		return bytes;
	}
	if (bytes < 4) {
		/* packet is probably incomplete */
#ifdef DEBUG
		LOG((PI_DBG_DLP, PI_DBG_LVL_ERR,
			"dlp_response_read: response too short (%d bytes)\n",
			bytes));
		if (bytes)
			pi_dumpdata(dlp_buf->data, (size_t)dlp_buf->used);
#endif
		return pi_set_error(sd, PI_ERR_DLP_COMMAND);
	}

	response = dlp_response_new ((enum dlpFunctions)(dlp_buf->data[0] & 0x7f), dlp_buf->data[1]);
	*res = response;

	/* note that in case an error occurs, we do not deallocate the response
	   since callers already do it under all circumstances */
	if (response == NULL) {
		pi_buffer_free (dlp_buf);
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);
	}

	response->err = (enum dlpErrors) get_short (&dlp_buf->data[2]);
	pi_set_palmos_error(sd, (int)response->err);

	/* FIXME: add bounds checking to make sure we don't access past
	 * the end of the buffer in case the data is corrupt */
	buf = dlp_buf->data + 4;
	for (i = 0; i < response->argc; i++) {
		argid = get_byte (buf) & 0x3f;
		if (get_byte(buf) & PI_DLP_ARG_FLAG_LONG) {
			if (pi_version(sd) < 0x0104) {
				/* we received a response from a device indicating that
				   it would have transmitted a >64k data block but DLP
				   versions prior to 1.4 don't have this capacity. In
				   this case (as observed on a T3), there is NO length
				   stored after the argid, it goes straigt to the data
				   contents. We need to report that the data is too large
				   to be transferred.
				*/
				pi_buffer_free (dlp_buf);
				return pi_set_error(sd, PI_ERR_DLP_DATASIZE);
			}
			len = get_long (&buf[2]);
			buf += 6;
		} else if (get_byte(buf) & PI_DLP_ARG_FLAG_SHORT) {
			len = get_short (&buf[2]);
			buf += 4;
		} else {
			argid = get_byte(buf);
			len = get_byte(&buf[1]);
			buf += 2;
		}
		
		response->argv[i] = dlp_arg_new (argid, len);
		if (response->argv[i] == NULL) {
			pi_buffer_free (dlp_buf);
			return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);
		}
		memcpy (response->argv[i]->data, buf, len);
		buf += len;
	}

	pi_buffer_free (dlp_buf);

	return response->argc ? response->argv[0]->len : 0;
}


/***************************************************************************
 *
 * Function:	dlp_request_write
 *
 * Summary:	writes dlp request
 *
 * Parameters:	dlpRequest**, sd
 *
 * Returns:     response length or -1 on error
 *
 ***************************************************************************/
ssize_t
dlp_request_write (struct dlpRequest *req, int sd)
{
	unsigned char *exec_buf, *buf;
	int i;
	size_t len;
	
	len = dlp_arg_len (req->argc, req->argv) + 2;
	exec_buf = (unsigned char *) malloc (sizeof (unsigned char) * len);
	if (exec_buf == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte (&exec_buf[PI_DLP_OFFSET_CMD], req->cmd);
	set_byte (&exec_buf[PI_DLP_OFFSET_ARGC], req->argc);

	buf = &exec_buf[PI_DLP_OFFSET_ARGV];	
	for (i = 0; i < req->argc; i++) {
		struct dlpArg *arg = req->argv[i];
		short argid = arg->id_;
		
		if (arg->len < PI_DLP_ARG_TINY_LEN &&
		    (argid & (PI_DLP_ARG_FLAG_SHORT | PI_DLP_ARG_FLAG_LONG)) == 0) {
			set_byte(&buf[0], argid | PI_DLP_ARG_FLAG_TINY);
			set_byte(&buf[1], arg->len);

			memcpy(&buf[2], arg->data, arg->len);
			buf += arg->len + 2;			
		} else if (arg->len < PI_DLP_ARG_SHORT_LEN &&
		           (argid & PI_DLP_ARG_FLAG_LONG) == 0) {
			set_byte(&buf[0], argid | PI_DLP_ARG_FLAG_SHORT);
			set_byte(&buf[1], 0);
			set_short(&buf[2], arg->len);

			memcpy (&buf[4], arg->data, arg->len);
			buf += arg->len + 4;			
		} else {
			set_byte (&buf[0], argid | PI_DLP_ARG_FLAG_LONG);
			set_byte(&buf[1], 0);
			set_long (&buf[2], arg->len);

			memcpy (&buf[6], arg->data, arg->len);
			buf += arg->len + 6;
		}
	}

	pi_flush(sd, PI_FLUSH_INPUT);

	if ((i = pi_write(sd, exec_buf, len)) < (ssize_t)len) {
		//errno = -EIO;
		if (i >= 0 && i < (ssize_t)len)
			i = -1;
	}

	free (exec_buf);

	return i;
}


/***************************************************************************
 *
 * Function:	dlp_request_free
 *
 * Summary:	frees a dlpRequest instance
 *
 * Parameters:	dlpRequest*
 *
 * Returns:     void
 *
 ***************************************************************************/
void
dlp_request_free (struct dlpRequest *req)
{
	int i;

	if (req == NULL)
		return;

	if (req->argv != NULL) {
		for (i = 0; i < req->argc; i++) {
			if (req->argv[i] != NULL)
				dlp_arg_free (req->argv[i]);
		}
		free (req->argv);
	}

	free (req);
}


/***************************************************************************
 *
 * Function:	dlp_response_free
 *
 * Summary:	frees a dlpResponse instance
 *
 * Parameters:	dlpResponse*
 *
 * Returns:     void
 *
 ***************************************************************************/
void
dlp_response_free (struct dlpResponse *res) 
{
	int i;

	if (res == NULL)
		return;
	
	if (res->argv != NULL) {
		for (i = 0; i < res->argc; i++) {
			if (res->argv[i] != NULL)
				dlp_arg_free (res->argv[i]);
		}
		free (res->argv);
	}

	free (res);	
}


/***************************************************************************
 *
 * Function:	dlp_exec
 *
 * Summary:	writes a dlp request and reads the response
 *
 * Parameters:	dlpResponse*
 *
 * Returns:     the number of response bytes, or -1 on error
 *
 ***************************************************************************/
int
dlp_exec(int sd, struct dlpRequest *req, struct dlpResponse **res)
{
	int bytes, result;
	*res = NULL;

	if ((result = dlp_request_write (req, sd)) < req->argc) {
		LOG((PI_DBG_DLP, PI_DBG_LVL_ERR,
			    "DLP sd:%i dlp_request_write returned %i\n",
			    sd, result));
		//errno = -EIO;
		return result;
	}

	if ((bytes = dlp_response_read (res, sd)) < 0) {
		LOG((PI_DBG_DLP, PI_DBG_LVL_ERR,
			    "DLP sd:%i dlp_response_read returned %i\n",
			    sd, bytes));
		//errno = -EIO;
		return bytes;
	}

	/* Check to make sure the response is for this command */
	if ((*res)->cmd != req->cmd) {
		/* The Palm m130 and Tungsten T return the wrong code for VFSVolumeInfo */
		/* Tungsten T5 (and maybe Treo 650) return dlpFuncEndOfSync for dlpFuncWriteResource */
		/* In some cases, the Tapwave Zodiac returns dlpFuncReadRecord instead of dlpFuncReadRecordEx */
		if ((req->cmd != dlpFuncVFSVolumeInfo || (*res)->cmd != dlpFuncVFSVolumeSize)
			&& req->cmd != dlpFuncWriteResource			/* T5 */
			&& req->cmd != dlpFuncReadRecord			/* Zodiac */
			&& req->cmd != dlpFuncReadRecordEx)			/* Zodiac */
		{
			//errno = -ENOMSG;

			LOG((PI_DBG_DLP, PI_DBG_LVL_DEBUG,
				"dlp_exec: result CMD 0x%02x doesn't match requested cmd 0x%02x\n",
				(unsigned)((*res)->cmd), (unsigned)req->cmd));

			return pi_set_error(sd, PI_ERR_DLP_COMMAND);
		}
	}

	/* Check to make sure there was no error  */
	if ((*res)->err != dlpErrNoError) {
		//errno = -ENOMSG;
		pi_set_palmos_error(sd, (int)((*res)->err));
		return pi_set_error(sd, PI_ERR_DLP_PALMOS);
	}

	return bytes;
}

/* These conversion functions are strictly for use within the DLP layer. 
   This particular date/time format does not occur anywhere else within the
   Palm or its communications. */

/* Notice: 
   The dates in the DLP protocol are expressed as absolute dates/times,
   without any time zone information. For example if a file was created
   on the device at 19:32:48, the time members will be 19, 32 and 48.
   This simplifies things a lot since we don't need to to time zone
   conversions. The functions below convert a breakdown DLP date to and
   from a time_t expressed in the machine's local timezone.
   -- FP */

time_t
dlp_ptohdate(const unsigned char *data)
{
	struct tm t;

	/* Seems like year comes back as all zeros if the date is "empty"
	   (but other fields can vary).  And mktime() chokes on 1900 B.C. 
	   (result of 0 minus 1900), returning -1, which the higher level
	   code can't deal with (passes it straight on to utime(), which
	   simply leaves the file's timestamp "as-is").
	 
	   So, since year 0 appears to mean "no date", we'll return an odd
	   number that works out to precisely one day before the start of
	   the Palm's clock (thus little chance of being run into by any
	   Palm-based time stamp). */

	if (data[0] == 0 && data[1] == 0) {

		/* This original calculation was wrong, and reported one day
		   earlier than it was supposed to report. You can verify
		   this with the following:

			perl -e '$date=localtime(0x83D8FE00); print $date,"\n"'

		return (time_t) 0x83D8FE00;	// Wed Dec 30 16:00:00 1903 GMT

		Here are others, depending on what your system requirements are: 

		return (time_t) 0x83D96E80;	// Thu Dec 31 00:00:00 1903 GMT
		return (time_t) 0x00007080;	// Thu Jan  1 00:00:00 1970 GMT

		Palm's own Conduit Development Kit references using 1/1/1904, 
		so that's what we'll use here until something else breaks
		it.
		*/

		return (time_t) 0x83DAC000;	/* Fri Jan  1 00:00:00 1904 GMT */
	}

	memset(&t, 0, sizeof(t));
	t.tm_sec 	= (int) data[6];
	t.tm_min 	= (int) data[5];
	t.tm_hour 	= (int) data[4];
	t.tm_mday 	= (int) data[3];
	t.tm_mon 	= (int) data[2] - 1;
	t.tm_year 	= (((int)data[0] << 8) | (int)data[1]) - 1900;
	t.tm_isdst 	= -1;

	return mktime(&t);
}

void
dlp_htopdate(time_t time_interval, unsigned char *data)
{				/* @+ptrnegate@ */
	int 	year;
	const struct tm *t;

	/* Fri Jan  1 00:00:00 1904 GMT */
        time_t palm_epoch = 0x83DAC000;

	if (time_interval == palm_epoch) {
		memset(data, 0, 8);
		return;
	}

        t = localtime(&time_interval);
	ASSERT(t != NULL);

	year = t->tm_year + 1900;

	data[7] = (unsigned char) 0;	/* packing spacer */
	data[6] = (unsigned char) t->tm_sec;
	data[5] = (unsigned char) t->tm_min;
	data[4] = (unsigned char) t->tm_hour;
	data[3] = (unsigned char) t->tm_mday;
	data[2] = (unsigned char) (t->tm_mon + 1);
	data[0] = (unsigned char) (year >> 8);
	data[1] = (unsigned char) year;
}

int
dlp_GetSysDateTime(int sd, time_t *t)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	Trace(dlp_GetSysDateTime);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncGetSysDateTime, 0);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);
	
	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	
	if (result > 0) {
		*t = dlp_ptohdate((const unsigned char *)DLP_RESPONSE_DATA (res, 0, 0));

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			"DLP GetSysDateTime %s", ctime(t)));
	}

	dlp_response_free(res);
	
	return result;
}

int
dlp_SetSysDateTime(int sd, time_t t)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_SetSysDateTime,"time=0x%08x",t);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncSetSysDateTime, 1, 8);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);
	
	dlp_htopdate(t, (unsigned char *)DLP_REQUEST_DATA(req, 0, 0));

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	dlp_response_free(res);

	return result;
}

int
dlp_ReadStorageInfo(int sd, int cardno, struct CardInfo *c)
{
	int 	result;
	size_t len1, len2;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_ReadStorageInfo,"cardno=%d",cardno);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncReadStorageInfo, 1, 2);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), cardno);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), 0);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);

	if (result > 0) {
		c->more 	= get_byte(DLP_RESPONSE_DATA(res, 0, 0)) 
			|| (get_byte(DLP_RESPONSE_DATA(res, 0, 3)) > 1);
		c->card 	= get_byte(DLP_RESPONSE_DATA(res, 0, 5));
		c->version 	= get_byte(DLP_RESPONSE_DATA(res, 0, 6));
		c->creation 	= get_date((const unsigned char *)DLP_RESPONSE_DATA(res, 0, 8));
		c->romSize 	= get_long(DLP_RESPONSE_DATA(res, 0, 16));
		c->ramSize 	= get_long(DLP_RESPONSE_DATA(res, 0, 20));
		c->ramFree 	= get_long(DLP_RESPONSE_DATA(res, 0, 24));

		len1 = get_byte(DLP_RESPONSE_DATA(res, 0, 28));
		memcpy(c->name, DLP_RESPONSE_DATA(res, 0, 30), len1);
		c->name[len1] = '\0';

		len2 = get_byte(DLP_RESPONSE_DATA(res, 0, 29));
		memcpy(c->manufacturer, DLP_RESPONSE_DATA(res, 0, 30 + len1),
			len2);
		c->manufacturer[len2] = '\0';

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "DLP Read Cardno: %d, Card Version: %d, Creation time: %s",
		    c->card, c->version, ctime(&c->creation)));
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "  Total ROM: %u, Total RAM: %u, Free RAM: %u\n",
		    c->romSize, c->ramSize, c->ramFree));
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "  Card name: '%s'\n", c->name));
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "  Manufacturer name: '%s'\n", c->manufacturer));
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "  More: %s\n", c->more ? "Yes" : "No"));
	}

	dlp_response_free (res);
	
	return result;
}

int
dlp_ReadSysInfo(int sd, struct SysInfo *s)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	
	Trace(dlp_ReadSysInfo);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncReadSysInfo, 1, 4);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_short (DLP_REQUEST_DATA (req, 0, 0), dlp_version_major);
	set_short (DLP_REQUEST_DATA (req, 0, 2), dlp_version_minor);
	
	result = dlp_exec(sd, req, &res);

	dlp_request_free (req);

	if (result > 0) {
		s->romVersion = get_long (DLP_RESPONSE_DATA (res, 0, 0));
		s->locale = get_long (DLP_RESPONSE_DATA (res, 0, 4));
		/* The 8th byte is a filler byte */
		s->prodIDLength = get_byte (DLP_RESPONSE_DATA (res, 0, 9));
		memcpy(s->prodID, DLP_RESPONSE_DATA(res, 0, 10),
			 s->prodIDLength);

		if (res->argc > 1) {
			/* response added in DLP 1.2 */
			pi_socket_t *ps = find_pi_socket(sd);

			s->dlpMajorVersion =
				 get_short (DLP_RESPONSE_DATA (res, 1, 0));
			s->dlpMinorVersion =
				 get_short (DLP_RESPONSE_DATA (res, 1, 2));
			s->compatMajorVersion =
				 get_short (DLP_RESPONSE_DATA (res, 1, 4));
			s->compatMinorVersion =
				 get_short (DLP_RESPONSE_DATA (res, 1, 6));
			s->maxRecSize =
				get_long  (DLP_RESPONSE_DATA (res, 1, 8));

			/* update socket information */
			ps->dlpversion = (s->dlpMajorVersion << 8) | s->dlpMinorVersion;
			ps->maxrecsize = s->maxRecSize;
		} else {
			s->dlpMajorVersion = 0;
			s->dlpMinorVersion = 0;
			s->compatMajorVersion = 0;
			s->compatMinorVersion = 0;
			s->maxRecSize = 0;
		}

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "DLP ReadSysInfo ROM Ver=0x%8.8lX Locale=0x%8.8lX\n",
		    s->romVersion, s->locale));
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "  Product ID=0x%8.8lX\n", s->prodID));
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "  DLP Major Ver=0x%4.4lX DLP Minor Ver=0x%4.4lX\n",
		    s->dlpMajorVersion, s->dlpMinorVersion));
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "  Compat Major Ver=0x%4.4lX Compat Minor Vers=0x%4.4lX\n",
		    s->compatMajorVersion, s->compatMinorVersion));
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "  Max Rec Size=%d\n", s->maxRecSize));
	} else {
        LOG((PI_DBG_DLP, PI_DBG_LVL_ERR, "dlp_ReadSysInfo error code %d\n", result));
    }

	dlp_response_free (res);
	
	return result;
}

int
dlp_ReadDBList(int sd, int cardno, int flags, int start, pi_buffer_t *info)
{
	int 	result,
		i,
		count;
	struct dlpRequest *req;
	struct dlpResponse *res;
	unsigned char *p;
	struct DBInfo db;

	TraceX(dlp_ReadDBList,"cardno=%d flags=0x%04x start=%d",cardno,flags,start);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncReadDBList, 1, 4);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	pi_buffer_clear (info);
	
	/* `multiple' only supported in DLP 1.2 and above */
	if (pi_version(sd) < 0x0102)
		flags &= ~dlpDBListMultiple;

	set_byte(DLP_REQUEST_DATA(req, 0, 0), (unsigned char) flags);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), (unsigned char) cardno);
	set_short(DLP_REQUEST_DATA(req, 0, 2), start);

	result = dlp_exec (sd, req, &res);

	dlp_request_free(req);
	
	if (result > 0) {
		p = (unsigned char *)DLP_RESPONSE_DATA(res, 0, 0);
		db.more = get_byte(p + 2);
		count = get_byte(p + 3);

		for (i=0; i < count; i++) {
			/* PalmOS 2.0 has additional flag */
			if (pi_version(sd) > 0x0100)
				db.miscFlags = get_byte(p + 5);
			else
				db.miscFlags = 0;

			db.flags	= get_short(p + 6);
			db.type		= get_long(p + 8);
			db.creator      = get_long(p + 12);
			db.version      = get_short(p + 16);
			db.modnum       = get_long(p + 18);
			db.createDate   = get_date(p + 22);
			db.modifyDate   = get_date(p + 30);
			db.backupDate   = get_date(p + 38);
			db.index	= get_short(p + 46);

			memset(db.name, 0, sizeof(db.name));
			strncpy(db.name, (char *)(p + 48), 32);

			LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			    "DLP ReadDBList Name: '%s', Version: %d, More: %s\n",
			    db.name, db.version, db.more ? "Yes" : "No"));
			LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			    "  Creator: '%s'", printlong(db.creator)));
			LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			    " Type: '%s' Flags: %s%s%s%s%s%s%s%s%s%s",
			    printlong(db.type),
			    (db.flags & dlpDBFlagResource) ? "Resource " : "",
			    (db.flags & dlpDBFlagReadOnly) ? "ReadOnly " : "",
			    (db.flags & dlpDBFlagAppInfoDirty) ?
					 "AppInfoDirty " : "",
			    (db.flags & dlpDBFlagBackup) ? "Backup " : "",
			    (db.flags & dlpDBFlagReset) ? "Reset " : "",
			    (db.flags & dlpDBFlagNewer) ? "Newer " : "",
			    (db.flags & dlpDBFlagCopyPrevention) ?
					"CopyPrevention " : "",
			    (db.flags & dlpDBFlagStream) ? "Stream " : "",
			    (db.flags & dlpDBFlagOpen) ? "Open " : "",
			    (!db.flags) ? "None" : ""));
			LOG((PI_DBG_DLP, PI_DBG_LVL_INFO, " (0x%2.2X)\n", db.flags));
			LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			    "  Modnum: %d, Index: %d, Creation date: 0x%08x, %s",
			    db.modnum, db.index, db.createDate, ctime(&db.createDate)));
			LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			    " Modification date: 0x%08x, %s", db.modifyDate, ctime(&db.modifyDate)));
			LOG((PI_DBG_DLP, PI_DBG_LVL_INFO, 
			    " Backup date: 0x%08x, %s", db.backupDate, ctime(&db.backupDate)));

			if (pi_buffer_append(info, &db, sizeof(db)) == NULL) {
				result = pi_set_error(sd, PI_ERR_GENERIC_MEMORY);
				break;
			}

			p += get_byte(p + 4);
		}
	} else {
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
				"Error in dlp_ReadDBList: %d\n", result));
	}

	dlp_response_free (res);

	return result;
}

int
dlp_FindDBInfo(int sd, int cardno, int start, const char *dbname,
	       uint32_t type, uint32_t creator,
	       struct DBInfo *info)
{
	int 	i,
		j;
	pi_buffer_t *buf;

    TraceX(dlp_FindDBInfo,"cardno=%d start=%d",cardno,start);
	pi_reset_errors(sd);

	buf = pi_buffer_new (sizeof (struct DBInfo));
	if (buf == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	if (start < 0x1000) {
		i = start;
		while (dlp_ReadDBList(sd, cardno, 0x80 | dlpDBListMultiple, i, buf) >= 0) {
			for (j=0; j < (int)(buf->used / sizeof(struct DBInfo)); j++) {
				memcpy (info, buf->data + j * sizeof(struct DBInfo), sizeof(struct DBInfo));
				if ((!dbname || strcmp(info->name, dbname) == 0)
					&& (!type || info->type == type)
					&& (!creator || info->creator == creator))
					goto found;
				i = info->index + 1;
			}
		}
		start = 0x1000;
	}

	i = start & 0xFFF;
	while (dlp_ReadDBList(sd, cardno, 0x40 | dlpDBListMultiple, i, buf) >= 0) {
		for (j=0; j < (int)(buf->used / sizeof(struct DBInfo)); j++) {
			memcpy (info, buf->data + j * sizeof(struct DBInfo), sizeof(struct DBInfo));
			if ((!dbname || strcmp(info->name, dbname) == 0)
				&& (!type || info->type == type)
				&& (!creator || info->creator == creator))
			{
				info->index |= 0x1000;
				goto found;
			}
			i = info->index + 1;
		}
	}

	pi_buffer_free (buf);
	return -1;

found:
	pi_buffer_free (buf);
	return 0;
}

/***************************************************************************
 *
 * Function:	dlp_decode_finddb_response
 *
 * Summary:	Response decoding for the three variants of dlp_FindDB
 *
 * Parameters:	None
 *
 * Returns:	Nothing
 *
 ***************************************************************************/
static void
dlp_decode_finddb_response(struct dlpResponse *res, int *cardno, uint32_t *localid,
	int *dbhandle, struct DBInfo *info, struct DBSizeInfo *size)
{
	int arg, argid;
	for (arg = 0; arg < res->argc; arg++) {
		argid = (res->argv[arg]->id_ & 0x7f) - PI_DLP_ARG_FIRST_ID;
		if (argid == 0) {
			if (cardno)
				*cardno = get_byte(DLP_RESPONSE_DATA(res, arg, 0));
			if (localid)
				*localid = get_long(DLP_RESPONSE_DATA(res, arg, 2));
			if (dbhandle)
				*dbhandle = get_long(DLP_RESPONSE_DATA(res, arg, 6));

			if (info) {
				info->more = 0;
				info->miscFlags =
					get_byte(DLP_RESPONSE_DATA(res, arg, 11));
				info->flags =
					get_short(DLP_RESPONSE_DATA(res, arg, 12));
				info->type =
					get_long(DLP_RESPONSE_DATA(res, arg, 14));
				info->creator =
					get_long(DLP_RESPONSE_DATA(res, arg, 18));
				info->version =
					 get_short(DLP_RESPONSE_DATA(res, arg, 22));
				info->modnum =
					get_long(DLP_RESPONSE_DATA(res, arg, 24));
				info->createDate =
					 get_date((const unsigned char *)DLP_RESPONSE_DATA(res, arg, 28));
				info->modifyDate =
					 get_date((const unsigned char *)DLP_RESPONSE_DATA(res, arg, 36));
				info->backupDate =
					 get_date((const unsigned char *)DLP_RESPONSE_DATA(res, arg, 44));
				info->index =
					 get_short(DLP_RESPONSE_DATA(res, arg, 52));

				strncpy(info->name, DLP_RESPONSE_DATA(res, arg, 54), 32);
				info->name[32] = '\0';

				LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
					 "DLP FindDB Name: '%s', "
					 "Version: %d, More: %s\n",
					 info->name, info->version,
					 info->more ? "Yes" : "No"));
				LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
					 "  Creator: '%s'", printlong(info->creator)));
				LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
					 " Type: '%s' Flags: %s%s%s%s%s%s%s%s%s%s",
					 printlong(info->type),
					 (info->flags & dlpDBFlagResource) ?
					"Resource " : "",
					 (info->flags & dlpDBFlagReadOnly) ?
					"ReadOnly " : "",
					 (info->flags & dlpDBFlagAppInfoDirty) ?
					"AppInfoDirty " : "",
					 (info->flags & dlpDBFlagBackup) ?
					"Backup " : "",
					 (info->flags & dlpDBFlagReset) ?
					"Reset " : "",
					 (info->flags & dlpDBFlagNewer) ?
					"Newer " : "",
					 (info->flags & dlpDBFlagCopyPrevention) ?
					"CopyPrevention " : "",
					 (info->flags & dlpDBFlagStream) ?
					"Stream " : "",
					 (info->flags & dlpDBFlagOpen) ?
					"Open " : "",
					 (!info->flags) ? "None" : ""));
				LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
					" (0x%2.2X)\n", info->flags));
				LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
						"  Modnum: %d, Index: %d, "
					"Creation date: %s",
						info->modnum, info->index,
					ctime(&info->createDate)));
				LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
						" Modification date: %s",
					ctime(&info->modifyDate)));
				LOG((PI_DBG_DLP, PI_DBG_LVL_INFO, 
					 " Backup date: %s",
					ctime(&info->backupDate)));
			}
		}
		else if (argid == 1) {
			if (size) {
				size->numRecords =
					get_long(DLP_RESPONSE_DATA(res, arg, 0));
				size->totalBytes =
					get_long(DLP_RESPONSE_DATA(res, arg, 4));
				size->dataBytes =
					get_long(DLP_RESPONSE_DATA(res, arg, 8));
				size->appBlockSize =
					get_long(DLP_RESPONSE_DATA(res, arg, 12));
				size->sortBlockSize =
					get_long(DLP_RESPONSE_DATA(res, arg, 16));
				size->maxRecSize =
					get_long(DLP_RESPONSE_DATA(res, arg, 20));
			}
		}
	}
}

int
dlp_FindDBByName (int sd, int cardno, PI_CONST char *name, uint32_t *localid,
	int *dbhandle, struct DBInfo *info, struct DBSizeInfo *size)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	int flags = 0;
	
	TraceX(dlp_FindDBByName,"cardno=%d name='%s'",cardno,name);
	pi_reset_errors(sd);

	if (pi_version(sd) < 0x0102)
		return pi_set_error(sd, PI_ERR_DLP_UNSUPPORTED);

	req = dlp_request_new(dlpFuncFindDB, 1, 2 + (strlen(name) + 1));
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	if (localid || dbhandle || info)
		flags |= dlpFindDBOptFlagGetAttributes;
	if (size)
		flags |= dlpFindDBOptFlagGetSize;

	set_byte(DLP_REQUEST_DATA(req, 0, 0), flags);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), cardno);
	strcpy(DLP_REQUEST_DATA(req, 0, 2), name);

	result = dlp_exec(sd, req, &res);
	
	dlp_request_free(req);

	if (result > 0)
		dlp_decode_finddb_response(res, NULL, localid, dbhandle, info, size);
	
	dlp_response_free(res);
	
	return result;	
}

int
dlp_FindDBByOpenHandle (int sd, int dbhandle, int *cardno,
	 uint32_t *localid, struct DBInfo *info, struct DBSizeInfo *size)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	int flags = 0;
	
	Trace(dlp_FindDBByOpenHandle);
	pi_reset_errors(sd);

	if (pi_version(sd) < 0x0102)
		return pi_set_error(sd, PI_ERR_DLP_UNSUPPORTED);

	req = dlp_request_new_with_argid(dlpFuncFindDB, 0x21, 1, 2);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	/* Note: there is a bug in HotSync -- requesting the maxRecSize
	 * crashes the device, so we don't. This is supposed to work only
	 * for this variant of FindDB anyway.
	 */
	if (cardno || localid || info)
		flags |= dlpFindDBOptFlagGetAttributes;
	if (size)
		flags |= dlpFindDBOptFlagGetSize;

	set_byte(DLP_REQUEST_DATA(req, 0, 0), flags);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), dbhandle);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	
	if (result > 0)
		dlp_decode_finddb_response(res, cardno, localid, NULL, info, size);
	
	dlp_response_free(res);
	
	return result;	
}

int
dlp_FindDBByTypeCreator (int sd, uint32_t type, uint32_t creator,
 	int start, int latest, int *cardno, uint32_t *localid,
	int *dbhandle, struct DBInfo *info, struct DBSizeInfo *size)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	int flags = 0, search_flags = 0;
	
	TraceX(dlp_FindDBByTypeCreator,"type='%4.4s' creator='%4.4s' start=%d latest=%d",
	    (const char *)&type,(const char *)&creator,start,latest);
	pi_reset_errors(sd);

	if (pi_version(sd) < 0x0102)
		return pi_set_error(sd, PI_ERR_DLP_UNSUPPORTED);

	req = dlp_request_new_with_argid(dlpFuncFindDB, 0x22, 1, 10);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	if (cardno || localid || dbhandle || info)
		flags |= dlpFindDBOptFlagGetAttributes;
	if (size)
		flags |= (dlpFindDBOptFlagGetSize |
			 dlpFindDBOptFlagMaxRecSize);

	if (start)
		search_flags |= dlpFindDBSrchFlagNewSearch;
	if (latest)
		search_flags |= dlpFindDBSrchFlagOnlyLatest;

	
	set_byte(DLP_REQUEST_DATA(req, 0, 0), flags);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), search_flags);
	set_long(DLP_REQUEST_DATA(req, 0, 2), type);
	set_long(DLP_REQUEST_DATA(req, 0, 6), creator);

	result = dlp_exec(sd, req, &res);
	
	dlp_request_free(req);
	
	if (result > 0)
		dlp_decode_finddb_response(res, cardno, localid, dbhandle, info, size);
	
	dlp_response_free(res);
	
	return result;	
}

int
dlp_OpenDB(int sd, int cardno, int mode, PI_CONST char *name, int *dbhandle)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_OpenDB,"'%s'",name);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncOpenDB, 1, 2 + strlen(name) + 1);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), cardno);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), mode);
	strcpy(DLP_REQUEST_DATA(req, 0, 2), name);

	result = dlp_exec(sd, req, &res);
	
	dlp_request_free(req);
	
	if (result > 0) {
		*dbhandle = get_byte(DLP_RESPONSE_DATA(res, 0, 0));

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "-> dlp_OpenDB dbhandle=%d\n", *dbhandle));
	}
	
	dlp_response_free(res);
	
	return result;
}

int
dlp_DeleteDB(int sd, int card, const char *name)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_DeleteDB,"%s",name);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncDeleteDB, 1, 2 + (strlen(name) + 1));
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), card);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), 0);
	strcpy(DLP_REQUEST_DATA(req, 0, 2), name);

	result = dlp_exec(sd, req, &res);
	
	dlp_request_free(req);
	dlp_response_free(res);
	
	return result;
}

int
dlp_CreateDB(int sd, uint32_t creator, uint32_t type, int cardno,
	 int flags, unsigned int version, const char *name, int *dbhandle)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_CreateDB,"'%s' type='%4.4s' creator='%4.4s' flags=0x%04x version=%d",
	    name,(const char *)&type,(const char *)&creator,flags,version);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncCreateDB, 1, 14 + (strlen(name) + 1));
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_long(DLP_REQUEST_DATA(req, 0, 0), creator);
	set_long(DLP_REQUEST_DATA(req, 0, 4), type);
	set_byte(DLP_REQUEST_DATA(req, 0, 8), cardno);
	set_byte(DLP_REQUEST_DATA(req, 0, 9), 0);
	set_short(DLP_REQUEST_DATA(req, 0, 10), flags);
	set_short(DLP_REQUEST_DATA(req, 0, 12), version);
	strcpy(DLP_REQUEST_DATA(req, 0, 14), name);

	result = dlp_exec(sd, req, &res);
	
	dlp_request_free(req);
	
	if (result > 0 && dbhandle) {
		*dbhandle = get_byte(DLP_RESPONSE_DATA(res, 0, 0));

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "DLP CreateDB Handle=%d\n", *dbhandle));
	}
	
	dlp_response_free(res);

	return result;
}

int
dlp_CloseDB(int sd, int dbhandle)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	Trace(dlp_CloseDB);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncCloseDB, 1, 1);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), (unsigned char) dbhandle);

	result = dlp_exec(sd, req, &res);
	
	dlp_request_free(req);	
	dlp_response_free(res);
	
	return result;
}

int
dlp_CloseDB_All(int sd)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	Trace(dlp_CloseDB_All);
	pi_reset_errors(sd);

	req = dlp_request_new_with_argid(dlpFuncCloseDB, 0x21, 0);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	result = dlp_exec(sd, req, &res);
	
	dlp_request_free(req);	
	dlp_response_free(res);
	
	return result;
}

int
dlp_CallApplication(int sd, uint32_t creator, uint32_t type,
		    int action, size_t length, const void *data,
		    uint32_t *retcode, pi_buffer_t *retbuf)
{
	int 	result,
		version = pi_version(sd),
		previous_honor_rx_timeout,
		no_rx_timeout = 0;
	size_t	data_len;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_CallApplication,"type='%4.4s' creator='%4.4s' action=0x%04x dataLength=%d",
        (const char *)&type,(const char *)&creator,action,(int)length);
	pi_reset_errors(sd);
	if (retbuf)
		pi_buffer_clear(retbuf);

	/* we are going to temporarily disable PI_SOCK_HONOR_RX_TIMEOUT
	 * so that lengthy tasks on the device side don't cause a
	 * connection timeout
	 */
	data_len = sizeof(previous_honor_rx_timeout);
	pi_getsockopt(sd, PI_LEVEL_SOCK, PI_SOCK_HONOR_RX_TIMEOUT,
		&previous_honor_rx_timeout, &data_len);

	if (version >= 0x0101) {	/* PalmOS 2.0 call encoding */

		if (length + 22 > DLP_BUF_SIZE) {
			LOG((PI_DBG_DLP, PI_DBG_LVL_ERR,
			     "DLP CallApplication: data too large (>64k)"));
			pi_set_error(sd, PI_ERR_DLP_DATASIZE);
			return -131;
		}

		req = dlp_request_new_with_argid(
				dlpFuncCallApplication, 0x21, 1, 22 + length);
		if (req == NULL)
			return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

		set_long(DLP_REQUEST_DATA(req, 0, 0), creator);
		set_long(DLP_REQUEST_DATA(req, 0, 4), type);
		set_short(DLP_REQUEST_DATA(req, 0, 8), action);
		set_long(DLP_REQUEST_DATA(req, 0, 10), length);
		set_long(DLP_REQUEST_DATA(req, 0, 14), 0);
		set_long(DLP_REQUEST_DATA(req, 0, 18), 0);
		if (length)
			memcpy(DLP_REQUEST_DATA(req, 0, 22), data, length);

		data_len = sizeof(no_rx_timeout);
		pi_setsockopt(sd, PI_LEVEL_SOCK, PI_SOCK_HONOR_RX_TIMEOUT,
			&no_rx_timeout, &data_len);

		result = dlp_exec(sd, req, &res);

		pi_setsockopt(sd, PI_LEVEL_SOCK, PI_SOCK_HONOR_RX_TIMEOUT,
			&previous_honor_rx_timeout, &data_len);

		dlp_request_free(req);

		if (result > 0) {
			data_len = res->argv[0]->len - 16;
			
			if (retcode)
				*retcode = get_long(DLP_RESPONSE_DATA(res, 0, 0));
			if (retbuf)
				pi_buffer_append(retbuf, DLP_RESPONSE_DATA(res, 0, 16), data_len);

			LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			     "DLP CallApplication Result: %u (0x%08x), "
				 "and %d bytes:\n",
			     get_long(DLP_RESPONSE_DATA(res, 0, 0)), 
			     get_long(DLP_RESPONSE_DATA(res, 0, 0)),
			     data_len));
			CHECK(PI_DBG_DLP, PI_DBG_LVL_DEBUG, 
			      pi_dumpdata(DLP_RESPONSE_DATA(res, 0, 16),
					(size_t)data_len));
		}

	} else {		/* PalmOS 1.0 call encoding */

		if (length + 8 > DLP_BUF_SIZE) {
			LOG((PI_DBG_DLP, PI_DBG_LVL_ERR,
			     "DLP CallApplication: data too large (>64k)"));
			pi_set_error(sd, PI_ERR_DLP_DATASIZE);
			return -131;
		}

		req = dlp_request_new (dlpFuncCallApplication, 1, 8 + length);
		if (req == NULL)
			return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

		set_long(DLP_REQUEST_DATA(req, 0, 0), creator);
		set_short(DLP_REQUEST_DATA(req, 0, 4), action);
		set_short(DLP_REQUEST_DATA(req, 0, 6), length);
		memcpy(DLP_REQUEST_DATA(req, 0, 8), data, length);

		data_len = sizeof(no_rx_timeout);
		pi_setsockopt(sd, PI_LEVEL_SOCK, PI_SOCK_HONOR_RX_TIMEOUT,
			&no_rx_timeout, &data_len);

		result = dlp_exec(sd, req, &res);

		pi_setsockopt(sd, PI_LEVEL_SOCK, PI_SOCK_HONOR_RX_TIMEOUT,
			&previous_honor_rx_timeout, &data_len);

		dlp_request_free(req);

		if (result > 0) {
			data_len = res->argv[0]->len - 6;
			if (retcode)
				*retcode = get_short(DLP_RESPONSE_DATA(res, 0, 2));
			if (retbuf)
				pi_buffer_append(retbuf, DLP_RESPONSE_DATA(res, 0, 6), data_len);
			
			LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			     "DLP CallApplication Action: %d Result:"
				" %u (0x%04x), and %d bytes:\n",
			     (int)get_short(DLP_RESPONSE_DATA(res, 0, 0)), 
			     (unsigned int)get_short(DLP_RESPONSE_DATA(res, 0, 2)), 
			     (unsigned int)get_short(DLP_RESPONSE_DATA(res, 0, 2)),
			     data_len));
			CHECK(PI_DBG_DLP, PI_DBG_LVL_DEBUG, 
			      pi_dumpdata(DLP_RESPONSE_DATA(res, 0, 6),
				(size_t)data_len));
		}
	}

	dlp_response_free(res);
	return result;
}

int
dlp_ResetSystem(int sd)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	Trace(dlp_ResetSystem);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncResetSystem, 0);

	result = dlp_exec(sd, req, &res);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	dlp_request_free(req);	
	dlp_response_free(res);
	
	return result;
}

int
dlp_AddSyncLogEntry(int sd, char *entry)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_AddSyncLogEntry,"%s",entry);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncAddSyncLogEntry, 1, strlen(entry) + 1);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	strcpy(DLP_REQUEST_DATA(req, 0, 0), entry);
	
	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);	
	dlp_response_free(res);

	if (result > 0) {
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "DLP AddSyncLogEntry Entry: \n  %s\n", entry));
	}

	return result;
}

int
dlp_ReadOpenDBInfo(int sd, int dbhandle, int *records)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	Trace(dlp_ReadOpenDBInfo);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncReadOpenDBInfo, 1, 1);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
	
	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);

	if (result > 0) {
		if (records)
			*records = get_short(DLP_RESPONSE_DATA(res, 0, 0));
		
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "DLP ReadOpenDBInfo %d records\n", 
		    get_short(DLP_RESPONSE_DATA(res, 0, 0))));
	}
	
	dlp_response_free(res);
	
	return result;
}

int
dlp_SetDBInfo (int sd, int dbhandle, int flags, int clearFlags,
	unsigned int version, time_t createDate, time_t modifyDate,
	time_t backupDate, uint32_t type, uint32_t creator)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
 	
	Trace(dlp_SetDBInfo);
	pi_reset_errors(sd);

	if (pi_version(sd) < 0x0102)
		return pi_set_error(sd, PI_ERR_DLP_UNSUPPORTED);

	req = dlp_request_new(dlpFuncSetDBInfo, 1, 40);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), 0);
	set_short(DLP_REQUEST_DATA(req, 0, 2), clearFlags);
	set_short(DLP_REQUEST_DATA(req, 0, 4), flags);
	set_short(DLP_REQUEST_DATA(req, 0, 6), version);
	set_date((unsigned char *)DLP_REQUEST_DATA(req, 0, 8), createDate);
	set_date((unsigned char *)DLP_REQUEST_DATA(req, 0, 16), modifyDate);
	set_date((unsigned char *)DLP_REQUEST_DATA(req, 0, 24), backupDate);
	set_long(DLP_REQUEST_DATA(req, 0, 32), type);
	set_long(DLP_REQUEST_DATA(req, 0, 36), creator);
	
	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	dlp_response_free(res);
	
	return result;
}

int
dlp_MoveCategory(int sd, int handle, int fromcat, int tocat)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_MoveCategory,"from %d to %d",fromcat,tocat);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncMoveCategory, 1, 4);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), handle);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), fromcat);
	set_byte(DLP_REQUEST_DATA(req, 0, 2), tocat);
	set_byte(DLP_REQUEST_DATA(req, 0, 3), 0);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);	
	dlp_response_free(res);

	if (result >= 0) {
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "DLP MoveCategory Handle: %d, From: %d, To: %d\n",
		    handle, fromcat, tocat));
	}

	return result;
}

int
dlp_OpenConduit(int sd)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	Trace(dlp_OpenConduit);
	pi_reset_errors(sd);
	
	req = dlp_request_new(dlpFuncOpenConduit, 0);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);	
	dlp_response_free(res);
	
	/* if this was not done yet, this will read and cache the DLP version
	   that the Palm is running. We need this when reading responses during
	   record/resource transfers */
	if (result >= 0)
		pi_version(sd);

	return result;
}

int
dlp_EndOfSync(int sd, int status)
{
	int 	result;
	pi_socket_t	*ps;
	struct dlpRequest *req;
	struct dlpResponse *res;

	Trace(dlp_EndOfSync);
	pi_reset_errors(sd);

	ps = find_pi_socket(sd);
	if (ps == NULL) {
		//errno = ESRCH;
		return PI_ERR_SOCK_INVALID;
	}

	req = dlp_request_new(dlpFuncEndOfSync, 1, 2);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_short(DLP_REQUEST_DATA(req, 0, 0), status);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);	
	dlp_response_free(res);

	/* Messy code to set end-of-sync flag on socket 
	   so pi_close won't do it for us */
	if (result == 0)
		ps->state = PI_SOCK_CONN_END;

	return result;
}

int
dlp_AbortSync(int sd)
{
	pi_socket_t	*ps;

	Trace(dlp_AbortSync);
	pi_reset_errors(sd);

	/* Pretend we sent the sync end */
	if ((ps = find_pi_socket(sd)) == NULL) {
		//errno = ESRCH;
		return PI_ERR_SOCK_INVALID;
	}

	ps->state = PI_SOCK_CONN_END;

	return 0;
}

int
dlp_WriteUserInfo(int sd, const struct PilotUser *User)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	int len;
	
	Trace(dlp_WriteUserInfo);
	pi_reset_errors(sd);

	len = strlen (User->username) + 1;
	
	req = dlp_request_new (dlpFuncWriteUserInfo, 1, 22 + len);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_long(DLP_REQUEST_DATA(req, 0, 0), User->userID);
	set_long(DLP_REQUEST_DATA(req, 0, 4), User->viewerID);
	set_long(DLP_REQUEST_DATA(req, 0, 8), User->lastSyncPC);
	set_date((unsigned char *)DLP_REQUEST_DATA(req, 0, 12), User->lastSyncDate);
	set_byte(DLP_REQUEST_DATA(req, 0, 20), 0xff);
	set_byte(DLP_REQUEST_DATA(req, 0, 21), len);
	strcpy(DLP_REQUEST_DATA(req, 0, 22), User->username);

	result = dlp_exec (sd, req, &res);

	dlp_request_free (req);
	dlp_response_free (res);
	
	return result;
}

int
dlp_ReadUserInfo(int sd, struct PilotUser *User)
{
	int 	result;
	size_t	userlen;
	struct dlpRequest *req;
	struct dlpResponse *res;
	
	Trace(dlp_ReadUserInfo);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncReadUserInfo, 0);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);
	
	result = dlp_exec (sd, req, &res);

	dlp_request_free (req);

	if (result > 0) {
		User->userID =
			 get_long(DLP_RESPONSE_DATA (res, 0, 0));
		User->viewerID =
			 get_long(DLP_RESPONSE_DATA (res, 0, 4));
		User->lastSyncPC =
			 get_long(DLP_RESPONSE_DATA (res, 0, 8));
		User->successfulSyncDate =
			 get_date((const unsigned char *)DLP_RESPONSE_DATA (res, 0, 12));
		User->lastSyncDate =
			 get_date((const unsigned char *)DLP_RESPONSE_DATA (res, 0, 20));
		userlen = 
			get_byte(DLP_RESPONSE_DATA (res, 0, 28));
		User->passwordLength  =
			 get_byte(DLP_RESPONSE_DATA (res, 0, 29));

		memcpy(User->username,
			 DLP_RESPONSE_DATA (res, 0, 30), userlen);
		memcpy(User->password,
			 DLP_RESPONSE_DATA (res, 0, 30 + userlen),
				User->passwordLength);

		if (userlen < sizeof(User->username))
			User->username[userlen] = '\0';
		if (User->passwordLength < sizeof(User->password))
			User->password[User->passwordLength] = '\0';

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    	"DLP ReadUserInfo UID=0x%8.8lX VID=0x%8.8lX "
			"PCID=0x%8.8lX\n",
			    User->userID, User->viewerID, User->lastSyncPC));
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			"  Last Sync=%s  Last Successful Sync=%s",
		    	ctime (&User->lastSyncDate),
			ctime (&User->successfulSyncDate)));
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		 	"  Username=%s\n", User->username));
	}
	
	dlp_response_free (res);
	
	return result;
}

int
dlp_ReadNetSyncInfo(int sd, struct NetSyncInfo *i)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	Trace(dlp_ReadNetSyncInfo);
	pi_reset_errors(sd);

	if (pi_version(sd) < 0x0101)
		return pi_set_error(sd, PI_ERR_DLP_UNSUPPORTED);

	req = dlp_request_new(dlpFuncReadNetSyncInfo, 0);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	result = dlp_exec (sd, req, &res);

	dlp_request_free(req);

	if (result >= 0) {
		size_t str_offset = 24;
		
		i->lanSync = get_byte(DLP_RESPONSE_DATA(res, 0, 0));
		
		i->hostName[0] = '\0';
		memcpy(i->hostName, DLP_RESPONSE_DATA(res, 0, str_offset), 
		       get_short(DLP_RESPONSE_DATA(res, 0, 18)));
		str_offset += get_short(DLP_RESPONSE_DATA(res, 0, 18));

		i->hostAddress[0] = '\0';
		memcpy(i->hostAddress, DLP_RESPONSE_DATA(res, 0, str_offset), 
		       get_short(DLP_RESPONSE_DATA(res, 0, 20)));
		str_offset += get_short(DLP_RESPONSE_DATA(res, 0, 20));

		i->hostSubnetMask[0] = '\0';
		memcpy(i->hostSubnetMask, DLP_RESPONSE_DATA(res, 0, str_offset), 
		       get_short(DLP_RESPONSE_DATA(res, 0, 22)));

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "DLP ReadNetSyncInfo Active: %d\n", i->lanSync ? 1 : 0));
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "  PC hostname: '%s', address '%s', mask '%s'\n",
		    i->hostName, i->hostAddress, i->hostSubnetMask));
	}

	dlp_response_free(res);
	
	return result;
}

int
dlp_WriteNetSyncInfo(int sd, const struct NetSyncInfo *i)
{
	int 	result,
		str_offset = 24;
	struct dlpRequest *req;
	struct dlpResponse *res;

	Trace(dlp_WriteNetSyncInfo);
	pi_reset_errors(sd);

	if (pi_version(sd) < 0x0101)
		return pi_set_error(sd, PI_ERR_DLP_UNSUPPORTED);

	LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
	    "DLP ReadNetSyncInfo Active: %d\n", i->lanSync ? 1 : 0));
	LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
	    "  PC hostname: '%s', address '%s', mask '%s'\n",
	    i->hostName, i->hostAddress, i->hostSubnetMask));

	req = dlp_request_new(dlpFuncWriteNetSyncInfo, 1,
		24 + strlen(i->hostName) + 
		strlen(i->hostAddress) + strlen(i->hostSubnetMask) + 3);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	/* Change all settings */
	set_byte(DLP_REQUEST_DATA(req, 0, 0), 0x80 | 0x40 | 0x20 | 0x10);

	set_byte(DLP_REQUEST_DATA(req, 0, 1), i->lanSync);
	set_long(DLP_REQUEST_DATA(req, 0, 2), 0);  /* Reserved1 */
	set_long(DLP_REQUEST_DATA(req, 0, 6), 0);  /* Reserved2 */
	set_long(DLP_REQUEST_DATA(req, 0, 10), 0); /* Reserved3 */
	set_long(DLP_REQUEST_DATA(req, 0, 14), 0); /* Reserved4 */
	set_short(DLP_REQUEST_DATA(req, 0, 18), strlen(i->hostName) + 1);
	set_short(DLP_REQUEST_DATA(req, 0, 20), strlen(i->hostAddress) + 1);
	set_short(DLP_REQUEST_DATA(req, 0, 22), strlen(i->hostSubnetMask) + 1);

	strcpy(DLP_REQUEST_DATA(req, 0, str_offset), i->hostName);
	str_offset += strlen(i->hostName) + 1;
	strcpy(DLP_REQUEST_DATA(req, 0, str_offset), i->hostAddress);
	str_offset += strlen(i->hostAddress) + 1;
	strcpy(DLP_REQUEST_DATA(req, 0, str_offset), i->hostSubnetMask);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	dlp_response_free(res);
	
	return result;
}

#ifdef _PILOT_SYSPKT_H
int
dlp_RPC(int sd, struct RPC_params *p, uint32_t *result)
{
	int 	i,
		err = 0;
	int32_t 	D0 = 0,
		A0 = 0;
	unsigned char *c;
	pi_buffer_t *dlp_buf;

	Trace(dlp_RPC);
	pi_reset_errors(sd);

	/* RPC through DLP breaks all the rules and isn't well documented to
	   boot */
	dlp_buf = pi_buffer_new (DLP_BUF_SIZE);
	if (dlp_buf == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	dlp_buf->data[0] = 0x2D;
	dlp_buf->data[1] = 1;
	dlp_buf->data[2] = 0;		/* Unknown filler */
	dlp_buf->data[3] = 0;

	InvertRPC(p);

	set_short(dlp_buf->data + 4, p->trap);
	set_long(dlp_buf->data + 6, D0);
	set_long(dlp_buf->data + 10, A0);
	set_short(dlp_buf->data + 14, p->args);

	c = dlp_buf->data + 16;
	for (i = p->args - 1; i >= 0; i--) {
		set_byte(c, p->param[i].byRef);
		c++;
		set_byte(c, p->param[i].size);
		c++;
		if (p->param[i].data)
			memcpy(c, p->param[i].data, p->param[i].size);
		c += p->param[i].size;
		if (p->param[i].size & 1)
			*c++ = 0;
	}

	if (pi_write(sd, dlp_buf->data, (size_t)(c - dlp_buf->data)) > 0) {
		err = 0;
		if (p->reply) {
			int l = pi_read(sd, dlp_buf, (size_t)(c - dlp_buf->data + 2));

			if (l < 0)
				err = l;
			else if (l < 6)
				err = -1;
			else if (dlp_buf->data[0] != 0xAD)
				err = -2;
			else if (get_short(dlp_buf->data + 2)) {
				err = -get_short(dlp_buf->data + 2);
				pi_set_palmos_error(sd, -err);
			} else {
				D0 = get_long(dlp_buf->data + 8);
				A0 = get_long(dlp_buf->data + 12);
				c = dlp_buf->data + 18;
				for (i = p->args - 1; i >= 0; i--) {
					if (p->param[i].byRef && p->param[i].data)
						memcpy(p->param[i].data, c + 2,
							   p->param[i].size);
					c += 2 + ((p->param[i].size + 1) & 
							(unsigned)~1);
				}
			}
		}
	}

	pi_buffer_free (dlp_buf);

	UninvertRPC(p);

	if (result) {
		if (p->reply == RPC_PtrReply) {
			*result = A0;
		} else if (p->reply == RPC_IntReply) {
			*result = D0;
		}
	}

	return err;
}


int
dlp_ReadFeature(int sd, uint32_t creator, int num,
		uint32_t *feature)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_ReadFeature,"creator='%4.4s' num=%d",(const char *)&creator,num);
	pi_reset_errors(sd);

	if (pi_version(sd) < 0x0101) {
		struct RPC_params p;		
		int val;
		uint32_t errCode;

		if (feature == NULL)
			return 0;

		*feature = 0x12345678;

		PackRPC(&p, 0xA27B, RPC_IntReply, RPC_Long(creator), RPC_Short(num), RPC_LongPtr(feature), RPC_End);

		val = dlp_RPC(sd, &p, &errCode);

		if (val < 0) {
			LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			    "DLP ReadFeature Error: %s (%d)\n",
			    dlp_errorlist[-val], val));

			return val;
		}
		
		if (errCode) {
			LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			    "DLP ReadFeature FtrGet error 0x%8.8lX\n",
			    res));
			pi_set_palmos_error(sd, (int)errCode);
			return pi_set_error(sd, PI_ERR_DLP_PALMOS);
		}

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    " DLP ReadFeature Feature: 0x%8.8lX\n",
		    (uint32_t) *feature));
		
		return 0;
	}

	Trace(dlp_ReadFeatureV2);

	req = dlp_request_new(dlpFuncReadFeature, 1, 6);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_long(DLP_REQUEST_DATA(req, 0, 0), creator);
	set_short(DLP_REQUEST_DATA(req, 0, 4), num);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	
	if (result > 0) {
		if (feature)
			*feature = (uint32_t)
			 get_long(DLP_RESPONSE_DATA(res, 0, 0));
		
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			"DLP ReadFeature Feature: 0x%8.8lX\n",
			(uint32_t)
			get_long(DLP_RESPONSE_DATA(res, 0, 0))));
	}

	dlp_response_free(res);

	return result;
}
#endif				/* IFDEF _PILOT_SYSPKT_H */

int
dlp_GetROMToken(int sd, uint32_t token, void *buffer, size_t *size)
{
	uint32_t result;

	struct RPC_params p;
	
	int val;
	uint32_t buffer_ptr;

	Trace(dlp_GetROMToken);
	pi_reset_errors(sd);
	
#ifdef DLP_TRACE
	if (dlp_trace) {
	  debug(DEBUG_TRACE, SYS_DEBUG,
		  " Wrote: Token: '%s'\n",
		  printlong(token));
	}
#endif

	PackRPC(&p, 0xa340, RPC_IntReply,		/* sysTrapHwrGetROMToken */
		RPC_Short(0),
		RPC_Long(token),
		RPC_LongPtr(&buffer_ptr),
		RPC_ShortPtr(size), RPC_End);
	
	val = dlp_RPC(sd, &p, &result);

#ifdef DLP_TRACE
	if (dlp_trace) {
	  if (val < 0)
	    debug(DEBUG_TRACE, SYS_DEBUG,
		    "Result: Error: %s (%d)",
		    dlp_errorlist[-val], val);
	  else if (result)
	    debug(DEBUG_TRACE, SYS_DEBUG,
		    "FtrGet error 0x%8.8lX",
		    (uint32_t) result);
	  else
	    debug(DEBUG_TRACE, SYS_DEBUG,
		    "Read: Buffer Ptr: 0x%8.8lX Size: %d",
		    (uint32_t) buffer_ptr, *size);
	}
#endif		

	if (buffer) {
		((unsigned char *)buffer)[*size] = 0;

		PackRPC(&p, 0xa026, RPC_IntReply,		/* sysTrapMemMove */
		  RPC_Ptr(buffer, *size),
		  RPC_Long(buffer_ptr),
		  RPC_Long((uint32_t) *size), 
		  RPC_End);

		val = dlp_RPC(sd, &p, &result);
	}

#ifdef DLP_TRACE
	if (dlp_trace) {
	  if (val < 0)
	    debug(DEBUG_TRACE, SYS_DEBUG,
		    "Result: Error: %s (%d)",
		    dlp_errorlist[-val], val);
	  else if (result)
	    debug(DEBUG_TRACE, SYS_DEBUG,
		    "FtrGet error 0x%8.8lX",
		    (uint32_t) result);
	  else
	    debug(DEBUG_TRACE, SYS_DEBUG,
		    "Read: Buffer: %s", buffer);
	}
#endif	

	if (val < 0)
		return val;
	
	if (result)
		return -((int)result);

	return result;
}

int
dlp_ResetLastSyncPC(int sd)
{
	int 	err;
	struct 	PilotUser User;

	Trace(dlp_ResetLastSyncPC);

	if ((err = dlp_ReadUserInfo(sd, &User)) < 0)
		return err;

	User.lastSyncPC = 0;

	return dlp_WriteUserInfo(sd, &User);
}

int
dlp_ResetDBIndex(int sd, int dbhandle)
{
	int 	result;
	pi_socket_t	*ps;
	struct dlpRequest *req;
	struct dlpResponse *res;

	Trace(dlp_ResetDBIndex);
	pi_reset_errors(sd);

	if ((ps = find_pi_socket(sd)) == NULL) {
		//errno = ESRCH;
		return PI_ERR_SOCK_INVALID;
	}

	ps->dlprecord = 0;

	req = dlp_request_new(dlpFuncResetRecordIndex, 1, 1);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
	
	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);	
	dlp_response_free(res);
	
	return result;
}

int
dlp_ReadRecordIDList(int sd, int dbhandle, int sort, int start, int max,
		     recordid_t * IDs, int *count)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_ReadRecordIDList,"sort=%d start=%d max=%d",
	    sort,start,max);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncReadRecordIDList, 1, 6);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), sort ? 0x80 : 0);
	set_short(DLP_REQUEST_DATA(req, 0, 2), start);
	set_short(DLP_REQUEST_DATA(req, 0, 4), max);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);

	if (result > 0) {
		int ret, i;
		
		ret = get_short(DLP_RESPONSE_DATA(res, 0, 0));
		for (i = 0; i < ret; i++)
			IDs[i] =
			 get_long(DLP_RESPONSE_DATA(res, 0, 2 + (i * 4)));

		if (count)
			*count = ret;

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "DLP ReadRecordIDList %d IDs:\n", ret));
		CHECK(PI_DBG_DLP, PI_DBG_LVL_DEBUG, 
		      pi_dumpdata(DLP_RESPONSE_DATA(res, 0, 2), 
			(size_t)(ret * 4)));
	}

	dlp_response_free(res);
	
	return result;
}

int
dlp_WriteRecord(int sd, int dbhandle, int flags, recordid_t recID,
		int catID, const void *data, size_t length, recordid_t *pNewRecID)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	Trace(dlp_WriteRecord);
	pi_reset_errors(sd);

	if (length == (size_t)-1)
		length = strlen((char *) data) + 1;

	if (pi_version(sd) >= 0x0104) {
		req = dlp_request_new(dlpFuncWriteRecordEx, 1, 12 + length);
		if (req == NULL)
			return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

		set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
		set_byte(DLP_REQUEST_DATA(req, 0, 1), 0x80);	/* "data included" */
		set_long(DLP_REQUEST_DATA(req, 0, 2), recID);
		set_byte(DLP_REQUEST_DATA(req, 0, 6), flags);
		set_byte(DLP_REQUEST_DATA(req, 0, 7), catID);
		set_long(DLP_REQUEST_DATA(req, 0, 8), 0);

		memcpy(DLP_REQUEST_DATA(req, 0, 12), data, length);
	} else {
		if ((length + 8) > DLP_BUF_SIZE) {
			LOG((PI_DBG_DLP, PI_DBG_LVL_ERR,
			     "DLP WriteRecord: data too large (>64k)"));
			return PI_ERR_DLP_DATASIZE;
		}

		req = dlp_request_new(dlpFuncWriteRecord, 1, 8 + length);
		if (req == NULL)
			return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

		set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
		set_byte(DLP_REQUEST_DATA(req, 0, 1), 0x80);	/* "data included" */
		set_long(DLP_REQUEST_DATA(req, 0, 2), recID);
		set_byte(DLP_REQUEST_DATA(req, 0, 6), flags);
		set_byte(DLP_REQUEST_DATA(req, 0, 7), catID);

		memcpy(DLP_REQUEST_DATA(req, 0, 8), data, length);
	}

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);

	if (result > 0) {
		if (pNewRecID)
			*pNewRecID = get_long(DLP_RESPONSE_DATA(res, 0, 0));

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "DLP WriteRecord Record ID: 0x%8.8lX\n",
		    get_long(DLP_RESPONSE_DATA(res, 0, 0))));

		CHECK(PI_DBG_DLP, PI_DBG_LVL_DEBUG,
			record_dump(
				get_long(DLP_RESPONSE_DATA(res, 0, 0)), /* recID */
				0xffff,									/* index */
				flags,
				catID,
				(const char *)data, (int)length));
	}
	
	dlp_response_free(res);
	
	return result;
}

int
dlp_DeleteRecord(int sd, int dbhandle, int all, recordid_t recID)
{
	int 	result,
		flags = all ? 0x80 : 0;
	struct dlpRequest *req;
	struct dlpResponse *res;

	Trace(dlp_DeleteRecord);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncDeleteRecord, 1, 6);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), flags);
	set_long(DLP_REQUEST_DATA(req, 0, 2), recID);

	result = dlp_exec(sd, req, &res);
	
	dlp_request_free(req);
	dlp_response_free(res);
	
	return result;
}

int
dlp_DeleteCategory(int sd, int dbhandle, int category)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_DeleteCategory,"category=%d",category);
	pi_reset_errors(sd);

	if (pi_version(sd) < 0x0101) {
		/* Emulate if not connected to PalmOS 2.0 */
		int i, cat, attr;
		recordid_t id_;

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "DLP DeleteCategory Emulating with: Handle: %d, "
			"Category: %d\n",
		    dbhandle, category & 0xff));

		for (i = 0;
		     (result = dlp_ReadRecordByIndex(sd, dbhandle, i, NULL, &id_,
					   &attr, &cat)) >= 0; i++) {
			if (cat != category
				|| (attr & dlpRecAttrDeleted)
			    || (attr & dlpRecAttrArchived))
				continue;
			result = dlp_DeleteRecord(sd, dbhandle, 0, id_);
			if (result < 0)
				break;
			i--; /* Sigh, deleting record moves it to the end. */
		}

		return result;
	} else {
		int flags = 0x40;
		
		req = dlp_request_new(dlpFuncDeleteRecord, 1, 6);
		if (req == NULL)
			return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);
		
		set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
		set_byte(DLP_REQUEST_DATA(req, 0, 1), flags);
		set_long(DLP_REQUEST_DATA(req, 0, 2), category & 0xff);
		
		result = dlp_exec(sd, req, &res);

		dlp_request_free(req);
		dlp_response_free(res);

		return result;
	}
}

int
dlp_ReadResourceByType(int sd, int dbhandle, uint32_t type, int resID,
		       pi_buffer_t *buffer, int *resindex)
{
	int 	result,
		data_len;
	struct dlpRequest *req;
	struct dlpResponse *res;
	int maxBufferSize = pi_maxrecsize(sd) - RECORD_READ_SAFEGUARD_SIZE;

	TraceX(dlp_ReadResourceByType,"type='%4.4s' resID=%d",(const char *)&type,resID);
	pi_reset_errors(sd);

	req = dlp_request_new_with_argid(dlpFuncReadResource, 0x21, 1, 12);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), 0);
	set_long(DLP_REQUEST_DATA(req, 0, 2), type);
	set_short(DLP_REQUEST_DATA(req, 0, 6), resID);
	set_short(DLP_REQUEST_DATA(req, 0, 8), 0);
	set_short(DLP_REQUEST_DATA(req, 0, 10), buffer ? maxBufferSize : 0);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	
	if (result > 0) {
		data_len = res->argv[0]->len - 10;
		if (resindex)
			*resindex = get_short(DLP_RESPONSE_DATA(res, 0, 6));
		if (buffer) {
			pi_buffer_clear (buffer);
			pi_buffer_append (buffer, DLP_RESPONSE_DATA(res, 0, 10),
				(size_t)data_len);

			/* Some devices such as the Tungsten TX, Treo 650 and Treo 700p lock up if you try to read the entire record if the
			** record is almost at the maximum record size. The following mitigates this and allows the record
			** to be read in two chunks.
			*/
			if (data_len == maxBufferSize) {
				dlp_response_free(res);
				req = dlp_request_new_with_argid(dlpFuncReadResource, 0x21, 1, 12);
				if (req != NULL) {
					set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
					set_byte(DLP_REQUEST_DATA(req, 0, 1), 0);
					set_long(DLP_REQUEST_DATA(req, 0, 2), type);
					set_short(DLP_REQUEST_DATA(req, 0, 6), resID);
					set_short(DLP_REQUEST_DATA(req, 0, 8), maxBufferSize);	/* Offset in record */
					set_short(DLP_REQUEST_DATA(req, 0, 10), RECORD_READ_SAFEGUARD_SIZE);

					result = dlp_exec(sd, req, &res);
					
					dlp_request_free(req);
					
					if (result > 0) {
						data_len = res->argv[0]->len - 10;	/* number of bytes returned by the second read... */
						pi_buffer_append (buffer, DLP_RESPONSE_DATA(res, 0, 10), (size_t)data_len);
						data_len += maxBufferSize;		/* ...that add up to the bytes received in the first read */
					}
				}
			}
		}
		
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "DLP ReadResourceByType  Type: '%s', ID: %d, "
			 "Index: %d, and %d bytes:\n",
		    printlong(type), resID, 
		    get_short(DLP_RESPONSE_DATA(res, 0, 6)),(size_t)data_len));
		CHECK(PI_DBG_DLP, PI_DBG_LVL_DEBUG, 
		      pi_dumpdata(DLP_RESPONSE_DATA(res, 0, 10),(size_t)data_len));
	} else {
		data_len = result;
	}

	dlp_response_free(res);
	
	return data_len;
}

int
dlp_ReadResourceByIndex(int sd, int dbhandle, unsigned int resindex, pi_buffer_t *buffer,
			uint32_t *type, int *resID)
{
	int 	result,
		data_len,
		large = 0;
	struct dlpRequest *req;
	struct dlpResponse *res;
	int maxBufferSize = pi_maxrecsize(sd) - RECORD_READ_SAFEGUARD_SIZE;

	TraceX(dlp_ReadResourceByIndex,"resindex=%d",resindex);
	pi_reset_errors(sd);

	/* TapWave (DLP 1.4) implements a `large' version of dlpFuncReadResource,
	 * which can return resources >64k
	 */
	if (pi_version(sd) >= 0x0104) {
		req = dlp_request_new (dlpFuncReadResourceEx, 1, 12);
		if (req == NULL)
			return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

		set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
		set_byte(DLP_REQUEST_DATA(req, 0, 1), 0);
		set_short(DLP_REQUEST_DATA(req, 0, 2), resindex);
		set_long(DLP_REQUEST_DATA(req, 0, 4), 0);
		set_long(DLP_REQUEST_DATA(req, 0, 8), pi_maxrecsize(sd));
		large = 1;
	} else {
		req = dlp_request_new (dlpFuncReadResource, 1, 8);
		if (req == NULL)
			return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

		set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
		set_byte(DLP_REQUEST_DATA(req, 0, 1), 0);
		set_short(DLP_REQUEST_DATA(req, 0, 2), resindex);
		set_long(DLP_REQUEST_DATA(req, 0, 4), maxBufferSize);
	}

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	
	if (result > 0) {
		data_len = res->argv[0]->len - (large ? 12 : 10);
		if (type)
			*type = get_long(DLP_RESPONSE_DATA(res, 0, 0));
		if (resID)
			*resID = get_short(DLP_RESPONSE_DATA(res, 0, 4));
		if (buffer) {
			pi_buffer_clear (buffer);
			pi_buffer_append (buffer, DLP_RESPONSE_DATA(res, 0, large ? 12 : 10),
				(size_t)data_len);

			/* Some devices such as the Tungsten TX, Treo 650 and Treo 700p lock up if you try to read the entire record if the
			** record is almost at the maximum record size. The following mitigates this and allows the record
			** to be read in two chunks.
			*/
			if (data_len == maxBufferSize && !large) {
				dlp_response_free(res);
				req = dlp_request_new (dlpFuncReadResource, 1, 8);
				if (req != NULL) {
					set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
					set_byte(DLP_REQUEST_DATA(req, 0, 1), 0);
					set_short(DLP_REQUEST_DATA(req, 0, 2), resindex);
					set_short(DLP_REQUEST_DATA(req, 0, 4), maxBufferSize); /* Offset in record */
					set_short(DLP_REQUEST_DATA(req, 0, 6), RECORD_READ_SAFEGUARD_SIZE);
					
					result = dlp_exec(sd, req, &res);
					
					dlp_request_free(req);
					
					if (result > 0) {
						data_len = res->argv[0]->len - (large ? 12 : 10); /* number of bytes returned by the second read... */
						pi_buffer_append (buffer, DLP_RESPONSE_DATA(res, 0, large ? 12 : 10),
										  (size_t)data_len);
						data_len += maxBufferSize;			/* ...that add up to the bytes received in the first read */
					}
				}
			}			
		}

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "DLP ReadResourceByIndex Type: '%s', ID: %d, "
			"Index: %d, and %d bytes:\n",
		    printlong(get_long(DLP_RESPONSE_DATA(res, 0, 0))),
		    get_short(DLP_RESPONSE_DATA(res, 0, 4)),
		    resindex, data_len));
		CHECK(PI_DBG_DLP, PI_DBG_LVL_DEBUG, 
		      pi_dumpdata(DLP_RESPONSE_DATA(res, 0, (large ? 12 : 10)),
			 (size_t)data_len));
	} else {
		data_len = result;
	}

	dlp_response_free(res);
	
	return data_len;
}

int
dlp_WriteResource(int sd, int dbhandle, uint32_t type, int resID,
		  const void *data, size_t length)
{
	int 	result,
		large = 0;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_WriteResource,"'%4.4s' #%d",(const char *)&type,resID);
	pi_reset_errors(sd);

	/* TapWave (DLP 1.4) implements a `large' version of dlpFuncWriteResource,
	 * which can store records >64k
	 */
	if (pi_version(sd) >= 0x0104) {
		req = dlp_request_new_with_argid(dlpFuncWriteResourceEx,
			PI_DLP_ARG_FIRST_ID | PI_DLP_ARG_FLAG_LONG, 1, 12 + length);
		large = 1;
	} else {
		if (length > 0xffff)
			length = 0xffff;
		req = dlp_request_new(dlpFuncWriteResource, 1, 10 + length);
	}
	if (req == NULL) {
		LOG((PI_DBG_DLP, PI_DBG_LVL_ERR,
			    "DLP sd:%i large:%i dlp_request_new failed\n",
			    sd, large));
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);
	}

	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), 0);
	set_long(DLP_REQUEST_DATA(req, 0, 2), type);
	set_short(DLP_REQUEST_DATA(req, 0, 6), resID);
	if (large)
		set_long (DLP_REQUEST_DATA(req, 0, 8), 0);      /* device doesn't want length here (it computes it) */
	else
		set_short(DLP_REQUEST_DATA(req, 0, 8), length);

	memcpy(DLP_REQUEST_DATA(req, 0, large ? 12 : 10), data, length);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	dlp_response_free(res);

	return result;
}


int
dlp_DeleteResource(int sd, int dbhandle, int all, uint32_t restype,
		   int resID)
{
	int 	result,
		flags = all ? 0x80 : 0;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_DeleteResource,"restype='%4.4s' resID=%d all=%d",
	        (const char *)&restype,resID,all);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncDeleteResource, 1, 8);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), flags);
	set_long(DLP_REQUEST_DATA(req, 0, 2), restype);
	set_short(DLP_REQUEST_DATA(req, 0, 6), resID);

	result = dlp_exec(sd, req, &res);
	
	dlp_request_free(req);
	dlp_response_free(res);
	
	return result;
}

int
dlp_ReadAppBlock(int sd, int dbhandle, int offset, int reqbytes, pi_buffer_t *retbuf)
{
	int 	result,
		data_len;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_ReadAppBlock,"offset=%d reqbytes=%d",offset,reqbytes);
	pi_reset_errors(sd);

	if (retbuf)
		pi_buffer_clear(retbuf);

	req = dlp_request_new(dlpFuncReadAppBlock, 1, 6);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), 0);
	set_short(DLP_REQUEST_DATA(req, 0, 2), offset);
	set_short(DLP_REQUEST_DATA(req, 0, 4), reqbytes);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	
	if (result >= 0) {
		if (result < 2)
			data_len = PI_ERR_DLP_COMMAND;
		else {
			data_len = res->argv[0]->len - 2;
			if (retbuf && data_len)
				pi_buffer_append(retbuf, DLP_RESPONSE_DATA(res, 0, 2),
					(size_t)data_len);
			
			LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
				"DLP ReadAppBlock %d bytes\n", data_len));
			CHECK(PI_DBG_DLP, PI_DBG_LVL_DEBUG, 
				  pi_dumpdata(DLP_RESPONSE_DATA(res, 0, 2),
				(size_t)data_len));
		}
	} else {
		data_len = result;
	}

	dlp_response_free(res);
	
	return data_len;
}

int
dlp_WriteAppBlock(int sd, int dbhandle, const /* @unique@ */ void *data,
      size_t length)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_WriteAppBlock,"length=%d",length);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncWriteAppBlock, 1, 4 + length);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);
	
	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), 0);
	set_short(DLP_REQUEST_DATA(req, 0, 2), length);

	if (length + 10 > DLP_BUF_SIZE) {
		LOG((PI_DBG_DLP, PI_DBG_LVL_ERR,
		     "DLP WriteAppBlock: data too large (>64k)"));
		pi_set_error(sd, PI_ERR_DLP_DATASIZE);
		return -131;
	}
	if (length)
		memcpy(DLP_REQUEST_DATA(req, 0, 4), data, length);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	dlp_response_free(res);

	return result;
}

int
dlp_ReadSortBlock(int sd, int dbhandle, int offset, int reqbytes, pi_buffer_t *retbuf)
{
	int 	result,
		data_len;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_ReadSortBlock,"offset=%d reqbytes=%d",offset,reqbytes);
	pi_reset_errors(sd);

	if (retbuf)
		pi_buffer_clear(retbuf);

	req = dlp_request_new(dlpFuncReadSortBlock, 1, 6);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), 0);
	set_short(DLP_REQUEST_DATA(req, 0, 2), offset);
	set_short(DLP_REQUEST_DATA(req, 0, 4), reqbytes);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	
	if (result >= 0) {
		if (result < 2)
			data_len = PI_ERR_DLP_COMMAND;
		else {
			data_len = res->argv[0]->len - 2;
			if (retbuf)
				pi_buffer_append(retbuf, DLP_RESPONSE_DATA(res, 0, 2), 
					(size_t)data_len);
			
			LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
				"DLP ReadSortBlock %d bytes\n", data_len));
			CHECK(PI_DBG_DLP, PI_DBG_LVL_DEBUG, 
				  pi_dumpdata(DLP_RESPONSE_DATA(res, 0, 2),
				(size_t)data_len));
		}
	} else {
		data_len = result;
	}

	dlp_response_free(res);
	
	return data_len;
}

int
dlp_WriteSortBlock(int sd, int dbhandle, const /* @unique@ */ void *data,
		       size_t length)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_WriteSortBlock,"length=%d",length);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncWriteSortBlock, 1, 4 + length);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), 0);
	set_short(DLP_REQUEST_DATA(req, 0, 2), length);

	if (length + 10 > DLP_BUF_SIZE) {
		LOG((PI_DBG_DLP, PI_DBG_LVL_ERR,
		     "DLP WriteSortBlock: data too large (>64k)"));
		pi_set_error(sd, PI_ERR_DLP_DATASIZE);
		return -131;
	}
	memcpy(DLP_REQUEST_DATA(req, 0, 4), data, length);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	dlp_response_free(res);

	return result;
}

int
dlp_CleanUpDatabase(int sd, int dbhandle)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	Trace(dlp_CleanUpDatabase);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncCleanUpDatabase, 1, 1);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);

	result = dlp_exec(sd, req, &res);
	
	dlp_request_free(req);
	dlp_response_free(res);
	
	return result;
}

int
dlp_ResetSyncFlags(int sd, int dbhandle)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	Trace(dpl_ResetSyncFlags);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncResetSyncFlags, 1, 1);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);

	result = dlp_exec(sd, req, &res);
	
	dlp_request_free(req);
	dlp_response_free(res);
	
	return result;
}

int
dlp_ReadNextRecInCategory(int sd, int dbhandle, int category,
			  pi_buffer_t *buffer, recordid_t *recuid, int *recindex,
			  int *attr)
{
	int 	result,
		data_len,
		flags;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_ReadNextRecInCategory,"category=%d",category);
	pi_reset_errors(sd);

	if (pi_version(sd) < 0x0101) {
		/* Emulate for PalmOS 1.0 */
		int 	cat,
			rec;
		pi_socket_t *ps;

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "DLP ReadNextRecInCategory Emulating with: Handle: %d, "
			 "Category: %d\n",
		    dbhandle, category));

		if ((ps = find_pi_socket(sd)) == 0) {
			//errno = ESRCH;
			return -130;
		}

		for (;;) {
			/* Fetch next modified record (in any category) */
			rec = dlp_ReadRecordByIndex(sd, dbhandle,
						    ps->dlprecord, 0, 0,
						    0, &cat);

			if (rec < 0)
				break;

			if (cat != category) {
				ps->dlprecord++;
				continue;
			}

			rec = dlp_ReadRecordByIndex(sd, dbhandle,
						    ps->dlprecord, buffer,
						    recuid, attr, &cat);

			if (rec >= 0) {
				if (recindex)
					*recindex = ps->dlprecord;
				ps->dlprecord++;
			} else {
				/* If none found, reset modified pointer so
				   that another search on a different (or
				   the same!) category will work */

				/* Freeow! Do _not_ reset, as the Palm
				   itself does not!

				   ps->dlprecord = 0; */
			}

			break;
		}

		return rec;
	}
	
	req = dlp_request_new(dlpFuncReadNextRecInCategory, 1, 2);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), category);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);

	if (result > 0) {
		data_len = res->argv[0]->len - 10;
		if (recuid)
			*recuid = get_long(DLP_RESPONSE_DATA(res, 0, 0));
		if (recindex)
			*recindex = get_short(DLP_RESPONSE_DATA(res, 0, 4));
		if (attr)
			*attr = get_byte(DLP_RESPONSE_DATA(res, 0, 8));
		if (buffer) {
			pi_buffer_clear (buffer);
			pi_buffer_append (buffer, DLP_RESPONSE_DATA(res, 0, 10),
				(size_t)data_len);
		}

		flags = get_byte(DLP_RESPONSE_DATA(res, 0, 8));
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			"DLP ReadNextRecInCategory ID: 0x%8.8lX, "
			 "Index: %d, Category: %d\n"
			"  Flags: %s%s%s%s%s%s (0x%2.2X) and %d bytes:\n",
			(uint32_t) get_long(DLP_RESPONSE_DATA(res,
				0, 0)),
			get_short(DLP_RESPONSE_DATA(res, 0, 4)),
			(int) get_byte(DLP_RESPONSE_DATA(res, 0, 9)),
			(flags & dlpRecAttrDeleted) ? " Deleted" : "",
			(flags & dlpRecAttrDirty) ? " Dirty" : "",
			(flags & dlpRecAttrBusy) ? " Busy" : "",
			(flags & dlpRecAttrSecret) ? " Secret" : "",
			(flags & dlpRecAttrArchived) ? " Archive" : "",
			(!flags) ? " None" : "",
			flags, data_len));
		CHECK(PI_DBG_DLP, PI_DBG_LVL_DEBUG, 
			  pi_dumpdata(DLP_RESPONSE_DATA(res, 0, 10),
			(size_t)data_len));
	} else {
		data_len = result;
	}

	dlp_response_free(res);
	
	return data_len;
}

int
dlp_ReadAppPreference(int sd, uint32_t creator, int prefID, int backup,
		      int maxsize, void *buffer, size_t *size, int *version)
{
	int 	result,
		data_len;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_ReadAppPreference,"creator=0x%08X prefID=%d backup=%d maxsize=%d",
	    creator,prefID,backup,maxsize);
	pi_reset_errors(sd);

	if (pi_version(sd) < 0x0101) {
		/* Emulate on PalmOS 1.0 */
		int 	db;
		pi_buffer_t *buf;

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "DLP ReadAppPreference Emulating with: Creator: '%s', "
			"Id: %d, Size: %d, Backup: %d\n",
		    printlong(creator), prefID,
		    buffer ? maxsize : 0, backup ? 0x80 : 0));

		result = dlp_OpenDB(sd, 0, dlpOpenRead, "System Preferences", &db);
		if (result < 0)
			return result;

		buf = pi_buffer_new (1024);
		
		result = dlp_ReadResourceByType(sd, db, creator, prefID, buf,NULL);

		if (result < 0) {
			/* have to keep the previous error codes to properly return it */
			int err1 = pi_error(sd);
			int err2 = pi_palmos_error(sd);

			pi_buffer_free (buf);
			if (err1 != PI_ERR_SOCK_DISCONNECTED)
				dlp_CloseDB(sd, db);

			pi_set_error(sd, err1);
			pi_set_palmos_error(sd, err2);
			return result;
		}

		if (size)
			*size = buf->used - 2;

		if (version)
			*version = get_short(buf->data);

		if (result > 2) {
			result -= 2;
			memcpy(buffer, buf->data + 2, (size_t)result);
		} else {
			result = 0;
		}

		pi_buffer_free (buf);
		dlp_CloseDB(sd, db);
		return result;
	}
	
	req = dlp_request_new(dlpFuncReadAppPreference, 1, 10);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_long(DLP_REQUEST_DATA(req, 0, 0), creator);
	set_short(DLP_REQUEST_DATA(req, 0, 4), prefID);
	set_short(DLP_REQUEST_DATA(req, 0, 6), buffer ? maxsize : 0);
	set_byte(DLP_REQUEST_DATA(req, 0, 8), backup ? 0x80 : 0);
	set_byte(DLP_REQUEST_DATA(req, 0, 9), 0); /* Reserved */

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	
	if (result > 0) {
		data_len = get_short(DLP_RESPONSE_DATA(res, 0, 4));
		if (version)
			*version = get_short(DLP_RESPONSE_DATA(res, 0, 0));
		if (size && !buffer) *size =
			get_short(DLP_RESPONSE_DATA(res, 0, 2)); /* Total sz */
		if (size && buffer)
			*size = data_len;	/* Size returned */
		if (buffer)
			memcpy(buffer, DLP_RESPONSE_DATA(res, 0, 6),
				(size_t)data_len);

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			"DLP ReadAppPref Version: %d, "
			"Total size: %d, Read %d bytes:\n",
			get_short(DLP_RESPONSE_DATA(res, 0, 0)), 
			get_short(DLP_RESPONSE_DATA(res, 0, 2)),
			get_short(DLP_RESPONSE_DATA(res, 0, 4))));
		CHECK(PI_DBG_DLP, PI_DBG_LVL_DEBUG, 
			  pi_dumpdata(DLP_RESPONSE_DATA(res, 0, 6),
			(size_t)data_len));
	} else {
		data_len = result;
	}

	dlp_response_free(res);

	return data_len;
}

int
dlp_WriteAppPreference(int sd, uint32_t creator, int prefID, int backup,
		       int version, const void *buffer, size_t size)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_WriteAppPreference,"creator='%4.4s' prefID=%d backup=%d version=%d size=%d",
	    (const char *)&creator,prefID,backup,version,size);
	pi_reset_errors(sd);

	if (pi_version(sd) < 0x0101) {
		/* Emulate on PalmOS 1.0 */
		int 	db,
			err1,
			err2;

		if ((result = dlp_OpenDB(sd, 0, dlpOpenWrite, "System Preferences",
			       &db)) < 0)
			return result;

		if (buffer && size) {
			unsigned char dlp_buf[DLP_BUF_SIZE];
			memcpy(dlp_buf + 2, buffer, size);
			set_short(dlp_buf, version);
			result = dlp_WriteResource(sd, db, creator, prefID, dlp_buf,
						size);
		} else {
			result = dlp_WriteResource(sd, db, creator, prefID, NULL,
						0);
		}
		err1 = pi_error(sd);
		err2 = pi_palmos_error(sd);

		if (err1 != PI_ERR_SOCK_DISCONNECTED)
			dlp_CloseDB(sd, db);

		if (result < 0) {
			/* restore previous error after DB close */
			pi_set_error(sd, err1);
			pi_set_palmos_error(sd, err2);
		}
		return result;
	}

	req = dlp_request_new(dlpFuncWriteAppPreference, 1, 12 + size);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_long(DLP_REQUEST_DATA(req, 0, 0), creator);
	set_short(DLP_REQUEST_DATA(req, 0, 4), prefID);
	set_short(DLP_REQUEST_DATA(req, 0, 6), version);
	set_short(DLP_REQUEST_DATA(req, 0, 8), size);
	set_byte(DLP_REQUEST_DATA(req, 0, 10), backup ? 0x80 : 0);
	set_byte(DLP_REQUEST_DATA(req, 0, 11), 0); 	/* Reserved */

	if ((size + 12) > DLP_BUF_SIZE) {
		LOG((PI_DBG_DLP, PI_DBG_LVL_ERR,
			 "DLP WriteAppPreferenceV2: data too large (>64k)"));
		return PI_ERR_DLP_DATASIZE;
	}
	memcpy(DLP_REQUEST_DATA(req, 0, 12), buffer, size);

	result = dlp_exec (sd, req, &res);

	dlp_request_free(req);
	dlp_response_free(res);

	return result;
}

int
dlp_ReadNextModifiedRecInCategory(int sd, int dbhandle, int category,
				  pi_buffer_t *buffer, recordid_t *recID,
				  int *recindex, int *attr)
{
	int 	result,
		data_len;
	struct dlpRequest *req;
	struct dlpResponse *res;

	TraceX(dlp_ReadNextModifiedRecInCategory,"category=%d",category);
	pi_reset_errors(sd);

	if (pi_version(sd) < 0x0101) {
		/* Emulate for PalmOS 1.0 */
		int 	cat;

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		    "DLP ReadNextModifiedRecInCategory"
			" Emulating with: Handle: %d, Category: %d\n",
		    dbhandle, category));

		do {
			/* Fetch next modified record (in any category) */
			result = dlp_ReadNextModifiedRec(sd, dbhandle, buffer,
						recID, recindex, attr, &cat);

			/* If none found, reset modified pointer so that
			 another search on a different (or the same!) category
			 will start from the beginning */

			/* Working on same assumption as ReadNextRecInCat,
				elide this:
			   if (r < 0)
			   dlp_ResetDBIndex(sd, fHandle);
			 */

			/* Loop until we fail to get a record or a record
			 is found in the proper category */
		}
		while (result >= 0 && cat != category);
		
		return result;
	}

	req = dlp_request_new(dlpFuncReadNextModifiedRecInCategory, 1, 2);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), category);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);

	if (result > 0) {
		data_len = res->argv[0]->len - 10;

		if (recID)
			*recID = get_long(DLP_RESPONSE_DATA(res, 0, 0));
		if (recindex)
			*recindex = get_short(DLP_RESPONSE_DATA(res, 0, 4));
		if (attr)
			*attr = get_byte(DLP_RESPONSE_DATA(res, 0, 8));

		if (buffer) {
			pi_buffer_clear (buffer);
			pi_buffer_append (buffer, DLP_RESPONSE_DATA(res, 0, 10),
				(size_t)data_len);
		}

		CHECK(PI_DBG_DLP, PI_DBG_LVL_DEBUG,
			 record_dump(
				get_long(DLP_RESPONSE_DATA(res, 0, 0)),			/* recID */
				get_short(DLP_RESPONSE_DATA(res, 0, 4)),		/* index */
				get_byte(DLP_RESPONSE_DATA(res, 0, 8)),			/* flags */
				get_byte(DLP_RESPONSE_DATA(res, 0, 9)),			/* catID */
				DLP_RESPONSE_DATA(res, 0, 10), data_len));
	} else {
		data_len = result;
	}

	dlp_response_free(res);

	return data_len;
}

int
dlp_ReadNextModifiedRec(int sd, int dbhandle, pi_buffer_t *buffer, recordid_t * recID,
			int *recindex, int *attr, int *category)
{
	int 	result,
		data_len;
	struct dlpRequest *req;
	struct dlpResponse *res;

	Trace(dlp_ReadNextModifiedRec);
	pi_reset_errors(sd);
	
	req = dlp_request_new (dlpFuncReadNextModifiedRec, 1, 1);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
	
	result = dlp_exec (sd, req, &res);

	dlp_request_free(req);
	
	if (result >= 0) {
		data_len = res->argv[0]->len -10;
		if (recID)
			*recID = get_long(DLP_RESPONSE_DATA(res, 0, 0));
		if (recindex)
			*recindex = get_short(DLP_RESPONSE_DATA(res, 0, 4));
		if (attr)
			*attr = get_byte(DLP_RESPONSE_DATA(res, 0, 8));
		if (category)
			*category = get_byte(DLP_RESPONSE_DATA(res, 0, 9));

		if (buffer) {
			pi_buffer_clear (buffer);
			pi_buffer_append (buffer, DLP_RESPONSE_DATA(res, 0, 10),
				(size_t)data_len);
		}

		CHECK(PI_DBG_DLP, PI_DBG_LVL_DEBUG,
			 record_dump(
				get_long(DLP_RESPONSE_DATA(res, 0, 0)),			/* recID */
				get_short(DLP_RESPONSE_DATA(res, 0, 4)),		/* index */
				get_byte(DLP_RESPONSE_DATA(res, 0, 8)),			/* flags */
				get_byte(DLP_RESPONSE_DATA(res, 0, 9)),			/* catID */
				DLP_RESPONSE_DATA(res, 0, 10), data_len));
	} else {
		data_len = result;
	}

	dlp_response_free(res);
	
	return data_len;
}

int
dlp_ReadRecordById(int sd, int dbhandle, recordid_t recuid, pi_buffer_t *buffer,
		   int *recindex, int *attr, int *category)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	int maxBufferSize = pi_maxrecsize(sd) - RECORD_READ_SAFEGUARD_SIZE;

	TraceX(dlp_ReadRecordById,"recuid=0x%08x",recuid);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncReadRecord, 1, 10);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
	set_byte(DLP_REQUEST_DATA(req, 0, 1), 0);
	set_long(DLP_REQUEST_DATA(req, 0, 2), recuid); 
	set_short(DLP_REQUEST_DATA(req, 0, 6), 0); /* Offset into record */
	set_short(DLP_REQUEST_DATA(req, 0, 8), buffer ? maxBufferSize : 0);	/* length to return */

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	
	if (result > 0) {
		result = res->argv[0]->len - 10;
		if (recindex)
			*recindex = get_short(DLP_RESPONSE_DATA(res, 0, 4));
		if (attr)
			*attr = get_byte(DLP_RESPONSE_DATA(res, 0, 8));
		if (category)
			*category = get_byte(DLP_RESPONSE_DATA(res, 0, 9));
		if (buffer) {
			pi_buffer_clear (buffer);
			pi_buffer_append (buffer, DLP_RESPONSE_DATA(res, 0, 10),
				(size_t)result);

			/* Some devices such as the Tungsten TX, Treo 650 and Treo 700p lock up if you try to read the entire record if the
			** record is almost at the maximum record size. The following mitigates this and allows the record
			** to be read in two chunks.
			*/
			if (result == maxBufferSize) {
				dlp_response_free(res);
				req = dlp_request_new(dlpFuncReadRecord, 1, 10);
				if (req != NULL) {
					set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
					set_byte(DLP_REQUEST_DATA(req, 0, 1), 0);
					set_long(DLP_REQUEST_DATA(req, 0, 2), recuid); 
					set_short(DLP_REQUEST_DATA(req, 0, 6), maxBufferSize); /* Offset into record */
					set_short(DLP_REQUEST_DATA(req, 0, 8), buffer ? RECORD_READ_SAFEGUARD_SIZE : 0);	/* length to return */

					result = dlp_exec(sd, req, &res);
					dlp_request_free(req);

					if (result > 0) {
						result = res->argv[0]->len - 10;
						pi_buffer_append (buffer, DLP_RESPONSE_DATA(res, 0, 10),
										  (size_t)result);
						result += maxBufferSize;
					}
				}
			}
		}

		CHECK(PI_DBG_DLP, PI_DBG_LVL_DEBUG,
			 record_dump(
				get_long(DLP_RESPONSE_DATA(res, 0, 0)),			/* recID */
				get_short(DLP_RESPONSE_DATA(res, 0, 4)),		/* index */
				get_byte(DLP_RESPONSE_DATA(res, 0, 8)),			/* flags */
				get_byte(DLP_RESPONSE_DATA(res, 0, 9)),			/* catID */
				DLP_RESPONSE_DATA(res, 0, 10), result));
	}
	
	dlp_response_free(res);
	
	return result;
}

int
dlp_ReadRecordByIndex(int sd, int dbhandle, int recindex, pi_buffer_t *buffer,
	recordid_t * recuid, int *attr, int *category)
{
	int 	result,
		large = 0;
	struct dlpRequest *req;
	struct dlpResponse *res;
	int maxBufferSize = pi_maxrecsize(sd) - RECORD_READ_SAFEGUARD_SIZE;

	TraceX(dlp_ReadRecordByIndex,"recindex=%d",recindex);
	pi_reset_errors(sd);

	/* TapWave (DLP 1.4) implements a `large' version of dlpFuncReadRecord,
	 * which can return records >64k
	 */
	if (pi_version(sd) >= 0x0104) {
		req = dlp_request_new_with_argid(dlpFuncReadRecordEx, 0x21, 1, 12);
		if (req == NULL)
			return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

		set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
		set_byte(DLP_REQUEST_DATA(req, 0, 1), 0x00);
		set_short(DLP_REQUEST_DATA(req, 0, 2), recindex);
		set_long(DLP_REQUEST_DATA(req, 0, 4), 0); /* Offset into record */
		set_long(DLP_REQUEST_DATA(req, 0, 8), pi_maxrecsize(sd));	/* length to return */
		large = 1;
	} else {
		req = dlp_request_new_with_argid(dlpFuncReadRecord, 0x21, 1, 8);
		if (req == NULL)
			return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

		set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
		set_byte(DLP_REQUEST_DATA(req, 0, 1), 0x00);
		set_short(DLP_REQUEST_DATA(req, 0, 2), recindex);
		set_short(DLP_REQUEST_DATA(req, 0, 4), 0); /* Offset into record */
		set_short(DLP_REQUEST_DATA(req, 0, 6), buffer ? maxBufferSize : 0);	/* length to return */
	}
	result = dlp_exec(sd, req, &res);
	dlp_request_free(req);
	
	if (result > 0) {
		result = res->argv[0]->len - (large ? 14 : 10);
		if (recuid)
			*recuid = get_long(DLP_RESPONSE_DATA(res, 0, 0));
		if (attr)
			*attr = get_byte(DLP_RESPONSE_DATA(res, 0, large ? 12 : 8));
		if (category)
			*category = get_byte(DLP_RESPONSE_DATA(res, 0, large ? 13 : 9));
		if (buffer) {
			pi_buffer_clear (buffer);
			pi_buffer_append (buffer, DLP_RESPONSE_DATA(res, 0, large ? 14 : 10),
				(size_t)result);

			/* Some devices such as the Tungsten TX, Treo 650 and Treo 700p lock up if you try to read the entire record if the
			** record is almost at the maximum record size. The following mitigates this and allows the record
			** to be read in two chunks.
			*/
			if (result == maxBufferSize && !large) {
				dlp_response_free(res);
				req = dlp_request_new_with_argid(dlpFuncReadRecord, 0x21, 1, 8);
				if (req != NULL) {
					set_byte(DLP_REQUEST_DATA(req, 0, 0), dbhandle);
					set_byte(DLP_REQUEST_DATA(req, 0, 1), 0x00);
					set_short(DLP_REQUEST_DATA(req, 0, 2), recindex);
					set_short(DLP_REQUEST_DATA(req, 0, 4), maxBufferSize); /* Offset into record */
					set_short(DLP_REQUEST_DATA(req, 0, 6), buffer ? RECORD_READ_SAFEGUARD_SIZE : 0);	/* length to return */

					result = dlp_exec(sd, req, &res);
					dlp_request_free(req);

					if (result > 0) {
						result = res->argv[0]->len - (large ? 14 : 10);
						pi_buffer_append (buffer, DLP_RESPONSE_DATA(res, 0, large ? 14 : 10),
										  (size_t)result);
						
						result += maxBufferSize;
					}
				}
			}
		}

		CHECK(PI_DBG_DLP, PI_DBG_LVL_DEBUG,
			 record_dump(
				get_long(DLP_RESPONSE_DATA(res, 0, 0)),			/* recUID */
				get_short(DLP_RESPONSE_DATA(res, 0, 4)),		/* index */
				get_byte(DLP_RESPONSE_DATA(res, 0, large ? 12 : 8)),	/* flags */
				get_byte(DLP_RESPONSE_DATA(res, 0, large ? 13 : 9)),	/* catID */
				DLP_RESPONSE_DATA(res, 0, large ? 14 : 10), result));
	}

	dlp_response_free(res);
	
	return result;
}

int
dlp_ExpSlotEnumerate(int sd, int *numSlots, int *slotRefs)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	Trace(dlp_ExpSlotEnumerate);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncExpSlotEnumerate, 0);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	
	if (result > 0) {
		int slots, i;

		slots = get_short(DLP_RESPONSE_DATA (res, 0, 0));

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			 "DLP ExpSlotEnumerate %d\n", slots));

		if (slots) {
			for (i = 0; i < slots && i < *numSlots; i++) {
				slotRefs[i] =
			  	get_short(DLP_RESPONSE_DATA (res, 0,
					 2 + (2 * i)));

				LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
				 "  %d Slot-Refnum %d\n", i, slotRefs[i]));
			}
		}

		*numSlots = slots;
	}
	 
	dlp_response_free(res);

	return result;
}

int
dlp_ExpCardPresent(int sd, int slotRef)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	RequireDLPVersion(sd,1,2);
	TraceX(dlp_ExpCardPresent,"slotRef=%d",slotRef);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncExpCardPresent, 1, 2);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_short(DLP_REQUEST_DATA(req, 0, 0), slotRef);

	result = dlp_exec (sd, req, &res);

	dlp_request_free(req);
	dlp_response_free(res);

	return result;
}

int
dlp_ExpCardInfo(int sd, int slotRef, uint32_t *flags, int *numStrings,
				char **strings)
{
	int result;
	struct dlpRequest* req;
	struct dlpResponse* res;

	RequireDLPVersion(sd,1,2);
	TraceX(dlp_ExpCardInfo,"slotRef=%d",slotRef);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncExpCardInfo, 1, 2);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_short(DLP_REQUEST_DATA(req, 0, 0), slotRef);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	
	if (result > 0) {
		*flags = get_long(DLP_RESPONSE_DATA (res, 0, 0));
		*numStrings = get_byte(DLP_RESPONSE_DATA (res, 0, 4));

		if (strings && *numStrings) {
			int i, len, sz = 0;
			char *p = DLP_RESPONSE_DATA (res, 0, 8);

			for (i=0; i < *numStrings; i++, sz+=len, p+=len)
				len = strlen (p) + 1;

			*strings = (char *) malloc ((size_t)sz);
			if (*strings)
				memcpy (*strings, DLP_RESPONSE_DATA (res, 0, 8), (size_t)sz);
			else
				result = pi_set_error(sd, PI_ERR_GENERIC_MEMORY);
		}

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			 "DLP ExpCardInfo flags: 0x%08x numStrings: %d\n",
			 *flags, *numStrings));
	}

	dlp_response_free(res);

	return result;
}

int
dlp_VFSGetDefaultDir(int sd, int volRefNum, const char *type, char *dir,
	int *len)
{
	int result, buflen;
	struct dlpRequest *req;
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSGetDefaultDir,"volRefNum=%d",volRefNum);
	pi_reset_errors(sd);
	
	req = dlp_request_new(dlpFuncVFSGetDefaultDir,
		1, 2 + (strlen(type) + 1));
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_short(DLP_REQUEST_DATA(req, 0, 0), volRefNum);
	strcpy(DLP_REQUEST_DATA(req, 0, 2), type);
	
	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	
	if (result > 0) {
		buflen = get_short(DLP_RESPONSE_DATA (res, 0, 0));
		
		if (*len < buflen + 1)
			result = pi_set_error(sd, PI_ERR_DLP_BUFSIZE);
		else {
			if (buflen)
				strncpy(dir, DLP_RESPONSE_DATA (res, 0, 2), 
					(size_t)buflen);
			else
				dir[0] = '\0';
			
			*len = buflen;

			LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
				 "Default dir is %s\n", dir));
		}
	}
	
	dlp_response_free(res);
	
	return result;
}

int
dlp_VFSImportDatabaseFromFile(int sd, int volRefNum, const char *path,
	int *cardno, uint32_t *localid)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSImportDatabaseFromFile,"volRefNum=%d path='%s'",volRefNum,path);
	pi_reset_errors(sd);

	LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		"Import file <%s>%d\n", path));

	req = dlp_request_new(dlpFuncVFSImportDatabaseFromFile,
		1, 2 + (strlen(path) + 1));
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_short(DLP_REQUEST_DATA(req, 0, 0), volRefNum);
	strcpy(DLP_REQUEST_DATA(req, 0, 2), path);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);

	if (result > 0) {
		if (cardno)
			*cardno = get_short(DLP_RESPONSE_DATA (res, 0, 0));
		if (localid)
			*localid = get_short(DLP_RESPONSE_DATA (res, 0, 2)); 

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		     "Database imported as: cardNo:%d dbID:%d\n", 
		     get_short(DLP_RESPONSE_DATA (res, 0, 0)), 
		     get_short(DLP_RESPONSE_DATA (res, 0, 2))));
	}
	
	dlp_response_free(res);

	return result;
}

int
dlp_VFSExportDatabaseToFile(int sd, int volRefNum, const char *path, 
	int cardno, unsigned int localid)  
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSExportDatabaseToFile,"cardno=%d localid=0x%08x volRefNum=%d path='%s'",
	    cardno,(int32_t)localid,volRefNum,path);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncVFSExportDatabaseToFile,
		1, 8 + (strlen(path) + 1));
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_short(DLP_REQUEST_DATA(req, 0, 0), volRefNum);
	set_short(DLP_REQUEST_DATA(req, 0, 2), cardno);
	set_long(DLP_REQUEST_DATA(req, 0, 4), localid);
	strcpy(DLP_REQUEST_DATA(req, 0, 8), path);

	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	dlp_response_free(res);

	return result;
}

int
dlp_VFSFileCreate(int sd, int volRefNum, const char *name)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSFileCreate,"volRefNum=%d name='%s'",volRefNum,name);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSFileCreate, 1, 2 + (strlen(name) + 1));
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_short (DLP_REQUEST_DATA (req, 0, 0), volRefNum);
	strcpy (DLP_REQUEST_DATA (req, 0, 2), name);

	result = dlp_exec (sd, req, &res);

	dlp_request_free (req);
	dlp_response_free (res);

	return result;
}

int
dlp_VFSFileOpen(int sd, int volRefNum, const char *path, int openMode, 
	    FileRef *fileRef)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSFileOpen,"volRefNum=%d mode=0x%04x path='%s'",
	    volRefNum,openMode,path);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSFileOpen, 1, 4 + (strlen (path) + 1));
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_short (DLP_REQUEST_DATA (req, 0, 0), volRefNum);
	set_short (DLP_REQUEST_DATA (req, 0, 2), openMode);
	strcpy (DLP_REQUEST_DATA (req, 0, 4), path);

	result = dlp_exec (sd, req, &res);

	dlp_request_free (req);
	
	if (result > 0) {
		*fileRef = get_long(DLP_RESPONSE_DATA (res, 0, 0));

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		     "OpenFileRef: 0x%x\n", *fileRef));
	}
	
	dlp_response_free(res);

	return result;
}

int
dlp_VFSFileClose(int sd, FileRef fileRef)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSFileClose,"fileRef=%d",fileRef);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSFileClose, 1, 4);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_long (DLP_REQUEST_DATA (req, 0, 0), fileRef);

	result = dlp_exec (sd, req, &res);

	dlp_request_free (req);
	dlp_response_free (res);
	
	LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		"Closed FileRef: %x\n", fileRef));

	return result;
}

int
dlp_VFSFileWrite(int sd, FileRef fileRef, const void *data, size_t len)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res = NULL;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSFileWrite,"fileRef=%d len=%d",(int32_t)fileRef,(int32_t)len);
	pi_reset_errors(sd);

	LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		"Write to FileRef: %x bytes %d\n", fileRef, len));
	
	req = dlp_request_new (dlpFuncVFSFileWrite, 1, 8);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_long (DLP_REQUEST_DATA (req, 0, 0), fileRef);
	set_long (DLP_REQUEST_DATA (req, 0, 4), len);

	result = dlp_exec (sd, req, &res);

	dlp_request_free (req);

	if (result >= 0) {
		int bytes = pi_write (sd, data, len);
		result = bytes;
		if (result < (int)len) {
			LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			     "send failed %d\n", result));
		} else {
			dlp_response_free (res);
			res = NULL;

			result = dlp_response_read (&res, sd);

			if (result > 0) {
				pi_set_palmos_error(sd, get_short(DLP_RESPONSE_DATA (res, 0, 2)));
				LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
					"send success (%d) res 0x%04x!\n", len, pi_palmos_error(sd)));
				result = bytes;
			}
		} 
	}

	dlp_response_free (res);

	return result;
}

int
dlp_VFSFileRead(int sd, FileRef fileRef, pi_buffer_t *data, size_t len)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	size_t bytes = 0;
	int freeze_txid = 1;
	size_t opt_size = sizeof(int);

	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSFileRead,"fileRef=%d len=%d",(int32_t)fileRef,(int32_t)len);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSFileRead, 1, 8);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_long (DLP_REQUEST_DATA (req, 0, 0), fileRef);
	set_long (DLP_REQUEST_DATA (req, 0, 4), len);

	/* if we're using PADP, this socket option is required so that
	 * the transfer can complete without error
	 */
	pi_setsockopt(sd, PI_LEVEL_PADP, PI_PADP_FREEZE_TXID, &freeze_txid, &opt_size);

	result = dlp_exec (sd, req, &res);	

	dlp_request_free (req);

	pi_buffer_clear (data);

	if (result >= 0) {

		do {
			result = pi_read(sd, data, len);
			if (result > 0) {
				len -= result;
				bytes += result;
			}
		} while (result > 0 && len > 0);

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		     "dlp_VFSFileRead: read %u bytes (last pi_read was %d)\n",
			(unsigned)bytes, result));

		if (result >= 0)
			result = bytes;
	}

	dlp_response_free(res);

	freeze_txid = 0;
	pi_setsockopt(sd, PI_LEVEL_PADP, PI_PADP_FREEZE_TXID, &freeze_txid, &opt_size);

	return result;
}

int
dlp_VFSFileDelete(int sd, int volRefNum, const char *path)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSFileDelete,"volRefNum=%d path='%s'",volRefNum,path);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSFileDelete, 1, 2 + (strlen (path) + 1));
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);
	
	set_short (DLP_REQUEST_DATA (req, 0, 0), volRefNum);
	strcpy (DLP_REQUEST_DATA (req, 0, 2), path);
	
	result = dlp_exec (sd, req, &res);

	dlp_request_free (req);
	dlp_response_free (res);

	return result;
}

int
dlp_VFSFileRename(int sd, int volRefNum, const char *path, 
		      const char *newname)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSFileRename,"volRefNum=%d file '%s' renamed '%s'",
	    volRefNum,path,rename);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSFileRename,
		1, 4 + (strlen (path) + 1) + (strlen (newname) + 1));
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_short (DLP_REQUEST_DATA (req, 0, 0), volRefNum);
	set_short (DLP_REQUEST_DATA (req, 0, 2), 2);
	strcpy (DLP_REQUEST_DATA (req, 0, 4), path);
	strcpy (DLP_REQUEST_DATA (req, 0, 4 + (strlen(path) + 1)), newname);
	
	result = dlp_exec (sd, req, &res);

	dlp_request_free (req);
	dlp_response_free (res);

	return result;
}

int
dlp_VFSFileEOF(int sd, FileRef fileRef)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSFileEOF,"fileRef=%d",fileRef);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSFileEOF, 1, 4);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);
	
	set_long (DLP_REQUEST_DATA (req, 0, 0), fileRef);
	
	result = dlp_exec (sd, req, &res);
	
	dlp_request_free (req);
	dlp_response_free (res);

	return result;
}

int
dlp_VFSFileTell(int sd, FileRef fileRef,int *position)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSFileTell,"fileRef=%d",fileRef);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncVFSFileTell, 1, 4);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_long (DLP_REQUEST_DATA (req, 0, 0), fileRef);

	result = dlp_exec (sd, req, &res);
	
	dlp_request_free (req);

	if (result > 0) {
		*position = get_long (DLP_RESPONSE_DATA (res, 0, 0));
	}
	
	dlp_response_free (res);

	return result;
}

int
dlp_VFSFileGetAttributes (int sd, FileRef fileRef, uint32_t *attributes)
{
	int	result;
	struct dlpRequest *req; 
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSFileGetAttributes,"fileRef=%d",fileRef);
	pi_reset_errors(sd);
	
	req = dlp_request_new (dlpFuncVFSFileGetAttributes, 1, 4);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_long (DLP_REQUEST_DATA (req, 0, 0), fileRef);
	
	result = dlp_exec (sd, req, &res);
	
	dlp_request_free (req);

	if (result > 0) {
		*attributes = get_long (DLP_RESPONSE_DATA (res, 0, 0));
	}

	dlp_response_free(res);	

	return result;
}

int
dlp_VFSFileSetAttributes(int sd, FileRef fileRef, uint32_t attributes)
{
	int	result;
	struct dlpRequest *req; 
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSFileSetAttributes,"fileRef=%d attributes=0x%08x",
	    fileRef,attributes);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSFileSetAttributes, 1, 8);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_long (DLP_REQUEST_DATA (req, 0, 0), fileRef);
	set_long (DLP_REQUEST_DATA (req, 0, 4), attributes);
	
	result = dlp_exec (sd, req, &res);

	dlp_request_free (req);
	dlp_response_free (res);

	return result;
}

int
dlp_VFSFileGetDate(int sd, FileRef fileRef, int which, time_t *date)
{
	int	result;
	struct dlpRequest *req; 
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSFileGetDate,"fileRef=%d which=%d",fileRef,which);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSFileGetDate, 1, 6);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_long (DLP_REQUEST_DATA (req, 0, 0), fileRef);
	set_short (DLP_REQUEST_DATA (req, 0, 4), which);

	result = dlp_exec (sd, req, &res);

	dlp_request_free (req);
	
	if (result > 0) {
		*date = get_long (DLP_RESPONSE_DATA (res, 0, 0)) - 2082852000;
	
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		     "Requested date(%d): %d / %x calc %d / %x\n",which,
		     get_long(DLP_RESPONSE_DATA (res, 0, 0)),
		     get_long(DLP_RESPONSE_DATA (res, 0, 0)),
		     *date,*date));
	}
	
	dlp_response_free (res);

	return result;
}

int
dlp_VFSFileSetDate(int sd, FileRef fileRef, int which, time_t date)
{
	int	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSFileSetDate,"fileRef=%d which=%d date=0x%08x",
	    (int32_t)fileRef,which,(int32_t)date);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncVFSFileSetDate, 1, 10);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_long (DLP_REQUEST_DATA(req, 0, 0), fileRef);
	set_short (DLP_REQUEST_DATA(req, 0, 4), which);
	set_long (DLP_REQUEST_DATA(req, 0, 6), date + 2082852000);

	result = dlp_exec (sd, req, &res);

	dlp_request_free (req);
	dlp_response_free (res);	

	return result;
}

int
dlp_VFSDirCreate(int sd, int volRefNum, const char *path)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSDirCreate,"volRefNum=%d path='%s'",volRefNum,path);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSDirCreate, 1, 2 + (strlen(path) + 1));
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_short (DLP_REQUEST_DATA (req, 0, 0), volRefNum);
	strcpy (DLP_REQUEST_DATA (req, 0, 2), path);
	
	result = dlp_exec (sd, req, &res);

	dlp_request_free (req);
	dlp_response_free (res);

	return result;
}

int
dlp_VFSDirEntryEnumerate(int sd, FileRef dirRefNum, 
	uint32_t *dirIterator, int *maxDirItems, struct VFSDirInfo *data)
{
	unsigned int result,
		entries,
		from,
		at,
		slen,
		count;
	struct dlpRequest *req;
	struct dlpResponse *res;

	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSDirEntryEnumerate,"dirRef=%d",dirRefNum);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSDirEntryEnumerate, 1, 12);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_long (DLP_REQUEST_DATA (req, 0, 0), dirRefNum);
	set_long (DLP_REQUEST_DATA (req, 0, 4), *dirIterator);
	set_long (DLP_REQUEST_DATA (req, 0, 8), 8 + *maxDirItems * (4 + vfsMAXFILENAME));

	result = dlp_exec (sd, req, &res);
	
	dlp_request_free (req);
	
	if (result > 0) {
		if (result) {
			*dirIterator = get_long (DLP_RESPONSE_DATA (res, 0, 0));
			entries = get_long (DLP_RESPONSE_DATA (res, 0, 4));
		} else {
			*dirIterator = vfsIteratorStop;
			entries = 0;
		}

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		     "%d results returnd (ilterator: %d)\n", entries,
		     *dirIterator));
	
		from  = 8;
		count = 0;
	
		for (at = 0; at < entries; at++) {
			if (*maxDirItems > at) {
				data[at].attr = 
					get_long(DLP_RESPONSE_DATA (res, 0, from));

				/* fix for Sony sims (and probably devices too): they return
				   the attributes in the high word of attr instead of the low
				   word. We can safely shift it since the high 16 bits are not
				   used for VFS flags */
				if ((data[at].attr & 0x0000FFFF) == 0 &&
					(data[at].attr & 0xFFFF0000) != 0)
					data[at].attr >>= 16;

				strncpy (data[at].name,
					DLP_RESPONSE_DATA(res, 0, from + 4),
					vfsMAXFILENAME);
				data[at].name[vfsMAXFILENAME-1] = 0;
				count++;
			}
	
			/* Zero terminated string. Strings that have an
			 even length will be null terminated and have a
			 pad byte. */
			slen = strlen (DLP_RESPONSE_DATA(res, 0, from + 4)) + 1;
			if (slen & 1)
				slen++;	/* make even stringlen + NULL */
	
			/* 6 = 4 (attr) + 1 (NULL)  -+ 1 (PADDING) */
			from += slen + 4;
		}
		*maxDirItems = count;
	}
	
	dlp_response_free (res);

	return result;  
}

int
dlp_VFSVolumeFormat(int sd, unsigned char flags,
	int fsLibRef, struct VFSSlotMountParam *param)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	Trace(dlp_VFSVolumeFormat);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncVFSVolumeFormat, 1, 4);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

/* FIXME check sizes, list the mount params properly */
	set_short(DLP_REQUEST_DATA(req, 0, 0), fsLibRef);
	set_short(DLP_REQUEST_DATA(req, 0, 2),
		 sizeof(struct VFSSlotMountParam));
	set_byte(DLP_REQUEST_DATA(req, 0, 4), flags);
	set_byte(DLP_REQUEST_DATA(req, 0, 4), 0); /* unused */

	set_short(DLP_REQUEST_DATA(req, 0, 6), param->vfsMountParam.volRefNum);
	set_short(DLP_REQUEST_DATA(req, 0, 8), param->vfsMountParam.reserved); 
	set_long(DLP_REQUEST_DATA(req, 0, 10), param->vfsMountParam.mountClass);
	set_short(DLP_REQUEST_DATA(req, 0, 14), param->slotLibRefNum);
	set_short(DLP_REQUEST_DATA(req, 0, 16), param->slotRefNum);   
	
	result = dlp_exec(sd, req, &res);

	dlp_request_free(req);
	dlp_response_free(res);

	return result;
}

int
dlp_VFSVolumeEnumerate(int sd, int *numVols, int *volRefs)
{
	int	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	Trace(dlp_VFSVolumeEnumerate);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSVolumeEnumerate, 0);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	result = dlp_exec (sd, req, &res);
	
	dlp_request_free (req);

	if (result > 0) {
		int vols, i;

		vols = get_short (DLP_RESPONSE_DATA (res, 0, 0));

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			"DLP VFSVolumeEnumerate %d\n", vols));

		if (vols) {
			for (i = 0; i < vols && i < *numVols; i++) {
				volRefs[i] =
				  get_short (DLP_RESPONSE_DATA (res,
					 0, 2 + (2 * i)));

				LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
				 "  %d Volume-Refnum %d\n", i, volRefs[i]));
			}
		}
		*numVols = vols;
	}
	else
		*numVols = 0;

	dlp_response_free (res);

	return result;
}

int
dlp_VFSVolumeInfo(int sd, int volRefNum, struct VFSInfo *volInfo)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;

	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSVolumeInfo,"volRefNum=%d",volRefNum);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSVolumeInfo, 1, 2);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_short (DLP_REQUEST_DATA(req, 0, 0), volRefNum);

	result = dlp_exec (sd, req, &res);

	dlp_request_free (req);
	
	if (result > 0) {
		volInfo->attributes		= get_long (DLP_RESPONSE_DATA (res, 0, 0));
		volInfo->fsType			= get_long (DLP_RESPONSE_DATA (res, 0, 4));  
		volInfo->fsCreator		= get_long (DLP_RESPONSE_DATA (res, 0, 8));
		volInfo->mountClass		= get_long (DLP_RESPONSE_DATA (res, 0, 12));
		volInfo->slotLibRefNum  = get_short (DLP_RESPONSE_DATA (res, 0, 16));
		volInfo->slotRefNum		= get_short (DLP_RESPONSE_DATA (res, 0, 18));
		volInfo->mediaType		= get_long (DLP_RESPONSE_DATA (res, 0, 20));
		volInfo->reserved		= get_long (DLP_RESPONSE_DATA (res, 0, 24));      

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		     "VFSVolumeInfo: fstype '%s' ", printlong(volInfo->fsType)));
		
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		     "fscreator: '%s'\nSlotlibref %d Slotref %d\n", 
		     printlong(volInfo->fsCreator),
		     volInfo->slotLibRefNum,
		     volInfo->slotRefNum));
		
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		     "Media: '%s'\n", printlong(volInfo->mediaType)));
	}
	
	dlp_response_free(res);
	
	return result;
}

int
dlp_VFSVolumeGetLabel(int sd, int volRefNum, int *len, char *name)
{
	int 	result;
	struct dlpRequest *req; 
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSVolumeGetLabel,"volRefNum=%d",volRefNum);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSVolumeGetLabel, 1, 2);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_short (DLP_REQUEST_DATA (req, 0, 0), volRefNum);
	
	result = dlp_exec (sd, req, &res);
	
	dlp_request_free (req);

	if (result > 0) {
		strncpy(name, DLP_RESPONSE_DATA(res, 0, 0),
			 (size_t)(*len - 1));
		*len = strlen(name);

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			 "DLP VFSVolumeGetLabel %s\n", name));
	}
	
	dlp_response_free(res);

	return result;
}

int
dlp_VFSVolumeSetLabel(int sd, int volRefNum, const char *name)
{
	int 	result;
	struct dlpRequest *req; 
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSVolumeSetLabel,"volRefNum=%d name='%s'",volRefNum,name);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSVolumeSetLabel, 1,
			2 + (strlen(name) + 1));
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_short (DLP_REQUEST_DATA (req, 0, 0), volRefNum);
	strcpy (DLP_REQUEST_DATA (req, 0, 2), name);

	result = dlp_exec (sd, req, &res);

	dlp_response_free (res);
	dlp_request_free (req);

	return result;
}

int
dlp_VFSVolumeSize(int sd, int volRefNum, int32_t *volSizeUsed,
	int32_t *volSizeTotal)
{
	int 	result;
	struct dlpRequest *req; 
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSVolumeSize,"volRefNum=%d",volRefNum);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSVolumeSize, 1, 2);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_short (DLP_REQUEST_DATA (req, 0, 0), volRefNum);

	result = dlp_exec (sd, req, &res);

	dlp_request_free (req);
	
	if (result > 0) {
		*volSizeUsed = get_long (DLP_RESPONSE_DATA (res, 0, 0));
		*volSizeTotal = get_long (DLP_RESPONSE_DATA (res, 0, 4));
	
		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
		     "DLP VFS Volume Size total: %d used: %d\n",
		     *volSizeTotal, *volSizeUsed));
	}

	dlp_response_free (res);

	return result;
}

int
dlp_VFSFileSeek(int sd, FileRef fileRef, int origin, int offset)
{
	int 	result;
	struct dlpRequest *req; 
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSFileSeek,"fileRef=%d origin=%d offset=%d",
	    fileRef,origin,offset);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSFileSeek, 1, 10);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_long (DLP_REQUEST_DATA (req, 0, 0), fileRef);
	set_short (DLP_REQUEST_DATA (req, 0, 4), origin);
	set_long (DLP_REQUEST_DATA (req, 0, 6), offset); 

	result = dlp_exec (sd, req, &res);

	dlp_request_free (req);
	dlp_response_free (res);

	return result;
}

int
dlp_VFSFileResize(int sd, FileRef fileRef, int newSize)
{
	int 	result;
	struct dlpRequest *req; 
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSFileResize,"fileRef=%d newSize=%d",fileRef,newSize);
	pi_reset_errors(sd);

	req = dlp_request_new(dlpFuncVFSFileResize, 1, 8);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_long(DLP_REQUEST_DATA(req, 0, 0), fileRef);
	set_long(DLP_REQUEST_DATA(req, 0, 4), newSize);
	
	result = dlp_exec(sd, req, &res);

	dlp_request_free (req);
	dlp_response_free (res);
	
	return result;
}

int
dlp_VFSFileSize(int sd, FileRef fileRef, int *size)
{
	int 	result;
	struct dlpRequest *req;
	struct dlpResponse *res;
	
	RequireDLPVersion(sd,1,2);
	TraceX(dlp_VFSFileSize,"fileRef=%d",fileRef);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncVFSFileSize, 1, 4);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_long (DLP_REQUEST_DATA (req, 0, 0), fileRef);
	
	result = dlp_exec (sd, req, &res);
	
	dlp_request_free (req);
	
	if (result > 0) {
		*size = get_long (DLP_RESPONSE_DATA (res, 0, 0));

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			"DLP VFS File Size: %d\n", *size));
	}
	
	dlp_response_free (res);

	return result;
}

int
dlp_ExpSlotMediaType(int sd, int slotNum, uint32_t *mediaType)
{
	int     result;
	struct dlpRequest *req;
	struct dlpResponse *res;
 
	RequireDLPVersion(sd,1, 4);
	TraceX(dlp_ExpSlotMediaType,"slotNum=%d",slotNum);
	pi_reset_errors(sd);

	req = dlp_request_new (dlpFuncExpSlotMediaType, 1, 2);
	if (req == NULL)
		return pi_set_error(sd, PI_ERR_GENERIC_MEMORY);

	set_short (DLP_REQUEST_DATA (req, 0, 0), slotNum);

	result = dlp_exec (sd, req, &res);

	dlp_request_free (req);

	if (result > 0) {
		*mediaType = get_long (DLP_RESPONSE_DATA (res, 0, 0));

		LOG((PI_DBG_DLP, PI_DBG_LVL_INFO,
			"DLP Media Type for slot %d: %4.4s\n", 
			slotNum, mediaType));
	}

	dlp_response_free (res);

	return result;
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
