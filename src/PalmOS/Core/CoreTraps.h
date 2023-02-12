/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: CoreTraps.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *      Palm OS core trap numbers
 *
 *****************************************************************************/

 #ifndef __CORETRAPS_H_
 #define __CORETRAPS_H_

// Include elementary types
#include <PalmTypes.h>

#if CPU_TYPE == CPU_68K
#include <M68KHwr.h>
#endif

// Regular traps start here and go up by 1's
#define	sysTrapBase										0xA000

// ======================================================================
// Palm OS 1.0 Traps
// ======================================================================

#define sysTrapMemInit									0xA000
#define sysTrapMemInitHeapTable							0xA001
#define sysTrapMemStoreInit								0xA002
#define sysTrapMemCardFormat							0xA003
#define sysTrapMemCardInfo								0xA004
#define sysTrapMemStoreInfo								0xA005
#define sysTrapMemStoreSetInfo							0xA006
#define sysTrapMemNumHeaps								0xA007
#define sysTrapMemNumRAMHeaps							0xA008
#define sysTrapMemHeapID								0xA009
#define sysTrapMemHeapPtr								0xA00A
#define sysTrapMemHeapFreeBytes							0xA00B
#define sysTrapMemHeapSize								0xA00C
#define sysTrapMemHeapFlags								0xA00D
#define sysTrapMemHeapCompact							0xA00E
#define sysTrapMemHeapInit								0xA00F
#define sysTrapMemHeapFreeByOwnerID						0xA010
#define sysTrapMemChunkNew								0xA011
#define sysTrapMemChunkFree								0xA012
#define sysTrapMemPtrNew								0xA013
#define sysTrapMemPtrRecoverHandle						0xA014
#define sysTrapMemPtrFlags								0xA015
#define sysTrapMemPtrSize								0xA016
#define sysTrapMemPtrOwner								0xA017
#define sysTrapMemPtrHeapID								0xA018
#define sysTrapMemPtrCardNo								0xA019
#define sysTrapMemPtrToLocalID							0xA01A
#define sysTrapMemPtrSetOwner							0xA01B
#define sysTrapMemPtrResize								0xA01C
#define sysTrapMemPtrResetLock							0xA01D
#define sysTrapMemHandleNew								0xA01E
#define sysTrapMemHandleLockCount						0xA01F
#define sysTrapMemHandleToLocalID						0xA020
#define sysTrapMemHandleLock							0xA021
#define sysTrapMemHandleUnlock							0xA022
#define sysTrapMemLocalIDToGlobal						0xA023
#define sysTrapMemLocalIDKind							0xA024
#define sysTrapMemLocalIDToPtr							0xA025
#define sysTrapMemMove									0xA026
#define sysTrapMemSet									0xA027
#define sysTrapMemStoreSearch							0xA028
#define sysTrapSysReserved10Trap1						0xA029  /* "Reserved" trap in Palm OS 1.0 and later (was sysTrapMemPtrDataStorage) */

#define sysTrapMemKernelInit							0xA02A
#define sysTrapMemHandleFree							0xA02B
#define sysTrapMemHandleFlags							0xA02C
#define sysTrapMemHandleSize							0xA02D
#define sysTrapMemHandleOwner							0xA02E
#define sysTrapMemHandleHeapID							0xA02F
#define sysTrapMemHandleDataStorage						0xA030
#define sysTrapMemHandleCardNo							0xA031
#define sysTrapMemHandleSetOwner						0xA032
#define sysTrapMemHandleResize							0xA033
#define sysTrapMemHandleResetLock						0xA034
#define sysTrapMemPtrUnlock								0xA035
#define sysTrapMemLocalIDToLockedPtr					0xA036
#define sysTrapMemSetDebugMode							0xA037
#define sysTrapMemHeapScramble							0xA038
#define sysTrapMemHeapCheck								0xA039
#define sysTrapMemNumCards								0xA03A
#define sysTrapMemDebugMode								0xA03B
#define sysTrapMemSemaphoreReserve						0xA03C
#define sysTrapMemSemaphoreRelease						0xA03D
#define sysTrapMemHeapDynamic							0xA03E
#define sysTrapMemNVParams								0xA03F


#define sysTrapDmInit									0xA040
#define sysTrapDmCreateDatabase							0xA041
#define sysTrapDmDeleteDatabase							0xA042
#define sysTrapDmNumDatabases							0xA043
#define sysTrapDmGetDatabase							0xA044
#define sysTrapDmFindDatabase							0xA045
#define sysTrapDmDatabaseInfo							0xA046
#define sysTrapDmSetDatabaseInfo						0xA047
#define sysTrapDmDatabaseSize							0xA048
#define sysTrapDmOpenDatabase							0xA049
#define sysTrapDmCloseDatabase							0xA04A
#define sysTrapDmNextOpenDatabase						0xA04B
#define sysTrapDmOpenDatabaseInfo						0xA04C
#define sysTrapDmResetRecordStates						0xA04D
#define sysTrapDmGetLastErr								0xA04E
#define sysTrapDmNumRecords								0xA04F
#define sysTrapDmRecordInfo								0xA050
#define sysTrapDmSetRecordInfo							0xA051
#define sysTrapDmAttachRecord							0xA052
#define sysTrapDmDetachRecord							0xA053
#define sysTrapDmMoveRecord								0xA054
#define sysTrapDmNewRecord								0xA055
#define sysTrapDmRemoveRecord							0xA056
#define sysTrapDmDeleteRecord							0xA057
#define sysTrapDmArchiveRecord							0xA058
#define sysTrapDmNewHandle								0xA059
#define sysTrapDmRemoveSecretRecords					0xA05A
#define sysTrapDmQueryRecord							0xA05B
#define sysTrapDmGetRecord								0xA05C
#define sysTrapDmResizeRecord							0xA05D
#define sysTrapDmReleaseRecord							0xA05E
#define sysTrapDmGetResource							0xA05F
#define sysTrapDmGet1Resource							0xA060
#define sysTrapDmReleaseResource						0xA061
#define sysTrapDmResizeResource							0xA062
#define sysTrapDmNextOpenResDatabase					0xA063
#define sysTrapDmFindResourceType						0xA064
#define sysTrapDmFindResource							0xA065
#define sysTrapDmSearchResource							0xA066
#define sysTrapDmNumResources							0xA067
#define sysTrapDmResourceInfo							0xA068
#define sysTrapDmSetResourceInfo						0xA069
#define sysTrapDmAttachResource							0xA06A
#define sysTrapDmDetachResource							0xA06B
#define sysTrapDmNewResource							0xA06C
#define sysTrapDmRemoveResource							0xA06D
#define sysTrapDmGetResourceIndex						0xA06E
#define sysTrapDmQuickSort								0xA06F
#define sysTrapDmQueryNextInCategory					0xA070
#define sysTrapDmNumRecordsInCategory					0xA071
#define sysTrapDmPositionInCategory						0xA072
#define sysTrapDmSeekRecordInCategory					0xA073
#define sysTrapDmMoveCategory							0xA074
#define sysTrapDmOpenDatabaseByTypeCreator				0xA075
#define sysTrapDmWrite									0xA076
#define sysTrapDmStrCopy								0xA077
#define sysTrapDmGetNextDatabaseByTypeCreator			0xA078
#define sysTrapDmWriteCheck								0xA079
#define sysTrapDmMoveOpenDBContext						0xA07A
#define sysTrapDmFindRecordByID							0xA07B
#define sysTrapDmGetAppInfoID							0xA07C
#define sysTrapDmFindSortPositionV10					0xA07D
#define sysTrapDmSet									0xA07E
#define sysTrapDmCreateDatabaseFromImage				0xA07F

#define sysTrapDbgSrcMessage							0xA080
#define sysTrapDbgMessage								0xA081
#define sysTrapDbgGetMessage							0xA082
#define sysTrapDbgCommSettings							0xA083

#define sysTrapErrDisplayFileLineMsg					0xA084
#define sysTrapErrSetJump								0xA085
#define sysTrapErrLongJump								0xA086
#define sysTrapErrThrow									0xA087
#define sysTrapErrExceptionList							0xA088

