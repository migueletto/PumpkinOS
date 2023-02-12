/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: M68KHwr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Pilot debugger remote hardware/system info 
 *
 *****************************************************************************/

#ifndef __M68KHWR_H
#define __M68KHWR_H

// Pilot common definitions
#include <PalmTypes.h>


/***********************************************************************
 * Breakpoint words we use
 ***********************************************************************/
#define m68kTrapInstr			0x4E40
#define m68kTrapVectorMask		0x000F

/***********************************************************************
 * 68000 Exception Vector table
 ***********************************************************************/
typedef struct M68KExcTableType {
	UInt32	initStack;								// initial stack pointer
	UInt32	initPC;									// initial PC
	
	UInt32	busErr;									// 08 
	UInt32	addressErr;								// 0C  
	UInt32	illegalInstr;							// 10  
	UInt32	divideByZero;							// 14  
	UInt32	chk;										// 18
	UInt32	trap;										// 1C
	UInt32	privilege;								// 20
	UInt32	trace;									// 24
	UInt32	aTrap;									// 28
	UInt32	fTrap;									// 2C
	UInt32	reserved12;								// 30
	UInt32	coproc;									// 34
	UInt32	formatErr;								// 38
	UInt32	unitializedInt;						// 3C
	
	UInt32	reserved[8];							// 40-5C
	
	UInt32	spuriousInt;							// 60
	UInt32	autoVec1;								// 64
	UInt32	autoVec2;								// 68
	UInt32	autoVec3;								// 6C
	UInt32	autoVec4;								// 70
	UInt32	autoVec5;								// 74
	UInt32	autoVec6;								// 78
	UInt32	autoVec7;								// 7C
	
	UInt32	trapN[16];								// 80 - BC
	
	UInt32	unassigned[16];						// C0 - FC
	} M68KExcTableType;
 
 

/**************************************************************************************
 *  structure for the Motorolla 68000 processor registers (variables).
 *
 *	WARNING:
 *	This structure is used as the body of the 'read regs' command response
 *  packet.  Any changes to it will require changes in the nub's code.
 *
 **************************************************************************************/
typedef struct M68KRegsType {
	UInt32	d[8];							/*  data registers  */
	UInt32	a[7];							/*  address registers  */
	UInt32	usp;							/*  user stack pointer  */
	UInt32	ssp;							/*  supervisor stack pointer  */
	UInt32	pc;							/*  program counter  */
	UInt16	sr;							/*  status register  */
} M68KRegsType;



 
/**************************************************************************************
 *  bit masks for testing M68000 status register fields
 **************************************************************************************/

/* trace mode */
#define	m68kSrTraceMask			0x08000
#define	m68kSrTraceBit				15

/* supervisor state */
#define	m68kSrSupervisorMask		0x02000

/* interrupt mask */
#define	m68kSrInterruptMask		0x00700
#define	m68kSrInterruptOffset	8			/* offset for right-shifting interrupt mask */

/* condition codes */
#define	m68kSrExtendMask			0x00010
#define	m68kSrNegativeMask		0x00008
#define	m68kSrZeroMask				0x00004
#define	m68kSrOverflowMask		0x00002
#define	m68kSrCarryMask			0x00001



#endif	//__M68KHWR_H
