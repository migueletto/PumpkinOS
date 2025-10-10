/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Form.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This file defines form structures and routines.
 *
 *****************************************************************************/

#ifndef __FORM_H__
#define __FORM_H__

#include <PalmTypes.h>
#include <CoreTraps.h>
#include <Preferences.h>
#include <Window.h>
#include <Menu.h>

#include <Field.h>
#include <Control.h>
#include <List.h>
#include <ScrollBar.h>
#include <Table.h>

#define noFocus 0xffff

#define frmInvalidObjectId		0xffff
#define frmNoSelectedControl	0xff

// Update code send as part of a frmUpdate event.
#define frmRedrawUpdateCode			0x8000

// Magic button IDs used by FrmCustomResponseAlert callbacks
#define frmResponseCreate		1974
#define frmResponseQuit			((Int16) 0xBEEF)

// Dynamic Input Area policies
#define frmDIAPolicyStayOpen	0
#define frmDIAPolicyCustom		1


// Alert constants and structures
enum alertTypes {
	informationAlert,
	confirmationAlert,
	warningAlert,
	errorAlert
};
typedef enum alertTypes AlertType;

typedef struct AlertTemplateTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FORMS	// These fields will not be available in the next OS release!
{
	UInt16		alertType;
	UInt16		helpRscID;
	UInt16		numButtons;
	UInt16		defaultButton;
  Char      *title;
  Char      *message;
  Char      *button[8];
}
#endif
AlertTemplateType;


// Types of object in a dialog box
enum formObjects {
	frmFieldObj,
	frmControlObj,
	frmListObj,
	frmTableObj,
	frmBitmapObj,
	frmLineObj,
	frmFrameObj,
	frmRectangleObj,
	frmLabelObj,
	frmTitleObj,
	frmPopupObj,
	frmGraffitiStateObj,
	frmGadgetObj,
	frmScrollBarObj };
typedef enum formObjects FormObjectKind;


typedef struct FormObjAttrTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FORMS	// These fields will not be available in the next OS release!
{
	UInt16 usable			:1;	// Set if part of ui
	UInt16 visible			:1;	// not part of PalmOS: PumpkinOS extension
	UInt16 reserved		:14;	// pad it out
}
#endif
FormObjAttrType;


// Gadget support:
#define formGadgetDrawCmd			0	// paramP is unspecified
#define formGadgetEraseCmd			1	// paramP is unspecified
#define formGadgetHandleEventCmd	2	// paramP is an EventType *for the relevant event.
#define formGadgetDeleteCmd		3	// paramP is unspecified.


// access to this is allowed only within the gadget callback, and not otherwise.
typedef struct FormGadgetAttrTag
{
	UInt16 usable			:1;	// Set if part of ui - "should be drawn"
	UInt16 extended		:1;	// Set if the structure is an "Extended" gadget (i.e., the 'handler' field is present)
	UInt16 visible			:1;	// Set if drawn - "has been drawn" or "must do work to erase"
	UInt16 reserved		:13;	// pad it out
}
FormGadgetAttrType;

struct FormType; // forward reference to FormType so we can declare the handler type:
struct FormGadgetTypeInCallback; // forward reference to FormGadgetTypeInCallback so we can declare the handler type:
typedef Boolean (FormGadgetHandlerType) (struct FormGadgetTypeInCallback *gadgetP, UInt16 cmd, void *paramP);

typedef struct FormGadgetType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FORMS	// These fields will not be available in the next OS release!
{
	// hack to allow 68K programs to access fields inside FormGadgetType.
	UInt16 mId, mAttr, x, y, w, h;
  UInt32 mData, mHandler;

	UInt16						id;
	FormGadgetAttrType		attr;
	RectangleType				rect;
	const void *			   data;
	FormGadgetHandlerType	*handler;
        uint32_t m68k_handler, m68k_data;
}
#endif
FormGadgetType;

// access to this is allowed only within the gadget callback, and not otherwise.
typedef struct FormGadgetTypeInCallback
{
	// hack to allow 68K programs to access fields inside FormGadgetType.
	UInt16 mId, mAttr, x, y, w, h;
        UInt32 mData, mHandler;

	UInt16						id;
	FormGadgetAttrType		attr;
	RectangleType				rect;
	const void *			   data;
	FormGadgetHandlerType	*handler;
        uint32_t m68k_handler, m68k_data;
}
FormGadgetTypeInCallback;

