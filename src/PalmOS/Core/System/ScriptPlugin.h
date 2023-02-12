/******************************************************************************
 *
 * Copyright (c) 1998-2003 PalmSource, Inc. All rights reserved.
 *
 * File: ScriptPlugin.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Include file for script plugin modules of the Network Pref Panel
 *		and the Net library. Note that you need to include the file
 *		<systemMgr.h> in your plugin before this file.
 *
 *****************************************************************************/

#ifndef __SCRIPTPLUGIN_H__
#define	__SCRIPTPLUGIN_H__

#include <PalmTypes.h>
#include <SystemMgr.h>

// Plugin Launch command codes
//
typedef enum {
	scptLaunchCmdDoNothing = sysAppLaunchCmdCustomBase, 
	scptLaunchCmdListCmds,
	scptLaunchCmdExecuteCmd
} ScriptPluginLaunchCodesEnum;

// Commands for the callback selector function
//
#define pluginNetLibDoNothing				0 // For debug purposes.
#define pluginNetLibReadBytes				1 // Receive X number of bytes. 
#define pluginNetLibWriteBytes				2 // Send X number of bytes.
#define pluginNetLibGetUserName				3 // Get the user name from the service profile.
#define pluginNetLibGetUserPwd				4 // Get the user password from the service profile.
#define pluginNetLibCheckCancelStatus		5 // Check the user cancel status.
#define pluginNetLibPromptUser				6 // Prompt the user for data and collect it.
#define pluginNetLibConnLog					7 // Write to the connection log.
#define pluginNetLibCallUIProc				8 // Call the plugin's UI function.
#define pluginNetLibGetSerLibRefNum			9 // Get the Serial library reference number.

		
		
// Plugin constants
//
#define pluginMaxCmdNameLen					15
#define pluginMaxModuleNameLen				15
#define pluginMaxNumOfCmds					10
#define pluginMaxLenTxtStringArg			63

typedef struct {
    Char  		commandName[pluginMaxCmdNameLen + 1];
    Boolean 	hasTxtStringArg;
	UInt8		reserved;					// explicitly account for 16-bit alignment padding
} PluginCmdType;

typedef PluginCmdType *PluginCmdPtr;

    
typedef struct {
	Char			pluginName[pluginMaxModuleNameLen + 1];	
	UInt16			numOfCommands;			
	PluginCmdType	command[pluginMaxNumOfCmds];								
} PluginInfoType;

typedef PluginInfoType *PluginInfoPtr;



// Plugin Execute structures

typedef Err (*ScriptPluginSelectorProcPtr) (void *handle, UInt16 command, void *dataBufferP, 
											UInt16 *sizeP, UInt16 *dataTimeoutP, void *procAddrP);


typedef struct {
     ScriptPluginSelectorProcPtr  selectorProcP;
} PluginCallbackProcType, *PluginCallbackProcPtr;


typedef struct {
    Char  					commandName[pluginMaxCmdNameLen + 1];
    Char					txtStringArg[pluginMaxLenTxtStringArg + 1];
    PluginCallbackProcPtr	procP;
	void *					handle;
} PluginExecCmdType, *PluginExecCmdPtr;	


#endif	//	__SCRIPTPLUGIN_H__