#define sysTrapSysBroadcastActionCode					0xA089
#define sysTrapSysUnimplemented							0xA08A
#define sysTrapSysColdBoot								0xA08B
#define sysTrapSysReset									0xA08C
#define sysTrapSysDoze									0xA08D
#define sysTrapSysAppLaunch								0xA08E
#define sysTrapSysAppStartup							0xA08F
#define sysTrapSysAppExit								0xA090
#define sysTrapSysSetA5									0xA091
#define sysTrapSysSetTrapAddress						0xA092
#define sysTrapSysGetTrapAddress						0xA093
#define sysTrapSysTranslateKernelErr					0xA094
#define sysTrapSysSemaphoreCreate						0xA095
#define sysTrapSysSemaphoreDelete						0xA096
#define sysTrapSysSemaphoreWait							0xA097
#define sysTrapSysSemaphoreSignal						0xA098
#define sysTrapSysTimerCreate							0xA099
#define sysTrapSysTimerWrite							0xA09A
#define sysTrapSysTaskCreate							0xA09B
#define sysTrapSysTaskDelete							0xA09C
#define sysTrapSysTaskTrigger							0xA09D
#define sysTrapSysTaskID								0xA09E
#define sysTrapSysTaskUserInfoPtr						0xA09F
#define sysTrapSysTaskDelay								0xA0A0
#define sysTrapSysTaskSetTermProc						0xA0A1
#define sysTrapSysUILaunch								0xA0A2
#define sysTrapSysNewOwnerID							0xA0A3
#define sysTrapSysSemaphoreSet							0xA0A4
#define sysTrapSysDisableInts							0xA0A5
#define sysTrapSysRestoreStatus							0xA0A6
#define sysTrapSysUIAppSwitch							0xA0A7
#define sysTrapSysCurAppInfoPV20						0xA0A8
#define sysTrapSysHandleEvent							0xA0A9
#define sysTrapSysInit									0xA0AA
#define sysTrapSysQSort									0xA0AB
#define sysTrapSysCurAppDatabase						0xA0AC
#define sysTrapSysFatalAlert							0xA0AD
#define sysTrapSysResSemaphoreCreate					0xA0AE
#define sysTrapSysResSemaphoreDelete					0xA0AF
#define sysTrapSysResSemaphoreReserve					0xA0B0
#define sysTrapSysResSemaphoreRelease					0xA0B1
#define sysTrapSysSleep									0xA0B2
#define sysTrapSysKeyboardDialogV10						0xA0B3
#define sysTrapSysAppLauncherDialog						0xA0B4
#define sysTrapSysSetPerformance						0xA0B5
#define sysTrapSysBatteryInfoV20						0xA0B6
#define sysTrapSysLibInstall							0xA0B7
#define sysTrapSysLibRemove								0xA0B8
#define sysTrapSysLibTblEntry							0xA0B9
#define sysTrapSysLibFind								0xA0BA
#define sysTrapSysBatteryDialog							0xA0BB
#define sysTrapSysCopyStringResource					0xA0BC
#define sysTrapSysKernelInfo							0xA0BD
#define sysTrapSysLaunchConsole							0xA0BE
#define sysTrapSysTimerDelete							0xA0BF
#define sysTrapSysSetAutoOffTime						0xA0C0
#define sysTrapSysFormPointerArrayToStrings				0xA0C1
#define sysTrapSysRandom								0xA0C2
#define sysTrapSysTaskSwitching							0xA0C3
#define sysTrapSysTimerRead								0xA0C4

#define sysTrapStrCopy									0xA0C5
#define sysTrapStrCat									0xA0C6
#define sysTrapStrLen									0xA0C7
#define sysTrapStrCompare								0xA0C8
#define sysTrapStrIToA									0xA0C9
#define sysTrapStrCaselessCompare						0xA0CA
#define sysTrapStrIToH									0xA0CB
#define sysTrapStrChr									0xA0CC
#define sysTrapStrStr									0xA0CD
#define sysTrapStrAToI									0xA0CE
#define sysTrapStrToLower								0xA0CF

#define sysTrapSerReceiveISP							0xA0D0

#define sysTrapSlkOpen									0xA0D1
#define sysTrapSlkClose									0xA0D2
#define sysTrapSlkOpenSocket							0xA0D3
#define sysTrapSlkCloseSocket							0xA0D4
#define sysTrapSlkSocketRefNum							0xA0D5
#define sysTrapSlkSocketSetTimeout						0xA0D6
#define sysTrapSlkFlushSocket							0xA0D7
#define sysTrapSlkSetSocketListener						0xA0D8
#define sysTrapSlkSendPacket							0xA0D9
#define sysTrapSlkReceivePacket							0xA0DA
#define sysTrapSlkSysPktDefaultResponse					0xA0DB
#define sysTrapSlkProcessRPC							0xA0DC

#define sysTrapConPutS									0xA0DD
#define sysTrapConGetS									0xA0DE

#define sysTrapFplInit									0xA0DF  /* Obsolete, here for compatibilty only! */
#define sysTrapFplFree									0xA0E0  /* Obsolete, here for compatibilty only! */
#define sysTrapFplFToA									0xA0E1  /* Obsolete, here for compatibilty only! */
#define sysTrapFplAToF									0xA0E2  /* Obsolete, here for compatibilty only! */
#define sysTrapFplBase10Info							0xA0E3  /* Obsolete, here for compatibilty only! */
#define sysTrapFplLongToFloat							0xA0E4  /* Obsolete, here for compatibilty only! */
#define sysTrapFplFloatToLong							0xA0E5  /* Obsolete, here for compatibilty only! */
#define sysTrapFplFloatToULong							0xA0E6  /* Obsolete, here for compatibilty only! */
#define sysTrapFplMul									0xA0E7  /* Obsolete, here for compatibilty only! */
#define sysTrapFplAdd									0xA0E8  /* Obsolete, here for compatibilty only! */
#define sysTrapFplSub									0xA0E9  /* Obsolete, here for compatibilty only! */
#define sysTrapFplDiv									0xA0EA  /* Obsolete, here for compatibilty only! */

#define sysTrapWinScreenInit							0xA0EB  /* was sysTrapScrInit */
#define sysTrapScrCopyRectangle							0xA0EC
#define sysTrapScrDrawChars								0xA0ED
#define sysTrapScrLineRoutine							0xA0EE
#define sysTrapScrRectangleRoutine						0xA0EF
#define sysTrapScrScreenInfo							0xA0F0
#define sysTrapScrDrawNotify							0xA0F1
#define sysTrapScrSendUpdateArea						0xA0F2
#define sysTrapScrCompressScanLine						0xA0F3
#define sysTrapScrDeCompressScanLine					0xA0F4

#define sysTrapTimGetSeconds							0xA0F5
#define sysTrapTimSetSeconds							0xA0F6
#define sysTrapTimGetTicks								0xA0F7
#define sysTrapTimInit									0xA0F8
#define sysTrapTimSetAlarm								0xA0F9
#define sysTrapTimGetAlarm								0xA0FA
#define sysTrapTimHandleInterrupt						0xA0FB
#define sysTrapTimSecondsToDateTime						0xA0FC
#define sysTrapTimDateTimeToSeconds						0xA0FD
#define sysTrapTimAdjust								0xA0FE
#define sysTrapTimSleep									0xA0FF
#define sysTrapTimWake									0xA100

#define sysTrapCategoryCreateListV10					0xA101
#define sysTrapCategoryFreeListV10						0xA102
#define sysTrapCategoryFind								0xA103
#define sysTrapCategoryGetName							0xA104
#define sysTrapCategoryEditV10							0xA105
#define sysTrapCategorySelectV10						0xA106
#define sysTrapCategoryGetNext							0xA107
#define sysTrapCategorySetTriggerLabel					0xA108
#define sysTrapCategoryTruncateName						0xA109

#define sysTrapClipboardAddItem							0xA10A
#define sysTrapClipboardCheckIfItemExist				0xA10B
#define sysTrapClipboardGetItem							0xA10C

#define sysTrapCtlDrawControl							0xA10D
#define sysTrapCtlEraseControl							0xA10E
#define sysTrapCtlHideControl							0xA10F
#define sysTrapCtlShowControl							0xA110
#define sysTrapCtlGetValue								0xA111
#define sysTrapCtlSetValue								0xA112
#define sysTrapCtlGetLabel								0xA113
#define sysTrapCtlSetLabel								0xA114
#define sysTrapCtlHandleEvent							0xA115
#define sysTrapCtlHitControl							0xA116
#define sysTrapCtlSetEnabled							0xA117
#define sysTrapCtlSetUsable								0xA118
#define sysTrapCtlEnabled								0xA119

#define sysTrapEvtInitialize							0xA11A
#define sysTrapEvtAddEventToQueue						0xA11B
#define sysTrapEvtCopyEvent								0xA11C
#define sysTrapEvtGetEvent								0xA11D
#define sysTrapEvtGetPen								0xA11E
#define sysTrapEvtSysInit								0xA11F
#define sysTrapEvtGetSysEvent							0xA120
#define sysTrapEvtProcessSoftKeyStroke					0xA121
#define sysTrapEvtGetPenBtnList							0xA122
#define sysTrapEvtSetPenQueuePtr						0xA123
#define sysTrapEvtPenQueueSize							0xA124
#define sysTrapEvtFlushPenQueue							0xA125
#define sysTrapEvtEnqueuePenPoint						0xA126
#define sysTrapEvtDequeuePenStrokeInfo					0xA127
#define sysTrapEvtDequeuePenPoint						0xA128
#define sysTrapEvtFlushNextPenStroke					0xA129
#define sysTrapEvtSetKeyQueuePtr						0xA12A
#define sysTrapEvtKeyQueueSize							0xA12B
#define sysTrapEvtFlushKeyQueue							0xA12C
#define sysTrapEvtEnqueueKey							0xA12D
#define sysTrapEvtDequeueKeyEvent						0xA12E
#define sysTrapEvtWakeup								0xA12F
#define sysTrapEvtResetAutoOffTimer						0xA130
#define sysTrapEvtKeyQueueEmpty							0xA131
#define sysTrapEvtEnableGraffiti						0xA132

