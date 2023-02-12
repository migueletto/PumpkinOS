/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Table.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This file defines table structures and routines.
 *
 *****************************************************************************/

#ifndef __TABLE_H__
#define __TABLE_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

#include <Field.h>

//-------------------------------------------------------------------
// Table structures
//-------------------------------------------------------------------

#define tableDefaultColumnSpacing 	1
#define tableNoteIndicatorWidth		7
#define tableNoteIndicatorHeight		11
#define tableMaxTextItemSize 			255	// does not incude terminating null

#define tblUnusableRow					0xffff

// Display style of a table item
//
enum tableItemStyles { checkboxTableItem, 
                       customTableItem, 
                       dateTableItem, 
                       labelTableItem,
                       numericTableItem,
                       popupTriggerTableItem,
                       textTableItem,
                       textWithNoteTableItem,
                       timeTableItem,
                       narrowTextTableItem,
                       tallCustomTableItem
                       };
typedef enum tableItemStyles TableItemStyleType;


typedef struct TableItemTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_TABLES	// These fields will not be available in the next OS release!
{
	TableItemStyleType	itemType;		
   FontID					fontID;		// font for drawing text
	Int16						intValue;
	Char *  					ptr;
}
#endif
TableItemType;
typedef TableItemType *TableItemPtr;


// Draw item callback routine prototype, used only by customTableItem.
typedef void TableDrawItemFuncType  
		(void *tableP, Int16 row, Int16 column, RectangleType *bounds);

typedef TableDrawItemFuncType *TableDrawItemFuncPtr;


// Load data callback routine prototype
typedef Err TableLoadDataFuncType 
		(void *tableP, Int16 row, Int16 column, Boolean editable, 
		MemHandle * dataH, Int16 *dataOffset, Int16 *dataSize, FieldPtr fld);

typedef TableLoadDataFuncType *TableLoadDataFuncPtr;


// Save data callback routine prototype
typedef	Boolean TableSaveDataFuncType
		(void *tableP, Int16 row, Int16 column);

typedef TableSaveDataFuncType *TableSaveDataFuncPtr;

typedef struct TableColumnAttrTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_TABLES	// These fields will not be available in the next OS release!
{
	Coord							width;					// width in pixels
	UInt16						reserved1 		: 5;
	UInt16						masked			: 1;  // if both row + column masked, draw only grey box
	UInt16						editIndicator	: 1;
	UInt16						usable 			: 1;
	UInt16						reserved2		: 8;
	Coord							spacing;					// space after column
	TableDrawItemFuncPtr		drawCallback;
	TableLoadDataFuncPtr		loadDataCallback;
	TableSaveDataFuncPtr		saveDataCallback;

	uint32_t m68k_drawfunc;
	uint32_t m68k_loadfunc;
	uint32_t m68k_savefunc;
}
#endif
TableColumnAttrType;


typedef struct TableRowAttrTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_TABLES	// These fields will not be available in the next OS release!
{
	UInt16						id;
	Coord							height;					// row height in pixels
	//UInt32						data;
	UIntPtr						data;

	UInt16						reserved1		: 7;
	UInt16						usable			: 1;
	UInt16						reserved2		: 4;
	UInt16						masked			: 1;  // if both row + column masked, draw only grey box
	UInt16						invalid			: 1;	// true if redraw needed
	UInt16 						staticHeight	: 1;  // Set if height does not expands as text is entered
	UInt16						selectable		: 1;

	UInt16						reserved3;
}
#endif
TableRowAttrType;


typedef struct TableAttrTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_TABLES	// These fields will not be available in the next OS release!
{
	UInt16 						visible:1;			// Set if drawn, used internally
  UInt16 						editable:1;			// Set if editable
	UInt16 						editing:1;			// Set if in edit mode
	UInt16 						selected:1;			// Set if the current item is selected
	UInt16						hasScrollBar:1;	// Set if the table has a scroll bar
	UInt16 						usable:1;			// Set if in table is visible in the current form
	UInt16						reserved:10;
}
#endif
TableAttrType;


typedef struct TableType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_TABLES	// These fields will not be available in the next OS release!
{
	UInt16						id;
   RectangleType				bounds;
   TableAttrType				attr;
	Int16							numColumns;
	Int16							numRows;
	Int16							currentRow;
	Int16							currentColumn;
	Int16							topRow;
	TableColumnAttrType *	columnAttrs;
   TableRowAttrType *    	rowAttrs;
	TableItemPtr				items;
	FieldType					currentField;
}
#endif
TableType;

