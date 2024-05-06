/*
 * $Id: pi-syspkt.h,v 1.22 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-syspkt.h
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _PILOT_SYSPKT_H
#define _PILOT_SYSPKT_H

#include "pi-args.h"

#ifdef __cplusplus
extern "C" {
#endif

	struct Pilot_registers {
		uint32_t A[7];
		uint32_t D[8];
		uint32_t USP, SSP;
		uint32_t PC, SR;
	};

	struct Pilot_breakpoint {
		uint32_t address;
		int enabled;
	};

	struct Pilot_state {
		struct Pilot_registers regs;
		int reset;
		int exception;
		int instructions[30];
		struct Pilot_breakpoint breakpoint[6];
		uint32_t func_start, func_end;
		char func_name[32];
		int trap_rev;
	};

	struct Pilot_watch {
		uint32_t address;
		uint32_t length;
		uint32_t checksum;
	};

	struct RPC_param {
		int byRef;
		size_t size;
		int invert;
		int arg;
		void *data;
	};

	struct RPC_params {
		int trap;
		int reply;
		int args;
		struct RPC_param param[20];
	};

	extern int sys_UnpackState
	    PI_ARGS((void *buffer, struct Pilot_state * s));

	extern int sys_UnpackRegisters
	    PI_ARGS((void *buffer, struct Pilot_registers * r));


	extern int sys_Continue
	    PI_ARGS((int sd, struct Pilot_registers * r,
		     struct Pilot_watch * w));
	extern int sys_Step PI_ARGS((int sd));

	extern int sys_QueryState PI_ARGS((int sd));
	extern int sys_ReadMemory
	    PI_ARGS((int sd, uint32_t addr, uint32_t len,
		     void *buf));
	extern int sys_WriteMemory
	    PI_ARGS((int sd, uint32_t addr, uint32_t len,
		     void *buf));

	extern int sys_ToggleDbgBreaks PI_ARGS((int sd));

	extern int sys_SetTrapBreaks PI_ARGS((int sd, int *traps));
	extern int sys_GetTrapBreaks PI_ARGS((int sd, int *traps));

	extern int sys_SetBreakpoints
	    PI_ARGS((int sd, struct Pilot_breakpoint * b));
	extern int sys_Find
	    PI_ARGS((int sd, uint32_t startaddr,
		     uint32_t stopaddr, size_t len, int caseinsensitive,
		     void *data, uint32_t *found));

	extern int sys_RemoteEvent
	    PI_ARGS((int sd, int penDown, int x, int y, int keypressed,
		     int keymod, int keyasc, int keycode));

	extern int sys_RPC
	    PI_ARGS((int sd, int sock, int trap, int32_t *D0, int32_t *A0,
		     int params, struct RPC_param * param, int rep));

#define RPC_Byte(data) (-2),((unsigned int)htons((data)<<8))
#define RPC_Short(data) (-2),((unsigned int)htons((data)))
#define RPC_Long(data) (-4),((unsigned int)htonl((data)))
#define RPC_Ptr(data,len) (len),((void*)(data)),0
#define RPC_LongPtr(ptr) (4),((void*)(ptr)),1
#define RPC_ShortPtr(ptr) (2),((void*)(ptr)),1
#define RPC_BytePtr(ptr) (2),((void*)(ptr)),2
#define RPC_LongRef(ref) (4),((void*)(&(ref))),1
#define RPC_ShortRef(ref) (2),((void*)(&(ref))),1
#define RPC_ByteRef(ref) (2),((void*)(&(ref))),2
#define RPC_NullPtr RPC_Long(0)
#define RPC_End 0

#define RPC_IntReply  2
#define RPC_PtrReply  1
#define RPC_NoReply 0

	extern int RPC
	    PI_ARGS((int sd, int sock, int trap, int ret, ...));

	extern void InvertRPC PI_ARGS((struct RPC_params * p));
	extern void UninvertRPC PI_ARGS((struct RPC_params * p));

	extern int PackRPC
	    PI_ARGS((struct RPC_params * p, int trap, int reply, ...));

	extern uint32_t DoRPC
	    PI_ARGS((int sd, int sock, struct RPC_params * p,
		     int *error));

	extern int dlp_ProcessRPC
	    PI_ARGS((int sd, int trap, int ret, ...));

	extern int RPC_Int_Void PI_ARGS((int sd, int trap));
	extern int RPC_Ptr_Void PI_ARGS((int sd, int trap));

#ifdef __cplusplus
}
#endif
#endif				/*_PILOT_SYSPKT_H_*/
