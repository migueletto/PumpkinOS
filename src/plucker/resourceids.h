/*
 * $Id: resourceids.h,v 1.140 2004/05/12 01:52:07 prussar Exp $
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

/*
 * Be careful with this file - it's used by things other than cpp.
 */

/*

   Font resource ids are three hex digits long.

   Last hex digit:

   Std          0
   Std Bold     1
   Large        2
   Large Bold   3
   Narrow       4
   Narrow Bold  5
   Fixed        6
   Fixed Bold   7        (not available)
   Narrow Fixed 8
   Narrow Fixed Bold 9  (not available)
   Tiny         A
   Tiny Bold    B
   Small        C
   Small Bold   D

   Note that bold fonts are always right after their standard versions and non-bold fonts are
   even-numbered.

   Middle digit:

   Plucker-supplied                0
   User-substitutable - nonitalic  2
   User-substitutable - italic     6

   First digit:

   72 dpi type v1     2
   144 dpi type v1    4
   144 dpi type v2    6
*/

/* 72 dpi Fonts */
#define narrowFont              128
#define narrowFontID            0x204
#define narrowFixedFont         129
#define narrowFixedFontID       0x208
#define narrowBoldFont          130
#define narrowBoldFontID        0x205

/* 144 dpi Fonts : type v1 */
#define stdFixedFont_sony       131
#define stdFixedFontID_sony     0x406

/* 144 dpi Fonts : type v2 */
#define stdFixedFont_palm       132
#define stdFixedFontID_palm     0x606
#define tinyFont_palm           133
#define tinyFontID_palm         0x60A
#define tinyBoldFont_palm       134
#define tinyBoldFontID_palm     0x60B
#define smallFont_palm          135
#define smallFontID_palm        0x60C
#define smallBoldFont_palm      136
#define smallBoldFontID_palm    0x60D


#define START_USER_FONTS        137

/* 72 dpi Fonts */
#define userStdFont             START_USER_FONTS
#define userStdFontID           0x220
#define userBoldFont            138
#define userBoldFontID          0x221
#define userLargeFont           139
#define userLargeFontID         0x222
#define userLargeBoldFont       140
#define userLargeBoldFontID     0x223
#define userNarrowFont          141
#define userNarrowFontID        0x224
#define userFixedFont           142
#define userFixedFontID         0x226

/* 144 dpi Fonts : type v1 */
#define userStdFont_sony        143
#define userStdFontID_sony      0x420
#define userBoldFont_sony       144
#define userBoldFontID_sony     0x421
#define userLargeFont_sony      145
#define userLargeFontID_sony    0x422
#define userLargeBoldFont_sony  146
#define userLargeBoldFontID_sony 0x423
#define userNarrowFont_sony     147
#define userNarrowFontID_sony   0x424
#define userFixedFont_sony      148
#define userFixedFontID_sony    0x426

/* 144 dpi Fonts : type v2 */
#define userStdFont_palm        149
#define userStdFontID_palm      0x620
#define userBoldFont_palm       150
#define userBoldFontID_palm     0x621
#define userLargeFont_palm      151
#define userLargeFontID_palm    0x622
#define userLargeBoldFont_palm  152
#define userLargeBoldFontID_palm 0x623
#define userNarrowFont_palm     153
#define userNarrowFontID_palm   0x624
#define userFixedFont_palm      154
#define userFixedFontID_palm    0x626

#define START_ITALIC_USER_FONTS   155
#define ITALIC_FONT_DELTA         ( START_ITALIC_USER_FONTS - START_USER_FONTS )
#define ITALIC_FONTID_MASK        0x0040
#define END_ITALIC_USER_FONTS     ( START_ITALIC_USER_FONTS + \
                                    ITALIC_FONT_DELTA - 1 )
#define NUM_ITALIC_USER_FONTS     ( END_ITALIC_USER_FONTS + 1 - \
                                    START_ITALIC_USER_FONTS )

