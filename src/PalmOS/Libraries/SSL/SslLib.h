/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SslLib.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 *****************************************************************************/

#ifndef __SslLib68K_H__
#define __SslLib68K_H__

/********************************************************************
 * Constants
 ********************************************************************/

#include <NetMgr.h>

#define kSslLibType		sysFileTLibrary
#define kSslLibCreator		'ssl0'
#define kSslDBName		"SslLib"

/********************************************************************
 * Types
 ********************************************************************/

typedef struct SslLib_st		SslLib;
typedef struct SslContext_st		SslContext;
typedef UInt32 SslAttribute;

/* This strucure is a template used with the SslContextSet_IoStruct() and
 * SslContextGet_IoStruct() calls */
typedef struct SslSocket_st
	{
	NetSocketRef socket;
	UInt16 flags;
	UInt16 addrLen;
	Err err;
	Int32 timeout;
	NetSocketAddrType addr;
	} SslSocket;

/* These structures are used to define the InfoCallback
 * and the VerifyCallback */
typedef struct SslCallback_st SslCallback;
typedef Int32 (*SslCallbackFunc)(SslCallback *,
		Int32,Int32,void *);
struct SslCallback_st
	{
	void *reserved;
	SslCallbackFunc callback;
	void *data;
	SslContext *ssl;
	};

/* The protocol versions supported by SslLib.  This value is used with the
 * ProtocolVersion and SslCipherSuiteInfo attributes */
#define sslVersionSSLv3		0x0300

/* These are the possible errors returned from SslLib.
 * In addition to these errors, any NetLib errors that occur while SslLib
 * is performing network IO will be imediatly returned to the application */
//#define sslErrorClass                         0x3900
#define sslErrOk				(sslErrorClass+0) /* 3900 */
#define sslErrFailed				(sslErrorClass+1) /* 3901 */
#define sslErrEof				(sslErrorClass+2) /* 3902 */
#define sslErrOutOfMemory			(sslErrorClass+3) /* 3903 */
#define sslErrCbAbort				(sslErrorClass+4) /* 3904 */
#define sslErrIo				(sslErrorClass+5) /* 3905 */
#define sslErrNotFound				(sslErrorClass+6) /* 3906 */
#define sslErrDivByZero				(sslErrorClass+7) /* 3907 */
#define sslErrNoModInverse			(sslErrorClass+8) /* 3908 */
#define sslErrBadDecode				(sslErrorClass+9) /* 3909 */
#define sslErrInitNotCalled			(sslErrorClass+10) /* 390A */
#define sslErrBufferTooSmall			(sslErrorClass+11) /* 390B */
#define sslErrNullArg				(sslErrorClass+12) /* 390C */
#define sslErrBadLength				(sslErrorClass+13) /* 390D */
#define sslErrNoDmem				(sslErrorClass+14) /* 390E */
#define sslErrNoMethodSet			(sslErrorClass+15) /* 390F */
#define sslErrNoRandom				(sslErrorClass+16) /* 3910 */
#define sslErrBadArgument			(sslErrorClass+17) /* 3911 */
#define sslErrBadOption				(sslErrorClass+18) /* 3912 */
#define sslErrNotImplemented			(sslErrorClass+19) /* 3913 */
#define sslErrReallocStaticData			(sslErrorClass+20) /* 3914 */
#define sslErrInternalError			(sslErrorClass+21) /* 3915 */
#define sslErrRecordError			(sslErrorClass+37) /* 3925 */
#define sslErrCsp				(sslErrorClass+38) /* 3926 */
#define sslErrCert				(sslErrorClass+39) /* 3927 */
#define sslErrHandshakeEncoding			(sslErrorClass+40) /* 3928 */
#define sslErrMissingProvider			(sslErrorClass+41) /* 3929 */
#define sslErrHandshakeProtocol			(sslErrorClass+42) /* 392A */
#define sslErrExtraHandshakeData		(sslErrorClass+43) /* 392B */
#define sslErrWrongMessage			(sslErrorClass+44) /* 392C */
#define sslErrFatalAlert			(sslErrorClass+45) /* 392D */
#define sslErrBadPeerFinished			(sslErrorClass+46) /* 392E */
#define sslErrBadSignature			(sslErrorClass+47) /* 392F */

#define sslErrUnexpectedRecord			(sslErrorClass+49) /* 3931 */
#define sslErrReadAppData			(sslErrorClass+50) /* 3932 */
#define sslErrCertDecodeError			(sslErrorClass+51) /* 3933 */
#define sslErrUnsupportedCertType		(sslErrorClass+52) /* 3934 */
#define sslErrUnsupportedSignatureType		(sslErrorClass+53) /* 3935 */

