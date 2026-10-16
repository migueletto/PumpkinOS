/******************************************************************************
 *
 * Copyright (c) 1995-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: UIResourcesPrv.h
 *
 * Release: 
 *
 * Description:
 *    This file defines resource IDs which are private to Palm OS.
 *
 * History:
 *    ??/??/??  ??? Created.
 *    06/29/99  CS Added constantRscType & ResLoadConstant().
 *    04/13/00  jmp Added info about systemDefaultUIColors8.
 *    06/05/00  kwk Moved fontIndexRscID to IntlPrv.rh. Moved all keyboard
 *              resource IDs into IntlPrv.rh.
 *    08/21/00  kwk Deleted 135XX and 136XX #defines for defunct daylight
 *              saving dialogs.
 *
 *****************************************************************************/

#ifndef __UIRESOURCESPRV_H__
#define __UIRESOURCESPRV_H__

// Midi resources
#define coverageOKMidiID              10000

// System bitmaps
#define palmLogoBitmap                10000
#define InformationAlertBitmap        10004
#define ConfirmationAlertBitmap       10005
#define WarningAlertBitmap            10006
#define ErrorAlertBitmap              10007
//The following 6 must be consecutive - resource ID calculated from RSSIBars0Bitmap
#define RSSIBars0Bitmap               10010 
#define RSSIBars1Bitmap               10011
#define RSSIBars2Bitmap               10012
#define RSSIBars3Bitmap               10013
#define RSSIBars4Bitmap               10014
#define RSSIBars5Bitmap               10015

// DOLATER kwk - what in god's name are these constants doing here?
#define WirelessIndicatorWidth        19
#define WirelessIndicatorHeight       11

#define SliderDefaultThumbBitmap      13350
#define SliderDefaultBackgroundBitmap 13351

#define splashScreenBitmap            10100
#define hardResetScreenBitmap         10101

// System string resources

// Product name:  If this string exists, use it instead of "PalmPilot"
// This string was first created for the IBM WorkPad (v2.0.2) ROM.
#define oemProductNameStrID           10012

#define proStrID                      10013  // Appended to version string on Professional
#define demoStrID                     10014  // Appended to version string on Demo unit

#define sysSendStatusBeamingStrID     10015
#define sysSendStatusReceivingStrID   10016

#define localeModuleNameStrID         10017  // Name of locale module DB, used in IntlInit().

// 11/15/00  kwk  Added for support of default FEP, so that licensees can create
// ROMs with a different default FEP.
#define tsmDefaultFepNameStrID        10018  // Name of default Fep, used in TsmInit().
// DOLATER kwk - note that 10019 and 10020 are used for tips strings, but I don't think
// they get defined anywhere. This makes it hard to figure out what are safe IDs to use
// for new resources. It would also be great to have some sort of standard format for
// changes here, versus having a choice of either making no comment, or adding it to
// the header.

#define ConnectionMgrCurrentProfileName 15000
#define ConnectionMgrDefaultProfileName 15001

// System string list resources
// (Note that 10200--10205 are duplicated in PalmCompatibility.h.) 
#define daysOfWeekShortStrListID      10200
#define daysOfWeekStdStrListID        10201
#define daysOfWeekLongStrListID       10202
#define monthNamesShortStrListID      10203
#define monthNamesStdStrListID        10204
#define monthNamesLongStrListID       10205
#define prefDateFormatsStrListID      10206
#define prefDOWDateFormatsStrListID   10207

// System default command bar
#define sysDefaultCommandBar          10000
#define sysEnglishDefaultCommandBar   10400

//  English edit menu bar and menus - text comparison
// by MenuCtl on localized devices to see if cut,copy,paste present
// in English.
#define sysEnglishEditMenuBar         10400
//  Resource: MENU 10401
#define sysEnglishEditMenu            10400
#define sysEnglishEditUndo            10400  // Command Key: U
#define sysEnglishEditCut             10401  // Command Key: X
#define sysEnglishEditCopy            10402  // Command Key: C
#define sysEnglishEditPaste           10403  // Command Key: P

