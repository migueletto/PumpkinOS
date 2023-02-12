/******************************************************************************
 *
 * Copyright (c) 1996-2003 PalmSource, Inc. All rights reserved.
 *
 * File: NetMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This module contains the interface definition for the TCP/IP
 *  library on Pilot.
 *
 *****************************************************************************/

#ifndef 	__NETMGR_H__
#define	__NETMGR_H__

#include <PalmTypes.h>
#include <LibTraps.h>
#include <SysEvent.h>
#include <Event.h>

// Get rid of warnings about unused pragmas when compiling with
//  Visual C
#ifdef _MSC_VER
#pragma warning( disable : 4068)
#endif

/********************************************************************
 * Type and creator of Net Library database
 ********************************************************************/
 
// Creator. Used for both the database that contains the Net Library and
//  it's preferences database.
#define		netCreator				'netl'		// Our Net Library creator

// Feature Creators and numbers, for use with the FtrGet() call. This
//  feature can be obtained to get the current version of the Net Library
#define		netFtrCreator			netCreator
#define		netFtrNumVersion		0				// get version of Net Library
			// 0xMMmfsbbb, where MM is major version, m is minor version
			// f is bug fix, s is stage: 3-release,2-beta,1-alpha,0-development,
			// bbb is build number for non-releases 
			// V1.12b3   would be: 0x01122003
			// V2.00a2   would be: 0x02001002
			// V1.01     would be: 0x01013000

// Begin Change (BGT)
// Feature for defining the number of command blocks to allocate
#define	netFtrCommandBlocks		1				// get the number of command blocks


// Types. Used to identify the Net Library from it's prefs.
#define		netLibType				'libr'		// Our Net Code Resources Database type
#define		netPrefsType			'rsrc'		// Our Net Preferences Database type


// All Network interface's have the following type:
#define		netIFFileType			'neti'		// The filetype of all Network Interfaces

// Each Network interface has a unique creator:
#define		netIFCreatorLoop		'loop'		// Loopback network interface creator.
#define		netIFCreatorSLIP		'slip'		// SLIP network interface creator.
#define		netIFCreatorPPP		'ppp_'		// PPP network interface creator.
//<chg 1-28-98 RM>
#define		netIFCreatorRAM		'ram_'		// Mobitex network interface creator


// Special value for configIndex parameter to NetLibOpenConfig that tells it
// to use the current settings - even if they are not the defined default settings
// This is provided for testing purposes
#define		netConfigIndexCurSettings	0xFFFF


//-----------------------------------------------------------------------------
// Misc. constants
//-----------------------------------------------------------------------------
#define	netDrvrTypeNameLen		8				// Maximum driver type length
#define	netDrvrHWNameLen			16				// Maximum driver hardware name length
#define	netIFNameLen				10				// Maximum interface name (driver type + instance num)
#define	netIFMaxHWAddrLen			14				// Maximum size of a hardware address
#define	netMaxIPAddrStrLen		16				// Max length of an IP address string with null terminator (255.255.255.255)



//-----------------------------------------------------------------------------
// Names of built-in configuration aliases available through the
//  NetLibConfigXXX calls
//-----------------------------------------------------------------------------
#define	netCfgNameDefault			".Default"		// The default configuration
#define	netCfgNameDefWireline	".DefWireline"	// The default wireline configuration
#define	netCfgNameDefWireless	".DefWireless"	// The default wireless configuration
#define	netCfgNameCTPWireline	".CTPWireline"	// Wireline through the Jerry Proxy
#define	netCfgNameCTPWireless	".CTPWireless"	// Wireless through the Jerry Proxy


//-----------------------------------------------------------------------------
//Flags for the NetUWirelessAppHandleEvent() utility routine 
//-----------------------------------------------------------------------------
#define	netWLAppEventFlagCTPOnly		0x00000001	// using wireless radio for CTP protocol only
#define	netWLAppEventFlagDisplayErrs	0x00000002	// Show error alerts for any errors

//-----------------------------------------------------------------------------
// Option constants that can be passed to NetSocketOptionSet and NetSocketOptionGet
// When an option is set or retrieved, both the level of the option and the
// option number must be specified. The level refers to which layer the option
// refers to, like the uppermost socket layer, for example.
//-----------------------------------------------------------------------------

// Socket level options
typedef enum {
	// IP Level options
	netSocketOptIPOptions = 1,						// options in IP header (IP_OPTIONS)
	
	// TCP Level options
	netSocketOptTCPNoDelay = 1,					// don't delay send to coalesce packets
	netSocketOptTCPMaxSeg = 2,						// TCP maximum segment size (TCP_MAXSEG)

	// Socket level options
	netSocketOptSockDebug = 0x0001,				// turn on debugging info recording
	netSocketOptSockAcceptConn = 0x0002,		// socket has had listen
	netSocketOptSockReuseAddr = 0x0004,			// allow local address reuse
	netSocketOptSockKeepAlive = 0x0008,			// keep connections alive
	netSocketOptSockDontRoute = 0x0010,			// just use interface addresses
	netSocketOptSockBroadcast = 0x0020,			// permit sending of broadcast msgs
	netSocketOptSockUseLoopback = 0x0040,		// bypass hardware when possible
	netSocketOptSockLinger = 0x0080,				// linger on close if data present
	netSocketOptSockOOBInLine = 0x0100,			// leave received OutOfBand data in line
	
	netSocketOptSockSndBufSize = 0x1001,		// send buffer size
	netSocketOptSockRcvBufSize = 0x1002,		// receive buffer size
	netSocketOptSockSndLowWater = 0x1003,		// send low-water mark
	netSocketOptSockRcvLowWater = 0x1004,		// receive low-water mark
	netSocketOptSockSndTimeout = 0x1005,		// send timeout
	netSocketOptSockRcvTimeout = 0x1006,		// receive timeout
	netSocketOptSockErrorStatus= 0x1007,		// get error status and clear
	netSocketOptSockSocketType = 0x1008,		// get socket type
	
	// The following are Pilot specific options
	netSocketOptSockNonBlocking = 0x2000,		// set non-blocking mode on or off
	netSocketOptSockRequireErrClear = 0x2001,	// return error from all further calls to socket
															//  unless  netSocketOptSockErrorStatus is cleared.
	netSocketOptSockMultiPktAddr = 0x2002,		// for SOCK_RDM (RMP) sockets. This is the 
															// fixed IP addr (i.e. Mobitex MAN #) to use
															//  for multiple packet requests.
	// for socket notification
	// 05/20/00 jhl
	netSocketOptSockNotice = 0x2003				// prime socket for notification

	} NetSocketOptEnum;


// Option levels for SocketOptionSet and SocketOptionGet
typedef enum {
	netSocketOptLevelIP = 0,					// IP level options (IPPROTO_IP)
	netSocketOptLevelTCP = 6,					// TCP level options (IPPROTO_TCP)
	netSocketOptLevelSocket = 0xFFFF			// Socket level options (SOL_SOCKET)
	} NetSocketOptLevelEnum;


// Structure used for manipulating the linger option
typedef struct {
	Int16		onOff;								// option on/off
	Int16		time;									// linger time in seconds
	} NetSocketLingerType;

//-----------------------------------------------------------------------------
// Enumeration of Socket domains and types passed to NetSocketOpen
//-----------------------------------------------------------------------------
typedef enum {
	netSocketAddrRaw=0,									// (AF_UNSPEC, AF_RAW)
	netSocketAddrINET=2									// (AF_INET)
	} NetSocketAddrEnum;

typedef enum {
	netSocketTypeStream=1,								// (SOCK_STREAM)
	netSocketTypeDatagram=2,							// (SOCK_DGRAM)
	netSocketTypeRaw=3,									// (SOCK_RAW)
	netSocketTypeReliableMsg=4,						// (SOCK_RDM)
	netSocketTypeLicensee=8								// Socket entry reserved for licensees.  
	} NetSocketTypeEnum;

// Protocols, passed in the protocol parameter to NetLibSocketOpen
#define	netSocketProtoIPICMP  1						// IPPROTO_ICMP
#define	netSocketProtoIPTCP   6						// IPPROTO_TCP
#define	netSocketProtoIPUDP   17					// IPPROTO_UDP
#define	netSocketProtoIPRAW   255					// IPPROTO_RAW


//-----------------------------------------------------------------------------
// Enumeration of Socket direction, passed to NetSocketShutdown
//-----------------------------------------------------------------------------
typedef enum {
	netSocketDirInput=0,
	netSocketDirOutput=1,
	netSocketDirBoth=2
	} NetSocketDirEnum;


//-----------------------------------------------------------------------------
// Basic Types
//-----------------------------------------------------------------------------
// Socket refnum
#ifdef PALMOS
typedef	Int16			NetSocketRef;		
#else
typedef	Int32			NetSocketRef;		
#endif

// Type used to hold internet addresses
typedef	UInt32		NetIPAddr;		// a 32-bit IP address.

// IFMediaEvent notifications types
typedef enum {
	netIFMediaUp = 1,	// Usually sent by Network interfaces 
						// after they have displayed the UI for displaying
						// connection establishment progress.
 						
	netIFMediaDown		// Sent by Network interface's when their inactivity timer
						// is ellapsed.
} NetLibIFMediaEventNotificationTypeEnum;