#define sysTrapFldCopy									0xA133
#define sysTrapFldCut									0xA134
#define sysTrapFldDrawField								0xA135
#define sysTrapFldEraseField							0xA136
#define sysTrapFldFreeMemory							0xA137
#define sysTrapFldGetBounds								0xA138
#define sysTrapFldGetTextPtr							0xA139
#define sysTrapFldGetSelection							0xA13A
#define sysTrapFldHandleEvent							0xA13B
#define sysTrapFldPaste									0xA13C
#define sysTrapFldRecalculateField						0xA13D
#define sysTrapFldSetBounds								0xA13E
#define sysTrapFldSetText								0xA13F
#define sysTrapFldGetFont								0xA140
#define sysTrapFldSetFont								0xA141
#define sysTrapFldSetSelection							0xA142
#define sysTrapFldGrabFocus								0xA143
#define sysTrapFldReleaseFocus							0xA144
#define sysTrapFldGetInsPtPosition						0xA145
#define sysTrapFldSetInsPtPosition						0xA146
#define sysTrapFldSetScrollPosition						0xA147
#define sysTrapFldGetScrollPosition						0xA148
#define sysTrapFldGetTextHeight							0xA149
#define sysTrapFldGetTextAllocatedSize					0xA14A
#define sysTrapFldGetTextLength							0xA14B
#define sysTrapFldScrollField							0xA14C
#define sysTrapFldScrollable							0xA14D
#define sysTrapFldGetVisibleLines						0xA14E
#define sysTrapFldGetAttributes							0xA14F
#define sysTrapFldSetAttributes							0xA150
#define sysTrapFldSendChangeNotification				0xA151
#define sysTrapFldCalcFieldHeight						0xA152
#define sysTrapFldGetTextHandle							0xA153
#define sysTrapFldCompactText							0xA154
#define sysTrapFldDirty									0xA155
#define sysTrapFldWordWrap								0xA156
#define sysTrapFldSetTextAllocatedSize					0xA157
#define sysTrapFldSetTextHandle							0xA158
#define sysTrapFldSetTextPtr							0xA159
#define sysTrapFldGetMaxChars							0xA15A
#define sysTrapFldSetMaxChars							0xA15B
#define sysTrapFldSetUsable								0xA15C
#define sysTrapFldInsert								0xA15D
#define sysTrapFldDelete								0xA15E
#define sysTrapFldUndo									0xA15F
#define sysTrapFldSetDirty								0xA160
#define sysTrapFldSendHeightChangeNotification			0xA161
#define sysTrapFldMakeFullyVisible						0xA162

#define sysTrapFntGetFont								0xA163
#define sysTrapFntSetFont								0xA164
#define sysTrapFntGetFontPtr							0xA165
#define sysTrapFntBaseLine								0xA166
#define sysTrapFntCharHeight							0xA167
#define sysTrapFntLineHeight							0xA168
#define sysTrapFntAverageCharWidth						0xA169
#define sysTrapFntCharWidth								0xA16A
#define sysTrapFntCharsWidth							0xA16B
#define sysTrapFntDescenderHeight						0xA16C
#define sysTrapFntCharsInWidth							0xA16D
#define sysTrapFntLineWidth								0xA16E

#define sysTrapFrmInitForm								0xA16F
#define sysTrapFrmDeleteForm							0xA170
#define sysTrapFrmDrawForm								0xA171
#define sysTrapFrmEraseForm								0xA172
#define sysTrapFrmGetActiveForm							0xA173
#define sysTrapFrmSetActiveForm							0xA174
#define sysTrapFrmGetActiveFormID						0xA175
#define sysTrapFrmGetUserModifiedState					0xA176
#define sysTrapFrmSetNotUserModified					0xA177
#define sysTrapFrmGetFocus								0xA178
#define sysTrapFrmSetFocus								0xA179
#define sysTrapFrmHandleEvent							0xA17A
#define sysTrapFrmGetFormBounds							0xA17B
#define sysTrapFrmGetWindowHandle						0xA17C
#define sysTrapFrmGetFormId								0xA17D
#define sysTrapFrmGetFormPtr							0xA17E
#define sysTrapFrmGetNumberOfObjects					0xA17F
#define sysTrapFrmGetObjectIndex						0xA180
#define sysTrapFrmGetObjectId							0xA181
#define sysTrapFrmGetObjectType							0xA182
#define sysTrapFrmGetObjectPtr							0xA183
#define sysTrapFrmHideObject							0xA184
#define sysTrapFrmShowObject							0xA185
#define sysTrapFrmGetObjectPosition						0xA186
#define sysTrapFrmSetObjectPosition						0xA187
#define sysTrapFrmGetControlValue						0xA188
#define sysTrapFrmSetControlValue						0xA189
#define sysTrapFrmGetControlGroupSelection				0xA18A
#define sysTrapFrmSetControlGroupSelection				0xA18B
#define sysTrapFrmCopyLabel								0xA18C
#define sysTrapFrmSetLabel								0xA18D
#define sysTrapFrmGetLabel								0xA18E
#define sysTrapFrmSetCategoryLabel						0xA18F
#define sysTrapFrmGetTitle								0xA190
#define sysTrapFrmSetTitle								0xA191
#define sysTrapFrmAlert									0xA192
#define sysTrapFrmDoDialog								0xA193
#define sysTrapFrmCustomAlert							0xA194
#define sysTrapFrmHelp									0xA195
#define sysTrapFrmUpdateScrollers						0xA196
#define sysTrapFrmGetFirstForm							0xA197
#define sysTrapFrmVisible								0xA198
#define sysTrapFrmGetObjectBounds						0xA199
#define sysTrapFrmCopyTitle								0xA19A
#define sysTrapFrmGotoForm								0xA19B
#define sysTrapFrmPopupForm								0xA19C
#define sysTrapFrmUpdateForm							0xA19D
#define sysTrapFrmReturnToForm							0xA19E
#define sysTrapFrmSetEventHandler						0xA19F
#define sysTrapFrmDispatchEvent							0xA1A0
#define sysTrapFrmCloseAllForms							0xA1A1
#define sysTrapFrmSaveAllForms							0xA1A2
#define sysTrapFrmGetGadgetData							0xA1A3
#define sysTrapFrmSetGadgetData							0xA1A4
#define sysTrapFrmSetCategoryTrigger					0xA1A5

#define sysTrapUIInitialize								0xA1A6
#define sysTrapUIReset									0xA1A7

#define sysTrapInsPtInitialize							0xA1A8
#define sysTrapInsPtSetLocation							0xA1A9
#define sysTrapInsPtGetLocation							0xA1AA
#define sysTrapInsPtEnable								0xA1AB
#define sysTrapInsPtEnabled								0xA1AC
#define sysTrapInsPtSetHeight							0xA1AD
#define sysTrapInsPtGetHeight							0xA1AE
#define sysTrapInsPtCheckBlink							0xA1AF

#define sysTrapLstSetDrawFunction						0xA1B0
#define sysTrapLstDrawList								0xA1B1
#define sysTrapLstEraseList								0xA1B2
#define sysTrapLstGetSelection							0xA1B3
#define sysTrapLstGetSelectionText						0xA1B4
#define sysTrapLstHandleEvent							0xA1B5
#define sysTrapLstSetHeight								0xA1B6
#define sysTrapLstSetSelection							0xA1B7
#define sysTrapLstSetListChoices						0xA1B8
#define sysTrapLstMakeItemVisible						0xA1B9
#define sysTrapLstGetNumberOfItems						0xA1BA
#define sysTrapLstPopupList								0xA1BB
#define sysTrapLstSetPosition							0xA1BC

#define sysTrapMenuInit									0xA1BD
#define sysTrapMenuDispose								0xA1BE
#define sysTrapMenuHandleEvent							0xA1BF
#define sysTrapMenuDrawMenu								0xA1C0
#define sysTrapMenuEraseStatus							0xA1C1
#define sysTrapMenuGetActiveMenu						0xA1C2
#define sysTrapMenuSetActiveMenu						0xA1C3

#define sysTrapRctSetRectangle							0xA1C4
#define sysTrapRctCopyRectangle							0xA1C5
#define sysTrapRctInsetRectangle						0xA1C6
#define sysTrapRctOffsetRectangle						0xA1C7
#define sysTrapRctPtInRectangle							0xA1C8
#define sysTrapRctGetIntersection						0xA1C9

