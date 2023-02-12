/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SslLibMac.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 *****************************************************************************/

#ifndef __SslLibMac68K_H__
#define __SslLibMac68K_H__

#define sslAttrLibCompat		0x0F010103
#define sslAttrLibInfoInterest		0x0F020203
#define sslAttrLibProtocolVersion	0x0F030303
#define sslAttrLibMode			0x0F040403
#define sslAttrLibInfoCallback		0x0F080805
#define sslAttrLibVerifyCallback	0x0F0A0A05
#define sslAttrLibReadStreaming		0x0F0B0B02
#define sslAttrLibAutoFlush		0x0F0C0C02
#define sslAttrLibBufferedReuse		0x0F0D0D02
#define sslAttrLibAppPtr		0x0F0E0E01
#define sslAttrLibAppInt32		0x0F0F0F03
#define sslAttrLibRbufSize		0x0F101003
#define sslAttrLibWbufSize		0x0F111103
#define sslAttrLibDontSendShutdown	0x0F121202
#define sslAttrLibDontWaitForShutdown	0x0F131302
#define sslAttrCompat			0x0F010113
#define sslAttrInfoInterest		0x0F020213
#define sslAttrProtocolVersion		0x0F030313
#define sslAttrMode			0x0F048013
#define sslAttrErrorState		0x0F05FF14
#define sslAttrReadStreaming		0x0F060612
#define sslAttrAutoFlush		0x0F070712
#define sslAttrBufferedReuse		0x0F080812
#define sslAttrAppPtr			0x0F090911
#define sslAttrAppInt32			0x0F0A0A13
#define sslAttrInfoCallback		0x0F0E0E15
#define sslAttrVerifyCallback		0x0F101015
#define sslAttrError			0x0F111113
#define sslAttrHsState			0x0F12FF13
#define sslAttrLastAlert		0x0F131313
#define sslAttrSessionReused		0x0F14FF12
#define sslAttrWriteBufPending		0x0F15FF13
#define sslAttrReadBufPending		0x0F16FF13
#define sslAttrReadRecPending		0x0F17FF13
#define sslAttrReadOutstanding		0x0F18FF13
#define sslAttrRbufSize			0x0F1B8113
#define sslAttrWbufSize			0x0F1C8213
#define sslAttrStreaming		0x0F1DFF12
#define sslAttrLastIo			0x0F1EFF12
#define sslAttrLastApi			0x0F1FFF12
#define sslAttrClientCertRequest	0x0F20FF12
#define sslAttrDontSendShutdown		0x0F212112
#define sslAttrDontWaitForShutdown	0x0F222212
#define sslAttrCspSslSession		0x00808001
#define sslAttrCspCipherSuites		0x00008101
#define sslAttrCspCipherSuite		0x0001FF01
#define sslAttrCspCipherSuiteInfo	0x0082FF01
#define sslAttrCertPeerCert		0x0100FF01
#define sslAttrCertSslVerify		0x0101FF04
#define sslAttrCertPeerCommonName	0x0180FF01
#define sslAttrIoStruct			0x04008001
#define sslAttrIoSocket			0x04010103
#define sslAttrIoTimeout		0x04020203
#define sslAttrIoFlags			0x04030303

#define SslLibGet_Compat(refnum,lib) \
	SslLibGetLong((refnum),(lib),sslAttrLibCompat)
#define SslLibSet_Compat(refnum,lib,v) \
	SslLibSetLong((refnum),(lib),sslAttrLibCompat,(v))
#define SslLibGet_InfoInterest(refnum,lib) \
	SslLibGetLong((refnum),(lib),sslAttrLibInfoInterest)
#define SslLibSet_InfoInterest(refnum,lib,v) \
	SslLibSetLong((refnum),(lib),sslAttrLibInfoInterest,(v))
#define SslLibGet_ProtocolVersion(refnum,lib) \
	SslLibGetLong((refnum),(lib),sslAttrLibProtocolVersion)
