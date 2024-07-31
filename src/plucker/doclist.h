/*
 * $Id: doclist.h,v 1.12 2003/07/10 12:47:14 prussar Exp $
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

#ifndef PLUCKER_DOCLIST_H
#define PLUCKER_DOCLIST_H

#include "viewer.h"
#include "genericfile.h"


#define ALL_ELEMENTS 0


/* Store category info for document */
extern void StoreCategoriesInDocumentList(Char* name,
                UInt16 newValue) DOCLIST_SECTION;

/* Store time when document is read */
extern void StoreReadTimeInDocumentList(Char* name,
                UInt32 newValue) DOCLIST_SECTION;

/* Check if category already exists */
extern Boolean CategoryExists( UInt8 index ) DOCLIST_SECTION;

/* Get category name */
extern void GetCategoryName( UInt8 index, Char* name ) DOCLIST_SECTION;

/* Open document list */
extern void OpenDocList( void ) DOCLIST_SECTION;

/* Remove record with document info, return result from database call */
extern void RemoveDocInfoRecord( UInt16 index ) DOCLIST_SECTION;

/* Get a handle to record containing document info */
extern MemHandle ReturnDocInfoHandle( const UInt16 index ) DOCLIST_SECTION;

/* Rename document name in document list, return result from database call */
extern void UpdateDocumentName( UInt16 index, const Char*name, 
    const Char* filename ) DOCLIST_SECTION;

/* Set category name, return dmAllCategories if successful, otherwise
   the index for the already existing category */
extern UInt8 StoreCategoryName( UInt8 index, Char* name ) DOCLIST_SECTION;

/* Return number of Plucker documents */
extern UInt16 GetNumOfDocuments( void ) DOCLIST_SECTION;

/* Close document list */
extern void CloseDocList( void ) DOCLIST_SECTION;

/* Merge categories */
extern void MergeCategories( UInt8 dst, UInt8 src ) DOCLIST_SECTION;

/* Remove category */
extern void RemoveCategoryName( UInt8 index ) DOCLIST_SECTION;

/* Add Plucker document */
extern void AddDocument( DocumentInfo* docInfo, Char* volumeLabel ) DOCLIST_SECTION;

/* Add Plucker document */
extern void UpdateDocument( DocumentInfo* docInfo, Char* volumeLabel,
                UInt16 index, MemHandle* handle ) DOCLIST_SECTION;

/* Find record with document data in doclist, always returns a valid
   handle or throws an exception */
extern MemHandle FindDocData( Char* name, UInt16 numOfElements, 
                    UInt16* index ) DOCLIST_SECTION;

/* indicate document as found in remove list */
extern void SetFoundDocument( UInt16 index ) DOCLIST_SECTION;

/* Retrieve the default category/categories from the document */
extern UInt16 GetDefaultCategories( DocumentInfo* docInfo ) DOCLIST_SECTION;

/* Return info for last openned document */
extern DocumentInfo* GetLastDocInfo( void ) DOCLIST_SECTION;

/* Release memory allocated for last openned document */
void ReleaseLastDocInfo( void ) DOCLIST_SECTION;

Boolean ProcessSelectCategory(FormType* form, Boolean title,
            UInt16* category, Char* name) DOCLIST_SECTION;

UInt16 GetNextCategory(UInt16 currentCategory) DOCLIST_SECTION;

#endif