#define sysTrapTblDrawTable								0xA1CA
#define sysTrapTblEraseTable							0xA1CB
#define sysTrapTblHandleEvent							0xA1CC
#define sysTrapTblGetItemBounds							0xA1CD
#define sysTrapTblSelectItem							0xA1CE
#define sysTrapTblGetItemInt							0xA1CF
#define sysTrapTblSetItemInt							0xA1D0
#define sysTrapTblSetItemStyle							0xA1D1
#define sysTrapTblUnhighlightSelection					0xA1D2
#define sysTrapTblSetRowUsable							0xA1D3
#define sysTrapTblGetNumberOfRows						0xA1D4
#define sysTrapTblSetCustomDrawProcedure				0xA1D5
#define sysTrapTblSetRowSelectable						0xA1D6
#define sysTrapTblRowSelectable							0xA1D7
#define sysTrapTblSetLoadDataProcedure					0xA1D8
#define sysTrapTblSetSaveDataProcedure					0xA1D9
#define sysTrapTblGetBounds								0xA1DA
#define sysTrapTblSetRowHeight							0xA1DB
#define sysTrapTblGetColumnWidth						0xA1DC
#define sysTrapTblGetRowID								0xA1DD
#define sysTrapTblSetRowID								0xA1DE
#define sysTrapTblMarkRowInvalid						0xA1DF
#define sysTrapTblMarkTableInvalid						0xA1E0
#define sysTrapTblGetSelection							0xA1E1
#define sysTrapTblInsertRow								0xA1E2
#define sysTrapTblRemoveRow								0xA1E3
#define sysTrapTblRowInvalid							0xA1E4
#define sysTrapTblRedrawTable							0xA1E5
#define sysTrapTblRowUsable								0xA1E6
#define sysTrapTblReleaseFocus							0xA1E7
#define sysTrapTblEditing								0xA1E8
#define sysTrapTblGetCurrentField						0xA1E9
#define sysTrapTblSetColumnUsable						0xA1EA
#define sysTrapTblGetRowHeight							0xA1EB
#define sysTrapTblSetColumnWidth						0xA1EC
#define sysTrapTblGrabFocus								0xA1ED
#define sysTrapTblSetItemPtr							0xA1EE
#define sysTrapTblFindRowID								0xA1EF
#define sysTrapTblGetLastUsableRow						0xA1F0
#define sysTrapTblGetColumnSpacing						0xA1F1
#define sysTrapTblFindRowData							0xA1F2
#define sysTrapTblGetRowData							0xA1F3
#define sysTrapTblSetRowData							0xA1F4
#define sysTrapTblSetColumnSpacing						0xA1F5

#define sysTrapWinCreateWindow							0xA1F6
#define sysTrapWinCreateOffscreenWindow					0xA1F7
#define sysTrapWinDeleteWindow							0xA1F8
#define sysTrapWinInitializeWindow						0xA1F9
#define sysTrapWinAddWindow								0xA1FA
#define sysTrapWinRemoveWindow							0xA1FB
#define sysTrapWinSetActiveWindow						0xA1FC
#define sysTrapWinSetDrawWindow							0xA1FD
#define sysTrapWinGetDrawWindow							0xA1FE
#define sysTrapWinGetActiveWindow						0xA1FF
#define sysTrapWinGetDisplayWindow						0xA200
#define sysTrapWinGetFirstWindow						0xA201
#define sysTrapWinEnableWindow							0xA202
#define sysTrapWinDisableWindow							0xA203
#define sysTrapWinGetWindowFrameRect					0xA204
#define sysTrapWinDrawWindowFrame						0xA205
#define sysTrapWinEraseWindow							0xA206
#define sysTrapWinSaveBits								0xA207
#define sysTrapWinRestoreBits							0xA208
#define sysTrapWinCopyRectangle							0xA209
#define sysTrapWinScrollRectangle						0xA20A
#define sysTrapWinGetDisplayExtent						0xA20B
#define sysTrapWinGetWindowExtent						0xA20C
#define sysTrapWinDisplayToWindowPt						0xA20D
#define sysTrapWinWindowToDisplayPt						0xA20E
#define sysTrapWinGetClip								0xA20F
#define sysTrapWinSetClip								0xA210
#define sysTrapWinResetClip								0xA211
#define sysTrapWinClipRectangle							0xA212
#define sysTrapWinDrawLine								0xA213
#define sysTrapWinDrawGrayLine							0xA214
#define sysTrapWinEraseLine								0xA215
#define sysTrapWinInvertLine							0xA216
#define sysTrapWinFillLine								0xA217
#define sysTrapWinDrawRectangle							0xA218
#define sysTrapWinEraseRectangle						0xA219
#define sysTrapWinInvertRectangle						0xA21A
#define sysTrapWinDrawRectangleFrame					0xA21B
#define sysTrapWinDrawGrayRectangleFrame				0xA21C
#define sysTrapWinEraseRectangleFrame					0xA21D
#define sysTrapWinInvertRectangleFrame					0xA21E
#define sysTrapWinGetFramesRectangle					0xA21F
#define sysTrapWinDrawChars								0xA220
#define sysTrapWinEraseChars							0xA221
#define sysTrapWinInvertChars							0xA222
#define sysTrapWinGetPattern							0xA223
#define sysTrapWinSetPattern							0xA224
#define sysTrapWinSetUnderlineMode						0xA225
#define sysTrapWinDrawBitmap							0xA226
#define sysTrapWinModal									0xA227
#define sysTrapWinGetDrawWindowBounds					0xA228
#define sysTrapWinFillRectangle							0xA229
#define sysTrapWinDrawInvertedChars						0xA22A

#define sysTrapPrefOpenPreferenceDBV10					0xA22B
#define sysTrapPrefGetPreferences						0xA22C
#define sysTrapPrefSetPreferences						0xA22D
#define sysTrapPrefGetAppPreferencesV10					0xA22E
#define sysTrapPrefSetAppPreferencesV10					0xA22F

#define sysTrapSndInit									0xA230
#define sysTrapSndSetDefaultVolume						0xA231
#define sysTrapSndGetDefaultVolume						0xA232
#define sysTrapSndDoCmd									0xA233
#define sysTrapSndPlaySystemSound						0xA234

#define sysTrapAlmInit									0xA235
#define sysTrapAlmCancelAll								0xA236
#define sysTrapAlmAlarmCallback							0xA237
#define sysTrapAlmSetAlarm								0xA238
#define sysTrapAlmGetAlarm								0xA239
#define sysTrapAlmDisplayAlarm							0xA23A
#define sysTrapAlmEnableNotification					0xA23B

#define sysTrapHwrGetRAMMapping							0xA23C
#define sysTrapHwrMemWritable							0xA23D
#define sysTrapHwrMemReadable							0xA23E
#define sysTrapHwrDoze									0xA23F
#define sysTrapHwrSleep									0xA240
#define sysTrapHwrWake									0xA241
#define sysTrapHwrSetSystemClock						0xA242
#define sysTrapHwrSetCPUDutyCycle						0xA243
#define sysTrapHwrDisplayInit							0xA244  /* Before OS 3.5, this trap a.k.a. sysTrapHwrLCDInit */
#define sysTrapHwrDisplaySleep							0xA245  /* Before OS 3.5, this trap a.k.a. sysTrapHwrLCDSleep, */
#define sysTrapHwrTimerInit								0xA246
#define sysTrapHwrCursorV33								0xA247  /* This trap obsoleted for OS 3.5 and later */
#define sysTrapHwrBatteryLevel							0xA248
#define sysTrapHwrDelay									0xA249
#define sysTrapHwrEnableDataWrites						0xA24A
#define sysTrapHwrDisableDataWrites						0xA24B
#define sysTrapHwrLCDBaseAddrV33						0xA24C  /* This trap obsoleted for OS 3.5 and later */
#define sysTrapHwrDisplayDrawBootScreen					0xA24D  /* Before OS 3.5, this trap a.k.a. sysTrapHwrLCDDrawBitmap */
#define sysTrapHwrTimerSleep							0xA24E
#define sysTrapHwrTimerWake								0xA24F
#define sysTrapHwrDisplayWake							0xA250  /* Before OS 3.5, this trap a.k.a. sysTrapHwrLCDWake */
#define sysTrapHwrIRQ1Handler							0xA251
#define sysTrapHwrIRQ2Handler							0xA252
#define sysTrapHwrIRQ3Handler							0xA253
#define sysTrapHwrIRQ4Handler							0xA254
#define sysTrapHwrIRQ5Handler							0xA255
#define sysTrapHwrIRQ6Handler							0xA256
#define sysTrapHwrDockSignals							0xA257
#define sysTrapHwrPluggedIn								0xA258

#define sysTrapCrc16CalcBlock							0xA259

#define sysTrapSelectDayV10								0xA25A
#define sysTrapSelectTimeV33							0xA25B

#define sysTrapDayDrawDaySelector						0xA25C
#define sysTrapDayHandleEvent							0xA25D
#define sysTrapDayDrawDays								0xA25E
#define sysTrapDayOfWeek								0xA25F
#define sysTrapDaysInMonth								0xA260
#define sysTrapDayOfMonth								0xA261

#define sysTrapDateDaysToDate							0xA262
#define sysTrapDateToDays								0xA263
#define sysTrapDateAdjust								0xA264
#define sysTrapDateSecondsToDate						0xA265
#define sysTrapDateToAscii								0xA266
#define sysTrapDateToDOWDMFormat						0xA267
#define sysTrapTimeToAscii								0xA268