//This is used to check whether the FormGadgetTypeInCallback definition is available
#define FORM_GADGET_TYPE_IN_CALLBACK_DEFINED


// All of the smaller form objects:

typedef struct FormBitmapTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FORMS	// These fields will not be available in the next OS release!
{
	FormObjAttrType			attr;
	PointType					pos;
	UInt16		     			rscID;
}
#endif
FormBitmapType;


typedef struct FormLineTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FORMS	// These fields will not be available in the next OS release!
{
	FormObjAttrType			attr;
	PointType	      		point1;
	PointType	      		point2;
}
#endif
FormLineType;


typedef struct FormFrameTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FORMS	// These fields will not be available in the next OS release!
{
	UInt16						id;
	FormObjAttrType			attr;
	RectangleType				rect;
	UInt16     					frameType;
}
#endif
FormFrameType;


typedef struct FormRectangleTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FORMS	// These fields will not be available in the next OS release!
{
	FormObjAttrType			attr;
	RectangleType				rect;
}
#endif
FormRectangleType;


typedef struct FormLabelTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FORMS	// These fields will not be available in the next OS release!
{
	UInt16						id;
	PointType					pos;
	FormObjAttrType			attr;
	FontID						fontID;
	UInt8 						reserved;
	Char *						text;

  UInt16 marker;
	PointType extent;
  Int16 len;
  Char buf[0];
}
#endif
FormLabelType;


typedef struct FormTitleTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FORMS	// These fields will not be available in the next OS release!
{
	RectangleType				rect;
	Char *						text;

  UInt16 objIndex;
}
#endif
FormTitleType;


typedef struct FormPopupTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FORMS	// These fields will not be available in the next OS release!
{
	UInt16						controlID;
	UInt16						listID;
}
#endif
FormPopupType;


typedef struct FormGraffitiStateTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FORMS	// These fields will not be available in the next OS release!
{
	PointType					pos;
}
#endif
FrmGraffitiStateType;


typedef union FormObjectTag
{
	void *            		ptr;
	FieldType *					field;
	ControlType *				control;
	GraphicControlType *		graphicControl;
	SliderControlType *		sliderControl;
	ListType	*					list;
	TableType *					table;
	FormBitmapType *			bitmap;
//	FormLineType *				line;
//	FormFrameType *			frame;
//	FormRectangleType *		rectangle;
	FormLabelType *			label;
	FormTitleType *			title;
	FormPopupType *			popup;
	FrmGraffitiStateType *	grfState;
	FormGadgetType *			gadget;
	ScrollBarType *			scrollBar;
}
FormObjectType;

// typedef FormObjectType *FormObjectPtr;


typedef struct FormObjListTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FORMS	// These fields will not be available in the next OS release!
{
  UInt16 id;
	FormObjectKind				objectType;
	UInt8 						reserved;
	FormObjectType				object;
}
#endif
FormObjListType;


typedef struct FormAttrTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FORMS	// These fields will not be available in the next OS release!
{
	UInt16 usable             :1; // Set if part of ui 
	UInt16 enabled            :1; // Set if interactable (not grayed out)
	UInt16 visible            :1; // Set if drawn, used internally
	UInt16 dirty              :1; // Set if dialog has been modified
	UInt16 saveBehind         :1; // Set if bits behind form are save when form ids drawn
	UInt16 graffitiShift      :1; // Set if graffiti shift indicator is supported
	UInt16 globalsAvailable   :1; // Set by Palm OS if globals are available for the form event handler
	UInt16 doingDialog        :1; // FrmDoDialog is using for nested event loop
	UInt16 exitDialog         :1; // tells FrmDoDialog to bail out and stop using this form
	UInt16 attnIndicator      :1; // Set if attention indicator is supported
	UInt16 reserved           :6; // pad to 16
	
	UInt16 frmDIAPolicy       :1; // set if custom form policy
	UInt16 inputAreaState     :3; // maps to PINInputAreaStateType
	UInt16 statusState        :1; // if set, control bar open
	UInt16 inputTrigger       :1; // maps to PINInputTriggerStateType
	UInt16 orientation        :3; // reserved for future use
	UInt16 orientationTrigger :1; // reserved for future use
	UInt16 drawing            :1;
	UInt16 reserved2          :5;
}
#endif
FormAttrType;


typedef Boolean FormEventHandlerType (EventType *eventP);