typedef TableType *TablePtr;


//-------------------------------------------------------------------
// Table routines
//-------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

extern void TblDrawTable (TableType *tableP)
							SYS_TRAP(sysTrapTblDrawTable);

extern void TblRedrawTable (TableType *tableP)
							SYS_TRAP(sysTrapTblRedrawTable);

extern void TblEraseTable (TableType *tableP)
							SYS_TRAP(sysTrapTblEraseTable);

extern Boolean TblHandleEvent (TableType *tableP, EventType *event)
							SYS_TRAP(sysTrapTblHandleEvent);

extern void TblGetItemBounds (const TableType *tableP, Int16 row, Int16 column, RectangleType *rP)
							SYS_TRAP(sysTrapTblGetItemBounds);

extern void TblSelectItem (TableType *tableP, Int16 row, Int16 column)
							SYS_TRAP(sysTrapTblSelectItem);

extern Int16 TblGetItemInt (const TableType *tableP, Int16 row, Int16 column)
							SYS_TRAP(sysTrapTblGetItemInt);

extern void TblSetItemInt (TableType *tableP, Int16 row, Int16 column, Int16 value)
							SYS_TRAP(sysTrapTblSetItemInt);

extern void TblSetItemPtr (TableType *tableP, Int16 row, Int16 column, void *value)
							SYS_TRAP(sysTrapTblSetItemPtr);

extern void TblSetItemStyle (TableType *tableP, Int16 row, Int16 column, TableItemStyleType type)
							SYS_TRAP(sysTrapTblSetItemStyle);

extern void TblUnhighlightSelection (TableType *tableP)
							SYS_TRAP(sysTrapTblUnhighlightSelection);

extern Boolean TblRowUsable  (const TableType *tableP, Int16 row)
							SYS_TRAP(sysTrapTblRowUsable);

extern void TblSetRowUsable (TableType *tableP, Int16 row, Boolean usable)
							SYS_TRAP(sysTrapTblSetRowUsable);

extern Int16 TblGetLastUsableRow (const TableType *tableP)
							SYS_TRAP(sysTrapTblGetLastUsableRow);

extern void TblSetColumnUsable (TableType *tableP, Int16 column, Boolean usable)
							SYS_TRAP(sysTrapTblSetColumnUsable);

extern void TblSetRowSelectable (TableType *tableP, Int16 row, Boolean selectable)
							SYS_TRAP(sysTrapTblSetRowSelectable);

extern Boolean TblRowSelectable (const TableType *tableP, Int16 row)
							SYS_TRAP(sysTrapTblRowSelectable);

extern Int16 TblGetNumberOfRows (const TableType *tableP)
							SYS_TRAP(sysTrapTblGetNumberOfRows);

extern void TblSetCustomDrawProcedure (TableType *tableP, Int16 column,
	TableDrawItemFuncPtr drawCallback)
							SYS_TRAP(sysTrapTblSetCustomDrawProcedure);

extern void TblSetLoadDataProcedure (TableType *tableP, Int16 column,
	TableLoadDataFuncPtr loadDataCallback)
							SYS_TRAP(sysTrapTblSetLoadDataProcedure);

extern void TblSetSaveDataProcedure (TableType *tableP, Int16 column,
	TableSaveDataFuncPtr saveDataCallback)
							SYS_TRAP(sysTrapTblSetSaveDataProcedure);

extern void TblGetBounds (const TableType *tableP, RectangleType *rP)
							SYS_TRAP(sysTrapTblGetBounds);

extern void TblSetBounds (TableType *tableP, const RectangleType *rP)
							SYS_TRAP(sysTrapTblSetBounds);

extern Coord TblGetRowHeight (const TableType *tableP, Int16 row)
							SYS_TRAP(sysTrapTblGetRowHeight);

extern void TblSetRowHeight (TableType *tableP, Int16 row, Coord height)
							SYS_TRAP(sysTrapTblSetRowHeight);

extern Coord TblGetColumnWidth (const TableType *tableP, Int16 column)
							SYS_TRAP(sysTrapTblGetColumnWidth);

extern void TblSetColumnWidth (TableType *tableP, Int16 column, Coord width)
							SYS_TRAP(sysTrapTblSetColumnWidth);

extern Coord TblGetColumnSpacing (const TableType *tableP, Int16 column)
							SYS_TRAP(sysTrapTblGetColumnSpacing);