/* These error code are returned due to errors encountered while verifiying
 * the peers certificate. */
#define sslErrVerifyBadSignature		(sslErrorClass+128) /* 3980 */
#define sslErrVerifyNoTrustedRoot		(sslErrorClass+129) /* 3981 */
#define sslErrVerifyNotAfter			(sslErrorClass+130) /* 3982 */
#define sslErrVerifyNotBefore			(sslErrorClass+131) /* 3983 */
#define sslErrVerifyConstraintViolation		(sslErrorClass+132) /* 3984 */
#define sslErrVerifyUnknownCriticalExtension	(sslErrorClass+133) /* 3985 */
#define sslErrVerifyCallback			(sslErrorClass+134) /* 3986 */
#define sslErrVerify(a) (((a) >= (sslErrorClass+128)) && \
			((a) <= (sslErrorClass+134)))

/* These options can be passed to SslOpen() */
#define sslOpenModeClear		0x0001/* Set_Mode(s,sslModeClear) */
#define sslOpenModeSsl			0x0002/* Set_Mode(s,sslModeSsl) */
#define sslOpenNewConnection		0x0004/* Set_Session(s,NULL) */
#define sslOpenNoAutoFlush		0x0008/* Set_AutoFlush(s,0) */
#define sslOpenUseDefaultTimeout	0x0020/* Ignore the timeout parameter */
#define sslOpenBufferedReuse		0x0040/* Set_BufferedReuse(s,1) */
#define sslOpenDelayHandshake		0x0080/* Set_BufferedReuse(s,1) */

/* Options to pass to SslClose() */
#define sslCloseUseDefaultTimeout	0x0020/* Ignore the timeout parameter */
#define sslCloseDontSendShutdown	0x0001/* Don't send a shutdown msg */
#define sslCloseDontWaitForShutdown	0x0002/* Don't wait for a shutdown msg*/

/* Values to use when calling SslXxxSet_Mode();  The sslModeFlush value
 * will only affect SslContextSet_Mode() and will cause the SslContexts
 * data bufferes to be flushed if they contain any data bytes (due to the
 * SslContext being reused */
#define sslModeClear            0x0000
#define sslModeSsl              0x0002
#define sslModeSslClient        0x000A
#define sslModeFlush        	0x8000

/* Values that can be passed to any SslCallback functions */
#define sslCmdNew		0x0001
#define sslCmdFree		0x0002
#define sslCmdReset		0x0003
#define sslCmdGet		0x0004
#define sslCmdSet		0x0005

/* Values that can be passed to specific SslCallback types */
#define sslCmdRead		0x0010	/* Internal use */
#define sslCmdWrite		0x0011	/* Internal use */
#define sslCmdInfo		0x0012	/* InfoCallback */
#define sslCmdVerify		0x0013	/* VerifyCallback */

/* Passed to SslXxxSet_InfoInterest() to specify which 'interest'
 * events we wish to be notified about */
#define sslFlgInfoAlert		0x0001
#define sslFlgInfoHandshake	0x0002
#define sslFlgInfoIo		0x0004
#define sslFlgInfoCert		0x0008

/* These values are the argi values to sslCmdInfo callback calls
 * (the InfoCallback) */
#define sslArgInfoHandshake	0x0001
#define sslArgInfoAlert		0x0002
#define sslArgInfoReadBefore	sslCmdRead		/* 0x0010 */
#define sslArgInfoReadAfter	(sslCmdRead|0x8000)	/* 0x8010 */
#define sslArgInfoWriteBefore	sslCmdWrite		/* 0x0011 */
#define sslArgInfoWriteAfter	(sslCmdWrite|0x8000)	/* 0x8011 */
#define sslArgInfoCert		0x0003

/* Turn on bug compatability, use with
 * SslXxxSet_Compat() */
#define sslCompatReuseCipherBug         0x0001
#define sslCompatNetscapeCaDnBug        0x0002
#define sslCompat1RecordPerMessage      0x0004
#define sslCompatBigRecords		0x0008
#define sslCompatAll                    0xffff

/* Specify the mode to operate in, use with
 * SslXxxSet_Mode() */
#define sslModeClear                    0x0000
#define sslModeSsl                      0x0002
#define sslModeSslClient                0x000A

/* Possible handshake states for the SSL protocol handshake.
 * Refer to the SSLv3 specification for an idea as to what the
 * states mean.  These values are returned by
 * SslContextGet_HsState(ssl); */