typedef FormEventHandlerType *FormEventHandlerPtr;

typedef struct FormType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FORMS	// These fields will not be available in the next OS release!
{
	WindowType					window;
	UInt16						formId;
   FormAttrType				attr;
	WinHandle	       		bitsBehindForm;
	FormEventHandlerType *	handler;
	UInt16						focus;
	UInt16						defaultButton;
	UInt16						helpRscId;
	UInt16						menuRscId;
	UInt16						numObjects;
	FormObjListType *			objects;

  int selectedObject, activeList, activeField;
  RectangleType helpRect;
  MenuBarType *mbar;
  uint32_t m68k_handler;
  UInt16 diaPolicy;
  void *rsrc;
}
#endif
FormType;

typedef FormType *FormPtr;


// FormActiveStateType: this structure is passed to FrmActiveState for
// saving and restoring active form/window state; this structure's
// contents are abstracted because the contents will differ significantly
// as PalmOS evolves
// Added for PalmOS 3.0
typedef struct FormActiveStateType {
	UInt16	data[11];
} FormActiveStateType;
	

// FrmCustomResponseAlert callback routine prototype
typedef Boolean FormCheckResponseFuncType
		(Int16 button, Char * attempt);

typedef FormCheckResponseFuncType *FormCheckResponseFuncPtr;


//-----------------------------------------------
//  Macros
//-----------------------------------------------

#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
#define ECFrmValidatePtr(formP) FrmValidatePtr(formP)
#else
#define ECFrmValidatePtr(formP)
#endif

//--------------------------------------------------------------------
//
// Form Function
//
//--------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

extern FormType * FrmInitForm (UInt16 rscID)
							SYS_TRAP(sysTrapFrmInitForm);

extern void FrmDeleteForm (FormType *formP)
							SYS_TRAP(sysTrapFrmDeleteForm);

extern void FrmDrawForm (FormType *formP)
							SYS_TRAP(sysTrapFrmDrawForm);

extern void FrmEraseForm (FormType *formP)
							SYS_TRAP(sysTrapFrmEraseForm);

extern FormType * FrmGetActiveForm (void)
							SYS_TRAP(sysTrapFrmGetActiveForm);

extern void FrmSetActiveForm (FormType *formP)
							SYS_TRAP(sysTrapFrmSetActiveForm);

extern UInt16 FrmGetActiveFormID (void)
							SYS_TRAP(sysTrapFrmGetActiveFormID);

extern FieldType* FrmGetActiveField(const FormType* formP)
							SYS_TRAP(sysTrapFrmGetActiveField);

extern Boolean FrmGetUserModifiedState (const FormType *formP)
							SYS_TRAP(sysTrapFrmGetUserModifiedState);

extern void FrmSetNotUserModified (FormType *formP)
							SYS_TRAP(sysTrapFrmSetNotUserModified);

extern UInt16 FrmGetFocus (const FormType *formP)
							SYS_TRAP(sysTrapFrmGetFocus);
            
extern void FrmSetFocus (FormType *formP, UInt16 fieldIndex)
							SYS_TRAP(sysTrapFrmSetFocus);

extern Boolean FrmHandleEvent (FormType *formP, EventType *eventP)
							SYS_TRAP(sysTrapFrmHandleEvent);

extern void FrmGetFormBounds (const FormType *formP, RectangleType *rP)
							SYS_TRAP(sysTrapFrmGetFormBounds);

extern WinHandle FrmGetWindowHandle (const FormType *formP)
							SYS_TRAP(sysTrapFrmGetWindowHandle);

extern UInt16 FrmGetFormId (const FormType *formP)
							SYS_TRAP(sysTrapFrmGetFormId);

extern FormType *FrmGetFormPtr (UInt16 formId)
							SYS_TRAP(sysTrapFrmGetFormPtr);

extern FormType *FrmGetFirstForm (void)
							SYS_TRAP(sysTrapFrmGetFirstForm);

extern UInt16 FrmGetNumberOfObjects (const FormType *formP)
							SYS_TRAP(sysTrapFrmGetNumberOfObjects);

extern UInt16 FrmGetObjectIndex (const FormType *formP, UInt16 objID)
							SYS_TRAP(sysTrapFrmGetObjectIndex);

extern UInt16 FrmGetObjectIndexFromPtr (const FormType *formP, void* objP)
							SYS_TRAP(sysTrapFrmGetObjectIndexFromPtr);

