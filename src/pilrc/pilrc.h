/*
 * @(#)pilrc.h
 *
 * Copyright 1997-1999, Wes Cherry   (mailto:wesc@technosis.com)
 *           2000-2005, Aaron Ardiri (mailto:aaron@ardiri.com)
 * All rights reserved.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;  either version 2, or (at your option)
 * any version.
 *
 * This program is distributed in the hope that it will be useful,  but
 * WITHOUT  ANY  WARRANTY;   without  even   the  implied  warranty  of 
 * MERCHANTABILITY  or FITNESS FOR A  PARTICULAR  PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You  should have  received a  copy of the GNU General Public License
 * along with this program;  if not,  please write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Revisions:
 * ==========
 *
 * pre 18-Jun-2000 <numerous developers>
 *                 creation
 *     18-Jun-2000 Aaron Ardiri
 *                 GNU GPL documentation additions
 *     23-Jun-2000 Mark Garlanger
 *                 Additions to support #ifdef/#ifndef/#else/#endif in
 *                 both .rcp files and .h files.
 *     20-Nov-2000 Renaud Malaval
 *                 additions PalmOS 3.5 support
 *                 additions to support LE32 resouces
 *     03-Jun-2002 Renaud Malaval
 *                 additions of SEARCH flag in list
 *     22-Aug-2002 Ben Combee
 *                 Added dependency tracking and better AUTOID support
 */

#ifndef _pilrc_h
#define _pilrc_h                                 // RMA : multiples include protection

//#if (SIZEOF_INT == SIZEOF_CHAR_P)
//typedef int p_int;
//#elif (SIZEOF_LONG == SIZEOF_CHAR_P)
typedef long p_int;
//#endif

#ifndef strdup
char *strdup(const char *s);
#endif

#include "std.h"
#include "util.h"
#include "lex.h"
#include "plex.h"
#include "font.h"

/*-----------------------------------------------------------------------------
|	 PILOT STRUCTS
|
|
|	a note on the strings following some of the following structures:
|
|	the strings define how to dump the structure out in the same format as the
|	pilot structs.  This way you do not have to include pilot include files
|	which are definitely not compiler/processor independent.  
|
|	note:  we make the assumption that sizeof(p_int) == sizeof(void *) -- the emitter
|	moves through the structs as if they were an array of ints and emits the struct
|	according to the template
|
|	template syntax:
|
|	base types:
|		b : byte (8 bits)
|		w : word (16 bits)
|		l : long (32 bits)
|		t : bit (1 bit of 8 bits)
|		u : bits (1 bits of 16 bits)		// RMa additions
|		p : pointer (32 bits)
|
|	prefixes: (optional)
|		z : zero -- there is no corresponding field in our structs -- emit the
|			base type filled w/ 0
|
|	suffixes: (optional)
|		# : (decimal numeric) --  number of times to repeat the previous type.
|			For bits (t) this specifies the number of bits to shift in.
|			Note that the bits must not cross byte boundaries in any one field emission.
-------------------------------------------------------------WESC------------*/

#define leftAlign 0
#define centerAlign 1
#define rightAlign 2

#define noButtonFrame 0
#define standardButtonFrame 1
#define boldButtonFrame 2
#define rectangleButtonFrame 3

#define Enum(type) typedef enum type
#define EndEnum

Enum(ControlStyles)
{
  buttonCtl, pushButtonCtl, checkboxCtl, popupTriggerCtl, selectorTriggerCtl, repeatingButtonCtl, sliderCtl, feedbackSliderCtl  /* RMa add support sliders */
}

EndEnum ControlStyles;

Enum(AlertType)
{
informationAlert, confirmationAlert, warningAlert, errorAlert}

EndEnum AlertType;

Enum(FormObjectKind)
{
  frmFieldObj, frmControlObj, frmListObj, frmTableObj, frmBitmapObj, frmLineObj, frmFrameObj, frmRectangleObj, frmLabelObj, frmTitleObj, frmPopupObj, frmGraffitiStateObj, frmGadgetObj, frmScrollBarObj, frmSliderObj, /* RMa add support sliders */
    frmGraphicalControlObj                       /* RMa add support graphical control */
}

EndEnum FormObjectKind;

Enum(FontID)
{
stdFont, boldFont, largeFont, symbolFont, symbol11Font, symbol7Font, ledFont}

EndEnum FontID;

typedef struct RCPOINT
{
  p_int x;
  p_int y;
}
RCPOINT;

typedef struct RCRECT
{
  RCPOINT topLeft;
  RCPOINT extent;
}
RCRECT;

/************************************************\
 * RMa update all structure to be compleant with
 *  PalmOS 3.5
 *  LE32 ( 16 and 32 bits memory alignment)
\************************************************/

/*
 * window.h 
 */
typedef struct RCFRAMEBITS
{
  p_int cornerDiam;                              /* u8   */
  /*
   * p_int reserved;           zu3 
   */
  p_int threeD;                                  /* u   */
  p_int shadowWidth;                             /* u2  */
  p_int width;                                   /* u2  */
}
RCFRAMEBITS;                                     /* u8zu3uu2u2 */

typedef struct RCWINDOWFLAGS
{
  /*
   * Word format:1;      zu 
   */
  /*
   * Word offscreen:1;   zu 
   */
  p_int modal;                                   /* u  */
  p_int focusable;                               /* u  */
  /*
   * Word enabled:1;    zu 
   */
  /*
   * Word visible:1;     zu  
   */
  p_int dialog;                                  /* u */
  /*
   * Word freeBitmap:1   zu 
   */
  /*
   * Word reserved :8;   zu8 
   */
}
RCWINDOWFLAGS;                                   /* zuzuuu zuzuuzu zu8 */

typedef struct RCWindowBA16Type
{
  p_int displayWidth;                            /* w */
  p_int displayHeight;                           /* w */
  /*
   * p_int displayAddr;                                        zl 
   */
  RCWINDOWFLAGS windowFlags;                     /* zuzuuu zuzuuzu zu8 */
  RCRECT windowBounds;                           /* w4 */
  /*
   * RCRECT clippingBounds;                          zw4 
   */
  /*
   * BitmapPtr bitmapP;                              zl 
   */
  RCFRAMEBITS frameType;                         /* u8zu3uu2u2 */
  /*
   * GraphicStatePtr drawStateP;             zl 
   */
  /*
   * struct RCWindowBA16Type *nextWindow; zl 
   */
}
RCWindowBA16Type;

#define szRCWindowBA16 "w,w,zl,zuzuuuzuzuuzuzu8,w4,zw4,zl,u8zu3uu2u2,zl,zl"

typedef struct RCWindowBA32Type
{
  p_int displayWidth;                            /* w */
  p_int displayHeight;                           /* w */
  /*
   * p_int displayAddr;                                        zl 
   */
  RCRECT windowBounds;                           /* w4 */
  /*
   * RCRECT clippingBounds;                          zw4 
   */
  /*
   * BitmapPtr bitmapP;                              zl 
   */
  RCWINDOWFLAGS windowFlags;                     /* zuzuuu zuzuuzu zu8 */
  RCFRAMEBITS frameType;                         /* u8zu3uu2u2 */
  /*
   * GraphicStatePtr drawStateP;             zl 
   */
  /*
   * struct RCWindow32Type *nextWindow;  zl 
   */
}
RCWindowBA32Type;

#define szRCWindowBA32 "w,w,zl,w4,zw4,zl,zuzuuuzuzuuzuzu8,u8zu3uu2u2,zl,zl"

/*
 * ------------------ List ------------------ 
 */
typedef struct RCLISTATTR
{
  p_int usable;                                  /* u */
  p_int enabled;                                 /* u */
  p_int visible;                                 /* u */
  p_int poppedUp;                                /* u */
  p_int hasScrollBar;                            /* u */
  p_int search;                                  /* u */
  /*
   * p_int reserved;           zu10 
   */
}
RCLISTATTR;                                      /* uuuuuuzu10 */

typedef struct RCListBA16Type
{
  p_int id;                                      /* w */
  RCRECT bounds;                                 /* w4 */
  RCLISTATTR attr;                               /* uuuuuuzu10 */
  char *itemsText;                               /* p */
  p_int numItems;                                /* w */
  /*
   * Word currentItem;       zw 
   */
  /*
   * Word topItem;                   zw 
   */
  p_int font;                                    /* b  */
  /*
   * UInt8 reserved;             zb 
   */
  /*
   * WinHandle popupWin;     zl 
   */
  /*
   * ListDrawDataFuncPtr  drawItemsCallback;  zl 
   */
  /*
   * private, not stored into file 
   */
  int cbListItems;
}
RCListBA16Type;

#define szRCListBA16 "w,w4,uuuuuuzu10,p,w,zw,zw,bzb,zl,zl"

typedef struct RCListBA32Type
{
  p_int id;                                      /* w */
  RCLISTATTR attr;                               /* uuuuuuzu10 */
  RCRECT bounds;                                 /* w4 */
  char *itemsText;                               /* p */
  p_int numItems;                                /* w */
  /*
   * Word currentItem;       zw 
   */
  /*
   * Word topItem;                   zw 
   */
  p_int font;                                    /* b  */
  /*
   * UInt8 reserved;             zb 
   */
  /*
   * WinHandle popupWin;     zl 
   */
  /*
   * ListDrawDataFuncPtr  drawItemsCallback;  zl 
   */
  /*
   * private, not stored into file 
   */
  int cbListItems;
}
RCListBA32Type;

#define szRCListBA32 "w,uuuuuuzu10,w4,p,w,zw,zw,bzb,zl,zl"

typedef union RCListType
{
  RCListBA16Type s16;
  RCListBA32Type s32;
}
RCListType;

/*
 * ------------------ Field ------------------ 
 */
typedef struct RCFIELDATTR
{
  p_int usable;                                  /* u  */
  p_int visible;                                 /* u  */
  p_int editable;                                /* u  */
  p_int singleLine;                              /* u  */

  p_int hasFocus;                                /* u  */
  p_int dynamicSize;                             /* u  */
  p_int insPtVisible;                            /* u  */
  p_int dirty;                                   /* u  */

  p_int underlined;                              /* u2 */
  p_int justification;                           /* u2 */
  p_int autoShift;                               /* u  */
  p_int hasScrollBar;                            /* u  */
  p_int numeric;                                 /* u  */
  /*
   * zu 
   */
}
RCFIELDATTR;                                     /* uuuuuuuu u2u2uuuzu */

typedef struct RCFieldBA16Type
{
  p_int id;                                      /* w */
  RCRECT rect;                                   /* w4 */
  RCFIELDATTR attr;                              /* uuuuuuuu u2u2uuuzu */
  char *text;                                    /* p */
  /*
   * VoidHand                    zl       textHandle;                      block the contains the text string 
   */
  /*
   * LineInfoPtr         zl       lines; 
   */
  /*
   * Word                        zw       textLen; 
   */
  /*
   * Word                        zw       textBlockSize; 
   */
  p_int maxChars;                                /* w */
  /*
   * p_int selFirstPos;    zw  
   */
  /*
   * p_int selLastPos;     zw 
   */
  /*
   * p_int insPtXPos;      zw 
   */
  /*
   * p_int insPtYPos;      zw 
   */
  p_int fontID;                                  /* b */
  /*
   * p_int reserved;           zb 
   */
}
RCFieldBA16Type;

