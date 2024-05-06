#ifndef _PILOT_VERSION_H_
#define _PILOT_VERSION_H_
/*
 * pi-version.h: Version numbers and global macros.
 *
 * This file contains #defines for pilot-link's version number,
 * and some macros that may be used anywhere in the codebase.
 *
 * PILOT_LINK_IS(0,12,0) may be used to determine if the
 * pilot-link version is _at least_ 0.12.0 (added in 0.12.0).
 *
 */

#define PILOT_LINK_VERSION 0
#define PILOT_LINK_MAJOR 12
#define PILOT_LINK_MINOR 5

#define PILOT_LINK_PATCH ""

#define PILOT_LINK_IS(a,b,c) \
	((PILOT_LINK_VERSION > a) || \
	((PILOT_LINK_VERSION == a) && \
		((PILOT_LINK_MAJOR > b) || \
		((PILOT_LINK_MAJOR == b) && (PILOT_LINK_MINOR >= c)))))


#endif	/* _PILOT_VERSION_H_ */