#define sslHsStateNone			0
#define sslHsStateStart			1
#define sslHsStateClientHello		2
#define sslHsStateServerHello		3
#define sslHsStateFlush			4
#define sslHsStateWriteFlush		5
#define sslHsStateWrite			6
#define sslHsStateCert			7
#define sslHsStateCertB			8
#define sslHsStateSkEx			9
#define sslHsStateSkExRsa		10
#define sslHsStateSkExDh		11
#define sslHsStateSkExAnonDh		12
#define sslHsStateCertReq		13
#define sslHsStateCertReqB		14
#define sslHsStateServerDone		15
#define sslHsStateClientCert		16
#define sslHsStateCkEx			17
#define sslHsStateWriteCcs		18
#define sslHsStateFinished		19
#define sslHsStateReadCcs		20
#define sslHsStateGenerateKeys		21
#define sslHsStateReadFinished		22
#define sslHsStateReadFinishedB		23
#define sslHsStateReadFinishedC		24
#define sslHsStateCleanup		25
#define sslHsStateDone			26
#define sslHsStateShutdown		27
#define sslHsStateClosed		28
#define sslHsStateHelloRequest		29
#define sslHsStateWriteClose		30

/* These are the defined Sslv3/TLSv1 alerts as definined in the SSLv3 and
 * TLSv1 specifications.  For their meanings, refer to those specifications.
 * These values can be returned by
 * SslContextGet_LastAlert(ssl); */
#define sslAlertCloseNotify		(0x0100+ 0)	/* SSL3 */
#define sslAlertUnexpectedMessage	(0x0200+10)	/* SSL3 */
#define sslAlertBadRecordMac		(0x0200+20)	/* SSL3 */
#define sslAlertDecryptionFailed       	(0x0200+21)	/* TLS */
#define sslAlertRecordOverflow         	(0x0200+22)	/* TLS */
#define sslAlertDecompressionFailure	(0x0200+30)	/* SSL3 */
#define sslAlertHandshakeFailure	(0x0200+40)	/* SSL3 */
#define sslAlertNoCertificate		(0x0100+41)	/* SSL3 */
#define sslAlertBadCertificate		(0x0100+42)	/* SSL3 */
#define sslAlertUnsupportedCertificate	(0x0100+43)	/* SSL3 */
#define sslAlertCertificateRevoked     	(0x0100+44)	/* SSL3 */
#define sslAlertCertificateExpired     	(0x0100+45)	/* SSL3 */
#define sslAlertCertificateUnknown     	(0x0100+46)	/* SSL3 */
#define sslAlertIllegalParameter       	(0x0200+47)	/* SSL3 */
#define sslAlertUnknownCa              	(0x0200+48)	/* TLS fatal */
#define sslAlertAccessDenied           	(0x0200+49)	/* TLS fatal */
#define sslAlertDecodeError            	(0x0200+50)	/* TLS fatal */
#define sslAlertDecryptError           	(0x0200+51)	/* TLS */
#define sslAlertExportRestricion       	(0x0200+60)	/* TLS fatal */
#define sslAlertProtocolVersion        	(0x0200+70)	/* TLS fatal */
#define sslAlertInsufficientSecurity   	(0x0200+71)	/* TLS fatal */
#define sslAlertInternalError          	(0x0200+80)	/* TLS fatal */
#define sslAlertUserCancled            	(0x0100+90)	/* TLS */
#define sslAlertNoRenegotiation        	(0x0100+100)	/* TLS */

/* The last NetLib operation performed by a SslContext, return values from
 * SslContextGet_LastIo(ssl); */
#define sslLastIoNone           0x00
#define sslLastIoRead           0x01
#define sslLastIoWrite          0x02

/* The last SslLib API call made that could cause NetLib activity, returned by
 * SslContextGet_LastApi(ssl); */
#define sslLastApiNone          0x00
#define sslLastApiOpen		0x01
#define sslLastApiRead          0x02
#define sslLastApiWrite         0x03
#define sslLastApiFlush         0x04
#define sslLastApiShutdown      0x05

/* SSLv3 Cipher suite identification strings.  These values are relevent to
 * the CipherSuites, CipherSuite, CipherSuiteInfo, SslSession and
 * SslCipherSuiteInfo attributes */
#define sslCs_RSA_RC4_128_MD5	0x00,0x04
#define sslCs_RSA_RC4_128_SHA1	0x00,0x05
#define sslCs_RSA_RC4_56_SHA1	0x00,0x64
#define sslCs_RSA_RC4_40_MD5	0x00,0x03