#define szRCFieldBA16 "w,w4,uuuuuuuu,u2u2uuuzu,p,zl,zl,zw,zw,w,zw,zw,zw,zw,b,zb"

typedef struct RCFieldBA32Type
{
  RCRECT rect;                                   /* w4 */
  p_int id;                                      /* w */
  RCFIELDATTR attr;                              /* uuuuuuuu u2u2uuuzu */
  char *text;                                    /* p */
  /*
   * VoidHand                    zl       textHandle;                      block the contains the text string 
   */
  /*
   * LineInfoPtr         zl       lines; 
   */
  /*
   * Word                        zw       textLen; 
   */
  /*
   * Word                        zw       textBlockSize; 
   */
  p_int maxChars;                                /* w */
  /*
   * p_int selFirstPos;    zw  
   */
  /*
   * p_int selLastPos;     zw 
   */
  /*
   * p_int insPtXPos;      zw 
   */
  /*
   * p_int insPtYPos;      zw 
   */
  p_int fontID;                                  /* b */
  /*
   * p_int reserved;           zb 
   */
}
RCFieldBA32Type;

#define szRCFieldBA32 "w4,w,uuuuuuuu,u2u2uuuzu,p,zl,zl,zw,zw,w,zw,zw,zw,zw,b,zb"

typedef union RCFieldType
{
  RCFieldBA16Type s16;
  RCFieldBA32Type s32;
}
RCFieldType;

/*
 * ------------------ Table ------------------ 
 */
typedef struct RCTableColumnAttrBA16Type
{
  p_int width;                                   /* w  */
  /*
   * p_int reserved1;                                              zt5 
   */
  p_int masked;                                  /* t  */
  p_int editIndicator;                           /* t  */
  p_int usable;                                  /* t */
  /*
   * p_int reserved2;                                                  zb 
   */
  p_int spacing;                                 /* w  */
  /*
   * TableDrawItemFuncPtr drawCallback;           zl 
   */
  /*
   * TableLoadDataFuncPtr loadDataCallback;   zl 
   */
  /*
   * TableSaveDataFuncPtr saveDataCallback;   zl 
   */
}
RCTableColumnAttrBA16Type;

#define szRCTableColumnAttrBA16 "w,zt5tttzb,w,zl,zl,zl"

typedef struct RCTableColumnAttrBA32Type
{
  p_int width;                                   /* w  */
  p_int spacing;                                 /* w  */
  /*
   * p_int reserved1;                                              zt5 
   */
  p_int masked;                                  /* t  */
  p_int editIndicator;                           /* t  */
  p_int usable;                                  /* t */
  /*
   * p_int reserved2;                                                  zb 
   */
  /*
   * p_int reserved3;                                                  zw 
   */
  /*
   * TableDrawItemFuncPtr drawCallback;                   zl 
   */
  /*
   * TableLoadDataFuncPtr loadDataCallback;   zl 
   */
  /*
   * TableSaveDataFuncPtr saveDataCallback;   zl 
   */
}
RCTableColumnAttrBA32Type;

#define szRCTableColumnAttrBA32 "w,w,zt5tttzb,zw,zl,zl,zl"

#define szRCTABLECOLUMNATTR (vfLE32?szRCTableColumnAttrBA32:szRCTableColumnAttrBA16)
typedef union RCTABLECOLUMNATTR
{
  RCTableColumnAttrBA16Type s16;
  RCTableColumnAttrBA32Type s32;
}
RCTABLECOLUMNATTR;

typedef struct RCTABLEROWATTR
{
  p_int id;                                      /* w  */
  p_int height;                                  /* w  */
  /*
   * DWord data;             zl 
   */
  /*
   * p_int reserved1;                  zt7 
   */
  p_int usable;                                  /* t  */
  /*
   * p_int reserved2;                  zt4 
   */
  p_int masked;                                  /* t  */
  p_int invalid;                                 /* t  */
  p_int staticHeight;                            /* t  */
  p_int selectable;                              /* t  */
  /*
   * p_int reserved3;                  zw 
   */
}
RCTABLEROWATTR;

#define szRCTABLEROWATTR "w,w,zl,zt7t,zt4tttt,zw"

/*
 * this is bogus...don't know why 
 */
#define szRCTABLEPADDING "zl,zl"

typedef struct RCTABLEATTR
{
  p_int visible;                                 /* u */
  p_int editable;                                /* u */
  p_int editing;                                 /* u */
  p_int selected;                                /* u */
  p_int hasScrollBar;                            /* u */
  /*
   * p_int reserved;                   zu11 
   */
}
RCTABLEATTR;                                     /* uuuu uzu11 */

typedef struct RCTableBA16Type
{
  p_int id;                                      /* w  */
  RCRECT bounds;                                 /* w4 */
  RCTABLEATTR attr;                              /* uuuu uzu11 */
  p_int numColumns;                              /* w */
  p_int numRows;                                 /* w */
  p_int currentRow;                              /* w */
  p_int currentColumn;                           /* w */
  p_int topRow;                                  /* w */
  /*
   * TableColumnAttrType * columnAttrs;           zl 
   */
  /*
   * TableRowAttrType *      rowAttrs;            zl 
   */
  /*
   * TableItemPtr            items;                   zl 
   */
  RCFieldBA16Type currentField;
  /*
   * not emitted 
   */
  p_int *rgdxcol;
}
RCTableBA16Type;

#define szRCTableBA16 "w,w4,uuuuuzu11,w,w,w,w,w,zl,zl,zl" szRCFieldBA16
typedef struct RCTableBA32Type
{
  p_int id;                                      /* w  */
  RCTABLEATTR attr;                              /* uuuu uzu11 */
  RCRECT bounds;                                 /* w4 */
  p_int numColumns;                              /* w */
  p_int numRows;                                 /* w */
  p_int currentRow;                              /* w */
  p_int currentColumn;                           /* w */
  p_int topRow;                                  /* w */
  /*
   * p_int reserved                                    zw 
   */
  /*
   * TableColumnAttrType *   columnAttrs; zl 
   */
  /*
   * TableRowAttrType *      rowAttrs;            zl 
   */
  /*
   * TableItemPtr            items;                    zl 
   */
  RCFieldBA32Type currentField;
  /*
   * not emitted 
   */
  p_int *rgdxcol;
}
RCTableBA32Type;

#define szRCTableBA32 "w,uuuuuzu11,w4,w,w,w,w,w,zw,zl,zl,zl" szRCFieldBA32

typedef union RCTableType
{
  RCTableBA16Type s16;
  RCTableBA32Type s32;
}
RCTableType;

/*
 * ------------------ Form Label ------------------ 
 */

/*
 * form.h 
 */
typedef struct RCFORMOBJATTR
{
  p_int usable;                                  /* t,zt7 (opt) */
  /*
   * p_int reserved            zb 
   */
}
RCFORMOBJATTR;                                   /* tzt7,zb */

typedef struct RCFormLabelBA16Type
{
  p_int id;                                      /* w */
  RCPOINT pos;                                   /* w2 */
  RCFORMOBJATTR attr;                            /* uzu15 */
  p_int fontID;                                  /* b */
  /*
   * p_int reserved        zb 
   */
  char *text;                                    /* p */
}
RCFormLabelBA16Type;

#define szRCFormLabelBA16 "w,w2,uzu15,b,zb,p"

typedef struct RCFormLabelBA32Type
{
  RCPOINT pos;                                   /* w2 */
  char *text;                                    /* p */
  p_int id;                                      /* w */
  RCFORMOBJATTR attr;                            /* uzu15 */
  p_int fontID;                                  /* b */
  /*
   * p_int reserved        zb 
   */
  /*
   * p_int padding;            zw 
   */
}
RCFormLabelBA32Type;

#define szRCFormLabelBA32 "w2,p,w,uzu15,b,zb,zw"

typedef union RCFormLabelType
{
  RCFormLabelBA16Type s16;
  RCFormLabelBA32Type s32;
}
RCFormLabelType;

/*
 * ------------------ Form Title ------------------ 
 */
typedef struct RCFORMTITLE
{
  RCRECT rect;                                   /* w4 */
  char *text;                                    /* p */
}
RCFORMTITLE;

#define szRCFORMTITLE "w4,p"

/*
 * ------------------ Form Popup ------------------ 
 */
typedef struct RCFORMPOPUP
{
  p_int controlID;                               /* w */
  p_int listID;                                  /* w */
}
RCFORMPOPUP;

#define szRCFORMPOPUP "ww"

/*
 * ------------------ Form Graffiti State ------------------ 
 */
typedef struct RCFORMGRAFFITISTATE
{
  RCPOINT pos;                                   /* w2 */
}
RCFORMGRAFFITISTATE;

#define szRCFORMGRAFFITISTATE "w2"

/*
 * ------------------ Form Gadget ------------------ 
 */
typedef struct RCFormGadgetAttr
{
  p_int usable;                                  /* u */
  p_int extended;                                /* u */
  p_int visible;                                 /* u */
  /*
   * p_int reserved            zu13 
   */
}
RCFormGadgetAttr;                                /* uuuzu13 */

typedef struct RCFORMGADGET
{
  p_int id;                                      /* w */
  RCFormGadgetAttr attr;                         /* uuuzu13 */
  RCRECT rect;                                   /* w4 */
  /*
   * VoidPtr data;                                   zl 
   */
  /*
   * FormGadgetHandlerType        *handler   zl 
   */
}
RCFORMGADGET;

#define szRCFORMGADGET "w,uuuzu13,w4,zl,zl"

/*
 * ------------------ Form Bitmap ------------------ 
 */
typedef struct RCFormBitMapBA16Type
{
  RCFORMOBJATTR attr;                            /* uzu15 */
  RCPOINT pos;                                   /* w2 */
  p_int rscID;                                   /* w */
}
RCFormBitMapBA16Type;

#define szRCFormBitMapBA16 "uzu15,w2,w"

typedef struct RCFormBitMapBA32Type
{
  RCFORMOBJATTR attr;                            /* uzu15 */
  p_int rscID;                                   /* w */
  RCPOINT pos;                                   /* w2 */
}
RCFormBitMapBA32Type;

#define szRCFormBitMapBA32 "uzu15,w,w2"

typedef union RCFormBitMapType
{
  RCFormBitMapBA16Type s16;
  RCFormBitMapBA32Type s32;
}
RCFormBitMapType;

/*
 * ------------------ Control ------------------ 
 */
typedef struct RCCONTROLATTR
{
  p_int usable;                                  /* u  */
  p_int enabled;                                 /* u  */
  p_int visible;                                 /* u  *//* OS use internal */
  p_int on;                                      /* u  */
  p_int leftAnchor;                              /* u  */
  p_int frame;                                   /* u3 */

  p_int drawnAsSelected;                         /* u  */
  p_int graphical;                               /* u  */
  p_int vertical;                                /* u  */
  /*
   * p_int reserved;                   zu5 
   */
}
RCCONTROLATTR;                                   /* uuuuuu3 uuuzu5 */

typedef struct RCControlBA16Type
{
  p_int id;                                      /* w */
  RCRECT bounds;                                 /* w4 */
  p_int bitmapid;                                /* s / w */
  p_int selectedbitmapid;                        /* s / w */
  char *text;                                    /* p / s */
  RCCONTROLATTR attr;                            /* uuuuuu3 uuuzu5 */
  p_int style;                                   /* b */
  p_int font;                                    /* b */
  p_int group;                                   /* b */
}
RCControlBA16Type;