#define SslLibSet_ProtocolVersion(refnum,lib,v) \
	SslLibSetLong((refnum),(lib),sslAttrLibProtocolVersion,(v))
#define SslLibGet_Mode(refnum,lib) \
	SslLibGetLong((refnum),(lib),sslAttrLibMode)
#define SslLibSet_Mode(refnum,lib,v) \
	SslLibSetLong((refnum),(lib),sslAttrLibMode,(v))
#define SslLibGet_InfoCallback(refnum,lib,v) \
	SslLibGetPtr((refnum),(lib),sslAttrLibInfoCallback,(void **)(v))
#define SslLibSet_InfoCallback(refnum,lib,v) \
	SslLibSetPtr((refnum),(lib),sslAttrLibInfoCallback,(void *)(v))
#define SslLibGet_VerifyCallback(refnum,lib,v) \
	SslLibGetPtr((refnum),(lib),sslAttrLibVerifyCallback,(void **)(v))
#define SslLibSet_VerifyCallback(refnum,lib,v) \
	SslLibSetPtr((refnum),(lib),sslAttrLibVerifyCallback,(void *)(v))
#define SslLibGet_ReadStreaming(refnum,lib) \
	SslLibGetLong((refnum),(lib),sslAttrLibReadStreaming)
#define SslLibSet_ReadStreaming(refnum,lib,v) \
	SslLibSetLong((refnum),(lib),sslAttrLibReadStreaming,(v))
#define SslLibGet_AutoFlush(refnum,lib) \
	SslLibGetLong((refnum),(lib),sslAttrLibAutoFlush)
#define SslLibSet_AutoFlush(refnum,lib,v) \
	SslLibSetLong((refnum),(lib),sslAttrLibAutoFlush,(v))
#define SslLibGet_BufferedReuse(refnum,lib) \
	SslLibGetLong((refnum),(lib),sslAttrLibBufferedReuse)
#define SslLibSet_BufferedReuse(refnum,lib,v) \
	SslLibSetLong((refnum),(lib),sslAttrLibBufferedReuse,(v))
#define SslLibGet_AppPtr(refnum,lib,v) \
	SslLibGetPtr((refnum),(lib),sslAttrLibAppPtr,(void **)(v))
#define SslLibSet_AppPtr(refnum,lib,v) \
	SslLibSetPtr((refnum),(lib),sslAttrLibAppPtr,(void *)(v))
#define SslLibGet_AppInt32(refnum,lib) \
	SslLibGetLong((refnum),(lib),sslAttrLibAppInt32)
#define SslLibSet_AppInt32(refnum,lib,v) \
	SslLibSetLong((refnum),(lib),sslAttrLibAppInt32,(v))
#define SslLibGet_RbufSize(refnum,lib) \
	SslLibGetLong((refnum),(lib),sslAttrLibRbufSize)
#define SslLibSet_RbufSize(refnum,lib,v) \
	SslLibSetLong((refnum),(lib),sslAttrLibRbufSize,(v))
#define SslLibGet_WbufSize(refnum,lib) \
	SslLibGetLong((refnum),(lib),sslAttrLibWbufSize)
#define SslLibSet_WbufSize(refnum,lib,v) \
	SslLibSetLong((refnum),(lib),sslAttrLibWbufSize,(v))
#define SslLibGet_DontSendShutdown(refnum,lib) \
	SslLibGetLong((refnum),(lib),sslAttrLibDontSendShutdown)
#define SslLibSet_DontSendShutdown(refnum,lib,v) \
	SslLibSetLong((refnum),(lib),sslAttrLibDontSendShutdown,(v))
#define SslLibGet_DontWaitForShutdown(refnum,lib) \
	SslLibGetLong((refnum),(lib),sslAttrLibDontWaitForShutdown)
#define SslLibSet_DontWaitForShutdown(refnum,lib,v) \
	SslLibSetLong((refnum),(lib),sslAttrLibDontWaitForShutdown,(v))
