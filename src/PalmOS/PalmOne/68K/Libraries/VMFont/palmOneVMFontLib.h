/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @defgroup VMFont Versamail Font Library
 *
 * @{
 * @}
 */

/**
 * @file 	palmOneVMFontLib.h
 * @version 1.0
 * @date 	08/26/2003
 * @brief Defines APIs and structures for the Palm VersaMail Font Picker / Font Manager
 * Service which lets users get / set font characteristics.
 */

/*
 * @author  Darsono Sutedja
 *
 * <hr>
 */

#ifndef __PALMONEVMFONTLIBRARY_H__
#define __PALMONEVMFONTLIBRARY_H__

#include <PalmTypes.h>
#include <Font.h>
#include <DataMgr.h>
#include <LibTraps.h>
#include <Common/Libraries/VMFont/VMFontLibCommon.h>

#ifdef BUILDING_VMFONTLIB
#define VMFONTLIB_TRAP( trapNum )
#else
#define VMFONTLIB_TRAP( trapNum )  SYS_TRAP( trapNum )
#endif


/********************************************************************
 * Prototypes
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opens the Font Picker library. Does Initialization of library.
 *
 * @brief This function should be called prior to calling the other Font
 *		  Picker functions.
 *
 * @param  	libRefNum:		IN: Reference number of the Font Picker library.
 * @param 	clientContextP:	OUT: pointer to variable for returning client context.
 *							The client context is used to maintain client-specific
 *							data for multiple client support.The value returned here
 *							will be used as a parameter for other library functions
 *							which require a client context.
 *
 * @retval	Err The error code returned from the library. If this is errNone, the
 *			function was sucessful.
 *
 * @code
 * UInt16 fontPickerLibRefNum = 0;
 * UInt32 context;
 * err = SysLibFind(fontPickerLibName, &fontPickerLibRefNum);
 * if( err == sysErrLibNotFound )
 * {
 *     err = SysLibLoad(fontPickerType, fontPickerCreatorID, &fontPickerLibRefNum);
 *         if( !err ) {
 *             err = VMFontOpen(fontPickerLibRefNum, &context);
 *         }
 * } @endcode
 *
 * @see VMFontClose
 */

extern Err VMFontOpen( UInt16 libRefNum, UInt32 *clientContextP )
				VMFONTLIB_TRAP( sysLibTrapOpen );


/**
 * Closes the Font Picker library and perform cleanup.
 *
 * @brief This function should be called after your application has
 * 		  finished with the Font Picker library.
 *
 * @param  	libRefNum:		IN: Reference number of the Font Picker library.
 * @param 	clientContext:	IN: Library's client context number got during open.
 *
 * @retval	Err The error code returned from the library. If this is errNone, the
 *			function was sucessful.
 *
 * @code
 * err = VMFontClose(fontPickerLibRefNum, context); @endcode
 *
 * @see VMFontOpen
 */

extern Err VMFontClose( UInt16 libRefNum, UInt32 clientContext )
				VMFONTLIB_TRAP( sysLibTrapClose );



/**
 * Get the Font Library Version
 *
 * @param  	libRefNum:		IN: Reference number of the Font Picker library.
 * @param 	versionP:		OUT: Pointer to the version number
 *
 * @retval	Err Always errNone
 *
 */

extern Err VMFontVersion( UInt16 libRefNum, UInt32 *versionP )
				VMFONTLIB_TRAP( flTrapAPIVersion );

/**
 * Get the font ID based on the given font characteristics.
 * The returned font ID is NOT guaranteed to be always available.
 * Therefore to avoid any error, please make sure that VMFontExists
 * is used whenever a font is needed.
 *
 * @param  	libRefNum:		IN: Reference number of the Font Picker library.
 * @param 	fontP:			IN: the font characteristics.
 *
 * @retval	UInt8 The defined font ID
 *
 * @code
 * // get the font ID for gill Sans MT 9 pt Italic
 * VMFontPtr fontP = (VMFontPtr) MemPtrNew (sizeof (VMFontType));
 * fontP->face = gillSans;
 * fontP->size = size6;
 * fontP->style = italic;
 * UInt8 myFontID = stdFont;
 * if (VMFontExists( theRefNum, fontP ))
 *	{    myFontID = VMFontGetFont( theRefNum, fontP );   } @endcode
 *
 * @see VMFontExists
 */

extern UInt8 VMFontGetFont(UInt16 libRefNum, const VMFontPtr fontP)
	VMFONTLIB_TRAP(flTrapAPIGetFont);

/**
 * Get the Font characteristics based on a saved FontID
 *
 * @param  	libRefNum:		IN: Reference number of the Font Picker library.
 * @param 	targetP:		IN: the font characteristics.
 * @param	fontID:			IN: the font ID passed in.
 *
 * @retval	UInt8 The defined font ID
 *
 * @code
 * UInt8 myOldID;
 * UInt8 myFontID;
 * MemHandle handle;
 * VMFontPtr fontP;
 *
 * handle = MemHandleNew(sizeof(VMFontType));
 * fontP = (VMFontPtr) MemHandleLock(handle);
 *
 * myOldID = GetFontIDFromPrefs(); // assume that we have a funct to get the old ID
 * myFontID = VMFontGetFontByID( gRefNum, fontP, myOldID );
 * @endcode
 *
 *
 */

extern UInt8 VMFontGetFontByID(UInt16 libRefNum, VMFontPtr targetP, UInt8 fontID)
	VMFONTLIB_TRAP(flTrapAPIGetFontByID);

