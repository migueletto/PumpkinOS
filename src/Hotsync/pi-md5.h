/* include/pi-md5.h.  Generated from pi-md5.h.in by configure.  */
/*
 * $Id: pi-md5.h.in,v 1.1 2008/11/06 01:14:13 judd Exp $
 *
 * pi-md5.h: Header file for Colin Plumb's MD5 implementation.
 *           Modified by Ian Jackson so as not to use Colin Plumb's
 *           'usuals.h'.
 *           
 *           Originally intended to be used for Palm password support
 * 
 *           This file is in the public domain.
 */

#ifndef MD5_H
#define MD5_H

#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1

#if HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#if HAVE_STDINT_H
#include <stdint.h>
#endif
//#ifdef HAVE_INTTYPES_H || HAVE_STDINT_H
#ifdef HAVE_STDINT_H
#define UINT8 uint8_t
#define UINT32 uint32_t
#else
#define UINT8 unsigned char
#define UINT32 unsigned int
#endif

struct MD5Context {
	UINT32 buf[4];
	UINT32 bytes[2];
	UINT32 in[16];
};

void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context *context, UINT8 const *buf, unsigned len);
void MD5Final(unsigned char digest[16], struct MD5Context *context);
void MD5Transform(UINT32 buf[4], UINT32 const in[16]);

void byteSwap(UINT32 * buf, unsigned words);

#endif				/* !MD5_H */