#define SslLibGet_CipherSuites(refnum,ssl,v) \
	SslLibGetPtr((refnum),(lib),sslAttrCspCipherSuites,(void **)(v))
#define SslLibSet_CipherSuites(refnum,lib,v) \
	SslLibSetPtr((refnum),(lib),sslAttrCspCipherSuites,(void *)(v))

#define SslContextGet_Compat(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrCompat)
#define SslContextSet_Compat(refnum,ssl,v) \
	SslContextSetLong((refnum),(ssl),sslAttrCompat,(v))
#define SslContextGet_InfoInterest(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrInfoInterest)
#define SslContextSet_InfoInterest(refnum,ssl,v) \
	SslContextSetLong((refnum),(ssl),sslAttrInfoInterest,(v))
#define SslContextGet_ProtocolVersion(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrProtocolVersion)
#define SslContextSet_ProtocolVersion(refnum,ssl,v) \
	SslContextSetLong((refnum),(ssl),sslAttrProtocolVersion,(v))
#define SslContextGet_Mode(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrMode)
#define SslContextSet_Mode(refnum,ssl,v) \
	SslContextSetLong((refnum),(ssl),sslAttrMode,(v))
#define SslContextGet_ReadStreaming(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrReadStreaming)
#define SslContextSet_ReadStreaming(refnum,ssl,v) \
	SslContextSetLong((refnum),(ssl),sslAttrReadStreaming,(v))
#define SslContextGet_AutoFlush(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrAutoFlush)
#define SslContextSet_AutoFlush(refnum,ssl,v) \
	SslContextSetLong((refnum),(ssl),sslAttrAutoFlush,(v))
#define SslContextGet_BufferedReuse(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrBufferedReuse)
#define SslContextSet_BufferedReuse(refnum,ssl,v) \
	SslContextSetLong((refnum),(ssl),sslAttrBufferedReuse,(v))
#define SslContextGet_AppPtr(refnum,ssl,v) \
	SslContextGetPtr((refnum),(ssl),sslAttrAppPtr,(void **)(v))
#define SslContextSet_AppPtr(refnum,ssl,v) \
	SslContextSetPtr((refnum),(ssl),sslAttrAppPtr,(void *)(v))
#define SslContextGet_AppInt32(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrAppInt32)
#define SslContextSet_AppInt32(refnum,ssl,v) \
	SslContextSetLong((refnum),(ssl),sslAttrAppInt32,(v))
#define SslContextGet_InfoCallback(refnum,ssl,v) \
	SslContextGetPtr((refnum),(ssl),sslAttrInfoCallback,(void **)(v))
#define SslContextSet_InfoCallback(refnum,ssl,v) \
	SslContextSetPtr((refnum),(ssl),sslAttrInfoCallback,(void *)(v))
#define SslContextGet_VerifyCallback(refnum,ssl,v) \
	SslContextGetPtr((refnum),(ssl),sslAttrVerifyCallback,(void **)(v))
#define SslContextSet_VerifyCallback(refnum,ssl,v) \
	SslContextSetPtr((refnum),(ssl),sslAttrVerifyCallback,(void *)(v))
#define SslContextGet_Error(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrError)
#define SslContextSet_Error(refnum,ssl,v) \
	SslContextSetLong((refnum),(ssl),sslAttrError,(v))
#define SslContextGet_HsState(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrHsState)
#define SslContextGet_LastAlert(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrLastAlert)
#define SslContextSet_LastAlert(refnum,ssl,v) \
	SslContextSetLong((refnum),(ssl),sslAttrLastAlert,(v))
#define SslContextGet_SessionReused(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrSessionReused)
#define SslContextGet_WriteBufPending(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrWriteBufPending)
#define SslContextGet_ReadBufPending(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrReadBufPending)
#define SslContextGet_ReadRecPending(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrReadRecPending)
#define SslContextGet_ReadOutstanding(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrReadOutstanding)
#define SslContextGet_RbufSize(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrRbufSize)
#define SslContextSet_RbufSize(refnum,ssl,v) \
	SslContextSetLong((refnum),(ssl),sslAttrRbufSize,(v))