#define szRCControlBA16          "w,w4,ssp,uuuuuu3,uuuzu5,b,b,b,zb"
#define szRCGraphicalControlBA16 "w,w4,wws,uuuuuu3,uuuzu5,b,b,b,zb"

typedef struct RCControlBA32Type
{
  p_int id;                                      /* w */
  RCCONTROLATTR attr;                            /* uuuuuu3 uuuzu5 */
  RCRECT bounds;                                 /* w4 */
  p_int bitmapid;                                /* s / w */
  p_int selectedbitmapid;                        /* s / w */
  char *text;                                    /* p / s */
  p_int style;                                   /* b */
  p_int font;                                    /* b */
  p_int group;                                   /* b */
  /*
   * p_int reserved;                           zb 
   */
}
RCControlBA32Type;

#define szRCControlBA32          "w,uuuuuu3,uuuzu5,w4,ssp,b,b,b,zb"
#define szRCGraphicalControlBA32 "w,uuuuuu3,uuuzu5,w4,wws,b,b,b,zb"

typedef union RCControlType
{
  RCControlBA16Type s16;
  RCControlBA32Type s32;
}
RCControlType;

/*
 * ------------------ Slider ------------------ 
 */
typedef struct RCSliderControlBA16Type
{
  p_int id;                                      /* w  */
  RCRECT bounds;                                 /* w4 */
  p_int thumbid;                                 /* w */// overlays text in ControlBA16Type
  p_int backgroundid;                            /* w */// overlays text in ControlBA16Type
  RCCONTROLATTR attr;                            /* uuuuuu3,uuuzu5 */// graphical *is* set
  p_int style;                                   /* b */// must be sliderCtl or repeatingSliderCtl
  /*
   * UInt8                        reserved;                  zb 
   */
  p_int minValue;                                /* w */
  p_int maxValue;                                /* w */
  p_int pageSize;                                /* w *//* pageJump == pageSize */
  p_int value;                                   /* w */
  /*
   * MemPtr                       activeSliderP;     zl 
   */
}
RCSliderControlBA16Type;

#define szRCSliderControlBA16 "w,w4,w,w,uuuuuu3,uuuzu5,b,zb,w,w,w,w,zl"

typedef struct RCSliderControlBA32Type
{
  p_int id;                                      /* w */
  RCCONTROLATTR attr;                            /* uuuuuu3,uuuzu5 */// graphical *is* set
  RCRECT bounds;                                 /* w4 */
  p_int thumbid;                                 /* w  */// overlays text in ControlBA16Type
  p_int backgroundid;                            /* w  */// overlays text in ControlBA16Type
  p_int style;                                   /* b  */// must be sliderCtl or repeatingSliderCtl
  /*
   * UInt8                        reserved;                  zb 
   */
  p_int minValue;                                /* w */
  p_int maxValue;                                /* w */
  p_int pageSize;                                /* w */
  p_int value;                                   /* w */
  /*
   * p_int                          reserved2;                 zw 
   */
  /*
   * MemPtr                       activeSliderP;     zl 
   */
}
RCSliderControlBA32Type;

#define szRCSliderControlBA32 "w,uuuuuu3,uuuzu5,w4,w,w,b,zb,w,w,w,w,zw,zl"

typedef union RCSliderControlType
{
  RCSliderControlBA16Type s16;
  RCSliderControlBA32Type s32;
}
RCSliderControlType;

/*
 * ------------------ ScrollBar ------------------ 
 */
typedef struct RCSCROLLBARATTR
{
  p_int usable;                                  /* t */
  p_int visible;                                 /* t */
  p_int hilighted;                               /* t */
  p_int shown;                                   /* t */
  p_int activeRegion;                            /* t4 */
  /*
   * p_int reserved;                   zb 
   */
}
RCSCROLLBARATTR;

typedef struct RCSCROLLBAR
{
  RCRECT bounds;                                 /* w4 */
  p_int id;                                      /* w */
  RCSCROLLBARATTR attr;                          /* ttttt4,zb */
  p_int value;                                   /* w */
  p_int minValue;                                /* w */
  p_int maxValue;                                /* w */
  p_int pageSize;                                /* w */
  /*
   * Short penPosInCar;
   */
  /*
   * zw 
   */
  /*
   * Short savePos;
   */
  /*
   * zw 
   */
}
RCSCROLLBAR;

#define szRCSCROLLBAR "w4,w,ttttt4,zb,w,w,w,w,zw,zw"

/*
 * ------------------ Forms ------------------ 
 */
typedef union RCFORMOBJECT
{
  void *ptr;
  RCFieldType *field;
  RCControlType *control;
  RCSliderControlType *slider;
  RCListType *list;
  RCTableType *table;
  RCFormBitMapType *bitmap;
  RCFormLabelType *label;

  /*
   * RCFORMLINE *                         line; 
   */

  /*
   * RCFORMFRAME *                        frame; 
   */

  /*
   * RCFORMRECTANGLE *            rectangle; 
   */
  RCFORMTITLE *title;
  RCFORMPOPUP *popup;
  RCFORMGRAFFITISTATE *grfState;
  RCFORMGADGET *gadget;
  RCSCROLLBAR *scrollbar;
}
RCFORMOBJECT;

typedef struct RCFormObjListBA16Type
{
  p_int objectType;                              /* b  */
  /*
   * p_int reserved;                                   zb 
   */
  union
  {
    RCFORMOBJECT object;                         /* l */
    p_int ibobj;
  }
  u;
}
RCFormObjListBA16Type;

#define szRCFormObjListBA16 "b,zb,l"

typedef struct RCFormObjListBA32Type
{
  union
  {
    RCFORMOBJECT object;                         /* l */
    p_int ibobj;
  }
  u;
  p_int objectType;                              /* b */
  /*
   * p_int reserved;                                   zb 
   */
  /*
   * p_int padding;                            zw 
   */
}
RCFormObjListBA32Type;

#define szRCFormObjListBA32 "l,b,zb,zw"

#define szRCFORMOBJLIST (vfLE32?szRCFormObjListBA32:szRCFormObjListBA16)
typedef union RCFORMOBJLIST
{
  RCFormObjListBA16Type s16;
  RCFormObjListBA32Type s32;
}
RCFORMOBJLIST;

typedef struct RCFORMATTR
{
  p_int usable;                                  /* u */
  p_int enabled;                                 /* u */
  p_int visible;                                 /* u */
  p_int dirty;                                   /* u */
  p_int saveBehind;                              /* u */
  p_int graffitiShift;                           /* u */
  p_int globalsAvailable;                        /* u */
  p_int doingDialog;                             /* u */
  p_int exitDialog;                              /* u */
  /*
   * p_int reserved;                           zu7 
   */
  /*
   * p_int reserved2;                  zw 
   */
}
RCFORMATTR;                                      /* uuuuuuuu,uzu7,zw */

typedef struct RCFormBA16Type
{
  RCWindowBA16Type window;
  p_int formId;                                  /* w  */
  RCFORMATTR attr;                               /* uuuuuuuu uzu7 zw */
  /*
   * WinHandle bitsBehindForm;               zl 
   */
  /*
   * FormEventHandlerPtr handler;            zl 
   */
  /*
   * Word focus;                                             zw 
   */
  p_int defaultButton;                           /* w  */
  p_int helpRscId;                               /* w  */
  p_int menuRscId;                               /* w  */
  p_int numObjects;                              /* w  */
  /*
   * RCFormObjListBA16Type *objects;     zl 
   */
}
RCFormBA16Type;

#define szRCFormBA16 szRCWindowBA16 ",w,uuuuuuuuuzu7,zw,zl,zl,zw,w,w,w,w,zl"
typedef struct RCFormBA32Type
{
  RCWindowBA32Type window;
  RCFORMATTR attr;                               /* uuuuuuuu uzu7 zw */
  /*
   * WinHandle bitsBehindForm;               zl 
   */
  /*
   * FormEventHandlerPtr handler;            zl 
   */
  p_int formId;                                  /* w  */
  /*
   * Word focus;                                             zw 
   */
  p_int defaultButton;                           /* w  */
  p_int helpRscId;                               /* w  */
  p_int menuRscId;                               /* w  */
  p_int numObjects;                              /* w  */
  /*
   * RCFormObjListBA32Type *objects;         zl 
   */
}
RCFormBA32Type;

#define szRCFormBA32 szRCWindowBA32 ",uuuuuuuuuzu7,zw,zl,zl,w,zw,w,w,w,w,zl"

#define szRCFORM (vfLE32?szRCFormBA32:szRCFormBA16)

/*-----------------------------------------------------------------------------
|	MENUS
-------------------------------------------------------------WESC------------*/

typedef struct RCMENUITEM
{
  p_int id;                                      /* w */
  p_int command;                                 /* b */
  p_int hidden;                                  /* t */
  /*
   * p_int reserved;         zt7 
   */
  char *itemStr;                                 /* l */
}
RCMENUITEM;

#define szRCMENUITEM "w,b,tzt7,l"

typedef struct RCMenuPullDownBA16Type
{
  /*
   * WinHandle menuWin;       zl 
   */
  RCRECT bounds;                                 /* w4 */
  /*
   * WinHandle bitsBehind;    zl 
   */
  RCRECT titleBounds;                            /* w4 */
  char *title;                                   /* l */
  p_int hidden;                                  /* u */
  p_int numItems;                                /* u15 */
  RCMENUITEM *items;                             /* l */
}
RCMenuPullDownBA16Type;

#define szRCMenuPullDownBA16 "zl,w4,zl,w4,l,uu15,l"

typedef struct RCMenuPullDownBA32Type
{
  /*
   * WinHandle menuWin;       zl 
   */
  RCRECT bounds;                                 /* w4 */
  /*
   * WinHandle bitsBehind;    zl 
   */
  RCRECT titleBounds;                            /* w4 */
  char *title;                                   /* l  */
  p_int hidden;                                  /* u  */
  /*
   * p_int reserved                         zu15 
   */
  p_int numItems;                                /*     w  */
  RCMENUITEM *items;                             /* l  */
}
RCMenuPullDownBA32Type;

#define szRCMenuPullDownBA32 "zl,w4,zl,w4,l,uzu15,w,l"

#define szRCMENUPULLDOWN (vfLE32?szRCMenuPullDownBA32:szRCMenuPullDownBA16)
typedef union RCMENUPULLDOWN
{
  RCMenuPullDownBA16Type s16;
  RCMenuPullDownBA32Type s32;
}
RCMENUPULLDOWN;

typedef struct RCMENUBARATTR
{
  p_int visible;                                 /* u  */
  /*
   * WORD commandPending;    zu 
   */
  /*
   * WORD insPtEnabled;      zu 
   */
  /*
   * p_int needsRecalc;                zu 
   */
  /*
   * p_int reserved;                           zu12 
   */
}
RCMENUBARATTR;

