/******************************************************************************
 *
 * Copyright (c) 1998-2003 PalmSource, Inc. All rights reserved.
 *
 * File: ConnectionMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Connection Manager Interface.  The Connection Manager allows
 *      other applications to access, add, and delete connection profiles
 *      contained in the Connection Panel.
 *
 *****************************************************************************/

#ifndef __CONNECTIONMGR_H__
#define __CONNECTIONMGR_H__

// Include elementary types
#include <PalmTypes.h>
#include <CoreTraps.h>
#include <ErrorBase.h>
#include <ModemMgr.h>
#include <SystemMgr.h>
#include <SerialMgr.h>
#include <DataMgr.h>
#include <SystemResources.h>

/***********************************************************************
 * Definition
 ***********************************************************************/

typedef UInt32 CncProfileID;

/***********************************************************************
 *	Connection Profile Broadcasting
 ***********************************************************************/

#define kCncProfileNotifyCurrentVersion		1

typedef struct CncProfileNotifyDetailsTag {

	// In: 	version - This definition is version 1 (kCncProfileNotifyCurrentVersion)
	// 		later versions should include all the fields of version 1 plus
	//			any additional fields of CncProfileNotifyDetailsType
	UInt16			version;
	
	// In: Broacasted Profile ID 
	CncProfileID	profileID;	
	
	// In:  Device Kind of the profile
	UInt16			deviceKind;			

	// In: Resquested Action
	UInt16			request;				
} CncProfileNotifyDetailsType;


/***********************************************************************
 * Constants
 ***********************************************************************/
 
#define kCncProfileInvalidId				((CncProfileID)(0))
 
// Request's modifiers flags
#define kCncNotifyBecomeCurrentModifier		0x8000  	// Change the Client current settings
#define kCncNotifyAlertUserModifier				0x4000  	// ask for Client UI
#define kNotifyRequestMofifiersMask				(kCncNotifyBecomeCurrentModifier | kCncNotifyAlertUserModifier)						

// Requests
#define kCncNotifyCreateRequest					1		// the profile has been created
#define kCncNotifyModifyRequest					2		// the profile has been modified
#define kCncNotifyDeleteRequest					3		// the profile has been deleted
#define kCncNotifyUpdateListRequest				4		// the profile has been deleted

/***********************************************************************
 * Connection Profile ParamID Definition Macros and Constants
 ***********************************************************************/
 
#define kCncParamOSRange 							0x0000	// bit #15 set at 0
#define kCncParamThirdPartiesRange				0x8000	// bit #15 set at 1

#define kCncParamFixedLength						0x0000 	// bit #14 set to 0
#define kCncParamVariableLength					0x4000 	// bit #14 set to 1

#define kCncParamIDMask								0x07FF   // bit #0 to #10 set to 1 (11 bits)
#define kCncParamTypeMask							0x7800   // bit #11 to #14 set to 1 (4 bits)

// parameter type definition macros
#define kCncParamFixedLen							0x00 		// higth bit of 4 set to 0 
#define kCncParamVariableLen 						0x08 		// higth bit of 4 set to 1 

#define CncDefineParameterType( variableBit , typeOrder) ( ( (variableBit) | (typeOrder) ) << 11)

// bit number is comprised between 0 and 31 
#define CncDefineSystemFlagMask(bitnum) 		( 1 << (bitnum) )

#define kCncParamSystemFlag						CncDefineParameterType(kCncParamFixedLen, 0)		// 0x0000  
#define kCncParamUInt8								CncDefineParameterType(kCncParamFixedLen, 1)		// 0x0800
#define kCncParamUInt16								CncDefineParameterType(kCncParamFixedLen, 2)		// 0x1000
#define kCncParamUInt32								CncDefineParameterType(kCncParamFixedLen, 3)		// 0x1800
// [free slot from 4 to 7]

#define kCncParamSystemFlagSize					kCncParamUInt8Size 
#define kCncParamUInt8Size							( sizeof(UInt8) )
#define kCncParamUInt16Size						( sizeof(UInt16) )
#define kCncParamUInt32Size						( sizeof(UInt32) )

#define kCncParamString  							CncDefineParameterType(kCncParamVariableLen, 1)			// 0x4800	
#define kCncParamBuffer								CncDefineParameterType(kCncParamVariableLen, 2)			// 0x5000
// [free slot from 3 to 7]

// full Parameter ID  definition macro
#define CncDefineParamID(parameterRange, parameterType, parameterID) 	( (parameterRange) | (parameterType)  | (parameterID) )

