/******************************************************************************
 *
 * Copyright (c) 1997-2003 PalmSource, Inc. All rights reserved.
 *
 * File: CTP.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	 Header for the CTP Proxy service component of Elaine. 
 *
 *****************************************************************************/

#ifndef _CTP_H_
#define _CTP_H_ 

#include "PalmLocRawData.h"

//##########################################################
// CTP Commands
// This is the CTP command UIntV which is sent in every
//  CTP Request in the common area (CTPReqCommon)
//##########################################################
typedef enum {
	ctpCmdReqURL = 0,					// request URL
	ctpCmdReqMail = 1,				// request mail
	ctpCmdEcho = 2,
	ctpCmdMsgGen = 3,
	ctpCmdDiscard = 4,
	ctpCmdReqSessionID = 5
	} CTPCmdEnum;


//##########################################################
// CTP Content Conversion Types
// The requestor can pass one of these constants in the
//   ctpExtConvertTo extension in order to specify the
//	 the type of content conversion desired.
// If the extension is missing, then ctpConvCML is assumed.
//##########################################################
typedef enum {
	ctpConvCML = 0,					// CML 
	ctpConvCML8Bit,					// CML in 8-bit debug form
	ctpConvNoneLZ77,					// Raw data with LZ77 (No primer)
	ctpConvNone,						// Return in native web format.
											//   When this is specified, then the response
											//   will include the ctpRspExtContentType
											//   and ctpRspExtContentEncoding extensions
	ctpConvBest,						// Best compression between ctpConvCML, ctpConvCML8Bit & ctpConvCMLLZ77Primer1
											// Used together: ctpConvBest, ctpExtCompressSupported & CtpExtCompressedWith
	ctpConvCMLLZ77Primer1			// Lz77 Primer#1 (English Clipper)
	} CTPConvEnum;

// ctpExtCompressSupported Bits definition
#define	ctpSupportCml5Bit			0x01
#define	ctpSupportCml8Bit			0x02
#define	ctpSupportLZ77Primer1	0x04
#define	ctpSupportLZ77				0x08
// This device supports cml5bit, cml8bit and Lz77 Primer #1
#define	ctpSupportDecompress		(ctpSupportCml5Bit | ctpSupportCml8Bit | ctpSupportLZ77Primer1)

// inetFtrNumCtpDeviceBits1 feature Bits definition (See INetMgr.h for the feature definition)
#define	ctpDeviceBits1Ctp1252Secondary	0x00000001	// Cp1252 is supported in Clipper as a secondary charset for Japanese devices


// default CTP header request values
#define ctpHeaderVersion	 0					// current version of CTP header
#define ctpConvAlgorithm	ctpConvCML		// encoding type	
#define ctpMaxResponseSize 1024				// max response size
#define ctpContentWidth		160				// displayable width
#define ctpContentDepth		1				// 1 = 2 bit default display depth, 2 = 4 bit, 3 = 8 bit

#define ctpContentVersion1			0x8001
#define ctpContentVersion2			0x8002
#define ctpContentVersionFirst	ctpContentVersion1
#define ctpContentVersionLast		ctpContentVersion2
#define ctpContentVersion			ctpContentVersionLast	// encoding version (must be kept up to date!!!)

#define ctpCharsEscapedByClient	"=&/%:?#+;@*["


//-----------------------------------------------------------------------------
// %WCDevCaps bits definition Palm OS 4.0 and after
//-----------------------------------------------------------------------------
#define	wcDevCaps1ScreenBits						0x0000007F
 #define wcDevCaps1ScreenBitShift				0
 #define wcDevCaps1ScreenBitDepth1				0x01
 #define wcDevCaps1ScreenBitDepth2				0x02
 #define wcDevCaps1ScreenBitDepth4				0x04
 #define wcDevCaps1ScreenBitDepth8				0x08
 #define wcDevCaps1ScreenBitDepth16				0x10
#define	wcDevCaps1Lz77								0x00000080
 #define wcDevCaps1Reserved1Shift				7
#define	wcDevCaps1CommBandwidthBits			0x00000F00
 #define wcDevCaps1CommBandwidthBitsShift		8