typedef struct RCMenuBarBA16Type
{
  /*
   * WinHandle barWin;                               zl 
   */
  /*
   * WinHandle bitsBehind;                           zl 
   */
  /*
   * WinHandle savedActiveWin;               zl 
   */
  /*
   * WinHandle bitsBehindStatus;     zl 
   */
  RCMENUBARATTR attr;                            /* uzu3zu12 */
  /*
   * SWord curMenu;                                  zw 
   */
  p_int curItem;                                 /* w  */
  /*
   * SDWord commandTick;                     zl 
   */
  p_int numMenus;                                /* w   number of menus */
  /*
   * MenuPullDownPtr menus;                  zl  array of menus 
   */
}
RCMenuBarBA16Type;

#define szRCMenuBarBA16 "zl,zl,zl,zl,uzu3zu12,zw,w,zl,w,zl"
typedef struct RCMenuBarBA32Type
{
  /*
   * WinHandle barWin;                               zl 
   */
  /*
   * WinHandle bitsBehind;                           zl 
   */
  /*
   * WinHandle savedActiveWin;               zl 
   */
  /*
   * WinHandle bitsBehindStatus;     zl 
   */
  RCMENUBARATTR attr;                            /* uzu3zu12 */
  /*
   * SWord curMenu;                                  zw 
   */
  p_int curItem;                                 /* w  */
  p_int numMenus;                                /* w   number of menus */
  /*
   * SDWord commandTick;                     zl 
   */
  /*
   * MenuPullDownPtr menus;                  zl  array of menus 
   */
}
RCMenuBarBA32Type;

#define szRCMenuBarBA32 "zl,zl,zl,zl,uzu3zu12,zw,w,w,zl,zl"

#define szRCMENUBAR (vfLE32?szRCMenuBarBA32:szRCMenuBarBA16)
typedef union RCMENUBAR
{
  RCMenuBarBA16Type s16;
  RCMenuBarBA32Type s32;
}
RCMENUBAR;

/*-----------------------------------------------------------------------------
|	ALERTS
-------------------------------------------------------------WESC------------*/

typedef struct RCALERTTEMPLATE
{
  p_int alertType;                               /* w */
  p_int helpRscID;                               /* w */
  p_int numButtons;                              /* w */
  p_int defaultButton;                           /* w */
}
RCALERTTEMPLATE;

#define szRCALERTTEMPLATE "w,w,w,w"

/*-----------------------------------------------------------------------------
|	BITMAP
-------------------------------------------------------------WESC------------*/
typedef struct RCBitmapFlagsType
{
  p_int compressed;                              /* u *//* Data format:  0=raw; 1=compressed */
  p_int hasColorTable;                           /* u *//* if true, color table stored before bits[] */
  p_int hasTransparency;                         /* u *//* true if transparency is used */
  p_int indirect;                                /* u *//* true if bits are stored indirectly */
  /*
   * Never set this flag. Only the display (screen) bitmap has the indirect bit set. 
   */
  p_int forScreen;                               /* u *//* system use only */
  p_int directColor;                             /* u  *//* direct color bitmap */

  p_int indirectColorTable;                      /* u  *//* if true, color table pointer follows BitmapType structure */
  p_int noDither;                                /* u  *//* if true, blitter does not dither */
  //p_int reserved;                 /* zu8 */ 
}
RCBitmapFlagsType;

typedef struct RCBITMAP
{                                                /* b */
  p_int cx;                                      /* w */
  p_int cy;                                      /* w */
  p_int cbRow;                                   /* w */
  RCBitmapFlagsType flags;                       /* uuuuuuuuzu8 *//* RMa struct updated */
  p_int pixelsize;                               /* b */
  p_int version;                                 /* b */
  p_int nextDepthOffset;                         /* w */
  p_int transparentIndex;                        /* b */
  p_int compressionType;                         /* b */
  /*
   * ushort and_reserved_and_colorTable[3] 
   */
  /*
   * z1w 
   */
  unsigned char *pbBits;

  /*
   * private, not stored into file 
   */
  int cbDst;
}
RCBITMAP;

#define szRCBITMAP "w,w,w,uuuuuuuuzu8,b,b,w,b,b,zw"

/*
 * RMa add support for density > 1
 */
Enum(PixelFormatBV3Type)
{
  pixelFormatIndexed, pixelFormat565, pixelFormat565LE, // not used by 68K-based OS
    pixelFormatIndexedLE                         // not used by 68K-based OS
}

EndEnum PixelFormatBV3Type;

typedef struct RCBITMAP_V3
{                                                /* bm */
  p_int cx;                                      /* w */
  p_int cy;                                      /* w */
  p_int cbRow;                                   /* w */
  RCBitmapFlagsType flags;                       /* uuuuuuuuzu8 */
  p_int pixelsize;                               /* b */
  p_int version;                                 /* b */
  p_int size;                                    /* b */
  p_int pixelFormat;                             /* b */
  //  p_int unused;                                                                                        /* zb */
  p_int compressionType;                         /* b */
  p_int density;                                 /* w */
  p_int transparentValue;                        /* l */
  p_int nextDepthOffset;                         /* l */
  /*
   * ushort colorTable[3] 
   */

  unsigned char *pbBits;

  /*
   * private, not stored into file 
   */
  int cbDst;
}
RCBITMAP_V3;

#define szRCBITMAP_V3 "w,w,w,uuuuuuuuzu8,b,b,b,b,zb,b,w,l,l"

/*-----------------------------------------------------------------------------
|	FONT
-------------------------------------------------------------RMa------------*/

/*
Font Format:
Single density fonts:
---------------------
Header:
 Int16 fontType;    // font type
 Int16 firstChar;   // ASCII code of first character
 Int16 lastChar;    // ASCII code of last character
 Int16 maxWidth;    // maximum character width
 Int16 kernMax;     // negative of maximum character kern
 Int16 nDescent;    // negative of descent
 Int16 fRectWidth;  // width of font rectangle
 Int16 fRectHeight; // height of font rectangle
 Int16 owTLoc;      // offset to offset/width table
 Int16 ascent;      // ascent
 Int16 descent;     // descent
 Int16 leading;     // leading
 Int16 rowWords;    // row width of bit image / 2
 
Glyph:
 rowWords * 2 * font->header.fRectHeight bytes
 
Columns:
 array of Int16 containing the start column in the glyph bitmap
 for each char.  The start of this array is located at
 the end of the header + rowWords * 2 * font->header.fRectHeight bytes
 
CharInfoTag:
 array of FontCharInfoTag (Int8 offset, Int8 width). This is redundant with
 the width found using cols.  offset does not seem to be used. Both bytes are
 set to 0xFF if the char is missing.  The start of this array is located at
 tloc * 2 bytes after the offset of owTLoc in the header.
 

New format for high density fonts:
----------------------------------
Header:
 // first part is basically the same as NFNT FontRec
 Int16 fontType;
 Int16 firstChar;   // character code of first character
 Int16 lastChar;    // character code of last character
 Int16 maxWidth;    // widMax = maximum character width
 Int16 kernMax;     // negative of maximum character kern
 Int16 nDescent;    // negative of descent
 Int16 fRectWidth;  // width of font rectangle
 Int16 fRectHeight; // height of font rectangle
 Int16 owTLoc;      // offset to offset/width table
 Int16 ascent;      // ascent
 Int16 descent;     // descent
 Int16 leading;     // leading
 Int16 rowWords;    // row width of bit image / 2
 
 // New fields (if fntExtendedFormatMask is set)
 Int16 version;     // 1 = PalmNewFontVersion
 Int16 densityCount;// num of glygh density present 
 
 // array of 1 or more records of (Int16 density, UInt32 glyphBitsOffset)
 FontDensityType densities[0];
 
Columns:
 array of Int16 containing the start column in the glyph bitmap
 for each char.  The start of this array is located at
 the end of the header + rowWords * 2 * font->header.fRectHeight bytes
 
CharInfoTag:
 array of FontCharInfoTag (Int8 offset, Int8 width). This is redundant with
 the width found using cols.  offset does not seem to be used. Both bytes are
 set to 0xFF if the char is missing.  The start of this array is located at
 tloc * 2 bytes after the offset of owTLoc in the header.

Glyphs:
 Found using the densities array glyphBitsOffset members. Offset from the
 beginning of the font
*/

typedef struct RCFONTCHARINFOTYPE
{
  p_int offset;                                  /* b */
  p_int width;                                   /* b */
}
RCFONTCHARINFOTYPE,
 *RCFONTCHARINFOTYPEPTR;

#define szRCFONTCHARINFO "b,b"

typedef struct RCFONTTYPE
{
  p_int fontType;                                /* w */// font type
  p_int firstChar;                               /* w */// ASCII code of first character
  p_int lastChar;                                /* w */// ASCII code of last character
  p_int maxWidth;                                /* w */// maximum character width
  p_int kernMax;                                 /* w */// negative of maximum character kern
  p_int nDescent;                                /* w */// negative of descent
  p_int fRectWidth;                              /* w */// width of font rectangle
  p_int fRectHeight;                             /* w */// height of font rectangle
  p_int owTLoc;                                  /* w */// offset to offset/width table
  p_int ascent;                                  /* w */// ascent
  p_int descent;                                 /* w */// descent
  p_int leading;                                 /* w */// leading
  p_int rowWords;                                /* w */// row width of bit image / 2
  // Glyph
  // Columns
  // CharInfoTag
}
RCFONTTYPE;

#define szRCFONT "w,w,w,w,w,w,w,w,w,w,w,w,w"


typedef struct RCFONTDENSITYBA16TYPE
{
  p_int density;                                 /* w */
  p_int glyphBitsOffset;                         /* l */
}
RCFONTDENSITYBA16TYPE;

#define szRCFONTDENSITYBA16TYPE "w,l"


typedef struct RCFONTDENSITYBA32TYPE
{
  p_int density;                                 /* w */
//p_int reserved;                                /* zw */
  p_int glyphBitsOffset;                         /* l */
}
RCFONTDENSITYBA32TYPE;

#define szRCFONTDENSITYBA32TYPE "w,zw,l"


#define szRCFONTDENSITYTYPE (vfLE32?szRCFONTDENSITYBA32TYPE:szRCFONTDENSITYBA16TYPE)
typedef union RCFONTDENSITYTYPE
{
  RCFONTDENSITYBA16TYPE s16;
  RCFONTDENSITYBA32TYPE s32;
}
RCFONTDENSITYTYPE;

typedef struct RCFONT2TYPE
{
  p_int fontType;                                /* w */// font type
  p_int firstChar;                               /* w */// ASCII code of first character
  p_int lastChar;                                /* w */// ASCII code of last character
  p_int maxWidth;                                /* w */// maximum character width
  p_int kernMax;                                 /* w */// negative of maximum character kern
  p_int nDescent;                                /* w */// negative of descent
  p_int fRectWidth;                              /* w */// width of font rectangle
  p_int fRectHeight;                             /* w */// height of font rectangle
  p_int owTLoc;                                  /* w */// offset to offset/width table
  p_int ascent;                                  /* w */// ascent
  p_int descent;                                 /* w */// descent
  p_int leading;                                 /* w */// leading
  p_int rowWords;                                /* w */// row width of bit image / 2
  // New fields (if fntExtendedFormatMask is set)
  p_int version;                                 /* w */// 1 = PalmNewFontVersion
  p_int densityCount;                            /* w */

// FontDensityType densities[0];                 // array of 1 or more records of (Int16 density, UInt32 glyphBitsOffset)
// Columns
// CharInfoTag
// Glyphs
}
RCFONT2TYPE;

