/*
 * $Id: list.h,v 1.2 2003/01/29 15:16:15 chrish Exp $
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

#ifndef PLUCKER_LIST_H
#define PLUCKER_LIST_H


#include "viewer.h"


typedef struct ListNode {
    MemPtr              data;
    struct ListNode*    next;
} ListNode;


struct LinkedListType {
    UInt16      count;
    ListNode*   first;
    ListNode*   last;
};

typedef struct LinkedListType* LinkedList;


/* create new list */
extern LinkedList ListCreate( void );

/* delete list (will not release any contents in the list) */
extern void ListDelete( LinkedList list );

/* delete list and release any contents in the list */
extern void ListRelease( LinkedList list );

/* get number of items in list */
extern UInt16 ListSize( LinkedList list );

/* check if list is empty */
extern Boolean ListIsEmpty( LinkedList list );

/* add new item to end of list */
extern void ListAppend( LinkedList list, MemPtr item );

/* add new item to beginning of list */
extern void ListPrepend( LinkedList list, MemPtr item );

/* add new item to list after specified item */
extern void ListInsert( LinkedList list, MemPtr prevItem, MemPtr item );

/* get first item from list */
extern MemPtr ListFirst( LinkedList list );

/* get last item from list */
extern MemPtr ListLast( LinkedList list );

/* get next item from list */
extern MemPtr ListNext( LinkedList list, MemPtr item );

/* get previous item from list */
extern MemPtr ListPrev( LinkedList list, MemPtr item );

/* get item at given index (counting from 1) */
extern MemPtr ListGet( LinkedList list, UInt16 index );

/* return list index (counting from 1) for given item, returns 0 if not found */
extern UInt16 ListIndex( LinkedList list, MemPtr item );

/* remove item from list */
extern void ListTakeOut( LinkedList list, MemPtr item );


#endif
