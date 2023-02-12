/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SerialLinkMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Source for Serial Link Routines on Pilot
 *
 *****************************************************************************/

#ifndef __SERIAL_LINK_H
#define __SERIAL_LINK_H



// Pilot common definitions
#include <PalmTypes.h>
#include <ErrorBase.h>


//*************************************************************************
//   Pre-defined, fixxed  Socket ID's
//*************************************************************************
#define		slkSocketDebugger			0			// Debugger Socket
#define		slkSocketConsole			1			// Console Socket
#define		slkSocketRemoteUI			2			// Remote UI Socket
#define		slkSocketDLP				3			// Desktop Link Socket
#define		slkSocketFirstDynamic	4			// first dynamic socket ID


//*************************************************************************
//  Packet Types
//*************************************************************************
#define		slkPktTypeSystem			0			// System packets
#define		slkPktTypeUnused1			1			// used to be: Connection Manager packets
#define		slkPktTypePAD				2			// PAD Protocol packets
#define		slkPktTypeLoopBackTest	3			// Loop-back test packets



//*************************************************************************
//
// Packet structure:
//		header
//		body (0-dbgMaxPacketBodyLength bytes of data)
//		footer
//
//*************************************************************************

//----------------------------------------------------------------------
// packet header
// Fields marked with -> must be filled in by caller
// Fields marked with X  will be filled in by SlkSendPacket.
//----------------------------------------------------------------------

typedef	UInt8	SlkPktHeaderChecksum;

typedef struct SlkPktHeaderType {
	UInt16					signature1;				// X  first 2 bytes of signature
	UInt8						signature2;				// X  3 and final byte of signature
	UInt8						dest;						// -> destination socket Id
	UInt8						src;						// -> src socket Id
	UInt8						type;						// -> packet type
	UInt16					bodySize;				// X  size of body
	UInt8						transId;					// -> transaction Id
															//    if 0 specified, it will be replaced 
	SlkPktHeaderChecksum	checksum;				// X  check sum of header
	} SlkPktHeaderType;

typedef SlkPktHeaderType*	SlkPktHeaderPtr;

#define	slkPktHeaderSignature1	0xBEEF
#define	slkPktHeaderSignature2	0xED

#define	slkPktHeaderSigFirst		0xBE			// First byte
#define	slkPktHeaderSigSecond	0xEF			// second byte
#define	slkPktHeaderSigThird		0xED			// third byte

//----------------------------------------------------------------------
// packet footer
//----------------------------------------------------------------------
typedef struct SlkPktFooterType {
	UInt16		crc16;				// header and body crc
	} SlkPktFooterType;

typedef SlkPktFooterType*	SlkPktFooterPtr;


//*************************************************************************
//
// Write Data Structure passed to SlkSendPacket. This structure 
//  Tells SlkSendPacket where each of the chunks that comprise the body are
//  and the size of each. SlkSendPacket accepts a pointer to an array
//  of SlkWriteDataTypes, the last one has a size field of 0.
//
//*************************************************************************
typedef struct SlkWriteDataType {
	UInt16	size;					// last one has size of 0
	const void*		dataP;				// pointer to data
	} SlkWriteDataType;
typedef SlkWriteDataType*	SlkWriteDataPtr;




//*************************************************************************
//
// CPU-dependent macros for getting/setting values from/to packets
//
//*************************************************************************

//--------------------------------------------------------------------
// macros to get packet values
//--------------------------------------------------------------------

#define	slkGetPacketByteVal(srcP)	(*(UInt8 *)(srcP))


#if (CPU_TYPE == CPU_x86)
#define	slkGetPacketWordVal(srcP)								\
	(	(UInt16)															\
		(																	\
		((UInt16)((UInt8 *)(srcP))[0] << 8) |					\
		((UInt16)((UInt8 *)(srcP))[1])							\
		)																	\
	)
#else
#define	slkGetPacketWordVal(srcP)								\
	( *((UInt16 *)(srcP)) )
#endif	//CPU_TYPE == CPU_x86


#if (CPU_TYPE == CPU_x86)
#define	slkGetPacketDWordVal(srcP)								\
	(	(UInt32)															\
		(																	\
		((UInt32)((UInt8 *)(srcP))[0] << 24) |					\
		((UInt32)((UInt8 *)(srcP))[1] << 16) |					\
		((UInt32)((UInt8 *)(srcP))[2] << 8) |					\
		((UInt32)((UInt8 *)(srcP))[3])							\
		)																	\
	)
#else
#define	slkGetPacketDWordVal(srcP)								\
	( *((UInt32 *)(srcP)) )
#endif	//CPU_TYPE == CPU_x86


#define	slkGetPacketSignature1(sigP)							\
	slkGetPacketWordVal(sigP)

#define	slkGetPacketSignature2(sigP)							\
	slkGetPacketByteVal(sigP)