#define FONT_IS_ITALIC( x )       ( START_ITALIC_USER_FONTS <= ( x ) && \
                                    ( x ) <= END_ITALIC_USER_FONTS )
#define FONT_IS_USER( x )         ( START_USER_FONTS <= ( x ) && \
                                    ( x ) <= END_ITALIC_USER_FONTS )

/* Control IDs */
#define HOMECONTROL             0
#define LEFTCONTROL             1
#define RIGHTCONTROL            2
#define LIBRARYCONTROL          3
#define FINDCONTROL             4
#define AGAINCONTROL            5
#define MENUCONTROL             6
#define OFFSETCONTROL           7
#define BOOKMARKCONTROL         8
#define AUTOSCROLLSTARTCONTROL  9
#define AUTOSCROLLSTOPCONTROL   10
#define AUTOSCROLLINCRCONTROL   11
#define AUTOSCROLLDECRCONTROL   12
#define COPYTOMEMOCONTROL       13
#define COMMANDKEYCONTROL       14
#define SELECTEDWORDCONTROL     15

/* Control bitmaps */
#define bmpHome                 1000 + HOMECONTROL
#define bmpLeft                 1000 + LEFTCONTROL
#define bmpRight                1000 + RIGHTCONTROL
#define bmpDbase                1000 + LIBRARYCONTROL
#define bmpFind                 1000 + FINDCONTROL
#define bmpAgain                1000 + AGAINCONTROL
#define bmpMenu                 1000 + MENUCONTROL
/* RESERVED                     1000 + OFFSETCONTROL */
#define bmpBookmark             1000 + BOOKMARKCONTROL
#define bmpAutoscrollStart      1000 + AUTOSCROLLSTARTCONTROL
#define bmpAutoscrollStop       1000 + AUTOSCROLLSTOPCONTROL
#define bmpAutoscrollIncr       1000 + AUTOSCROLLINCRCONTROL
#define bmpAutoscrollDecr       1000 + AUTOSCROLLDECRCONTROL
#define bmpCopyToMemo           1000 + COPYTOMEMOCONTROL
/* RESERVED                     1000 + COMMANDKEYCONTROL */
/* RESERVED                     1000 + SELECTEDWORDCONTROL */

/* Other bitmaps */
#define bmpWait                 2000
#define bmpChicken              2001
#define bmpMode1                2002
#define bmpMode2                2003
#define bmpMode3                2004
#define bmpBar                  2005
#define bmpShortBar             2006
#define bmpSyncList             2007
#define bmpSettings             2008
#define bmpTop                  2009
#define bmpBottom               2010
#define bmpTapAction            2011
#define bmpBtnAction            2012
#define bmpGestureUp            2013
#define bmpGestureRight         2014
#define bmpGestureDown          2015
#define bmpGestureLeft          2016
#define bmpGestureTap           2017
#define bmpJogdialUp            2018
#define bmpJogdialDown          2019
#define bmpJogdialPush          2020
#define bmpJogdialPushUp        2021
#define bmpJogdialPushDown      2022
#define bmpJogdialBack          2023
#define bmpMemoryStick          2024
#define bmpMemoryStick_half     2025
#define bmpMemoryStick_x2       2026
#define bmpSmallPluckerIcon     2027
#define bmpSmallPluckerIcon_half 2028
#define bmpSmallPluckerIcon_x2  2029
#define bmpCompactFlash         2030
#define bmpCompactFlash_half    2031
#define bmpCompactFlash_x2      2032
#define bmpSecureDigital        2033
#define bmpSecureDigital_half   2034
#define bmpSecureDigital_x2     2035
#define bmpLongBar              2036
#define bmpArrowLeft            2037
#define bmpArrowRight           2038
#define bmpArrowUp              2039
#define bmpArrowDown            2040
#define bmpLookup               2041

