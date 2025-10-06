/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Sound
 *
 */

/**
 *
 * @file	PalmSoundCustomCtrl.h
 *
 * @brief	Public 68K include file for the Sound Manager custom controls for the
 *			IMA_ADPCM codec for Treo 600 and Treo 650 smartphones.
 *
 * This header file and associated header files support the specific sound
 * functionality of the Treo smartphones. You should use the Palm OS Sound
 * Manager APIs for most of your work.
 *
 * Notes:
 * How to use the IMA_ADPCM code custom control:
 *
 * When you create a stream, make sure you set the format to sndFormatIMA_ADPCM and
 * that the type is set to 0.
 *
 * Then use the custom control to set the block align  paramter:
 *
 * SndCodecCustomControlType ct;
 * UInt16 blockAlign= Swap16(blockAlign); // This needs to be in little endian
 * ct.apiCreator	= Swap32(codecDriverIMAADPCM);
 * ct.apiSelector	= Swap32(codecIMAADPCMSetBlockSize);
 * ct.valueP		= (void*)Swap32(&blockAlign);
 * ct.valueLenP		= NULL;
 * err = SndStreamDeviceControl(streamRef, sndControlCodecCustomControl, &ct, sizeof(ct));
 *
 * The calling application should always load this library with
 * SysLibLoad() before use, even if it is already open by another
 * application(ie, SysLibFind() returns a valid refnum). When
 * the application is done with the library, it should be
 * unloaded with SysLibRemove(). We do this because there is
 * no good way to synchronize loading and unloading of libraries
 * among multiple applications. It also greatly simplifies internal
 * synchronization.
 */

#ifndef __PALMSOUNDCUSTOMCTRL_H__
#define __PALMSOUNDCUSTOMCTRL_H__

/**
 * @name Custom Control for the IMA_ADPCM codec
 *
 */
/*@{*/
#define	codecDriverIMAADPCM					'APCM'	/**< IMA ADPCM codec driver. */
#define	codecIMAADPCMSetBlockSize			0x0001	/**< IMA ADPCM block size set command. */
/*@}*/

/**
 * IMA_ADPCM codec Custom Control type
 *
 */
enum {
    sndControlCodecCustomControl = 12	/**< Param is a SndCodecCustomControlType*. */
};

/**
 * @brief Holds Custom Control type information for the Extended Sound Manager.
 *
 * This structure is used to set/get values specific to a codec.
 * Note that if this custom control comes from a 68K app, it's the responsibility
 * of the 68K app to flip and align the fields of the structure correctly. The fields
 * are defined as being in little-endian format.
 */
typedef struct {
	UInt32	apiCreator;		/**<Codec creator. (codecDriverIMAADPCM...) */
	UInt32	apiSelector;	/**<Codec selector. (codecIMAADPCMSetBlockSize...) */
	void	*valueP;		/**<Pointer to the parameters to be passed to the codec. */
	UInt16	*valueLenP;		/**<Size of the parameters to be passed to the codec. */
} SndCodecCustomControlType;


#endif // __PALMSOUNDCUSTOMCTRL_H__

