/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddrEdit.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#ifndef ADDREDIT_H
#define ADDREDIT_H

#include <Event.h>


/************************************************************
 * Function Prototypes
 *************************************************************/

Boolean	EditHandleEvent (EventType * event) SEC("code2");
void	EditNewRecord () SEC("code2");

#endif // ADDREDIT_H