// Notification structure sent in SysNotifyNetLibIFMedia.
typedef struct SysNotifyNetLibIFMediaTag {
	NetLibIFMediaEventNotificationTypeEnum	eType;
	UInt32									ifCreator;	// interface creator
	UInt16									ifInstance;	// interface instance
} SysNotifyNetLibIFMediaType;


//-----------------------------------------------------------------------------
// For socket notification
// 05/20/00 jhl
//-----------------------------------------------------------------------------

// Notice types
typedef enum {
	netSocketNoticeNotify = 1,
// ummmm...
// shouldn't do this - must fix EventMgr before background/ISR events can be posted
	netSocketNoticeEvent,
	netSocketNoticeMailbox,
	netSocketNoticeCallback,
	netSocketNoticeWake
} NoticeTypeEnum;

// Notification structure sent for netSocketNoticeNotify.
typedef struct SysNotifyNetSocketType {
	NetSocketRef		socketRef;		// Socket sending the notification
	UInt32				condition;		// Bit field reporting trigger conditions
} SysNotifyNetSocketType;

// Event structure sent for netSocketNoticeEvent.
// This should be defined via Event.h, so it stays in sync.
typedef struct NetSocketNoticeEventType {
	eventsEnum			eType;			// User specified event type
	Boolean				penDown;
	UInt8					tapCount;
	Int16					screenX;
	Int16					screenY;
	union {
		struct _GenericEventType generic;	// Establish size of union
		struct {
			NetSocketRef	socketRef;	// Socket sending the notification
			UInt32			condition;	// Bit field reporting trigger conditions
		} netSocketNotice;
	} data;
} NetSocketNoticeEventType;

// Mailbox structure sent for netSocketNoticeMailbox.
typedef struct NetSocketNoticeMailboxType {
	UInt32				message;			// User specified message
	UInt16				reserved;
	NetSocketRef		socketRef;		// Socket sending the notification
	UInt32				condition;		// Bit field reporting trigger conditions
} NetSocketNoticeMailboxType;

// Callback definition for netSocketNoticeCallback.
typedef Err (*NetSocketNoticeCallbackPtr)(void *userDataP,UInt16 socketRef,UInt32 condition);

// Structure used to register for a notice
typedef struct NetSocketNoticeType {
	UInt32				condition;		// Bit field specifying trigger conditions
	NoticeTypeEnum		type;				// Notice type
	union {
		struct {
			UInt32		notifyType;		// Notification type
			// sends SysNotifyNetSocketType in notification
		} notify;
// ummmm...
// shouldn't do this - must fix EventMgr before background/ISR events can be posted
		struct {
   		eventsEnum	eType;			// Event type
			// adds NetSocketNoticeEventType event to UI event queue
		} event;
		struct {
			UInt32		mailboxID;		// ID of mailbox for send
			UInt32		message;			// first element of mailbox message
			UInt32		wAck;				// third argument to SysMailboxSend()
			// sends NetSocketNoticeMailboxType message to specified mailboxID
		} mailbox;
		struct {
			NetSocketNoticeCallbackPtr callbackP;	// Callback proc pointer
			void			*userDataP;		// User specified ptr passed as callback parameter
			// (*callbackP)(userDataP,socketRef,condition)
		} callback;
		struct {
			UInt32		taskID;			// ID of task to wake
			NetSocketRef	*socketRefP;	// address to receive socketRef
			UInt32		*conditionP;	// address to receive trigger condition
		} wake;								// SysTaskWake(taskID)
	} notice;
} NetSocketNoticeType;

// Bit values for specifying and reporting trigger conditions
#define netSocketNoticeErr					0x00000001
#define netSocketNoticeUDPReceive		0x00000002
#define netSocketNoticeTCPReceive		0x00000004
#define netSocketNoticeTCPTransmit		0x00000008
#define netSocketNoticeTCPRemoteClosed	0x00000010
#define netSocketNoticeTCPClosed			0x00000020
#define netSocketNoticeConnectInbound	0x00000040
#define netSocketNoticeConnectOutbound	0x00000080


//-----------------------------------------------------------------------------
// Structure used to hold an internet socket address. This includes the internet 
//  address and the port number. This structure directly maps to the BSD unix 
//  struct sockaddr_in.
//-----------------------------------------------------------------------------
typedef struct NetSocketAddrINType {
	Int16			family;					// Address family in HBO (Host UInt8 Order)
	UInt16		port;						// the UDP port in NBO (Network UInt8 Order)
	NetIPAddr	addr;						// IP address in NBO (Network UInt8 Order)
	} NetSocketAddrINType;
	
// Constant that means "use the local machine's IP address"
#define	netIPAddrLocal		0			// Can be used in NetSockAddrINType.addr


// Structure used to hold a generic socket address. This is a generic struct 
// designed to hold any type of address including internet addresses. This 
// structure directly maps to the BSD unix struct sockaddr.
typedef struct  NetSocketAddrType {
	Int16			family;					// Address family
	UInt8			data[14];				// 14 bytes of address 
	} NetSocketAddrType;
	

// Structure used to hold a raw socket address. When using the netSocketAddrRaw
//  protocol family, the caller must bind() the socket to an interface and
//  specifies the interface using this structure. IMPORTANT: NUMEROUS
//  ROUTINES IN NETLIB RELY ON THE FACT THAT THIS STRUCTURE IS THE SAME
//  SIZE AS A NetSocketAddrINType STRUCTURE. 
typedef struct NetSocketAddrRawType {
	Int16			family;					// Address family in HBO (Host UInt8 Order)
	UInt16		ifInstance;				// the interface instance number 
	UInt32		ifCreator;				// the interface creator
	} NetSocketAddrRawType;
	
	
	
//-----------------------------------------------------------------------------
// Structure used to hold information about data to be sent. This structure
//  is passed to NetLibSendMsg and contains the optional address to send to,
//  a scatter-write array of data to be sent, and optional access rights
//-----------------------------------------------------------------------------

// Scatter/Gather array type. A pointer to an array of these structs is
//  passed to the NetLibSendPB and NetLibRecvPB calls. It specifies where
//  data should go to or come from as a list of buffer addresses and sizes.
typedef struct NetIOVecType {
	UInt8 *			bufP;							// buffer address
	UInt16			bufLen;						// buffer length
	} NetIOVecType, *NetIOVecPtr;
	
#define	netIOVecMaxLen			16				// max# of NetIOVecTypes in an array

// Read/Write ParamBlock type. Passed directly to the SendPB and RecvPB calls.
typedef struct {
	UInt8 *			addrP;						// address - or 0 for default
	UInt16			addrLen;						// length of address
	NetIOVecPtr		iov;							// scatter/gather array
	UInt16			iovLen;						// length of above array
	UInt8 *			accessRights;				// access rights
	UInt16			accessRightsLen;			// length of accessrights
	} NetIOParamType, *NetIOParamPtr;
	
// Flags values for the NetLibSend, NetLibReceive calls
#define	netIOFlagOutOfBand		0x01		// process out-of-band data
#define	netIOFlagPeek				0x02		// peek at incoming message
#define	netIOFlagDontRoute		0x04		// send without using routing


						
//-----------------------------------------------------------------------------
// Structures used for looking up a host by name or address (NetLibGetHostByName)
//-----------------------------------------------------------------------------

// Equates for DNS names, from RFC-1035
#define	netDNSMaxDomainName		255
#define	netDNSMaxDomainLabel		63

#define	netDNSMaxAliases			1				// max # of aliases for a host
#define	netDNSMaxAddresses		4				// max # of addresses for a host


// The actual results of NetLibGetHostByName() are returned in this structure.
// This structure is designed to match the "struct hostent" structure in Unix.
typedef struct  {
	Char *		nameP;								// official name of host
	Char **		nameAliasesP;						// array of alias's for the name
	UInt16		addrType;							// address type of return addresses
	UInt16		addrLen;								// the length, in bytes, of the addresse
															//Note this denotes length of a address, not # of addresses.
	UInt8 **		addrListP;							// array of ptrs to addresses in HBO
	} NetHostInfoType, *NetHostInfoPtr;
	

// "Buffer" passed to call as a place to store the results
typedef struct {
	NetHostInfoType	hostInfo;					// high level results of call are here

	// The following fields contain the variable length data that 
	//  hostInfo points to
	Char	name[netDNSMaxDomainName+1];			// hostInfo->name

	Char *aliasList[netDNSMaxAliases+1];		// +1 for 0 termination.
	Char	aliases[netDNSMaxAliases][netDNSMaxDomainName+1];

	NetIPAddr*	addressList[netDNSMaxAddresses];
	NetIPAddr	address[netDNSMaxAddresses];

	} NetHostInfoBufType, *NetHostInfoBufPtr;
	
	
//-----------------------------------------------------------------------------
// Structures used for looking up a service (NetLibGetServByName)
//-----------------------------------------------------------------------------

// Equates for service names
#define	netServMaxName				15				// max # characters in service name
#define	netProtoMaxName			15				// max # characters in protocol name
#define	netServMaxAliases			1				// max # of aliases for a service


// The actual results of NetLibGetServByName() are returned in this structure.
// This structure is designed to match the "struct servent" structure in Unix.
typedef struct {
	Char *		nameP;								// official name of service
	Char **		nameAliasesP;						// array of alias's for the name
	UInt16		port;									// port number for this service
	Char *		protoP;								// name of protocol to use
	} NetServInfoType, *NetServInfoPtr;
	
