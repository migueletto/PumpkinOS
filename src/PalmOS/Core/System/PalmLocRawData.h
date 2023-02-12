/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PalmLocRawData.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Web Clipping proxy server (WCPS) supports a way to automatically
 *		determine where the handheld is located on the planet.
 *		This is done through the use of raw location information provided
 *		by the low level driver that establishes the connection to the
 *		communication device (ex: phone, modem, over GSM, BellSouth, Japan's
 *		DoCoMo wireless system).  The raw data is passed to WCPS through
 *		the CTP protocol and WCPS pass it back to Location DLLs.
 *
 *		WCPS uses the palmLocRawDataType defined below to determine
 *		what Location DLLs it will have to call from an array of DLLs in order
 *		to decode the raw data into meaningful content for content providers
 *		like Postal code, City name, Country, GPS, etc.
 *
 * WARNING:		THIS FILE MUST STAY INDEPENDENT OF PALM OS
 *		This file will be given to 3rd party companies in order for them
 *		to develop Windows DLLs.  So it is very important that this file
 *		not be tied to Palm OS.
 *
 *****************************************************************************/

#ifndef		_PALMLOCRAWDATA_H_
#define		_PALMLOCRAWDATA_H_

// Let's define a type of UInt8 and a macro to define the values
// instead of using an enum because enums are not generating values
// of type UInt8 independently of the platform.
//
typedef	UInt8						  palmLocRawDataType;
#define PALM_RAW_DATA_VALUE(value)	((palmLocRawDataType)value)

typedef	UInt8		LocPacketSizeType;
typedef	UInt8		RawLocDataFirstByteType;
typedef	struct
{
	LocPacketSizeType			rawLocPacketSize;		// Size of this raw location packet
	palmLocRawDataType			rawLocDataType;			// Identifying the raw location info
														// from a value defined bellow in this file
	RawLocDataFirstByteType		rawLocDataFirstByte;	// actual data bytes of the raw location info
} RawLocPacketType, *RawLocPacketTypeP ;
#define	sizeofRawLocPacketTypeHeader	(sizeof(LocPacketSizeType) +sizeof(palmLocRawDataType))

// Location programming requires sending of raw location data to the server
// Elaine will then substitude the %Location:... string with the appropriate
// Location string for the content provider.

#define	palmLocRawDataNone	   			PALM_RAW_DATA_VALUE( 0 )
#define	palmLocRawDataEnd	   			PALM_RAW_DATA_VALUE( 0 )

// Raw location information returned by TelMgr drivers
#define	palmLocRawDataCDMA	   			PALM_RAW_DATA_VALUE( 1 )
#define	palmLocRawDataGSM	   			PALM_RAW_DATA_VALUE( 2 )
#define	palmLocRawDataTDMA	   			PALM_RAW_DATA_VALUE( 3 )
#define	palmLocRawDataPDC	   			PALM_RAW_DATA_VALUE( 4 )

#define	palmLocRawDataBellSouthTowerID	PALM_RAW_DATA_VALUE( 5 )

#define	palmLocRawDataDoCoMoIP			PALM_RAW_DATA_VALUE( 6 )


//
// Message to Companies developing location solutions
// --------------------------------------------------
// If your already registered raw data is not listed above,
// or you would like to register a new raw data, please contact
// Palm OS Development Support:
//   http://www.palmos.com/dev/tech/support


#endif	//	_PALMLOCRAWDATA_H_
