/******************************************************************************
 *                                                                            *
 *            (C) Copyright 2000-2002, Sony Corporation                       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *                                                                            *
 *    <IDENTIFICATION>                                                        *
 *       file name    : $Workfile: SonyTypes.h $
 *                                                                            *
 *    <PROFILE>                                                               *
 *       General defitinions for Sony System & Libraries                      *
 *                                                                            *
 *    <HISTORY>                                                               *
 *       Started on   : 02/05/25                                              *
 *       Last Modified: $Date: 03/05/10 16:09 $
 *                                                                            *
 ******************************************************************************/
/* this file is best viewed by setting TAB-stop as 3 */

#ifndef __SONYTYPES_H__
#define __SONYTYPES_H__

/******************************************************************************
 *    Environment configuration
 ******************************************************************************/
#define _BUILD_FOR_PALMOS_5_
	/* this definition must not appear in SDK for PalmOS 4 or older (for 3.5) */
	/* Defining this here and not in SonyCLIE.h is reasonable, since SonyCLIE.h
	     is not included in all libraries. */

#endif	// __SONYTYPES_H__