// "Buffer" passed to call as a place to store the results
typedef struct {
	NetServInfoType	servInfo;					// high level results of call are here

	// The following fields contain the variable length data that 
	//  servInfo points to
	Char			name[netServMaxName+1];					// hostInfo->name

	Char *		aliasList[netServMaxAliases+1];		// +1 for 0 termination.
	Char			aliases[netServMaxAliases][netServMaxName];
	Char			protoName[netProtoMaxName+1];

	UInt8			reserved;
	} NetServInfoBufType, *NetServInfoBufPtr;
	


//--------------------------------------------------------------------
// Structure of a configuration name. Used by NetLibConfigXXX calls
// <chg 1-28-98 RM> added for the new Config calls. 
//---------------------------------------------------------------------
#define	netConfigNameSize		32
typedef struct {
	Char			name[netConfigNameSize];			  // name of configuration
	} NetConfigNameType, *NetConfigNamePtr;



/********************************************************************
 * Tracing Flags. These flags are ORed together and passed as a UInt32
 *  in the netSettingTraceFlags setting and netIFSettingTraceFlags to
 *  enable/disable various trace options.
 ********************************************************************/
#define		netTracingErrors		0x00000001			// record errors
#define		netTracingMsgs			0x00000002			// record messages
#define		netTracingPktIP		0x00000004			// record packets sent/received
																	//  to/from interfaces at the IP layer
																	// NOTE:  netTracingPktData40 & netTracingPktData
																	//  will control how much data of each packet is
																	//  recorded. 
#define		netTracingFuncs		0x00000008			// record function flow
#define		netTracingAppMsgs		0x00000010			// record application messages
																	// (NetLibTracePrintF, NetLibTracePutS)
#define		netTracingPktData40	0x00000020			// record first 40 bytes of packets 
																	//  when netTracingPktsXX is also on. 
																	// NOTE: Mutually exclusive with 
																	//  netTracingPktData and only applicable if
																	//  one of the netTracingPktsXX bits is also set
#define		netTracingPktData		0x00000040			// record all bytes of IP packets 
																	//  sent/received to/from interfaces
																	// NOTE: Mutually exclusive with 
																	//  netTracingPkts & netTracingPktData64
#define		netTracingPktIFHi		0x00000080			// record packets sent/received at highest layer
																	//  of interface (just below IP layer). 
																	// NOTE:  netTracingPktData40 & netTracingPktData
																	//  will control how much data of each packet is
																	//  recorded. 
#define		netTracingPktIFMid	0x00000100			// record packets sent/received at mid layer
																	//  of interface (just below IFHi layer). 
																	// NOTE:  netTracingPktData40 & netTracingPktData
																	//  will control how much data of each packet is
																	//  recorded. 
#define		netTracingPktIFLow	0x00000200			// record packets sent/received at low layer
																	//  of interface (just below IFMid layer). 
																	// NOTE:  netTracingPktData40 & netTracingPktData
																	//  will control how much data of each packet is
																	//  recorded. 


// OBSOLETE tracing bit, still used by Network Panel
#define		netTracingPkts			netTracingPktIP


/********************************************************************
 * Command numbers and parameter blocks for the NetLibMaster() call.
 * This call is used to put the Net library into certain debugging modes
 *		or for obtaining statistics from the Net Library.
 * 
 ********************************************************************/
#pragma mark Master
typedef enum {
	// These calls return info
	netMasterInterfaceInfo,
	netMasterInterfaceStats,
	netMasterIPStats,
	netMasterICMPStats,
	netMasterUDPStats,
	netMasterTCPStats,
	
	// This call used to read the trace buffer.
	netMasterTraceEventGet					// get trace event by index

	} NetMasterEnum;
	
	
typedef struct NetMasterPBType {

	// These fields are specific to each command
	union {
	
		//.............................................................
		// InterfaceInfo command
		//.............................................................
		struct  {
			UInt16		index;					// -> index of interface
			UInt32		creator;					// <- creator
			UInt16		instance;				// <- instance
			void *		netIFP;					// <- net_if pointer
			
			// driver level info
			Char			drvrName[netDrvrTypeNameLen];		// <- type of driver (SLIP,PPP, etc)
			Char			hwName[netDrvrHWNameLen];			// <- hardware name (Serial Library, etc)
			UInt8			localNetHdrLen;		// <- local net header length
			UInt8			localNetTrailerLen;	// <- local net trailer length
			UInt16		localNetMaxFrame;		// <- local net maximum frame size
			
			// media layer info
			Char			ifName[netIFNameLen];// <- interface name w/instance
			Boolean		driverUp;				// <- true if interface driver up
			Boolean		ifUp;						// <- true if interface is up
			UInt16		hwAddrLen;				// <- length of hardware address
			UInt8			hwAddr[netIFMaxHWAddrLen];		// <- hardware address
			UInt16		mtu;						// <- maximum transfer unit of interface
			UInt32		speed;					// <- speed in bits/sec.
			UInt32		lastStateChange;		// <- time in milliseconds of last state change
			
			// Address info
			NetIPAddr	ipAddr;					// Address of this interface
			NetIPAddr	subnetMask;				// subnet mask of local network
			NetIPAddr	broadcast;				// broadcast address of local network
			} interfaceInfo;
			
		//.............................................................
		// InterfaceStats command
		//.............................................................
		struct  {
			UInt16		index;					// -> index of interface
			UInt32		inOctets;				// <- ....
			UInt32		inUcastPkts;
			UInt32		inNUcastPkts;
			UInt32		inDiscards;
			UInt32		inErrors;
			UInt32		inUnknownProtos;
			UInt32		outOctets;
			UInt32		outUcastPkts;
			UInt32		outNUcastPkts;
			UInt32		outDiscards;
			UInt32		outErrors;
			} interfaceStats;
						
		//.............................................................
		// IPStats command
		//.............................................................
		struct  {
			UInt32		ipInReceives;	
			UInt32		ipInHdrErrors;
			UInt32		ipInAddrErrors;
			UInt32		ipForwDatagrams;
			UInt32		ipInUnknownProtos;
			UInt32		ipInDiscards;
			UInt32		ipInDelivers;
			UInt32		ipOutRequests;
			UInt32		ipOutDiscards;
			UInt32 		ipOutNoRoutes;
			UInt32 		ipReasmReqds;
			UInt32 		ipReasmOKs;
			UInt32 		ipReasmFails;
			UInt32		ipFragOKs;
			UInt32 		ipFragFails;
			UInt32 		ipFragCreates;
			UInt32 		ipRoutingDiscards;
			UInt32 		ipDefaultTTL;        
			UInt32 		ipReasmTimeout;      
			} ipStats;
						
		//.............................................................
		// ICMPStats command
		//.............................................................
		struct  {
			UInt32			icmpInMsgs;
			UInt32			icmpInErrors;
			UInt32			icmpInDestUnreachs;
			UInt32			icmpInTimeExcds;
			UInt32			icmpInParmProbs;
			UInt32			icmpInSrcQuenchs;
			UInt32			icmpInRedirects;
			UInt32			icmpInEchos;
			UInt32			icmpInEchoReps;
			UInt32			icmpInTimestamps;
			UInt32			icmpInTimestampReps;
			UInt32			icmpInAddrMasks;
			UInt32			icmpInAddrMaskReps;
			UInt32			icmpOutMsgs;
			UInt32			icmpOutErrors;
			UInt32			icmpOutDestUnreachs;
			UInt32			icmpOutTimeExcds;
			UInt32			icmpOutParmProbs;
			UInt32			icmpOutSrcQuenchs;
			UInt32			icmpOutRedirects;
			UInt32			icmpOutEchos;
			UInt32			icmpOutEchoReps;
			UInt32			icmpOutTimestamps;
			UInt32			icmpOutTimestampReps;
			UInt32			icmpOutAddrMasks;
			UInt32			icmpOutAddrMaskReps;
			} icmpStats;
						
		//.............................................................
		// UDPStats command
		//.............................................................
		struct  {
			UInt32			udpInDatagrams;
			UInt32			udpNoPorts;
			UInt32			udpInErrors;
			UInt32			udpOutDatagrams;
			} udpStats;
						
		//.............................................................
		// TCPStats command
		//.............................................................
		struct  {
			UInt32			tcpRtoAlgorithm;
			UInt32			tcpRtoMin;
			UInt32			tcpRtoMax;
			UInt32			tcpMaxConn;
			UInt32			tcpActiveOpens;
			UInt32			tcpPassiveOpens;
			UInt32			tcpAttemptFails;
			UInt32			tcpEstabResets;
			UInt32			tcpCurrEstab;
			UInt32			tcpInSegs;
			UInt32			tcpOutSegs;
			UInt32			tcpRetransSegs;
			UInt32			tcpInErrs;
			UInt32			tcpOutRsts;
			} tcpStats;
						
		//.............................................................
		// TraceEventGet command
		//.............................................................
		struct  {
			UInt16		index;				// which event
			Char *		textP;				// ptr to text string to return it in
			} traceEventGet;
			
		} param;

	} NetMasterPBType, *NetMasterPBPtr;
	
	

	
	