#define	wcDevCaps1MemOSFreeBits					0x0000F000
 #define wcDevCaps1MemOSFreeBitsShift			12
#define	wcDevCaps1PalmOSVerBits					0x001F0000
 #define wcDevCaps1PalmOSVerBitsShift			16
#define	wcDevCaps1PalmOSHeapSizeBits			0x00E00000
 #define wcDevCaps1PalmOSHeapSizeBitsShift	21
#define	wcDevCaps1Reserved2						0xFF000000
 #define wcDevCaps1Reserved2Shift				24


//##########################################################
// CTP Request Extensions
// These are the defined extension IDs that can appear
//  in the extensions area of a CTP request
//##########################################################

// Extensions defined for the ctpCmdReqMail command....
typedef enum {
	ctpExtIncHTTPResponse = 1,		// include HTTP response header in data
									//   only applicable if ctpExtConvertTo
									//   is set to ctpConvNone

	ctpExtEnableCookies = 2,		// Cookies are enabled on the device
									// If only this flag is on, the cookies
									// are to be sent to the device

	ctpExtNetID = 128,		// Followed by length byte of 4 and
									//  4 byte networkID..
									//  Must be specified when using TCP
									//  because tunneler is not used.

	ctpExtUserID = 129,		// Followed by length byte of 4 and
									//  4 byte userID.
									//  Must be specified when using TCP
									//  because Mobitex MAN# is not available

	ctpExtUserPW = 130,		// Followed by length byte of 16 and
									//  128 bit (16 byte) MD5 hashed password. 
									//  Must be specified when using TCP
									//  because Mobitex MAN# is not available

	ctpExtUserName = 131,	// Followed by length byte  and
									//  ascii user name string. 
									//  This or ctpExtUserID must be specified 
									//  when using TCP because Mobitex MAN# 
									//  is not available

	ctpExtServer = 132,		// Followed by a length byte of 1 and
									//  8 bits (1 byte) of server behavior
									//  flags.
									
	ctpExtConvertTo = 133,	// Followed by length byte of 1 and
									//   a 1 byte conversion identifier
									//   ctpConvXXX. If not present then
									//   ctpConvCML is assumed. 

	ctpExtLocale = 134,		// Followed by length byte of 3;
									//   1 byte Country as defined by CountryType in Preferences.h
									//   1 byte Language as defined by LanguageType in Preferences.h
									//   1 byte CharsetID as defined by CharEncodingType in TextMgr.h
									// Extension must not be sent if Local =
									// ( cUnitedStates, lEnglish, charEncodingPalmLatin )

	ctpExtCompressSupported = 135,	// During Implementation of LZ77
											// Used together: ctpConvBest, ctpExtCompressSupported & CtpExtCompressedWith
											// Followed by length byte of 1 and
											// 8 compression support bits

	ctpExtDeviceBits1 = 136, // Device may send bits to the server
									 // Followed by a length byte of 4
									 // and 4 bytes specifying the bits
									 // See bellow for a set of bits.

	ctpExtSendRawLocationInfo = 137, // Raw data information as returned by the connected hardware
									 // Followed by a length byte and the data

	ctpExtCookie = 138,				// Cookie Request Header from the device

	ctpExtContTypeDevice = 139,     // Device may send Content Type to the server
									// Followed by a length byte and the data

	ctpExtWCDevCaps = 140,     		// Web Clipping device Capabilities available through %WCDevCaps
									// Followed by a variable length byte and the data

	ctpExtPalmUserID = 141, // Followed by a length of 16 bytes
									// containing 8 bytes of PalmUserID
									// and 8 bytes of DeviceID

	ctpExtLastReqExtEnum = 255		// The last legal extension id.
	} CTPReqExtEnum;

// Enumerations for ctp extened extensions
// These extensions have a 16-bit length field
typedef enum {
	ctpExtExtCookie = 0				// used when the device must send a large cookie to the web-clipping proxy server
} CTPReqExtExtEnum;