// Date Selector Dialog
#define DateSelectorForm              10100
#define DateSelectorYearLabel         10102
#define DateSelectorPriorYearButton   10103
#define DateSelectorNextYearButton    10104
#define DateSelectorTodayButton       10118
#define DateSelectorCancelButton      10119
#define DateSelectorDayGadget         10120
#define DateSelectorThisWeekButton    10121
#define DateSelectorThisMonthButton   10122

// Time Selector Dialog
#define TimeSelectorForm              10200
#define TimeSelectorStartTimeButton   10204
#define TimeSelectorEndTimeButton     10205
#define TimeSelectorHourList          10206
#define TimeSelectorMinuteList        10207
#define TimeSelectorOKButton          10208
#define TimeSelectorCancelButton      10209
#define TimeSelectorNoTimeButton      10210
#define TimeSelectorAllDayButton      10212

// Help Dialog
#define HelpForm                      10400
#define HelpField                     10402
#define HelpDoneButton                10403
#define HelpUpButton                  10404
#define HelpDownButton                10405

// Find Dialog
#define FindDialog                    10500
#define FindStrField                  10503
#define FindOKButton                  10504

// Find Results Dialog
#define FindResultsDialog             10600
#define FindResultsMsgLabel           10602
#define FindResultsTable              10603
#define FindResultsGoToButton         10604
#define FindResultsCancelButton       10605
#define FindResultsMoreButton         10606
#define FindResultsStopButton         10607

#define FindResultsSearchingStr       10607
#define FindResultsMatchesStr         10608
#define FindResultsNoMatchesStr       10609
#define FindResultsContinueStr        10610

// Net Library Serial Net Interface dialogs. These don't really have to
// have ID's in the system range except in the Emulator. When running on 
// the device, the interface's resource file is opened up just to get these
// resources so that the ID's don't have to be unique with the application's
// resource IDs.
#define  netSerStringList             11300      // String list resource ID

// Net Serial Progress Dialog
#define  netSerProgressTitleStr       11300      // Title string for progress form
#define  netSerPictPhone              11300      // resource ID of phone picture
#define  netSerPictHandshake          11301      // resource ID of handshake picture
#define  netSerPictBook               11302      // resource ID of book picture
#define  netSerPictError              11303      // resource ID of error picture

// Net Serial Prompt Dialog (for password or username/id)
#define  netSerPromptFrm              11400      // Prompt for string form ID
#define  netSerPromptAsk              11402      // Label - ask string
#define  netSerPromptField            11403      // answer field
#define  netSerPromptBtnOK            11404      // OK button ID
#define  netSerPromptBtnCancel        11405      // Cancel button ID

// Net Serial Challenge Dialog (for challenge response or generic prompt)
#define  netSerChallengeFrm           11410      // Prompt for challenge form ID
#define  netSerChallengeAsk           11412      // Label - ask string
#define  netSerChallengeField         11413      // answer field
#define  netSerChallengeBtnOK         11414      // OK button ID
#define  netSerChallengeBtnCancel     11415      // Cancel button ID

// Launcher Dialog
#define launcherDialog                11500      // deleted 21-Jul-00
#define launcherGadget                11501      // deleted 21-Jul-00

// Progress Dialog
#define prgStringList                 11600      // String list resource ID
#define prgProgressFrm                11600      // progress form ID
#define prgProgressLabelStage         11602      // Label ID
#define prgProgressBtnCancel          11603      // Cancel button ID
#define prgProgressCounterGadget      11604      // resource ID of gadget used as opened counter
#define prgPictPhone                  11620      // resource ID of phone picture
#define prgPictHandshake              11621      // resource ID of handshake picture
#define prgPictBook                   11622      // resource ID of book picture
#define prgPictError                  11623      // resource ID of error picture

// IrLib Progress Icons
#define prgBeamSend1                  11640      // resource ID of 1st beam send
#define prgBeamSend2                  11641      // resource ID of 2nd beam send
#define prgBeamRec1                   11642      // resource ID of 1st beam receive
#define prgBeamRec2                   11643      // resource ID of 2nd beam receive
#define prgBeamPrepare1               11644      // preparing/accepting
#define prgBeamPrepare2               11645      // preparing/accepting
#define prgBeamIdle                   11646      // inactive

