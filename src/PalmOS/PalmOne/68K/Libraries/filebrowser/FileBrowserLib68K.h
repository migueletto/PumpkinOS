/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup FileBrowser
 */

/**
 * @file 	FileBrowserLib68K.h
 * @version 1.0
 * @brief This file contains the FileBrowser APIs
 *
 */



#ifndef __FILE_BROWSER_LIB_68K_H__
#define __FILE_BROWSER_LIB_68K_H__


/***********************************************************************
 *
 *   Functions
 *
 ***********************************************************************/

#ifdef NO_FILE_BROWSER_TRAPS
	#define FILE_BROWSER_LIB_TRAP(trapNum)
#else
	#define FILE_BROWSER_LIB_TRAP(trapNum)	SYS_TRAP(trapNum)
#endif

/// Standard File Browser library open function
///
/// @param refNum: IN: library reference number
/// @retval Err Error code
Err FileBrowserLibOpen(UInt16 refNum)
	FILE_BROWSER_LIB_TRAP(sysLibTrapOpen);
	// Call this before any other entry points.

/// Standard File Browser library close function
///
/// @param refNum: IN: library reference number
/// @retval Err Error code
Err FileBrowserLibClose(UInt16 refNum)
	FILE_BROWSER_LIB_TRAP(sysLibTrapClose);
	// Call this when you're done.

/// Standard File Browser library sleep function
///
/// @param refNum: IN: library reference number
/// @retval Err Error code
Err FileBrowserLibSleep(UInt16 refNum)
	FILE_BROWSER_LIB_TRAP(sysLibTrapSleep);
	// Called automatically when the device turns off.

/// Standard File Browser library wake function
///
/// @param refNum: IN: library reference number
/// @retval Err Error code
Err FileBrowserLibWake(UInt16 refNum)
	FILE_BROWSER_LIB_TRAP(sysLibTrapWake);
	// Called automatically when the device turns on.

/// Entry point to handle an event
///
/// @param refNum: IN: library reference number
/// @param eventP: IN: the event to handle
/// @retval Boolean True if the event was handled and should not be passed
///                 to a higher level handler.
Boolean FileBrowserLibHandleEvent(UInt16 refNum, EventType *eventP)
	FILE_BROWSER_LIB_TRAP(exgLibTrapHandleEvent);

/// Entry point to establish an exchange connection
///
/// @param refNum:	IN: library reference number
/// @param socketP:	IN: the exchange socket
/// @retval Err Error code.
Err FileBrowserLibConnect(UInt16 refNum, ExgSocketType *socketP)
	FILE_BROWSER_LIB_TRAP(exgLibTrapConnect);

/// Entry point to accept an incoming connection
///
/// @param refNum:	IN: library reference number
/// @param socketP: IN: the exchange socket
/// @retval Err Error code.
Err FileBrowserLibAccept(UInt16 refNum, ExgSocketType *socketP)
	FILE_BROWSER_LIB_TRAP(exgLibTrapAccept);

/// Entry point to disconnect an outgoing or incoming
/// connection and clean up afterwards.
///
/// @param refNum:	IN: library reference number
/// @param socketP:	IN: the exchange socket
/// @param err:		IN: OS error code to report to user
/// @retval Err Error code.
Err FileBrowserLibDisconnect(UInt16 refNum, ExgSocketType *socketP,
	Err err)
	FILE_BROWSER_LIB_TRAP(exgLibTrapDisconnect);

/// Entry point to start a Put operation. Creates a
/// connection if one doesn't already exist.
///
/// @param refNum:	IN: library reference number
/// @param socketP:	IN: the exchange socket
/// @retval Err Error code.
Err FileBrowserLibPut(UInt16 refNum, ExgSocketType *socketP)
	FILE_BROWSER_LIB_TRAP(exgLibTrapPut);

/// Entry point to start a Get operation
///
/// @param refNum:	IN: library reference number
/// @param socketP: IN: the exchange socket
/// @retval Err Error code.
Err FileBrowserLibGet(UInt16 refNum, ExgSocketType *socketP)
	FILE_BROWSER_LIB_TRAP(exgLibTrapGet);

/// Entry point to send data for an object
///
/// @param refNum:	IN: library reference number
/// @param socketP:	IN: the exchange socket
/// @param bufP:	IN: the buffer containing data to send
/// @param bufLen:	IN: bytes to send
/// @param errP:	OUT: passed back error code
/// @retval UInt32 number of bytes sent.
UInt32 FileBrowserLibSend(UInt16 refNum, ExgSocketType *socketP,
	const void *bufP, UInt32 bufLen, Err *errP)
	FILE_BROWSER_LIB_TRAP(exgLibTrapSend);

/// Entry point to receive data for an object
///
/// @param refNum:	IN:  the library reference number
/// @param socketP:	IN:  the exchange socket
/// @param bufP:	OUT: buffer to store received data
/// @param bufSize:	IN:  size of buffer
/// @param errP:	OUT: passed back error code
/// @retval UInt32 number of bytes received.
UInt32 FileBrowserLibReceive(UInt16 refNum, ExgSocketType *socketP,
	void *bufP, UInt32 bufSize, Err *errP)
	FILE_BROWSER_LIB_TRAP(exgLibTrapReceive);

