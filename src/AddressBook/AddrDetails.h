/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddrDetails.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#ifndef ADDRDETAILS_H
#define ADDRDETAILS_H

#include <PalmOS.h>
#include <Event.h>


/************************************************************
 * Function Prototypes
 *************************************************************/

Boolean DetailsHandleEvent (EventType * event) SEC("code2");
Boolean DetailsDeleteRecord (void) SEC("code2");

#endif // ADDRDETAILS_H
