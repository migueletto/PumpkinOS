/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup NETPREF
 */

/**
 * @file 	NetPrefLibrary.h
 * @version 1.0
 * @date 	06/24/2002
 *
 * @brief   This is the main header file for the NetPref Library.
 *
 * USAGE MODEL:
 * The model for using the NetPref library is that you "link" it
 * in when needed and "unlink" it as you're done -- i.e. we don't
 * leave it permanently installed as we do with some other libraries.
 * This helps us avoid some unpleasant HotSync issues (such as trying
 * to install over a protected database).  For examples of linking
 * and unlinking, grep for calls to  NetPrefUtilNetPrefLibLink and
 * NetPrefUtilNetPrefLibUnlink in NetworkPanel.c (Network panel src
 * folder).  NetPrefUtilNetPrefLibLink and NetPrefUtilNetPrefLibUnlink
 * are defined in Viewer\Libraries\NetPref\Public\Util\NetPrefUtils.c/h --
 * you may include the .c file with your code, or just copy and paste out
 * of those functions.
 *
 */

/*
 * @author   Vitaly Kruglikov
 *
 * History:
 *	  12-Dec-2001	vmk	  Created by Vitaly Kruglikov
 *	  24-Jun-2002	vmk	  Removed inclusion of private header file
 *						  OSServices.h.
 * <hr>
 */

#ifndef _NET_PREF_LIBRARY_H_
#define _NET_PREF_LIBRARY_H_


#include <Common/Libraries/NetPref/NetPrefLibTarget.h>
#include <Common/Libraries/NetPref/NetPrefLibTraps.h>
#include <Common/Libraries/NetPref/NetPrefLibErrors.h>
#include <68K/Libraries/NetPref/NetPrefLibTypes.h>



#ifdef BUILDING_NET_PREF_LIB_DISPATCH_TABLE
  #define NETPREF_LIB_TRAP(trapNum)
#else
  #define NETPREF_LIB_TRAP(trapNum) SYS_TRAP(trapNum)
#endif