#define CncIsSystemRange(parameterID) 												( ( (parameterID) & kCncParamThirdPartiesRange)  != kCncParamThirdPartiesRange)
#define CncIsThirdPartiesRange(parameterID) 										( ( (parameterID) & kCncParamThirdPartiesRange ) == kCncParamThirdPartiesRange)

#define CncIsFixedLengthParamType(parameterID) 									( ( (parameterID) & kCncParamVariableLength)  != kCncParamVariableLength )
#define CncIsVariableLengthParamType(parameterID) 								( ( (parameterID) & kCncParamVariableLength ) == kCncParamVariableLength)

#define CncGetTrueParamID(parameterID)  											( (parameterID) & kCncParamIDMask) 
#define CncGetParamType(parameterID)  												( (parameterID) & kCncParamTypeMask) 

#define CncIsSystemFlags(parameterID) 												(  ! (CncGetParamType( (parameterID) ) ) )
#define CncGetSystemFlagBitnum(parameterID)  									CncGetTrueParamID(parameterID)

// Some tests


/***********************************************************************
 * Cnc Manager Feature
 ***********************************************************************/

#define kCncFtrCncMgrCreator 						'cmgr'

#define kCncFtrCncMgrVersion						0			
#define kCncMgrVersion				 				0x00040001 	// 4.0 =  4->high 0->low
// feature index 1 and 2 are reserved

/***********************************************************************
 * Parameter size values
 ***********************************************************************/

//	22 for compatibility
#define kCncProfileNameSize					22

//	81 defined in ModemMgr.h
#define kCncProfileUsualInitStringSize		mdmCmdBufSize	
	
// 	81  defined in ModemMgr.h				
#define kCncProfileClassicResetStringSize		mdmCmdSize	// Old size was 8						
#define kCncProfileUsualResetStringSize		mdmCmdBufSize							

/***********************************************************************
 * Parameters values
 ***********************************************************************/

// device kinds 
#define kCncDeviceKindSerial 						0
#define kCncDeviceKindModem 						1
#define kCncDeviceKindPhone 						2
#define kCncDeviceKindLocalNetwork				3

// Old flow controls 
#define kCncFlowControlAuto						0
#define kCncFlowControlOFF 						1
#define kCncFlowControlON							2

#define kCncProfileVersion							4

/***********************************************************************
 * Error Codes
 ***********************************************************************/

#define kCncErrAddProfileFailed					(cncErrorClass | 0x01)		// Add profile attempt failed
#define kCncErrProfileListFull					(cncErrorClass | 0x02) 		// Add attempt failed because the
																							// profile list is full.
#define kCncErrGetProfileFailed					(cncErrorClass | 0x03)		// Get profile attempt failed
#define kCncErrDBAccessFailed					(cncErrorClass | 0x04)		// Connection database not found or access failed
#define kCncErrGetProfileListFailed				(cncErrorClass | 0x05)		// Could not get profile list
#define kCncErrProfileReadOnly					(cncErrorClass | 0x06) 		// The profile can not be altered
#define kCncErrProfileNotFound					(cncErrorClass | 0x07) 		// The profile could not be found

// New API error code
#define kCncErrProfileParamNotFound				(cncErrorClass | 0x08) 		// The profile parameter could not be found
#define kCncErrProfileParamReadOnly				(cncErrorClass | 0x09) 		// The profile parameter can only be read
#define kCncErrProfileParamNameHasChange		(cncErrorClass | 0x0a) 	// The profile parameter Name has been modified to be unique
#define kCncErrProfileGetParamFailed			(cncErrorClass | 0x0b) 	// failed to get a parameter in a profile
#define kCncErrProfileSetParamFailed			(cncErrorClass | 0x0c) 	// failed to Set a parameter in a profile
#define kCncErrProfileBadParamSize				(cncErrorClass | 0x0d) 	// failed to Set a parameter in a profile
#define kCncErrProfileBadSystemFlagBitnum		(cncErrorClass | 0x0e) 	// the bit num of a system flag is not comprise between 0 and 31

/***********************************************************************
 * Parameters ID  and Sizes
 ***********************************************************************/

// void param has a size of zero bytes 
#define kCncNoParam									0
#define kCncNoParamSize								0																				

// 22 bytes limited  - for compatibility 
#define kCncParamName								CncDefineParamID( kCncParamOSRange, kCncParamString, 1 )		
#define kCncParamNameMaxSize						kCncProfileNameSize												

#define kCncParamPort								CncDefineParamID( kCncParamOSRange , kCncParamUInt32, 2 ) 
#define kCncParamPortSize							kCncParamUInt32Size		

