/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 * @defgroup DispExt Display Extent Library
 *
 * @{
 * @}
 */
/** 
 * @ingroup DispExt
 *
 */
 
/**
 * @file 	PalmDisplayExtent.h
 * @version 1.0
 *
 * @brief Public API for the Display Extent Library.
 * 
 *
 * <hr>
 */

 
#ifndef __PALMDISPLAYEXTENT_H__
#define __PALMDISPLAYEXTENT_H__

#include <PalmTypes.h>
#include <LibTraps.h>

/**
 * @name Library type and creator
 *
 */
/*@{*/
#define dexLibName			"rotmgr"		/**< PalmDisplayExtent library name. */
#define dexLibCreator		'rotM'				/**< PalmDisplayExtent Creator ID. */
#define dexLibType			sysFileTLibrary 	/**< PalmDisplayExtent type. */
#define dexFtrNumVersion	(0)				/**< PalmDisplayExtent feature number. */
/*@}*/


/**
 * @name Library Traps
 *
 */
/*@{*/
#define kDexLibTrapOpen				sysLibTrapOpen		/**<		*/
#define kDexLibTrapClose			sysLibTrapClose		/**<		*/


#define kDexLibTrapDexGetDisplayAddress		(sysLibTrapCustom + 4)	/**<		*/
#define kDexLibTrapDexGetDisplayDimensions	(sysLibTrapCustom + 5)	/**<		*/
/*@}*/

/********************************************************************
 * Prototypes
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the physical screen buffer (Display Address).
 *
 * 
 * @param refnum:	IN:  Reference number of the Display Extent Manager library.
 *
 * @retval Returns a pointer to the physical screen buffer.
 *
 * @remarks You should only use this function if direct screen access is required.
 *	    This function returns a pointer to the physical display.
 *	    If you need to do direct screen access in 90, 180, and 270 rotation, use this function.
 *	    If you use the regular APIs (eg something like BmpGetBits(WinGetBimatp(WinDisplay())))
 * 	    you would get a back buffer pointer instead and would not see anything until you force
 *	    a screen copy of this back buffer. Note that the address points to the top-left corner
 *          of the device in potrait mode always.
 * @code
 *      // this code draws 6400 red pixels in the screen.
 *
 *		RGBColorType color = { 0, 255, 0, 0 }; 
 *		IndexedColorType index = WinRGBToIndex(&color); 
 *		UInt8 *bufferP = (UInt8*)DexGetDisplayAddress(gDexLibRefNum);
 *		UInt32 x = 0; 
 *		
 *		for( x = 0; x < 6400; x++) 
 *	    	 bufferP[x] = index; 
 *@endcode
 *
 */
void *DexGetDisplayAddress(UInt16 refnum)                                       
				SYS_TRAP(kDexLibTrapDexGetDisplayAddress);

/**
 * @brief Get the physical screen width, height, and rowbytes.
 *
 * 
 * @param refnum:	IN:  Reference number of the Display Extent Manager library.
 * @param width		IN:  Pointer to the width of the screen.
 * @param height	IN:  Pointer to the height of the screen.
 * @param rowBytes	IN:  Pointer to the rowbytes of the screen.
 *
 * @retval Nothing
 *
 */
void DexGetDisplayDimensions(UInt16 refnum, Coord *width, Coord *height, UInt16 *rowBytes)     
				SYS_TRAP(kDexLibTrapDexGetDisplayDimensions);

#ifdef __cplusplus
}
#endif

#endif