// Bits definition for ctpExtDeviceBits1:
// initialized by the inetFtrNumCtpDeviceBits1 feature Bits definition
// (See INetmgr.h for the feature definition)
#define	ctpDeviceBits1Cp1252Secondary		0x00000001	// Cp1252 is supported in Clipper as a secondary charset
//
#define	ctpDeviceBits1PostCharsetBitShift	1			// Shift bits left by 1 bit
#define	ctpDeviceBits1PostCharsetBitMask	0x000001FE


//##########################################################
// CTP Response Extensions
// These are the defined extension IDs that can appear
//  in the extensions area of a CTP  response
//##########################################################
// Extensions defined for the ctpCmdReqURL response
typedef enum {
	ctpExtContentType = 128,		// Followed by length byte of 'len' and
									//   a 'len' byte string containing the
									//   MIME type of the content. This extension
									//   is only returned when the request has
									//   a ctpReqExtConverTo value of ctpConvNone
	
	ctpExtContentEncoding = 129,	// Followed by length byte of 'len' and
									//   a 'len' byte string containing the
									//   encoding type ("x-gzip, "compress", "lz77", 
									//   etc.)
									//   This extension is only returned when the 
									//   request has a ctpReqExtConverTo value of 
									//   ctpConvNone

	ctpExtUncompressSize = 130,		// Followed by length byte of 4 and
									//   a UInt32 containing the uncompressed
									//   size of the content. Currently only
									//   sent when the requested content conversion
									//   type is ctpConvCMLLZSS

	ctpExtUntruncatedSize = 131,	// Followed by length byte of 4 and
									//   a UInt32 containing the untruncated
									//   size of the content. Currently only
									//   sent when the CTP result is ctpWarnTruncated

	ctpExtContentVersion = 132,	// Followed by length byte of 2
									//	and 2 byte desired version number of content

	ctpExtCompressedWith = 133,	// During Implementation of LZ77
									// Used together: ctpConvBest, ctpExtCompressSupported & CtpExtCompressedWith
									// Followed by a length byte of 1
									// and 1 byte specifying the specific CTPConvEnum used
	
	ctpExtServerBits1 = 134,	// Server may send bits to the device
									// Followed by a length byte of 4
									// and 4 bytes specifying the bits
									// See bellow for a set of bits.
									
	ctpExtSetCookie = 135,		// SetCookie http response header
								// Variable length data which is tagged by field lengths

	ctpExtLastRspExtURLEnum = 255	// The last ctp extension;
	} CTPRspExtURLEnum;

// Enumerations for ctp extened extensions
// These extensions have a 16-bit length field
typedef enum {
	ctpExtExtSetCookie = 0		// Used to send large cookies to the device
} CTPRspExtExtURLEnum;

// Bits definition for ctpExtServerBits1:
#define	ctpServerBits1SecondaryCharsetUsed		0x00000001
//
#define	ctpServerBits1DocumentCharsetBitShift	1			// Shift bits left by 1 bit
#define	ctpServerBits1DocumentCharsetBitMask	0x000001FE

#define ctpServerBits1DoNotCache				0x00000200
//##########################################################
// CTP Network IDs.
// These are used along with the userID to identify a user's
//   mailbox account. All mailboxes created for the Mobitex
//   network with have a network ID of 0. The network ID
//   is passed along with the userID to the mail proxy. 
// For wireless networks, the tunneler fills in the network
//  ID into the ?? field of the tunneled IP packets. For wireline
//  access, the client must include the network ID in the extensions
//  field of the CTP mail request. 
//##########################################################
typedef enum {
	ctpNetIDMobitex = 0,
	ctpNetIDUnknown = 0xFFFF
	} CTPNetIDEnum;


//##########################################################
// CTP Schemes
// This is the scheme encoding which is used as a field
//  in the document address portion in a URL request
//##########################################################
typedef enum {
	ctpSchemeHTTP = 0,			// http://
	ctpSchemeHTTPS = 1,			// https://
	ctpSchemeFTP = 2,				// ftp://
	ctpSchemeEmpty = 7
	} CTPSchemeEnum;