extern void TblSetColumnSpacing (TableType *tableP, Int16 column, Coord spacing)
							SYS_TRAP(sysTrapTblSetColumnSpacing);

extern Boolean TblFindRowID (const TableType *tableP, UInt16 id, Int16 *rowP)
							SYS_TRAP(sysTrapTblFindRowID);

extern Boolean TblFindRowData (const TableType *tableP, UIntPtr data, Int16 *rowP)
							SYS_TRAP(sysTrapTblFindRowData);

extern UInt16 TblGetRowID (const TableType *tableP, Int16 row)
							SYS_TRAP(sysTrapTblGetRowID);

extern void TblSetRowID (TableType *tableP, Int16 row, UInt16 id)
							SYS_TRAP(sysTrapTblSetRowID);

extern UIntPtr TblGetRowData (const TableType *tableP, Int16 row)
							SYS_TRAP(sysTrapTblGetRowData);

extern void TblSetRowData (TableType *tableP, Int16 row, UIntPtr data)
							SYS_TRAP(sysTrapTblSetRowData);

extern Boolean TblRowInvalid (const TableType *tableP, Int16 row)
							SYS_TRAP(sysTrapTblRowInvalid);

extern void TblMarkRowInvalid (TableType *tableP, Int16 row)
							SYS_TRAP(sysTrapTblMarkRowInvalid);

extern void TblMarkTableInvalid (TableType *tableP)
							SYS_TRAP(sysTrapTblMarkTableInvalid);

extern Boolean TblGetSelection (const TableType *tableP, Int16 *rowP, Int16 *columnP)
							SYS_TRAP(sysTrapTblGetSelection);

extern void TblInsertRow (TableType *tableP, Int16 row)
							SYS_TRAP(sysTrapTblInsertRow);
							
extern void TblRemoveRow (TableType *tableP, Int16 row)
							SYS_TRAP(sysTrapTblRemoveRow);

extern void TblReleaseFocus (TableType *tableP)
							SYS_TRAP(sysTrapTblReleaseFocus);
							
extern Boolean TblEditing (const TableType *tableP)
							SYS_TRAP(sysTrapTblEditing);

extern FieldPtr TblGetCurrentField (const TableType *tableP)
							SYS_TRAP(sysTrapTblGetCurrentField);
							
extern void TblGrabFocus (TableType *tableP, Int16 row, Int16 column)
							SYS_TRAP(sysTrapTblGrabFocus);

extern void TblSetColumnEditIndicator (TableType *tableP, Int16 column, Boolean editIndicator)
							SYS_TRAP(sysTrapTblSetColumnEditIndicator);

extern void TblSetRowStaticHeight (TableType *tableP, Int16 row, Boolean staticHeight)
							SYS_TRAP(sysTrapTblSetRowStaticHeight);

extern void TblHasScrollBar (TableType *tableP, Boolean hasScrollBar)
							SYS_TRAP(sysTrapTblHasScrollBar);

extern FontID TblGetItemFont (const TableType *tableP, Int16 row, Int16 column)
							SYS_TRAP(sysTrapTblGetItemFont);

extern  void TblSetItemFont (TableType *tableP, Int16 row, Int16 column, FontID fontID)
							SYS_TRAP(sysTrapTblSetItemFont);

extern void *TblGetItemPtr (const TableType *tableP, Int16 row, Int16 column)
							SYS_TRAP(sysTrapTblGetItemPtr);

extern Boolean TblRowMasked  (const TableType *tableP, Int16 row)
							SYS_TRAP(sysTrapTblRowMasked);
							
extern void TblSetRowMasked  (TableType *tableP, Int16 row, Boolean masked)
							SYS_TRAP(sysTrapTblSetRowMasked);
							
extern void TblSetColumnMasked  (TableType *tableP, Int16 column, Boolean masked)
							SYS_TRAP(sysTrapTblSetColumnMasked);

extern Int16 TblGetNumberOfColumns (const TableType *tableP)
							SYS_TRAP(sysTrapTblGetNumberOfColumns);

extern Int16 TblGetTopRow (const TableType *tableP)
							SYS_TRAP(sysTrapTblGetTopRow);

extern void TblSetSelection (TableType *tableP, Int16 row, Int16 column)
							SYS_TRAP(sysTrapTblSetSelection);

#ifdef __cplusplus 
}
#endif

#endif //__TABLE_H__
