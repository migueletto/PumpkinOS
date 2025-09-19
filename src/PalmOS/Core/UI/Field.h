/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Field.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This file defines field structures and routines.
 *
 *****************************************************************************/

#ifndef __FIELD_H__
#define __FIELD_H__

#include <PalmTypes.h>

#include <Font.h>
#include <Event.h>
#include <Window.h>

#define maxFieldTextLen	0x7fff

// default maximun number of line the a dynamicly sizing field will expand to.
// Can be changed with FldSetMaxVisibleLines
#define  maxFieldLines 	11


// kind alignment values
enum justifications {leftAlign, centerAlign, rightAlign};
typedef enum justifications JustificationType;


#define undoBufferSize 100

typedef enum { undoNone, undoTyping, undoBackspace, undoDelete,
					undoPaste, undoCut, undoInput } UndoMode;

typedef struct FieldUndoTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FIELDS	// These fields will not be available in the next OS release!
{
	UndoMode		mode;
	UInt8 		reserved;
	UInt16		start;
	UInt16		end;
	UInt16		bufferLen;
	Char 			*buffer;
}
#endif
FieldUndoType;


typedef struct FieldAttrTag
{
	UInt16 usable			:1;	// Set if part of ui 
	UInt16 visible			:1;	// Set if drawn, used internally
	UInt16 editable		:1;	// Set if editable
	UInt16 singleLine		:1;	// Set if only a single line is displayed
	UInt16 hasFocus      :1;   // Set if the field has the focus
	UInt16 dynamicSize	:1;   // Set if height expands as text is entered
	UInt16 insPtVisible	:1;	// Set if the ins pt is scolled into view
	UInt16 dirty			:1;	// Set if user modified
	UInt16 underlined		:2;	// text underlined mode
	UInt16 justification	:2;	// text alignment
	UInt16 autoShift		:1;	// Set if auto case shift
	UInt16 hasScrollBar	:1;	// Set if the field has a scroll bar
	UInt16 numeric			:1;	// Set if numeric, digits and secimal separator only
	UInt16 reserved		:1;	// Reserved for future use
} FieldAttrType;

typedef FieldAttrType *FieldAttrPtr;

typedef struct LineInfoTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FIELDS	// These fields will not be available in the next OS release!
{
  UInt16	start;			// position in text string of first char.
  UInt16	length;			// number of character in the line
}
#endif
LineInfoType;

typedef LineInfoType *LineInfoPtr;


typedef struct FieldType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FIELDS	// These fields will not be available in the next OS release!
{
	UInt16				id;
	RectangleType		rect;
	FieldAttrType		attr;
	Char 					*text;					// pointer to the start of text string 
	MemHandle			textHandle;				// block the contains the text string
	LineInfoPtr			lines;
	UInt16				textLen;
	UInt16				textBlockSize;
	UInt16				maxChars;
	UInt16				selFirstPos;
	UInt16				selLastPos;
	UInt16				insPtXPos;
	UInt16				insPtYPos;
	FontID				fontID;
	UInt8 				maxVisibleLines;		// added in 4.0 to support FldSetMaxVisibleLines

  UInt16 objIndex, numUsedLines, totalLines;
  UInt16 offset, size; // for textHandle
  Boolean updateTextHandle;
  UInt16 top, pos;
  Char *textBuf;
  void *formP;
}
#endif
FieldType;


typedef FieldType *FieldPtr;					// deprecated, use FieldType *


//---------------------------------------------------------------------
//	Field Functions
//---------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

extern void FldCopy (const FieldType *fldP)
							SYS_TRAP(sysTrapFldCopy);

extern void FldCut (FieldType *fldP)
							SYS_TRAP(sysTrapFldCut);

extern void FldDrawField (FieldType *fldP)
							SYS_TRAP(sysTrapFldDrawField);

extern void FldEraseField (FieldType *fldP)
							SYS_TRAP(sysTrapFldEraseField);
 
extern void FldFreeMemory (FieldType *fldP)
							SYS_TRAP(sysTrapFldFreeMemory);

extern void FldGetBounds (const FieldType *fldP, RectanglePtr rect)
							SYS_TRAP(sysTrapFldGetBounds);

extern FontID FldGetFont (const FieldType *fldP)
							SYS_TRAP(sysTrapFldGetFont);

extern void FldGetSelection (const FieldType *fldP, UInt16 *startPosition, UInt16 *endPosition)
							SYS_TRAP(sysTrapFldGetSelection);

extern MemHandle FldGetTextHandle (const FieldType *fldP)
							SYS_TRAP(sysTrapFldGetTextHandle);

extern Char * FldGetTextPtr (const FieldType *fldP)
							SYS_TRAP(sysTrapFldGetTextPtr);

extern Boolean FldHandleEvent (FieldType *fldP, EventType *eventP)
							SYS_TRAP(sysTrapFldHandleEvent);

extern void FldPaste (FieldType *fldP)
							SYS_TRAP(sysTrapFldPaste);

extern void FldRecalculateField (FieldType *fldP, Boolean redraw)
							SYS_TRAP(sysTrapFldRecalculateField);

extern void FldSetBounds (FieldType *fldP, const RectangleType *rP)
							SYS_TRAP(sysTrapFldSetBounds);

extern void FldSetFont (FieldType *fldP, FontID fontID)
							SYS_TRAP(sysTrapFldSetFont);

extern void FldSetText (FieldType *fldP, MemHandle textHandle, UInt16 offset, UInt16 size)
							SYS_TRAP(sysTrapFldSetText);

extern void FldSetTextHandle (FieldType *fldP, MemHandle textHandle)
							SYS_TRAP(sysTrapFldSetTextHandle);