//-----------------------------------------------------------------------------
// Enumeration of Net settings as passed to NetLibSettingGet/Set. 
//-----------------------------------------------------------------------------
#pragma mark Settings
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Global environment settings common to all attached network interfaces,
//   passed to NetLibSettingGet/Set
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
typedef enum {	
	netSettingResetAll,						// void, NetLibSettingSet only, resets all settings
													//  to their defaults.

	netSettingPrimaryDNS,					// UInt32, IP address of Primary DN Server
	netSettingSecondaryDNS,					// UInt32, IP address of Secondary DN Server
	netSettingDefaultRouter,				// UInt32, IP address of Default router
	netSettingDefaultIFCreator,			// UInt32, Creator type of default interface
	netSettingDefaultIFInstance,			// UInt16, Instance# of default interface
	netSettingHostName,						// Char[64], name of host (not including domain)
	netSettingDomainName,					// Char[256], domain name of hosts's domain
	netSettingHostTbl,						// Char[], host table 
	netSettingCloseWaitTime,				// UInt32, time in milliseconds to stay in close-wait state
	netSettingInitialTCPResendTime,		// UInt32, time in milliseconds before TCP resends a packet.
													//  This is just the initial value, the timeout is adjusted
													//  from this initial value depending on history of ACK times.
													//  This is sometimes referred to as the RTO (Roundtrip Time Out)
													//  See RFC-1122 for additional information.
	
	
	// The following settings are not used for configuration, but rather put the
	//  stack into various modes for debugging, etc. 
	netSettingTraceBits = 0x1000,			// UInt32, enable/disable various trace flags (netTraceBitXXXX)
	netSettingTraceSize,						// UInt32, max trace buffer size in bytes. Default 0x800.
													//  Setting this will also clear the trace buffer.
	netSettingTraceStart,					// UInt32, for internal use ONLY!!
	netSettingTraceRoll,						// UInt8, if true, trace buffer will rollover after it fills.
													//  Default is true.
													
	netSettingRTPrimaryDNS,					// used internally by Network interfaces
													//  that dynamically obtain the DNS address
	netSettingRTSecondaryDNS,				// used internally by Network interfaces
													//  that dynamically obtain the DNS address
													
	netSettingConfigTable					// used internally by NetLib - NOT FOR USE BY
													//  APPLICATIONS!!

	} NetSettingEnum;


//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Settings for each Network Interface, passed to NetLibIFSettingGet/Set
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
typedef enum {
	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// Reset all settings to defaults
	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	netIFSettingResetAll,					// void, NetLibIFSettingSet only, resets all settings
													//  to their defaults.
													
	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// Status - read only
	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	netIFSettingUp,							// UInt8, true if interface is UP.
	netIFSettingName,							// Char[32], name of interface

	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// Addressing
	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	netIFSettingReqIPAddr,					// UInt32, requested IP address of this interface
	netIFSettingSubnetMask,					// UInt32, subnet mask of this interface
	netIFSettingBroadcast,					// UInt32, broadcast address for this interface

	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// User Info
	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	netIFSettingUsername,					// Char[], login script user name
													//				 If 0 length, then user will be prompted for it
	netIFSettingPassword,					// Char[], login script user password
													//				 If 0 length, then user will be prompted for it
	netIFSettingDialbackUsername,			// Char[], login script dialback user name.  
													//				 If 0 length, then netIFSettingUsername is used
	netIFSettingDialbackPassword,			// Char[], login script dialback user password. 
													//				 If 0 length, then user will be prompted for it
	netIFSettingAuthUsername,				// Char[], PAP/CHAP name. 
													//				 If 0 length, then netIFSettingUsername is used
	netIFSettingAuthPassword,				// Char[], PAP/CHAP password. 
													//				 If "$", then user will be prompted for it
													//				 else If 0 length, then netIFSettingPassword or result
													//					of it's prompt (if it was empty) will be used
													//				 else it is used as-is.
	
	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// Connect Settings
	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	netIFSettingServiceName,				// Char[], name of service
	netIFSettingLoginScript,				// Char[], login script
	netIFSettingConnectLog,					// Char[], connect log 
	netIFSettingInactivityTimeout,		// UInt16, # of seconds of inactivity allowed before
													//  interface is brought down. If 0 then
													//  no inactivity timeout enforced.
	netIFSettingEstablishmentTimeout,	// UInt16, max delay in seconds between connection 
													//  establishment stages
	
	// Serial based protocol options
	netIFSettingDynamicIP,					// UInt8, if true, get IP address from server
													//  N/A for SLIP
	netIFSettingVJCompEnable,				// UInt8, if true enable VJ Header compression
													//  Default is on for PPP, off for SLIP
	netIFSettingVJCompSlots,				// UInt8, # of slots to use for VJ compression.
													//  Default is 4 for PPP, 16 for SLIP
													//  (each slot uses 256 bytes of RAM).
	netIFSettingMTU,							// UInt16, maximum transmission unit in bytes
													//  ignored in current PPP and SLIP interfaces
	netIFSettingAsyncCtlMap,				// UInt32, bitmask of characters to escape
													//  ignored in current PPP interfaces

	// Serial settings, used by serial based network interfaces
	netIFSettingPortNum,						// UInt16, port number to use
	netIFSettingBaudRate,					// UInt32, baud rate in bits/sec.
	netIFSettingFlowControl,				// UInt8, flow control setting bits. Set to 0x01 for
													//   hardware flow control, else set to 0x00.
	netIFSettingStopBits,					// UInt8, # of stop bits
	netIFSettingParityOn,					// UInt8, true if parity on
	netIFSettingParityEven,					// UInt8, true if parity even

	// Modem settings, optionally used by serial based network interfaces
	netIFSettingUseModem,					// UInt8, if true dial-up through modem
	netIFSettingPulseDial,					// UInt8, if true use pulse dial, else tone
	netIFSettingModemInit,					// Char[], modem initialization string
	netIFSettingModemPhone,					// Char[], modem phone number string
	netIFSettingRedialCount,				// UInt16, # of times to redial
	

	//---------------------------------------------------------------------------------
	// New Settings as of PalmOS 3.0
	// Power control, usually only implemented by wireless interfaces 
	//---------------------------------------------------------------------------------
	netIFSettingPowerUp,						// UInt8, true if this interface is powered up
													//       false if this interface is in power-down mode
													//  interfaces that don't support power modes should
													//  quietly ignore this setting. 
												
	// Wireless or Wireline, read-only, returns true for wireless interfaces. this
	//  setting is used by application level functions to determine which interface(s)
	//  to attach/detach given user preference and/or state of the antenna.
	netIFSettingWireless,					// UInt8, true if this interface is wireless
	


	// Option to query server for address of DNS servers
	netIFSettingDNSQuery,					// UInt8, if true PPP queries for DNS address. Default true


	//---------------------------------------------------------------------------------
	// New Settings as of PalmOS 3.2
	// Power control, usually only implemented by wireless interfaces 
	//---------------------------------------------------------------------------------
												
	netIFSettingQuitOnTxFail,				// BYTE  W-only. Power down RF on tx fail
	netIFSettingQueueSize,					// UInt8  R-only. The size of the Tx queue in the RF interface
	netIFSettingTxInQueue,					// BYTE  R-only. Packets remaining to be sent
	netIFSettingTxSent,						// BYTE  R-only. Packets sent since SocketOpen
	netIFSettingTxDiscard,					// BYTE  R-only. Packets discarded on SocketClose
	netIFSettingRssi,							// char	 R-only. signed value in dBm.
	netIFSettingRssiAsPercent,				// char	 R-only. signed value in percent, with 0 being no coverage and 100 being excellent.
	netIFSettingRadioState,					// enum	 R-only. current state of the radio
	netIFSettingBase,							// UInt32 R-only. Interface specific
	netIFSettingRadioID,						// UInt32[2] R-only, two 32-bit. interface specific
	netIFSettingBattery,						// UInt8, R-only. percentage of battery left
	netIFSettingNetworkLoad,				// UInt8, R-only. percent estimate of network loading

	//---------------------------------------------------------------------------------
	// New Settings as of PalmOS 3.3
	//---------------------------------------------------------------------------------

	netIFSettingConnectionName,			// Char [] Connection Profile Name


	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// The following settings are not used for configuration, but rather put the
	//  stack into various modes for debugging, etc. 
	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	netIFSettingTraceBits = 0x1000,		// UInt32, enable/disable various trace flags (netTraceBitXXXX)
	netIFSettingGlobalsPtr,					// UInt32, (Read-Only) sinterface's globals ptr
	netIFSettingActualIPAddr,				// UInt32, (Read-Only) the actual IP address that the interface
													//   ends up using. The login script executor stores
													//   the result of the "g" script command here as does
													//   the PPP negotiations. 
	netIFSettingServerIPAddr,				// UInt32, (Read-Only) the IP address of the PPP server
													//  we're connected to 
	
	
	// The following setting should be true if this network interface should be
	// brought down when the Pilot is turned off.
	netIFSettingBringDownOnPowerDown,	// UInt8, if true interface will be brought down when
													//  Pilot is turned off.
													
	// The following setting is used by the TCP/IP stack ONLY!! It tells the interface
	//  to pass all received packets as-is to the NetIFCallbacksPtr->raw_rcv() routine. 
	//  This setting gets setup when an application creates a raw socket in the raw domain
	netIFSettingRawMode,						// UInt32, parameter to pass to raw_rcv() along with
													//  packet pointer. 

	//---------------------------------------------------------------------------------
	// New Settings as of PalmOS 4.0
	//---------------------------------------------------------------------------------

	// The following setting is a new interface in PalmOS 4.0 that allow INetlib
	// or other NetLib clients to get raw location information as described in
	// PalmLocRawData.h.
	// NetLib will return a pointer to a newly allocated memory buffer containing
	// the raw location information to send to Elaine (Web Clipping proxy server).
	// Elaine will then use a Windows DLL to analyse the raw location information
	// in order to transform it into something useful like zipcode, cityname, etc.
	// See PalmLocRawData.h for more details...
	netIFSettingLocRawInfo,					// void* R-only: Allocated memory buffer - must be free by caller

	//---------------------------------------------------------------------------------
	// New Settings as of PalmOS 5.1
	//---------------------------------------------------------------------------------
    
    // The following settting provide an interface between applications and an 802.11
    // interface.
     
    netIFSettingDriverVersion,				// Char[20] R-only: Version of the interface driver
    
    netIFSettingFirmwareVersion,			// Char[20] R-only: Version of the radio firmware
    
    netIFSettingFirmwareDate,				// UInt32 R-only: Version date of the firmware
    
    netIFSetting80211Device,				// UInt8 R-only: True if this is an 802.11 device
    
    netIFSetting80211ESSID,					// Char[32] R-only: The radio's ESS ID
    
    netIFSetting80211AccessPointBSSID,		// UInt8[6] R-only: The BSSID of the access point 
    										// that the radio is currently connected to.
    
    netIFSetting80211AssociationStatus,		// UInt8 R-only: True if the radio is associated with an
    										// access point.
    
    netIFSetting80211MKKCallSign,			// Char[15] R-only: MKK Call sign
    
    netIFSetting80211CountryText,     		// Char[34] R-only: Text description of country       

	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// 3rd party settings start here...
	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	netIFSettingCustom = 0x8000
	
	} NetIFSettingEnum;