/* Forms */
#define frmMainTop              3000
#define frmMainBottom           3001
#define frmMainNone             3002
#define frmMainScrollBar        3003
#define frmMainPercentPopup     3004
#define frmMainPercentList      3005
#define frmMainBookmarkList     3006
#define frmMainImageDialog      3007
#define frmMainImageDialogPopup 3008
#define frmMainTopHandera       3009
#define frmMainBottomHandera    3010
#define frmMainNoneHandera      3011
#define frmMainBottomHanderaLow 3012
#define frmMainBottomSonyLow    3013
#define frmMainBottomSonyWide   3014
#define frmMainSonyFull         3015
#define frmMainDrawingArea      3016

#define frmAbout                3100
#define frmAboutOK              3101

#define frmPrefs                          3200
#define frmPrefsOK                        3201
#define frmPrefsCancel                    3202
#define frmPrefsSectionPopup              3203
#define frmPrefsSectionList               3204
#define frmPrefsHelp                      3205

#define frmPrefsGeneralScreenDepthLabel   3210
#define frmPrefsGeneralScreenDepthPopup   3211
#define frmPrefsGeneralScreenDepthList    3212
#define frmPrefsGeneralScrollbarLabel     3213
#define frmPrefsGeneralScrollbarPopup     3214
#define frmPrefsGeneralScrollbarList      3215
#define frmPrefsGeneralMenuToolbarLabel   3216
#define frmPrefsGeneralMenuToolbarPopup   3217
#define frmPrefsGeneralMenuToolbarList    3218
#define frmPrefsGeneralToolbarModeLabel   3219
#define frmPrefsGeneralToolbarModePopup   3220
#define frmPrefsGeneralToolbarModeList    3221

#define frmPrefsBrowsingStrikethrough     3226
#define frmPrefsBrowsingUnderline         3227
#define frmPrefsBrowsingForceDefaultColors 3229
#define frmPrefsBrowsingPageControlsLink  3231
#define frmPrefsBrowsingAlignLabel        3232
#define frmPrefsBrowsingAlignPopup        3233
#define frmPrefsBrowsingAlignList         3234
#define frmPrefsBrowsingEnableSoftHyphens 3235

#define frmPrefsLibrarySyncPolicyLabel    3240
#define frmPrefsLibrarySyncPolicyPopup    3241
#define frmPrefsLibrarySyncPolicyList     3242
#define frmPrefsLibraryCategoryStyleLabel 3243
#define frmPrefsLibraryCategoryStylePopup 3244
#define frmPrefsLibraryCategoryStyleList  3245
#define frmPrefsLibrarySortByLabel        3246
#define frmPrefsLibrarySortByPopup        3247
#define frmPrefsLibrarySortByList         3248
#define frmPrefsLibrarySortOrderLabel     3249
#define frmPrefsLibrarySortOrderPopup     3250
#define frmPrefsLibrarySortOrderList      3251
#define frmPrefsLibraryShowType           3252
#define frmPrefsLibraryShowDate           3253
#define frmPrefsLibraryShowSize           3254
#define frmPrefsLibraryIndicateOpened     3255
#define frmPrefsLibraryDateTime           3256

#define frmPrefsAutoscrollJumpLabel       3260
#define frmPrefsAutoscrollJumpButton      3261
#define frmPrefsAutoscrollJumpUpButton    3262
#define frmPrefsAutoscrollJumpDownButton  3263
#define frmPrefsAutoscrollModePopup       3264
#define frmPrefsAutoscrollModeList        3265
#define frmPrefsAutoscrollDirLabel        3266
#define frmPrefsAutoscrollDirPopup        3267
#define frmPrefsAutoscrollDirList         3268
#define frmPrefsAutoscrollIntervalLabel   3269
#define frmPrefsAutoscrollIntervalButton  3270
#define frmPrefsAutoscrollIntervalUpButton 3271
#define frmPrefsAutoscrollIntervalDownButton 3272
#define frmPrefsAutoscrollMillisecondsLabel 3273
#define frmPrefsAutoscrollStayOn          3274