extern void FldSetTextPtr (FieldType *fldP, Char *textP)
							SYS_TRAP(sysTrapFldSetTextPtr);

extern void FldSetUsable (FieldType *fldP, Boolean usable)
							SYS_TRAP(sysTrapFldSetUsable);

extern void FldSetSelection (FieldType *fldP, UInt16 startPosition, UInt16 endPosition)
							SYS_TRAP(sysTrapFldSetSelection);

extern void FldGrabFocus (FieldType *fldP)
							SYS_TRAP(sysTrapFldGrabFocus);

extern void FldReleaseFocus (FieldType *fldP)
							SYS_TRAP(sysTrapFldReleaseFocus);

extern UInt16 FldGetInsPtPosition (const FieldType *fldP)
							SYS_TRAP(sysTrapFldGetInsPtPosition);

extern void FldSetInsPtPosition (FieldType *fldP, UInt16 pos)
							SYS_TRAP(sysTrapFldSetInsPtPosition);

extern void FldSetInsertionPoint (FieldType *fldP, UInt16 pos)
							SYS_TRAP(sysTrapFldSetInsertionPoint);

extern UInt16 FldGetScrollPosition (const FieldType *fldP)
							SYS_TRAP(sysTrapFldGetScrollPosition);

extern void FldSetScrollPosition (FieldType *fldP, UInt16 pos)
							SYS_TRAP(sysTrapFldSetScrollPosition);
							
extern void FldGetScrollValues (const FieldType *fldP, UInt16 *scrollPosP,
	UInt16 *textHeightP, UInt16 *fieldHeightP)
							SYS_TRAP(sysTrapFldGetScrollValues);

extern UInt16 FldGetTextLength (const FieldType *fldP)
							SYS_TRAP(sysTrapFldGetTextLength);

extern void FldScrollField (FieldType *fldP, UInt16 linesToScroll, WinDirectionType direction)
							SYS_TRAP(sysTrapFldScrollField);
							
extern Boolean FldScrollable (const FieldType *fldP,  WinDirectionType direction)
							SYS_TRAP(sysTrapFldScrollable);

extern UInt16 FldGetVisibleLines (const FieldType *fldP)
							SYS_TRAP(sysTrapFldGetVisibleLines);

extern UInt16 FldGetTextHeight (const FieldType *fldP)
							SYS_TRAP(sysTrapFldGetTextHeight);

extern UInt16 FldCalcFieldHeight (const Char *chars, UInt16 maxWidth)
							SYS_TRAP(sysTrapFldCalcFieldHeight);

extern UInt16 FldWordWrap (const Char *chars, Int16 maxWidth)
							SYS_TRAP(sysTrapFldWordWrap);

extern void FldCompactText (FieldType *fldP)
							SYS_TRAP(sysTrapFldCompactText);

extern Boolean FldDirty (const FieldType *fldP)
							SYS_TRAP(sysTrapFldDirty);

extern void FldSetDirty (FieldType *fldP, Boolean dirty)
							SYS_TRAP(sysTrapFldSetDirty);

extern UInt16 FldGetMaxChars (const FieldType *fldP)
							SYS_TRAP(sysTrapFldGetMaxChars);

extern void FldSetMaxChars (FieldType *fldP, UInt16 maxChars)
							SYS_TRAP(sysTrapFldSetMaxChars);

extern Boolean FldInsert (FieldType *fldP, const Char *insertChars, UInt16 insertLen)
							SYS_TRAP(sysTrapFldInsert);

extern void FldDelete (FieldType *fldP, UInt16 start, UInt16 end)
							SYS_TRAP(sysTrapFldDelete);

extern void FldUndo (FieldType *fldP)
							SYS_TRAP(sysTrapFldUndo);

extern UInt16 FldGetTextAllocatedSize (const FieldType *fldP)
							SYS_TRAP(sysTrapFldGetTextAllocatedSize);

extern void FldSetTextAllocatedSize (FieldType *fldP, UInt16 allocatedSize)
							SYS_TRAP(sysTrapFldSetTextAllocatedSize);

extern void FldGetAttributes (const FieldType *fldP, FieldAttrPtr attrP)
							SYS_TRAP(sysTrapFldGetAttributes);

extern void FldSetAttributes (FieldType *fldP, const FieldAttrType *attrP)
							SYS_TRAP(sysTrapFldSetAttributes);

extern void FldSendChangeNotification (const FieldType *fldP)
							SYS_TRAP(sysTrapFldSendChangeNotification);

extern void FldSendHeightChangeNotification (const FieldType *fldP, UInt16 pos, Int16 numLines)
							SYS_TRAP(sysTrapFldSendHeightChangeNotification);

extern Boolean FldMakeFullyVisible (FieldType *fldP)
							SYS_TRAP(sysTrapFldMakeFullyVisible);

extern UInt16 FldGetNumberOfBlankLines (const FieldType *fldP)
							SYS_TRAP(sysTrapFldGetNumberOfBlankLines);

extern FieldType *FldNewField (void **formPP, UInt16 id, 
	Coord x, Coord y, Coord width, Coord height, 
	FontID font, UInt32 maxChars, Boolean editable, Boolean underlined, 
	Boolean singleLine, Boolean dynamicSize, JustificationType justification, 
	Boolean autoShift, Boolean hasScrollBar, Boolean numeric)
							SYS_TRAP(sysTrapFldNewField);

// added in 4.0
extern void FldSetMaxVisibleLines (FieldType *fldP, UInt8 maxLines)
							SYS_TRAP(sysTrapFldSetMaxVisibleLines);

#ifdef __cplusplus 
}
#endif

#endif // __FIELD_H__