#define kCncParamBaud								CncDefineParamID( kCncParamOSRange , kCncParamUInt32, 3 )
#define kCncParamBaudSize							kCncParamUInt32Size

#define kCncParamVolume								CncDefineParamID( kCncParamOSRange , kCncParamUInt16, 4 )	
#define kCncParamVolumeSize						kCncParamUInt16Size

#define kCncParamFlowControl						CncDefineParamID( kCncParamOSRange , kCncParamUInt16, 5 )									
#define kCncParamFlowControlSize					kCncParamUInt16Size

// New piece of info - communication time Out  (CTS) 
#define kCncParamTimeOut							CncDefineParamID( kCncParamOSRange , kCncParamUInt32, 6 ) 	
#define kCncParamTimeOutSize						kCncParamUInt32Size													 	

#define kCncParamInitString						CncDefineParamID( kCncParamOSRange, kCncParamString, 7 )									
#define kCncParamInitStringMaxSize				mdmCmdBufSize										

#define kCncParamResetString						CncDefineParamID( kCncParamOSRange, kCncParamString, 8)											
#define kCncParamResetStringMaxSize				mdmCmdBufSize											

// New piece of info -  extented device kind cf kCncDeviveXXX  after
#define kCncParamDeviceKind						CncDefineParamID( kCncParamOSRange, kCncParamUInt16, 9)		
#define kCncParamDeviceKindSize					kCncParamUInt16Size		

// country index for the profile 
#define kCncParamCountryIndex					CncDefineParamID( kCncParamOSRange, kCncParamUInt16, 11 )		
#define kCncParamCountryIndexSize				kCncParamUInt16Size			

// dialing mode, old pulse param
#define kCncParamDialingMode						CncDefineParamID( kCncParamOSRange, kCncParamUInt8, 12 )	
#define kCncParamDialingModeSize					kCncParamUInt8Size	

#define kCncParamVersion							CncDefineParamID( kCncParamOSRange, kCncParamUInt8, 13 )
#define kCncParamVersionSize						kCncParamUInt8Size

#define kCncParamReceiveTimeOut					CncDefineParamID( kCncParamOSRange , kCncParamUInt32, 14 )	
#define kCncParamReceiveTimeOutSize				kCncParamUInt32Size														

// International Reset string (count [strings])
#define kCncParamIntlModemResetStringList 		CncDefineParamID(kCncParamOSRange, kCncParamBuffer, 15) 


// International country string (count [strings])
#define kCncParamIntlModemCountryStringList		CncDefineParamID(kCncParamOSRange, kCncParamBuffer, 16) 

// special parameters : system flags
// the meaning of these parameters is for the connection panel
// up to 32 flags system flag will be possible

// bit numbering
#define kCncParamReadOnlyBit					0
#define kCncParamInvisibleBit					1
#define kCncParamNonEditableBit					2
#define kCncParamNoDetailsBit					3
#define kCncParamLockedBit						4 	
#define kCncParamReservedBit5					5 	
#define kCncParamReservedBit6					6	
#define kCncParamReservedBit7					7
#define kCncParamReservedBit8					8
#define kCncParamReservedBit9					9
#define kCncParamReservedBit10					10
#define kCncParamReservedBit11					11
#define kCncParamReservedBit12					12
#define kCncParamReservedBit13					13
#define kCncParamReservedBit14					14
#define kCncParamReservedBit15					15
#define kCncParamSystemBit16					16
#define kCncParamSystemBit17					17
#define kCncParamReservedBit18					18
#define kCncParamReservedBit19					19
#define kCncParamReservedBit20					20
#define kCncParamReservedBit21					21
#define kCncParamReservedBit22					22
#define kCncParamReservedBit23					23
#define kCncParamReservedBit24					24
#define kCncParamReservedBit25					25
#define kCncParamReservedBit26					26
#define kCncParamReservedBit27					27
#define kCncParamReservedBit28					28
#define kCncParamReservedBit29					29
#define kCncParamReservedBit30					30
#define kCncParamReservedBit31					31

#define kCncParamSystemFlagsNum					0x07FF

// the following parameter handles  the system flags as an UInt32 integer (all the flags, at once)
#define kCncParamSystemFlags						CncDefineParamID( kCncParamOSRange, kCncParamSystemFlag, kCncParamSystemFlagsNum)		
#define kCncParamSystemFlagsSize					kCncParamUInt32Size		

// bit parameters definition : to handle flags bit per bit
#define kCncParamReadOnly							CncDefineParamID( kCncParamOSRange, kCncParamSystemFlag, 0)	

