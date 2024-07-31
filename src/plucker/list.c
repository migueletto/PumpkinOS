/*
 * $Id: list.c,v 1.4 2003/01/30 14:11:40 chrish Exp $
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

#include "debug.h"
#include "util.h"

#include "list.h"


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static Boolean IsFirst( LinkedList list, ListNode* node ) LIST_SECTION;
static Boolean IsLast( LinkedList list, ListNode* node ) LIST_SECTION;
static ListNode* FindNode( LinkedList list, MemPtr item ) LIST_SECTION;
static ListNode* FindPrevNode( LinkedList list, MemPtr item ) LIST_SECTION;


/* check if node is first in list */
static Boolean IsFirst
    (
    LinkedList  list,   /* list handler */
    ListNode*   node    /* list node */
    )
{
    return ( list->first == node );
}



/* check if node is last in list */
static Boolean IsLast
    (
    LinkedList  list,   /* list handler */
    ListNode*   node    /* list node */
    )
{
    return ( list->last == node );
}


/* find node with given item */
static ListNode* FindNode
    (
    LinkedList  list,   /* list handler */
    MemPtr      item    /* list item */
    )
{
    ListNode* node;

    node = NULL;
    if ( list != NULL ) {
        node = list->first;
        while ( node != NULL && node->data != item )
            node = node->next;
    }
    return node;
}



/* find node located before node with given item */
static ListNode* FindPrevNode
    (
    LinkedList  list,   /* list handler */
    MemPtr      item    /* list item */
    )
{
    ListNode* node;
    ListNode* prev;

    node = NULL;
    prev = NULL;
    /* there must be more than one item in the list */
    if ( list != NULL && 1 < list->count ) {
        node = list->first;
        while ( node != NULL && node->data != item ) {
            prev = node;
            node = node->next;
        }
        /* we didn't find the given item in the list */
        if ( node == NULL )
            prev = NULL;
    }
    return prev;
}



/* create new list */
LinkedList ListCreate( void )
{
    LinkedList list;

    list        = SafeMemPtrNew( sizeof *list );
    list->count = 0;
    list->first = NULL;
    list->last  = NULL;

    return list;
}



/* delete list (will not release any contents in the list) */
void ListDelete
    (
    LinkedList list /* list handler */
    )
{
    if ( list != NULL ) {
        ListNode* node;

        node = list->first;
        while ( node != NULL ) {
            ListNode* next;

            next = node->next;
            SafeMemPtrFree( node );
            list->count--;
            node = next;
        }
        SafeMemPtrFree( list );
    }
}



/* delete list and release any contents in the list */
void ListRelease
    (
    LinkedList list
    )
{
    if ( list != NULL ) {
        ListNode* node;

        node = list->first;
        while ( node != NULL ) {
            ListNode*   next;
            MemPtr      data;

            data = node->data;
            SafeMemPtrFree( data );

            next = node->next;
            SafeMemPtrFree( node );

            list->count--;
            node = next;
        }
        SafeMemPtrFree( list );
    }
}



/* get number of items in list */
UInt16 ListSize
    (
    LinkedList list /* list handler */
    )
{
    if ( list != NULL )
        return list->count;
    else
        return 0;
}



/* check if list is empty */
Boolean ListIsEmpty
    (
    LinkedList list /* list handler */
    )
{
    if ( list != NULL )
        return ( list->count == 0 );
    else
        return true;
}



/* add new item to end of list */
void ListAppend
    (
    LinkedList  list,   /* list handler */
    MemPtr      item    /* list item */
    )
{
    if ( list != NULL ) {
        ListNode* node;

        node        = SafeMemPtrNew( sizeof *node );
        node->data  = item;
        node->next  = NULL;
        if ( list->last != NULL ) {
            list->last->next    = node;
            list->last          = node;
        }
        else {
            list->first = node;
            list->last  = node;
        }
        list->count++;
    }
}



/* add new item to beginning of list */
void ListPrepend
    (
    LinkedList  list,   /* list handler */
    MemPtr      item    /* list item */
    )
{
    if ( list != NULL ) {
        ListNode* node;

        node        = SafeMemPtrNew( sizeof *node );
        node->data  = item;
        node->next  = NULL;
        if ( list->first != NULL ) {
            node->next  = list->first;
            list->first = node;
        }
        else {
            list->first = node;
            list->last  = node;
        }
        list->count++;
    }
}



/* add new item to list after specified item, if the specified item 
   isn't in the list nothing will happen */
void ListInsert
    (
    LinkedList  list,       /* list handler */
    MemPtr      prevItem,   /* existing list item */
    MemPtr      item        /* list item */
    )
{
    ListNode* prevNode;

    prevNode = FindNode( list, prevItem );
    if ( prevNode == list->last ) {
        ListAppend( list, item );
    }
    else if ( prevNode != NULL ) {
        ListNode* node;

        node            = SafeMemPtrNew( sizeof *node );
        node->data      = item;
        node->next      = prevNode->next;
        prevNode->next  = node;
            
        list->count++;
    }
}



/* get first item from list */
MemPtr ListFirst
    (
    LinkedList list /* list handler */
    )
{
    MemPtr data;

    data = NULL;
    if ( list != NULL && list->first != NULL )
        data = list->first->data;

    return data;
}



/* get last item from list */
MemPtr ListLast
    (
    LinkedList list /* list handler */
    )
{
    MemPtr data;

    data = NULL;
    if ( list != NULL && list->last != NULL )
        data = list->last->data;

    return data;
}



/* get next item from list */
MemPtr ListNext
    (
    LinkedList  list,   /* list handler */
    MemPtr      item    /* list item */
    )
{
    ListNode* node;

    node = FindNode( list, item );
    if ( node != NULL && node->next != NULL )
        return node->next->data;
    else
        return NULL;
}



/* get previous item from list */
MemPtr ListPrev
    (
    LinkedList  list,   /* list handler */
    MemPtr      item    /* list item */
    )
{
    ListNode* node;

    node = FindPrevNode( list, item );
    if ( node != NULL )
        return node->data;
    else
        return NULL;
}



/* get item at given index (counting from 1) */
MemPtr ListGet
    (
    LinkedList  list,   /* list handler */
    UInt16      index   /* list index */
    )
{
    MemPtr data;

    data = NULL;
    if ( list != NULL && 0 < index && index <= list->count ) {
        ListNode* node;

        node = list->first;
        while ( --index )
            node = node->next;
        data = node->data;
    }
    return data;
}



/* return list index (counting from 1) for given item, returns 0 if not found */
UInt16 ListIndex
    (
    LinkedList  list,   /* list handler */
    MemPtr      item    /* list item */
    )
{
    ListNode*   node;
    UInt16      index;

    node    = NULL;
    index   = 0;
    if ( list != NULL ) {
        node = list->first;
        while ( node != NULL && node->data != item ) {
            node    = node->next;
            index  += 1;
        }
        if ( node == NULL )
            index = 0;
    }
    return index;
}



/* remove item from list */
void ListTakeOut
    (
    LinkedList  list,   /* list handler */
    MemPtr      item    /* list item */
    )
{
    if ( list != NULL && item != NULL ) {
        ListNode* node;
        ListNode* prev;

        node = list->first;
        prev = NULL;

        while ( node != NULL && node->data != item ) {
            prev = node;
            node = node->next;
        }

        if ( node != NULL ) {
            if ( IsFirst( list, node ) )
                list->first = node->next;
            if ( IsLast( list, node ) )
                list->last = prev;
            if ( prev != NULL )
                prev->next = node->next;

            SafeMemPtrFree( node );
            list->count--;
        }
    }
}