#define frmPrefsHardcopyActionLabel       3280
#define frmPrefsHardcopyActionPopup       3281
#define frmPrefsHardcopyActionList        3282
#define frmPrefsHardcopyRangeLabel        3283
#define frmPrefsHardcopyRangePopup        3284
#define frmPrefsHardcopyRangeList         3285
#define frmPrefsHardcopyLinkLabel         3286
#define frmPrefsHardcopyLinkPopup         3287
#define frmPrefsHardcopyLinkList          3288
#define frmPrefsHardcopyTitle             3289

#define frmPrefsButtonHardKeys            3290
#define frmPrefsButtonArrowKeys           3291
#define frmPrefsButtonSelectAction        3292
#define frmPrefsButtonActionList          3293
/*** DON'T CHANGE ORDER ***/
#define frmPrefsButtonDatebook            3294
#define frmPrefsButtonAddress             3295
#define frmPrefsButtonTodo                3296
#define frmPrefsButtonMemo                3297
#define frmPrefsButtonUp                  3298
#define frmPrefsButtonDown                3299
#define frmPrefsButtonLeft                3300
#define frmPrefsButtonRight               3301
#define frmPrefsButtonSelect              3302
/*** DON'T CHANGE ORDER ***/

#define frmPrefsControlList1              3310
#define frmPrefsControlList2              3311
#define frmPrefsControlList3              3312
#define frmPrefsControlList4              3313
/*** DON'T CHANGE ORDER ***/
#define frmPrefsControlPopup1             3314
#define frmPrefsControlPopup2             3315
#define frmPrefsControlPopup3             3316
#define frmPrefsControlPopup4             3317
#define frmPrefsControlMode1              3318
#define frmPrefsControlMode2              3319
#define frmPrefsControlMode3              3320
#define frmPrefsControlLabel1             3321
#define frmPrefsControlLabel2             3322
#define frmPrefsControlLabel3             3323
#define frmPrefsControlLabel4             3324
#define frmPrefsControlMsg1               3325
#define frmPrefsControlMsg2               3326
#define frmPrefsControlMsg3               3327
#define frmPrefsControlMsg4               3328
#define frmPrefsControlRegionLabel        3329

#define frmPrefsGestureGestures           3330
#define frmPrefsGestureSelectAction       3331
#define frmPrefsGestureActionList         3332
/*** DON'T CHANGE ORDER ***/
#define frmPrefsGestureUp                 3333
#define frmPrefsGestureRight              3334
#define frmPrefsGestureDown               3335
#define frmPrefsGestureLeft               3336
#define frmPrefsGestureTap                3337
/*** DON'T CHANGE ORDER ***/

#define frmPrefsJogdialJogEnabled         3340
#define frmPrefsJogdialSelectAction       3341
#define frmPrefsJogdialActionList         3342
#define frmPrefsJogdialUp                 3343
#define frmPrefsJogdialDown               3344
#define frmPrefsJogdialPush               3345
#define frmPrefsJogdialPushUp             3346
#define frmPrefsJogdialPushDown           3347
#define frmPrefsJogdialBack               3348

#define frmPrefsLookupAlwaysActive 3350
#define frmPrefsLookupActionLabel  3351
#define frmPrefsLookupActionList   3352
#define frmPrefsLookupActionPopup  3353

#define frmLibrary              3400
#define frmLibraryTable         3401
#define frmLibraryScrollBar     3402
#define frmLibraryPopup         3403
#define frmLibraryList          3404
#define frmLibraryDetails       3405
#define frmLibraryCategoryBtn   3406
#define frmLibraryDate          3407
#define frmLibrarySize          3408
#define frmLibraryHelp          3409
#define frmLibrarySortOrder     3410
#define frmLibrarySortPopup     3411
#define frmLibrarySortList      3412
#define frmRenameDoc            3413
#define frmRenameDocOK          3414
#define frmRenameDocField       3415
#define frmRenameDocCancel      3416
#define frmLibraryHandera       3417
#define frmLibraryHanderaLow    3418
#define frmLibrarySonyWide      3419
#define frmLibraryCategoryPopup 3420
#define frmLibraryCategoryList  3421
#define frmLibraryNoDocuments   3422