/**
 * Check to see if the specified font exists in the database.
 *
 * @param  	libRefNum:		IN: Reference number of the Font Picker library.
 * @param 	fontP:			IN: the font characteristics we want to check.
 *
 * @retval	Boolean True if the font exists in the database, false otherwise.
 *
 * @code
 * MemHandle handle;
 * VMFontPtr fontP;
 * Boolean exists = false;
 *
 * handle = MemHandleNew(sizeof( VMFontType ));
 * fontP = (VMFontPtr) MemHandleLock(handle);
 *
 * fontP->face = segoe;
 * fontP->size = size6;
 * fontP->style = bold;
 *
 * exists = VMFontExists( gRefNum, fontP );
 *
 * MemHandleUnlock(handle);
 * MemHandleFree(handle);
 * @endcode
 *
 * @see VMFontGetFont
 */

extern Boolean VMFontExists(UInt16 libRefNum, const VMFontPtr fontP)
	VMFONTLIB_TRAP(flTrapAPIFontExists);

/**
 * Given a font characteristics (defined as VMFontPtr), get the
 * human-readable font description of that font in the form of
 * "<NAME> <SIZE> <STYLE>", e.g. "Gill Sans MT 9 Bold".
 *
 * @param  	libRefNum:		IN: Reference number of the Font Picker library.
 * @param 	fontP:			IN: the font characteristics we want to check.
 * @param   stringP:		OUT: Pointer to output string buffer.
 *
 * @retval	Nothing
 *
 */
extern void	VMFontToString(UInt16 libRefNum, const VMFontPtr fontP, char *stringP)
	VMFONTLIB_TRAP(flTrapAPIFontToString);


/**
 * Given a font face, find out how many size/styles were associated with it.
 *
 * @param  	libRefNum:		IN: Reference number of the Font Picker library.
 * @param 	face:			IN: the font face we need details about.
 * @param   sizeP:          OUT: contains total sizes for the font face
 * @param   styleP:         OUT: contains total styles for the font face
 *
 * @retval	Nothing
 *
 */
extern void VMFontCountStyleAndSize(UInt16 libRefNum, VMFontFace face, Int8 *sizeP, Int8 *styleP)
	VMFONTLIB_TRAP(flTrapAPIFontCountStyleAndSize);

// Font Picker functions

/**
 * Initialize the font picker UI internal data structure.
 * Please note that it is very important that this function
 * is called before any Font picker specific functions are called.
 *
 * @param  	libRefNum:		IN: Reference number of the Font Picker library.
 *
 * @retval	Nothing
 *
 */

extern void	InitFontUI(UInt16 libRefNum)
	VMFONTLIB_TRAP(flTrapAPIInitFontUI);

/**
 * Popup the font picker dialog, and let the user pick a font from the
 * list.  Once the user taps OK, the selected font id will be copied
 * into the target parameter.
 *
 * @param  	libRefNum:		IN:  Reference number of the Font Picker library.
 * @param   fontIDP:        OUT: fontID that user has selected.
 * @param   oldFontID :     IN:  The font picker will use this font to pre populate the font list.
 *                               This is very useful when you have a previously saved font in
 *                               the preference that you want to use when popping up the dialog.
 *                               If no saved preference is used, then you can always pass in
 *                               stdFont as the argument.
 *
 *
 * @retval Boolean True if user tapped OK false if user selected cancel
 */

extern Boolean DoFontDialog(UInt16 libRefNum, UInt8 *fontIDP, UInt8 oldFontID)
	VMFONTLIB_TRAP(flTrapAPIDoFontDialog);


/**
 * Clear the interal cache of the font library
 * @param libRefNum: IN: library reference number.
 */
extern void	VMFontClearCache(UInt16 libRefNum)
	VMFONTLIB_TRAP(flTrapAPIClearCache);

/**
 * Define the UI rules, e.g.: hide style picker, do not show the style picker,
 * show only small fonts, etc.
 *
 * @brief                   Currently the only controllable UI components are
 *
 *						    Components visibility controls
 *							- Font face picker: true/false
 *							- Font style picker: true/false
 *							- Font size picker: true/false
 *							List items controls
 * 							- Font Size: allSize, smallFonts, largeFonts
 *							- Font Style: allStyle, plainOnly
 *
 * @param  	libRefNum:		IN:Reference number of the Font Picker library.
 * @param   fontRules:      IN:fontRules that need to be applied
 *
 * @retval	Nothing
 *
 * @code
 *
 * VMFontUIRules rules;
 * UInt8 myFontID;
 * UInt8 oldFontID;
 *
 * // let's hide style drop down list, and only display small fonts
 * //(size 6 if available and size 9 fonts)
 * rules.fontFaceVisible = true;
 * rules.fontSizeVisible = true;
 * rules.fontStyleVisible = true;
 * rules.sizeRule = smallFonts;
 * rules.styleRule = allStyle;
 *
 * InitFontUI( libRefNum );
 * SetFontUIRules( libRefNum, rules ); // apply the rules
 * if  (DoFontDialog( libRefNum, &myFontID, oldFontID ))
 * {
 *    // do something with the font
 * }
 * else
 * {
 *    // handle the case when user cancels out of the font picker UI
 * }
 * @endcode
 */

extern void SetFontUIRules(UInt16 libRefNum, VMFontUIRules fontRules)
	VMFONTLIB_TRAP(flTrapAPISetFontUIRules);

#ifdef __cplusplus
}
#endif

#endif