#define sysTrapFind										0xA269
#define sysTrapFindStrInStr								0xA26A
#define sysTrapFindSaveMatch							0xA26B
#define sysTrapFindGetLineBounds						0xA26C
#define sysTrapFindDrawHeader							0xA26D

#define sysTrapPenOpen									0xA26E
#define sysTrapPenClose									0xA26F
#define sysTrapPenGetRawPen								0xA270
#define sysTrapPenCalibrate								0xA271
#define sysTrapPenRawToScreen							0xA272
#define sysTrapPenScreenToRaw							0xA273
#define sysTrapPenResetCalibration						0xA274
#define sysTrapPenSleep									0xA275
#define sysTrapPenWake									0xA276

#define sysTrapResLoadForm								0xA277
#define sysTrapResLoadMenu								0xA278

#define sysTrapFtrInit									0xA279
#define sysTrapFtrUnregister							0xA27A
#define sysTrapFtrGet									0xA27B
#define sysTrapFtrSet									0xA27C
#define sysTrapFtrGetByIndex							0xA27D

#define sysTrapGrfInit									0xA27E
#define sysTrapGrfFree									0xA27F
#define sysTrapGrfGetState								0xA280
#define sysTrapGrfSetState								0xA281
#define sysTrapGrfFlushPoints							0xA282
#define sysTrapGrfAddPoint								0xA283
#define sysTrapGrfInitState								0xA284
#define sysTrapGrfCleanState							0xA285
#define sysTrapGrfMatch									0xA286
#define sysTrapGrfGetMacro								0xA287
#define sysTrapGrfFilterPoints							0xA288
#define sysTrapGrfGetNumPoints							0xA289
#define sysTrapGrfGetPoint								0xA28A
#define sysTrapGrfFindBranch							0xA28B
#define sysTrapGrfMatchGlyph							0xA28C
#define sysTrapGrfGetGlyphMapping						0xA28D
#define sysTrapGrfGetMacroName							0xA28E
#define sysTrapGrfDeleteMacro							0xA28F
#define sysTrapGrfAddMacro								0xA290
#define sysTrapGrfGetAndExpandMacro						0xA291
#define sysTrapGrfProcessStroke							0xA292
#define sysTrapGrfFieldChange							0xA293

#define sysTrapGetCharSortValue							0xA294
#define sysTrapGetCharAttr								0xA295
#define sysTrapGetCharCaselessValue						0xA296

#define sysTrapPwdExists								0xA297
#define sysTrapPwdVerify								0xA298
#define sysTrapPwdSet									0xA299
#define sysTrapPwdRemove								0xA29A

#define sysTrapGsiInitialize							0xA29B
#define sysTrapGsiSetLocation							0xA29C
#define sysTrapGsiEnable								0xA29D
#define sysTrapGsiEnabled								0xA29E
#define sysTrapGsiSetShiftState							0xA29F

#define sysTrapKeyInit									0xA2A0
#define sysTrapKeyHandleInterrupt						0xA2A1
#define sysTrapKeyCurrentState							0xA2A2
#define sysTrapKeyResetDoubleTap						0xA2A3
#define sysTrapKeyRates									0xA2A4
#define sysTrapKeySleep									0xA2A5
#define sysTrapKeyWake									0xA2A6

#define sysTrapDlkControl								0xA2A7  /* was sysTrapCmBroadcast */

#define sysTrapDlkStartServer							0xA2A8
#define sysTrapDlkGetSyncInfo							0xA2A9
#define sysTrapDlkSetLogEntry							0xA2AA

#define sysTrapIntlDispatch								0xA2AB  /* REUSED IN v3.1 (was sysTrapPsrInit in 1.0, removed in 2.0) */
#define sysTrapSysLibLoad								0xA2AC  /* REUSED IN v2.0 (was sysTrapPsrClose) */
#define sysTrapSndPlaySmf								0xA2AD  /* REUSED IN v3.0 (was sysTrapPsrGetCommand in 1.0, removed in 2.0) */
#define sysTrapSndCreateMidiList						0xA2AE  /* REUSED IN v3.0 (was sysTrapPsrSendReply in 1.0, removed in 2.0) */

#define sysTrapAbtShowAbout								0xA2AF

#define sysTrapMdmDial									0xA2B0
#define sysTrapMdmHangUp								0xA2B1

#define sysTrapDmSearchRecord							0xA2B2

#define sysTrapSysInsertionSort							0xA2B3
#define sysTrapDmInsertionSort							0xA2B4

#define sysTrapLstSetTopItem							0xA2B5

// ======================================================================
// Palm OS 2.X traps					Palm Pilot and 2.0 Upgrade Card
// ======================================================================

#define sysTrapSclSetScrollBar							0xA2B6
#define sysTrapSclDrawScrollBar							0xA2B7
#define sysTrapSclHandleEvent							0xA2B8

#define sysTrapSysMailboxCreate							0xA2B9
#define sysTrapSysMailboxDelete							0xA2BA
#define sysTrapSysMailboxFlush							0xA2BB
#define sysTrapSysMailboxSend							0xA2BC
#define sysTrapSysMailboxWait							0xA2BD

#define sysTrapSysTaskWait								0xA2BE
#define sysTrapSysTaskWake								0xA2BF
#define sysTrapSysTaskWaitClr							0xA2C0
#define sysTrapSysTaskSuspend							0xA2C1
#define sysTrapSysTaskResume							0xA2C2

#define sysTrapCategoryCreateList						0xA2C3
#define sysTrapCategoryFreeList							0xA2C4
#define sysTrapCategoryEditV20							0xA2C5
#define sysTrapCategorySelect							0xA2C6

#define sysTrapDmDeleteCategory							0xA2C7

#define sysTrapSysEvGroupCreate							0xA2C8
#define sysTrapSysEvGroupSignal							0xA2C9
#define sysTrapSysEvGroupRead							0xA2CA
#define sysTrapSysEvGroupWait							0xA2CB

#define sysTrapEvtEventAvail							0xA2CC
#define sysTrapEvtSysEventAvail							0xA2CD
#define sysTrapStrNCopy									0xA2CE

#define sysTrapKeySetMask								0xA2CF

#define sysTrapSelectDay								0xA2D0

#define sysTrapPrefGetPreference						0xA2D1
#define sysTrapPrefSetPreference						0xA2D2
#define sysTrapPrefGetAppPreferences					0xA2D3
#define sysTrapPrefSetAppPreferences					0xA2D4

#define sysTrapFrmPointInTitle							0xA2D5

#define sysTrapStrNCat									0xA2D6

#define sysTrapMemCmp									0xA2D7

#define sysTrapTblSetColumnEditIndicator				0xA2D8

#define sysTrapFntWordWrap								0xA2D9

#define sysTrapFldGetScrollValues						0xA2DA

#define sysTrapSysCreateDataBaseList					0xA2DB
#define sysTrapSysCreatePanelList						0xA2DC

#define sysTrapDlkDispatchRequest						0xA2DD

#define sysTrapStrPrintF								0xA2DE
#define sysTrapStrVPrintF								0xA2DF

#define sysTrapPrefOpenPreferenceDB						0xA2E0

#define sysTrapSysGraffitiReferenceDialog				0xA2E1

#define sysTrapSysKeyboardDialog						0xA2E2

#define sysTrapFntWordWrapReverseNLines					0xA2E3
#define sysTrapFntGetScrollValues						0xA2E4

#define sysTrapTblSetRowStaticHeight					0xA2E5
#define sysTrapTblHasScrollBar							0xA2E6

#define sysTrapSclGetScrollBar							0xA2E7

#define sysTrapFldGetNumberOfBlankLines					0xA2E8

#define sysTrapSysTicksPerSecond						0xA2E9
#define sysTrapHwrBacklightV33							0xA2EA  /* This trap obsoleted for OS 3.5 and later */
#define sysTrapDmDatabaseProtect						0xA2EB

#define sysTrapTblSetBounds								0xA2EC

#define sysTrapStrNCompare								0xA2ED
#define sysTrapStrNCaselessCompare						0xA2EE

#define sysTrapPhoneNumberLookup						0xA2EF

#define sysTrapFrmSetMenu								0xA2F0

#define sysTrapEncDigestMD5								0xA2F1

#define sysTrapDmFindSortPosition						0xA2F2

#define sysTrapSysBinarySearch							0xA2F3
#define sysTrapSysErrString								0xA2F4
#define sysTrapSysStringByIndex							0xA2F5

#define sysTrapEvtAddUniqueEventToQueue					0xA2F6

#define sysTrapStrLocalizeNumber						0xA2F7
#define sysTrapStrDelocalizeNumber						0xA2F8
#define sysTrapLocGetNumberSeparators					0xA2F9

#define sysTrapMenuSetActiveMenuRscID					0xA2FA

#define sysTrapLstScrollList							0xA2FB

#define sysTrapCategoryInitialize						0xA2FC

#define sysTrapEncDigestMD4								0xA2FD
#define sysTrapEncDES									0xA2FE

#define sysTrapLstGetVisibleItems						0xA2FF

#define sysTrapWinSetBounds								0xA300

#define sysTrapCategorySetName							0xA301