#define kCncParamReadOnlySize						kCncParamSystemFlagSize		
																		
#define kCncParamInvisible							CncDefineParamID( kCncParamOSRange, kCncParamSystemFlag, 1 )
#define kCncParamInvisibleSize					kCncParamSystemFlagSize

#define kCncParamNonEditable						CncDefineParamID( kCncParamOSRange, kCncParamSystemFlag, 2 )
#define kCncParamNonEditableSize					kCncParamSystemFlagSize

#define kCncParamNoDetails							CncDefineParamID( kCncParamOSRange, kCncParamSystemFlag, 3 )
#define kCncParamNoDetailsSize					kCncParamSystemFlagSize

#define kCncParamLocked								CncDefineParamID( kCncParamOSRange, kCncParamSystemFlag, 4 )
#define kCncParamLockedSize						kCncParamSystemFlagSize

/* Bluetooth parameter IDs - New pieces of info */

// 48 bit blue Tooth address (BD_ADDR) - This address is derived from the IEEE802 standard

#define kCncParamBluetoothDeviceAddr			CncDefineParamID(kCncParamOSRange, kCncParamBuffer, 50 )		
#define kCncParamBluetoothDeviceAddrSize		8

// Bluetooth device name - 248 bytes coded according to the UTF-8 standard at max + NULL terninaison																		
#define kCncParamBluetoothDeviceName			CncDefineParamID(kCncParamOSRange, kCncParamString, 51 ) 	
#define kCncParamBluetoothDeviceNameMaxSize	249																			
																								
// Caution :  system parameter range ID from 80 to 200 are reserved for telephony services
// and should never be reused by any other component

/***********************************************************************
 * Telephony Manager parameter 
 ***********************************************************************/



/* TT-AT specific parameters */

// New piece gathering several parts (uses the serial manager flags cf SerialMgr.h )
#define kCncParamSerialPortFlags					CncDefineParamID(kCncParamOSRange, kCncParamUInt32, 84 )		
#define kCncParamSerialPortFlagsSize			kCncParamUInt32Size														

// Telephony Task type  - mobile telephony 
#define kCncParamTTType								CncDefineParamID( kCncParamOSRange, kCncParamUInt32, 90 ) 	
#define kCncParamTTTypeSize						kCncParamUInt32Size														

// Telephony Task Creator  - mobile telephony 
#define kCncParamTTCreator							CncDefineParamID( kCncParamOSRange, kCncParamUInt32, 91 )	
#define kCncParamTTCreatorSize					kCncParamUInt32Size														

// Phone Driver Name - mobile telephony
#define kCncParam_PSDName							CncDefineParamID( kCncParamOSRange, kCncParamString, 92 )	
#define kCncParam_PSDNameSize						dmDBNameLength	

// Phone Driver creator - mobile telephony
#define kCncParam_PSDCreator						CncDefineParamID( kCncParamOSRange, kCncParamUInt32, 93 )	
#define kCncParam_PSDCreatorSize					kCncParamUInt32Size			

// Phone Driver type - mobile telephony
#define kCncParam_PSDType							CncDefineParamID(kCncParamOSRange, kCncParamUInt32, 94 )		
#define kCncParam_PSDTypeSize						kCncParamUInt32Size			

// Phone Driver Param Buffer - mobile telephony
#define kCncParam_PSDParameterBuffer			CncDefineParamID(kCncParamOSRange, kCncParamBuffer, 100 )	

/***********************************************************************
 * New Connection Manager trap selectors
 ***********************************************************************/
 
#define sysTrapCncMgrProfileSettingGet						1	
#define sysTrapCncMgrProfileSettingSet						2	
#define sysTrapCncMgrProfileGetCurrent						3	
#define sysTrapCncMgrProfileSetCurrent						4	
#define sysTrapCncMgrProfileGetIDFromName					5	
#define sysTrapCncMgrProfileCreate							6	
#define sysTrapCncMgrProfileDelete							7	
#define sysTrapCncMgrProfileGetIDFromIndex					8	
#define sysTrapCncMgrProfileGetIndex						9	
#define sysTrapCncMgrProfileCount							10	
#define sysTrapCncMgrProfileOpenDB							11	
#define sysTrapCncMgrProfileCloseDB							12	


/***********************************************************************
 * Connection Manager  Library Macros
 ***********************************************************************/

#ifndef USE_CNCMGR_TRAPS 
	#if EMULATION_LEVEL == EMULATION_NONE
		#define	USE_CNCMGR_TRAPS 1						// use Pilot traps
	#else
		#define	USE_CNCMGR_TRAPS 0						// direct link
	#endif