#define SslContextGet_WbufSize(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrWbufSize)
#define SslContextSet_WbufSize(refnum,ssl,v) \
	SslContextSetLong((refnum),(ssl),sslAttrWbufSize,(v))
#define SslContextGet_Streaming(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrStreaming)
#define SslContextGet_LastIo(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrLastIo)
#define SslContextGet_LastApi(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrLastApi)
#define SslContextGet_ClientCertRequest(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrClientCertRequest)

#define SslContextGet_DontSendShutdown(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrDontSendShutdown)
#define SslContextSet_DontSendShutdown(refnum,ssl,v) \
	SslContextSetLong((refnum),(ssl),sslAttrDontSendShutdown,(v))
#define SslContextGet_DontWaitForShutdown(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrDontWaitForShutdown)
#define SslContextSet_DontWaitForShutdown(refnum,ssl,v) \
	SslContextSetLong((refnum),(ssl),sslAttrDontWaitForShutdown,(v))

#define SslContextGet_SslSession(refnum,ssl,v) \
	SslContextGetPtr((refnum),(ssl),sslAttrCspSslSession,(void **)(v))
#define SslContextSet_SslSession(refnum,ssl,v) \
	SslContextSetPtr((refnum),(ssl),sslAttrCspSslSession,(void *)(v))
#define SslContextGet_CipherSuites(refnum,ssl,v) \
	SslContextGetPtr((refnum),(ssl),sslAttrCspCipherSuites,(void **)(v))
#define SslContextSet_CipherSuites(refnum,ssl,v) \
	SslContextSetPtr((refnum),(ssl),sslAttrCspCipherSuites,(void *)(v))
#define SslContextGet_CipherSuite(refnum,ssl,v) \
	SslContextGetPtr((refnum),(ssl),sslAttrCspCipherSuite,(void **)(v))
#define SslContextGet_CipherSuiteInfo(refnum,ssl,v) \
	SslContextGetPtr((refnum),(ssl),sslAttrCspCipherSuiteInfo,(void **)(v))
#define SslContextGet_PeerCert(refnum,ssl,v) \
	SslContextGetPtr((refnum),(ssl),sslAttrCertPeerCert,(void **)(v))
#define SslContextGet_SslVerify(refnum,ssl,v) \
	SslContextGetPtr((refnum),(ssl),sslAttrCertSslVerify,(void **)(v))
#define SslContextGet_PeerCommonName(refnum,ssl,v) \
	SslContextGetPtr((refnum),(ssl),sslAttrCertPeerCommonName,(void **)(v))
#define SslContextGet_IoStruct(refnum,ssl,v) \
	SslContextGetPtr((refnum),(ssl),sslAttrIoStruct,(void **)(v))
#define SslContextSet_IoStruct(refnum,ssl,v) \
	SslContextSetPtr((refnum),(ssl),sslAttrIoStruct,(void *)(v))

#define SslContextGet_Socket(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrIoSocket)
#define SslContextSet_Socket(refnum,ssl,v) \
	SslContextSetLong((refnum),(ssl),sslAttrIoSocket,(v))

#define SslContextGet_IoTimeout(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrIoTimeout)
#define SslContextSet_IoTimeout(refnum,ssl,v) \
	SslContextSetLong((refnum),(ssl),sslAttrIoTimeout,(v))
#define SslContextGet_IoFlags(refnum,ssl) \
	SslContextGetLong((refnum),(ssl),sslAttrIoFlags)
#define SslContextSet_IoFlags(refnum,ssl,v) \
	SslContextSetLong((refnum),(ssl),sslAttrIoFlags,(v))

#endif /* __SslLibMac68K_H__ */
