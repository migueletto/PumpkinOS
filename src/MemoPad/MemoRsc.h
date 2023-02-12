/******************************************************************************
 *
 * Copyright (c) 1995-2002 PalmSource, Inc. All rights reserved.
 *
 * File: MemoRsc.h
 *
 * Release: Palm OS Developer Suite 5 SDK (68K) 4.0
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
#define EditMemoField						1109
#define EditMemoScrollBar					1110
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

// Delete Memo Dialog
#define DeleteMemoDialog					1600
#define DeleteMemoSaveBackup				1604
#define DeleteMemoOk							1606
#define DeleteMemoCancel					1607

// Sort Memo Alert		
#define alphabeticSortAlert				2000
#define alphabeticSortYes					0
#define alphabeticSortNo					1

// Menus
#define ListViewMenuBar						1000

#define EditViewMenuBar						1100

// List View Menu commands
#define ListRecordBeamCategoryCmd		100
#define ListRecordSendCategoryCmd		101
#define ListOptionsFontsCmd				200
#define ListOptionsPreferencesCmd		201
#define ListOptionsSecurityCmd         202
#define ListOptionsAboutCmd            203

// Edit View Menu commands
#define NewMemoCmd							100
#define DeleteMemoCmd						101
#define BeamMemoCmd							103
#define SendMemoCmd							104

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
#define EditMenuCtl							100 // hasCCP, has extras
#define ListMenuCtl							200 // don't use MenuCtlxxxButtonIndex defaults
#define ListMenuCtlSecure					0

// Strings
#define FindMemoHeaderStr					100
#define BeamDescriptionStr					1000
#define BeamFilenameStr						1001
#define ExgDescriptionStr					1002
