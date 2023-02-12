/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PalmUtils.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *    The utilities in this file are not a supported part of the Palm OS
 *    API and hence are not documented in the Palm OS Reference.  They are
 *    here to provide a common vocabulary for various trivial operations in
 *    the SDK Source Code Examples.  These examples are part of the SDK, so
 *    this file is part of the SDK too -- but not of the supported API.
 *
 *    These macros are all trivial -- the value of this file is not in the
 *    definitions, but in the common vocabulary provided.  You can use this
 *    header in your applications if you wish, but it will be just as easy
 *    to add relevant macros to your project itself.  Since this header is
 *    unsupported and subject to change in future SDKs, there is some risk
 *    in using it directly in your projects.
 *
 *    Since this header is not part of the Palm OS API, it is not included
 *    from PalmOS.h or any other API headers.  (In particular, in C++ there
 *    are better alternatives to the facilities defined here, and parts of
 *    this header (the min/max macros) actively conflict with their C++
 *    alternatives.  Hence this header should not be forced on unsuspecting
 *    C++ users.)
 *
 *****************************************************************************/

#ifndef PALMUTILS_H
#define PALMUTILS_H

// Macros for documenting that a function parameter is unused and
// suppressing warnings if the compiler supports a notation for that.

#if defined __MWERKS__
	#define UNUSED_PARAM_ID(id)
	#define UNUSED_PARAM_ATTR
#elif defined __GNUC__
	#define UNUSED_PARAM_ID(id)  id
	#define UNUSED_PARAM_ATTR  __attribute__ ((__unused__))
#else
	#define UNUSED_PARAM_ID(id)  id
	#define UNUSED_PARAM_ATTR
#endif

#define UNUSED_PARAM(id)  UNUSED_PARAM_ID(id) UNUSED_PARAM_ATTR

// In most cases, for simple parameters like "int x", the UNUSED_PARAM macro
// can be used, as in foo() below.  Put only the parameter identifier as
// the argument to UNUSED_PARAM.
//
// Occasionally there might be an unused parameter with a complicated type
// which has some notation trailing after the parameter identifier, as in
// foo()'s "int array[]" parameter below.  In this case you need to use 
// the pair of macros as below: UNUSED_PARAM_ID has the function parameter
// identify as an argument, and UNUSED_PARAM_ATTR goes after the end of the
// parameter's type notation.
//
// void foo (int UNUSED_PARAM(x),
//           int UNUSED_PARAM_ID(array)[] UNUSED_PARAM_ATTR) { }


// Note that these macros will interfere with the function templates of
// the same names in the C++ standard header <algorithm>.  If you want to
// use this header file with C++ and <algorithm> (which is unlikely), you
// should #undef min and max after #including this header.

#undef min
#undef max
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#endif