#define szRCFONT2 "w,w,w,w,w,w,w,w,w,w,w,w,w,w,w"


/*----------------------------------------------------------------------------
|	SYSAPPPREFS
-------------------------------------------------------------RMa------------*/
typedef struct RCSysAppPrefsBA16Type
{
  p_int priority;                                // task priority
  p_int stackSize;                               // required stack space
  p_int minHeapSpace;                            // minimum heap space required
}
RCSysAppPrefsBA16Type;

#define szRCSysAppPrefsBA16EmitStr "w,l,l"

typedef struct RCSysAppPrefsBA32Type
{
  p_int priority;                                // task priority
  //      p_int           reserved;
  p_int stackSize;                               // required stack space
  p_int minHeapSpace;                            // minimum heap space required
}
RCSysAppPrefsBA32Type;

#define szRCSysAppPrefsBA32EmitStr "w,zw,l,l"

#define szRCSYSAPPPREFS (vfLE32?szRCSysAppPrefsBA32EmitStr:szRCSysAppPrefsBA16EmitStr)
typedef union RCSYSAPPPREFS
{
  RCSysAppPrefsBA16Type s16;
  RCSysAppPrefsBA32Type s32;
}
RCSYSAPPPREFS;

/*----------------------------------------------------------------------------
|	NAVIGATION
|
| The "navigation" resource comes from the Handspring Treo 600 SDK and is
| used to describe the tab order on their forms, as well as the up/down
| relationships between items.  There is no LE32 version of this resource.
-------------------------------------------------------------BLC------------*/

typedef struct RCNAVIGATION
{
  p_int	version;
  p_int numOfObjects;
  p_int headerSizeInBytes;     /* always 20 for version 1 */
  p_int navElementSizeInBytes; /* always 8 for version 1 */
  p_int navFlags;
  p_int initialObjectIDHint;
  p_int jumpObjectIDHint;
  p_int bottomLeftObjectIDHint;
}
RCNAVIGATION;

#define szRCNAVIGATION "w,w,w,w,l,w,w,w,zw"

typedef struct RCNAVIGATIONITEM
{
  p_int	objectID;
  p_int objectFlags;
  p_int aboveObjectID;
  p_int belowObjectID;
}
RCNAVIGATIONITEM;

#define szRCNAVIGATIONITEM "w,w,w,w"

/*-----------------------------------------------------------------------------
These macros are used to manipulate PalmOS Objects without the need of taking
care of byte ordering.
Depending on the value of vfLE32, it uses the appropriate struct in the union
defining each object.
Using these macros allows to avoid to test vfLE32 and write the code twice...
-------------------------------------------------------------RNi-------------*/
#define BAFIELD(obj, field) (vfLE32 ? obj.s32.field : obj.s16.field)
#define PBAFIELD(pobj, field) (vfLE32 ? pobj->s32.field : pobj->s16.field)
#define SETBAFIELD(obj, field, value)   /*lint -e{717}*/ do { if (vfLE32) obj.s32.field = (value); else obj.s16.field = (value); } while (0)
#define SETPBAFIELD(pobj, field, value) /*lint -e{717}*/ do { if (vfLE32) pobj->s32.field = (value); else pobj->s16.field = (value); } while (0)
#define BAFIELD16(obj, field) obj.s16.field
#define BAFIELD32(obj, field) obj.s32.field
#define PBAFIELD16(obj, field) obj->s16.field
#define PBAFIELD32(obj, field) obj->s32.field

/*-----------------------------------------------------------------------------
|	Other PILRC types and such
-------------------------------------------------------------WESC------------*/

//#define ifrmMax 32
DEFPL(PLEXFORMOBJLIST)
     typedef struct FRMBA16Type
     {
       RCFormBA16Type form;
       PLEXFORMOBJLIST pllt;
       //      RCFORMOBJLIST *rglt;
     }
FRMBA16Type;
     typedef struct FRMBA32Type
     {
       RCFormBA32Type form;
       PLEXFORMOBJLIST pllt;
       //      RCFORMOBJLIST *rglt;
     }
FRMBA32Type;

     typedef union FRM
     {
       FRMBA16Type s16;
       FRMBA32Type s32;
     }
FRM;

/*
 * Translation Entry -- used to map to foreign languages 
 */
     typedef struct TE
     {
       char *szOrig;
       char *szTrans;
       struct TE *pteNext;
     }
TE;

/*
 * Symbol Table 
 */
     typedef struct SYM
     {
       char *sz;
       int wVal;
       char* sVal;
       BOOL fAutoId;
       struct SYM *psymNext;
     }
SYM;

/*
 * RCPFILE -- contains everything in a .rcp file 
 */
DEFPL(PLEXFRM)
     typedef struct RCPFILE
     {
       PLEXFRM plfrm;
     }
RCPFILE;
     void FreeRcpfile(RCPFILE * prcpfile);

/*
 * ReservedWord 
 */
     typedef enum RW
     {
       rwFLD, rwLST, rwTBL, rwFBM, rwLBL, rwTTL, rwPUL, rwGSI, rwGDT,
       rwBTN, rwPBN, rwCBX, rwPUT, rwSLT, rwREP, rwSLD, rwSCL,

       rwForm, rwBegin, rwEnd, rwModal, rwSaveBehind, rwNoSaveBehind,
       rwHelpId, rwDefaultBtnId, rwMenuId, rwNoGSI,
       rwEnabled, rwDisabled, rwUsable, rwNonUsable, rwLeftAnchor,
       rwExtended, rwNonExtended,                /* MBr add: gadget */
       rwRightAnchor, rwGroup, rwFont,
       rwFrame, rwNoFrame, rwBoldFrame, rwRectFrame, rwVertical, rwGraphical,
       
       rwEditable, rwNonEditable, rwUnderlined, rwSingleLine,
       rwMultipleLines, rwDynamicSize, rwLeftAlign, rwHasScrollBar,
       rwRightAlign, rwMaxChars,
       rwVisibleItems, rwAutoShift, rwNumeric,
       rwChecked,
       rwSearch,

       rwBitmap,
       rwBitmapGrey,
       rwBitmapGrey16,
       rwBitmapColor16,
       rwBitmapColor256,
       rwBitmapColor16k,
       rwBitmapColor24k,
       rwBitmapColor32k,
       rwBitmapFamily,
       rwBitmapFamily_special,
       rwBitmapFamilyEx,
       rwBitmapBpp,
       rwBitmapDensity,

       rwBootScreenFamily,
       rwBitmapPalette,
       rwPalette,

       rwInteger,
       rwPrevLeft, rwPrevRight, rwPrevWidth, rwPrevTop, rwPrevBottom,
       rwPrevHeight,
       rwMenu,
       rwPullDown,
       rwMenuItem,
       rwSeparator,

       rwValue,
       rwMinValue,
       rwMaxValue,
       rwPageSize,

       rwFeedback,
       rwThumbID,
       rwBackgroundID,
       rwBitmapID,
       rwSelectedBitmapID,

       rwAlert,
       rwMessage,
       rwButtons,

       rwInformation,                            /* order assumed */
       rwConfirmation,
       rwWarning,
       rwError,

       rwVersion,
       rwString,
       rwStringTable,
       rwFile,
       rwLauncherCategory,                       /* RMa add: application default launcher category */
       rwApplicationIconName,
       rwApplication,
       rwCategories,

       rwWordList,
       rwLongWordList,
       rwByteList,
       rwPaletteTable,
       rwMidi,
       rwSysAppPrefs,                            /* RMa system Application preferences */
       rwPriority,
       rwStackSize,
       rwMinHeapSpace,

       rwTranslation,
       rwLocale,                                 /* RMa Locatisation management */

#ifdef PALM_INTERNAL
       rwCountryLocalisation,                    /* 'cnty' */
       rwNumber,
       rwName,
       rwDateFormat,
       rwLongDateFormat,
       rwWeekStartDay,
       rwTimeFormat,
       rwNumberFormat,
       rwCurrencyName,
       rwCurrencySymbol,
       rwCurrencyUniqueSymbol,
       rwCurrencyDecimalPlaces,
       rwDaylightSavings,
       rwMinutesWestOfGmt,
       rwMeasurementSystem,

       rwLocales,                                /* 'locs' */
       rwLanguages,                              /* 'locs' */
       rwCountrys,                               /* 'locs' */
       rwCountryName,                            /* 'locs' */
       rwTimeZone,                               /* 'locs' */

       rwFontIndex,
       rwFontMap,
#endif

       rwFontType,                               /* 'NFNT' & 'fntm' */
       rwFirstChar,
       rwLastChar,
       rwmaxWidth,
       rwkernMax,
       rwnDescent,
       rwfRectWidth,
       rwfRectHeight,
       rwOwTLoc,
       rwAscent,
       rwDescent,
       rwLeading,
       rwRowWords,
       rwFlag,
       rwState,
       rwFontFamily,                             /* nfnt */

#ifdef PALM_INTERNAL
       rwGraffitiInputArea,
       rwCreator,
       rwLanguage,
       rwCountry,
       rwArea,
       rwScreen,
       rwGraffiti,
       rwAreaIndex,
       rwKeyDownChr,
       rwKeyDownKeyCode,
       rwKeyDownModifiers,

       rwFeature,
       rwEntry,
       rwKeyboard,
       rwDefaultItem,

       rwHardSoftButtonDefault,                  /* RMa add Hard/Soft button default */

       rwTableList,                              /* ttli */
       rwType,
       rwIndex,
       rwCharsetList,                            /* csli */

       rwTextTable,                              /* ttbl */
       rwTableType,
       rwDefaultOutput,
       rwNumElementBits,
       rwNumIndexedDataLenBits,
       rwNumResultBits,
       rwIndexDataOffset,
       rwClasses,
       rwCases,
       rwCasesPerState,
       rwAction,
       rwNextState,
       rwIn,
       rwOut,
       rwOffset,
       rwNumUniqueResults,
       rwNumUniqueResultTableBits,

       rwSearchTable,                            /* tSCH */
       rwOffset_s0,
       rwOffset_sD,
       rwOffset_sK,
       rwOffset_sM,
       rwOffset_sU,
       rwOffset_sP,
       rwOffset_sA,
       rwOffset_sG,
       rwOffset_sR,
#endif

       rwCenter,
       rwRight,
       rwBottom,
       rwAuto,
       rwAt,
       rwId,
       rwAutoId,

       rwNumColumns,
       rwNumRows,
       rwColumnWidths,

       rwInclude,
       rwDefine,
       rwEqu,
       rwUndef,
       rwIfdef,
       rwIfndef,
       rwElse,
       rwEndif,
       rwLine,
       rwExtern,
       rwIcon,
       //      rwIconGrey,
       //      rwIconGrey16,
       //      rwIconColor,     
       // Aaron Ardiri 
       // - removed these due to be purely backward 
       //   compatable in the applications launcher
       rwIconFamily,

       rwIconSmall,
       //      rwIconSmallGrey,
       //      rwIconSmallGrey16,
       //      rwIconSmallColor,
       // Aaron Ardiri 
       // - removed these due to be purely backward 
       //   compatable in the applications launcher
       rwIconSmallFamily,

       rwIconFamilyEx,
       rwIconSmallFamilyEx,

       rwTrap,

       rwHex,
       rwData,

       rwTransparency,
       rwTransparencyIndex,
       rwNoColorTable,
       rwColorTable,

       rwNoCompress,
       rwAutoCompress,
       rwForceCompress,

       rwCompressScanLine,
       rwCompressRLE,
       rwCompressPackBits,
       rwCompressBest,

       rwRscType,
       rwIncludeClut,
       rwDepth2,
       rwDepth4,
       rwDepth8,
       rwDepth16,

       rwResetAutoID,
       rwGenerateHeader,

       rwNavigation,
       rwInitialState,
       rwInitialObjectID,
       rwJumpObjectID,
       rwBottomLeftObjectID,
       rwAbove,
       rwBelow,
       rwSkip,
       rwForceInteraction,
       rwBigButton,
       rwNavigationMap,
       rwRow,

       rwPublic,
       rwShort,
       rwInt,
       rwStatic,
       rwFinal,

       rwNil
     }