#define frmDetails              3500
#define frmDetailsOK            3501
#define frmDetailsCancel        3502
#define frmDetailsStatusRead    3503
#define frmDetailsStatusUnread  3504
#define frmDetailsShowImages    3505
#define frmDetailsLink          3506
#define frmDetailsCopy          3507

#define frmSearch               3700
#define frmSearchOK             3701
#define frmSearchField          3702
#define frmSearchCancel         3703
#define frmSearchList           3704
#define frmSearchPopup          3705
#define frmSearchCasesensitive  3706
#define frmSearchHistory        3707
#define frmSearchHistoryList    3708
#define frmSearchPhrase         3709
#define frmSearchXlitPopup      3710
#define frmSearchXlitList       3711
#define frmSearchHelp           3712

#define frmResult               3750
#define frmResultStop           3751
#define frmResultCancel         3752
#define frmResultFindMore       3753
#define frmResultStatus         3754
#define frmResultSonySilkMin    3755

#define frmCategory             3800
#define frmCatOK                3801
#define frmCatCancel            3802
#define frmCatAll               3803
#define frmCatNone              3804
#define frmCat1                 3805
#define frmCat2                 3806
#define frmCat3                 3807
#define frmCat4                 3808
#define frmCat5                 3809
#define frmCat6                 3810
#define frmCat7                 3811
#define frmCat8                 3812
#define frmCat9                 3813
#define frmCat10                3814
#define frmCat11                3815
#define frmCat12                3816
#define frmCat13                3817
#define frmCat14                3818
#define frmCat15                3819
#define frmCat16                3820
#define frmCatOpen1             3821
#define frmCatOpen2             3822
#define frmCatOpen3             3823
#define frmCatOpen4             3824
#define frmCatOpen5             3825
#define frmCatOpen6             3826
#define frmCatOpen7             3827
#define frmCatOpen8             3828
#define frmCatOpen9             3829
#define frmCatOpen10            3830
#define frmCatOpen11            3831
#define frmCatOpen12            3832
#define frmCatOpen13            3833
#define frmCatOpen14            3834
#define frmCatOpen15            3835
#define frmCatOpen16            3836
#define frmCatMultiple          3837
#define frmCatList              3838
#define frmCatPopup             3839
#define frmCatAND               3840
#define frmCatOR                3841
#define frmNewCategory          3842
#define frmNewCatOK             3843
#define frmNewCatCancel         3844
#define frmNewCatName           3845
/* #define frmExportLink           3849 */

#define frmEmail                3900
#define frmEmailTo              3901
#define frmEmailCc              3902
#define frmEmailSubject         3903
#define frmEmailMessage         3904
#define frmEmailSend            3905
#define frmEmailCancel          3906
#define frmEmailToArrow         3907
#define frmEmailCcArrow         3908
#define frmEmailSubjectArrow    3909
#define frmEmailScrollBar       3910
#define frmEmailToLabel         3911
#define frmEmailCcLabel         3912
#define frmEmailSubjectLabel    3913

#define frmAddBookmark          4000
#define frmAddBookmarkAdd       4001
#define frmAddBookmarkCancel    4002
#define frmAddBookmarkName      4003
#define frmBookmarks            4004
#define frmBookmarksDone        4005
#define frmBookmarksDelete      4006
#define frmBookmarksList        4007
#define frmBookmarksGo          4008

#define frmExternalLinks        4100
#define frmExternalLinksLink    4101
#define frmExternalLinksBack    4102
#define frmExternalLinksCopy    4103
#define frmExternalLinksBrowse  4104