//=========================================================================================
// Enums for the netIFSettingRadioState setting
//
// JB added for the radio state setting.
// <chg 3-17-98 RM> fixed naming conventions. 
//=========================================================================================
typedef enum	{
	netRadioStateOffNotConnected=0,
	netRadioStateOnNotConnected,			// scanning
	netRadioStateOnConnected,				// have channel
	netRadioStateOffConnected
	} NetRadioStateEnum; 



/************************************************************
 * Net Library Macros
 *************************************************************/
// Return current time in milliseconds.
#define NetNow()     (TimGetTicks() * 1000L/sysTicksPerSecond) 


// File Descriptor macros used for the NetLibSelect() call
typedef UInt32					NetFDSetType;
#define	netFDSetSize		32

#define	netFDSet(n,p)		((*p) |= (1L << n))
#define	netFDClr(n,p)		((*p) &= ~(1L << n))
#define	netFDIsSet(n,p)	((*p) & (1L << n))
#define	netFDZero(p)		((*p) = 0)



//-----------------------------------------------------------------------------
// Net error codes
//-----------------------------------------------------------------------------
#pragma mark ErrorCodes
#define	netErrAlreadyOpen					(netErrorClass | 1)
#define	netErrNotOpen						(netErrorClass | 2)
#define	netErrStillOpen					(netErrorClass | 3)
#define	netErrParamErr						(netErrorClass | 4)
#define	netErrNoMoreSockets				(netErrorClass | 5)
#define	netErrOutOfResources				(netErrorClass | 6)
#define	netErrOutOfMemory					(netErrorClass | 7)		// Might be because free heap space is <32K probably because handheld's RAM <2MB
#define	netErrSocketNotOpen				(netErrorClass | 8)
#define	netErrSocketBusy					(netErrorClass | 9)		//EINPROGRESS
#define	netErrMessageTooBig				(netErrorClass | 10)
#define	netErrSocketNotConnected 		(netErrorClass | 11)
#define	netErrNoInterfaces				(netErrorClass | 12)		//ENETUNREACH
#define	netErrBufTooSmall					(netErrorClass | 13)
#define	netErrUnimplemented				(netErrorClass | 14)
#define	netErrPortInUse					(netErrorClass | 15)		//EADDRINUSE
#define	netErrQuietTimeNotElapsed		(netErrorClass | 16)		//EADDRINUSE
#define	netErrInternal						(netErrorClass | 17)
#define	netErrTimeout						(netErrorClass | 18)		//ETIMEDOUT
#define	netErrSocketAlreadyConnected 	(netErrorClass | 19)		//EISCONN
#define	netErrSocketClosedByRemote 	(netErrorClass | 20)
#define	netErrOutOfCmdBlocks			 	(netErrorClass | 21)
#define	netErrWrongSocketType			(netErrorClass | 22)
#define	netErrSocketNotListening		(netErrorClass | 23)
#define	netErrUnknownSetting				(netErrorClass | 24)
#define	netErrInvalidSettingSize		(netErrorClass | 25)
#define	netErrPrefNotFound				(netErrorClass | 26)
#define	netErrInvalidInterface			(netErrorClass | 27)
#define	netErrInterfaceNotFound			(netErrorClass | 28)
#define	netErrTooManyInterfaces			(netErrorClass | 29)
#define	netErrBufWrongSize				(netErrorClass | 30)
#define	netErrUserCancel					(netErrorClass | 31)
#define	netErrBadScript					(netErrorClass | 32)
#define	netErrNoSocket						(netErrorClass | 33)
#define	netErrSocketRcvBufFull			(netErrorClass | 34)
#define	netErrNoPendingConnect			(netErrorClass | 35)
#define	netErrUnexpectedCmd				(netErrorClass | 36)
#define	netErrNoTCB							(netErrorClass | 37)
#define	netErrNilRemoteWindowSize		(netErrorClass | 38)
#define	netErrNoTimerProc					(netErrorClass | 39)
#define	netErrSocketInputShutdown		(netErrorClass | 40)		// EOF to sockets API
#define	netErrCmdBlockNotCheckedOut	(netErrorClass | 41)		 
#define	netErrCmdNotDone					(netErrorClass | 42)		 
#define	netErrUnknownProtocol			(netErrorClass | 43)		 
#define	netErrUnknownService				(netErrorClass | 44)		 
#define	netErrUnreachableDest			(netErrorClass | 45)	
#define	netErrReadOnlySetting			(netErrorClass | 46)	 
#define	netErrWouldBlock					(netErrorClass | 47)		//EWOULDBLOCK
#define	netErrAlreadyInProgress			(netErrorClass | 48)		//EALREADY
#define	netErrPPPTimeout					(netErrorClass | 49)
#define	netErrPPPBroughtDown				(netErrorClass | 50)
#define	netErrAuthFailure					(netErrorClass | 51)
#define	netErrPPPAddressRefused			(netErrorClass | 52)
// The following map into the Epilogue DNS errors declared in DNS.ep.h:
//  and MUST be kept in this order!!
#define	netErrDNSNameTooLong				(netErrorClass | 53)
#define	netErrDNSBadName					(netErrorClass | 54)
#define	netErrDNSBadArgs					(netErrorClass | 55)
#define	netErrDNSLabelTooLong			(netErrorClass | 56)
#define	netErrDNSAllocationFailure		(netErrorClass | 57)
#define	netErrDNSTimeout					(netErrorClass | 58)
#define	netErrDNSUnreachable				(netErrorClass | 59)
#define	netErrDNSFormat					(netErrorClass | 60)
#define	netErrDNSServerFailure			(netErrorClass | 61)
#define	netErrDNSNonexistantName		(netErrorClass | 62)
#define	netErrDNSNIY						(netErrorClass | 63)
#define	netErrDNSRefused					(netErrorClass | 64)
#define	netErrDNSImpossible				(netErrorClass | 65)
#define	netErrDNSNoRRS						(netErrorClass | 66)
#define	netErrDNSAborted					(netErrorClass | 67)
#define	netErrDNSBadProtocol				(netErrorClass | 68)
#define	netErrDNSTruncated				(netErrorClass | 69)
#define	netErrDNSNoRecursion				(netErrorClass | 70)
#define	netErrDNSIrrelevant				(netErrorClass | 71)
#define	netErrDNSNotInLocalCache		(netErrorClass | 72)
#define	netErrDNSNoPort					(netErrorClass | 73)
// The following map into the Epilogue IP errors declared in IP.ep.h:
//  and MUST be kept in this order!!
#define	netErrIPCantFragment				(netErrorClass | 74)
#define	netErrIPNoRoute					(netErrorClass | 75)
#define	netErrIPNoSrc						(netErrorClass | 76)
#define	netErrIPNoDst						(netErrorClass | 77)
#define	netErrIPktOverflow				(netErrorClass | 78)
// End of Epilogue IP errors
#define	netErrTooManyTCPConnections	(netErrorClass | 79)
#define  netErrNoDNSServers				(netErrorClass | 80)
#define	netErrInterfaceDown				(netErrorClass | 81)