RW;

     typedef struct RWT
     {
       const char *sz1;
       const char *sz2;
       RW rw;
     }
RWT;

/*
 * could just use index into this table as rw but that is kinda fragile. 
 */
#ifdef EMITRWT
     RWT rgrwt[] = {
       {"tFLD", "field", rwFLD},
       {"tLST", "list", rwLST},                  /* List */
       {"tTBL", "table", rwTBL},                 /* Table */
       {"tFBM", "formbitmap", rwFBM},            /* Form Bitmap */
       {"tLBL", "label", rwLBL},                 /* Label */
       {"tTTL", "title", rwTTL},                 /* Title                */
       {"tPUL", "popuplist", rwPUL},
       {"tGSI", "graffitistateindicator", rwGSI},       /* Graffiti State */
       {"tGDT", "gadget", rwGDT},                /* Gadget */

       {"tBTN", "button", rwBTN},
       {"tPBN", "pushbutton", rwPBN},
       {"tCBX", "checkbox", rwCBX},              /*  */
       {"tPUT", "popuptrigger", rwPUT},          /* Popup trigger */
       {"tSLT", "selectortrigger", rwSLT},
       {"tREP", "repeatbutton", rwREP},
       {"tSLD", "slider", rwSLD},                /* RMa add */
       {"tSCL", "scrollbar", rwSCL},             /* Scrollbar */

       {"form", "tFRM", rwForm},
       {"begin", NULL, rwBegin},
       {"end", NULL, rwEnd},
       {"modal", NULL, rwModal},
       {"savebehind", NULL, rwSaveBehind},
       {"nosavebehind", NULL, rwNoSaveBehind},
       {"helpid", NULL, rwHelpId},
       {"defaultbtnid", "defaultbutton", rwDefaultBtnId},       /* RMa form & alert */
       {"menuid", NULL, rwMenuId},
       {"nograffitistateindicator", NULL, rwNoGSI}, /* No Graffiti State */

       {"enabled", NULL, rwEnabled},
       {"disabled", NULL, rwDisabled},
       {"usable", NULL, rwUsable},
       {"nonusable", NULL, rwNonUsable},
       {"leftanchor", NULL, rwLeftAnchor},
       {"rightanchor", NULL, rwRightAnchor},
       {"group", NULL, rwGroup},
       {"font", "fontid", rwFont},

       {"frame", NULL, rwFrame},
       {"noframe", NULL, rwNoFrame},
       {"boldframe", NULL, rwBoldFrame},
       {"rectframe", NULL, rwRectFrame},

       {"vertical", NULL, rwVertical},
       {"graphical", NULL, rwGraphical},
       {"extended", NULL, rwExtended},           /* MBr add: gadget */
       {"nonextended", NULL, rwNonExtended},     /* MBr add: gadget */
       {"editable", NULL, rwEditable},
       {"noneditable", NULL, rwNonEditable},
       {"underlined", NULL, rwUnderlined},
       {"singleline", NULL, rwSingleLine},
       {"multiplelines", "multipleline", rwMultipleLines},
       {"dynamicsize", NULL, rwDynamicSize},
       {"leftalign", NULL, rwLeftAlign},
       {"hasscrollbar", NULL, rwHasScrollBar},
       {"rightalign", NULL, rwRightAlign},
       {"maxchars", NULL, rwMaxChars},
       {"visibleitems", NULL, rwVisibleItems},
       {"autoshift", NULL, rwAutoShift},
       {"numeric", NULL, rwNumeric},
       {"checked", "on", rwChecked},
       {"search", NULL, rwSearch},

       {"bitmap", NULL, rwBitmap},
       {"bitmapgrey", "bitmapgray", rwBitmapGrey},
       {"bitmapgrey16", "bitmapgray16", rwBitmapGrey16},
       {"bitmapcolor16", "bitmapcolour16", rwBitmapColor16},
       {"bitmapcolor", "bitmapcolour", rwBitmapColor256},

       {"bitmapcolor16k", "bitmapcolour16k", rwBitmapColor16k},
       {"bitmapcolor24k", "bitmapcolour24k", rwBitmapColor24k},
       {"bitmapcolor32k", "bitmapcolour32k", rwBitmapColor32k},
       {"bitmapfamily", NULL, rwBitmapFamily},
       {"bitmapfamilyspecial", NULL, rwBitmapFamily_special},
       {"bitmapfamilyex", NULL, rwBitmapFamilyEx},

       {"bpp", NULL, rwBitmapBpp},
       {"density", NULL, rwBitmapDensity},
#ifdef PALM_INTERNAL
       {"bootscreenfamily", NULL, rwBootScreenFamily},  /* 'tbsb' it look like as a bitmapfamily with an header (size & crc) */
#endif
       {"bitmappalette", NULL, rwBitmapPalette},
       {"palette", NULL, rwPalette},

       {"integer", NULL, rwInteger},
       {"prevleft", NULL, rwPrevLeft},
       {"prevright", NULL, rwPrevRight},
       {"prevwidth", NULL, rwPrevWidth},
       {"prevtop", NULL, rwPrevTop},
       {"prevbottom", NULL, rwPrevBottom},
       {"prevheight", NULL, rwPrevHeight},

       {"menu", "MBAR", rwMenu},
       {"pulldown", NULL, rwPullDown},
       {"menuitem", NULL, rwMenuItem},
       {"separator", NULL, rwSeparator},

       {"value", NULL, rwValue},
       {"min", "minvalue", rwMinValue},
       {"max", "maxvalue", rwMaxValue},
       {"pagesize", NULL, rwPageSize},

       {"feedback", NULL, rwFeedback},
       {"thumbid", NULL, rwThumbID},
       {"backgroundid", NULL, rwBackgroundID},
       {"bitmapid", NULL, rwBitmapID},
       {"selectedbitmapid", NULL, rwSelectedBitmapID},

       {"alert", "tALT", rwAlert},
       {"message", NULL, rwMessage},
       {"buttons", NULL, rwButtons},
       {"information", NULL, rwInformation},
       {"confirmation", NULL, rwConfirmation},
       {"warning", NULL, rwWarning},
       {"error", NULL, rwError},

       {"version", "tVER", rwVersion},
       {"stringtable", "tSTL", rwStringTable},
       {"string", "tSTR", rwString},
       {"file", "tSTR", rwFile},
       {"applicationiconname", NULL, rwApplicationIconName},
       {"application", "APPL", rwApplication},
       {"categories", "tAIS", rwCategories},
       {"wordlist", NULL, rwWordList},
       {"longwordlist", NULL, rwLongWordList},   /* 'DLST' */
       {"bytelist", NULL, rwByteList},           /* 'BLST' */
       {"palettetable", NULL, rwPaletteTable},
       {"midi", NULL, rwMidi},                   /* 'MIDI' */

       {"translation", NULL, rwTranslation},
       {"locale", NULL, rwLocale},

       {"launchercategory", "taic", rwLauncherCategory},
       {"fontfamily", NULL, rwFontFamily},       /* nfnt */

#ifdef PALM_INTERNAL
       {"fontindex", NULL, rwFontIndex},

       {"fontmap", NULL, rwFontMap},             /* 'NFNT' & 'fntm' */
       {"fonttype", NULL, rwFontType},
       {"firstchar", NULL, rwFirstChar},
       {"lastChar", NULL, rwLastChar},
       {"maxwidth", NULL, rwmaxWidth},
       {"kernmax", NULL, rwkernMax},
       {"ndescent", NULL, rwnDescent},
       {"frectwidth", NULL, rwfRectWidth},
       {"frectheight", NULL, rwfRectHeight},
       {"owtloc", NULL, rwOwTLoc},
       {"ascent", NULL, rwAscent},
       {"descent", NULL, rwDescent},
       {"leading", NULL, rwLeading},
       {"rowwords", NULL, rwRowWords},
       {"flag", NULL, rwFlag},
       {"state", NULL, rwState},

       {"graffitiinputarea", NULL, rwGraffitiInputArea},        /* 'silk' */
       {"creator", NULL, rwCreator},
       {"language", NULL, rwLanguage},
       {"country", NULL, rwCountry},
       {"area", NULL, rwArea},
       {"screen", NULL, rwScreen},
       {"graffiti", NULL, rwGraffiti},
       {"index", NULL, rwAreaIndex},
       {"keydownchr", NULL, rwKeyDownChr},
       {"keydownkeycode", NULL, rwKeyDownKeyCode},
       {"keydownmodifiers", NULL, rwKeyDownModifiers},
       {"countrylocalisation", NULL, rwCountryLocalisation},    /* 'cnty' */
       {"number", NULL, rwNumber},
       {"name", NULL, rwName},
       {"dateformat", NULL, rwDateFormat},
       {"longdateformat", NULL, rwLongDateFormat},
       {"weekstartday", NULL, rwWeekStartDay},
       {"timeformat", NULL, rwTimeFormat},
       {"numberformat", NULL, rwNumberFormat},
       {"currencyname", NULL, rwCurrencyName},
       {"currencysymbol", NULL, rwCurrencySymbol},
       {"currencyuniquesymbol", NULL, rwCurrencyUniqueSymbol},
       {"currencydecimalplaces", NULL, rwCurrencyDecimalPlaces},
       {"daylightsavings", NULL, rwDaylightSavings},
       {"minuteswestofgmt", NULL, rwMinutesWestOfGmt},
       {"measurementsystem", NULL, rwMeasurementSystem},
       {"locales", NULL, rwLocales},             /* 'locs' */
       {"languages", NULL, rwLanguages},         /* 'locs' */
       {"countrys", NULL, rwCountrys},           /* 'locs' */
       {"CountryName", NULL, rwCountryName},     /* 'locs' */
       {"timezone", NULL, rwTimeZone},           /* 'locs' */

       {"feature", NULL, rwFeature},             /* 'feat' */
       {"entry", NULL, rwEntry},
       {"keyboard", NULL, rwKeyboard},           /* 'tkbd' */
       {"defaultitem", NULL, rwDefaultItem},     /* 'DLST' & 'BLST' */

       {"SysApplicationPreferences", NULL, rwSysAppPrefs},      /* 'pref' */
       {"priority", NULL, rwPriority},           /* 'pref' */
       {"stacksize", NULL, rwStackSize},         /* 'pref' */
       {"minheapspace", NULL, rwMinHeapSpace},   /* 'pref' */

       {"hardsoftbuttondefault", NULL, rwHardSoftButtonDefault},

       {"tablelist", NULL, rwTableList},         /* ttli */
       {"type", NULL, rwType},

       {"charsetlist", NULL, rwCharsetList},     /* csli */

       {"texttable", NULL, rwTextTable},         /* ttbl */
       {"tabletype", NULL, rwTableType},
       {"defaultoutput", NULL, rwDefaultOutput},
       {"numelementbits", NULL, rwNumElementBits},
       {"numindexeddatalenbits", NULL, rwNumIndexedDataLenBits},
       {"numresultbits", NULL, rwNumResultBits},
       {"indexdataoffset", NULL, rwIndexDataOffset},
       {"classes", NULL, rwClasses},
       {"cases", NULL, rwCases},
       {"casesperstate", NULL, rwCasesPerState},
       {"action", NULL, rwAction},
       {"nextstate", NULL, rwNextState},
       {"in", "input", rwIn},
       {"out", "output", rwOut},
       {"offset", NULL, rwOffset},
       {"rwnumuniqueresults", NULL, rwNumUniqueResults},
       {"rwnumuniqueresulttablebits", NULL, rwNumUniqueResultTableBits},

       {"searchtable", NULL, rwSearchTable},     /* tSCH */
       {"offset_s0", NULL, rwOffset_s0},         /* tSCH */
       {"offset_sd", NULL, rwOffset_sD},         /* tSCH */
       {"offset_sk", NULL, rwOffset_sK},         /* tSCH */
       {"offset_sm", NULL, rwOffset_sM},         /* tSCH */
       {"offset_su", NULL, rwOffset_sU},         /* tSCH */
       {"offset_sp", NULL, rwOffset_sP},         /* tSCH */
       {"offset_sa", NULL, rwOffset_sA},         /* tSCH */
       {"offset_sg", NULL, rwOffset_sG},         /* tSCH */
       {"offset_sr", NULL, rwOffset_sR},         /* tSCH */
#endif

       {"center", NULL, rwCenter},
       {"right", NULL, rwRight},
       {"bottom", NULL, rwBottom},
       {"auto", NULL, rwAuto},
       {"at", NULL, rwAt},
       {"id", NULL, rwId},
       {"autoid", NULL, rwAutoId},

       {"columns", "numcolumns", rwNumColumns},
       {"rows", "numrows", rwNumRows},
       {"columnwidths", "widths", rwColumnWidths},

       {"define", NULL, rwDefine},
       {"equ", NULL, rwEqu},
       {"include", NULL, rwInclude},
       {"undef", NULL, rwUndef},
       {"ifdef", NULL, rwIfdef},
       {"ifndef", NULL, rwIfndef},
       {"else", NULL, rwElse},
       {"endif", NULL, rwEndif},
       {"line", NULL, rwLine},
       {"extern", NULL, rwExtern},

       {"icon", NULL, rwIcon},
       //      {"icongrey",     "icongray",     rwIconGrey},
       //      {"icongrey16",   "icongray16",   rwIconGrey16},
       //      {"iconcolor",    "iconcolour",   rwIconColor},
       // Aaron Ardiri 
       // - removed these due to be purely backward 
       //   compatable in the applications launcher

       {"iconfamily", NULL, rwIconFamily},

       {"smallicon", NULL, rwIconSmall},
       //      {"smallicongrey",     "smallicongray",     rwIconSmallGrey},
       //      {"smallicongrey16",   "smallicongray16",   rwIconSmallGrey16},
       //      {"smalliconcolor",    "smalliconcolour",   rwIconSmallColor},
       // Aaron Ardiri 
       // - removed these due to be purely backward 
       //   compatable in the applications launcher

       {"smalliconfamily", NULL, rwIconSmallFamily},

       {"iconfamilyEx", NULL, rwIconFamilyEx},
       {"smalliconfamilyEx", NULL, rwIconSmallFamilyEx},

       {"trap", NULL, rwTrap},

       {"hex", NULL, rwHex},
       {"data", NULL, rwData},

       {"transparency", "transparent", rwTransparency},
       {"transparencyIndex", "transparentindex", rwTransparencyIndex},
       {"colortable", "colourtable", rwColorTable},
       {"nocolortable", "nocolourtable", rwNoColorTable},
       {"nocompress", NULL, rwNoCompress},
       {"autocompress", "compress", rwAutoCompress},
       {"forcecompress", NULL, rwForceCompress},

       {"compressscanline", NULL, rwCompressScanLine},
       {"compressrle", NULL, rwCompressRLE},
       {"compresspackbits", NULL, rwCompressPackBits},
       {"compressbest", NULL, rwCompressBest},

       {"rsctype", NULL, rwRscType},
       {"includeclut", NULL, rwIncludeClut},
       {"depth2", NULL, rwDepth2},
       {"depth4", NULL, rwDepth4},
       {"depth8", NULL, rwDepth8},
       {"depth16", NULL, rwDepth16},

       {"resetautoid", NULL, rwResetAutoID},
       {"generateheader", NULL, rwGenerateHeader},

       {"navigation", NULL, rwNavigation},
       {"initialstate", NULL, rwInitialState},
       {"initialobjectid", NULL, rwInitialObjectID},
       {"jumpobjectid", NULL, rwJumpObjectID},
       {"bottomleftobjectid", NULL, rwBottomLeftObjectID},
       {"above", NULL, rwAbove},
       {"below", NULL, rwBelow},
       {"skip", NULL, rwSkip},
       {"forceinteraction", NULL, rwForceInteraction},
       {"bigbutton", NULL, rwBigButton},
       {"navigationmap", NULL, rwNavigationMap},
       {"row", NULL, rwRow},

       /*
        * Java specific 
        */
       {"public", NULL, rwPublic},
       {"short", NULL, rwShort},
       {"int", NULL, rwInt},
       {"static", NULL, rwStatic},
       {"final", NULL, rwFinal},
       {NULL, NULL, rwNil},
     };
