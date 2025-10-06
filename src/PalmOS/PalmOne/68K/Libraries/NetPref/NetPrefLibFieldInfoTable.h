/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @defgroup NETPREF NetPref Library
 *
 * @{
 * @}
 */

/**
 * @file 	NetPrefLibFieldInfoTable.h
 * @version 1.0
 * @date 	01/11/2002
 *
 * @brief This header file contains record field information for
 *        NetPref settings.
 *
 */

/*
 * @author   Vitaly Kruglikov
 * <hr>
 */


#ifndef __NETPREFLIBFIELDINFOTABLE_H__
#define __NETPREFLIBFIELDINFOTABLE_H__


// ***NOTE: all macros must be defined by the including module; for convenience,
// this file includes all the undefining statements at the end of the table.
// The usage model for this table requires that the includer first define each
// of the required macros and then include this file for the appropriate effect.

// Required macros:

// #define NetPrefRecFieldInfoPreamble()

// #define NetPrefRecFieldInfoFIRST(fieldID, idValue)

// #define NetPrefRecFieldInfoGrpStart(fieldID, idValue)

// #define NetPrefRecFieldInfoGrpEnd(fieldID)

// #define NetPrefRecFieldInfoExtended(fieldID, idValue, type, maxSize, flags,
//									   tagText, tagSize)

// #define NetPrefRecFieldInfoLegacy(fieldID, idValue, type, maxSize, flags,
//									   tagText, tagSize)

// if maxSize = 0, then size boundaries are determined entirely by field type

// #define NetPrefRecFieldInfoLAST(fieldID)