// Mobitex network radio interface error code returns
#define	netErrNoChannel					(netErrorClass | 82)	// The datalink layer cannot acquire a channel 
#define	netErrDieState						(netErrorClass | 83)	// Mobitex network has issued a DIE command.
#define	netErrReturnedInMail				(netErrorClass | 84) // The addressed of the transmitted packet was not available, and the message was placed in the network's mailbox.
#define	netErrReturnedNoTransfer		(netErrorClass | 85)	// This message cannot be transferred or put in the network mailbox.
#define	netErrReturnedIllegal			(netErrorClass | 86)	// The message could not be switched to the network
#define	netErrReturnedCongest			(netErrorClass | 87)	// Line, radio channels, or network nodes are congested.
#define	netErrReturnedError				(netErrorClass | 88)	// Technical error in the network.
#define	netErrReturnedBusy				(netErrorClass | 89)	// The B-party is busy.
#define	netErrGMANState					(netErrorClass | 90)	// The modem has not registered with the network.
#define	netErrQuitOnTxFail				(netErrorClass | 91) // Couldn't get packet through, shutdown.
#define	netErrFlexListFull				(netErrorClass | 92) // raw IF error message: see Mobitex spec.
#define	netErrSenderMAN					(netErrorClass | 93) // ditto
#define	netErrIllegalType					(netErrorClass | 94) // ditto
#define	netErrIllegalState				(netErrorClass | 95) // ditto
#define	netErrIllegalFlags				(netErrorClass | 96) // ditto
#define	netErrIllegalSendlist			(netErrorClass | 97)	// ditto
#define	netErrIllegalMPAKLength			(netErrorClass | 98)	// ditto
#define	netErrIllegalAddressee			(netErrorClass | 99)	// ditto
#define	netErrIllegalPacketClass		(netErrorClass | 100) // ditto
#define	netErrBufferLength				(netErrorClass | 101) // any 
#define	netErrNiCdLowBattery				(netErrorClass | 102) // any 
#define	netErrRFinterfaceFatal			(netErrorClass | 103) // any
#define	netErrIllegalLogout				(netErrorClass | 104) // raw IF error message
#define	netErrAAARadioLoad				(netErrorClass | 105)	// 7/20/98 JB.  If there is insufficient AAA
#define	netErrAntennaDown					(netErrorClass | 106)
#define	netErrNiCdCharging				(netErrorClass | 107)	// just for charging
#define	netErrAntennaWentDown			(netErrorClass | 108)
#define	netErrNotActivated				(netErrorClass | 109)	// The unit has not been FULLY activated.  George and Morty completed.
#define	netErrRadioTemp					(netErrorClass | 110)	// Radio's temp is too high for FCC compliant TX
#define	netErrNiCdChargeError			(netErrorClass | 111)	// Charging stopped due to NiCd charging characteristic
#define	netErrNiCdSag						(netErrorClass | 112)	// the computed sag or actual sag indicates a NiCd with diminished capacity.
#define	netErrNiCdChargeSuspend			(netErrorClass | 113)	// Charging has been suspended due to low AAA batteries.
// Left room for more Mobitex errors

// Configuration errors
#define	netErrConfigNotFound				(netErrorClass | 115)
#define	netErrConfigCantDelete			(netErrorClass | 116)
#define	netErrConfigTooMany				(netErrorClass | 117)
#define	netErrConfigBadName				(netErrorClass | 118)
#define	netErrConfigNotAlias				(netErrorClass | 119)
#define	netErrConfigCantPointToAlias	(netErrorClass | 120)
#define	netErrConfigEmpty					(netErrorClass | 121)
#define	netErrAlreadyOpenWithOtherConfig		(netErrorClass | 122)
#define	netErrConfigAliasErr				(netErrorClass | 123)
#define	netErrNoMultiPktAddr				(netErrorClass | 124)
#define	netErrOutOfPackets				(netErrorClass | 125) 
#define	netErrMultiPktAddrReset			(netErrorClass | 126)
#define	netErrStaleMultiPktAddr			(netErrorClass | 127)

// Login scripting plugin errors
#define	netErrScptPluginMissing			(netErrorClass | 128)
#define	netErrScptPluginLaunchFail		(netErrorClass | 129)
#define	netErrScptPluginCmdFail			(netErrorClass | 130)
#define	netErrScptPluginInvalidCmd		(netErrorClass | 131)

// Telephony errors
#define	netErrTelMissingComponent		(netErrorClass | 132)
#define	netErrTelErrorNotHandled		(netErrorClass | 133)

#define netErrDeviceNotReady			(netErrorClass | 134)
#define netErrDeviceInitFail			(netErrorClass | 135)
#define netErrInterfaceIncompatible     (netErrorClass | 136)

#define	netErrMobitexStart				netErrNoChannel
#define	netErrMobitexEnd					netErrNiCdChargeSuspend         

//-----------------------------------------------------------------------------
// Net library call ID's. Each library call gets the trap number:
//   netTrapXXXX which serves as an index into the library's dispatch table.
//   The constant sysLibTrapCustom is the first available trap number after
//   the system predefined library traps Open,Close,Sleep & Wake.
//
// WARNING!!! This order of these traps MUST match the order of the dispatch
//  table in NetDispatch.c!!!
//-----------------------------------------------------------------------------

#define netLibTrapAddrINToA					(sysLibTrapCustom)
#define netLibTrapAddrAToIN					(sysLibTrapCustom+1)

#define netLibTrapSocketOpen					(sysLibTrapCustom+2)
#define netLibTrapSocketClose					(sysLibTrapCustom+3)
#define netLibTrapSocketOptionSet			(sysLibTrapCustom+4)
#define netLibTrapSocketOptionGet			(sysLibTrapCustom+5)
#define netLibTrapSocketBind					(sysLibTrapCustom+6)
#define netLibTrapSocketConnect				(sysLibTrapCustom+7)
#define netLibTrapSocketListen				(sysLibTrapCustom+8)
#define netLibTrapSocketAccept				(sysLibTrapCustom+9)
#define netLibTrapSocketShutdown				(sysLibTrapCustom+10)

#define netLibTrapSendPB						(sysLibTrapCustom+11)
#define netLibTrapSend							(sysLibTrapCustom+12)
#define netLibTrapReceivePB					(sysLibTrapCustom+13)
#define netLibTrapReceive						(sysLibTrapCustom+14)
#define netLibTrapDmReceive					(sysLibTrapCustom+15)
#define netLibTrapSelect						(sysLibTrapCustom+16)

#define netLibTrapPrefsGet						(sysLibTrapCustom+17)
#define netLibTrapPrefsSet						(sysLibTrapCustom+18)

// The following traps are for internal and Network interface
// use only.
#define netLibTrapDrvrWake						(sysLibTrapCustom+19)
#define netLibTrapInterfacePtr				(sysLibTrapCustom+20)
#define netLibTrapMaster						(sysLibTrapCustom+21)

// New Traps
#define netLibTrapGetHostByName				(sysLibTrapCustom+22)
#define netLibTrapSettingGet					(sysLibTrapCustom+23)
#define netLibTrapSettingSet					(sysLibTrapCustom+24)
#define netLibTrapIFAttach						(sysLibTrapCustom+25)
#define netLibTrapIFDetach						(sysLibTrapCustom+26)
#define netLibTrapIFGet							(sysLibTrapCustom+27)
#define netLibTrapIFSettingGet				(sysLibTrapCustom+28)
#define netLibTrapIFSettingSet				(sysLibTrapCustom+29)
#define netLibTrapIFUp							(sysLibTrapCustom+30)
#define netLibTrapIFDown						(sysLibTrapCustom+31)
#define netLibTrapIFMediaUp					(sysLibTrapCustom+32)
#define netLibTrapScriptExecuteV32			(sysLibTrapCustom+33)
#define netLibTrapGetHostByAddr				(sysLibTrapCustom+34)
#define netLibTrapGetServByName				(sysLibTrapCustom+35)
#define netLibTrapSocketAddr					(sysLibTrapCustom+36)
#define netLibTrapFinishCloseWait			(sysLibTrapCustom+37)
#define netLibTrapGetMailExchangeByName	(sysLibTrapCustom+38)
#define netLibTrapPrefsAppend					(sysLibTrapCustom+39)
#define netLibTrapIFMediaDown					(sysLibTrapCustom+40)
#define netLibTrapOpenCount					(sysLibTrapCustom+41)

#define netLibTrapTracePrintF					(sysLibTrapCustom+42)
#define netLibTrapTracePutS					(sysLibTrapCustom+43)

#define netLibTrapOpenIfCloseWait			(sysLibTrapCustom+44)
#define netLibTrapHandlePowerOff				(sysLibTrapCustom+45)

#define netLibTrapConnectionRefresh			(sysLibTrapCustom+46)

// Traps added after 1.0 release of NetLib
#define netLibTrapBitMove						(sysLibTrapCustom+47)
#define netLibTrapBitPutFixed					(sysLibTrapCustom+48)
#define netLibTrapBitGetFixed					(sysLibTrapCustom+49)
#define netLibTrapBitPutUIntV					(sysLibTrapCustom+50)
#define netLibTrapBitGetUIntV					(sysLibTrapCustom+51)
#define netLibTrapBitPutIntV					(sysLibTrapCustom+52)
#define netLibTrapBitGetIntV					(sysLibTrapCustom+53)

// Traps added after 2.0 release of NetLib
#define netLibOpenConfig						(sysLibTrapCustom+54)
#define netLibConfigMakeActive				(sysLibTrapCustom+55)
#define netLibConfigList						(sysLibTrapCustom+56)
#define netLibConfigIndexFromName			(sysLibTrapCustom+57)
#define netLibConfigDelete						(sysLibTrapCustom+58)
#define netLibConfigSaveAs						(sysLibTrapCustom+59)
#define netLibConfigRename						(sysLibTrapCustom+60)
#define netLibConfigAliasSet					(sysLibTrapCustom+61)
#define netLibConfigAliasGet					(sysLibTrapCustom+62)

