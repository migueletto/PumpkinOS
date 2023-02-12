/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddressRsc.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#ifndef ADDRESSRSC_H
#define ADDRESSRSC_H


// List View
#define ListView							1000
#define ListCategoryTrigger					1003
#define ListCategoryList					1004
#define ListTable							1005
#define ListLookupField						1007
#define ListNewButton						1008
#define ListUpButton						1009
#define ListDownButton						1010

// Details Dialog Box
#define DetailsDialog						1200
#define DetailsCategoryTrigger				1204
#define DetailsCategoryList					1205
#define DetailsSecretCheckbox				1207
#define DetailsOkButton						1208
#define DetailsCancelButton					1209
#define DetailsDeleteButton					1210
#define DetailsNoteButton					1211
#define DetailsPhoneList					1213
#define DetailsPhoneTrigger					1214

// Options Dialog
#define OptionsDialog						1400
#define OptionsSortByPriority				1403
#define OptionsSortByDueDate				1404
#define OptionsShowCompleted				1405
#define OptionsShowDueItems					1407
#define OptionsShowDueDates					1409
#define OptionsShowPriorities				1411
#define OptionsOkButton						1413
#define OptionsCancelButton					1414

// Detele Completed Dialog
#define DeleteCompletedDialog				1500
#define DeleteCompletedSaveBackup			1504
#define DeleteCompletedOk					1506
#define DeleteCompletedCancel				1507

// Delete Addr Dialog
#define DeleteAddrDialog					1600
#define DeleteAddrSaveBackup				1604
#define DeleteAddrOk						1606
#define DeleteAddrCancel					1607

// Address Record View
#define RecordView							1700
#define RecordCategoryLabel					1702
#define RecordDoneButton					1704
#define RecordEditButton					1705
#define RecordNewButton						1706
#define RecordUpButton						1707
#define RecordDownButton					1708
#define RecordViewDisplay					1709
#define RecordViewBusinessCardBmp			1710


// Edit Address View
#define EditView							1800
#define EditCategoryTrigger					1803
#define EditCategoryList					1804
#define EditTable							1805
#define EditDoneButton						1806
#define EditDetailsButton					1807
#define EditUpButton						1808
#define EditDownButton						1809
#define EditPhoneList						1810
#define EditNoteButton						1812
#define EditViewBusinessCardBmp				1813

// Custom Edit
#define CustomEditDialog					1900
#define CustomEditFirstField				1903
#define CustomEditOkButton					1907
#define CustomEditCancelButton				1908

// Preferences
#define PreferencesDialog						2000
#define PreferencesRememberCategoryCheckbox		2001
#define PreferencesEnableTapDialingCheckbox		2002
#define PreferencesLastName						2003
#define PreferencesCompanyName					2004
#define PreferencesOkButton						2005
#define PreferencesCancelButton					2006
#define PreferencesEnableTapDialingHeightGadget	2007

// Lookup View
#define LookupView							2100
#define LookupTitle							2101
#define LookupTable							2102
#define LookupLookupField					2104
#define LookupPasteButton					2105
#define LookupCancelButton					2106
#define LookupUpButton						2107
#define LookupDownButton					2108

// Sorting Message
#define SortingMessageDialog				2200
#define SortingMessageLabel					2201

// Addr Dial list
#define DialListDialog                      2300
#define DialListDialButton                  2301
#define DialListCancelButton                2302
#define DialListNumberField                 2305
#define DialListDescriptionGadget           2304
#define DialListPhoneRectangleGadget        2307
#define DialListNumberToDialLabel           2306
#define DialListList                        2303

// Delete Note Alert
#define DeleteNoteAlert						2001
#define DeleteNoteYes						0
#define DeleteNoteNo           			  	1

#define NameNeededAlert						2003

// Select Business Card Alert
#define SelectBusinessCardAlert				2004
#define SelectBusinessCardYes				0
#define SelectBusinessCardNo        		1

// Send Business Card Alert
#define SendBusinessCardAlert				2005

// Menus
#define ListViewMenuBar						1000
#define RecordViewMenuBar					1100
#define EditViewMenuBar						1200

// Menu commands
#define ListRecordDuplicateAddressCmd		100
#define ListRecordDialCmd					101
#define ListRecordSeparator1				102
#define ListRecordBeamCategoryCmd			103
#define ListRecordSendCategoryCmd			104
#define ListRecordBeamBusinessCardCmd		105

// The below two menu commands aren't actually on the menu anymore, but
// it is easiest to pretend that they are, for the command bar's sake.

#define ListRecordDeleteRecordCmd			106
#define ListRecordBeamRecordCmd				107

#define ListEditUndoCmd						10000
#define ListEditCutCmd						10001
#define ListEditCopyCmd						10002
#define ListEditPasteCmd					10003
#define ListEditSeparator					10004
#define ListEditKeyboardCmd					10005
#define ListEditGraffitiHelpCmd				10006


#define ListOptionsFontCmd					300
#define ListOptionsListByCmd				301
#define ListOptionsEditCustomFldsCmd		302
#define ListOptionsSecurityCmd				303
#define ListOptionsAboutCmd					304

#define RecordRecordDeleteRecordCmd			100
#define RecordRecordDuplicateAddressCmd		101
#define RecordRecordBeamRecordCmd			102
#define RecordRecordSendRecordCmd			103
#define RecordRecordDialCmd					104
#define RecordRecordSeparator1				105
#define RecordRecordAttachNoteCmd			106
#define RecordRecordDeleteNoteCmd			107
#define RecordRecordSeparator2				108
#define RecordRecordSelectBusinessCardCmd 	109
#define RecordRecordBeamBusinessCardCmd 	110

#define RecordOptionsFontCmd				200
#define RecordOptionsEditCustomFldsCmd		201
#define RecordOptionsAboutCmd				202

#define EditRecordDeleteRecordCmd			100
#define EditRecordDuplicateAddressCmd		101
#define EditRecordBeamRecordCmd				102
#define EditRecordSendRecordCmd				103
#define EditRecordDialCmd					104
#define EditRecordSeparator1				105
#define EditRecordAttachNoteCmd				106
#define EditRecordDeleteNoteCmd				107
#define EditRecordSeparator2				108
#define EditRecordSelectBusinessCardCmd 	109
#define EditRecordBeamBusinessCardCmd 		110

#define EditEditUndoCmd						10000
#define EditEditCutCmd						10001
#define EditEditCopyCmd						10002
#define EditEditPasteCmd					10003
#define EditEditSelectAllCmd				10004
#define EditSeparator						10005
#define EditEditKeyboardCmd					10006
#define EditEditGraffitiHelpCmd				10007

#define EditOptionsFontCmd					300
#define EditOptionsEditCustomFldsCmd		301
#define EditOptionsAboutCmd					302


// Strings
#define FindAddrHeaderStr					100
#define UnnamedRecordStr					1000
#define BeamDescriptionStr					1001
#define BeamFilenameStr						1002
#define DuplicatedRecordIndicatorStr		1003
#define ZipCodePrefixStr					1004
#define DeleteRecordStr						1005
#define BeamRecordStr						1006
#define ExgDescriptionStr					1007
#define ExgMultipleDescriptionStr			1008

// String lists
#define FieldNamesStrList					1000	// Reserved range up to 1255!!!

// Field mapping
#define FieldMapID							1000


#define titleAFInitStr						200
#define companyAFInitStr					201
#define cityAFInitStr						202
#define stateAFInitStr						203
#define countryAFInitStr					204

#endif // ADDRESSRSC_H
