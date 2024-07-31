/*
 * $Id: externalform.h,v 1.18 2003/10/08 15:54:29 matto Exp $
 *
 * Viewer - a part of Plucker, the free off-line HTML viewer for PalmOS
 * Copyright (c) 1998-2002, Mark Ian Lillywhite and Michael Nordstrom
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef PLUCKER_EXTERNALFORM_H
#define PLUCKER_EXTERNALFORM_H

#include "viewer.h"


/* Event handler for the externallinks form */
extern Boolean ExternalLinksFormHandleEvent( EventType* event );

/* Set current link in the ExternalLinks record */
extern void SetLinkIndex( Int16 index ) EXTERNALFORM_SECTION;

/* Initialize field with URL string */
extern Boolean AddURLToField( FieldType* fldPtr,
                Int16 index ) EXTERNALFORM_SECTION;

/* Write the text from a TextField to a Memo */
extern void WriteTextFieldToMemo( FieldType* field ) EXTERNALFORM_SECTION;

/* Write the text to a Memo */
extern void WriteTextToMemo( Char* textPtr ) EXTERNALFORM_SECTION;

#endif