#define	slkGetPacketDest(addressP)								\
	slkGetPacketByteVal(addressP)
	
#define	slkGetPacketSrc(addressP)								\
	slkGetPacketByteVal(addressP)
	
#define	slkGetPacketType(commandP)								\
	slkGetPacketByteVal(commandP)
	

#define	slkGetPacketBodySize(lengthP)							\
	slkGetPacketWordVal(lengthP)

#define	slkGetPacketTransId(transIDP)							\
	slkGetPacketByteVal(transIDP)

#define	slkGetPacketHdrChecksum(checksumP)					\
	slkGetPacketByteVal(checksumP)


#define	slkGetPacketTotalChecksum(checksumP)				\
	slkGetPacketWordVal(checksumP)






//--------------------------------------------------------------------
// macros to set packet values
//--------------------------------------------------------------------


#define	slkSetPacketByteVal(srcByteVal, destP)					\
	( *(UInt8 *)(destP) = (UInt8)(srcByteVal) )

#if (CPU_TYPE == CPU_x86)
#define	slkSetPacketWordVal(srcWordVal, destP)					\
																				\
	do {																		\
		UInt16	___srcVal;												\
		UInt8 *	___srcValP;												\
																				\
		___srcVal = (UInt16)(srcWordVal);							\
		___srcValP = (UInt8 *)(&___srcVal);							\
																				\
		((UInt8 *)(destP))[0] = ___srcValP[1];						\
		((UInt8 *)(destP))[1] = ___srcValP[0];						\
	} while( false )
#else
#define	slkSetPacketWordVal(srcWordVal, destP)					\
	( *((UInt16 *)(destP)) = (UInt16)(srcWordVal) )
#endif	//CPU_TYPE == CPU_x86


#if (CPU_TYPE == CPU_x86)
#define	slkSetPacketDWordVal(srcDWordVal, destP)				\
	do {																		\
		UInt32	___srcVal;												\
		UInt8 *	___srcValP;												\
																				\
		___srcVal = (UInt32)(srcDWordVal);							\
		___srcValP = (UInt8 *)(&___srcVal);							\
																				\
		((UInt8 *)(destP))[0] = ___srcValP[3];						\
		((UInt8 *)(destP))[1] = ___srcValP[2];						\
		((UInt8 *)(destP))[2] = ___srcValP[1];						\
		((UInt8 *)(destP))[3] = ___srcValP[0];						\
	} while( false )
#else
#define	slkSetPacketDWordVal(srcDWordVal, destP)				\
	( *((UInt32 *)(destP)) = (UInt32)(srcDWordVal) )
#endif	//CPU_TYPE == CPU_x86



#define slkSetPacketSignature1(magic, destP)						\
	slkSetPacketWordVal(magic, destP)

#define slkSetPacketSignature2(magic, destP)						\
	slkSetPacketByteVal(magic, destP)


#define slkSetPacketDest(dest, destP)								\
	slkSetPacketByteVal(dest, destP)

#define slkSetPacketSrc(src, destP)									\
	slkSetPacketByteVal(src, destP)


#define slkSetPacketType(type, destP)								\
	slkSetPacketByteVal(type, destP)


#define slkSetPacketBodySize(numBytes, destP)					\
	slkSetPacketWordVal(numBytes, destP)


#define slkSetPacketTransId(transID, destP)						\
	slkSetPacketByteVal(transID, destP)

#define slkSetPacketHdrChecksum(checksum, destP)				\
	slkSetPacketByteVal(checksum, destP)

#define slkSetPacketTotalChecksum(checksum, destP)				\
	slkSetPacketWordVal(checksum, destP)






/*******************************************************************
 * Serial Link Manager Errors
 * the constant slkErrorClass is defined in SystemMgr.h
 *******************************************************************/
#define	slkErrChecksum				(slkErrorClass | 1)
#define	slkErrFormat				(slkErrorClass | 2)
#define	slkErrBuffer				(slkErrorClass | 3)
#define	slkErrTimeOut				(slkErrorClass | 4)
#define	slkErrHandle				(slkErrorClass | 5)
#define	slkErrBodyLimit			(slkErrorClass | 6)
#define	slkErrTransId				(slkErrorClass | 7)
#define	slkErrResponse				(slkErrorClass | 8)
#define	slkErrNoDefaultProc		(slkErrorClass | 9)
#define	slkErrWrongPacketType	(slkErrorClass | 10)
#define 	slkErrBadParam				(slkErrorClass | 11)
#define 	slkErrAlreadyOpen			(slkErrorClass | 12)
#define	slkErrOutOfSockets		(slkErrorClass | 13)
#define	slkErrSocketNotOpen		(slkErrorClass | 14)
#define	slkErrWrongDestSocket	(slkErrorClass | 15)
#define	slkErrWrongPktType		(slkErrorClass | 16)
#define	slkErrBusy					(slkErrorClass | 17)	// called while sending a packet
																		// only returned on single-threaded
																		// emulation implementations 