//##########################################################
// CTP Errors
// These are the possible error codes returned in a CTP response
//##########################################################
typedef enum {
	ctpErrNone = 0,
	ctpErrMalformedRequest,			// malformed CTP-layer request from
										// client
										//  Optional Data:
										//    UInt32		ctpErr
										//	  Char		errMsg[]
										//
	ctpErrUnknownCmd,					// unsupported CTP command
										//  no extra data
										//
	ctpErrProxy,						// content or other err specific to
										// a Palm Proxy Server
										// (ie, messaging or clipping proxy) 
										//  Data:
										//    UInt32		proxyErr
										//	  Char		errMsg[]
										//
	ctpErrServer,						// err on WEB Server
										//  Data:
										//    UInt32		serverErr
										//	  Char		errMsg[]
										//
	ctpErrSecReplayIDBad,				// encrypted request had an
										//   invalid sequence number 
										//	 or sessionID, or anything 
										//   for replay attack
										//   no extra data
										//
	ctpErrSecPublicKeyBad,				// encrypted request is using an
										//   invalid server public key
										//  Data:
										//    UInt8		newPubKey[]
										//
										// GT, 01-04-99.
	ctpErrMsgProxy,						// messaging proxy specific error
										//  Data:
										//    UInt32		proxyErr
										//	  Char		errMsg[]
										//
	ctpErrRedirectURL,					// Redirecting the URL all the way to
										// the device,
										// Data:
										//	  UInt8		redirectedURL[]
										//
	ctpWarnTruncated = 0x8000			// response is truncated
	} CTPErrEnum;


// NOTE:
// ctpErrMalformedRequest is an error at the outer CTP
// layer of the protocol - i.e. the outer wrapper around either a Web or Mail
// request. Errors at this level are very basic, akin to errors at the IP layer
// over TCP/IP networks. This kind of error is discovered before Elaine even
// knows if it's a web or mail request.

// The ctpErrProxy errors are errors discovered in the "insides" of a CTP request,
// akin to having bad data in an FTP request for example. These aren't
// discovered until the CTP request wrapper has been stripped off and the guts
// of the request have been passed to the portion of Elaine that handles web
// or mail requests specifically.


//=====================================================
// Functions
//====================================================
#ifdef __cplusplus
extern "C" {
#endif

// <chg 2-13-98 RM> 
// NOTE: The following routines are no longer "public" system calls.
//  They are now static private functions in :Libraries:INet:Src:INetLibWebCTP.c
//  that can be included into another source file if needed. 
// The only place they are used is in the INetLib and some test code.
//
// UInt32	PrvCTPURLEncode(Char * urlP, UInt8 * dstP, UInt32 * dstBitOffsetP,
//			UInt32	dstBitLen);

// UInt32	PrvCTPURLDecode(UInt8 * srcP, UInt32 * srcBitOffsetP,
//			Char * urlP, UInt32	urlBufSize);

// Convert a base26 ascii string as used in a CTP URL into an
//  integer. Return's 0 if no error, updates *strPP to point
//  to next character past base26 ascii string. 
// UInt32	PrvCTPBase26AToI(Char ** strPP, UInt32 * intP);


// MACRO: CtpMaxEncodedURLSize(UInt32 srcByteCount)
//
// Given a source URL size, including terminating zero-byte, returns
// the buffer size (in bytes) that is required to hold the encoded URL;
//
// This value is arrived at as follows: We use the 5-bit encoding with every
// byte escaped as the limit; this consists of 3 scheme bits, 1 encoding bit
// and the string characters where each character is preceded with a 5-bit escape,
// and the terminating zero-byte; plus, we add 10 bytes as a fudge factor for good
// measure.  (in this model, every string byte has 5/8ths byte overhead; we
// increase this to 6/8 to get a total of 1 6/8 or 7/4 result bytes for every
// source byte)
//
// *IMPORTANT* if the encoding method changes, this macro will need to be updated.
//
#define CtpMaxEncodedURLSize(srcByteCount)	(1 + (srcByteCount * 7 / 4) + 10)


#ifdef __cplusplus
}
#endif

#endif /* _CTP_H_ */