// ***IMPORTANT***
// All field ID values within a field group must be contiguous.
// The field id is included at the beginning of each tag, and separated
// by the ':' character from the rest of each tag.
//
// For most field values, if not set, the appropriate defaults will apply.
// The exceptions are account specific settings, such as username, password,
// phone number (for dial-up profiles), etc.

  // PREAMBLE
  NetPrefRecFieldInfoPreamble()

  // reserve 0
  NetPrefRecFieldInfoFIRST(		netPrefRecFieldUnknown,			  0)

  // ========================================================================
  // Range for extended fields:
  // ========================================================================
  // start of Handspring-extended fields
  NetPrefRecFieldInfoGrpStart(	netPrefRecFieldExtendedSTART,	  1)

  // UInt32; reserved for internal Handspring use only
  NetPrefRecFieldInfoExtended(	netPrefRecFieldUInt32Reserved1,	  2, \
			  netPrefFieldTypeUInt32, 0, 0, <2:i32rsvd1>, 12)

  // UInt16, Service Medium alias -- NetPrefSvcMediumEnum;
  // this overrides the connection profile setting (netPrefRecFieldConnection)
  // with a dynamically determined connection profile
  NetPrefRecFieldInfoExtended(	netPrefRecFieldSvcMediumAlias,	  3, \
			  netPrefFieldTypeUInt16, 0, 0, <3:medium>, 10)

  // UInt16, NetPrefSvcBindingEnum; not needed in most cases.
  // Handspring internal use only!
  NetPrefRecFieldInfoExtended(	netPrefRecFieldBinding,			  4, \
			  netPrefFieldTypeUInt16, 0, 0, <4:bind>, 8)

  // UInt16, Binding index; 0-based; for example: CCSM Home1/Roam1 binding
  // index should be set to 0; for Home2/Roam2 to 1, etc.  Used when
  // netPrefRecFieldBinding is set to netPrefSvcBindingCCSM
  // Handspring internal use only!
  NetPrefRecFieldInfoExtended(	netPrefRecFieldBindingIndex,	  5, \
			  netPrefFieldTypeUInt16, 0, 0, <5:bindidx>, 11)

  // UInt32,  NetPrefSvcGatewayType - Internet, WAP, etc. (1 or more
  // netPrefSvcGateway... constants bitwise-or'ed together; if the field is
  // absent, Internet is implied.
  NetPrefRecFieldInfoExtended(	netPrefRecFieldSvcGatewayType,	  6, \
			  netPrefFieldTypeUInt32,  0, 0, <6:gtwytyp>, 11)

  // UInt32, unique record ID of fallback service (must be one of the
  // records in the Network Preferences database)
  NetPrefRecFieldInfoExtended(	netPrefRecFieldFallbackID,		  7, \
			  netPrefFieldTypeUInt32, 0, 0, <7:fback>, 9)

  // char*, APN name for GPRS services
  NetPrefRecFieldInfoExtended(	netPrefRecFieldAPN,				  8, \
			  netPrefFieldTypeStr,	  0, 0, <8:apn>, 7)

  // char*, Modem Init override string (for CCSM, etc.).  For
  // CSD, ISND, QNC, and plain dial-up services only -- if specified,
  // it will override the modem init string that may have been specified
  // in the Connection profile in Connection Panel.
  NetPrefRecFieldInfoExtended(	netPrefRecFieldMdmInitStr,		  9, \
			  netPrefFieldTypeStr,	  0, 0, <9:mdmins>, 10)

  // UInt8; Is Mobile-IP; non-zero=true (0x01), zero=false (CDMA/1xRTT only)
  NetPrefRecFieldInfoExtended(	netPrefRecFieldIsMobileIP,		  10, \
			  netPrefFieldTypeUInt8,  0, 0, <10:mip>, 8)

  // UInt32, Primary HA Address, if Mobile-IP (CDMA/1xRTT)
  NetPrefRecFieldInfoExtended(	netPrefRecFieldHAAddrPrimary,	  11, \
			  netPrefFieldTypeUInt32, 0, 0, <11:ha1>, 8)

  // UInt32, Secondary HA Address, if Mobile-IP (CDMA/1xRTT)
  NetPrefRecFieldInfoExtended(	netPrefRecFieldHAAddrSecondary,	  12, \
			  netPrefFieldTypeUInt32, 0, 0, <12:ha2>, 8)

  // Binary, HA Password, if Mobile-IP  (CDMA/1xRTT)
  NetPrefRecFieldInfoExtended(	netPrefRecFieldMnHaSS,			  13, \
			  netPrefFieldTypeBin,	  0, 0, <13:hapsw>, 10)

  // UInt16, NetPref1xRTTTunnelingEnum; Tunneling, if Mobile-IP (CDMA/1xRTT)
  NetPrefRecFieldInfoExtended(	netPrefRecField1xRTTTunneling,	  14, \
			  netPrefFieldTypeUInt16, 0, 0, <14:1xtnl>, 10)

  // UInt32, SPI for MN-HA authentication, if Mobile-IP (CDMA/1xRTT)
  NetPrefRecFieldInfoExtended(	netPrefRecFieldMnHaSpi,			  15, \
			  netPrefFieldTypeUInt32, 0, 0, <15:haspi>, 10)

  // UInt32, SPI for MN-AAA authentication, if Mobile-IP (CDMA/1xRTT)
  NetPrefRecFieldInfoExtended(	netPrefRecFieldMnAaaSpi,		  16, \
			  netPrefFieldTypeUInt32, 0, 0, <16:aaasp>, 10)

  // char*, Override Dial String for special network connections (CCSM, 1xRTT,
  // etc.).  If set, overrides the default 1xRTT dial string for CDMA/1xRTT
  // services, and netPrefRecFieldPhoneNumber (and related fields) for CSD,
  // ISDN, QNC, and other dial-up services.
  NetPrefRecFieldInfoExtended(	netPrefRecFieldDialStr,			  17, \
			  netPrefFieldTypeStr,	  0, 0, <17:dials>, 10)

  // UInt32, NetPrefRecProtectionType -- record protection options
  NetPrefRecFieldInfoExtended(	netPrefRecFieldRecProtection,	  18, \
			  netPrefFieldTypeUInt32, 0, 0, <18:protect>, 12)

  // UInt32, -- permanent PDP index in radio's NV table (for CDMA/1xRTT
  // services with netPrefSvcBindingIOTA binding only -- internal use only!)
  NetPrefRecFieldInfoExtended(	netPrefRecFieldPDPIndex,		  19, \
			  netPrefFieldTypeUInt32, 0, 0, <19:pdpidx>, 11)

  // void*, -- Unique ID/Change number of the PDP slot in radio's NV table.
  // Presently used for CDMA/1xRTT IOTA-bound profiles. For internal use only!
  NetPrefRecFieldInfoExtended(	netPrefRecFieldPDPChangeID,		  20, \
			  netPrefFieldTypeBin,	  0, 0, <20:pdpchg>, 11)

  // UInt32,  Service Creator ID [OPTIONAL] -- an application that creates a
  // new service may set this field to a unique Creator ID value (the unique
  // Creator ID MUST be registered with a CreatorID registration authority --
  // for Palm OS, this is done through Palm's developer support WEB pages).
  // Applications that set this field, may also save creator-specific data
  // inside the netPrefRecFieldCreatorData field
  NetPrefRecFieldInfoExtended(	netPrefRecFieldCreatorID,		  21, \
			  netPrefFieldTypeUInt32,  0, 0, <21:crid>, 9)

  // void*, Creator-specific binary data field; applications that set
  // the netPrefRecFieldCreatorID field may also save application-specific
  // data inside this field.  see also netPrefRecFieldCreatorID.
  NetPrefRecFieldInfoExtended(	netPrefRecFieldCreatorData,		  22, \
			  netPrefFieldTypeBin,	  0, 0, <22:crdata>, 11)

  // NetPrefGPRSQOSType -- GPRS Quality of Service parameters
  NetPrefRecFieldInfoExtended(	netPrefRecFieldGPRSQOS,			  23, \
			  netPrefFieldTypeBin,	  0, 0, <23:gprsqos>, 12)

  // NetPref1xRTTQOSType -- 1xRTT Quality of Service parameters.
  // UNDEFINED/RESERVED BY HANDSPRING
  NetPrefRecFieldInfoExtended(	netPrefRecField1xRTTQOS,		  24, \
			  netPrefFieldTypeBin,	  0, 0, <24:1xrttqos>, 13)

  // void*; reserved for internal Handspring use only
  NetPrefRecFieldInfoExtended(	netPrefRecFieldBinReserved1,	  25, \
			  netPrefFieldTypeBin,	  0, 0, <25:binrsvd1>, 13)

  // char*, 1xRTT NAI field (CDMA/1xRTT)
  NetPrefRecFieldInfoExtended(	netPrefRecFieldNaiStr,			  26, \
			  netPrefFieldTypeStr,	  0, 0, <26:nai>, 8)

  // Binary, 1xRTT MN-AAA Shared Secret (NAI Password) (CDMA/1xRTT)
  //
  // IMPORTANT:
  //
  // ***The NAI Password may be text or binary.  If it is binary, it may
  // contain zeros anywhere in the password, so you cannot use string functions
  // (such as strlen, strcpy, etc.) on this field.
  //
  // ***If an ascii string password is being stored, it MUST be stored *without*
  // the terminating zero-byte!!!
  //
  // The NAI Password may be binary or ascii, depending on the method
  // the operator used to set its value (IOTA/MD Digest/etc.), or whether it was
  // entered by the user (user will enter in ascii, most likely).
  NetPrefRecFieldInfoExtended(	netPrefRecFieldMnAaaSs,			  27, \
			  netPrefFieldTypeBin,	  0, 0, <27:mnaaass>, 12)

  // UInt8, -- initial number of VJ Compression slots if VJ compression
  // is enabled; used to set
  // NetLib's netIFSettingVJCompSlots setting (see also
  // netPrefRecFieldVJCompEnable).
  NetPrefRecFieldInfoExtended(	netPrefRecFieldVJCompSlots,		  28, \
			  netPrefFieldTypeUInt8, 0, 0, <28:vjslots>, 12)

  // NetPrefFallbackInfoType -- Fallback Info: algrorithm and status.
  // NOTE: this field MUST be defined as READ/WRITE because the algorithm
  // will need to maintain the fallback status data in the same field.
  NetPrefRecFieldInfoExtended(	netPrefRecFieldFallbackInfo,	  29, \
			  netPrefFieldTypeBin,	  0, 0, <29:fbkinfo>, 12)


  // end of Handspring-extended fields (for internal use only -- # of extended
  // fields will likely grow in the future)
  NetPrefRecFieldInfoGrpEnd(	netPrefRecFieldExtendedEND)


  // ========================================================================
  // Range for legacy fields:
  // ========================================================================

  // START OF LEGACY FIELDS
  NetPrefRecFieldInfoGrpStart(	netPrefRecFieldLegacySTART,		  1000)

  // UInt32; 'slip', 'ppp-', 'loop' (netIFCreatorPPP, etc.), or OEM NPPI
  // creator if netPrefRecFieldSvcMediumAlias field is netPrefSvcMediumOEM.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldInterfaceID,		  1001, \
			  netPrefFieldTypeUInt32, 0, 0, <1001:ifid>, 11)

  // UInt8; non-zero=true, zero=false
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldIPAddrIsDynamic,	  1002, \
			  netPrefFieldTypeUInt8,  0, 0, <1002:dynip>, 12)

  // UInt32; IP address value, if not dynamic
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldIPAddr,			  1003, \
			  netPrefFieldTypeUInt32, 0, 0, <1003:ipaddr>, 13)

  // UInt8; non-zero=true (0x01), zero=false
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldDNSAddrIsDynamic,  1004, \
			  netPrefFieldTypeUInt8,  0, 0, <1004:dyndns>, 13)

  // UInt32; primary DNS address if not dynamic
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldDNSAddrPrimary,	  1005, \
			  netPrefFieldTypeUInt32, 0, 0, <1005:pridns>, 13)

  // UInt32; secondary DNS address if not dynamic
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldDNSAddrSecondary,  1006, \
			  netPrefFieldTypeUInt32, 0, 0, <1006:secdns>, 13)

  // UInt32; expressed in milliseconds; see NetPrefNetCloseWaitEnum for special
  // values; used to set NetLib's netSettingCloseWaitTime setting.
  // IMPORTANT: Beginning with Palm OS 5.0, netPrefRecFieldInactivityTimer
  // must be used to configure the idle timeout of the TCP/IP stack. The
  // "CloseWait" settting is used to control NetLib's CloseWait behavior
  // only.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldCloseWaitTime,	  1007, \
			  netPrefFieldTypeUInt32, 0, 0, <1007:cwtm>, 11)

  // UInt16; expressed in seconds; see NetPrefNetIdleTimeoutEnum for
  // special values; used to set NetLib interface's
  // netIFSettingInactivityTimeout setting (also known as "idle timeout").
  // IMPORTANT: This setting is enforced beginning with Palm OS 5.0 and
  // must be used to configure the idle timeout of the TCP/IP stack.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldInactivityTimer,	  1008, \
			  netPrefFieldTypeUInt16, 0, 0, <1008:inactm>, 13)

  // UInt16; used to set NetLib's
  // netIFSettingEstablishmentTimeout setting.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldEstablishmentTimeout,1009, \
			  netPrefFieldTypeUInt16, 0, 0, <1009:estbtm>, 13)

  // UInt8; non-zero=true, zero=false; used to set NetLib's
  // netIFSettingVJCompEnable setting.  See also netPrefRecFieldVJCompSlots.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldVJCompEnable,	  1010, \
			  netPrefFieldTypeUInt8,  0, 0, <1010:vjcmpr>, 13)

  // UInt8; not in interface -- NOT USED.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldUseModem,		  1011, \
			  netPrefFieldTypeUInt8,  0, 0, <1011:usemdm>, 13)

  // char*; service name
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldServiceName,		  1012, \
			  netPrefFieldTypeStr,	  0, 0, <1012:svcnam>, 13)

  // char*; username
  // (NOTE: the NAI for CDMA One-X (Mobile-IP and
  // Simple-IP) services is stored in another field -- netPrefRecFieldNaiStr.)
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldUsername,		  1013, \
			  netPrefFieldTypeStr,	  0, 0, <1013:usrnam>, 13)

  // char*; password
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldPassword,		  1014, \
			  netPrefFieldTypeStr,	  0, 0, <1014:usrpsw>, 13)

  // char*; domain name; used to set NetLib's netSettingDomainName setting;
  // might not be used by NetLib.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldDomainName,		  1015, \
			  netPrefFieldTypeStr,	  0, 0, <1015:domain>, 13)

  // char*; callback username, password; used to set NetLib's
  // netSettingHostTbl setting; might not be used by NetLib.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldHostTable,		  1016, \
			  netPrefFieldTypeStr,	  0, 0, <1016:hosttab>, 14)

  // char*; used to set NetLib's netIFSettingDialbackUsername -- yes,
  // netIFSettingDialbackUsername -- ask Palm why :-); might not be
  // used by NetLib.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldSMTPHost,		  1017, \
			  netPrefFieldTypeStr,	  0, 0, <1017:smtphst>, 14)

  // char*; presently not used.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldPOPHost,			  1018, \
			  netPrefFieldTypeStr,	  0, 0, <1018:pophst>, 13)

  // char*; presently not used.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldNewsServer,		  1019, \
			  netPrefFieldTypeStr,	  0, 0, <1019:newssrv>, 14)

  // char*; loginScript; scripts contain null terminated strings and end with
  // a double null; ***an empty script is represented with a single null (not
  // double null); used to set NetLib's netIFSettingLoginScript setting for
  // dial-up type of profiles.  Refer to Palm for login-script format.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldLoginScript,		  1020, \
			  netPrefFieldTypeZStrList,	  0, 0, <1020:script>, 13)

  // char*; connection name; obtained from Palm OS Connection Manager
  // via CncGetProfileList or CncGetProfileInfo -- but avoid the
  // "-Current-" setting, since the New Network panel doesn't use it any
  // longer.  netPrefRecFieldSvcMediumAlias overrides this setting.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldConnection,		  1021, \
			  netPrefFieldTypeStr,	  0, 0, <1021:connnam>, 14)

  // UInt8; non-zero=true, zero=false; corresponds to the dial prefix setting
  // in the Phone Setup dialog.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldDialPrefixEnable,  1022, \
			  netPrefFieldTypeUInt8,  0, 0, <1022:enprefx>, 14)

  // UInt8; non-zero=true, zero=false; corresponds to the Call Waiting
  // setting in the Phone Setup dialog.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldCallWaitingEnable, 1023, \
			  netPrefFieldTypeUInt8,  0, 0, <1023:encalwt>, 14)

  // UInt8; non-zero=true, zero=false; corresponds to the Calling Card
  // setting in the Phone Setup dialog.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldCallingCardEnable, 1024, \
			  netPrefFieldTypeUInt8,  0, 0, <1024:encalcd>, 14)

  // UInt8; non-zero=true, zero=false. Not used.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldPulseDialEnable,	  1025, \
			  netPrefFieldTypeUInt8,  0, 0, <1025:enpulse>, 14)

  // char*; phone number; corresponds to the Phone # field in the Phone
  // Setup dialog.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldPhoneNumber,		  1026, \
			  netPrefFieldTypeStr,	  0, 0, <1026:phone>, 12)

  // char*; corresponds to the Dial Prefix setting in the Phone Setup dialog.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldDialPrefix,		  1027, \
			  netPrefFieldTypeStr,	  0, 0, <1027:dlprefx>, 14)

  // char*; corresponds to the Call Waiting setting in the Phone Setup dialog.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldCallWaitingString, 1028, \
			  netPrefFieldTypeStr,	  0, 0, <1028:calwt>, 12)

  // char*; corresponds to the Calling Card setting in the Phone Setup dialog.
  NetPrefRecFieldInfoLegacy(	netPrefRecFieldCallingCardNumber, 1029, \
			  netPrefFieldTypeStr,	  0, 0, <1029:calcd>, 12)

  // END OF LEGACY FIELDS (for internal use only -- # of legacy
  // fields will likely grow in the future)
  NetPrefRecFieldInfoGrpEnd(	netPrefRecFieldLegacyEND)

  // ALWAYS KEEP THIS ENTRY AT THE END!!!
  NetPrefRecFieldInfoLAST(		netPrefRecFieldLAST)



// We undefine all required macros here for convenience
#undef NetPrefRecFieldInfoPreamble

#undef NetPrefRecFieldInfoFIRST

#undef NetPrefRecFieldInfoGrpStart

#undef NetPrefRecFieldInfoGrpEnd

#undef NetPrefRecFieldInfoExtended

#undef NetPrefRecFieldInfoLegacy

#undef NetPrefRecFieldInfoLAST

#endif