// Traps added after 3.2 release of NetLib
#define netLibTrapScriptExecute				(sysLibTrapCustom+63)

#define netLibTrapLast							(sysLibTrapCustom+64)


/************************************************************
 * Net Library procedures.
 *************************************************************/ 
#pragma mark Functions
#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------
// Library initialization, shutdown, sleep and wake
//--------------------------------------------------
Err				NetLibOpen (UInt16 libRefnum, UInt16 *netIFErrsP)
						SYS_TRAP(sysLibTrapOpen);
					
Err				NetLibClose (UInt16 libRefnum, UInt16 immediate)
						SYS_TRAP(sysLibTrapClose);
					
Err				NetLibSleep (UInt16 libRefnum)
						SYS_TRAP(sysLibTrapSleep);
					
Err				NetLibWake (UInt16 libRefnum)
						SYS_TRAP(sysLibTrapWake);
					
					
// This call forces the library to complete a close if it's
//  currently in the close-wait state. Returns 0 if library is closed,
//  Returns netErrFullyOpen if library is still open by some other task.
Err				NetLibFinishCloseWait(UInt16 libRefnum)
						SYS_TRAP(netLibTrapFinishCloseWait);
						
// This call is for use by the Network preference panel only. It
// causes the NetLib to fully open if it's currently in the close-wait 
//  state. If it's not in the close wait state, it returns an error code
Err				NetLibOpenIfCloseWait(UInt16 libRefnum)
						SYS_TRAP(netLibTrapOpenIfCloseWait);
						
// Get the open Count of the NetLib
Err				NetLibOpenCount (UInt16 refNum, UInt16 *countP)
						SYS_TRAP(netLibTrapOpenCount);
					
// Give NetLib a chance to close the connection down in response
// to a power off event. Returns non-zero if power should not be
//  turned off. EventP points to the event that initiated the power off
//  which is either a keyDownEvent of the hardPowerChr or the autoOffChr.
// Don't include unless building for Viewer
#ifdef __SYSEVENT_H__
Err				NetLibHandlePowerOff (UInt16 refNum, SysEventType *eventP)
						SYS_TRAP(netLibTrapHandlePowerOff);
#endif
						
	
// Check status or try and reconnect any interfaces which have come down.
// This call can be made by applications when they suspect that an interface
// has come down (like PPP or SLIP). NOTE: This call can display UI 
// (if 'refresh' is true) so it MUST be called from the UI task. 
Err				NetLibConnectionRefresh(UInt16 refNum, Boolean refresh, 
							UInt8 *allInterfacesUpP, UInt16 *netIFErrP)
						SYS_TRAP(netLibTrapConnectionRefresh); 
						
						
					
//--------------------------------------------------
// Net address translation and conversion routines.
//--------------------------------------------------

// (The NetHToNS, NetHToNL, NetNToHS, and NetNToHL macros which used to be
// defined here are now defined in NetBitUtils.h.  They can still be used
// by #including <NetMgr.h> (this file), because <NetBitUtils.h> is
// unconditionally included below.)

// Convert 32-bit IP address to ascii dotted decimal form. The Sockets glue
//  macro inet_ntoa will pass the address of an application global string in
//  spaceP.
Char *			NetLibAddrINToA(UInt16 libRefnum, NetIPAddr	inet, Char *spaceP)
						SYS_TRAP(netLibTrapAddrINToA);
					
// Convert a dotted decimal ascii string format of an IP address into
//  a 32-bit value.
NetIPAddr		NetLibAddrAToIN(UInt16 libRefnum, const Char *a)
						SYS_TRAP(netLibTrapAddrAToIN);
						


//--------------------------------------------------
// Socket creation and option setting
//--------------------------------------------------

// Create a socket and return a refnum to it. Protocol is normally 0. 
// Returns 0 on success, -1 on error. If error, *errP gets filled in with error code.
NetSocketRef	NetLibSocketOpen(UInt16 libRefnum, NetSocketAddrEnum domain, 
							NetSocketTypeEnum type, Int16 protocol, Int32 timeout, 
							Err *errP)
						SYS_TRAP(netLibTrapSocketOpen);

// Close a socket. 
// Returns 0 on success, -1 on error. If error, *errP gets filled in with error code.
Int16				NetLibSocketClose(UInt16 libRefnum, NetSocketRef socket, Int32 timeout, 
							Err *errP)
						SYS_TRAP(netLibTrapSocketClose);
						
// Set a socket option.	Level is usually netSocketOptLevelSocket. Option is one of
//  netSocketOptXXXXX. OptValueP is a pointer to the new value and optValueLen is
//  the length of the option value.				
// Returns 0 on success, -1 on error. If error, *errP gets filled in with error code.
Int16				NetLibSocketOptionSet(UInt16 libRefnum, NetSocketRef socket,
							UInt16 /*NetSocketOptLevelEnum*/ level, UInt16 /*NetSocketOptEnum*/ option, 
							void *optValueP, UInt16 optValueLen,
							Int32 timeout, Err *errP)
						SYS_TRAP(netLibTrapSocketOptionSet);
						
// Get a socket option.					
// Returns 0 on success, -1 on error. If error, *errP gets filled in with error code.
Int16				NetLibSocketOptionGet(UInt16 libRefnum, NetSocketRef socket,
							UInt16 /*NetSocketOptLevelEnum*/ level, UInt16 /*NetSocketOptEnum*/ option,
							void *optValueP, UInt16 *optValueLenP,
							Int32 timeout, Err *errP)
						SYS_TRAP(netLibTrapSocketOptionGet);
						

//--------------------------------------------------
// Socket Control
//--------------------------------------------------

// Bind a source address and port number to a socket. This makes the
//  socket accept incoming packets destined for the given socket address.
// Returns 0 on success, -1 on error. If error, *errP gets filled in with error code.
Int16				NetLibSocketBind(UInt16 libRefnum, NetSocketRef socket,
							NetSocketAddrType *sockAddrP, Int16 addrLen, Int32 timeout, 
							Err *errP)
						SYS_TRAP(netLibTrapSocketBind);
						
						
// Connect to a remote socket. For a stream based socket (i.e. TCP), this initiates
//  a 3-way handshake with the remote machine to establish a connection. For
//  non-stream based socket, this merely specifies a destination address and port
//  number for future outgoing packets from this socket.
// Returns 0 on success, -1 on error. If error, *errP gets filled in with error code.
Int16				NetLibSocketConnect(UInt16 libRefnum, NetSocketRef socket,
							NetSocketAddrType *sockAddrP, Int16 addrLen, Int32 timeout, 
							Err *errP)
						SYS_TRAP(netLibTrapSocketConnect);
						

// Makes a socket ready to accept incoming connection requests. The queueLen 
//  specifies the max number of pending connection requests that will be enqueued
//  while the server is busy handling other requests.
//  Only applies to stream based (i.e. TCP) sockets.
// Returns 0 on success, -1 on error. If error, *errP gets filled in with error code.
Int16				NetLibSocketListen(UInt16 libRefnum, NetSocketRef socket,
							UInt16	queueLen, Int32 timeout, Err *errP)
						SYS_TRAP(netLibTrapSocketListen);
						

// Blocks the current process waiting for an incoming connection request. The socket
//  must have previously be put into listen mode through the NetLibSocketListen call.
//  On return, *sockAddrP will have the remote machines address and port number.
//  Only applies to stream based (i.e. TCP) sockets.
// Returns 0 on success, -1 on error. If error, *errP gets filled in with error code.
Int16				NetLibSocketAccept(UInt16 libRefnum, NetSocketRef socket,
							NetSocketAddrType *sockAddrP, Int16 *addrLenP, Int32 timeout,
							Err *errP)
						SYS_TRAP(netLibTrapSocketAccept);


// Shutdown a connection in one or both directions.  
//  Only applies to stream based (i.e. TCP) sockets.
// Returns 0 on success, -1 on error. If error, *errP gets filled in with error code.
Int16				NetLibSocketShutdown(UInt16 libRefnum, NetSocketRef socket, 
							Int16 /*NetSocketDirEnum*/ direction, Int32 timeout, Err *errP)
						SYS_TRAP(netLibTrapSocketShutdown);
						


// Gets the local and remote addresses of a socket. Useful for TCP sockets that 
//  get dynamically bound at connect time. 
// Returns 0 on success, -1 on error. If error, *errP gets filled in with error code.
Int16				NetLibSocketAddr(UInt16 libRefnum, NetSocketRef socketRef,
							NetSocketAddrType *locAddrP, Int16 *locAddrLenP, 
							NetSocketAddrType *remAddrP, Int16 *remAddrLenP, 
							Int32 timeout, Err *errP)
						SYS_TRAP(netLibTrapSocketAddr);
						
						

//--------------------------------------------------
// Sending and Receiving
//--------------------------------------------------
// Send data through a socket. The data is specified through the NetIOParamType
//  structure.
// Flags is one or more of netMsgFlagXXX.
// Returns # of bytes sent on success, or -1 on error. If error, *errP gets filled 
//  in with error code.
Int16				NetLibSendPB(UInt16 libRefNum, NetSocketRef socket,
							NetIOParamType *pbP, UInt16 flags, Int32 timeout, Err *errP)
						SYS_TRAP(netLibTrapSendPB);