extern UInt16 FrmGetObjectId (const FormType *formP, UInt16 objIndex)
							SYS_TRAP(sysTrapFrmGetObjectId);

extern FormObjectKind FrmGetObjectType (const FormType *formP, UInt16 objIndex)
							SYS_TRAP(sysTrapFrmGetObjectType);

extern void *FrmGetObjectPtr (const FormType *formP, UInt16 objIndex)
							SYS_TRAP(sysTrapFrmGetObjectPtr);

extern void FrmGetObjectBounds (const FormType *formP, UInt16 objIndex,
	RectangleType *rP)
							SYS_TRAP(sysTrapFrmGetObjectBounds);

extern void FrmHideObject (FormType *formP, UInt16 objIndex)
							SYS_TRAP(sysTrapFrmHideObject);

extern void FrmShowObject (FormType *formP, UInt16 objIndex)
							SYS_TRAP(sysTrapFrmShowObject);

extern void FrmGetObjectPosition (const FormType *formP, UInt16 objIndex,
	Coord *x, Coord *y)
							SYS_TRAP(sysTrapFrmGetObjectPosition);

extern void FrmSetObjectPosition (FormType *formP, UInt16 objIndex,
	Coord x, Coord y)
							SYS_TRAP(sysTrapFrmSetObjectPosition);

extern void FrmSetObjectBounds (FormType *formP, UInt16 objIndex,
	const RectangleType *bounds)
							SYS_TRAP(sysTrapFrmSetObjectBounds);

extern Int16 FrmGetControlValue (const FormType *formP, UInt16 objIndex)
							SYS_TRAP(sysTrapFrmGetControlValue);

extern void FrmSetControlValue (const FormType *formP, UInt16 objIndex,
	Int16 newValue)
							SYS_TRAP(sysTrapFrmSetControlValue);

extern UInt16 FrmGetControlGroupSelection (const FormType *formP,
	UInt8 groupNum)
							SYS_TRAP(sysTrapFrmGetControlGroupSelection);

extern void FrmSetControlGroupSelection (const FormType *formP,
	UInt8 groupNum, UInt16 controlID)
							SYS_TRAP(sysTrapFrmSetControlGroupSelection);

extern void FrmCopyLabel (FormType *formP, UInt16 labelID,
	const Char *newLabel)
							SYS_TRAP(sysTrapFrmCopyLabel);

extern const Char *FrmGetLabel (const FormType *formP, UInt16 labelID)
							SYS_TRAP(sysTrapFrmGetLabel);

extern void FrmSetCategoryLabel (const FormType *formP, UInt16 objIndex,
	Char *newLabel)
							SYS_TRAP(sysTrapFrmSetCategoryLabel);

extern const Char *FrmGetTitle (const FormType *formP)
							SYS_TRAP(sysTrapFrmGetTitle);

extern void FrmSetTitle (FormType *formP, Char *newTitle)
							SYS_TRAP(sysTrapFrmSetTitle);

extern void FrmCopyTitle (FormType *formP, const Char *newTitle)
							SYS_TRAP(sysTrapFrmCopyTitle);

extern void *FrmGetGadgetData (const FormType *formP, UInt16 objIndex)
							SYS_TRAP(sysTrapFrmGetGadgetData);

extern void FrmSetGadgetData (FormType *formP, UInt16 objIndex,
	const void *data)
							SYS_TRAP(sysTrapFrmSetGadgetData);

extern void FrmSetGadgetHandler (FormType *formP, UInt16 objIndex,
	FormGadgetHandlerType *attrP)
							SYS_TRAP(sysTrapFrmSetGadgetHandler);

extern UInt16 FrmDoDialog (FormType *formP)
							SYS_TRAP(sysTrapFrmDoDialog);

extern UInt16 FrmAlert (UInt16 alertId)
							SYS_TRAP(sysTrapFrmAlert);
							
extern UInt16 FrmCustomAlert (UInt16 alertId, const Char *s1,
	const Char *s2, const Char *s3)
							SYS_TRAP(sysTrapFrmCustomAlert);

extern void FrmHelp (UInt16 helpMsgId)
							SYS_TRAP(sysTrapFrmHelp);

extern void FrmUpdateScrollers (FormType *formP, UInt16 upIndex,
	UInt16 downIndex, Boolean scrollableUp, Boolean scrollableDown)
							SYS_TRAP(sysTrapFrmUpdateScrollers);

