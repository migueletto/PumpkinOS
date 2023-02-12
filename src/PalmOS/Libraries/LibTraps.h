/******************************************************************************
 *
 * Copyright (c) 1999-2003 PalmSource, Inc. All rights reserved.
 *
 * File: LibTraps.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *      Palm OS Shared Library 'default' traps.
 *
 *****************************************************************************/

 #ifndef __LIBTRAPS_H_
 #define __LIBTRAPS_H_

// Include elementary types
#include <PalmTypes.h>

#if CPU_TYPE == CPU_68K
#include <M68KHwr.h>
#endif

//--------------------------------------------------------------------
// Define Library Trap Numbers
//--------------------------------------------------------------------
// Library traps start here and go up by 1's
#define	sysLibTrapBase		0xA800

#define sysLibTrapName		0xA800
#define sysLibTrapOpen		0xA801
#define sysLibTrapClose		0xA802
#define sysLibTrapSleep		0xA803
#define sysLibTrapWake		0xA804
#define sysLibTrapCustom	0xA805

#endif  //__LIBTRAPS_H_