#endif

#if (USE_CNCMGR_TRAPS == 1)
	#define OLD_CNCMGR_TRAP(TrapNum) TrapNum
	#define CNCMGR_TRAP(cncMgrSelectorNum) \
		_SYSTEM_API(_CALL_WITH_SELECTOR)(_SYSTEM_TABLE, sysTrapCncMgrDispatch, \
													cncMgrSelectorNum)
#else
	#define OLD_CNCMGR_TRAP(TrapNum)												
	#define CNCMGR_TRAP(cncMgrSelectorNum)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 * New  Connection Mgr API
 ***********************************************************************/

Err CncProfileSettingGet( CncProfileID profileId, UInt16 paramId, void* paramBufferP, UInt16 * ioParamSizeP )
	CNCMGR_TRAP(sysTrapCncMgrProfileSettingGet);
 
Err CncProfileSettingSet( CncProfileID iProfileId, UInt16 paramId, const void* paramBufferP, UInt16 paramSize )
	CNCMGR_TRAP(sysTrapCncMgrProfileSettingSet);
 
Err CncProfileSetCurrent( CncProfileID profileId )
	CNCMGR_TRAP(sysTrapCncMgrProfileSetCurrent);
 
Err CncProfileGetCurrent( CncProfileID * profileIdP )
	CNCMGR_TRAP(sysTrapCncMgrProfileGetCurrent);
 
Err CncProfileGetIDFromName( const Char* profileNameP, CncProfileID * profileIdP) 
	CNCMGR_TRAP(sysTrapCncMgrProfileGetIDFromName);

Err CncProfileCreate( CncProfileID * profileIdP )
	CNCMGR_TRAP(sysTrapCncMgrProfileCreate);

Err CncProfileDelete( CncProfileID  profileId )
	CNCMGR_TRAP(sysTrapCncMgrProfileDelete);

Err CncProfileGetIDFromIndex( UInt16 index, CncProfileID* profileIdP )
	CNCMGR_TRAP(sysTrapCncMgrProfileGetIDFromIndex);

Err CncProfileGetIndex( CncProfileID profileId, UInt16* indexP )
	CNCMGR_TRAP(sysTrapCncMgrProfileGetIndex);

Err CncProfileCount( UInt16* profilesCountP )
	CNCMGR_TRAP(sysTrapCncMgrProfileCount);

Err CncProfileOpenDB( void )
	CNCMGR_TRAP(sysTrapCncMgrProfileOpenDB);

Err CncProfileCloseDB( void )
	CNCMGR_TRAP(sysTrapCncMgrProfileCloseDB);

/***********************************************************************
 * Old  Connection Mgr API, For compatibility only
 ***********************************************************************/

// Maximum size for a Connection Profile Name
#define cncProfileNameSize	22

// Error Codes
#define cncErrAddProfileFailed		(cncErrorClass | 1)	// Add profile attempt failed
#define cncErrProfileListFull		(cncErrorClass | 2) // Add attempt failed because the
														// profile list is full.
#define cncErrGetProfileFailed		(cncErrorClass | 3)	// Get profile attempt failed
#define cncErrConDBNotFound			(cncErrorClass | 4)	// Connection database not found
#define cncErrGetProfileListFailed	(cncErrorClass | 5)	// Could not get profile list
#define cncErrProfileReadOnly		(cncErrorClass | 6) // The profile can not be altered
#define cncErrProfileNotFound		(cncErrorClass | 7) // The profile could not be found

// Functions

Err CncGetProfileList( Char*** nameListPPP, UInt16 *countP )
	OLD_CNCMGR_TRAP(SYS_TRAP(sysTrapCncGetProfileList));

Err CncGetProfileInfo( Char *name, UInt32 *port, UInt32 *baud, UInt16 *volume,
		UInt16 *handShake, Char *initString, Char *resetString,
		Boolean *isModem, Boolean *isPulse )
	OLD_CNCMGR_TRAP(SYS_TRAP(sysTrapCncGetProfileInfo));

Err CncAddProfile( Char *name, UInt32 port, UInt32 baud, UInt16 volume,
	 UInt16 handShake, const Char *initString, const Char *resetString, Boolean isModem,
	 Boolean isPulse )
	OLD_CNCMGR_TRAP(SYS_TRAP(sysTrapCncAddProfile));

Err CncDeleteProfile( const Char *name )
	OLD_CNCMGR_TRAP(SYS_TRAP(sysTrapCncDeleteProfile));

			
#ifdef __cplusplus 
}
#endif

#endif  // __CONNECTIONMGR_H__