/* These values are used to decode some of the fields in the
 * SslCipherSuiteInfo structure */
#define sslCsiKeyExchNull	0x00
#define sslCsiKeyExchRsa	0x01
#define sslCsiAuthNULL		0x00
#define sslCsiAuthRsa		0x01
#define sslCsiDigestNull	0x00
#define sslCsiDigestMd5		0x01
#define sslCsiDigestSha1	0x02
#define sslCsiDigestMd2		0x03
#define sslCsiCipherNull	0x00
#define sslCsiCipherRc4		0x01

typedef struct SslCipherSuiteInfo_st {
	UInt8 cipherSuite[2];
	UInt16 cipher;			/* sslCsiCipherXXX */
	UInt16 digest;			/* sslCsiDigestXXX */
	UInt16 keyExchange;		/* sslCsiKeyExchXXX */
	UInt16 authentication;		/* sslCsiAuthXXX */
	UInt16 version;
	UInt16 cipherBitLength;
	UInt16 cipherKeyLength;
	UInt16 keyExchangeLength;
	UInt16 authenticationLength;
	UInt16 export;
	} SslCipherSuiteInfo;

typedef struct SslIoBuf_st
	{
	SslContext *ssl;
	UInt8 *ptr;
	UInt32 outNum;
	UInt32 inNum;
	UInt32 max;
	UInt32 err;
	UInt32 flags;
	} SslIoBuf;

typedef struct SslSession_st
        {
        /* Lets hope these all pack correctly with 0 padding */
        UInt32 length;
        UInt16 version;
        unsigned char cipherSuite[2];
        unsigned char compression;
        unsigned char sessionId[33];   /* First byte is the length */
        unsigned char masterSecret[48];/* Master secret */
        unsigned char time[8];          /* Start time - host specific */
        unsigned char timeout[4];       /* Timeout in seconds */
        /* Optional Peer certificate, it the offset to a SslExtendedItems
         * If the offset if 0, it does not exist */
        UInt16 certificateOffset;
        /* Can be used to store anything, perhaps hostname of peer?
         * Application defined */
        UInt16 extraData;
        /* Extra bytes are located on the end.  The offsets are from
         * the front of the structure */
        } SslSession;
        
typedef struct SslExtendedItem_st {
	UInt16 type;
	UInt16 field;
	UInt16 dataType;
	UInt16 len;
	UInt32 offset;
	} SslExtendedItem;

typedef struct SslExtendedItems_st
	{
	UInt32 length;
	UInt32 num;
	SslExtendedItem eitem[1]; /* Dummy first element */
	} SslExtendedItems;

/* These are the possible values for the SslVerify->state field */
#define sslVerifyFindParent		1
#define sslVerifySignature		2
#define sslVerifyNotBefore		3
#define sslVerifyNotAfterFindParent	4
#define sslVerifyExtensions		5
#define sslVerifyDone			6

typedef struct SslVerify_st
	{
	SslExtendedItems *certificate;
	SslExtendedItems *fieldItems;	/* Problem field base */
	UInt32		field;		/* Problem field */
	SslExtendedItems *ex;		/* Extension */
	UInt32 depth;			/* Certificate depth */
	UInt32 state;			/* Verification state */
	} SslVerify;

#include <SslLibMac.h>
#include <SslLibAsn1.h>

/********************************************************************
 * Traps
 ********************************************************************/
#include <LibTraps.h>

#define kSslLibName					sysLibTrapName
#define kSslLibOpen					sysLibTrapOpen
#define kSslLibClose					sysLibTrapClose
#define kSslLibWake					sysLibTrapWake
#define kSslLibSleep					sysLibTrapSleep

#define kSslLibCreate  	        			(sysLibTrapCustom)
#define	kSslLibDestroy					(sysLibTrapCustom+1)
#define	kSslLibSetLong					(sysLibTrapCustom+2)
#define	kSslLibGetLong					(sysLibTrapCustom+3)
#define	kSslLibSetPtr					(sysLibTrapCustom+4)
#define	kSslLibGetPtr					(sysLibTrapCustom+5)
#define	kSslContextCreate				(sysLibTrapCustom+6)
#define	kSslContextDestroy				(sysLibTrapCustom+7)
#define	kSslContextGetLong				(sysLibTrapCustom+8)
#define	kSslContextSetLong				(sysLibTrapCustom+9)
#define	kSslContextGetPtr				(sysLibTrapCustom+10)
#define	kSslContextSetPtr				(sysLibTrapCustom+11)
#define	kSslOpen					(sysLibTrapCustom+12)
#define	kSslClose					(sysLibTrapCustom+13)
#define	kSslSend					(sysLibTrapCustom+14)
#define	kSslReceive					(sysLibTrapCustom+15)
#define	kSslRead					(sysLibTrapCustom+16)
#define	kSslWrite					(sysLibTrapCustom+17)
#define	kSslPeek					(sysLibTrapCustom+18)
#define kSslConsume					(sysLibTrapCustom+19)
#define	kSslFlush					(sysLibTrapCustom+20)