#define sysTrapFldSetInsertionPoint						0xA302

#define sysTrapFrmSetObjectBounds						0xA303

#define sysTrapWinSetColors								0xA304

#define sysTrapFlpDispatch								0xA305
#define sysTrapFlpEmDispatch							0xA306

// ======================================================================
// Palm OS 3.0 traps					Palm III and 3.0 Upgrade Card
// ======================================================================

#define sysTrapExgInit									0xA307
#define sysTrapExgConnect								0xA308
#define sysTrapExgPut									0xA309
#define sysTrapExgGet									0xA30A
#define sysTrapExgAccept								0xA30B
#define sysTrapExgDisconnect							0xA30C
#define sysTrapExgSend									0xA30D
#define sysTrapExgReceive								0xA30E
#define sysTrapExgRegisterData							0xA30F
#define sysTrapExgNotifyReceiveV35						0xA310
#define sysTrapSysReserved30Trap2 						0xA311	/* "Reserved" trap in Palm OS 3.0 and later (was sysTrapExgControl) */

#define sysTrapPrgStartDialogV31						0xA312  /* Updated in v3.2 */
#define sysTrapPrgStopDialog							0xA313
#define sysTrapPrgUpdateDialog							0xA314
#define sysTrapPrgHandleEvent							0xA315

#define sysTrapImcReadFieldNoSemicolon					0xA316
#define sysTrapImcReadFieldQuotablePrintable			0xA317
#define sysTrapImcReadPropertyParameter					0xA318
#define sysTrapImcSkipAllPropertyParameters				0xA319
#define sysTrapImcReadWhiteSpace						0xA31A
#define sysTrapImcWriteQuotedPrintable					0xA31B
#define sysTrapImcWriteNoSemicolon						0xA31C
#define sysTrapImcStringIsAscii							0xA31D

#define sysTrapTblGetItemFont							0xA31E
#define sysTrapTblSetItemFont							0xA31F

#define sysTrapFontSelect								0xA320
#define sysTrapFntDefineFont							0xA321

#define sysTrapCategoryEdit								0xA322

#define sysTrapSysGetOSVersionString					0xA323
#define sysTrapSysBatteryInfo							0xA324
#define sysTrapSysUIBusy								0xA325

#define sysTrapWinValidateHandle						0xA326
#define sysTrapFrmValidatePtr							0xA327
#define sysTrapCtlValidatePointer						0xA328
#define sysTrapWinMoveWindowAddr						0xA329
#define sysTrapFrmAddSpaceForObject						0xA32A
#define sysTrapFrmNewForm								0xA32B
#define sysTrapCtlNewControl							0xA32C
#define sysTrapFldNewField								0xA32D
#define sysTrapLstNewList								0xA32E
#define sysTrapFrmNewLabel								0xA32F
#define sysTrapFrmNewBitmap								0xA330
#define sysTrapFrmNewGadget								0xA331

#define sysTrapFileOpen									0xA332
#define sysTrapFileClose								0xA333
#define sysTrapFileDelete								0xA334
#define sysTrapFileReadLow								0xA335
#define sysTrapFileWrite								0xA336
#define sysTrapFileSeek									0xA337
#define sysTrapFileTell									0xA338
#define sysTrapFileTruncate								0xA339
#define sysTrapFileControl								0xA33A

#define sysTrapFrmActiveState							0xA33B

#define sysTrapSysGetAppInfo							0xA33C
#define sysTrapSysGetStackInfo							0xA33D

#define sysTrapWinScreenMode							0xA33E  /* was sysTrapScrDisplayMode */
#define sysTrapHwrLCDGetDepthV33						0xA33F  /* This trap obsoleted for OS 3.5 and later */
#define sysTrapHwrGetROMToken							0xA340

#define sysTrapDbgControl								0xA341

#define sysTrapExgDBRead								0xA342
#define sysTrapExgDBWrite								0xA343

#define sysTrapHostControl								0xA344  /* Renamed from sysTrapSysGremlins, functionality generalized */
#define sysTrapFrmRemoveObject							0xA345

#define sysTrapSysReserved30Trap1						0xA346  /* "Reserved" trap in Palm OS 3.0 and later (was sysTrapSysReserved1) */

// NOTE: The following two traps are reserved for future mgrs
// that may or may not be present on any particular device.
// They are NOT present by default; code must check first!
#define sysTrapExpansionDispatch						0xA347  /* Reserved for ExpansionMgr (was sysTrapSysReserved2) */
#define sysTrapFileSystemDispatch						0xA348  /* Reserved for FileSystemMgr (was sysTrapSysReserved3) */

#define sysTrapOEMDispatch								0xA349  /* OEM trap in Palm OS 3.0 and later trap table (formerly sysTrapSysReserved4) */

// ======================================================================
// Palm OS 3.1 traps					Palm IIIx and Palm V
// ======================================================================

#define sysTrapHwrLCDContrastV33						0xA34A  /* This trap obsoleted for OS 3.5 and later */
#define sysTrapSysLCDContrast							0xA34B
#define sysTrapUIContrastAdjust							0xA34C  /* Renamed from sysTrapContrastAdjust */
#define sysTrapHwrDockStatus							0xA34D

#define sysTrapFntWidthToOffset							0xA34E
#define sysTrapSelectOneTime							0xA34F
#define sysTrapWinDrawChar								0xA350
#define sysTrapWinDrawTruncChars						0xA351

#define sysTrapSysNotifyInit							0xA352  /* Notification Manager traps */
#define sysTrapSysNotifyRegister						0xA353
#define sysTrapSysNotifyUnregister						0xA354
#define sysTrapSysNotifyBroadcast						0xA355
#define sysTrapSysNotifyBroadcastDeferred				0xA356
#define sysTrapSysNotifyDatabaseAdded					0xA357
#define sysTrapSysNotifyDatabaseRemoved					0xA358

#define sysTrapSysWantEvent								0xA359

#define sysTrapFtrPtrNew								0xA35A
#define sysTrapFtrPtrFree								0xA35B
#define sysTrapFtrPtrResize								0xA35C

#define sysTrapSysReserved31Trap1						0xA35D  /* "Reserved" trap in Palm OS 3.1 and later (was sysTrapSysReserved5) */

// ======================================================================
// Palm OS 3.2 & 3.3 traps		Palm VII (3.2) and Fall '99 Palm OS Flash Update (3.3)
// ======================================================================

#define sysTrapHwrNVPrefSet								0xA35E  /* mapped to FlashParmsWrite */
#define sysTrapHwrNVPrefGet								0xA35F  /* mapped to FlashParmsRead */
#define sysTrapFlashInit								0xA360
#define sysTrapFlashCompress							0xA361
#define sysTrapFlashErase								0xA362
#define sysTrapFlashProgram								0xA363

#define sysTrapAlmTimeChange							0xA364
#define sysTrapErrAlertCustom							0xA365
#define sysTrapPrgStartDialog							0xA366  /* New version of sysTrapPrgStartDialogV31 */

#define sysTrapSerialDispatch							0xA367
#define sysTrapHwrBattery								0xA368
#define sysTrapDmGetDatabaseLockState					0xA369

#define sysTrapCncGetProfileList						0xA36A
#define sysTrapCncGetProfileInfo						0xA36B
#define sysTrapCncAddProfile							0xA36C
#define sysTrapCncDeleteProfile							0xA36D

#define sysTrapSndPlaySmfResource						0xA36E

#define sysTrapMemPtrDataStorage						0xA36F  /* Never actually installed until now. */

#define sysTrapClipboardAppendItem						0xA370

#define sysTrapWiCmdV32									0xA371  /* Code moved to INetLib; trap obsolete */

// ======================================================================
// Palm OS 3.5 traps				Palm IIIc and other products
// ======================================================================

// HAL Display-layer new traps
#define sysTrapHwrDisplayAttributes						0xA372
#define sysTrapHwrDisplayDoze							0xA373
#define sysTrapHwrDisplayPalette						0xA374

// Screen driver new traps
#define sysTrapBltFindIndexes							0xA375
#define sysTrapBmpGetBits								0xA376  /* was BltGetBitsAddr */
#define sysTrapBltCopyRectangle							0xA377
#define sysTrapBltDrawChars								0xA378
#define sysTrapBltLineRoutine							0xA379
#define sysTrapBltRectangleRoutine						0xA37A

// ScrUtils new traps
#define sysTrapScrCompress								0xA37B
#define sysTrapScrDecompress							0xA37C

// System Manager new traps
#define sysTrapSysLCDBrightness							0xA37D