// Exchange Manager progress strings (defaults in system)
#define exgPrgStringList              11650      // String list resource ID for Ir Exg lib
// Indexes into the preceeding list
#define  exgStrIndexOK                 1         // "OK"
#define  exgStrIndexCancelling         2         // "Cancelling"
#define  exgStrIndexError              3         // "Error: "
#define  exgStrIndexInitializing       4         // "Initializing"
#define  exgStrIndexStarting           5         // "Starting"
#define  exgStrIndexSearching          6         // "Searching"
#define  exgStrIndexConnected          7         // "Connected"
#define  exgStrIndexSending            8         // "Sending: "
#define  exgStrIndexReceiving          9         // "Receiving: "
#define  exgStrIndexDisconnecting     10         // "Disconnecting"
#define  exgStrIndexWaitingSender     11         // "Waiting for Sender"
#define  exgStrIndexPreparing         12         // "Preparing"
#define  exgStrIndexAccepting         13         // "Accepting"
#define  exgStrIndexInterrupted       14         // "Transfer Interrupted"

// Exchange Manager Resources
#define ExgAskForm                    11700      // Do you wanna accept dialog
#define ExgAskOKButton                11701  
#define ExgAskCancelButton            11702  
#define ExgAskMessageField            11703
#define ExgAskIconBitMap              11704  
#define ExgAskCategoryLabel           11705      // New optional category support
#define ExgAskCategoryTrigger         11706      //
#define ExgAskCategoryList            11707      // 
#define ExgAskDataGadget              11709      // Gadget holding exchange info for handler
#define ExgAskPreviewWithCategoryGadget 11710    // Bounds for graphical preview
#define ExgAskPreviewWithoutCategoryGadget 11711 // Same for use when there's no category picker
#define ExgAskPreviewField            11712      // Long string description
#define ExgAskCategoryNameGadget      11713      // Gadget holding category name for handler
#define ExchangeQuestionString        11710      // "do you want to" question string
#define ExchangeUnnamedString         11711      // name for unnamed item
#define ExchangeIrBeamString          11712      // name for Ir Library dialogs
#define ExchangeHeadingString         11713
#define ExchangeSendString            11714      // description for send URL scheme
#define ExchangeLowBatteryAlert       11720      // Low battery Ir alert
#define ExchangeReceiveDisabledAlert  11730      // Beam Receive disabled Ir alert
#define ExchangeIrUnsupportedAlert    11740      // No Ir Hardware alert  
#define ExchangeNoBeamLibrariesAlert  11750      // No "beam" exchange libraries alert
#define ExchangeNoSendLibrariesAlert  11760      // No "send" exchange libraries alert
#define ExchangeChooseForm            11770      // Form to ask user which transport to use
#define ExchangeChooseList            11771
#define ExchangeChooseCancelButton    11772
#define ExchangeChooseOKButton        11773

//INet Library Antenna Dialog
#define INetAntennaForm               11800      // Raise antenna form
#define INetAntennaCancelButton       11801      // Cancel button for the raise antenna form
#define INetAntennaField              11802      
#define INetAntennaString             11803
#define INetWirelineString            11804
#define INetMobitexString             11805

//INet Library Device bits
#define INetLibDeviceBits1Res         11830

// Font Selector Dialog
#define FontSelectorForm              11900
#define FontSelector1Button           11903
#define FontSelector2Button           11904
#define FontSelector3Button           11905
#define FontSelectorOKButton          11906
#define FontSelectorCancelButton      11907
#define FontSelectorFontGroup         1

// Custom Alert Dialog
#define CustomAlertDialog             12000
#define CustomAlertTextEntryField     12001
#define CustomAlertBitmap             12002
#define CustomAlertField              12003
#define CustomAlertFirstButton        12004