#endif                                           /* EMITRWT */

/*
 * TOKen 
 */
typedef struct TOK
{
  RW rw;
  LEX lex;
}
TOK;

/*-----------------------------------------------------------------------------
|	Various Konstant types -- basically deferred evaluation of constants
|	mainly for AUTO and CENTER because we can't evaluate them until we know
|	the font for the particular item.
-------------------------------------------------------------WESC------------*/

/*
 * Konstant Type 
 */
typedef enum KT
{
  ktConst,
  ktCenter,
  ktAuto,
  ktAutoGreaterThan,
  ktCenterAt,
  ktRightAt,
  ktBottomAt
}
KT;

/*
 * Konstant 
 */
typedef struct K
{
  KT kt;
  int wVal;
}
K;

/*
 * Konstant Point 
 */
typedef struct KPT
{
  K kX;
  K kY;
}
KPT;

/*
 * Konstant Rect 
 */
typedef struct KRC
{
  KPT kptUpperLeft;
  KPT kptExtent;
}
KRC;

/*-----------------------------------------------------------------------------
|	ITM
|		an item in a form -- grif and grif2 define the syntax of the item
|	and what to expect.
-------------------------------------------------------------WESC------------*/
typedef struct ITM
{
  int grif;
  int grifOut;
  int grif2;
  int grif2Out;
  int grif3;
  int grif3Out;
  int grif4;
  int grif4Out;
  char *text;
  int cbText;                                    /* length of text including nul terminator */
  int id;
  int listid;
  KRC krc;
  /*
   * RectangleType rc; 
   */
  RCRECT rc;
  KPT kpt;
  /*
   * PointType pt; 
   */
  RCPOINT pt;
  BOOL usable;
  BOOL leftAnchor;
  int frame;
  BOOL extended;                                 /* MBr add: gadget */
  BOOL enabled;
  BOOL on;                                       /* checked */
  BOOL editable;
  BOOL underlined;
  BOOL singleLine;
  BOOL dynamicSize;
  BOOL vertical;                                 /* RMa add: slider */
  BOOL graphical;                                /* RMa add: slider */
  BOOL search;                                   /* RMa add: lst enable incremental search */
  int justification;
  int maxChars;
  int autoShift;
  BOOL hasScrollBar;
  BOOL numeric;
  int numItems;
  int cvis;
  int group;
  int font;
  int rscID;
  BOOL modal;
  BOOL saveBehind;
  int helpId;
  int defaultBtnId;
  int menuId;
  int numRows;
  int numColumns;
  int rgdxcol[64];
  int value;                                     /* scrollbar */
  int minValue;                                  /* scrollbar */
  int maxValue;                                  /* scrollbar */
  int pageSize;                                  /* scrollbar */
  int thumbid;                                   /* RMa add: slider */
  int backgroundid;                              /* RMa add: slider */
  BOOL feedback;                                 /* RMa add: slider */
  int bitmapid;                                  /* RMa add: graphical button */
  int selectedbitmapid;                          /* RMa add: graphical button */
  int version;                                   /* RMa add: GraffitiInputArea 'silk', 'locs' */
  char *creator;                                 /* RMa add: GraffitiInputArea 'silk' */
  char *language;                                /* RMa add: GraffitiInputArea 'silk' */
  char *country;                                 /* RMa add: GraffitiInputArea 'silk' */
  int areaType;                                  /* RMa add: GraffitiInputArea 'silk' */
  int areaIndex;                                 /* RMa add: GraffitiInputArea 'silk' */
  int keyDownChr;                                /* RMa add: GraffitiInputArea 'silk' */
  int keyDownKeyCode;                            /* RMa add: GraffitiInputArea 'silk' */
  int keyDownModifiers;                          /* RMa add: GraffitiInputArea 'silk' */
  int Number;                                    /* RMa add: country 'cnty' */
  char *Name;                                    /* RMa add: country 'cnty', 'locs' */
  int DateFormat;                                /* RMa add: country 'cnty', 'locs' */
  int LongDateFormat;                            /* RMa add: country 'cnty', 'locs' */
  int WeekStartDay;                              /* RMa add: country 'cnty', 'locs' */
  int TimeFormat;                                /* RMa add: country 'cnty', 'locs' */
  int NumberFormat;                              /* RMa add: country 'cnty', 'locs' */
  char *CurrencyName;                            /* RMa add: country 'cnty', 'locs' */
  char *CurrencySymbol;                          /* RMa add: country 'cnty', 'locs' */
  char *CurrencyUniqueSymbol;                    /* RMa add: country 'cnty', 'locs' */
  int CurrencyDecimalPlaces;                     /* RMa add: country 'cnty', 'locs' */
  int DaylightSavings;                           /* RMa add: country 'cnty', 'locs' */
  int MinutesWestOfGmt;                          /* RMa add: country 'cnty' */
  int MeasurementSystem;                         /* RMa add: country 'cnty', 'locs' */
  int TimeZone;                                  /* RMa add: locales         'locs' */
  char *CountryName;                             /* RMa add: locales         'locs' */
  int Languages;                                 /* RMa add: locales         'locs' */
  int Countrys;                                  /* RMa add: locales         'locs' */
  int DefaultItem;                               /* RMa add: 'pref' */
  int Priority;                                  /* RMa add: 'pref' */
  int StackSize;                                 /* RMa add: 'pref' */
  int MinHeapSpace;                              /* RMa add: 'pref' */
  char *Locale;                                  /* RMa localisation management */
  int AlertType;                                 /* RMA alert */
  int density;                                   /* RMA add: 'bmpf' v3 & 'nfnt' */
  int FontType;                                  /* RMa add: font 'NFNT' & 'fntm' */
  int FirstChar;                                 /* RMa add: font 'NFNT' & 'fntm' */
  int LastChar;                                  /* RMa add: font 'NFNT' & 'fntm' */
  int maxWidth;                                  /* RMa add: font 'NFNT' & 'fntm' */
  int kernMax;                                   /* RMa add: font 'NFNT' & 'fntm' */
  int nDescent;                                  /* RMa add: font 'NFNT' & 'fntm' */
  int fRectWidth;                                /* RMa add: font 'NFNT' & 'fntm' */
  int fRectHeight;                               /* RMa add: font 'NFNT' & 'fntm' */
  int owTLoc;                                    /* RMa add: font 'NFNT' & 'fntm' */
  int Ascent;                                    /* RMa add: font 'NFNT' & 'fntm' */
  int Descent;                                   /* RMa add: font 'NFNT' & 'fntm' */
  int Leading;                                   /* RMa add: font 'NFNT' & 'fntm' */
  int rowWords;                                  /* RMa add: font 'NFNT' & 'fntm' */
  int flag;                                      /* RMa add: font 'NFNT' & 'fntm' */
  int state;                                     /* RMa add: font 'NFNT' & 'fntm' */
  BOOL compress;                                 /* RMa add: font 'ttbl' */
  int tableType;                                 /* RMa add: font 'ttbl' */
  int defaultOutput;                             /* RMa add: font 'ttbl' */
  int numElementBits;                            /* RMa add: font 'ttbl' */
  int numIndexedDataLenBits;                     /* RMa add: font 'ttbl' */
  int numResultBits;                             /* RMa add: font 'ttbl' */
  int indexDataOffset;                           /* RMa add: font 'ttbl' */
  int initialState;                              /* NAVIGATION */
  int initialObjectID;                           /* NAVIGATION */
  int jumpObjectID;                              /* NAVIGATION */
  int bottomLeftObjectID;                        /* NAVIGATION */
  int aboveID;                                   /* NAVIGATION */
  int belowID;                                   /* NAVIGATION */
  BOOL skip;                                     /* NAVIGATION */
  BOOL forceInteraction;                         /* NAVIGATION */
  BOOL bigButton;                                /* NAVIGATION */
}
ITM;