#define frmFont                 4600
#define frmFontOS2              4601
/* AVAILABLE                    4602 */
/* AVAILABLE                    4603 */
#define frmFontLabel            4604
#define frmFontOK               4605
#define frmFontCancel           4606
#define frmFont1                4607
#define frmFont2                4608
#define frmFont3                4609
#define frmFont4                4610
#define frmFont5                4611
#define frmFontUser             4612
#define frmFontUserFontPopup    4613
#define frmFontUserFontList     4614
#define frmFontLineSpacingLabel 4615
#define frmFontLineSpacingPopup 4616
#define frmFontLineSpacingList  4617
#define frmFontParagraphSpacingLabel 4618
#define frmFontParagraphSpacingPopup 4619
#define frmFontParagraphSpacingList  4620
#ifdef HAVE_ROTATE
#define frmFontRotateLabel      4621
#define frmFontRotateList       4622
#define frmFontRotatePopup      4623
#endif
#define frmFontIndividualFonts  4624
#define frmFontAsDefault        4625
#define frmFontHandera          4626
#define frmFontHelp             4627

#define frmHardcopy             4700
#define frmHardcopyField        4701
#define frmHardcopyExport       4702
#define frmHardcopyCancel       4703
#define frmHardcopyScrollBar    4704

#define frmKeyboard             4800
#define frmKeyboardStdKey       4801
#define frmKeyboardStdKeyActionPopup 4802
#define frmKeyboardStdKeyActionList  4803
#define frmKeyboardSpecialKeyPopup   4804
#define frmKeyboardSpecialKeyList    4805
#define frmKeyboardSpecialKeyActionPopup 4806
#define frmKeyboardSpecialKeyActionList  4807
#define frmKeyboardOK           4808
#define frmKeyboardCancel       4809
#define frmKeyboardDefault      4810
#define frmKeyboardClear        4811
#define frmKeyboardHelp         4812

#define frmFullscreen           4900
#define frmFullscreenBack       4901


/* Menus */
#define menuMainForm            5000
#define menuLibraryForm         5001
#define menuFullscreenForm      5002

/*** DON'T CHANGE ORDER ***/
#define mGoHome                 5101
#define mGoBack                 5102
#define mGoForward              5103
/*** DON'T CHANGE ORDER ***/
#define mGoSearch               5104
#define mGoSearchAgain          5105
#define mGoAddBookmark          5106
#define mGoBookmarks            5107
#define mGoTop                  5108
#define mGoBottom               5109
#define mGoLibrary              5110
#define mGoDeleteDocument       5111
#define mGoLookup               5112

#define mViewTopToolbar         5201
#define mViewBottomToolbar      5202
#define mViewNoToolbar          5203
#define mViewAutoscrollStart    5204
#define mViewAutoscrollStop     5205
#define mViewAutoscrollIncr     5206
#define mViewAutoscrollDecr     5207
#define mViewDetails            5208
#define mViewToggleFullscreen   5209
#define mViewCopyToMemo         5210

#define mOptionsAbout           5301
#define mOptionsContact         5302
#define mOptionsPref            5303
#define mOptionsDeleteAll       5304
#define mOptionsFont            5305
#define mOptionsType            5306
#define mOptionsDate            5307
#define mOptionsSize            5308
#define mOptionsSyncList        5309
#define mOptionsButton          5310
#define mOptionsControl         5311
#define mOptionsDeleteRead      5312
#define mOptionsKeyboard        5313

