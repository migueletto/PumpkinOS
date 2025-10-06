/*
	HS_HTTPLibApp.h
	Copyright(c) 1996-2002 ACCESS CO., LTD.
	All rights are reserved by ACCESS CO., LTD., whether the whole or
	part of the source code including any modifications.
*/

/**
 * @file 	HS_HTTPLibApp.h
 * @version 2.0
 * @date 	
 * <hr>
 */


#ifndef HS_HTTPLIBAPP_H__
#define HS_HTTPLIBAPP_H__


#define HS_HTTPLibID  		'AsHT'
#define HS_HTTPLibDBType 	'libr'
#define HS_HTTPLibName      "HTTPLibrary"

#define sysNotifyStopEvent 'test'

/* structure for application */

typedef MemHandle HS_HTTPLibHandle;
struct HS_HTTPLibOpaque_ {
	Int32 fDummy;
};
typedef struct HS_HTTPLibOpaque_ *HS_HTTPLibOpaque; 

struct HS_HTTPLibIPAddr_ {
	Int32 type;
	Char addr[16];
};
typedef struct HS_HTTPLibIPAddr_ HS_HTTPLibIPAddr;

#define httpIPAddrTypeV4 4
#define httpIPAddrLenV4 4


/* cookie info */



struct HS_HTTPLibAppinfo_ {
	UInt32 appCreator;
	Int32 maxSockets;
	Int32 isForeground;
	UInt32 recvBufferSize;
	/* cookie DB */
	Char *cookieDBName;
    UInt32 cookieDBCreator;
	UInt32 cookieDBType;
	UInt16 cookieDBVersion;
	Int32 cookieMaxJarSize;
	/* certDB */
	Char *certDBName;
	UInt32 certDBType;
	UInt16 certDBVersion;
};
typedef struct HS_HTTPLibAppinfo_ HS_HTTPLibAppInfo;

typedef Boolean (*HS_HTTPLibNetLibOpenProc)(HS_HTTPLibHandle in_libH, HS_HTTPLibOpaque in_aux);
typedef Boolean (*HS_HTTPLibNetLibCloseProc)(HS_HTTPLibHandle in_libH, HS_HTTPLibOpaque in_aux);
typedef Boolean (*HS_HTTPLibNetLibOnlineProc)(HS_HTTPLibHandle in_libH, HS_HTTPLibOpaque in_aux);
typedef Boolean (*HS_HTTPLibNetLibOfflineProc)(HS_HTTPLibHandle in_libH, HS_HTTPLibOpaque in_aux);
	
struct HS_HTTPLibNetLibInfo_
{
	UInt16 refNum;
	HS_HTTPLibNetLibOpenProc openProc;
	HS_HTTPLibNetLibCloseProc closeProc;
	HS_HTTPLibNetLibOnlineProc onlineProc;
	HS_HTTPLibNetLibOfflineProc offlineProc;
};
typedef struct HS_HTTPLibNetLibInfo_ HS_HTTPLibNetLibInfo;


/* peer functions */
typedef Int32 (*HS_TCPOpenPeer)(Int32 in_domain, HS_HTTPLibOpaque in_aux);
typedef void (*HS_TCPClosePeer)(Int32 in_desc, HS_HTTPLibOpaque in_aux);
typedef Int32 (*HS_TCPIsConnectedPeer)(Int32 in_desc, HS_HTTPLibOpaque in_aux);
typedef Int32 (*HS_TCPConnectPeer)(Int32 in_desc, HS_HTTPLibIPAddr *in_addr, Int32 in_port, HS_HTTPLibOpaque in_aux);
typedef Int32 (*HS_TCPReadPeer)(Int32 in_desc, Char *out_buf, Int32 in_len, HS_HTTPLibOpaque in_aux);
typedef Int32 (*HS_TCPWritePeer)(Int32 in_desc, Char *in_buf, Int32 in_len, HS_HTTPLibOpaque in_aux);
typedef Int32 (*HS_TCPCanReadWritePeer)(Int32 in_desc, Int32 in_rw, HS_HTTPLibOpaque in_aux);

struct HS_HTTPLibPeer_
{
	/* method ptr */
	Int32 (*HS_HTTPLibPeerTCPOpen)(Int32 in_domain, HS_HTTPLibOpaque in_aux);										/* Open method func 		*/
	void (*HS_HTTPLibPeerTCPClose)(Int32 in_desc, HS_HTTPLibOpaque in_aux);											/* Close method func 		*/
	Int32 (*HS_HTTPLibPeerTCPIsConnected)(Int32 in_desc, HS_HTTPLibOpaque in_aux);									/* IsConnected method func	*/
	Int32 (*HS_HTTPLibPeerTCPConnect)(Int32 in_desc, HS_HTTPLibIPAddr *in_addr, Int32 in_port, HS_HTTPLibOpaque in_aux);/* Connect method func 	*/
	Int32 (*HS_HTTPLibPeerTCPRead)(Int32 in_desc, Char *out_buf, Int32 in_len, HS_HTTPLibOpaque in_aux);	/* Read method func 		*/
	Int32 (*HS_HTTPLibPeerTCPWrite)(Int32 in_desc, Char *in_buf, Int32 in_len, HS_HTTPLibOpaque in_aux);	/* Write method func 		*/
	Int32 (*HS_HTTPLibPeerTCPCanReadWrite)(Int32 in_desc, Int32 in_rw, HS_HTTPLibOpaque in_aux);
};
typedef struct HS_HTTPLibPeer_ HS_HTTPLibPeer;

#endif /* HS_HTTPLIBAPP_H_ */

