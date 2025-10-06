/******************************************************************************
 *   Copyright :
 *	  This is free software; you can redistribute it and/or modify
 *	  it as you like.
 *
 *	  This program is distributed in the hope that it will be useful,
 *	  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 ***************************************************************************/

/**
 * @file 	FarCall.h
 * @version 1.0
 * @date 	06/06/2000
 *
 * Macros for declaring a function that will be located more than
 * 32K away from its caller.  This is trickier than it sounds, since
 * the branch instructions in the 68K don't support greater than 16-bit
 * jumps.
 *
 *  History:
 *	22-feb-2000  dia  Initial version
 *   08-mar-2000  dia  Fixed off-by-two error (don't know why it worked
 *                     before).
 *   06-Apr-2000  BP   Added support for gcc 2.95 and new callseq attribute
 *   11-Apr-2000  BP   Added FAR_PTR for function pointers
 *   06-jun-2000  dia  Fixed FAR_PTR--it wasn't loading right.
 
 * <hr>
 */

#ifndef __FAR_CALL_H__
#define __FAR_CALL_H__

        // The following macro can be used to declare a function as a 
        // FAR_CALL to another, normal function.  
        //
        // Sample usage:
        //     Err AddrGetDatabase (DmOpenRef *addrPP, UInt16 mode);
        //     Err FarAddrGetDatabase (DmOpenRef *addrPP, UInt16 mode)
        //       FAR_CALL (AddrGetDatabase);
        //
        // Preferred usage (saves typing and enforces naming):
        //     typedef Err AddrGetDatabaseFnType (DmOpenRef *addrPP, 
        //                                        UInt16 mode);
        //     FAR_DECLARE (AddrGetDatabase);
        //
        //
        // The way the code works is as follows:
        // 1. LEA     (funcName - .).L, A0
        //    Load the distance from the current PC to the function
        //    into A0.  This works because LEA allows you to specify a 
        //    full 32-bit address.
        //    ...note that in order to force this to work as absolute 
        //    and not as PC-relative, we can't use a standard LEA, so 
        //    we need to hand-assemble the correct LEA and then use a 
        //    DC.L to get the address.  Also note that the DC.Led
        //    address is 2 further than it would be if we just had
        //    an instruction (since the "." is the next instruction).
        // 2. JSR     -6(PC, A0.L)    ; aka -4(PC, A0.L) if your 
        //                            ; assembler thinks differently.
        //    Do the actual jump. Note that the -6 is needed because 
        //    the "." in instruction #1 is 6 bytes before the current 
        //    PC in this instruction (remember, the PC to start the 
        //    jump at is 2 bytes past the current instruction's start).
        //
        // This can also be looked at in assembly as:
        // 1. DC.W 0x41F9; DC.L (funcName - .)
        // 2. DC.W 0x4EBB; DC.W 0x88FA

	#if (defined __GNUC__) && (EMULATION_LEVEL == EMULATION_NONE)
	  #if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)

        #define FAR_CALL(funcName)                        \
            __attribute__ (( __callseq__ (                \
					"DC.W 0x41F9; "                       \
		        	"DC.L " #funcName " - .; "            \
                    "JSR -6(%%PC, %%A0.L)") ))

	  #else         // GNUC < 2.95

        #define FAR_CALL(funcName)                                       \
            __attribute__ (( inline (0x41F9, "DC.L " #funcName " - .",   \
                                     "JSR    -6(%%PC, %%A0.L)"     ) ))

  	  #endif        // GNUC < 2.95

      #define FAR_DECLARE(funcName)                               \
          funcName##FnType funcName;                              \
          funcName##FnType Far##funcName FAR_CALL (funcName)

      // FAR_PTR: 
	  //   This macro can be used to reference a function 
	  //   pointer greater than 32k away from the current code.
	  //   The assembly code is functionally similar to FAR_CALL. 
	  // 
	  // Sample Usage: 
	  //       void* far_start; 
	  //       FAR_PTR (&far_start, start);
	  //
	  //   where start() is a function within the current scope. 
	  //   
	  #define FAR_PTR(outPtr, funcPtr)               \
				  asm ("DC.W 0x41F9;"                \
				       "DC.L " #funcPtr " - .;"      \
                       "LEA -6(%%PC, %%A0.L), %0"    \
						 : "=a" (*(outPtr))          \
						 :                           \
                         : "a0" );           
        
	#else		// not GCC
        
        // UNTESTED:

        // In Metrowerks, just make it so that the macro evaluates to nothing
        // this will simply declare the far function but not do anything...
        #define FAR_CALL(funcName)

        // UNTESTED:

        // Declare the function, and make the far version just a #define...
        #define FAR_DECLARE(funcName)								\
            funcName##FnType funcName;                              \
            static funcName##FnType * const Far##funcName = funcName                       \
            // blank line to absorb ";"

		#define FAR_PTR(outPtr, funcPtr) \
			do { *(outPtr) = (funcPtr); } while (0)

	#endif		// not GCC

#endif	// __FAR_CALL_H__