// WindowColor new traps
#define sysTrapWinPaintChar								0xA37E
#define sysTrapWinPaintChars							0xA37F
#define sysTrapWinPaintBitmap							0xA380
#define sysTrapWinGetPixel								0xA381
#define sysTrapWinPaintPixel							0xA382
#define sysTrapWinDrawPixel								0xA383
#define sysTrapWinErasePixel							0xA384
#define sysTrapWinInvertPixel							0xA385
#define sysTrapWinPaintPixels							0xA386
#define sysTrapWinPaintLines							0xA387
#define sysTrapWinPaintLine								0xA388
#define sysTrapWinPaintRectangle						0xA389
#define sysTrapWinPaintRectangleFrame					0xA38A
#define sysTrapWinPaintPolygon							0xA38B
#define sysTrapWinDrawPolygon							0xA38C
#define sysTrapWinErasePolygon							0xA38D
#define sysTrapWinInvertPolygon							0xA38E
#define sysTrapWinFillPolygon							0xA38F
#define sysTrapWinPaintArc								0xA390
#define sysTrapWinDrawArc								0xA391
#define sysTrapWinEraseArc								0xA392
#define sysTrapWinInvertArc								0xA393
#define sysTrapWinFillArc								0xA394
#define sysTrapWinPushDrawState							0xA395
#define sysTrapWinPopDrawState							0xA396
#define sysTrapWinSetDrawMode							0xA397
#define sysTrapWinSetForeColor							0xA398
#define sysTrapWinSetBackColor							0xA399
#define sysTrapWinSetTextColor							0xA39A
#define sysTrapWinGetPatternType						0xA39B
#define sysTrapWinSetPatternType						0xA39C
#define sysTrapWinPalette								0xA39D
#define sysTrapWinRGBToIndex							0xA39E
#define sysTrapWinIndexToRGB							0xA39F
#define sysTrapWinScreenLock							0xA3A0
#define sysTrapWinScreenUnlock							0xA3A1
#define sysTrapWinGetBitmap								0xA3A2

// UIColor new traps
#define sysTrapUIColorInit								0xA3A3
#define sysTrapUIColorGetTableEntryIndex				0xA3A4
#define sysTrapUIColorGetTableEntryRGB					0xA3A5
#define sysTrapUIColorSetTableEntry						0xA3A6
#define sysTrapUIColorPushTable							0xA3A7
#define sysTrapUIColorPopTable							0xA3A8

// misc cleanup and API additions

#define sysTrapCtlNewGraphicControl						0xA3A9

#define sysTrapTblGetItemPtr							0xA3AA

#define sysTrapUIBrightnessAdjust						0xA3AB
#define sysTrapUIPickColor								0xA3AC

#define sysTrapEvtSetAutoOffTimer						0xA3AD

// Misc int'l/overlay support.
#define sysTrapTsmDispatch								0xA3AE
#define sysTrapOmDispatch								0xA3AF
#define sysTrapDmOpenDBNoOverlay						0xA3B0
#define sysTrapDmOpenDBWithLocale						0xA3B1
#define sysTrapResLoadConstant							0xA3B2

// new boot-time SmallROM HAL additions
#define sysTrapHwrPreDebugInit							0xA3B3
#define sysTrapHwrResetNMI								0xA3B4
#define sysTrapHwrResetPWM								0xA3B5

#define sysTrapKeyBootKeys								0xA3B6

#define sysTrapDbgSerDrvOpen							0xA3B7
#define sysTrapDbgSerDrvClose							0xA3B8
#define sysTrapDbgSerDrvControl							0xA3B9
#define sysTrapDbgSerDrvStatus							0xA3BA
#define sysTrapDbgSerDrvWriteChar						0xA3BB
#define sysTrapDbgSerDrvReadChar						0xA3BC

// new boot-time BigROM HAL additions
#define sysTrapHwrPostDebugInit							0xA3BD
#define sysTrapHwrIdentifyFeatures						0xA3BE
#define sysTrapHwrModelSpecificInit						0xA3BF
#define sysTrapHwrModelInitStage2						0xA3C0
#define sysTrapHwrInterruptsInit						0xA3C1

#define sysTrapHwrSoundOn								0xA3C2
#define sysTrapHwrSoundOff								0xA3C3

// Kernel clock tick routine
#define sysTrapSysKernelClockTick						0xA3C4

// MenuEraseMenu is exposed as of PalmOS 3.5, but there are
// no public interfaces for it yet.	 Perhaps in a later release.
#define sysTrapMenuEraseMenu							0xA3C5

#define sysTrapSelectTime								0xA3C6

// Menu Command Bar traps
#define sysTrapMenuCmdBarAddButton						0xA3C7
#define sysTrapMenuCmdBarGetButtonData					0xA3C8
#define sysTrapMenuCmdBarDisplay						0xA3C9

// Silkscreen info
#define sysTrapHwrGetSilkscreenID						0xA3CA
#define sysTrapEvtGetSilkscreenAreaList					0xA3CB

#define sysTrapSysFatalAlertInit						0xA3CC
#define sysTrapDateTemplateToAscii						0xA3CD

// New traps dealing with masking private records
#define sysTrapSecVerifyPW								0xA3CE
#define sysTrapSecSelectViewStatus						0xA3CF
#define sysTrapTblSetColumnMasked						0xA3D0
#define sysTrapTblSetRowMasked							0xA3D1
#define sysTrapTblRowMasked								0xA3D2

// New form trap for dialogs with text entry field
#define sysTrapFrmCustomResponseAlert					0xA3D3
#define sysTrapFrmNewGsi								0xA3D4

// New dynamic menu functions
#define sysTrapMenuShowItem								0xA3D5
#define sysTrapMenuHideItem								0xA3D6
#define sysTrapMenuAddItem								0xA3D7

// New form traps for "smart gadgets"
#define sysTrapFrmSetGadgetHandler						0xA3D8

// More new control functions
#define sysTrapCtlSetGraphics							0xA3D9
#define sysTrapCtlGetSliderValues						0xA3DA
#define sysTrapCtlSetSliderValues						0xA3DB
#define sysTrapCtlNewSliderControl						0xA3DC

// Bitmap manager functions
#define sysTrapBmpCreate								0xA3DD
#define sysTrapBmpDelete								0xA3DE
#define sysTrapBmpCompress								0xA3DF
// sysTrapBmpGetBits defined in Screen driver traps
#define sysTrapBmpGetColortable							0xA3E0
#define sysTrapBmpSize									0xA3E1
#define sysTrapBmpBitsSize								0xA3E2
#define sysTrapBmpColortableSize						0xA3E3

// extra window namager 
#define sysTrapWinCreateBitmapWindow					0xA3E4

// Ask for a null event sooner (replaces a macro which Poser hated)
#define sysTrapEvtSetNullEventTick						0xA3E5

// Exchange manager call to allow apps to select destination categories
#define sysTrapExgDoDialog								0xA3E6

// this call will remove temporary UI like popup lists
#define sysTrapSysUICleanup								0xA3E7

// The following 4 traps were "Reserved" traps, present only in SOME post-release builds of Palm OS 3.5
#define sysTrapWinSetForeColorRGB						0xA3E8
#define sysTrapWinSetBackColorRGB						0xA3E9
#define sysTrapWinSetTextColorRGB						0xA3EA
#define sysTrapWinGetPixelRGB							0xA3EB	

// ======================================================================
// Palm OS 4.0 Traps
// ======================================================================

#define sysTrapHighDensityDispatch						0xA3EC
#define sysTrapSysReserved40Trap2						0xA3ED
#define sysTrapSysReserved40Trap3						0xA3EE
#define sysTrapSysReserved40Trap4						0xA3EF

// New Trap selector added for New Connection Mgr API 
#define sysTrapCncMgrDispatch							0xA3F0

// new trap for notify from interrupt, implemented in SysEvtMgr.c
#define sysTrapSysNotifyBroadcastFromInterrupt			0xA3F1

// new trap for waking the UI without generating a null event
#define sysTrapEvtWakeupWithoutNilEvent					0xA3F2

// new trap for doing stable, fast, 7-bit string compare
#define sysTrapStrCompareAscii							0xA3F3

// New trap for accessors available thru PalmOS glue
#define sysTrapAccessorDispatch							0xA3F4

#define sysTrapBltGetPixel								0xA3F5	
#define sysTrapBltPaintPixel							0xA3F6

#define sysTrapScrScreenInit							0xA3F7
#define sysTrapScrUpdateScreenBitmap					0xA3F8
#define sysTrapScrPalette								0xA3F9
#define sysTrapScrGetColortable							0xA3FA
#define sysTrapScrGetGrayPat							0xA3FB
#define sysTrapScrScreenLock							0xA3FC
#define sysTrapScrScreenUnlock							0xA3FD

#define sysTrapFntPrvGetFontList						0xA3FE

// Exchange manager functions
#define sysTrapExgRegisterDatatype						0xA3FF
#define sysTrapExgNotifyReceive							0xA400
#define sysTrapExgNotifyGoto							0xA401
#define sysTrapExgRequest								0xA402
#define sysTrapExgSetDefaultApplication					0xA403
#define sysTrapExgGetDefaultApplication					0xA404
#define sysTrapExgGetTargetApplication					0xA405
#define sysTrapExgGetRegisteredApplications				0xA406
#define sysTrapExgGetRegisteredTypes					0xA407
#define sysTrapExgNotifyPreview							0xA408
#define sysTrapExgControl								0xA409

// 04/30/00	CS - New Locale Manager handles access to region-specific info like date formats
#define sysTrapLmDispatch								0xA40A