extern Boolean FrmVisible (const FormType *formP)
							SYS_TRAP(sysTrapFrmVisible);

extern void FrmSetEventHandler (FormType *formP, FormEventHandlerType *handler)
							SYS_TRAP(sysTrapFrmSetEventHandler);

extern Boolean FrmDispatchEvent (EventType *eventP)
							SYS_TRAP(sysTrapFrmDispatchEvent);




extern void FrmPopupForm (UInt16 formId)
							SYS_TRAP(sysTrapFrmPopupForm);

extern void FrmGotoForm (UInt16 formId)
							SYS_TRAP(sysTrapFrmGotoForm);

extern void FrmUpdateForm (UInt16 formId, UInt16 updateCode)
							SYS_TRAP(sysTrapFrmUpdateForm);
							
extern void FrmReturnToForm  (UInt16 formId)
							SYS_TRAP(sysTrapFrmReturnToForm);
							
extern void FrmCloseAllForms (void)
							SYS_TRAP(sysTrapFrmCloseAllForms);

extern void FrmSaveAllForms (void)
							SYS_TRAP(sysTrapFrmSaveAllForms);



extern Boolean FrmPointInTitle (const FormType *formP, Coord x, Coord y)
							SYS_TRAP(sysTrapFrmPointInTitle);

extern void FrmSetMenu (FormType *formP, UInt16 menuRscID)
							SYS_TRAP(sysTrapFrmSetMenu);

extern Boolean FrmValidatePtr (const FormType *formP)
							SYS_TRAP(sysTrapFrmValidatePtr);

extern Err FrmAddSpaceForObject (FormType **formPP, MemPtr *objectPP,
	FormObjectKind objectKind, UInt16 objectSize)
							SYS_TRAP(sysTrapFrmAddSpaceForObject);

extern Err FrmRemoveObject (FormType **formPP, UInt16 objIndex)
							SYS_TRAP(sysTrapFrmRemoveObject);

extern FormType *FrmNewForm (UInt16 formID, const Char *titleStrP,
	Coord x, Coord y, Coord width, Coord height, Boolean modal,
	UInt16 defaultButton, UInt16 helpRscID, UInt16 menuRscID)
							SYS_TRAP(sysTrapFrmNewForm);

extern FormLabelType *FrmNewLabel (FormType **formPP, UInt16 ID, const Char *textP,
	Coord x, Coord y, FontID font)
							SYS_TRAP(sysTrapFrmNewLabel);

extern FormBitmapType *FrmNewBitmap (FormType **formPP, UInt16 ID,
	UInt16 rscID, Coord x, Coord y)
							SYS_TRAP(sysTrapFrmNewBitmap);

extern FormGadgetType *FrmNewGadget (FormType **formPP, UInt16 id,
	Coord x, Coord y, Coord width, Coord height)
							SYS_TRAP(sysTrapFrmNewGadget);

extern Err FrmActiveState (FormActiveStateType *stateP, Boolean save)
							SYS_TRAP(sysTrapFrmActiveState);
						
extern UInt16 FrmCustomResponseAlert (UInt16 alertId, const Char *s1, const Char *s2,
	const Char *s3, Char *entryStringBuf, Int16 entryStringBufLength,
	FormCheckResponseFuncPtr callback)
							SYS_TRAP(sysTrapFrmCustomResponseAlert);
							
extern FrmGraffitiStateType *FrmNewGsi (FormType **formPP, Coord x, Coord y)
							SYS_TRAP(sysTrapFrmNewGsi);

#define FrmSaveActiveState(stateP)			FrmActiveState(stateP, true)
#define FrmRestoreActiveState(stateP)		FrmActiveState(stateP, false)

				
UInt16	FrmGetDIAPolicyAttr (FormPtr formP)
							PINS_TRAP(pinFrmGetDIAPolicyAttr);

Err		FrmSetDIAPolicyAttr (FormPtr formP, UInt16 diaPolicy)
							PINS_TRAP(pinFrmSetDIAPolicyAttr);

Int16 FrmObjectRightAlign(FormType *formP, UInt16 objIndex, Int16 d);
Int16 FrmObjectBottomAlign(FormType *formP, UInt16 objIndex, Int16 d);

#ifdef __cplusplus
}
#endif

#endif // __FORM_H__
