/**
 * @file  palmOneSystemCommon.h
 * @brief This is the common public header for palmOne system
 *		  definitions.
 */

/*
 * \author    Debbie Chyi
 * $Id:$
 *
 ****************************************************************/


#ifndef __PALMONE_SYSTEM_COMMON_H__
#define __PALMONE_SYSTEM_COMMON_H__


// Macro that builds a library version value from the passed-in major
//	and minor values.  Major values are incremented when a library's
//	API changes.  When a major value is incremented, the minor value
//	is reset to 0.  Minor values are incremented when a library is
//	released with code changes but the API itself has not changed
//	from the previous release.
#define sysMakeLibAPIVersion(major, minor)					\
		(													\
		(((UInt32)((UInt16)(major) & 0x0FFFF)) << 16) |		\
		(((UInt32)((UInt16)(minor) & 0x0FFFF)))				\
		)


// Macros for parsing the library version number.  Comments above
//	sysMakeLibVersion describe the difference between and major and
//	minor version numbers.
#define sysGetLibAPIVersionMajor(libVer)		(((UInt16)((libVer) >> 16)) & 0x0FFFF)
#define sysGetLibAPIVersionMinor(libVer)		(((UInt16)(libVer))         & 0x0FFFF)

#endif // __PALMONE_SYSTEM_COMMON_H__