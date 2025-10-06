/******************************************************************************
 *
 * Copyright (c) 1995-2002 PalmSource, Inc. All rights reserved.
 *
 * File: SampleRsc.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 *****************************************************************************/

// List View
#define ListView								1000
#define ListCategoryTrigger				1003
#define ListCategoryList					1004
#define ListNewButton						1005
#define ListTable								1008
#define ListScrollBar						1009

// Edit View
#define EditView								1100
#define EditViewTitle						1101
#define EditCategoryTrigger				1103
#define EditCategoryList					1104
#define EditDoneButton						1105
#define EditDetailsButton					1108
#define EditItemField						1109
#define EditItemScrollBar					1110
#define SaveChangesAlert					1100
#define SaveChangesYesButton				0
#define SaveChangeNoButton					1
#define SaveErrorAlert						1101
#define EditTitleString						1112
#define EditFontGroup						1

// Details Dialog Box
#define DetailsDialog						1200
#define DetailsCategoryTrigger			1204
#define DetailsCategoryList				1205
#define DetailsSecretCheckbox				1207
#define DetailsOkButton						1208
#define DetailsCancelButton				1209
#define DetailsDeleteButton				1210
#define DetailsHelpString					1211

// Options Dialog
#define PreferencesDialog					1400
#define PreferencesSortByTrigger			1404
#define PreferencesSortByList				1405
#define PreferencesOkButton				1406
#define PreferencesCancelButton			1407
#define PreferencesFontGroup				1

// Delete Item Dialog
#define DeleteItemDialog					1600
#define DeleteItemSaveBackup				1604
#define DeleteItemOk							1606
#define DeleteItemCancel					1607

// Sort Item Alert		
#define alphabeticSortAlert				2000
#define alphabeticSortYes					0
#define alphabeticSortNo					1

// Menus
#define ListViewMenuBar						1000

#define EditViewMenuBar						1100

// List View Menu commands
#define ListRecordOpenCmd				130//100
#define ListRecordBeamCategoryCmd		101
#define ListRecordSendCategoryCmd		102
#define ListOptionsFontsCmd				200
#define ListOptionsPreferencesCmd		201
#define ListOptionsSecurityCmd         202
#define ListOptionsAboutCmd            203

// Edit View Menu commands
#define NewItemCmd							120//100
#define SaveItemCmd							101
#define SaveItemAsCmd						102
#define DeleteItemCmd						103
#define BeamItemCmd							104
#define SendItemCmd							105

#define UndoCmd								200
#define CutCmd									201
#define CopyCmd								202
#define PasteCmd								203
#define SelectAllCmd							204
#define EditSeparator						205
#define KeyboardCmd							206

#define EditOptionsFontsCmd				300
#define EditOptionPhoneLookupCmd			301
#define EditOptionsAboutCmd				302

//Command bars
#define EditMenuCtl							110//100 // hasCCP, has extras
#define ListMenuCtl							200 // don't use MenuCtlxxxButtonIndex defaults
#define ListMenuCtlSecure					0

// Strings
#define FindItemHeaderStr					100
#define BeamDescriptionStr					1000
#define BeamFilenameStr						1001
#define ExgDescriptionStr					1002
#define FileDescriptionStr					1003

// Bitmaps
#define LargeDocumentIconBitmapFamily		1200
#define SmallDocumentIconBitmapFamily		1300

#define InformationAlert					1700
#define InformationAlertOK					0

// Alert
#define UnsupportedAlert                    1500