#ifdef __cplusplus
extern "C" {
#endif

/** Standard library open routine.
 *
 *  @param refnum:	IN:  Library reference number
 *  @retval Err		Library error code
 */
extern Err NetPrefLibOpen (UInt16 refNum, struct NetPrefContextTypeTag** cxtPP)
	NETPREF_LIB_TRAP (sysLibTrapOpen);

/** Standard library close routine.
 *
 *  @param refnum:	IN:  Library reference number
 *  @retval Err		Library error code
 */
extern Err NetPrefLibClose (UInt16 refNum, struct NetPrefContextTypeTag* cxtP)
	NETPREF_LIB_TRAP (sysLibTrapClose);

/** Standard library sleep routine.
 *
 *  @param refnum:	IN:  Library reference number
 *  @retval Err		Library error code
 */
extern Err NetPrefLibSleep (UInt16 refNum)
	NETPREF_LIB_TRAP (sysLibTrapSleep);

/** Standard library wake routine.
 *
 *  @param refnum:	IN:  Library reference number
 *  @retval Err		Library error code
 */
extern Err NetPrefLibWake (UInt16 refNum)
	NETPREF_LIB_TRAP (sysLibTrapWake);

/** Get Library API Version number.
 *
 *  @param refnum:          IN:  Library reference number
 *  @param majorVerP:       OUT: Major version number
 *  @param minorVerP:       OUT: Minor version number
 *  @param bugFixVerP:      OUT: Bug fix version number
 *  @retval Err             NetPref Library error code
 */
extern Err
NetPrefLibVersionGet (UInt16 refNum, UInt32* majorVerP, UInt32* minorVerP,
					  UInt32* bugFixVerP)
	NETPREF_LIB_TRAP (netPrefLibTrapVersionGet);


// Retrieve the count of records in the NetPref database
extern Err
NetPrefRecCountGet (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
					UInt16* countP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecCountGet);


// Get record ID given the 0-based record index, which must be
//  in range of 0..NetPrefRecCountGet()-1.
//  The record's index may change following the NetPrefRecSave opeation.
extern Err
NetPrefRecIDGetByIndex (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
						UInt16 recIndex, UInt32* recIDP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecIDGetByIndex);

// Get record index given the unique record ID, which must be
//  non-zero.
//  The record's index may change following the NetPrefRecSave opeation.
extern Err
NetPrefRecIndexGetByID (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
						UInt32 recID, UInt16* recIndexP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecIndexGetByID);


// Create a new record object.  Pass a non-zero dupRecID to duplicate
//  an existing record.
//  The caller is responsible for calling NetPrefRecRelease() to
//  dispose of the temporary resources associated with the loaded record
extern Err
NetPrefRecNew (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
			   UInt32 dupRecID, struct NetPrefRecordTypeTag** recPP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecNew);


// Load an existing record into memory; the caller is responsible for
//  calling NetPrefRecRelease() to dispose of the temporary resources
//  associated with the loaded record.
extern Err
NetPrefRecLoad (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
				UInt32 recID, struct NetPrefRecordTypeTag** recPP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecLoad);


// Call this to check if there was an error from application of a binding
//  (such as CCSM) to the record.
extern Err
NetPrefRecBindingErrorGet (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
						   struct NetPrefRecordTypeTag* recP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecBindingErrorGet);


// Dispose of the temporary resources associated with a record that
//  was created or loaded via NetPrefRecNew or NetPrefRecLoad.
//  It's the library client's (application's) responsibility to
//  call this function to free up the memory.  Resets *recPP to NULL.
extern Err
NetPrefRecRelease (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
				   struct NetPrefRecordTypeTag** recPP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecRelease);


// Save the record to the database, moving it to a new sorted position
//  if necessary.  For existing records, if any fields
//  have been changed, you must call NetPrefRecSave in order to
//  get an accurate record index.
extern Err
NetPrefRecSave (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
				struct NetPrefRecordTypeTag* recP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecSave);


// Check if this record object is "attached" to the database.  A record
//  that has been created via NetPrefRecObjAlloc and never saved is
//  not "attached" to the database.
extern Boolean
NetPrefRecIsAttached (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
					  struct NetPrefRecordTypeTag* recP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecIsAttached);



// Allocate a "blank slate" record object.  This record will not be "attached"
//  to the database until saved via NetPrefRecSave.  Intended for use with
//  NetPrefRecObjCopy, and by internal code.
extern Err
NetPrefRecObjAlloc (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
					struct NetPrefRecordTypeTag** recPP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecObjAlloc);

// Copy the contents of the source record object to the destination
//  record object.  This is useful for implementing revert functionality.
extern Err
NetPrefRecObjCopy (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
				   struct NetPrefRecordTypeTag* srcRecP,
				   struct NetPrefRecordTypeTag* dstRecP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecObjCopy);


// Get the unique record ID of the record's entry in the database.
//
// WARNING:
//  It is an error to call NetPrefRecIDGet on records that have not been
//  attached to the database -- see NetPrefRecIsAttached.
//
extern Err
NetPrefRecIDGet (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
				 struct NetPrefRecordTypeTag* recP, UInt32* recIDP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecIDGet);

// Get the 0-based index of the record's entry in the database.
//  The record's index may change following the NetPrefRecSave opeation.
//
// WARNING:
//  It is an error to call NetPrefRecIndexGet on records that have not been
//  attached to the database -- see NetPrefRecIsAttached.
extern Err
NetPrefRecIndexGet (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
					struct NetPrefRecordTypeTag* recP, UInt16* recIndexP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecIndexGet);


// Delete the given record from the database; if the record is bound
//  to another, such as that stored in non-volatile memory of the radio
//  module, will delete the primary record as well.  If the client
//	has a memory object representing the record being deleted, the client
//  is still responsible for releasing it via NetPrefRecRelease().
//
extern Err
NetPrefRecDelete (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
				  UInt32 recID)
	NETPREF_LIB_TRAP (netPrefLibTrapRecDelete);

// Get a pointer to a given record field; upon successful return,
//  the value of the field is to be interpreted as follows:
//
//   NULL	  -- field value is not assigned
//   non-NULL -- field value has been assigned, the
//			      value must be treated as read-only!
// Field view depends on the Medium and Binding of the record
//
extern Err
NetPrefRecFieldGet (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
				   struct NetPrefRecordTypeTag* recP,
				   NetPrefRecFieldEnum recFieldID, void** fieldValuePP,
				   UInt32* fieldLenP, NetPrefRecFieldViewType* viewFlagsP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecFieldGet);


// Get the attributes of a field
extern Err
NetPrefRecFieldAttrsGet (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
						 struct NetPrefRecordTypeTag* recP,
						 NetPrefRecFieldEnum recFieldID,
						 NetPrefRecFieldAttrType* fieldAttrsP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecFieldAttrsGet);


// NetPrefRecFieldSet:
// Set the value of the field.  The library will duplicate the value
//  passed in fieldValueP. Set fieldValueP to NULL to delete the
//  field's data. NOTE: setting a field invalidates any cached
//  pointer that you may already have to the field's value.
//
//  NOTES:
//	  1) Field must be in record's field-set before you can set its
//		 data or view flags.
//
//	  2) String field values MUST be zero-terminated and the fieldLen
//		 MUST include the zero-terminator.
//
//	  3) If a change results, the record object will be marked as dirty
//
extern Err
NetPrefRecFieldSet (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
					struct NetPrefRecordTypeTag* recP,
					NetPrefRecFieldEnum recFieldID, void* fieldValueP,
					UInt32 fieldLen)
	NETPREF_LIB_TRAP (netPrefLibTrapRecFieldSet);


// Set the view flags of the field.  If a change results, the record
//  object will be marked as dirty.
//
//  NOTES:
//	  1) Field must be in record's field-set before you can set its
//		 data or view flags.
//
//	  2) If a change results, the record object will be marked as dirty
//
extern Err
NetPrefRecFieldViewSet (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
						struct NetPrefRecordTypeTag* recP,
						NetPrefRecFieldEnum recFieldID,
						NetPrefRecFieldViewType flagsToClear,
						NetPrefRecFieldViewType flagsToSet)
	NETPREF_LIB_TRAP (netPrefLibTrapRecFieldViewSet);



// NetPrefRecReadOnlyOverrideStart/End: Increment/decrement the
// record object's readOnlyOverrideCount variable.  The two
// function calls must be balanced -- i.e., ...End must be called
// exactly as many times as ...Start regardless of other events,
// such as errors. If greater than zero, NetPrefRecFieldSet
// will allow the field to be set regardless of the record's read-only
// status.  This permits internal record "loaders" to initilize record
// fields regardless of the read-only status of the fields being loaded.
extern void
NetPrefRecReadOnlyOverrideStart (UInt16 refNum,
								 struct NetPrefContextTypeTag* cxtP,
								 struct NetPrefRecordTypeTag* recP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecReadOnlyOverrideStart);

extern void
NetPrefRecReadOnlyOverrideEnd (UInt16 refNum,
							   struct NetPrefContextTypeTag* cxtP,
							   struct NetPrefRecordTypeTag* recP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecReadOnlyOverrideEnd);


// Define the field set associated with the record; fields that
//  are not in the new field will be removed from the record
//  object.  For fields in the new field set, existing data will
//  be left alone.
//
//  You may pass a pointer to new
//  NetPrefRecFieldViewType in initialViewFlagsP to set those
//  view flags in all the fields in the new field set (or pass
//  NULL to ignore this parameter and leave field view flags
//  at their old values).
//
//  To remove all fields from the field set, set newNumFields to 0
//  and recFieldIDArrayP to NULL;
//
extern Err
NetPrefRecFieldSetDefine (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
						  struct NetPrefRecordTypeTag* recP,
						  NetPrefRecFieldEnum* recFieldIDArrayP,
						  UInt16 newNumFields,
						  NetPrefRecFieldViewType* initialViewFlagsP,
						  NetPrefFieldSetDefineOptionsType options)
	NETPREF_LIB_TRAP (netPrefLibTrapRecFieldSetDefine);



// Add a field to the Field-set of a given record and, optionally, set its
//  view flags (set initialViewFlagsP to NULL to leave the field's view flags
//  alone).
//
// NOTE: If the field was already part of the record's field-set, the field's
//  data will be preserved.  However, the field's view flags will be overwritten
//  with those supplied via initialViewFlagsP if initialViewFlagsP is not NULL.
//
extern Err
NetPrefRecFieldAddToSet (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
						 struct NetPrefRecordTypeTag* recP,
						 NetPrefRecFieldEnum recFieldID,
						 NetPrefRecFieldViewType* initialViewFlagsP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecFieldAddToSet);


// Define a "standard" field set based on Service Medium value -- basically,
//  this is what the New Network Preference panel uses when it creates new
//  services.
//
//  You may pass a pointer to new
//  NetPrefRecFieldViewType in initialViewFlagsP to set the
//  view flags of all the fields in the new field set (or pass
//  NULL for default flags).
extern Err
NetPrefRecFieldSetDefineStd (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
							 struct NetPrefRecordTypeTag* recP,
							 NetPrefSvcMediumEnum svcMedium,
							 NetPrefRecFieldViewType* initialViewFlagsP,
							 NetPrefFieldSetDefineOptionsType options)
	NETPREF_LIB_TRAP (netPrefLibTrapRecFieldSetDefineStd);


// Get the current field set of the record object.  The array
//  recFieldIDArrayP MUST have enough entries (numEntriesAvailable)
//  to recieve the field ID's of all fields in the field set.
//  The actual count of fields in the field set is returned in
//  *numFieldsInSetP.
//  To simply get the count of fields in the field set, without
//  returning the actual field ID's, set numEntriesAvailable to 0
//  and recFieldIDArrayP to NULL;
extern Err
NetPrefRecFieldSetGet (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
					   struct NetPrefRecordTypeTag* recP,
					   NetPrefRecFieldEnum* recFieldIDArrayP,
					   UInt16 numEntriesAvailable, UInt16* numFieldsInSetP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecFieldSetGet);

// Check if the record is dirty (has been modified by the client)
extern Err
NetPrefRecIsDirty (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
					struct NetPrefRecordTypeTag* recP,
					UInt8* isDirtyP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecIsDirty);


// Reset all of the record's "dirty" flags.  NOTE: NetPrefRecSave will not
//  save a record that is not marked as "dirty"
extern void
NetPrefRecDirtyFlagsReset (UInt16 refNum,
						   struct NetPrefContextTypeTag* cxtP,
						   struct NetPrefRecordTypeTag* recP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecDirtyFlagsReset);


// Mark a record as "dirty" so it would be saved by NetPrefRecSave.
extern void
NetPrefRecMarkDirty (UInt16 refNum,
					 struct NetPrefContextTypeTag* cxtP,
					 struct NetPrefRecordTypeTag* recP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecMarkDirty);


// Get the default service ID for the given target
extern Err
NetPrefDefaultTargetGet (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
						 NetPrefSvcTargetEnum svcTarget, UInt32* recIDP)
	NETPREF_LIB_TRAP (netPrefLibTrapDefaultTargetGet);


// Set the given service as the default service for the given target type;
//  Set recID to 0 (zero) to clear the default for that target type.
//(WAP, Internet, all, etc.)
extern Err
NetPrefDefaultTargetSet (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
						 NetPrefSvcTargetEnum svcTarget, UInt32 recID)
	NETPREF_LIB_TRAP (netPrefLibTrapDefaultTargetSet);


// Update the NetPref database from the Non-Volatile storage of the radio,
//  if any.  This should be called by the IOTA application after provisioning,
//  as well as when the wireless mode is turned on.  This function is optimized
//  to quickly determine if synchronizatino is necessary, and skip it if not.
//  Set "force" to true to skip optimization and force synchronization.
extern Err
NetPrefUpdateFromRadioNV (UInt16 refNum, struct NetPrefContextTypeTag* cxtP,
						  Boolean force)
	NETPREF_LIB_TRAP (netPrefLibTrapUpdateFromRadioNV);


// Called by the Network prefs. panel when it receives the
//  sysAppLaunchCmdSyncNotify action code from HotSync or IR stack.
//  This happens whenever the network preferences database NetworkDB
//  or the Network panel are HotSynced or IR-beamed to the device.
//
// NOTE: this may be called from a background task with a rather
//  small runtime stack (as in the case of HotSync).
extern Err
NetPrefHandleHotSyncNotify (UInt16 refNum,
							struct NetPrefContextTypeTag* cxtP,
							UInt16 cmd, void* cmdPBP)
	NETPREF_LIB_TRAP (netPrefLibTrapHandleHotSyncNotify);


/***********************************************************************
 *
 * FUNCTION:     NetPrefRecLongFieldGet
 *
 * DESCRIPTION:  Get a UInt32 field value from a record
 *
 * PARAMETERS:
 *	  refNum		  IN  NetPrefLib reference number (from SysLibFind
 *						   or SysLibLoad)
 *	  cxtP			  IN  Open NetPref library context as returned by
 *						   a successful call to NetPrefLibOpen
 *	  recP -
 *	  recFieldID	-
 *	  resultP	- value, set to zero if not available
 *	  viewFlagsP - OUT field's view flags -- OPTIONAL - set to
 *								  NULL to ignore
 *
 *	CALLED BY:
 *
 * RETURNED:     true if available (set), false if not
 ***********************************************************************/
extern Boolean
NetPrefRecLongFieldGet (UInt16 refNum,
						struct NetPrefContextTypeTag* cxtP,
						struct NetPrefRecordTypeTag* recP,
						NetPrefRecFieldEnum recFieldID,
						UInt32* resultP,
						NetPrefRecFieldViewType* viewFlagsP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecLongFieldGet);


/***********************************************************************
 *
 * FUNCTION:     NetPrefRecShortFieldGet
 *
 * DESCRIPTION:  Get a UInt16 field value from a record
 *
 * PARAMETERS:
 *	  refNum		  IN  NetPrefLib reference number (from SysLibFind
 *						   or SysLibLoad)
 *	  cxtP			  IN  Open NetPref library context as returned by
 *						   a successful call to NetPrefLibOpen
 *	  recP -
 *	  recFieldID	-
 *	  resultP	- value, set to zero if not available
 *	  viewFlagsP - [OPTIONAL] view flags
 *
 *	CALLED BY:
 *
 * RETURNED:     true if available (set), false if not
 ***********************************************************************/
extern Boolean
NetPrefRecShortFieldGet (UInt16 refNum,
						struct NetPrefContextTypeTag* cxtP,
						struct NetPrefRecordTypeTag* recP,
						NetPrefRecFieldEnum recFieldID,
						UInt16* resultP,
						NetPrefRecFieldViewType* viewFlagsP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecShortFieldGet);


/***********************************************************************
 *
 * FUNCTION:     NetPrefRecByteFieldGet
 *
 * DESCRIPTION:  Get a UInt8 field value from a record
 *
 * PARAMETERS:
 *	  refNum		  IN  NetPrefLib reference number (from SysLibFind
 *						   or SysLibLoad)
 *	  cxtP			  IN  Open NetPref library context as returned by
 *						   a successful call to NetPrefLibOpen
 *	  recP -
 *	  recFieldID	-
 *	  resultP	- value, set to zero if not available
 *
 *	CALLED BY:
 *
 * RETURNED:     true if available (set), false if not
 ***********************************************************************/
extern Boolean
NetPrefRecByteFieldGet (UInt16 refNum,
						struct NetPrefContextTypeTag* cxtP,
						struct NetPrefRecordTypeTag* recP,
						NetPrefRecFieldEnum recFieldID,
						UInt8* resultP,
						NetPrefRecFieldViewType* viewFlagsP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecByteFieldGet);


/**************************************************************
 *	Function:	  NetPrefRecFieldViewGet
 *
 *	Summary:
 *	  Returns field view flags for a given field of the given record.
 *
 *	Parameters:
 *	  refNum		  IN  NetPrefLib reference number (from SysLibFind
 *						   or SysLibLoad)
 *	  cxtP			  IN  Open NetPref library context as returned by
 *						   a successful call to NetPrefLibOpen
 *	  recP			  IN  NetPref record object
 *	  recFieldID	  IN  NetPref record field ID
 *
 *	Returns:
 *	  NetPrefRecFieldViewType
 ****************************************************************/
extern NetPrefRecFieldViewType
NetPrefRecFieldViewGet (UInt16 refNum,
						 struct NetPrefContextTypeTag* cxtP,
						 struct NetPrefRecordTypeTag* recP,
						 NetPrefRecFieldEnum recFieldID)
	NETPREF_LIB_TRAP (netPrefLibTrapRecFieldViewGet);


/***********************************************************************
 *
 * FUNCTION:     NetPrefRecLongFieldSetAsFlags
 *
 * DESCRIPTION:  Set a 32-bit field data value as flags. The clear
 *				 operation is performed before the set operation.  To
 *				 skip either operation, set the corresponding parameter to 0.
 *
 * PARAMETERS:
 *	  refNum		  IN  NetPrefLib reference number (from SysLibFind
 *						   or SysLibLoad)
 *	  cxtP			  IN  Open NetPref library context as returned by
 *						   a successful call to NetPrefLibOpen
 *	  recP -
 *	  recFieldID	-
 *	  resultP	- value, set to zero if not available
 *	  flagsToClear	  IN flags to clear, or 0 to skip
 *	  flagsToSet	  IN flags to set, or 0 to skip
 *
 *	CALLED BY:
 *
 * RETURNED:     0 on success
 ***********************************************************************/
extern Err
NetPrefRecLongFieldSetAsFlags (UInt16 refNum,
							   struct NetPrefContextTypeTag* cxtP,
							   struct NetPrefRecordTypeTag* recP,
							   NetPrefRecFieldEnum recFieldID,
							   UInt32 flagsToClear, UInt32 flagsToSet)
	NETPREF_LIB_TRAP (netPrefLibTrapRecLongFieldSetAsFlags);


/***************************************************************
 *	Function:	  NetPrefRecFieldSetFromBinHandle
 *
 *	Summary:	  Set a record object binary field from a PalmOS
 *				  memory handle.
 *
 *	Parameters:
 *	  refNum		  IN  NetPrefLib reference number (from SysLibFind
 *						   or SysLibLoad)
 *	  cxtP			  IN  Open NetPref library context as returned by
 *						   a successful call to NetPrefLibOpen
 *	  recP			  IN/OUT  record object
 *	  recFieldID	  IN	  field ID -- NetPrefRecFieldEnum
 *	  dataH			  IN	  data handle
 *
 *	Returns:
 *	  0 on success
 ****************************************************************/
extern Err
NetPrefRecFieldSetFromBinHandle (UInt16 refNum,
								struct NetPrefContextTypeTag* cxtP,
								struct NetPrefRecordTypeTag* recP,
								NetPrefRecFieldEnum recFieldID,
								MemHandle dataH)
	NETPREF_LIB_TRAP (netPrefLibTrapRecFieldSetFromBinHandle);


/***************************************************************
 *	Function:	  NetPrefRecFieldSetFromStrHandle
 *
 *	Summary:	  Set a record object string field from a PalmOS memory handle.
 *
 *	Parameters:
 *	  refNum		  IN  NetPrefLib reference number (from SysLibFind
 *						   or SysLibLoad)
 *	  cxtP			  IN  Open NetPref library context as returned by
 *						   a successful call to NetPrefLibOpen
 *	  recP			  IN/OUT  record object
 *	  recFieldID	  IN	  field ID -- NetPrefRecFieldEnum
 *	  dataH			  IN	  string handle -- zero-terminated; may be NULL
 *
 *	Returns:
 *	  0 on success
 ****************************************************************/
extern Err
NetPrefRecFieldSetFromStrHandle (UInt16 refNum,
								struct NetPrefContextTypeTag* cxtP,
								struct NetPrefRecordTypeTag* recP,
								NetPrefRecFieldEnum recFieldID,
								MemHandle dataH)
	NETPREF_LIB_TRAP (netPrefLibTrapRecFieldSetFromStrHandle);


/***************************************************************
 *	Function:	  NetPrefRecFieldSetFromStrPtr
 *
 *	Summary:	  Set a record object string field from a PalmOS memory handle.
 *
 *	Parameters:
 *	  refNum		  IN  NetPrefLib reference number (from SysLibFind
 *						   or SysLibLoad)
 *	  cxtP			  IN  Open NetPref library context as returned by
 *						   a successful call to NetPrefLibOpen
 *	  recP			  IN/OUT  record object
 *	  recFieldID	  IN	  field ID -- NetPrefRecFieldEnum
 *	  strP			  IN	  string pointer -- zero-terminated; may be NULL
 *
 *	Returns:
 *	  0 on success
 ****************************************************************/
extern Err
NetPrefRecFieldSetFromStrPtr (UInt16 refNum,
							  struct NetPrefContextTypeTag* cxtP,
							  struct NetPrefRecordTypeTag* recP,
							  NetPrefRecFieldEnum recFieldID,
							  char* strP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecFieldSetFromStrPtr);



/***************************************************************
 *	Function:	  NetPrefRecConnectionInfoGet
 *
 *	Summary:	  Get the port ID and modem status of the connection
 *				  referenced by the record object.
 *
 *	Parameters:
 *	  refNum		  IN  NetPrefLib reference number (from SysLibFind
 *						   or SysLibLoad)
 *	  cxtP			  IN  Open NetPref library context as returned by
 *						   a successful call to NetPrefLibOpen
 *	  recP			  IN  record object
 *
 *	Returns:
 *	  true if got information, false if connection is not in the Connection
 *	  Manager database
 ****************************************************************/
extern Boolean
NetPrefRecConnectionInfoGet (UInt16 refNum,
							struct NetPrefContextTypeTag* cxtP,
							struct NetPrefRecordTypeTag* recP,
							UInt32* portCreatorIDP, Boolean* isModemP)
	NETPREF_LIB_TRAP (netPrefLibTrapRecConnectionInfoGet);


/***************************************************************
 *	Function:	  NetPrefRecMediumDerive
 *
 *	Summary:	  Determine the record's "medium" (NetPrefSvcMediumEnum).
 *				  Determine based on Connection name and/or
 *				  the value of the netPrefRecFieldMedium.
 *				  The precedence goes to netPrefRecFieldMedium.
 *
 *	Parameters:
 *	  refNum		  IN  NetPrefLib reference number (from SysLibFind
 *						   or SysLibLoad)
 *	  cxtP			  IN  Open NetPref library context as returned by
 *						   a successful call to NetPrefLibOpen
 *	  recP			  IN  record object
 *
 *	Returns:
 *	  NetPrefSvcMediumEnum
 ****************************************************************/
extern NetPrefSvcMediumEnum
NetPrefRecMediumDerive (UInt16 refNum,
						 struct NetPrefContextTypeTag* cxtP,
						 struct NetPrefRecordTypeTag* recP,
						 UInt32 defWirelessDriverID)
	NETPREF_LIB_TRAP (netPrefLibTrapRecMediumDerive);


/***********************************************************************
 *
 * FUNCTION:	NetPrefRecIPAddrFieldSet
 *
 * DESCRIPTION:	Set the value of an IP address field in the record object.
 *				If the IP address is 0, saves a value of 0 length to
 *				optimize memory access performance
 *
 * PARAMETERS:
 *	  refNum		  IN  NetPrefLib reference number (from SysLibFind
 *						   or SysLibLoad)
 *	  cxtP			  IN  Open NetPref library context as returned by
 *						   a successful call to NetPrefLibOpen
 *	  recP  - record object
 *	  recFieldID - record object field ID to set
 *	  ipAddr - IP address
 *
 * RETURNED:	0 on success.
 ***********************************************************************/
extern Err
NetPrefRecIPAddrFieldSet (UInt16 refNum,
						  struct NetPrefContextTypeTag* cxtP,
						  struct NetPrefRecordTypeTag* recP,
						  NetPrefRecFieldEnum recFieldID, UInt32 ipAddr)
	NETPREF_LIB_TRAP (netPrefLibTrapRecIPAddrFieldSet);


/***********************************************************************
 *
 * FUNCTION:	  NetPrefUtilZStringListSizeGet
 *
 * DESCRIPTION:	  Return the length of a zero-terminated string list.  A
 *				  zero-terminated string list consists of zero-terminated
 *				  strings and ends with an empty string.  The size includes.
 *				  all of the zero-terminators, including the list terminator.
 *
 * PARAMETERS:
 *	  refNum		  IN  NetPrefLib reference number (from SysLibFind
 *						   or SysLibLoad)
 *	  cxtP			  IN  Open NetPref library context as returned by
 *						   a successful call to NetPrefLibOpen
 *	  zStrListP - pointer to the zero-terminated sting list
 *
 * RETURNED:
 *	  The size of the list, including all of the zero-terminators and
 *	  the list terminator.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		2/11/02		Created based on Palm's SetPrefStr from
 *								Network Panel.
 ***********************************************************************/
extern UInt32
NetPrefUtilZStringListSizeGet (UInt16 refNum,
							   struct NetPrefContextTypeTag* cxtP,
							   char* zStrListP)
	NETPREF_LIB_TRAP (netPrefLibTrapUtilZStringListSizeGet);

#define NetPrefUtilLoginScriptLen(refNum, cxtP, scriptP) \
			  	NetPrefUtilZStringListSizeGet ((refNum), (cxtP), \
											   (scriptP))


/***********************************************************************
 *
 * FUNCTION:     NetPrefRecPhoneStringCompose
 *
 * DESCRIPTION:	Compose a phone string from the record object.
 *				contcatenate the dial prefix, disable
 *				call waiting, calling card number, and the phone
 *				number
 *
 * PARAMETERS:
 *	  refNum		  IN  NetPrefLib reference number (from SysLibFind
 *						   or SysLibLoad)
 *	  cxtP			  IN  Open NetPref library context as returned by
 *						   a successful call to NetPrefLibOpen
 *	  recP			  IN  record object
 *	  destP			  OUT string buffer to hold the complete number
 *						   maxLen	  IN  the allocated length of destP -1
 *
 * RETURNED:	nothing
 ***********************************************************************/
extern void
NetPrefRecPhoneStringCompose (UInt16 refNum,
							  struct NetPrefContextTypeTag* cxtP,
							  struct NetPrefRecordTypeTag* recP,
							  Char* destP, UInt16 maxLen)
	NETPREF_LIB_TRAP (netPrefLibTrapRecPhoneStringCompose);


/***********************************************************************
 *
 * Function:	NetPrefUtilPlatformIDGet
 *
 * Summary:		Determine on which platform we are running -- CDMA or GSM
 *
 * Parameters:
 *	  nothing
 *
 * Returns:
 *	  refNum		  IN  NetPrefLib reference number (from SysLibFind
 *						   or SysLibLoad)
 *	  cxtP			  IN  Open NetPref library context as returned by
 *						   a successful call to NetPrefLibOpen -- MAY BE NULL!
 *    NetPrefPlatformEnum or netPrefPlatformUnknown if could not
 *	  identify
 ***********************************************************************/
extern NetPrefPlatformEnum
NetPrefUtilPlatformIDGet (UInt16 refNum, struct NetPrefContextTypeTag* cxtP)
	NETPREF_LIB_TRAP (netPrefLibTrapUtilPlatformIDGet);


/***********************************************************************
 *
 * Function:	NetPrefUtilDefWirelessDriverIDGet
 *
 * Summary:		Get the default wireless driver ID for the given platform
 *
 * Parameters:
 *	  refNum		  IN  NetPrefLib reference number (from SysLibFind
 *						   or SysLibLoad)
 *	  cxtP			  IN  Open NetPref library context as returned by
 *						   a successful call to NetPrefLibOpen -- MAY BE NULL!
 *	  platform		  IN	platform ID (from NetPrefUtilPlatformIDGet())
 *
 * Returns:
 *    Wireless driver ID (creator) or 0 if platform is not wireless
 ***********************************************************************/
extern UInt32
NetPrefUtilDefWirelessDriverIDGet (UInt16 refNum,
								   struct NetPrefContextTypeTag* cxtP,
								   NetPrefPlatformEnum platform)
	NETPREF_LIB_TRAP (netPrefLibTrapUtilDefWirelessDriverIDGet);


#ifdef __cplusplus
}
#endif


#endif // _NET_PREF_LIBRARY_H_