#define	slkErrNotOpen				(slkErrorClass | 18)



/*******************************************************************
 * Type definition for a Serial Link Socket Listener
 *
 *******************************************************************/
typedef	void (*SlkSocketListenerProcPtr)
			(SlkPktHeaderPtr headerP, void *bodyP);
			
typedef struct SlkSocketListenType {
	SlkSocketListenerProcPtr 	listenerP;
	SlkPktHeaderPtr				headerBufferP;		// App allocated buffer for header
	void*								bodyBufferP;		// App allocated buffer for body
	UInt32							bodyBufferSize;
	} SlkSocketListenType;
typedef SlkSocketListenType*	SlkSocketListenPtr;



/*******************************************************************
 * Prototypes
 *******************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------
// Initializes the Serial Link Manager
//-------------------------------------------------------------------
Err			SlkOpen(void)
					SYS_TRAP(sysTrapSlkOpen);

//-------------------------------------------------------------------
// Close down the Serial Link Manager
//-------------------------------------------------------------------
Err			SlkClose(void)
					SYS_TRAP(sysTrapSlkClose);




//-------------------------------------------------------------------
// Open up another Serial Link socket. The caller must have already
//  opened the comm library and set it to the right settings.
//-------------------------------------------------------------------

Err			SlkOpenSocket(UInt16 portID, UInt16 *socketP, Boolean staticSocket)
					SYS_TRAP(sysTrapSlkOpenSocket);


//-------------------------------------------------------------------
// Close up a Serial Link socket. 
//  Warning: This routine is assymetrical with SlkOpenSocket because it
//   WILL CLOSE the library for the caller (unless the refNum is the
//   refNum of the debugger comm library).
//-------------------------------------------------------------------
Err			SlkCloseSocket(UInt16 socket)
					SYS_TRAP(sysTrapSlkCloseSocket);
					

//-------------------------------------------------------------------
// Get the library refNum for a particular Socket
//-------------------------------------------------------------------


	Err SlkSocketPortID(UInt16 socket, UInt16 *portIDP)
					SYS_TRAP(sysTrapSlkSocketRefNum);

	#define SlkSocketRefNum SlkSocketPortID

			
//-------------------------------------------------------------------
// Set the in-packet timeout for a socket
//-------------------------------------------------------------------
Err			SlkSocketSetTimeout(UInt16 socket, Int32 timeout)
					SYS_TRAP(sysTrapSlkSocketSetTimeout);





//-------------------------------------------------------------------
// Flush a Socket
//-------------------------------------------------------------------
Err			SlkFlushSocket(UInt16 socket, Int32 timeout)
					SYS_TRAP(sysTrapSlkFlushSocket);


//-------------------------------------------------------------------
// Set up a Socket Listener
//-------------------------------------------------------------------
Err			SlkSetSocketListener(UInt16 socket,  SlkSocketListenPtr socketP)
					SYS_TRAP(sysTrapSlkSetSocketListener);


//-------------------------------------------------------------------
// Sends a packet's header, body, footer.  Stuffs the header's
// magic number and checksum fields.  Expects all other
// header fields to be filled in by caller.
// errors returned: dseHandle, dseLine, dseIO, dseParam, dseBodyLimit,
//					dseOther
//-------------------------------------------------------------------
Err 			SlkSendPacket(SlkPktHeaderPtr headerP, SlkWriteDataPtr writeList)
					SYS_TRAP(sysTrapSlkSendPacket);


//-------------------------------------------------------------------
// Receives and validates an entire packet.
// errors returned: dseHandle, dseParam, dseLine, dseIO, dseFormat,
//					dseChecksum, dseBuffer, dseBodyLimit, dseTimeOut,
//					dseOther
//-------------------------------------------------------------------
Err			SlkReceivePacket( UInt16 socket, Boolean andOtherSockets, 
						SlkPktHeaderPtr headerP, void *bodyP,  UInt16 bodySize,  
						Int32 timeout)
					SYS_TRAP(sysTrapSlkReceivePacket);


//-------------------------------------------------------------------
// Do Default processing of a System packet
//-------------------------------------------------------------------
Err 			SlkSysPktDefaultResponse(SlkPktHeaderPtr headerP, void *bodyP)
					SYS_TRAP(sysTrapSlkSysPktDefaultResponse);

//-------------------------------------------------------------------
// Do RPC call
//-------------------------------------------------------------------
Err 			SlkProcessRPC(SlkPktHeaderPtr headerP, void *bodyP)
					SYS_TRAP(sysTrapSlkProcessRPC);



#ifdef __cplusplus
}
#endif
	
	
#endif	//__SERIAL_LINK_H