// Time Dialog Box (SelectOneTime)
#define SelectOneTimeDialog           12100
#define timeHourButton                12105
#define timeMinutesTensButton         12106
#define timeMinutesOnesButton         12107
#define timeDecreaseButton            12109
#define timeIncreaseButton            12108
#define timeAMCheckbox                12110
#define timePMCheckbox                12111
#define timeSeparatorLabel            12102
#define timeOkButton                  12112
#define timeCancelButton              12113

// WebLib strings
#define WeblibTruncatedString         12200  // " [truncated] "
#define WeblibAssignedString          12201  // " -Assigned- "
#define WeblibUnassignedString        12202  // " -Unassigned- "
#define WeblibSelectDateString        12203  // "Select date"
#define WeblibSelectTimeString        12204  // "Select time"
#define WeblibBulletString            12205  // " \x95 "
#define WeblibImageAltTextString      12206  // "[image]"
#define WeblibCharRefString           12207  // 2 Reference chars for averageCharWidth
#define WeblibCharRefString2ndCharset 12208  // 2 Reference chars for averageCharWidth for 2nd charset (English content on Japanese Clipper)

// WebLib password Dialog
#define WebPasswordForm               12200  // Password
#define WebPasswordOKButton           12201  //
#define WebPasswordPasswordFldField   12203  // -------------
#define WebPasswordCancelButton       12204  // [OK] [Cancel]

// WebLib Lz77 Primer binary data type 'tbin'
#define WebLibLz77PrimerUncomp        12300 // Lz77 Uncompressed Primer (UInt16 len),Data...
#define WebLibLz77PrimerComp          12301 // Lz77 Compressed Primer (UInt16 len),Data...
#define WebLibFontStyleArray          12302 // Read doc in WebLibOpen() in WebLib.c
#define WebLibFontStyleSecondary      12303 // Used when feature ctpDeviceBits1Cp1252Secondary is ON
#define WebLibFontLoadMap             12304 // Fonts to load and Map: Read doc in WebLibOpen() in WebLib.c
#define WeblibConvTbl1252ToShiftJis   12305  // Japanese resource only

// Contrast Adjust Dialog
#define SysContrastAdjustForm         13100
#define SysContrastAdjustSlider       13101

// Brightness Adjust Dialog
#define SysBrightnessAdjustForm       13150
#define SysBrightnessAdjustSlider     13151

// common to Contrast & Brightness dialogs
#define SysLevelAdjustDoneButton      13102

// Old time selector that will be taken out in next release V35.
// Uses same buttons as the new Time Selector.
#define TimeSelectorFormV33           13200
#define TimeSelectorStartTimeButtonV33 TimeSelectorStartTimeButton
#define TimeSelectorEndTimeButtonV33  TimeSelectorEndTimeButton
#define TimeSelectorHourListV33       TimeSelectorHourList
#define TimeSelectorMinuteListV33     TimeSelectorMinuteList
#define TimeSelectorOKButtonV33       TimeSelectorOKButton
#define TimeSelectorCancelButtonV33   TimeSelectorCancelButton
#define TimeSelectorNoTimeButtonV33   TimeSelectorNoTimeButton

// Private record Dialogs
#define SecChangeViewForm             13200
#define SecChangeViewOkButton         13204
#define SecChangeViewCancelButton     13205
#define SecChangeViewCurPrivacyLabel  13201
#define SecChangeViewCurPrivacyList   13203
#define SecChangeViewCurPrivacyPopTrigger 13202
//Also in security app's SecRsc.h
//as a special case. No-one else should need it.
#define SecurityHelpString            10020

// Color Picker Dialog
#define ColorPickerForm               13300
#define ColorPickerOKButton           13301
#define ColorPickerCancelButton       13302
#define ColorPickerTypeTrigger        13303
#define ColorPickerTypePopupList      13304
#define ColorPickerTypePopupListPaletteItem 0
#define ColorPickerTypePopupListRGBItem     1
#define ColorPickerChitBoundsGadget   13305
#define ColorPickerRedSlider          13306
#define ColorPickerRedLabel           13307
#define ColorPickerGreenSlider        13308
#define ColorPickerGreenLabel         13309
#define ColorPickerBlueSlider         13310
#define ColorPickerBlueLabel          13311
#define ColorPickerSampleColorGadget  13312
#define ColorPickerRedValue           13313
#define ColorPickerGreenValue         13314
#define ColorPickerBlueValue          13315

