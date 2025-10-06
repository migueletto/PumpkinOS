/**
 * @file 	palmOne_68K.h
 * @version 1.0
 * @date 	03/05/2004
 *
 *	Main palmOne Palm OS API Header file
 * 
 * This is the main header file for all third party developers. A developer
 * would include if they wanted to use any of the palmOne components. This
 * includes all the extentions, libraries and any app components. This file is
 * for api calls that will work only in 68K code. 
 * This file automatically includes the Common/palmOneCommon.h file so there is 
 * no need for a developer to include that file seperately. 
 * This is the ONLY file that should be needed for any of the palmOne specific 
 * components. This file includes all the other header files
 * for the 68K space relative to this directory. It has to stay at the top of
 * the Incs tree. 
 *
 * \license
 *
 *    Copyright (c) 2004 palmOne Inc., All Rights Reserved
 *
 *****************************************************************************/
 
#ifndef __PALMONE_68K_H__
#define __PALMONE_68K_H__


//Including the standard Palm OS headers
#ifndef __PALMTYPES_H__
									#include <PalmTypes.h>
#endif
#ifndef __PALMCOMPATIBILITY_H__
									#include <PalmCompatibility.h>
#endif
#ifndef	__PALMOS_H__ 
									#include <PalmOS.h>
#endif

//Including the old Handspring Common files that are needed by certain apps
#ifndef __PALMONECHARS_H__
									#include <68K/System/palmOneChars.h>
#endif


//Now let's include the palmOne headers
#ifndef __HS_COMMON_H__
									#include <Common/HsCommon.h>
#endif
//5-Way Navigation Headers
#ifndef __PALMONENAVIGATOR_H__
									#include <68K/System/palmOneNavigator.h>
#endif
#ifndef __HWUTILS68K_H__
									#include <68K/System/HardwareUtils68K.h>
#endif
#ifndef __PALMBGSERVICE_H__
									#include <68K/System/PalmBGService.h>
#endif
#ifndef __PALMCREATORS_H__
									#include <Common/System/PalmCreators.h>
#endif
#ifndef __PALMDISPLAYEXTENT_H__
									#include <68K/System/PalmDisplayExtent.h>
#endif
#ifndef __PALMERRORBASE_H__
									#include <68K/System/PalmErrorBase.h>
#endif
#ifndef __PALMFEATURES_H__
									#include <Common/System/PalmFeatures.h>
#endif
#ifndef __PALMGOLCD_H__
									#include <68K/System/PalmGoLCD.h>
#endif
#ifndef __PalmHiResTime_H__
									#include <68K/System/PalmHiResTime.h>
#endif
#ifndef __PalmLED_H__
									#include <68K/System/PalmLED.h>
#endif
#ifndef __PALMLAUNCHCODES_H__
									#include <68K/System/PalmLaunchCodes.h>
#endif
#ifndef __PalmLcdOverlay_H__
									#include <68K/System/PalmLcdOverlay.h>
#endif
// we might be able to remove this 
//since it seems related to ARM development
#ifndef PALM_NATIVE_H_
									#include <68K/System/PalmNative.h>
#endif
#ifndef __PINLIB_H__
									#include <68K/System/PalmPin.h>
#endif
#ifndef __PalmPower_H__
									#include <68K/System/PalmPower.h>
#endif
#ifndef __PALMRESOURCES_H__
									#include <68K/System/PalmResources.h>
#endif
#ifndef __PALMONEVMFONTLIBRARY_H__
									#include <68K/Libraries/VMFont/palmOneVMFontLib.h>
#endif
#ifndef __PALMVMLAUNCH_H__
									#include <68K/System/PalmVMLaunch.h>
#endif
#ifndef __PALMVMPLUGIN_H__
									#include <68K/System/PalmVMPlugin.h>
#endif
#ifndef __SLIDER_H__
									#include <68K/System/Slider.h>
#endif
//Smart Text Engine
#ifndef __SMARTTEXTENGINE68K_H__
									#include <68K/Libraries/SmartTextEngine/SmartTextEngine.h>
#endif

//Audio
#ifndef __PalmAudioPlayback_H__
									#include <68K/Libraries/Audio/PalmAudioPlayback.h>
#endif
#ifndef __PALMSOUNDCUSTOMCTRL_H__
									#include <68K/Libraries/Audio/PalmSoundCustomCtrl.h>
#endif
// Not including PalmSoundMgrExt.h at this time because of overloading of enum
// already exists in the 5.2 OS.

//Camera
#ifndef __palmOneCameraMgr_H__
									#include <68K/Libraries/CameraMgr/palmOneCamera.h>
#endif
#ifndef __PalmOneCameraSlider_H__
									#include <68K/Libraries/CameraMgr/palmOneCameraSlider.h>
#endif

//Codec Pluggin Manager
#ifndef _PALMONECODECPLUGINMGR_H_
									#include <68K/Libraries/CodecPluginMgr/palmOneCodecPluginMgr.h>
#endif
#ifndef _PALMONECODECFORMAT_H_
									#include <Common/Libraries/CodecPluginMgr/palmOneCodecFormat.h>
#endif

//Network related libraries
#ifndef PALM_NETSERVICES_H__
									#include <68K/Libraries/Network/PalmNetServices.h>
#endif
#ifndef _PALM_VPPI_H_
									#include <68K/Libraries/Network/PalmVPPI.h>
#endif
#ifndef _PALM_WPPI_H_
									#include <68K/Libraries/Network/PalmWPPI.h>
#endif
#ifndef PALM_WIFI_COMMON_H_
									#include <68K/Libraries/Network/PalmWiFiCommon.h>
#endif

//Imaging Library
#ifndef PALM_PHOTO_H_
									#include <68K/Libraries/Imaging/PalmPhoto.h>
#endif

//File Browser Library
#ifndef __FILE_BROWSER_LIB_68K_H__
									#include <68K/Libraries/filebrowser/FileBrowserLib68K.h>
#endif

//SndFileStream Library
#ifndef __SNDFILESTREAM_H__
									#include <68K/Libraries/SndFileStream/palmOneSndFileStream.h>
#endif


//Transition of Old Handspring headers


#endif /* __PALMONE_68K_H__ */