/********************************************************************
 * Prototypes
 ********************************************************************/

Err SslLibName(UInt16 refnum)
			SYS_TRAP(kSslLibName);
Err SslLibOpen(UInt16 refnum)
			SYS_TRAP(kSslLibOpen);
Err SslLibClose(UInt16 refnum)
			SYS_TRAP(kSslLibClose);
Err SslLibSleep(UInt16 refnum)
			SYS_TRAP(kSslLibSleep);
Err SslLibWake(UInt16 refnum)
			SYS_TRAP(kSslLibWake);

Err SslLibCreate(UInt16 refnum,SslLib **lib)
			SYS_TRAP(kSslLibCreate);
Err SslLibDestroy(UInt16 refnum,SslLib *lib)
			SYS_TRAP(kSslLibDestroy);
Err SslLibSetLong(UInt16 refnum,SslLib *lib,SslAttribute attr,Int32 value)
			SYS_TRAP(kSslLibSetLong);
Err SslLibSetPtr(UInt16 refnum,SslLib *lib,SslAttribute attr,void *value)
			SYS_TRAP(kSslLibSetPtr);
Int32 SslLibGetLong(UInt16 refnum,SslLib *lib,SslAttribute attr)
			SYS_TRAP(kSslLibGetLong);
Err SslLibGetPtr(UInt16 refnum,SslLib *lib,SslAttribute attr,void **value)
			SYS_TRAP(kSslLibGetPtr);

Err SslContextCreate(UInt16 refnum,SslLib *lib,SslContext **ctx)
			SYS_TRAP(kSslContextCreate);
Err SslContextDestroy(UInt16 refnum,SslContext *ctx)
			SYS_TRAP(kSslContextDestroy);
Err SslContextSetLong(UInt16 refnum,SslContext *lib,SslAttribute attr,Int32 v)
			SYS_TRAP(kSslContextSetLong);
Err SslContextSetPtr(UInt16 refnum,SslContext *lib,SslAttribute attr,void *v)
			SYS_TRAP(kSslContextSetPtr);
Int32 SslContextGetLong(UInt16 refnum,SslContext *lib,SslAttribute attr)
			SYS_TRAP(kSslContextGetLong);
Err SslContextGetPtr(UInt16 refnum,SslContext *lib,SslAttribute attr,void **v)
			SYS_TRAP(kSslContextGetPtr);

Err SslOpen(UInt16 refnum,SslContext *ctx,UInt16 mode,UInt32 timeout)
			SYS_TRAP(kSslOpen);
Err SslClose(UInt16 refnum,SslContext *ctx,UInt16 mode,UInt32 timeout)
			SYS_TRAP(kSslClose);
Int16 SslSend(UInt16 refnum,SslContext *ctx,void *buffer,UInt16 bufferLen,
		UInt16 flags,void *toAddr, UInt16 toLen, Int32 timeout,
		Err *errRet)
			SYS_TRAP(kSslSend);
Int16 SslReceive(UInt16 refnum,SslContext *ctx,void *buffer,UInt16 bufferLen,
		UInt16 flags, void *fromAddr, UInt16 fromLen,
		Int32 timeout,Err *errRet)
			SYS_TRAP(kSslReceive);
Int32 SslRead(UInt16 refnum,SslContext *ctx,void *buffer,
		Int32 bufferLen,Err *errRet)
			SYS_TRAP(kSslRead);
Int32 SslWrite(UInt16 refnum,SslContext *ctx,void *buffer,
		Int32 bufferLen,Err *errRet)
			SYS_TRAP(kSslWrite);
Err SslPeek(UInt16 refnum,SslContext *ctx,void **buffer_ptr,
		Int32 *availableBytes,Int32 readSize)
			SYS_TRAP(kSslPeek);
void SslConsume(UInt16 refnum, SslContext *ctx, Int32 number)
			SYS_TRAP(kSslConsume);
Err SslFlush(UInt16 refnum,SslContext *ctx,Int32 *outstanding)
			SYS_TRAP(kSslFlush);
/*----------------------------------------------------------------*/


#endif 