/// Entry point to execute various commands.
///
/// @param refNum:		IN: the library reference number
/// @param op:			IN: command to execute
/// @param valueP:		IN,OUT: buffer with incoming or outgoing data
/// @param valueLenP:	IN,OUT: size of buffer and size of result (not used for some operations)
/// @retval Err Error code.
Err FileBrowserLibControl(UInt16 refNum, UInt16 op, void *valueP,
	UInt16 *valueLenP)
	FILE_BROWSER_LIB_TRAP(exgLibTrapControl);

/// Entry point to do a Request operation (Unimplemented)
///
/// @param refNum:	IN: the library reference number
/// @param socketP:	IN: the exchange socket
/// @retval Err Error code.
Err FileBrowserLibRequest(UInt16 refNum, ExgSocketType *socketP)
	FILE_BROWSER_LIB_TRAP(exgLibTrapRequest);

/// Pop up an Open dialog
///
/// @param refNum:			IN: library reference number
/// @param volRefNumP:		IN: the default volume and the result
/// @param path:			IN: the default path and the result (must be
///                     	    at least kFileBrowserLibPathBufferSize bytes).
/// @param numExtensions:	IN: number of extensions to display
/// @param extensions:		IN: compatible extensions to display
/// @param fileType:		IN: optional MIME type or extension to get
///                			    default directory on each volume
/// @param title:			IN: open dialog title
/// @param flags:			IN: any combo of kOpenFlag* flags
/// @retval Boolean True if a file is selected
Boolean FileBrowserLibShowOpenDialog(UInt16 refNum,
	UInt16 *volRefNumP, Char *path, UInt16 numExtensions,
	const Char **extensions, const Char *fileType, const Char *title,
	UInt32 flags)
	FILE_BROWSER_LIB_TRAP(kFileBrowserLibTrapShowOpenDialog);

/// Pop up a Save dialog
///
/// @param refNum:			IN: library reference number
/// @param volRefNumP:		IN: the default volume and the result
/// @param path:			IN: the default path and the result (must be at least
///            				    kFileBrowserLibPathBufferSize bytes)
/// @param numExtensions:	IN: number of extensions to display
/// @param extensions:		IN: valid extensions to display
/// @param fileType:		IN: optional MIME type or extension to get
///                 		    default directory on each volume
/// @param title:			IN: open dialog title
/// @param flags:			IN: any combo of kSaveAsFlag* flags
/// @retval Boolean True if a file is saved
Boolean FileBrowserLibShowSaveAsDialog(UInt16 refNum, UInt16 *volRefNumP,
	Char *path, UInt16 numExtensions, const Char **extensions,
	const Char *defaultExtension, const Char *fileType,
	const Char *title, UInt32 flags)
	FILE_BROWSER_LIB_TRAP(kFileBrowserLibTrapShowSaveAsDialog);

/// Entry point to parse a URL (file:) received in a
///	sysAppLaunchCmdExgReceiveData launch resulting from the
///	open dialog.  Pass NULL for unwanted results. Caller must
/// free the others.
///
/// @param refNum:		IN:  library reference number
/// @param urlP:		IN:  URL to be parsed
/// @param volRefNumP:	OUT: pass back the file's volume's volRefNum
/// @param filePath:	OUT: pass back the file's path
/// @retval Err Error code
Err FileBrowserLibParseFileURL(UInt16 refNum,
	const Char *urlP, UInt16 *volRefNumP, Char **filePathP)
	FILE_BROWSER_LIB_TRAP(kFileBrowserLibTrapParseFileURL);

/// Entry point to pop up a dialog in which multiple files can be selected
///
/// @param refNum:			IN: library reference number
/// @param volRefNumP:		IN: the default volume and the result
/// @param path:			IN: the default path
/// @param numFilesP		OUT: number of files selected
/// @param maxFiles			IN: maximum number of files allowed to select
/// @param filenames		OUT: names of files selected
/// @param numExtensions:	IN: number of extensions to display
/// @param extensions:		IN: compatible extensions to display
/// @param fileType:		IN: optional MIME type or extension to get
///                			    default directory on each volume
/// @param title:			IN: open dialog title
/// @param flags:			IN: any combo of kOpenFlag* flags
/// @retval Boolean True if one or more files are selected
Boolean FileBrowserLibShowMultiselectDialog(UInt16 refNum,
	UInt16 *volRefNumP, Char *path, UInt16 *numFilesP,
	UInt16 maxFiles, Char **filenames, UInt16 numExtensions,
	const Char **extensions, const Char *fileType,
	const Char *title, UInt32 flags)
	FILE_BROWSER_LIB_TRAP(kFileBrowserLibTrapShowMultiselectDialog);

#undef FILE_BROWSER_LIB_TRAP

#endif // __FILE_BROWSER_LIB_68K_H__