/*
 * Item Flags 
 */
#define ifNull         0x00000000
#define ifText         0x00000001
#define ifMultText     0x00000002
#define ifId           0x00000004
#define ifRc           0x00000008
#define ifPt           0x00000010
#define ifUsable       0x00000020
#define ifAnchor       0x00000040
#define ifFrame        0x00000080
#define ifEnabled      0x00000100
#define ifOn           0x00000200
#define ifEditable     0x00000400
#define ifSingleLine   0x00000800
#define ifDynamicSize  0x00001000
#define ifMaxChars     0x00002000
#define ifCvis         0x00004000
#define ifGroup        0x00008000
#define ifFont         0x00010000
#define ifAlign        0x00020000
#define ifUnderlined   0x00040000
#define ifListId       0x00080000
#define ifBitmap       0x00100000
#define ifExtended     0x00200000                /* MBr add: gadget */
#define ifSearch       0x00400000

/*
 * Form ifs 
 */
#define ifModal        0x00200000
#define ifSaveBehind   0x00400000
#define ifHelpId       0x00800000
#define ifDefaultBtnId 0x01000000
#define ifMenuId       0x02000000
/* unused              0x04000000 */
/* unused              0x08000000 */
/* unused              0x10000000 */
/* unused              0x20000000 */

/*
 * Ifs defining margins -- extra width to add to an 
 * item in addition to it's string width.  These don't
 * map to keywords used in the RCP file but are used
 * for AUTO width calculations.
 */
#define ifBigMargin    0x40000000
#define ifSmallMargin  0x80000000

/*
 * if2s -- ran out of bits in if! 
 */
#define if2Null					0x00000000
#define if2NumColumns			0x00000001
#define if2NumRows				0x00000002
#define if2ColumnWidths			0x00000004
#define if2Value				0x00000008
#define if2MinValue				0x00000010
#define if2MaxValue				0x00000020
#define if2PageSize				0x00000040
#define if2AutoShift			0x00000080
#define if2Scrollbar			0x00000100
#define if2Numeric				0x00000200
#define if2GSI                  0x00000400 /* used with ifPt for GSI size calculation */
#define if2Type 				0x00000800
#define if2File 				0x00001000
#define if2CreatorID			0x00002000
#define if2AppType				0x00004000
#define if2CreateTime			0x00008000
#define if2ModTime				0x00010000
#define if2BackupTime			0x00020000
#define if2AppInfo				0x00040000
#define if2SortInfo				0x00080000
#define if2ReadOnly				0x00100000
#define if2Backup				0x00200000
#define if2CopyProtect			0x00400000
#define if2Priority				0x00800000
#define if2ThumbID				0x01000000
#define if2BackgroundID			0x02000000
#define if2Vertical				0x04000000
#define if2Graphical			0x08000000
#define if2BitmapID				0x10000000
#define if2SelectedBitmapID     0x20000000
#define if2Feedback				0x40000000
#define if2ForceInteraction     0x80000000 /* NAVIGATION */

/*
 * if3s -- ran out of bits in if and if2!       RMa add 
 */
#define if3Null					0x00000000
#define if3Vers					0x00000001
#define if3Creator				0x00000002
#define if3Language				0x00000004
#define if3Country				0x00000008
#define if3areaType				0x00000010
#define if3areaIndex			0x00000020
#define if3keyDownChr			0x00000040
#define if3keyDownKeyCode		0x00000080
#define if3keyDownModifiers     0x00000100
#define if3Number				0x00000200
#define if3Name					0x00000400
#define if3DateFormat			0x00000800
#define if3LongDateFormat		0x00001000
#define if3WeekStartDay			0x00002000
#define if3TimeFormat			0x00004000
#define if3NumberFormat			0x00008000
#define if3CurrencyName			0x00010000
#define if3CurrencySymbol		0x00020000
#define if3CurrencyUniqueSymbol	0x00040000
#define if3CurrencyDecimalPlaces 0x00080000
#define if3DayLightSaving		0x00100000
#define if3MinutesWestOfGmt		0x00200000
#define if3MeasurementSystem	0x00400000
#define if3DefaultItm			0x00800000
#define if3TimeZone  			0x01000000
#define if3Languages			0x02000000
#define if3Countrys				0x04000000
#define if3CountryName			0x08000000
#define if3StackSize			0x10000000
#define if3MinHeapSpace			0x20000000
#define if3Locale				0x40000000
#define if3AlertType			0x80000000

/*
 * if4s -- ran out of bits in if, if2 and if3!       RMa add 
 */
#define if4Null					0x00000000      /* RMa 'NFNT' & 'fntm' */
#define if4FontType				0x00000001
#define if4firstChar			0x00000002
#define if4lastChar				0x00000004
#define if4maxWidth				0x00000008
#define if4kernMax				0x00000010
#define if4nDescent				0x00000020
#define if4fRectWidth			0x00000040
#define if4fRectHeight			0x00000080
#define if4owTLoc				0x00000100
#define if4Ascent				0x00000200
#define if4Descent				0x00000400
#define if4Leading				0x00000800
#define if4rowWords				0x00001000
#define if4flag					0x00002000
#define if4state				0x00004000
#define if4tableType			0x00008000
#define if4defaultOutput		0x00010000
#define if4numElementBits		0x00020000
#define if4numIndexedDataLenBits 0x00040000
#define if4numResultBits		0x00080000
#define if4indexDataOffset		0x00100000
#define if4compressed			0x00200000
#define if4BitmapExtBpp			0x00400000
#define if4BitmapExtDensity     0x00800000
#define if4InitialState         0x01000000 /* NAVIGATION */
#define if4InitialObjectID      0x02000000 /* NAVIGATION */
#define if4JumpObjectID         0x04000000 /* NAVIGATION */
#define if4BottomLeftObjectID   0x08000000 /* NAVIGATION */
#define if4Above                0x10000000 /* NAVIGATION */
#define if4Below                0x20000000 /* NAVIGATION */
#define if4Skip                 0x40000000 /* NAVIGATION */
#define if4BigButton            0x80000000 /* NAVIGATION */


/*
 * Parse globals 
 */

typedef struct INPUTCONTEXT
{
  FILELINE file;
  char buffer[4096];
  char *pch;
  const char *pchLex;
  BOOL fPendingTok;
  TOK pendingTok;
}
INPUTCONTEXT;

extern TOK tok;
extern INPUTCONTEXT vIn;

extern BOOL vfWinGUI;
extern BOOL vfQuiet;
extern BOOL vfAllowEditIDs;
extern BOOL vfAllowBadIconSizes;
extern BOOL vfAllowLargeResources;
extern BOOL vfNoEllipsis;
extern BOOL vfPalmRez;
extern BOOL vfVSErrors;
extern BOOL vfCheckDupes;
extern BOOL vfAppend;
extern BOOL vfRTL;
extern BOOL vfLE32;
extern BOOL vfAppIcon68K;
extern BOOL vfAutoAmdc;
extern BOOL vfTrackDepends;
extern BOOL vfInhibitOutput;

extern BOOL vfAutoId;
extern int  vfAutoDirection;
extern int  vfAutoStartID;
extern int  idAutoDirection;

extern char *szDllNameP;

//LDu : Output a Prc file
extern BOOL vfPrc;
extern const char *vfPrcName;
extern const char *vfPrcCreator;
extern const char *vfPrcType;
extern BOOL vfPrcTimeStamp;

/*
 * LDu Ignore Include File In Header Files
 */
extern BOOL vfIFIH;

#define DEFAULT_PRCNAME "PilRC resources"
#define DEFAULT_PRCCR8R 0x70524553               // 'pRES'
#define DEFAULT_PRCTYPE 0x64617461               // 'data'

// Translations
#define MAXLANG 10
extern int totalLanguages;
extern const char *aszLanguage[];

// Localisation management
extern char *szLocaleP;
extern BOOL vfStripNoLocRes;

#define dxScreen 160
#define dyScreen 160
#define maxCategories  16
#define categoryLength 16
#define maxSafeResourceSize 65000
#define	kGsiWidth      9
#define kGsiHeight     10

void ParseToFinalEnd(void);
void ParseItm(ITM * pitm,
              int grif,
              int grif2,
              int grif3,
              int grif4);
BOOL FGetTok(TOK *ptok);
VOID UngetTok(void);
const TOK *PeekTok(void);
int WGetConst(char *szErr);
BOOL FIsString(const TOK *ptok);
char *PchGetString(const char *szErr);
VOID GetExpectRw(RW rw);
int WGetId(char *szErr);
int WGetConstEx(char *szErr);

void CbInit(void);
int CbEmitStruct(void *pv,
                 char *szPic,
                 char **ppchText,
                 BOOL fEmit);
int CbStruct(char *szPic);
RCPFILE *ParseFile(const char *szIn,
          const char *szOutDir,
          const char *szResFile,
          const char *szIncFile,
          int fontType);
SYM *PsymLookupId(int id);
VOID AddSym(const char *sz, int wVal);
VOID AddSymString(const char* sz, const char* val);
VOID RemoveSym(const char *sz);

VOID AddDefineSymbol(void);

#endif                                           // _pilrc_h