// Dialog to select TimeZone
#define TimeZoneDialogForm            13400
#define TimeZoneDialogOKButton        13412
#define TimeZoneDialogCancelButton    13413
#define TimeZoneDialogCurrentTimeField 13401
#define TimeZoneDialogNewTimeField    13402
#define TimeZoneDialogNewTimeLabel    13405
#define TimeZoneDialogCurrentTimeLabel 13407
#define TimeZoneDialogTimeZoneList    13403

#define DOWformatString               13419
#define TimeZoneTipString             13400
#define UTCString                     13420
#define ZeroTimeZoneOffsetString      13421

// These three resources are linked, and form one array of
// name/gmt offset/country code records.
#define TimeZoneNamesStringList       13400  // 'tSTL'
#define TimeZoneGMTOffsetsList        13400  // 'wrdl'
#define TimeZoneCountriesList         13401  // 'wrdl'

//  Resource: tFRM 13500
#define AttentionDialog               13500
#define AttentionListAttentionsTable  13501
#define AttentionListOkButton         13503
#define AttentionListSnoozeButton     13504
#define AttentionListGotoButton       13505
#define AttentionListClearAllButton   13506
#define AttentionCloseButton          13507
#define AttentionDetailOkButton       13508
#define AttentionDetailSnoozeButton   13509
#define AttentionDetailGotoButton     13510
#define AttentionDetailDescGadget     13511
#define AttentionListUpButton         13512
#define AttentionListDownButton       13513

#define AttentionIndicatorBitmap      13510
#define AttentionIndicatorNewBitmap   13520
#define AttentionDetailTitleStrID     13500  // "Reminder"
#define AttentionListTitleStrID       13501  // "Reminders"

// Initial LED pattern.
// 0.1s on, 0.5s off, 0.1s on, 1.3sec off (2 sec total time), repeating 3 times.
// Note that the initial LED blink pattern is coordinated with the vibrator
// pattern so that the LED and vibrator aren't on at the same time, to reduce
// peak load on the battery.
#define AttentionInitialLEDPattern     13500
#define AttentionInitialLEDBlinkRate   13501
#define AttentionInitialLEDBlinkDelay  13502
#define AttentionInitialLEDRepeatCount 13503

// LED pattern to use after initial pattern completes.
// Switch to this pattern 35 seconds after starting the initial pattern.
// Was 0.1s on, 29.9s off, repeating forever, but is now simply off.
// Use these values to restore continuous blink:
// pattern = 3221225472, rate = 64, delay = 28000.
#define AttentionContinuousLEDPattern     13504
#define AttentionContinuousLEDBlinkRate   13505
#define AttentionContinuousLEDBlinkDelay  13506
#define AttentionContinuousLEDRepeatCount 13507
#define AttentionContinuousLEDStartDelay  13508

// Vibrator pattern.
// 0.5 sec on, 2 sec off (2.5 sec total time), repeating 3 times.
#define AttentionVibratePattern         13509
#define AttentionVibrateRate            13510
#define AttentionVibrateDelay           13511
#define AttentionVibrateRepeatCount     13512

// Optional hard button for snooze - use zero for none.
#define AttentionDetailSnoozeHardButton 13513
#define AttentionListSnoozeHardButton   13514

// Resource ID range 14000 - 14??? is reserved for expansion manager.

// DOLATER dje - Move this comment to Incs/PalmResources.h, along with any
// public resource IDs (if there are any).
//------------------------------------------------------------
// Reserved resource ranges
//
// Special "no resource" value             0
// Application resources:                  1 - 9999
// Reserved for system use:            10000 up
// HAL resources:                      19000 - 19999
// Bluetooth resources                 20000 - 20100
// Shared library resources:           31000 - 31999
// Locale module resources:            32000 - 32767
// Not supported in tools:             32768 - 65535 (ie. negative IDs)
//------------------------------------------------------------

#endif //__UIRESOURCESPRV_H__