// 05/10/00 kwk - New Memory Manager trap for retrieving ROM NVParam values (sys use only)
#define sysTrapMemGetRomNVParams						0xA40B

// 05/12/00 kwk - Safe character width Font Mgr call
#define sysTrapFntWCharWidth							0xA40C

// 05/17/00 kwk - Faster DmFindDatabase
#define sysTrapDmFindDatabaseWithTypeCreator			0xA40D

// New Trap selectors added for time zone picker API
#define sysTrapSelectTimeZone							0xA40E
#define sysTrapTimeZoneToAscii							0xA40F

// 08/18/00 kwk - trap for doing stable, fast, 7-bit string compare.
// 08/21/00 kwk - moved here in place of sysTrapSelectDaylightSavingAdjustment.
#define sysTrapStrNCompareAscii							0xA410

// New Trap selectors added for time zone conversion API
#define sysTrapTimTimeZoneToUTC							0xA411
#define sysTrapTimUTCToTimeZone							0xA412

// New trap implemented in PhoneLookup.c
#define sysTrapPhoneNumberLookupCustom					0xA413

// new trap for selecting debugger path.
#define sysTrapHwrDebugSelect							0xA414

#define sysTrapBltRoundedRectangle						0xA415
#define sysTrapBltRoundedRectangleFill					0xA416
#define sysTrapWinPrvInitCanvas							0xA417

#define sysTrapHwrCalcDynamicHeapSize					0xA418
#define sysTrapHwrDebuggerEnter							0xA419
#define sysTrapHwrDebuggerExit							0xA41A

#define sysTrapLstGetTopItem							0xA41B

#define sysTrapHwrModelInitStage3						0xA41C

// 06/21/00 peter - New Attention Manager
#define sysTrapAttnIndicatorAllow						0xA41D
#define sysTrapAttnIndicatorAllowed						0xA41E
#define sysTrapAttnIndicatorEnable						0xA41F
#define sysTrapAttnIndicatorEnabled						0xA420
#define sysTrapAttnIndicatorSetBlinkPattern				0xA421
#define sysTrapAttnIndicatorGetBlinkPattern				0xA422
#define sysTrapAttnIndicatorTicksTillNextBlink			0xA423
#define sysTrapAttnIndicatorCheckBlink					0xA424
#define sysTrapAttnInitialize							0xA425
#define sysTrapAttnGetAttention							0xA426
#define sysTrapAttnUpdate								0xA427
#define sysTrapAttnForgetIt								0xA428
#define sysTrapAttnGetCounts							0xA429
#define sysTrapAttnListOpen								0xA42A
#define sysTrapAttnHandleEvent							0xA42B
#define sysTrapAttnEffectOfEvent						0xA42C
#define sysTrapAttnIterate								0xA42D
#define sysTrapAttnDoSpecialEffects						0xA42E
#define sysTrapAttnDoEmergencySpecialEffects			0xA42F
#define sysTrapAttnAllowClose							0xA430
#define sysTrapAttnReopen								0xA431
#define sysTrapAttnEnableNotification					0xA432
#define sysTrapHwrLEDAttributes							0xA433
#define sysTrapHwrVibrateAttributes						0xA434

// Trap for getting and setting the device password hint.
#define sysTrapSecGetPwdHint							0xA435
#define sysTrapSecSetPwdHint							0xA436

#define sysTrapHwrFlashWrite							0xA437

#define sysTrapKeyboardStatusNew						0xA438
#define sysTrapKeyboardStatusFree						0xA439
#define sysTrapKbdSetLayout								0xA43A
#define sysTrapKbdGetLayout								0xA43B
#define sysTrapKbdSetPosition							0xA43C
#define sysTrapKbdGetPosition							0xA43D
#define sysTrapKbdSetShiftState							0xA43E
#define sysTrapKbdGetShiftState							0xA43F
#define sysTrapKbdDraw									0xA440
#define sysTrapKbdErase									0xA441
#define sysTrapKbdHandleEvent							0xA442

#define sysTrapOEMDispatch2								0xA443

#define sysTrapHwrCustom								0xA444

// 08/28/00 kwk - Trap for getting form's active field.
#define sysTrapFrmGetActiveField						0xA445

// 9/18/00 rkr - Added for playing sounds regardless of interruptible flag
#define sysTrapSndPlaySmfIrregardless					0xA446
#define sysTrapSndPlaySmfResourceIrregardless			0xA447
#define sysTrapSndInterruptSmfIrregardless				0xA448

// 10/14/00 ABa: UDA manager
#define sysTrapUdaMgrDispatch							0xA449

// WK: private traps for PalmOS
#define sysTrapPalmPrivate1								0xA44A
#define sysTrapPalmPrivate2								0xA44B
#define sysTrapPalmPrivate3								0xA44C
#define sysTrapPalmPrivate4								0xA44D

// 11/07/00 tlw: Added accessors
#define sysTrapBmpGetDimensions							0xA44E
#define sysTrapBmpGetBitDepth							0xA44F
#define sysTrapBmpGetNextBitmap							0xA450
#define sysTrapTblGetNumberOfColumns					0xA451
#define sysTrapTblGetTopRow								0xA452
#define sysTrapTblSetSelection							0xA453
#define sysTrapFrmGetObjectIndexFromPtr					0xA454

// 11/10/00 acs
#define sysTrapBmpGetSizes								0xA455

#define sysTrapWinGetBounds								0xA456

#define sysTrapBltPaintPixels							0xA457

// 11/22/00 bob
#define sysTrapFldSetMaxVisibleLines					0xA458

// 01/09/01 acs
#define sysTrapScrDefaultPaletteState					0xA459

// ======================================================================
// Palm OS 5.0 Traps
// No new traps were added for 4.1, though 4.1 SC (see below) added some.
// ======================================================================

// 11/16/01 bob
#define sysTrapPceNativeCall							0xA45A

// ======================================================================
// Palm OS 5.1 Traps
// ======================================================================

// 12/04/01 lrt
#define sysTrapSndStreamCreate							0xA45B
#define sysTrapSndStreamDelete							0xA45C
#define sysTrapSndStreamStart							0xA45D
#define sysTrapSndStreamPause							0xA45E
#define sysTrapSndStreamStop							0xA45F
#define sysTrapSndStreamSetVolume						0xA460
#define sysTrapSndStreamGetVolume						0xA461
#define sysTrapSndPlayResource							0xA462
#define sysTrapSndStreamSetPan							0xA463
#define sysTrapSndStreamGetPan							0xA464

// 04/12/02 jed
#define sysTrapMultimediaDispatch						0xA465

// TRAPS ABOVE THIS POINT CAN NOT CHANGE BECAUSE THEY HAVE
// BEEN RELEASED TO CUSTOMERS IN SHIPPING ROMS AND SDKS.
// (MOVE THIS COMMENT DOWN WHENEVER THE "NEXT" RELEASE OCCURS.)

// ======================================================================
// Palm OS 5.1.1 Traps
// ======================================================================

// 08/02/02 mne
#define sysTrapSndStreamCreateExtended					0xa466
#define sysTrapSndStreamDeviceControl					0xa467

// ======================================================================
// Palm OS 4.2SC (Simplified Chinese) Traps
// These were added to an older 68K-based version of Palm OS to support
// QVGA displays.
// ======================================================================

// 09/23/02 acs & bob
#define sysTrapBmpCreateVersion3						0xA468
#define sysTrapECFixedMul								0xA469
#define sysTrapECFixedDiv								0xA46A
#define sysTrapHALDrawGetSupportedDensity				0xA46B
#define sysTrapHALRedrawInputArea						0xA46C
#define sysTrapGrfBeginStroke							0xA46D
#define sysTrapBmpPrvConvertBitmap						0xA46E

// ======================================================================
// Palm OS 5.x Traps
// These were added for new features or extensions for 5.x
// ======================================================================
#define sysTrapSysReservedTrap5		 			        0xA46F


// 12/11/02 grant
#define sysTrapPinsDispatch								0xA470

// ======================================================================
// Palm OS 5.3 Traps
// These were added for new features or extensions for 5.2. Currently
// they aren't implemented by any version of Palm OS released by
// PalmSource, but are reserved for future implementation.
// ======================================================================
#define sysTrapSysReservedTrap1		 			0xA471
#define sysTrapSysReservedTrap2					0xA472
#define sysTrapSysReservedTrap3					0xA473
#define sysTrapSysReservedTrap4					0xA474

#define sysTrapPumpkinDebug					    0xA473
#define sysTrapPumpkinDebugBytes				0xA474

// WARNING!! LEAVE THIS AT THE END AND ALWAYS ADD NEW TRAPS TO
// THE END OF THE TRAP TABLE BUT RIGHT BEFORE THIS TRAP, AND THEN
// RENUMBER THIS ONE TO ONE MORE THAN THE ONE RIGHT BEFORE IT!!!!!!!!!

#define sysTrapLastTrapNumber							0xA475



#define	sysNumTraps	 (sysTrapLastTrapNumber - sysTrapBase)

	

#endif  //__CORETRAPS_H_
