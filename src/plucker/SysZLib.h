/* SysZLib.h -- interface of the 'SysZLib' shared library
 *
 *  Original version by Tom Zerucha (tz@execpc.com) 2000
 *  Revisions and ARMlet support by Justin Clark (jclark@copera.com) 2003
 */

#ifndef __ZLIB_H__
#define __ZLIB_H__

#include <PalmOS.h>
#include "zlib.h"

#define ZLIB_VERSION "1.1.4"

/* Sections taken from zconf.h needed for the interface */

#define FAR
typedef unsigned int  uInt;  /* 16 bits or more */
typedef unsigned long uLong; /* 32 bits or more */
typedef unsigned char Bytef;
typedef char  FAR charf;
typedef int   FAR intf;
typedef uInt  FAR uIntf;
typedef uLong FAR uLongf;
typedef void *voidpf;
typedef void *voidp;
#define OF(args) args


/* constants */
#define Z_NO_FLUSH      0
#define Z_PARTIAL_FLUSH 1 /* will be removed, use Z_SYNC_FLUSH instead */
#define Z_SYNC_FLUSH    2
#define Z_FULL_FLUSH    3
#define Z_FINISH        4
/* Allowed flush values; see deflate() below for details */
#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NEED_DICT     2
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
#define Z_VERSION_ERROR (-6)
/* Return codes for the compression/decompression functions. Negative
 * values are errors, positive values are used for special but normal events.
 */

#define Z_NO_COMPRESSION         0
#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)
/* compression levels */

#define Z_FILTERED            1
#define Z_HUFFMAN_ONLY        2
#define Z_DEFAULT_STRATEGY    0
/* compression strategy; see deflateInit2() below for details */

#define Z_BINARY   0
#define Z_ASCII    1
#define Z_UNKNOWN  2
/* Possible values of the data_type field */

#define Z_DEFLATED   8
/* The deflate compression method (the only one supported in this version) */

#define Z_NULL  0  /* for initializing zalloc, zfree, opaque */

#endif /* __ZLIB_H__ */
