/******************************************************************************
 *
 * Copyright (c) 1999-2003 PalmSource, Inc. All rights reserved.
 *
 * File: TraceMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *             	Tracing API
 *
 *****************************************************************************/

#ifndef __TRACEMGR_H__
#define __TRACEMGR_H__

/* ------------------------------------------------------------------- 

Expected syntax:
	TraceInit()
	TraceOutput(T   (errorClass,"format",...)     )
	TraceOutput(TL  (errorClass,"format",...)     )
	TraceOutput(B   (errorClass,addr,count)       )
	TraceOutput(VT  (errorClass,"format",va_list) )
	TraceOutput(VTL (errorClass,"format",va_list) )
	TraceClose()

Error classes listed in Incs\Core\System\ErrorBase.h starting with
Palm OS 3.5, in SystemMgr.h for earlier Palm OS versions. Applications
should use appErrorClass.

Format string: % flags width type

Supported flags: 
- 		Left justified display (default is right justified)
+		Always displays the sign symbol (default: display only '-')
space	Displays a space instead of a '+' symbol
 
Supported types:
ld		Int32
lu		UInt32
lx,lX	UInt32 in hexadecimal
hd		Int16
hu		UInt16
hx,hX	UInt16 in hexadecimal
s		0 terminated string
c		character
%		the % character
 
---------------------------------------------------------------------- */

#include <HostControl.h>

/* ------------------------------------------------------------------- */

#define TRACE_OUTPUT_OFF	0
#define TRACE_OUTPUT_ON		1

#include <PalmOptTraceLevel.h>

#if (TRACE_OUTPUT == TRACE_OUTPUT_ON)
	#define	TraceInit		HostTraceInit
	#define	TraceClose		HostTraceClose
	#define	TraceOutput(X)	Host##TraceOutput##X
#else
	#define	TraceOutput(X)		
	#define	TraceInit()		
	#define	TraceClose()		
#endif

/* ------------------------------------------------------------------- */

#define TraceDefine(x,y) (x+y)	// Used for custom error classes
// ex: #define myErrorClass TraceDefine(appErrorClass,3)

/*	---------------------------------------------------------------------------	*/

#endif	/* __TRACEMGR_H__ */