/* Strings */
/* AVAILABLE                    6000 */
#define strResultSearching      6001
#define strResultMatches        6002
#define strResultNoMatches      6003
#define strMainAddBookmark      6004
#define strMainViewBookmark     6005
#define strControlHelp          6006
#define strDetailsHelp          6007
#define strExternNoURL          6008
#define strCatHelp              6009
#define strCatDefault           6010
#define strEditCatHelp          6011
#define strLibraryHelp          6012
#define strPrefsTopToolbar      6013
#define strPrefsBottomToolbar   6014
#define strPrefsNoToolbar       6015
#define strPrefsLargeBoldFont   6016
#define strButtonHelp           6017
#define strPrefsSilkToolbar     6018
#define strGestureHelp          6019
#define strSclHelp              6021
#define strNoDocumentsFound     6022
#define strLang                 6023
/* AVAILABLE                    6024 */
#define strLibraryName          6025
#define strLibraryDate          6026
#define strLibrarySize          6027
#define strHardcopyPrefsHelp    6028
#define strTblActions           6029
#define strPrefsGeneral         6030
#define strPrefsBrowsing        6031
#define strPrefsLibrary         6032
#define strPrefsAutoscroll      6033
#define strPrefsHardcopy        6034
#define strPrefsButton          6035
#define strPrefsControl         6036
#define strPrefsGesture         6037
#define strPrefsJogdial         6038
#define strPrefsLookup          6039
#define strPrefsGeneralHelp     6040
#define strPrefsBrowsingHelp    3040
#define strPrefsLibraryHelp     6041
#define strPrefsAutoscrollHelp  6042
#define strPrefsHardcopyHelp    6043
#define strPrefsButtonHelp      6044
#define strPrefsControlHelp     6045
#define strPrefsGestureHelp     6046
#define strPrefsJogdialHelp     6047
#define strPrefsLookupHelp      6048
#define strLibraryOpen          6049
#define strLibraryCategorize    6050
#define strLibraryDelete        6051
#define strLibraryRename        6052
#define strLibraryBeam          6053
#define strLibraryUnread        6054
#define strTblKeys              6055
#define strFontHelp             6056
#define strKeyboardHelp         6057
#define strSearchHelp           6058
#define strNoTransliteration    6059
#define strSortDoc              6060
#define strOpenDocList          6061
#define strSyncDocList          6062

/* Alerts */
#define warnBrokenDocument      7001
#define infoWrongROMVersion     7002
#define confirmDelete           7003
#define errCannotDeleteDoc      7004
#define warnInsufficientMemory  7005
#define confirmEndOfDoc         7006
#define confirmEndOfPage        7007
#define errBadMailto            7008
#define errAlreadyExists        7009
#define warnLowEmailMem         7010
#define warnNoTo                7011
#define errReadOnly             7012
#define infoTooHighBitDepth     7013
#define warnLowImageMem         7014
#define infoEmptyPatterns       7015
#define warnNoZLibSupport       7016
#define errUnknownType          7017
#define errImageError           7018
#define errNoCategory           7019
#define errCategoryExists       7020
#define confirmMergeCategory    7021
#define confirmDeleteAllDoc     7022
#define errCannotDeleteAllDoc   7023
#define infoNoBeamSupport       7024
#define infoCopyProtected       7025
#define warnStayOn              7026
#define errCannotFind           7027
#define errUnhandledException   7028
#define errCannotAddBookmark    7029
#define errCannotDeleteBookmark 7030
#define errCannotRenameDoc      7031
#define warnInvalidOwner        7032
#define warnManualSync          7033
#define warnLowHardcopyMem      7034
#define infoDebug               7035
#define confirmDeleteMetaDoc    7036
#define confirmDeleteReadDoc    7037
#define errCannotDeleteReadDoc  7038

/* #define frmShowResults          8001 */
#define frmJumpToDoc            8002
#define frmUpdateList           8003
#define frmUpdateTable          8004
#define frmViewRecord           8005
#define frmUpdateAnchors        8006

#define resizeIndex             8500

/* Armlets */
#define armDoSearch             9000
#define armRotateBitmap         9001
#define armColorizeBitmap       9002

/* Custom events */
#define pluckerSelectedWordEvent  ( firstUserEvent + 0 )
#define pluckerClosePopupEvent    ( firstUserEvent + 1 )
#define pluckerDeleteFormEvent    ( firstUserEvent + 2 )

/* Miscellaneous */
#define MAX_BOOKMARK_LEN  100