// Send data through a socket. The data to send is passed in a single buffer,
//  unlike NetLibSendPB. If toAddrP is not nil, the data will be sent to 
//  address *toAddrP.
// Flags is one or more of netMsgFlagXXX.
// Returns # of bytes sent on success, or -1 on error. If error, *errP gets filled 
//  in with error code.
Int16				NetLibSend(UInt16 libRefNum, NetSocketRef socket,
							void *bufP, UInt16 bufLen, UInt16 flags,
							void *toAddrP, UInt16 toLen, Int32 timeout, Err *errP)
						SYS_TRAP(netLibTrapSend);

// Receive data from a socket. The data is gatthered into buffers specified in the 
//  NetIOParamType structure.
// Flags is one or more of netMsgFlagXXX.
// Timeout is max # of ticks to wait, or -1 for infinite, or 0 for none.
// Returns # of bytes received, or -1 on error. If error, *errP gets filled in 
//  with error code.
Int16				NetLibReceivePB(UInt16 libRefNum, NetSocketRef socket,
							NetIOParamType *pbP, UInt16 flags, Int32 timeout, Err *errP)
						SYS_TRAP(netLibTrapReceivePB);

// Receive data from a socket. The data is read into a single buffer, unlike
//  NetLibReceivePB. If fromAddrP is not nil, *fromLenP must be initialized to
//  the size of the buffer that fromAddrP points to and on exit *fromAddrP will
//  have the address of the sender in it.
// Flags is one or more of netMsgFlagXXX.
// Timeout is max # of ticks to wait, or -1 for infinite, or 0 for none.
// Returns # of bytes received, or -1 on error. If error, *errP gets filled in 
//  with error code.
Int16				NetLibReceive(UInt16 libRefNum, NetSocketRef socket,
							void *bufP, UInt16 bufLen, UInt16 flags, 
							void *fromAddrP, UInt16 *fromLenP, Int32 timeout, Err *errP)
						SYS_TRAP(netLibTrapReceive);


// Receive data from a socket directly into a (write-protected) Data Manager 
//  record. 
// If fromAddrP is not nil, *fromLenP must be initialized to
//  the size of the buffer that fromAddrP points to and on exit *fromAddrP will
//  have the address of the sender in it.
// Flags is one or more of netMsgFlagXXX.
// Timeout is max # of ticks to wait, or -1 for infinite, or 0 for none.
// Returns # of bytes received, or -1 on error. If error, *errP gets filled in 
//  with error code.
Int16				NetLibDmReceive(UInt16 libRefNum, NetSocketRef socket,
							void *recordP, UInt32 recordOffset, UInt16 rcvLen, UInt16 flags, 
							void *fromAddrP, UInt16 *fromLenP, Int32 timeout, Err *errP)
						SYS_TRAP(netLibTrapDmReceive);


//--------------------------------------------------
// Name Lookups
//--------------------------------------------------
NetHostInfoPtr	NetLibGetHostByName(UInt16 libRefNum, const Char *nameP, 
							NetHostInfoBufPtr bufP, Int32	timeout, Err *errP)
						SYS_TRAP(netLibTrapGetHostByName);


NetHostInfoPtr	NetLibGetHostByAddr(UInt16 libRefNum, UInt8 *addrP, UInt16 len, UInt16 type,
							NetHostInfoBufPtr bufP, Int32	timeout, Err *errP)
						SYS_TRAP(netLibTrapGetHostByAddr);


NetServInfoPtr	NetLibGetServByName(UInt16 libRefNum, const Char *servNameP, 
							const Char *protoNameP,  NetServInfoBufPtr bufP, 
							Int32	timeout, Err *errP)
						SYS_TRAP(netLibTrapGetServByName);

// Looks up a mail exchange name and returns a list of hostnames for it. Caller
//  must pass space for list of return names (hostNames), space for 
//  list of priorities for those hosts (priorities) and max # of names to 
//  return (maxEntries).
// Returns # of entries found, or -1 on error. If error, *errP gets filled in
//  with error code.
Int16				NetLibGetMailExchangeByName(UInt16 libRefNum, Char *mailNameP, 
							UInt16 maxEntries, 
							Char hostNames[][netDNSMaxDomainName+1], UInt16 priorities[], 
							Int32	timeout, Err *errP)
						SYS_TRAP(netLibTrapGetMailExchangeByName);


//--------------------------------------------------
// Interface setup
//--------------------------------------------------
Err				NetLibIFGet(UInt16 libRefNum, UInt16 index, UInt32 *ifCreatorP, 
								UInt16 *ifInstanceP)
						SYS_TRAP(netLibTrapIFGet);

Err				NetLibIFAttach(UInt16 libRefNum, UInt32 ifCreator, UInt16 ifInstance,
							Int32 timeout)
						SYS_TRAP(netLibTrapIFAttach);

Err				NetLibIFDetach(UInt16 libRefNum, UInt32 ifCreator, UInt16 ifInstance,
							Int32 timeout)
						SYS_TRAP(netLibTrapIFDetach);

Err				NetLibIFUp(UInt16 libRefNum, UInt32 ifCreator, UInt16 ifInstance)
						SYS_TRAP(netLibTrapIFUp);

Err				NetLibIFDown(UInt16 libRefNum, UInt32 ifCreator, UInt16 ifInstance,
							Int32 timeout)
						SYS_TRAP(netLibTrapIFDown);




//--------------------------------------------------
// Settings
//--------------------------------------------------
// General settings
Err				NetLibSettingGet(UInt16 libRefNum,
							UInt16 /*NetSettingEnum*/ setting, void *valueP, UInt16 *valueLenP)
						SYS_TRAP(netLibTrapSettingGet);

Err				NetLibSettingSet(UInt16 libRefNum, 
							UInt16 /*NetSettingEnum*/ setting, void *valueP, UInt16 valueLen)
						SYS_TRAP(netLibTrapSettingSet);
						
// Network interface specific settings.
Err				NetLibIFSettingGet(UInt16 libRefNum, UInt32 ifCreator, UInt16 ifInstance,
							UInt16 /*NetIFSettingEnum*/ setting, void *valueP, UInt16 *valueLenP)
						SYS_TRAP(netLibTrapIFSettingGet);

Err				NetLibIFSettingSet(UInt16 libRefNum, UInt32 ifCreator, UInt16 ifInstance,
							UInt16 /*NetIFSettingEnum*/ setting, void *valueP, UInt16 valueLen)
						SYS_TRAP(netLibTrapIFSettingSet);



//--------------------------------------------------
// System level
//--------------------------------------------------
Int16				NetLibSelect(UInt16 libRefNum, UInt16 width, NetFDSetType *readFDs, 
							NetFDSetType *writeFDs, NetFDSetType *exceptFDs,
							Int32	timeout, Err *errP)
						SYS_TRAP(netLibTrapSelect);



//--------------------------------------------------
// Debugging support
//--------------------------------------------------
Err				NetLibMaster(UInt16 libRefNum, UInt16 cmd, NetMasterPBPtr pbP,
						Int32 timeout)
						SYS_TRAP(netLibTrapMaster);

Err				NetLibTracePrintF(UInt16 libRefNum, const Char *formatStr, ...)
						SYS_TRAP(netLibTrapTracePrintF);
						
Err				NetLibTracePutS(UInt16 libRefNum, Char *strP)
						SYS_TRAP(netLibTrapTracePutS);
						



						
//--------------------------------------------------
// Configuration Calls
//--------------------------------------------------
Err				NetLibOpenConfig( UInt16 refNum, UInt16 configIndex, UInt32 openFlags,
							UInt16 *netIFErrP)
						SYS_TRAP(netLibOpenConfig);

Err				NetLibConfigMakeActive( UInt16 refNum, UInt16 configIndex)
						SYS_TRAP(netLibConfigMakeActive);
						
Err				NetLibConfigList( UInt16 refNum, NetConfigNameType nameArray[],
							UInt16 *arrayEntriesP)
						SYS_TRAP(netLibConfigList);
						
Err				NetLibConfigIndexFromName( UInt16 refNum, NetConfigNamePtr nameP,
							UInt16 *indexP)
						SYS_TRAP(netLibConfigIndexFromName);
						
Err				NetLibConfigDelete( UInt16 refNum, UInt16 index)
						SYS_TRAP(netLibConfigDelete);
						
Err				NetLibConfigSaveAs( UInt16 refNum, NetConfigNamePtr nameP)
						SYS_TRAP(netLibConfigSaveAs);
						
Err				NetLibConfigRename( UInt16 refNum, UInt16 index,
							NetConfigNamePtr newNameP)
						SYS_TRAP(netLibConfigRename);

Err				NetLibConfigAliasSet( UInt16 refNum, UInt16 configIndex,
							UInt16 aliasToIndex)
						SYS_TRAP(netLibConfigAliasSet);

Err				NetLibConfigAliasGet( UInt16 refNum, UInt16 aliasIndex,
							UInt16 *indexP, Boolean *isAnotherAliasP)
						SYS_TRAP(netLibConfigAliasGet);

						


#ifdef __cplusplus
}
#endif


// Include the NetMgr Bit Utils
#include	<NetBitUtils.h>

#endif // __NETMGR_H__
