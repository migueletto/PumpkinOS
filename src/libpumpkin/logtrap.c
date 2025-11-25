#ifdef LOGTRAP_SYS
#include "sys.h"

#define memset    sys_memset
#define memcpy    sys_memcpy
#define qsort     sys_qsort
#define vsnprintf sys_vsnprintf
#define snprintf  sys_snprintf
#define sprintf   sys_sprintf
#define strcmp    sys_strcmp
#define strcpy    sys_strcpy
#define strlen    sys_strlen
#define strncat   sys_strncat
#define strcat    sys_strcat
#define strdup    sys_strdup
#define va_list   sys_va_list
#define va_start  sys_va_start
#define va_end    sys_va_end

#else

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#endif

#include "logtrap.h"

#define sysTrapMemInit                  0xA000
#define sysTrapMemInitHeapTable              0xA001
#define sysTrapMemStoreInit                0xA002
#define sysTrapMemCardFormat              0xA003
#define sysTrapMemCardInfo                0xA004
#define sysTrapMemStoreInfo                0xA005
#define sysTrapMemStoreSetInfo              0xA006
#define sysTrapMemNumHeaps                0xA007
#define sysTrapMemNumRAMHeaps              0xA008
#define sysTrapMemHeapID                0xA009
#define sysTrapMemHeapPtr                0xA00A
#define sysTrapMemHeapFreeBytes              0xA00B
#define sysTrapMemHeapSize                0xA00C
#define sysTrapMemHeapFlags                0xA00D
#define sysTrapMemHeapCompact              0xA00E
#define sysTrapMemHeapInit                0xA00F
#define sysTrapMemHeapFreeByOwnerID            0xA010
#define sysTrapMemChunkNew                0xA011
#define sysTrapMemChunkFree                0xA012
#define sysTrapMemPtrNew                0xA013
#define sysTrapMemPtrRecoverHandle            0xA014
#define sysTrapMemPtrFlags                0xA015
#define sysTrapMemPtrSize                0xA016
#define sysTrapMemPtrOwner                0xA017
#define sysTrapMemPtrHeapID                0xA018
#define sysTrapMemPtrCardNo                0xA019
#define sysTrapMemPtrToLocalID              0xA01A
#define sysTrapMemPtrSetOwner              0xA01B
#define sysTrapMemPtrResize                0xA01C
#define sysTrapMemPtrResetLock              0xA01D
#define sysTrapMemHandleNew                0xA01E
#define sysTrapMemHandleLockCount            0xA01F
#define sysTrapMemHandleToLocalID            0xA020
#define sysTrapMemHandleLock              0xA021
#define sysTrapMemHandleUnlock              0xA022
#define sysTrapMemLocalIDToGlobal            0xA023
#define sysTrapMemLocalIDKind              0xA024
#define sysTrapMemLocalIDToPtr              0xA025
#define sysTrapMemMove                  0xA026
#define sysTrapMemSet                  0xA027
#define sysTrapMemStoreSearch              0xA028
#define sysTrapSysReserved10Trap1            0xA029  /* "Reserved" trap in Palm OS 1.0 and later (was sysTrapMemPtrDataStorage) */

#define sysTrapMemKernelInit              0xA02A
#define sysTrapMemHandleFree              0xA02B
#define sysTrapMemHandleFlags              0xA02C
#define sysTrapMemHandleSize              0xA02D
#define sysTrapMemHandleOwner              0xA02E
#define sysTrapMemHandleHeapID              0xA02F
#define sysTrapMemHandleDataStorage            0xA030
#define sysTrapMemHandleCardNo              0xA031
#define sysTrapMemHandleSetOwner            0xA032
#define sysTrapMemHandleResize              0xA033
#define sysTrapMemHandleResetLock            0xA034
#define sysTrapMemPtrUnlock                0xA035
#define sysTrapMemLocalIDToLockedPtr          0xA036
#define sysTrapMemSetDebugMode              0xA037
#define sysTrapMemHeapScramble              0xA038
#define sysTrapMemHeapCheck                0xA039
#define sysTrapMemNumCards                0xA03A
#define sysTrapMemDebugMode                0xA03B
#define sysTrapMemSemaphoreReserve            0xA03C
#define sysTrapMemSemaphoreRelease            0xA03D
#define sysTrapMemHeapDynamic              0xA03E
#define sysTrapMemNVParams                0xA03F


#define sysTrapDmInit                  0xA040
#define sysTrapDmCreateDatabase              0xA041
#define sysTrapDmDeleteDatabase              0xA042
#define sysTrapDmNumDatabases              0xA043
#define sysTrapDmGetDatabase              0xA044
#define sysTrapDmFindDatabase              0xA045
#define sysTrapDmDatabaseInfo              0xA046
#define sysTrapDmSetDatabaseInfo            0xA047
#define sysTrapDmDatabaseSize              0xA048
#define sysTrapDmOpenDatabase              0xA049
#define sysTrapDmCloseDatabase              0xA04A
#define sysTrapDmNextOpenDatabase            0xA04B
#define sysTrapDmOpenDatabaseInfo            0xA04C
#define sysTrapDmResetRecordStates            0xA04D
#define sysTrapDmGetLastErr                0xA04E
#define sysTrapDmNumRecords                0xA04F
#define sysTrapDmRecordInfo                0xA050
#define sysTrapDmSetRecordInfo              0xA051
#define sysTrapDmAttachRecord              0xA052
#define sysTrapDmDetachRecord              0xA053
#define sysTrapDmMoveRecord                0xA054
#define sysTrapDmNewRecord                0xA055
#define sysTrapDmRemoveRecord              0xA056
#define sysTrapDmDeleteRecord              0xA057
#define sysTrapDmArchiveRecord              0xA058
#define sysTrapDmNewHandle                0xA059
#define sysTrapDmRemoveSecretRecords          0xA05A
#define sysTrapDmQueryRecord              0xA05B
#define sysTrapDmGetRecord                0xA05C
#define sysTrapDmResizeRecord              0xA05D
#define sysTrapDmReleaseRecord              0xA05E
#define sysTrapDmGetResource              0xA05F
#define sysTrapDmGet1Resource              0xA060
#define sysTrapDmReleaseResource            0xA061
#define sysTrapDmResizeResource              0xA062
#define sysTrapDmNextOpenResDatabase          0xA063
#define sysTrapDmFindResourceType            0xA064
#define sysTrapDmFindResource              0xA065
#define sysTrapDmSearchResource              0xA066
#define sysTrapDmNumResources              0xA067
#define sysTrapDmResourceInfo              0xA068
#define sysTrapDmSetResourceInfo            0xA069
#define sysTrapDmAttachResource              0xA06A
#define sysTrapDmDetachResource              0xA06B
#define sysTrapDmNewResource              0xA06C
#define sysTrapDmRemoveResource              0xA06D
#define sysTrapDmGetResourceIndex            0xA06E
#define sysTrapDmQuickSort                0xA06F
#define sysTrapDmQueryNextInCategory          0xA070
#define sysTrapDmNumRecordsInCategory          0xA071
#define sysTrapDmPositionInCategory            0xA072
#define sysTrapDmSeekRecordInCategory          0xA073
#define sysTrapDmMoveCategory              0xA074
#define sysTrapDmOpenDatabaseByTypeCreator        0xA075
#define sysTrapDmWrite                  0xA076
#define sysTrapDmStrCopy                0xA077
#define sysTrapDmGetNextDatabaseByTypeCreator      0xA078
#define sysTrapDmWriteCheck                0xA079
#define sysTrapDmMoveOpenDBContext            0xA07A
#define sysTrapDmFindRecordByID              0xA07B
#define sysTrapDmGetAppInfoID              0xA07C
#define sysTrapDmFindSortPositionV10          0xA07D
#define sysTrapDmSet                  0xA07E
#define sysTrapDmCreateDatabaseFromImage        0xA07F

#define sysTrapDbgSrcMessage              0xA080
#define sysTrapDbgMessage                0xA081
#define sysTrapDbgGetMessage              0xA082
#define sysTrapDbgCommSettings              0xA083

#define sysTrapErrDisplayFileLineMsg          0xA084
#define sysTrapErrSetJump                0xA085
#define sysTrapErrLongJump                0xA086
#define sysTrapErrThrow                  0xA087
#define sysTrapErrExceptionList              0xA088

#define sysTrapSysBroadcastActionCode          0xA089
#define sysTrapSysUnimplemented              0xA08A
#define sysTrapSysColdBoot                0xA08B
#define sysTrapSysReset                  0xA08C
#define sysTrapSysDoze                  0xA08D
#define sysTrapSysAppLaunch                0xA08E
#define sysTrapSysAppStartup              0xA08F
#define sysTrapSysAppExit                0xA090
#define sysTrapSysSetA5                  0xA091
#define sysTrapSysSetTrapAddress            0xA092
#define sysTrapSysGetTrapAddress            0xA093
#define sysTrapSysTranslateKernelErr          0xA094
#define sysTrapSysSemaphoreCreate            0xA095
#define sysTrapSysSemaphoreDelete            0xA096
#define sysTrapSysSemaphoreWait              0xA097
#define sysTrapSysSemaphoreSignal            0xA098
#define sysTrapSysTimerCreate              0xA099
#define sysTrapSysTimerWrite              0xA09A
#define sysTrapSysTaskCreate              0xA09B
#define sysTrapSysTaskDelete              0xA09C
#define sysTrapSysTaskTrigger              0xA09D
#define sysTrapSysTaskID                0xA09E
#define sysTrapSysTaskUserInfoPtr            0xA09F
#define sysTrapSysTaskDelay                0xA0A0
#define sysTrapSysTaskSetTermProc            0xA0A1
#define sysTrapSysUILaunch                0xA0A2
#define sysTrapSysNewOwnerID              0xA0A3
#define sysTrapSysSemaphoreSet              0xA0A4
#define sysTrapSysDisableInts              0xA0A5
#define sysTrapSysRestoreStatus              0xA0A6
#define sysTrapSysUIAppSwitch              0xA0A7
#define sysTrapSysCurAppInfoPV20            0xA0A8
#define sysTrapSysHandleEvent              0xA0A9
#define sysTrapSysInit                  0xA0AA
#define sysTrapSysQSort                  0xA0AB
#define sysTrapSysCurAppDatabase            0xA0AC
#define sysTrapSysFatalAlert              0xA0AD
#define sysTrapSysResSemaphoreCreate          0xA0AE
#define sysTrapSysResSemaphoreDelete          0xA0AF
#define sysTrapSysResSemaphoreReserve          0xA0B0
#define sysTrapSysResSemaphoreRelease          0xA0B1
#define sysTrapSysSleep                  0xA0B2
#define sysTrapSysKeyboardDialogV10            0xA0B3
#define sysTrapSysAppLauncherDialog            0xA0B4
#define sysTrapSysSetPerformance            0xA0B5
#define sysTrapSysBatteryInfoV20            0xA0B6
#define sysTrapSysLibInstall              0xA0B7
#define sysTrapSysLibRemove                0xA0B8
#define sysTrapSysLibTblEntry              0xA0B9
#define sysTrapSysLibFind                0xA0BA
#define sysTrapSysBatteryDialog              0xA0BB
#define sysTrapSysCopyStringResource          0xA0BC
#define sysTrapSysKernelInfo              0xA0BD
#define sysTrapSysLaunchConsole              0xA0BE
#define sysTrapSysTimerDelete              0xA0BF
#define sysTrapSysSetAutoOffTime            0xA0C0
#define sysTrapSysFormPointerArrayToStrings        0xA0C1
#define sysTrapSysRandom                0xA0C2
#define sysTrapSysTaskSwitching              0xA0C3
#define sysTrapSysTimerRead                0xA0C4

#define sysTrapStrCopy                  0xA0C5
#define sysTrapStrCat                  0xA0C6
#define sysTrapStrLen                  0xA0C7
#define sysTrapStrCompare                0xA0C8
#define sysTrapStrIToA                  0xA0C9
#define sysTrapStrCaselessCompare            0xA0CA
#define sysTrapStrIToH                  0xA0CB
#define sysTrapStrChr                  0xA0CC
#define sysTrapStrStr                  0xA0CD
#define sysTrapStrAToI                  0xA0CE
#define sysTrapStrToLower                0xA0CF

#define sysTrapSerReceiveISP              0xA0D0

#define sysTrapSlkOpen                  0xA0D1
#define sysTrapSlkClose                  0xA0D2
#define sysTrapSlkOpenSocket              0xA0D3
#define sysTrapSlkCloseSocket              0xA0D4
#define sysTrapSlkSocketRefNum              0xA0D5
#define sysTrapSlkSocketSetTimeout            0xA0D6
#define sysTrapSlkFlushSocket              0xA0D7
#define sysTrapSlkSetSocketListener            0xA0D8
#define sysTrapSlkSendPacket              0xA0D9
#define sysTrapSlkReceivePacket              0xA0DA
#define sysTrapSlkSysPktDefaultResponse          0xA0DB
#define sysTrapSlkProcessRPC              0xA0DC

#define sysTrapConPutS                  0xA0DD
#define sysTrapConGetS                  0xA0DE

#define sysTrapFplInit                  0xA0DF  /* Obsolete, here for compatibilty only! */
#define sysTrapFplFree                  0xA0E0  /* Obsolete, here for compatibilty only! */
#define sysTrapFplFToA                  0xA0E1  /* Obsolete, here for compatibilty only! */
#define sysTrapFplAToF                  0xA0E2  /* Obsolete, here for compatibilty only! */
#define sysTrapFplBase10Info              0xA0E3  /* Obsolete, here for compatibilty only! */
#define sysTrapFplLongToFloat              0xA0E4  /* Obsolete, here for compatibilty only! */
#define sysTrapFplFloatToLong              0xA0E5  /* Obsolete, here for compatibilty only! */
#define sysTrapFplFloatToULong              0xA0E6  /* Obsolete, here for compatibilty only! */
#define sysTrapFplMul                  0xA0E7  /* Obsolete, here for compatibilty only! */
#define sysTrapFplAdd                  0xA0E8  /* Obsolete, here for compatibilty only! */
#define sysTrapFplSub                  0xA0E9  /* Obsolete, here for compatibilty only! */
#define sysTrapFplDiv                  0xA0EA  /* Obsolete, here for compatibilty only! */

#define sysTrapWinScreenInit              0xA0EB  /* was sysTrapScrInit */
#define sysTrapScrCopyRectangle              0xA0EC
#define sysTrapScrDrawChars                0xA0ED
#define sysTrapScrLineRoutine              0xA0EE
#define sysTrapScrRectangleRoutine            0xA0EF
#define sysTrapScrScreenInfo              0xA0F0
#define sysTrapScrDrawNotify              0xA0F1
#define sysTrapScrSendUpdateArea            0xA0F2
#define sysTrapScrCompressScanLine            0xA0F3
#define sysTrapScrDeCompressScanLine          0xA0F4

#define sysTrapTimGetSeconds              0xA0F5
#define sysTrapTimSetSeconds              0xA0F6
#define sysTrapTimGetTicks                0xA0F7
#define sysTrapTimInit                  0xA0F8
#define sysTrapTimSetAlarm                0xA0F9
#define sysTrapTimGetAlarm                0xA0FA
#define sysTrapTimHandleInterrupt            0xA0FB
#define sysTrapTimSecondsToDateTime            0xA0FC
#define sysTrapTimDateTimeToSeconds            0xA0FD
#define sysTrapTimAdjust                0xA0FE
#define sysTrapTimSleep                  0xA0FF
#define sysTrapTimWake                  0xA100

#define sysTrapCategoryCreateListV10          0xA101
#define sysTrapCategoryFreeListV10            0xA102
#define sysTrapCategoryFind                0xA103
#define sysTrapCategoryGetName              0xA104
#define sysTrapCategoryEditV10              0xA105
#define sysTrapCategorySelectV10            0xA106
#define sysTrapCategoryGetNext              0xA107
#define sysTrapCategorySetTriggerLabel          0xA108
#define sysTrapCategoryTruncateName            0xA109

#define sysTrapClipboardAddItem              0xA10A
#define sysTrapClipboardCheckIfItemExist        0xA10B
#define sysTrapClipboardGetItem              0xA10C

#define sysTrapCtlDrawControl              0xA10D
#define sysTrapCtlEraseControl              0xA10E
#define sysTrapCtlHideControl              0xA10F
#define sysTrapCtlShowControl              0xA110
#define sysTrapCtlGetValue                0xA111
#define sysTrapCtlSetValue                0xA112
#define sysTrapCtlGetLabel                0xA113
#define sysTrapCtlSetLabel                0xA114
#define sysTrapCtlHandleEvent              0xA115
#define sysTrapCtlHitControl              0xA116
#define sysTrapCtlSetEnabled              0xA117
#define sysTrapCtlSetUsable                0xA118
#define sysTrapCtlEnabled                0xA119

#define sysTrapEvtInitialize              0xA11A
#define sysTrapEvtAddEventToQueue            0xA11B
#define sysTrapEvtCopyEvent                0xA11C
#define sysTrapEvtGetEvent                0xA11D
#define sysTrapEvtGetPen                0xA11E
#define sysTrapEvtSysInit                0xA11F
#define sysTrapEvtGetSysEvent              0xA120
#define sysTrapEvtProcessSoftKeyStroke          0xA121
#define sysTrapEvtGetPenBtnList              0xA122
#define sysTrapEvtSetPenQueuePtr            0xA123
#define sysTrapEvtPenQueueSize              0xA124
#define sysTrapEvtFlushPenQueue              0xA125
#define sysTrapEvtEnqueuePenPoint            0xA126
#define sysTrapEvtDequeuePenStrokeInfo          0xA127
#define sysTrapEvtDequeuePenPoint            0xA128
#define sysTrapEvtFlushNextPenStroke          0xA129
#define sysTrapEvtSetKeyQueuePtr            0xA12A
#define sysTrapEvtKeyQueueSize              0xA12B
#define sysTrapEvtFlushKeyQueue              0xA12C
#define sysTrapEvtEnqueueKey              0xA12D
#define sysTrapEvtDequeueKeyEvent            0xA12E
#define sysTrapEvtWakeup                0xA12F
#define sysTrapEvtResetAutoOffTimer            0xA130
#define sysTrapEvtKeyQueueEmpty              0xA131
#define sysTrapEvtEnableGraffiti            0xA132

#define sysTrapFldCopy                  0xA133
#define sysTrapFldCut                  0xA134
#define sysTrapFldDrawField                0xA135
#define sysTrapFldEraseField              0xA136
#define sysTrapFldFreeMemory              0xA137
#define sysTrapFldGetBounds                0xA138
#define sysTrapFldGetTextPtr              0xA139
#define sysTrapFldGetSelection              0xA13A
#define sysTrapFldHandleEvent              0xA13B
#define sysTrapFldPaste                  0xA13C
#define sysTrapFldRecalculateField            0xA13D
#define sysTrapFldSetBounds                0xA13E
#define sysTrapFldSetText                0xA13F
#define sysTrapFldGetFont                0xA140
#define sysTrapFldSetFont                0xA141
#define sysTrapFldSetSelection              0xA142
#define sysTrapFldGrabFocus                0xA143
#define sysTrapFldReleaseFocus              0xA144
#define sysTrapFldGetInsPtPosition            0xA145
#define sysTrapFldSetInsPtPosition            0xA146
#define sysTrapFldSetScrollPosition            0xA147
#define sysTrapFldGetScrollPosition            0xA148
#define sysTrapFldGetTextHeight              0xA149
#define sysTrapFldGetTextAllocatedSize          0xA14A
#define sysTrapFldGetTextLength              0xA14B
#define sysTrapFldScrollField              0xA14C
#define sysTrapFldScrollable              0xA14D
#define sysTrapFldGetVisibleLines            0xA14E
#define sysTrapFldGetAttributes              0xA14F
#define sysTrapFldSetAttributes              0xA150
#define sysTrapFldSendChangeNotification        0xA151
#define sysTrapFldCalcFieldHeight            0xA152
#define sysTrapFldGetTextHandle              0xA153
#define sysTrapFldCompactText              0xA154
#define sysTrapFldDirty                  0xA155
#define sysTrapFldWordWrap                0xA156
#define sysTrapFldSetTextAllocatedSize          0xA157
#define sysTrapFldSetTextHandle              0xA158
#define sysTrapFldSetTextPtr              0xA159
#define sysTrapFldGetMaxChars              0xA15A
#define sysTrapFldSetMaxChars              0xA15B
#define sysTrapFldSetUsable                0xA15C
#define sysTrapFldInsert                0xA15D
#define sysTrapFldDelete                0xA15E
#define sysTrapFldUndo                  0xA15F
#define sysTrapFldSetDirty                0xA160
#define sysTrapFldSendHeightChangeNotification   0xA161
#define sysTrapFldMakeFullyVisible               0xA162

#define sysTrapFntGetFont                        0xA163
#define sysTrapFntSetFont                        0xA164
#define sysTrapFntGetFontPtr                     0xA165
#define sysTrapFntBaseLine                       0xA166
#define sysTrapFntCharHeight                     0xA167
#define sysTrapFntLineHeight                     0xA168
#define sysTrapFntAverageCharWidth               0xA169
#define sysTrapFntCharWidth                      0xA16A
#define sysTrapFntCharsWidth                     0xA16B
#define sysTrapFntDescenderHeight                0xA16C
#define sysTrapFntCharsInWidth                   0xA16D
#define sysTrapFntLineWidth                      0xA16E

#define sysTrapFrmInitForm                       0xA16F
#define sysTrapFrmDeleteForm                     0xA170
#define sysTrapFrmDrawForm                       0xA171
#define sysTrapFrmEraseForm                      0xA172
#define sysTrapFrmGetActiveForm                  0xA173
#define sysTrapFrmSetActiveForm                  0xA174
#define sysTrapFrmGetActiveFormID                0xA175
#define sysTrapFrmGetUserModifiedState           0xA176
#define sysTrapFrmSetNotUserModified             0xA177
#define sysTrapFrmGetFocus                       0xA178
#define sysTrapFrmSetFocus                       0xA179
#define sysTrapFrmHandleEvent                    0xA17A
#define sysTrapFrmGetFormBounds                  0xA17B
#define sysTrapFrmGetWindowHandle                0xA17C
#define sysTrapFrmGetFormId                      0xA17D
#define sysTrapFrmGetFormPtr                     0xA17E
#define sysTrapFrmGetNumberOfObjects             0xA17F
#define sysTrapFrmGetObjectIndex                 0xA180
#define sysTrapFrmGetObjectId                    0xA181
#define sysTrapFrmGetObjectType                  0xA182
#define sysTrapFrmGetObjectPtr                   0xA183
#define sysTrapFrmHideObject                     0xA184
#define sysTrapFrmShowObject                     0xA185
#define sysTrapFrmGetObjectPosition              0xA186
#define sysTrapFrmSetObjectPosition              0xA187
#define sysTrapFrmGetControlValue                0xA188
#define sysTrapFrmSetControlValue                0xA189
#define sysTrapFrmGetControlGroupSelection       0xA18A
#define sysTrapFrmSetControlGroupSelection       0xA18B
#define sysTrapFrmCopyLabel                      0xA18C
#define sysTrapFrmSetLabel                       0xA18D
#define sysTrapFrmGetLabel                       0xA18E
#define sysTrapFrmSetCategoryLabel               0xA18F
#define sysTrapFrmGetTitle                       0xA190
#define sysTrapFrmSetTitle                       0xA191
#define sysTrapFrmAlert                          0xA192
#define sysTrapFrmDoDialog                       0xA193
#define sysTrapFrmCustomAlert                    0xA194
#define sysTrapFrmHelp                           0xA195
#define sysTrapFrmUpdateScrollers                0xA196
#define sysTrapFrmGetFirstForm                   0xA197
#define sysTrapFrmVisible                        0xA198
#define sysTrapFrmGetObjectBounds                0xA199
#define sysTrapFrmCopyTitle                      0xA19A
#define sysTrapFrmGotoForm                       0xA19B
#define sysTrapFrmPopupForm                      0xA19C
#define sysTrapFrmUpdateForm                     0xA19D
#define sysTrapFrmReturnToForm                   0xA19E
#define sysTrapFrmSetEventHandler                0xA19F
#define sysTrapFrmDispatchEvent                  0xA1A0
#define sysTrapFrmCloseAllForms                  0xA1A1
#define sysTrapFrmSaveAllForms                   0xA1A2
#define sysTrapFrmGetGadgetData                  0xA1A3
#define sysTrapFrmSetGadgetData                  0xA1A4
#define sysTrapFrmSetCategoryTrigger             0xA1A5

#define sysTrapUIInitialize                      0xA1A6
#define sysTrapUIReset                           0xA1A7

#define sysTrapInsPtInitialize                   0xA1A8
#define sysTrapInsPtSetLocation                  0xA1A9
#define sysTrapInsPtGetLocation                  0xA1AA
#define sysTrapInsPtEnable                       0xA1AB
#define sysTrapInsPtEnabled                      0xA1AC
#define sysTrapInsPtSetHeight                    0xA1AD
#define sysTrapInsPtGetHeight                    0xA1AE
#define sysTrapInsPtCheckBlink                   0xA1AF

#define sysTrapLstSetDrawFunction                0xA1B0
#define sysTrapLstDrawList                       0xA1B1
#define sysTrapLstEraseList                      0xA1B2
#define sysTrapLstGetSelection                   0xA1B3
#define sysTrapLstGetSelectionText               0xA1B4
#define sysTrapLstHandleEvent                    0xA1B5
#define sysTrapLstSetHeight                      0xA1B6
#define sysTrapLstSetSelection                   0xA1B7
#define sysTrapLstSetListChoices                 0xA1B8
#define sysTrapLstMakeItemVisible                0xA1B9
#define sysTrapLstGetNumberOfItems               0xA1BA
#define sysTrapLstPopupList                      0xA1BB
#define sysTrapLstSetPosition                    0xA1BC
 
#define sysTrapMenuInit                          0xA1BD
#define sysTrapMenuDispose                       0xA1BE
#define sysTrapMenuHandleEvent                   0xA1BF
#define sysTrapMenuDrawMenu                      0xA1C0
#define sysTrapMenuEraseStatus                   0xA1C1
#define sysTrapMenuGetActiveMenu                 0xA1C2
#define sysTrapMenuSetActiveMenu                 0xA1C3

#define sysTrapRctSetRectangle                   0xA1C4
#define sysTrapRctCopyRectangle                  0xA1C5
#define sysTrapRctInsetRectangle                 0xA1C6
#define sysTrapRctOffsetRectangle                0xA1C7
#define sysTrapRctPtInRectangle                  0xA1C8
#define sysTrapRctGetIntersection                0xA1C9

#define sysTrapTblDrawTable                      0xA1CA
#define sysTrapTblEraseTable                     0xA1CB
#define sysTrapTblHandleEvent                    0xA1CC
#define sysTrapTblGetItemBounds                  0xA1CD
#define sysTrapTblSelectItem                     0xA1CE
#define sysTrapTblGetItemInt                     0xA1CF
#define sysTrapTblSetItemInt                     0xA1D0
#define sysTrapTblSetItemStyle                   0xA1D1
#define sysTrapTblUnhighlightSelection           0xA1D2
#define sysTrapTblSetRowUsable                   0xA1D3
#define sysTrapTblGetNumberOfRows                0xA1D4
#define sysTrapTblSetCustomDrawProcedure         0xA1D5
#define sysTrapTblSetRowSelectable               0xA1D6
#define sysTrapTblRowSelectable                  0xA1D7
#define sysTrapTblSetLoadDataProcedure           0xA1D8
#define sysTrapTblSetSaveDataProcedure           0xA1D9
#define sysTrapTblGetBounds                      0xA1DA
#define sysTrapTblSetRowHeight                   0xA1DB
#define sysTrapTblGetColumnWidth                 0xA1DC
#define sysTrapTblGetRowID                       0xA1DD
#define sysTrapTblSetRowID                       0xA1DE
#define sysTrapTblMarkRowInvalid                 0xA1DF
#define sysTrapTblMarkTableInvalid               0xA1E0
#define sysTrapTblGetSelection                   0xA1E1
#define sysTrapTblInsertRow                      0xA1E2
#define sysTrapTblRemoveRow                      0xA1E3
#define sysTrapTblRowInvalid                     0xA1E4
#define sysTrapTblRedrawTable                    0xA1E5
#define sysTrapTblRowUsable                      0xA1E6
#define sysTrapTblReleaseFocus                   0xA1E7
#define sysTrapTblEditing                        0xA1E8
#define sysTrapTblGetCurrentField                0xA1E9
#define sysTrapTblSetColumnUsable                0xA1EA
#define sysTrapTblGetRowHeight                   0xA1EB
#define sysTrapTblSetColumnWidth                 0xA1EC
#define sysTrapTblGrabFocus                      0xA1ED
#define sysTrapTblSetItemPtr                     0xA1EE
#define sysTrapTblFindRowID                      0xA1EF
#define sysTrapTblGetLastUsableRow               0xA1F0
#define sysTrapTblGetColumnSpacing               0xA1F1
#define sysTrapTblFindRowData                    0xA1F2
#define sysTrapTblGetRowData                     0xA1F3
#define sysTrapTblSetRowData                     0xA1F4
#define sysTrapTblSetColumnSpacing               0xA1F5

#define sysTrapWinCreateWindow              0xA1F6
#define sysTrapWinCreateOffscreenWindow          0xA1F7
#define sysTrapWinDeleteWindow              0xA1F8
#define sysTrapWinInitializeWindow            0xA1F9
#define sysTrapWinAddWindow                0xA1FA
#define sysTrapWinRemoveWindow              0xA1FB
#define sysTrapWinSetActiveWindow            0xA1FC
#define sysTrapWinSetDrawWindow              0xA1FD
#define sysTrapWinGetDrawWindow              0xA1FE
#define sysTrapWinGetActiveWindow            0xA1FF
#define sysTrapWinGetDisplayWindow            0xA200
#define sysTrapWinGetFirstWindow            0xA201
#define sysTrapWinEnableWindow              0xA202
#define sysTrapWinDisableWindow              0xA203
#define sysTrapWinGetWindowFrameRect          0xA204
#define sysTrapWinDrawWindowFrame            0xA205
#define sysTrapWinEraseWindow              0xA206
#define sysTrapWinSaveBits                0xA207
#define sysTrapWinRestoreBits              0xA208
#define sysTrapWinCopyRectangle              0xA209
#define sysTrapWinScrollRectangle            0xA20A
#define sysTrapWinGetDisplayExtent            0xA20B
#define sysTrapWinGetWindowExtent            0xA20C
#define sysTrapWinDisplayToWindowPt            0xA20D
#define sysTrapWinWindowToDisplayPt            0xA20E
#define sysTrapWinGetClip                0xA20F
#define sysTrapWinSetClip                0xA210
#define sysTrapWinResetClip                0xA211
#define sysTrapWinClipRectangle              0xA212
#define sysTrapWinDrawLine                0xA213
#define sysTrapWinDrawGrayLine              0xA214
#define sysTrapWinEraseLine                0xA215
#define sysTrapWinInvertLine              0xA216
#define sysTrapWinFillLine                0xA217
#define sysTrapWinDrawRectangle              0xA218
#define sysTrapWinEraseRectangle            0xA219
#define sysTrapWinInvertRectangle            0xA21A
#define sysTrapWinDrawRectangleFrame          0xA21B
#define sysTrapWinDrawGrayRectangleFrame        0xA21C
#define sysTrapWinEraseRectangleFrame          0xA21D
#define sysTrapWinInvertRectangleFrame          0xA21E
#define sysTrapWinGetFramesRectangle          0xA21F
#define sysTrapWinDrawChars                0xA220
#define sysTrapWinEraseChars              0xA221
#define sysTrapWinInvertChars              0xA222
#define sysTrapWinGetPattern              0xA223
#define sysTrapWinSetPattern              0xA224
#define sysTrapWinSetUnderlineMode            0xA225
#define sysTrapWinDrawBitmap              0xA226
#define sysTrapWinModal                  0xA227
#define sysTrapWinGetDrawWindowBounds          0xA228
#define sysTrapWinFillRectangle              0xA229
#define sysTrapWinDrawInvertedChars            0xA22A

#define sysTrapPrefOpenPreferenceDBV10          0xA22B
#define sysTrapPrefGetPreferences            0xA22C
#define sysTrapPrefSetPreferences            0xA22D
#define sysTrapPrefGetAppPreferencesV10          0xA22E
#define sysTrapPrefSetAppPreferencesV10          0xA22F

#define sysTrapSndInit                  0xA230
#define sysTrapSndSetDefaultVolume            0xA231
#define sysTrapSndGetDefaultVolume            0xA232
#define sysTrapSndDoCmd                  0xA233
#define sysTrapSndPlaySystemSound            0xA234

#define sysTrapAlmInit                  0xA235
#define sysTrapAlmCancelAll                0xA236
#define sysTrapAlmAlarmCallback              0xA237
#define sysTrapAlmSetAlarm                0xA238
#define sysTrapAlmGetAlarm                0xA239
#define sysTrapAlmDisplayAlarm              0xA23A
#define sysTrapAlmEnableNotification          0xA23B

#define sysTrapHwrGetRAMMapping              0xA23C
#define sysTrapHwrMemWritable              0xA23D
#define sysTrapHwrMemReadable              0xA23E
#define sysTrapHwrDoze                  0xA23F
#define sysTrapHwrSleep                  0xA240
#define sysTrapHwrWake                  0xA241
#define sysTrapHwrSetSystemClock            0xA242
#define sysTrapHwrSetCPUDutyCycle            0xA243
#define sysTrapHwrDisplayInit              0xA244  /* Before OS 3.5, this trap a.k.a. sysTrapHwrLCDInit */
#define sysTrapHwrDisplaySleep              0xA245  /* Before OS 3.5, this trap a.k.a. sysTrapHwrLCDSleep, */
#define sysTrapHwrTimerInit                0xA246
#define sysTrapHwrCursorV33                0xA247  /* This trap obsoleted for OS 3.5 and later */
#define sysTrapHwrBatteryLevel              0xA248
#define sysTrapHwrDelay                  0xA249
#define sysTrapHwrEnableDataWrites            0xA24A
#define sysTrapHwrDisableDataWrites            0xA24B
#define sysTrapHwrLCDBaseAddrV33            0xA24C  /* This trap obsoleted for OS 3.5 and later */
#define sysTrapHwrDisplayDrawBootScreen          0xA24D  /* Before OS 3.5, this trap a.k.a. sysTrapHwrLCDDrawBitmap */
#define sysTrapHwrTimerSleep              0xA24E
#define sysTrapHwrTimerWake                0xA24F
#define sysTrapHwrDisplayWake              0xA250  /* Before OS 3.5, this trap a.k.a. sysTrapHwrLCDWake */
#define sysTrapHwrIRQ1Handler              0xA251
#define sysTrapHwrIRQ2Handler              0xA252
#define sysTrapHwrIRQ3Handler              0xA253
#define sysTrapHwrIRQ4Handler              0xA254
#define sysTrapHwrIRQ5Handler              0xA255
#define sysTrapHwrIRQ6Handler              0xA256
#define sysTrapHwrDockSignals              0xA257
#define sysTrapHwrPluggedIn                0xA258

#define sysTrapCrc16CalcBlock              0xA259

#define sysTrapSelectDayV10                0xA25A
#define sysTrapSelectTimeV33              0xA25B

#define sysTrapDayDrawDaySelector            0xA25C
#define sysTrapDayHandleEvent              0xA25D
#define sysTrapDayDrawDays                0xA25E
#define sysTrapDayOfWeek                0xA25F
#define sysTrapDaysInMonth                0xA260
#define sysTrapDayOfMonth                0xA261

#define sysTrapDateDaysToDate              0xA262
#define sysTrapDateToDays                0xA263
#define sysTrapDateAdjust                0xA264
#define sysTrapDateSecondsToDate            0xA265
#define sysTrapDateToAscii                0xA266
#define sysTrapDateToDOWDMFormat            0xA267
#define sysTrapTimeToAscii                0xA268

#define sysTrapFind                    0xA269
#define sysTrapFindStrInStr                0xA26A
#define sysTrapFindSaveMatch              0xA26B
#define sysTrapFindGetLineBounds            0xA26C
#define sysTrapFindDrawHeader              0xA26D

#define sysTrapPenOpen                  0xA26E
#define sysTrapPenClose                  0xA26F
#define sysTrapPenGetRawPen                0xA270
#define sysTrapPenCalibrate                0xA271
#define sysTrapPenRawToScreen              0xA272
#define sysTrapPenScreenToRaw              0xA273
#define sysTrapPenResetCalibration            0xA274
#define sysTrapPenSleep                  0xA275
#define sysTrapPenWake                  0xA276

#define sysTrapResLoadForm                0xA277
#define sysTrapResLoadMenu                0xA278

#define sysTrapFtrInit                  0xA279
#define sysTrapFtrUnregister              0xA27A
#define sysTrapFtrGet                  0xA27B
#define sysTrapFtrSet                  0xA27C
#define sysTrapFtrGetByIndex              0xA27D

#define sysTrapGrfInit                  0xA27E
#define sysTrapGrfFree                  0xA27F
#define sysTrapGrfGetState                0xA280
#define sysTrapGrfSetState                0xA281
#define sysTrapGrfFlushPoints              0xA282
#define sysTrapGrfAddPoint                0xA283
#define sysTrapGrfInitState                0xA284
#define sysTrapGrfCleanState              0xA285
#define sysTrapGrfMatch                  0xA286
#define sysTrapGrfGetMacro                0xA287
#define sysTrapGrfFilterPoints              0xA288
#define sysTrapGrfGetNumPoints              0xA289
#define sysTrapGrfGetPoint                0xA28A
#define sysTrapGrfFindBranch              0xA28B
#define sysTrapGrfMatchGlyph              0xA28C
#define sysTrapGrfGetGlyphMapping            0xA28D
#define sysTrapGrfGetMacroName              0xA28E
#define sysTrapGrfDeleteMacro              0xA28F
#define sysTrapGrfAddMacro                0xA290
#define sysTrapGrfGetAndExpandMacro            0xA291
#define sysTrapGrfProcessStroke              0xA292
#define sysTrapGrfFieldChange              0xA293

#define sysTrapGetCharSortValue              0xA294
#define sysTrapGetCharAttr                0xA295
#define sysTrapGetCharCaselessValue            0xA296

#define sysTrapPwdExists                0xA297
#define sysTrapPwdVerify                0xA298
#define sysTrapPwdSet                  0xA299
#define sysTrapPwdRemove                0xA29A

#define sysTrapGsiInitialize              0xA29B
#define sysTrapGsiSetLocation              0xA29C
#define sysTrapGsiEnable                0xA29D
#define sysTrapGsiEnabled                0xA29E
#define sysTrapGsiSetShiftState              0xA29F

#define sysTrapKeyInit                  0xA2A0
#define sysTrapKeyHandleInterrupt            0xA2A1
#define sysTrapKeyCurrentState              0xA2A2
#define sysTrapKeyResetDoubleTap            0xA2A3
#define sysTrapKeyRates                  0xA2A4
#define sysTrapKeySleep                  0xA2A5
#define sysTrapKeyWake                  0xA2A6

#define sysTrapDlkControl                0xA2A7  /* was sysTrapCmBroadcast */

#define sysTrapDlkStartServer              0xA2A8
#define sysTrapDlkGetSyncInfo              0xA2A9
#define sysTrapDlkSetLogEntry              0xA2AA

#define sysTrapIntlDispatch                0xA2AB  /* REUSED IN v3.1 (was sysTrapPsrInit in 1.0, removed in 2.0) */
#define sysTrapSysLibLoad                0xA2AC  /* REUSED IN v2.0 (was sysTrapPsrClose) */
#define sysTrapSndPlaySmf                0xA2AD  /* REUSED IN v3.0 (was sysTrapPsrGetCommand in 1.0, removed in 2.0) */
#define sysTrapSndCreateMidiList            0xA2AE  /* REUSED IN v3.0 (was sysTrapPsrSendReply in 1.0, removed in 2.0) */

#define sysTrapAbtShowAbout                0xA2AF

#define sysTrapMdmDial                  0xA2B0
#define sysTrapMdmHangUp                0xA2B1

#define sysTrapDmSearchRecord              0xA2B2

#define sysTrapSysInsertionSort              0xA2B3
#define sysTrapDmInsertionSort              0xA2B4

#define sysTrapLstSetTopItem              0xA2B5

// ======================================================================
// Palm OS 2.X traps          Palm Pilot and 2.0 Upgrade Card
// ======================================================================

#define sysTrapSclSetScrollBar              0xA2B6
#define sysTrapSclDrawScrollBar              0xA2B7
#define sysTrapSclHandleEvent              0xA2B8

#define sysTrapSysMailboxCreate              0xA2B9
#define sysTrapSysMailboxDelete              0xA2BA
#define sysTrapSysMailboxFlush              0xA2BB
#define sysTrapSysMailboxSend              0xA2BC
#define sysTrapSysMailboxWait              0xA2BD

#define sysTrapSysTaskWait                0xA2BE
#define sysTrapSysTaskWake                0xA2BF
#define sysTrapSysTaskWaitClr              0xA2C0
#define sysTrapSysTaskSuspend              0xA2C1
#define sysTrapSysTaskResume              0xA2C2

#define sysTrapCategoryCreateList            0xA2C3
#define sysTrapCategoryFreeList              0xA2C4
#define sysTrapCategoryEditV20              0xA2C5
#define sysTrapCategorySelect              0xA2C6

#define sysTrapDmDeleteCategory              0xA2C7

#define sysTrapSysEvGroupCreate              0xA2C8
#define sysTrapSysEvGroupSignal              0xA2C9
#define sysTrapSysEvGroupRead              0xA2CA
#define sysTrapSysEvGroupWait              0xA2CB

#define sysTrapEvtEventAvail              0xA2CC
#define sysTrapEvtSysEventAvail              0xA2CD
#define sysTrapStrNCopy                  0xA2CE

#define sysTrapKeySetMask                0xA2CF

#define sysTrapSelectDay                0xA2D0

#define sysTrapPrefGetPreference            0xA2D1
#define sysTrapPrefSetPreference            0xA2D2
#define sysTrapPrefGetAppPreferences          0xA2D3
#define sysTrapPrefSetAppPreferences          0xA2D4

#define sysTrapFrmPointInTitle              0xA2D5

#define sysTrapStrNCat                  0xA2D6

#define sysTrapMemCmp                  0xA2D7

#define sysTrapTblSetColumnEditIndicator        0xA2D8

#define sysTrapFntWordWrap                0xA2D9

#define sysTrapFldGetScrollValues            0xA2DA

#define sysTrapSysCreateDataBaseList          0xA2DB
#define sysTrapSysCreatePanelList            0xA2DC

#define sysTrapDlkDispatchRequest            0xA2DD

#define sysTrapStrPrintF                0xA2DE
#define sysTrapStrVPrintF                0xA2DF

#define sysTrapPrefOpenPreferenceDB            0xA2E0

#define sysTrapSysGraffitiReferenceDialog        0xA2E1

#define sysTrapSysKeyboardDialog            0xA2E2

#define sysTrapFntWordWrapReverseNLines          0xA2E3
#define sysTrapFntGetScrollValues            0xA2E4

#define sysTrapTblSetRowStaticHeight          0xA2E5
#define sysTrapTblHasScrollBar              0xA2E6

#define sysTrapSclGetScrollBar              0xA2E7

#define sysTrapFldGetNumberOfBlankLines          0xA2E8

#define sysTrapSysTicksPerSecond            0xA2E9
#define sysTrapHwrBacklightV33              0xA2EA  /* This trap obsoleted for OS 3.5 and later */
#define sysTrapDmDatabaseProtect            0xA2EB

#define sysTrapTblSetBounds                0xA2EC

#define sysTrapStrNCompare                0xA2ED
#define sysTrapStrNCaselessCompare            0xA2EE

#define sysTrapPhoneNumberLookup            0xA2EF

#define sysTrapFrmSetMenu                0xA2F0

#define sysTrapEncDigestMD5                0xA2F1

#define sysTrapDmFindSortPosition            0xA2F2

#define sysTrapSysBinarySearch              0xA2F3
#define sysTrapSysErrString                0xA2F4
#define sysTrapSysStringByIndex              0xA2F5

#define sysTrapEvtAddUniqueEventToQueue          0xA2F6

#define sysTrapStrLocalizeNumber            0xA2F7
#define sysTrapStrDelocalizeNumber            0xA2F8
#define sysTrapLocGetNumberSeparators          0xA2F9

#define sysTrapMenuSetActiveMenuRscID          0xA2FA

#define sysTrapLstScrollList              0xA2FB

#define sysTrapCategoryInitialize            0xA2FC

#define sysTrapEncDigestMD4                0xA2FD
#define sysTrapEncDES                  0xA2FE

#define sysTrapLstGetVisibleItems            0xA2FF

#define sysTrapWinSetBounds                0xA300

#define sysTrapCategorySetName              0xA301

#define sysTrapFldSetInsertionPoint            0xA302

#define sysTrapFrmSetObjectBounds            0xA303

#define sysTrapWinSetColors                0xA304

#define sysTrapFlpDispatch                0xA305
#define sysTrapFlpEmDispatch              0xA306

// ======================================================================
// Palm OS 3.0 traps          Palm III and 3.0 Upgrade Card
// ======================================================================

#define sysTrapExgInit                  0xA307
#define sysTrapExgConnect                0xA308
#define sysTrapExgPut                  0xA309
#define sysTrapExgGet                  0xA30A
#define sysTrapExgAccept                0xA30B
#define sysTrapExgDisconnect              0xA30C
#define sysTrapExgSend                  0xA30D
#define sysTrapExgReceive                0xA30E
#define sysTrapExgRegisterData              0xA30F
#define sysTrapExgNotifyReceiveV35            0xA310
#define sysTrapSysReserved30Trap2             0xA311  /* "Reserved" trap in Palm OS 3.0 and later (was sysTrapExgControl) */

#define sysTrapPrgStartDialogV31            0xA312  /* Updated in v3.2 */
#define sysTrapPrgStopDialog              0xA313
#define sysTrapPrgUpdateDialog              0xA314
#define sysTrapPrgHandleEvent              0xA315

#define sysTrapImcReadFieldNoSemicolon          0xA316
#define sysTrapImcReadFieldQuotablePrintable      0xA317
#define sysTrapImcReadPropertyParameter          0xA318
#define sysTrapImcSkipAllPropertyParameters        0xA319
#define sysTrapImcReadWhiteSpace            0xA31A
#define sysTrapImcWriteQuotedPrintable          0xA31B
#define sysTrapImcWriteNoSemicolon            0xA31C
#define sysTrapImcStringIsAscii              0xA31D

#define sysTrapTblGetItemFont              0xA31E
#define sysTrapTblSetItemFont              0xA31F

#define sysTrapFontSelect                0xA320
#define sysTrapFntDefineFont              0xA321

#define sysTrapCategoryEdit                0xA322

#define sysTrapSysGetOSVersionString          0xA323
#define sysTrapSysBatteryInfo              0xA324
#define sysTrapSysUIBusy                0xA325

#define sysTrapWinValidateHandle            0xA326
#define sysTrapFrmValidatePtr              0xA327
#define sysTrapCtlValidatePointer            0xA328
#define sysTrapWinMoveWindowAddr            0xA329
#define sysTrapFrmAddSpaceForObject            0xA32A
#define sysTrapFrmNewForm                0xA32B
#define sysTrapCtlNewControl              0xA32C
#define sysTrapFldNewField                0xA32D
#define sysTrapLstNewList                0xA32E
#define sysTrapFrmNewLabel                0xA32F
#define sysTrapFrmNewBitmap                0xA330
#define sysTrapFrmNewGadget                0xA331

#define sysTrapFileOpen                  0xA332
#define sysTrapFileClose                0xA333
#define sysTrapFileDelete                0xA334
#define sysTrapFileReadLow                0xA335
#define sysTrapFileWrite                0xA336
#define sysTrapFileSeek                  0xA337
#define sysTrapFileTell                  0xA338
#define sysTrapFileTruncate                0xA339
#define sysTrapFileControl                0xA33A

#define sysTrapFrmActiveState              0xA33B

#define sysTrapSysGetAppInfo              0xA33C
#define sysTrapSysGetStackInfo              0xA33D

#define sysTrapWinScreenMode              0xA33E  /* was sysTrapScrDisplayMode */
#define sysTrapHwrLCDGetDepthV33            0xA33F  /* This trap obsoleted for OS 3.5 and later */
#define sysTrapHwrGetROMToken              0xA340

#define sysTrapDbgControl                0xA341

#define sysTrapExgDBRead                0xA342
#define sysTrapExgDBWrite                0xA343

#define sysTrapHostControl                0xA344  /* Renamed from sysTrapSysGremlins, functionality generalized */
#define sysTrapFrmRemoveObject              0xA345

#define sysTrapSysReserved30Trap1            0xA346  /* "Reserved" trap in Palm OS 3.0 and later (was sysTrapSysReserved1) */

// NOTE: The following two traps are reserved for future mgrs
// that may or may not be present on any particular device.
// They are NOT present by default; code must check first!
#define sysTrapExpansionDispatch            0xA347  /* Reserved for ExpansionMgr (was sysTrapSysReserved2) */
#define sysTrapExpansionMgr sysTrapExpansionDispatch
#define sysTrapFileSystemDispatch            0xA348  /* Reserved for FileSystemMgr (was sysTrapSysReserved3) */
#define sysTrapVFSMgr   sysTrapFileSystemDispatch

#define sysTrapOEMDispatch                0xA349  /* OEM trap in Palm OS 3.0 and later trap table (formerly sysTrapSysReserved4) */

// ======================================================================
// Palm OS 3.1 traps          Palm IIIx and Palm V
// ======================================================================

#define sysTrapHwrLCDContrastV33            0xA34A  /* This trap obsoleted for OS 3.5 and later */
#define sysTrapSysLCDContrast              0xA34B
#define sysTrapUIContrastAdjust              0xA34C  /* Renamed from sysTrapContrastAdjust */
#define sysTrapHwrDockStatus              0xA34D

#define sysTrapFntWidthToOffset              0xA34E
#define sysTrapSelectOneTime              0xA34F
#define sysTrapWinDrawChar                0xA350
#define sysTrapWinDrawTruncChars            0xA351

#define sysTrapSysNotifyInit              0xA352  /* Notification Manager traps */
#define sysTrapSysNotifyRegister            0xA353
#define sysTrapSysNotifyUnregister            0xA354
#define sysTrapSysNotifyBroadcast            0xA355
#define sysTrapSysNotifyBroadcastDeferred        0xA356
#define sysTrapSysNotifyDatabaseAdded          0xA357
#define sysTrapSysNotifyDatabaseRemoved          0xA358

#define sysTrapSysWantEvent                0xA359

#define sysTrapFtrPtrNew                0xA35A
#define sysTrapFtrPtrFree                0xA35B
#define sysTrapFtrPtrResize                0xA35C

#define sysTrapSysReserved31Trap1            0xA35D  /* "Reserved" trap in Palm OS 3.1 and later (was sysTrapSysReserved5) */

// ======================================================================
// Palm OS 3.2 & 3.3 traps    Palm VII (3.2) and Fall '99 Palm OS Flash Update (3.3)
// ======================================================================

#define sysTrapHwrNVPrefSet                0xA35E  /* mapped to FlashParmsWrite */
#define sysTrapHwrNVPrefGet                0xA35F  /* mapped to FlashParmsRead */
#define sysTrapFlashInit                0xA360
#define sysTrapFlashCompress              0xA361
#define sysTrapFlashErase                0xA362
#define sysTrapFlashProgram                0xA363

#define sysTrapAlmTimeChange              0xA364
#define sysTrapErrAlertCustom              0xA365
#define sysTrapPrgStartDialog              0xA366  /* New version of sysTrapPrgStartDialogV31 */

#define sysTrapSerialDispatch              0xA367
#define sysTrapHwrBattery                0xA368
#define sysTrapDmGetDatabaseLockState          0xA369

#define sysTrapCncGetProfileList            0xA36A
#define sysTrapCncGetProfileInfo            0xA36B
#define sysTrapCncAddProfile              0xA36C
#define sysTrapCncDeleteProfile              0xA36D

#define sysTrapSndPlaySmfResource            0xA36E

#define sysTrapMemPtrDataStorage            0xA36F  /* Never actually installed until now. */

#define sysTrapClipboardAppendItem            0xA370

#define sysTrapWiCmdV32                  0xA371  /* Code moved to INetLib; trap obsolete */

// ======================================================================
// Palm OS 3.5 traps        Palm IIIc and other products
// ======================================================================

// HAL Display-layer new traps
#define sysTrapHwrDisplayAttributes            0xA372
#define sysTrapHwrDisplayDoze              0xA373
#define sysTrapHwrDisplayPalette            0xA374

// Screen driver new traps
#define sysTrapBltFindIndexes              0xA375
#define sysTrapBmpGetBits                0xA376  /* was BltGetBitsAddr */
#define sysTrapBltCopyRectangle              0xA377
#define sysTrapBltDrawChars                0xA378
#define sysTrapBltLineRoutine              0xA379
#define sysTrapBltRectangleRoutine            0xA37A

// ScrUtils new traps
#define sysTrapScrCompress                0xA37B
#define sysTrapScrDecompress              0xA37C

// System Manager new traps
#define sysTrapSysLCDBrightness              0xA37D

// WindowColor new traps
#define sysTrapWinPaintChar                0xA37E
#define sysTrapWinPaintChars              0xA37F
#define sysTrapWinPaintBitmap              0xA380
#define sysTrapWinGetPixel                0xA381
#define sysTrapWinPaintPixel              0xA382
#define sysTrapWinDrawPixel                0xA383
#define sysTrapWinErasePixel              0xA384
#define sysTrapWinInvertPixel              0xA385
#define sysTrapWinPaintPixels              0xA386
#define sysTrapWinPaintLines              0xA387
#define sysTrapWinPaintLine                0xA388
#define sysTrapWinPaintRectangle            0xA389
#define sysTrapWinPaintRectangleFrame          0xA38A
#define sysTrapWinPaintPolygon              0xA38B
#define sysTrapWinDrawPolygon              0xA38C
#define sysTrapWinErasePolygon              0xA38D
#define sysTrapWinInvertPolygon              0xA38E
#define sysTrapWinFillPolygon              0xA38F
#define sysTrapWinPaintArc                0xA390
#define sysTrapWinDrawArc                0xA391
#define sysTrapWinEraseArc                0xA392
#define sysTrapWinInvertArc                0xA393
#define sysTrapWinFillArc                0xA394
#define sysTrapWinPushDrawState              0xA395
#define sysTrapWinPopDrawState              0xA396
#define sysTrapWinSetDrawMode              0xA397
#define sysTrapWinSetForeColor              0xA398
#define sysTrapWinSetBackColor              0xA399
#define sysTrapWinSetTextColor              0xA39A
#define sysTrapWinGetPatternType            0xA39B
#define sysTrapWinSetPatternType            0xA39C
#define sysTrapWinPalette                0xA39D
#define sysTrapWinRGBToIndex              0xA39E
#define sysTrapWinIndexToRGB              0xA39F
#define sysTrapWinScreenLock              0xA3A0
#define sysTrapWinScreenUnlock              0xA3A1
#define sysTrapWinGetBitmap                0xA3A2

// UIColor new traps
#define sysTrapUIColorInit                0xA3A3
#define sysTrapUIColorGetTableEntryIndex        0xA3A4
#define sysTrapUIColorGetTableEntryRGB          0xA3A5
#define sysTrapUIColorSetTableEntry            0xA3A6
#define sysTrapUIColorPushTable              0xA3A7
#define sysTrapUIColorPopTable              0xA3A8

// misc cleanup and API additions

#define sysTrapCtlNewGraphicControl            0xA3A9

#define sysTrapTblGetItemPtr              0xA3AA

#define sysTrapUIBrightnessAdjust            0xA3AB
#define sysTrapUIPickColor                0xA3AC

#define sysTrapEvtSetAutoOffTimer            0xA3AD

// Misc int'l/overlay support.
#define sysTrapTsmDispatch                0xA3AE
#define sysTrapOmDispatch                0xA3AF
#define sysTrapDmOpenDBNoOverlay            0xA3B0
#define sysTrapDmOpenDBWithLocale            0xA3B1
#define sysTrapResLoadConstant              0xA3B2

// new boot-time SmallROM HAL additions
#define sysTrapHwrPreDebugInit              0xA3B3
#define sysTrapHwrResetNMI                0xA3B4
#define sysTrapHwrResetPWM                0xA3B5

#define sysTrapKeyBootKeys                0xA3B6

#define sysTrapDbgSerDrvOpen              0xA3B7
#define sysTrapDbgSerDrvClose              0xA3B8
#define sysTrapDbgSerDrvControl              0xA3B9
#define sysTrapDbgSerDrvStatus              0xA3BA
#define sysTrapDbgSerDrvWriteChar            0xA3BB
#define sysTrapDbgSerDrvReadChar            0xA3BC

// new boot-time BigROM HAL additions
#define sysTrapHwrPostDebugInit              0xA3BD
#define sysTrapHwrIdentifyFeatures            0xA3BE
#define sysTrapHwrModelSpecificInit            0xA3BF
#define sysTrapHwrModelInitStage2            0xA3C0
#define sysTrapHwrInterruptsInit            0xA3C1

#define sysTrapHwrSoundOn                0xA3C2
#define sysTrapHwrSoundOff                0xA3C3

// Kernel clock tick routine
#define sysTrapSysKernelClockTick            0xA3C4

// MenuEraseMenu is exposed as of PalmOS 3.5, but there are
// no public interfaces for it yet.   Perhaps in a later release.
#define sysTrapMenuEraseMenu              0xA3C5

#define sysTrapSelectTime                0xA3C6

// Menu Command Bar traps
#define sysTrapMenuCmdBarAddButton            0xA3C7
#define sysTrapMenuCmdBarGetButtonData          0xA3C8
#define sysTrapMenuCmdBarDisplay            0xA3C9

// Silkscreen info
#define sysTrapHwrGetSilkscreenID            0xA3CA
#define sysTrapEvtGetSilkscreenAreaList          0xA3CB

#define sysTrapSysFatalAlertInit            0xA3CC
#define sysTrapDateTemplateToAscii            0xA3CD

// New traps dealing with masking private records
#define sysTrapSecVerifyPW                0xA3CE
#define sysTrapSecSelectViewStatus            0xA3CF
#define sysTrapTblSetColumnMasked            0xA3D0
#define sysTrapTblSetRowMasked              0xA3D1
#define sysTrapTblRowMasked                0xA3D2

// New form trap for dialogs with text entry field
#define sysTrapFrmCustomResponseAlert          0xA3D3
#define sysTrapFrmNewGsi                0xA3D4

// New dynamic menu functions
#define sysTrapMenuShowItem                0xA3D5
#define sysTrapMenuHideItem                0xA3D6
#define sysTrapMenuAddItem                0xA3D7

// New form traps for "smart gadgets"
#define sysTrapFrmSetGadgetHandler            0xA3D8

// More new control functions
#define sysTrapCtlSetGraphics              0xA3D9
#define sysTrapCtlGetSliderValues            0xA3DA
#define sysTrapCtlSetSliderValues            0xA3DB
#define sysTrapCtlNewSliderControl            0xA3DC

// Bitmap manager functions
#define sysTrapBmpCreate                0xA3DD
#define sysTrapBmpDelete                0xA3DE
#define sysTrapBmpCompress                0xA3DF
// sysTrapBmpGetBits defined in Screen driver traps
#define sysTrapBmpGetColortable              0xA3E0
#define sysTrapBmpSize                  0xA3E1
#define sysTrapBmpBitsSize                0xA3E2
#define sysTrapBmpColortableSize            0xA3E3

// extra window namager 
#define sysTrapWinCreateBitmapWindow          0xA3E4

// Ask for a null event sooner (replaces a macro which Poser hated)
#define sysTrapEvtSetNullEventTick            0xA3E5

// Exchange manager call to allow apps to select destination categories
#define sysTrapExgDoDialog                0xA3E6

// this call will remove temporary UI like popup lists
#define sysTrapSysUICleanup                0xA3E7

// The following 4 traps were "Reserved" traps, present only in SOME post-release builds of Palm OS 3.5
#define sysTrapWinSetForeColorRGB            0xA3E8
#define sysTrapWinSetBackColorRGB            0xA3E9
#define sysTrapWinSetTextColorRGB            0xA3EA
#define sysTrapWinGetPixelRGB              0xA3EB  

// ======================================================================
// Palm OS 4.0 Traps
// ======================================================================

#define sysTrapHighDensityDispatch            0xA3EC
#define sysTrapSysReserved40Trap2            0xA3ED
#define sysTrapSysReserved40Trap3            0xA3EE
#define sysTrapSysReserved40Trap4            0xA3EF

// New Trap selector added for New Connection Mgr API 
#define sysTrapCncMgrDispatch              0xA3F0

// new trap for notify from interrupt, implemented in SysEvtMgr.c
#define sysTrapSysNotifyBroadcastFromInterrupt      0xA3F1

// new trap for waking the UI without generating a null event
#define sysTrapEvtWakeupWithoutNilEvent          0xA3F2

// new trap for doing stable, fast, 7-bit string compare
#define sysTrapStrCompareAscii              0xA3F3

// New trap for accessors available thru PalmOS glue
#define sysTrapAccessorDispatch              0xA3F4

#define sysTrapBltGetPixel                0xA3F5  
#define sysTrapBltPaintPixel              0xA3F6

#define sysTrapScrScreenInit              0xA3F7
#define sysTrapScrUpdateScreenBitmap          0xA3F8
#define sysTrapScrPalette                0xA3F9
#define sysTrapScrGetColortable              0xA3FA
#define sysTrapScrGetGrayPat              0xA3FB
#define sysTrapScrScreenLock              0xA3FC
#define sysTrapScrScreenUnlock              0xA3FD

#define sysTrapFntPrvGetFontList            0xA3FE

// Exchange manager functions
#define sysTrapExgRegisterDatatype            0xA3FF
#define sysTrapExgNotifyReceive              0xA400
#define sysTrapExgNotifyGoto              0xA401
#define sysTrapExgRequest                0xA402
#define sysTrapExgSetDefaultApplication          0xA403
#define sysTrapExgGetDefaultApplication          0xA404
#define sysTrapExgGetTargetApplication          0xA405
#define sysTrapExgGetRegisteredApplications        0xA406
#define sysTrapExgGetRegisteredTypes          0xA407
#define sysTrapExgNotifyPreview              0xA408
#define sysTrapExgControl                0xA409

// 04/30/00  CS - New Locale Manager handles access to region-specific info like date formats
#define sysTrapLmDispatch                0xA40A

// 05/10/00 kwk - New Memory Manager trap for retrieving ROM NVParam values (sys use only)
#define sysTrapMemGetRomNVParams            0xA40B

// 05/12/00 kwk - Safe character width Font Mgr call
#define sysTrapFntWCharWidth              0xA40C

// 05/17/00 kwk - Faster DmFindDatabase
#define sysTrapDmFindDatabaseWithTypeCreator      0xA40D

// New Trap selectors added for time zone picker API
#define sysTrapSelectTimeZone              0xA40E
#define sysTrapTimeZoneToAscii              0xA40F

// 08/18/00 kwk - trap for doing stable, fast, 7-bit string compare.
// 08/21/00 kwk - moved here in place of sysTrapSelectDaylightSavingAdjustment.
#define sysTrapStrNCompareAscii              0xA410

// New Trap selectors added for time zone conversion API
#define sysTrapTimTimeZoneToUTC              0xA411
#define sysTrapTimUTCToTimeZone              0xA412

// New trap implemented in PhoneLookup.c
#define sysTrapPhoneNumberLookupCustom          0xA413

// new trap for selecting debugger path.
#define sysTrapHwrDebugSelect              0xA414

#define sysTrapBltRoundedRectangle            0xA415
#define sysTrapBltRoundedRectangleFill          0xA416
#define sysTrapWinPrvInitCanvas              0xA417

#define sysTrapHwrCalcDynamicHeapSize          0xA418
#define sysTrapHwrDebuggerEnter              0xA419
#define sysTrapHwrDebuggerExit              0xA41A

#define sysTrapLstGetTopItem              0xA41B

#define sysTrapHwrModelInitStage3            0xA41C

// 06/21/00 peter - New Attention Manager
#define sysTrapAttnIndicatorAllow            0xA41D
#define sysTrapAttnIndicatorAllowed            0xA41E
#define sysTrapAttnIndicatorEnable            0xA41F
#define sysTrapAttnIndicatorEnabled            0xA420
#define sysTrapAttnIndicatorSetBlinkPattern        0xA421
#define sysTrapAttnIndicatorGetBlinkPattern        0xA422
#define sysTrapAttnIndicatorTicksTillNextBlink      0xA423
#define sysTrapAttnIndicatorCheckBlink          0xA424
#define sysTrapAttnInitialize              0xA425
#define sysTrapAttnGetAttention              0xA426
#define sysTrapAttnUpdate                0xA427
#define sysTrapAttnForgetIt                0xA428
#define sysTrapAttnGetCounts              0xA429
#define sysTrapAttnListOpen                0xA42A
#define sysTrapAttnHandleEvent              0xA42B
#define sysTrapAttnEffectOfEvent            0xA42C
#define sysTrapAttnIterate                0xA42D
#define sysTrapAttnDoSpecialEffects            0xA42E
#define sysTrapAttnDoEmergencySpecialEffects      0xA42F
#define sysTrapAttnAllowClose              0xA430
#define sysTrapAttnReopen                0xA431
#define sysTrapAttnEnableNotification          0xA432
#define sysTrapHwrLEDAttributes              0xA433
#define sysTrapHwrVibrateAttributes            0xA434

// Trap for getting and setting the device password hint.
#define sysTrapSecGetPwdHint              0xA435
#define sysTrapSecSetPwdHint              0xA436

#define sysTrapHwrFlashWrite              0xA437

#define sysTrapKeyboardStatusNew            0xA438
#define sysTrapKeyboardStatusFree            0xA439
#define sysTrapKbdSetLayout                0xA43A
#define sysTrapKbdGetLayout                0xA43B
#define sysTrapKbdSetPosition              0xA43C
#define sysTrapKbdGetPosition              0xA43D
#define sysTrapKbdSetShiftState              0xA43E
#define sysTrapKbdGetShiftState              0xA43F
#define sysTrapKbdDraw                  0xA440
#define sysTrapKbdErase                  0xA441
#define sysTrapKbdHandleEvent              0xA442

#define sysTrapOEMDispatch2                0xA443

#define sysTrapHwrCustom                0xA444

// 08/28/00 kwk - Trap for getting form's active field.
#define sysTrapFrmGetActiveField            0xA445

// 9/18/00 rkr - Added for playing sounds regardless of interruptible flag
#define sysTrapSndPlaySmfIrregardless          0xA446
#define sysTrapSndPlaySmfResourceIrregardless      0xA447
#define sysTrapSndInterruptSmfIrregardless        0xA448

// 10/14/00 ABa: UDA manager
#define sysTrapUdaMgrDispatch              0xA449

// WK: private traps for PalmOS
#define sysTrapPalmPrivate1                0xA44A
#define sysTrapPalmPrivate2                0xA44B
#define sysTrapPalmPrivate3                0xA44C
#define sysTrapPalmPrivate4                0xA44D

// 11/07/00 tlw: Added accessors
#define sysTrapBmpGetDimensions              0xA44E
#define sysTrapBmpGetBitDepth              0xA44F
#define sysTrapBmpGetNextBitmap              0xA450
#define sysTrapTblGetNumberOfColumns          0xA451
#define sysTrapTblGetTopRow                0xA452
#define sysTrapTblSetSelection              0xA453
#define sysTrapFrmGetObjectIndexFromPtr          0xA454

// 11/10/00 acs
#define sysTrapBmpGetSizes                0xA455

#define sysTrapWinGetBounds                0xA456

#define sysTrapBltPaintPixels              0xA457

// 11/22/00 bob
#define sysTrapFldSetMaxVisibleLines          0xA458

// 01/09/01 acs
#define sysTrapScrDefaultPaletteState          0xA459

// ======================================================================
// Palm OS 5.0 Traps
// No new traps were added for 4.1, though 4.1 SC (see below) added some.
// ======================================================================

// 11/16/01 bob
#define sysTrapPceNativeCall              0xA45A

// ======================================================================
// Palm OS 5.1 Traps
// ======================================================================

// 12/04/01 lrt
#define sysTrapSndStreamCreate              0xA45B
#define sysTrapSndStreamDelete              0xA45C
#define sysTrapSndStreamStart              0xA45D
#define sysTrapSndStreamPause              0xA45E
#define sysTrapSndStreamStop              0xA45F
#define sysTrapSndStreamSetVolume            0xA460
#define sysTrapSndStreamGetVolume            0xA461
#define sysTrapSndPlayResource              0xA462
#define sysTrapSndStreamSetPan              0xA463
#define sysTrapSndStreamGetPan              0xA464

// 04/12/02 jed
#define sysTrapMultimediaDispatch            0xA465

// TRAPS ABOVE THIS POINT CAN NOT CHANGE BECAUSE THEY HAVE
// BEEN RELEASED TO CUSTOMERS IN SHIPPING ROMS AND SDKS.
// (MOVE THIS COMMENT DOWN WHENEVER THE "NEXT" RELEASE OCCURS.)

// ======================================================================
// Palm OS 5.1.1 Traps
// ======================================================================

// 08/02/02 mne
#define sysTrapSndStreamCreateExtended          0xa466
#define sysTrapSndStreamDeviceControl          0xa467

// ======================================================================
// Palm OS 4.2SC (Simplified Chinese) Traps
// These were added to an older 68K-based version of Palm OS to support
// QVGA displays.
// ======================================================================

// 09/23/02 acs & bob
#define sysTrapBmpCreateVersion3            0xA468
#define sysTrapECFixedMul                0xA469
#define sysTrapECFixedDiv                0xA46A
#define sysTrapHALDrawGetSupportedDensity        0xA46B
#define sysTrapHALRedrawInputArea            0xA46C
#define sysTrapGrfBeginStroke              0xA46D
#define sysTrapBmpPrvConvertBitmap            0xA46E

// ======================================================================
// Palm OS 5.x Traps
// These were added for new features or extensions for 5.x
// ======================================================================
#define sysTrapSysReservedTrap5                   0xA46F
#define sysTrapNavSelector                        0xA46F


// 12/11/02 grant
#define sysTrapPinsDispatch                0xA470

// ======================================================================
// Palm OS 5.3 Traps
// These were added for new features or extensions for 5.2. Currently
// they aren't implemented by any version of Palm OS released by
// PalmSource, but are reserved for future implementation.
// ======================================================================
#define sysTrapSysReservedTrap1           0xA471
#define sysTrapSysReservedTrap2          0xA472
#define sysTrapSysReservedTrap3          0xA473
#define sysTrapSysReservedTrap4          0xA474

#define sysLibTrapName          0xA800
#define sysLibTrapOpen          0xA801
#define sysLibTrapClose         0xA802
#define sysLibTrapSleep         0xA803
#define sysLibTrapWake          0xA804
#define sysLibTrapCustom        0xA805

// selectors
//
#define sysTrapCncMgrProfileSettingGet            1
#define sysTrapCncMgrProfileSettingSet            2
#define sysTrapCncMgrProfileGetCurrent            3
#define sysTrapCncMgrProfileSetCurrent            4
#define sysTrapCncMgrProfileGetIDFromName         5
#define sysTrapCncMgrProfileCreate              6
#define sysTrapCncMgrProfileDelete              7
#define sysTrapCncMgrProfileGetIDFromIndex          8
#define sysTrapCncMgrProfileGetIndex            9
#define sysTrapCncMgrProfileCount             10
#define sysTrapCncMgrProfileOpenDB              11
#define sysTrapCncMgrProfileCloseDB             12

#define expInit         0
#define expSlotDriverInstall  1
#define expSlotDriverRemove 2
#define expSlotLibFind      3
#define expSlotRegister     4
#define expSlotUnregister   5
#define expCardInserted     6
#define expCardRemoved      7
#define expCardPresent      8
#define expCardInfo       9
#define expSlotEnumerate    10
#define expCardGetSerialPort  11

#define sysFloatBase10Info      0
#define sysFloatFToA          1
#define sysFloatAToF          2
#define sysFloatCorrectedAdd    3
#define sysFloatCorrectedSub    4
#define sysFloatVersion       5
#define sysFloatEm_fp_round   0
#define sysFloatEm_fp_get_fpscr 1
#define sysFloatEm_fp_set_fpscr 2
#define sysFloatEm_f_utof     3
#define sysFloatEm_f_itof     4
#define sysFloatEm_f_ulltof   5
#define sysFloatEm_f_lltof      6
#define sysFloatEm_d_utod     7
#define sysFloatEm_d_itod     8
#define sysFloatEm_d_ulltod   9
#define sysFloatEm_d_lltod      10
#define sysFloatEm_f_ftod     11
#define sysFloatEm_d_dtof     12
#define sysFloatEm_f_ftoq     13
#define sysFloatEm_f_qtof     14
#define sysFloatEm_d_dtoq     15
#define sysFloatEm_d_qtod     16
#define sysFloatEm_f_ftou     17
#define sysFloatEm_f_ftoi     18
#define sysFloatEm_f_ftoull   19
#define sysFloatEm_f_ftoll      20
#define sysFloatEm_d_dtou     21
#define sysFloatEm_d_dtoi     22
#define sysFloatEm_d_dtoull   23
#define sysFloatEm_d_dtoll      24
#define sysFloatEm_f_cmp      25
#define sysFloatEm_f_cmpe     26
#define sysFloatEm_f_feq      27
#define sysFloatEm_f_fne      28
#define sysFloatEm_f_flt      29
#define sysFloatEm_f_fle      30
#define sysFloatEm_f_fgt      31
#define sysFloatEm_f_fge      32
#define sysFloatEm_f_fun      33
#define sysFloatEm_f_for      34
#define sysFloatEm_d_cmp      35
#define sysFloatEm_d_cmpe     36
#define sysFloatEm_d_feq      37
#define sysFloatEm_d_fne      38
#define sysFloatEm_d_flt      39
#define sysFloatEm_d_fle      40
#define sysFloatEm_d_fgt      41
#define sysFloatEm_d_fge      42
#define sysFloatEm_d_fun      43
#define sysFloatEm_d_for      44
#define sysFloatEm_f_neg      45
#define sysFloatEm_f_add      46
#define sysFloatEm_f_mul      47
#define sysFloatEm_f_sub      48
#define sysFloatEm_f_div      49
#define sysFloatEm_d_neg      50
#define sysFloatEm_d_add      51
#define sysFloatEm_d_mul      52
#define sysFloatEm_d_sub      53
#define sysFloatEm_d_div      54

#define HDSelectorBmpGetNextBitmapAnyDensity    0
#define HDSelectorBmpGetVersion           1
#define HDSelectorBmpGetCompressionType       2
#define HDSelectorBmpGetDensity           3
#define HDSelectorBmpSetDensity           4
#define HDSelectorBmpGetTransparentValue      5
#define HDSelectorBmpSetTransparentValue      6
#define HDSelectorBmpCreateBitmapV3         7
#define HDSelectorWinSetCoordinateSystem      8
#define HDSelectorWinGetCoordinateSystem      9
#define HDSelectorWinScalePoint           10
#define HDSelectorWinUnscalePoint         11
#define HDSelectorWinScaleRectangle         12
#define HDSelectorWinUnscaleRectangle       13
#define HDSelectorWinScreenGetAttribute       14
#define HDSelectorWinPaintTiledBitmap       15
#define HDSelectorWinGetSupportedDensity      16
#define HDSelectorEvtGetPenNative         17
#define HDSelectorWinScaleCoord           18
#define HDSelectorWinUnscaleCoord         19
#define HDSelectorWinPaintRoundedRectangleFrame 20
#define HDSelectorWinSetScalingMode         21
#define HDSelectorWinGetScalingMode         22

#define intlIntlInit          0
#define intlTxtByteAttr         1
#define intlTxtCharAttr         2
#define intlTxtCharXAttr        3
#define intlTxtCharSize         4
#define intlTxtGetPreviousChar      5
#define intlTxtGetNextChar        6
#define intlTxtGetChar          7
#define intlTxtSetNextChar        8
#define intlTxtCharBounds       9
#define intlTxtPrepFindString     10
#define intlTxtFindString       11
#define intlTxtReplaceStr       12
#define intlTxtWordBounds       13
#define intlTxtCharEncoding       14
#define intlTxtStrEncoding        15
#define intlTxtEncodingName       16
#define intlTxtMaxEncoding        17
#define intlTxtTransliterate      18
#define intlTxtCharIsValid        19
#define intlTxtCompare          20
#define intlTxtCaselessCompare      21
#define intlTxtCharWidth        22
#define intlTxtGetTruncationOffset    23
#define intlIntlGetRoutineAddress   24
#define intlIntlHandleEvent       25
#define intlTxtParamString        26
#define intlTxtConvertEncodingV35   27
#define intlTxtConvertEncoding      28
#define intlIntlSetRoutineAddress   29
#define intlTxtGetWordWrapOffset    30
#define intlTxtNameToEncoding     31
#define intlIntlStrictChecks      32
#define intlTxtUpperChar          33
#define intlTxtLowerChar          34

#define vfsTrapInit           0
#define vfsTrapCustomControl      1
#define vfsTrapFileCreate       2
#define vfsTrapFileOpen         3
#define vfsTrapFileClose        4
#define vfsTrapFileReadData     5
#define vfsTrapFileRead         6
#define vfsTrapFileWrite        7
#define vfsTrapFileDelete       8
#define vfsTrapFileRename       9
#define vfsTrapFileSeek         10
#define vfsTrapFileEOF          11
#define vfsTrapFileTell         12
#define vfsTrapFileResize       13
#define vfsTrapFileGetAttributes    14
#define vfsTrapFileSetAttributes    15
#define vfsTrapFileGetDate        16
#define vfsTrapFileSetDate        17
#define vfsTrapFileSize         18
#define vfsTrapDirCreate        19
#define vfsTrapDirEntryEnumerate    20
#define vfsTrapGetDefaultDirectory  21
#define vfsTrapRegisterDefaultDirectory 22
#define vfsTrapUnregisterDefaultDirectory 23
#define vfsTrapVolumeFormat     24
#define vfsTrapVolumeMount        25
#define vfsTrapVolumeUnmount      26
#define vfsTrapVolumeEnumerate    27
#define vfsTrapVolumeInfo       28
#define vfsTrapVolumeGetLabel     29
#define vfsTrapVolumeSetLabel     30
#define vfsTrapVolumeSize       31
#define vfsTrapInstallFSLib     32
#define vfsTrapRemoveFSLib        33
#define vfsTrapImportDatabaseFromFile 34
#define vfsTrapExportDatabaseToFile   35
#define vfsTrapFileDBGetResource    36
#define vfsTrapFileDBInfo       37
#define vfsTrapFileDBGetRecord    38
#define vfsTrapImportDatabaseFromFileCustom 39
#define vfsTrapExportDatabaseToFileCustom   40
#define vfsTrapPrivate1         41

#define omInit              0
#define omOpenOverlayDatabase   1
#define omLocaleToOverlayDBName 2
#define omOverlayDBNameToLocale 3
#define omGetCurrentLocale      4
#define omGetIndexedLocale      5
#define omGetSystemLocale     6
#define omSetSystemLocale     7
#define omGetRoutineAddress   8
#define omGetNextSystemLocale   9

#define pinPINSetInputAreaState       0
#define pinPINGetInputAreaState       1
#define pinPINSetInputTriggerState      2
#define pinPINGetInputTriggerState      3
#define pinPINAltInputSystemEnabled     4
#define pinPINGetCurrentPinletName      5
#define pinPINSwitchToPinlet        6
#define pinPINCountPinlets          7
#define pinPINGetPinletInfo         8
#define pinPINSetInputMode          9
#define pinPINGetInputMode          10
#define pinPINClearPinletState        11
#define pinPINShowReferenceDialog     12
#define pinWinSetConstraintsSize      13
#define pinFrmSetDIAPolicyAttr        14
#define pinFrmGetDIAPolicyAttr        15
#define pinStatHide             16
#define pinStatShow             17
#define pinStatGetAttribute         18
#define pinSysGetOrientation              19
#define pinSysSetOrientation              20
#define pinSysGetOrientationTriggerState  21
#define pinSysSetOrientationTriggerState  22

#define sysSerialInstall        0
#define sysSerialOpen         1
#define sysSerialOpenBkgnd        2
#define sysSerialClose          3
#define sysSerialSleep          4
#define sysSerialWake         5
#define sysSerialGetDeviceCount   6
#define sysSerialGetDeviceInfo    7
#define sysSerialGetStatus        8
#define sysSerialClearErr       9
#define sysSerialControl        10
#define sysSerialSend         11
#define sysSerialSendWait       12
#define sysSerialSendCheck        13
#define sysSerialSendFlush        14
#define sysSerialReceive        15
#define sysSerialReceiveWait      16
#define sysSerialReceiveCheck     17
#define sysSerialReceiveFlush     18
#define sysSerialSetRcvBuffer     19
#define sysSerialRcvWindowOpen    20
#define sysSerialRcvWindowClose   21
#define sysSerialSetWakeupHandler 22
#define sysSerialPrimeWakeupHandler 23
#define sysSerialOpenV4         24
#define sysSerialOpenBkgndV4      25
#define sysSerialCustomControl    26

#define tsmGetFepMode       0
#define tsmSetFepMode       1
#define tsmHandleEvent        2
#define tsmInit           3
#define tsmDrawMode         4
#define tsmGetSystemFep     5
#define tsmSetSystemFep     6
#define tsmGetCurrentFep      7
#define tsmSetCurrentFep      8
#define tsmGetSystemFepCreator  9
#define tsmSetSystemFepCreator  10
#define tsmGetCurrentFepCreator 11
#define tsmSetCurrentFepCreator 12
#define tsmFepHandleEvent     13
#define tsmFepMapEvent        14
#define tsmFepTerminate     15
#define tsmFepReset         16
#define tsmFepCommitAction    17
#define tsmFepOptionsList     18

#define sysUdaControl         0
#define sysUdaMemoryReaderNew     1
#define sysUdaExchangeReaderNew     11
#define sysUdaExchangeWriterNew     12

#define hostSelectorGetHostVersion      0x0100
#define hostSelectorGetHostID        0x0101
#define hostSelectorGetHostPlatform      0x0102
#define hostSelectorIsSelectorImplemented  0x0103
#define hostSelectorGestalt          0x0104
#define hostSelectorIsCallingTrap      0x0105


  // Profiler selectors

#define hostSelectorProfileInit        0x0200
#define hostSelectorProfileStart      0x0201
#define hostSelectorProfileStop        0x0202
#define hostSelectorProfileDump        0x0203
#define hostSelectorProfileCleanup      0x0204
#define hostSelectorProfileDetailFn      0x0205
#define hostSelectorProfileGetCycles    0x0206


  // Std C Library wrapper selectors

#define hostSelectorErrNo          0x0300

#define hostSelectorFClose          0x0301
#define hostSelectorFEOF          0x0302
#define hostSelectorFError          0x0303
#define hostSelectorFFlush          0x0304
#define hostSelectorFGetC          0x0305
#define hostSelectorFGetPos          0x0306
#define hostSelectorFGetS          0x0307
#define hostSelectorFOpen          0x0308
#define hostSelectorFPrintF          0x0309    /* Floating point not yet supported in Poser */
#define hostSelectorFPutC          0x030A
#define hostSelectorFPutS          0x030B
#define hostSelectorFRead          0x030C
#define hostSelectorRemove          0x030D
#define hostSelectorRename          0x030E
#define hostSelectorFReopen          0x030F    /* Not yet implemented in Poser */
#define hostSelectorFScanF          0x0310    /* Not yet implemented */
#define hostSelectorFSeek          0x0311
#define hostSelectorFSetPos          0x0312
#define hostSelectorFTell          0x0313
#define hostSelectorFWrite          0x0314
#define hostSelectorTmpFile          0x0315
#define hostSelectorTmpNam          0x0316

#define hostSelectorGetEnv          0x0317

#define hostSelectorMalloc          0x0318
#define hostSelectorRealloc          0x0319
#define hostSelectorFree          0x031A

  // time.h wrappers
#define hostSelectorAscTime          0x0370
#define hostSelectorClock          0x0371
#define hostSelectorCTime          0x0372
// #define hostSelectorDiffTime        0x0373
#define hostSelectorGMTime          0x0374
#define hostSelectorLocalTime        0x0375
#define hostSelectorMkTime          0x0376
#define hostSelectorStrFTime        0x0377
#define hostSelectorTime          0x0378

  // dirent.h wrappers
#define hostSelectorOpenDir          0x0380
#define hostSelectorReadDir          0x0381
// #define hostSelectorRewindDir        0x0382
#define hostSelectorCloseDir        0x0383
// #define hostSelectorTellDir          0x0384
// #define hostSelectorSeekDir          0x0385
// #define hostSelectorScanDir          0x0386

  // fcntl.h wrappers
// #define hostSelectorOpen          0x0386
// #define hostSelectorCreat          0x0388
// #define hostSelectorFcntl          0x0389

  // unistd.h wrappers
// #define hostSelectorAccess          0x038A
// #define hostSelectorChDir          0x038B
// #define hostSelectorClose          0x038C
// #define hostSelectorDup            0x038D
// #define hostSelectorDup2          0x038E
// #define hostSelectorGetCwd          0x038F
// #define hostSelectorIsATTY          0x0390
// #define hostSelectorLink          0x0391
// #define hostSelectorLSeek          0x0392
// #define hostSelectorRead          0x0393
#define hostSelectorRmDir          0x0394
// #define hostSelectorTTYName          0x0395
// #define hostSelectorUnlink          0x0396
// #define hostSelectorWrite          0x0397

// #define hostSelectorFChDir          0x0398
// #define hostSelectorFChMod          0x0399
// #define hostSelectorFileNo          0X039A
// #define hostSelectorFSync          0x039B
// #define hostSelectorFTruncate        0x039C
// #define hostSelectorGetHostName        0x039D
// #define hostSelectorGetWD          0x039E
// #define hostSelectorMkSTemp          0x039F
// #define hostSelectorMkTemp          0x03A0
// #define hostSelectorRe_Comp          0x03A1
// #define hostSelectorRe_Exec          0x03A2
// #define hostSelectorReadLink        0x03A3
// #define hostSelectorSetHostName        0x03A4
// #define hostSelectorSymLink          0x03A5
// #define hostSelectorSync          0x03A6
#define hostSelectorTruncate        0x03A7

  // sys/stat.h wrappers
// #define hostSelectorChMod          0x03A8
// #define hostSelectorFStat          0x03A9
#define hostSelectorMkDir          0x03AA
#define hostSelectorStat          0x03AB
// #define hostSelectorLStat          0x03AC

  // sys/time.h wrappers
// #define hostSelectorGetTimeOfDay      0x03AD
#define hostSelectorUTime          0x03AE

  // DOS attr 
#define hostSelectorGetFileAttr        0x03AF
#define hostSelectorSetFileAttr        0x03B0

  // Gremlin selectors

#define hostSelectorGremlinIsRunning    0x0400
#define hostSelectorGremlinNumber      0x0401
#define hostSelectorGremlinCounter      0x0402
#define hostSelectorGremlinLimit      0x0403
#define hostSelectorGremlinNew        0x0404


  // Database selectors

#define hostSelectorImportFile        0x0500
#define hostSelectorExportFile        0x0501
#define hostSelectorSaveScreen        0x0502
#define hostSelectorImportFileWithID    0x0503

#define hostSelectorExgLibOpen        0x0580
#define hostSelectorExgLibClose        0x0581
#define hostSelectorExgLibSleep        0x0582
#define hostSelectorExgLibWake        0x0583
#define hostSelectorExgLibHandleEvent    0x0584
#define hostSelectorExgLibConnect      0x0585
#define hostSelectorExgLibAccept      0x0586
#define hostSelectorExgLibDisconnect    0x0587
#define hostSelectorExgLibPut        0x0588
#define hostSelectorExgLibGet        0x0589
#define hostSelectorExgLibSend        0x058A
#define hostSelectorExgLibReceive      0x058B
#define hostSelectorExgLibControl      0x058C
#define hostSelectorExgLibRequest      0x058D


  // Preferences selectors

#define hostSelectorGetPreference      0x0600
#define hostSelectorSetPreference      0x0601


  // Logging selectors

#define hostSelectorLogFile          0x0700
#define hostSelectorSetLogFileSize      0x0701


  // RPC selectors

#define hostSelectorSessionCreate      0x0800    /* Not yet implemented in Poser */
#define hostSelectorSessionOpen        0x0801    /* Not yet implemented in Poser */
#define hostSelectorSessionClose      0x0802
#define hostSelectorSessionQuit        0x0803
#define hostSelectorSignalSend        0x0804
#define hostSelectorSignalWait        0x0805
#define hostSelectorSignalResume      0x0806
#define hostSelectorSessionSave        0x0807


  // External tracing tool support

#define hostSelectorTraceInit        0x0900
#define hostSelectorTraceClose        0x0901
#define hostSelectorTraceOutputT      0x0902
#define hostSelectorTraceOutputTL      0x0903
#define hostSelectorTraceOutputVT      0x0904
#define hostSelectorTraceOutputVTL      0x0905
#define hostSelectorTraceOutputB      0x0906


  // External debugger support

#define hostSelectorDbgSetDataBreak      0x0980    // mcc 13 june 2001
#define hostSelectorDbgClearDataBreak    0x0981    // mcc 13 june 2001


  // Slot support

#define hostSelectorSlotMax          0x0A00
#define hostSelectorSlotRoot        0x0A01
#define hostSelectorSlotHasCard        0x0A02


  // File choosing support

#define hostSelectorGetFile          0x0B00
#define hostSelectorPutFile          0x0B01
#define hostSelectorGetDirectory      0x0B02

#define hostSelectorLastTrapNumber      0x0BFF

#define lmInit                                                  0
#define lmGetNumLocales                         1
#define lmLocaleToIndex                         2
#define lmGetLocaleSetting                      3

#define vfsTrapInit            0
#define vfsTrapCustomControl      1

#define vfsTrapFileCreate        2
#define vfsTrapFileOpen          3
#define vfsTrapFileClose        4
#define vfsTrapFileReadData      5
#define vfsTrapFileRead          6
#define vfsTrapFileWrite        7
#define vfsTrapFileDelete        8
#define vfsTrapFileRename        9
#define vfsTrapFileSeek          10
#define vfsTrapFileEOF          11
#define vfsTrapFileTell          12
#define vfsTrapFileResize        13
#define vfsTrapFileGetAttributes    14
#define vfsTrapFileSetAttributes    15
#define vfsTrapFileGetDate        16
#define vfsTrapFileSetDate        17
#define vfsTrapFileSize          18

#define vfsTrapDirCreate        19
#define vfsTrapDirEntryEnumerate    20
#define vfsTrapGetDefaultDirectory  21
#define vfsTrapRegisterDefaultDirectory  22
#define vfsTrapUnregisterDefaultDirectory  23

#define vfsTrapVolumeFormat      24
#define vfsTrapVolumeMount        25
#define vfsTrapVolumeUnmount      26
#define vfsTrapVolumeEnumerate    27
#define vfsTrapVolumeInfo        28
#define vfsTrapVolumeGetLabel      29
#define vfsTrapVolumeSetLabel      30
#define vfsTrapVolumeSize        31

#define vfsTrapInstallFSLib      32
#define vfsTrapRemoveFSLib        33
#define vfsTrapImportDatabaseFromFile  34
#define vfsTrapExportDatabaseToFile    35
#define vfsTrapFileDBGetResource    36
#define vfsTrapFileDBInfo        37
#define vfsTrapFileDBGetRecord    38

#define vfsTrapImportDatabaseFromFileCustom  39
#define vfsTrapExportDatabaseToFileCustom    40

// System use only
#define vfsTrapPrivate1          41

#define intlMaxSelector   intlTxtLowerChar
#define omMaxSelector     omGetNextSystemLocale
#define vfsMaxSelector    vfsTrapPrivate1
#define tsmMaxSelector    tsmFepOptionsList
#define NavSelectorFrmNavObjectTakeFocus 0xA
#define HDSelectorInvalid 23

// end selectors


#define MAX_STACK 256

#include "trapArgs.c"

#include "m68kdasm.c"

struct logtrap_t {
  void *(*alloc)(uint32_t size, void *data);
  void *(*realloc)(void *p, uint32_t size, void *data);
  void (*free)(void *p, void *data);
  void (*print)(char *s, void *data);
  uint8_t (*read8)(uint32_t addr, void *data);
  uint16_t (*read16)(uint32_t addr, void *data);
  uint32_t (*read32)(uint32_t addr, void *data);
  uint32_t (*getreg)(uint32_t reg, void *data);
  int disasm;
  void *data;

  char *appname;
  uint32_t stackp;
  uint32_t stack[MAX_STACK];
  uint32_t stackt[MAX_STACK];
  uint32_t stacksel[MAX_STACK];
  uint32_t log_dbID;
  uint32_t log_dbRef;
  uint32_t log_f;
  trap_t *allTraps;
  m68k_disassemble_t dis;
};

typedef union {
  uint32_t t;
  uint8_t c[4];
} creator_id_t;

typedef enum {
  nilEvent = 0,        // system level
  penDownEvent,        // system level
  penUpEvent,          // system level
  penMoveEvent,        // system level
  keyDownEvent,        // system level
  winEnterEvent,        // system level
  winExitEvent,        // system level
  ctlEnterEvent,
  ctlExitEvent,
  ctlSelectEvent,
  ctlRepeatEvent,
  lstEnterEvent,
  lstSelectEvent,
  lstExitEvent,
  popSelectEvent,
  fldEnterEvent,
  fldHeightChangedEvent,
  fldChangedEvent,
  tblEnterEvent,
  tblSelectEvent,
  daySelectEvent,
  menuEvent,
  appStopEvent = 22,      // system level
  frmLoadEvent,
  frmOpenEvent,
  frmGotoEvent,
  frmUpdateEvent,
  frmSaveEvent,
  frmCloseEvent,
  frmTitleEnterEvent,
  frmTitleSelectEvent,
  tblExitEvent,
  sclEnterEvent,
  sclExitEvent,
  sclRepeatEvent,
  tsmConfirmEvent = 35,    // system level
  tsmFepButtonEvent,      // system level
  tsmFepModeEvent,        // system level
  attnIndicatorEnterEvent,  // for attention manager's indicator
  attnIndicatorSelectEvent,  // for attention manager's indicator
} eventsEnum;

#define lastRegularEvent attnIndicatorSelectEvent

static char *eventName[] = {
  "nilEvent",
  "penDownEvent",
  "penUpEvent",
  "penMoveEvent",
  "keyDownEvent",
  "winEnterEvent",
  "winExitEvent",
  "ctlEnterEvent",
  "ctlExitEvent",
  "ctlSelectEvent",
  "ctlRepeatEvent",
  "lstEnterEvent",
  "lstSelectEvent",
  "lstExitEvent",
  "popSelectEvent",
  "fldEnterEvent",
  "fldHeightChangedEvent",
  "fldChangedEvent",
  "tblEnterEvent",
  "tblSelectEvent",
  "daySelectEvent",
  "menuEvent",
  "appStopEvent",
  "frmLoadEvent",
  "frmOpenEvent",
  "frmGotoEvent",
  "frmUpdateEvent",
  "frmSaveEvent",
  "frmCloseEvent",
  "frmTitleEnterEvent",
  "frmTitleSelectEvent",
  "tblExitEvent",
  "sclEnterEvent",
  "sclExitEvent",
  "sclRepeatEvent",
  "tsmConfirmEvent",
  "tsmFepButtonEvent",
  "tsmFepModeEvent",
  "attnIndicatorEnterEvent",
  "attnIndicatorSelectEvent"
};

static char *nav_traps[] = {
  "FrmCountObjectsInNavOrder",
  "FrmGetNavOrder",
  "FrmSetNavOrder",
  "FrmGetNavEntry",
  "FrmSetNavEntry",
  "FrmGetNavState",
  "FrmSetNavState",
  "FrmNavDrawFocusRing",
  "FrmNavRemoveFocusRing",
  "FrmNavGetFocusRingInfo",
  "FrmNavObjectTakeFocus"
};

static void logtrap_hook(logtrap_t *lt, uint32_t pc);
static void logtrap_hook2(logtrap_t *lt, uint32_t pc);
static void logtrap_rethook(logtrap_t *lt, uint32_t pc);

static char *event_name(uint16_t eType) {
  if (eType <= lastRegularEvent) {
    return eventName[eType];
  }

  return NULL;
}

static char hex(uint8_t n) {
  n &= 0x0F;
  return n < 10 ? '0' + n : 'A' + n - 10;
}

static void logtrap_log(logtrap_t *lt, char *fmt, ...) {
  char tmp[1024], buf[1024];
  uint32_t i, j;
  va_list ap;

  va_start(ap, fmt);
  vsnprintf(tmp, sizeof(tmp)-1, fmt, ap);
  va_end(ap);

  for (i = 0, j = 0; tmp[i] && j < sizeof(buf)-5; i++) {
    if (tmp[i] >= 32) {
      buf[j++] = tmp[i];
    } else if (tmp[i+1]) {
      buf[j++] = '<';
      buf[j++] = hex((tmp[i] >> 4) & 0x0F);
      buf[j++] = hex(tmp[i] & 0x0F);
      buf[j++] = '>';
    }
  }
  buf[j] = 0;

  lt->print(buf, lt->data);
}

int logtrap_global_init(logtrap_def *def) {
  uint32_t trap, selector, i;

  def->hook = logtrap_hook;
  def->hook2 = logtrap_hook2;
  def->rethook = logtrap_rethook;

  for (i = 0; i < 0x1000; i++) {
    def->allTraps[i].name = "unknown";
  }

  for (i = 0; trapArgs[i].name; i++) {
    trap = trapArgs[i].trap;
    if (trap >= 0xA000 && trap < 0xB000) {
      trap -= 0xA000;
      selector = trapArgs[i].selector;
      if (selector == (uint32_t)-1) {
        def->allTraps[trap] = trapArgs[i];
      } else {
        if (def->allTraps[trap].trap == 0) {
          def->allTraps[trap].trap = trap;
          def->allTraps[trap].name = "dispatch";
          def->allTraps[trap].capsel = selector < 64 ? 64 : selector + 64;
          def->allTraps[trap].numsel = 1;
          if (selector > def->allTraps[trap].maxsel) def->allTraps[trap].maxsel = selector;
          def->allTraps[trap].selectors = (trap_t *)def->alloc(def->allTraps[trap].capsel * sizeof(trap_t), def->data);
          def->allTraps[trap].selectors[selector] = trapArgs[i];
        } else {
          if (selector >= def->allTraps[trap].capsel) {
            def->allTraps[trap].capsel = selector + 64;
            if (def->allTraps[trap].selectors) {
              def->allTraps[trap].selectors = (trap_t *)def->realloc(def->allTraps[trap].selectors, def->allTraps[trap].capsel * sizeof(trap_t), def->data);
            } else {
              def->allTraps[trap].selectors = (trap_t *)def->alloc(def->allTraps[trap].capsel * sizeof(trap_t), def->data);
            }
          }
          def->allTraps[trap].selectors[selector] = trapArgs[i];
          def->allTraps[trap].numsel++;
          if (selector > def->allTraps[trap].maxsel) def->allTraps[trap].maxsel = selector;
        }
      }
    }
  }

  return 0;
}

void logtrap_global_finish(logtrap_def *def) {
  uint32_t i;

  if (def) {
    for (i = 0; i < 0x1000; i++) {
      if (def->allTraps[i].selectors) {
        def->free(def->allTraps[i].selectors, def->data);
      }
    }
  }
}

logtrap_t *logtrap_init(logtrap_def *def) {
  logtrap_t *lt = NULL;

  if (def != NULL && (lt = def->alloc(sizeof(logtrap_t), def->data)) != NULL) {
    lt->alloc = def->alloc;
    lt->realloc = def->realloc;
    lt->free = def->free;
    lt->print = def->print;
    lt->read8 = def->read8;
    lt->read16 = def->read16;
    lt->read32 = def->read32;
    lt->getreg = def->getreg;
    lt->data = def->data;

    lt->allTraps = def->allTraps;

    lt->dis.read16 = def->read16;
    lt->dis.read32 = def->read32;
    lt->dis.data = def->data;
  }

  return lt;
}

void logtrap_start(logtrap_t *lt, int disasm, char *appname) {
  if (lt) {
    lt->disasm = disasm;

    if (appname) {
      lt->appname = strdup(appname);
      lt->log_f = 0;
    } else {
      lt->log_f = 1;
    }
  }
}

int logtrap_started(logtrap_t *lt) {
  return lt ? lt->log_f : 0;
}

void logtrap_finish(logtrap_t *lt) {
  if (lt) {
    if (lt->appname) lt->free(lt->appname, lt->data);
    lt->free(lt, lt->data);
  }
}

static char *id2s(uint32_t ID, char *s) {
  creator_id_t id;

  id.t = ID;
  s[0] = id.c[3];
  s[1] = id.c[2];
  s[2] = id.c[1];
  s[3] = id.c[0];
  s[4] = 0;

  return s;
}

static char *param_value(logtrap_t *lt, uint32_t type, uint32_t ptr, uint32_t size, uint32_t io, uint32_t value, uint32_t value0, char *aux, uint32_t len, int isret) {
  char str[128], sid[8], *ename;
  uint8_t red, green, blue;
  uint16_t etype, width, height, depth, density, version, printarg, isize;
  int16_t sec, min, hour, day, mon, year;
  int16_t x, y, dx, dy;
  uint32_t j;
  int32_t sig;
  uint32_t usig;

  if (ptr) {
    if (value) {
      printarg = (isret && !(io & OMIT_OUT)) || (!isret && !(io & OMIT_IN));
      switch (size) {
        case 1: isize = 8; break;
        case 2: isize = 16; break;
        case 4: isize = 32; break;
        case 8: isize = 64; break;
        default: isize = 0; // should not happen
      }

      switch (type) {
        case T_VOID:
          snprintf(aux, len - 1, "ptr_%08X", value);
          break;
        case T_STR:
          if (printarg) {
            for (j = 0; j < sizeof(str) - 1; j++) {
              str[j] = lt->read8(value + j, lt->data);
              if (str[j] == 0) break;
            }
            str[j] = 0;
            snprintf(aux, len - 1, "\"%s\"", str);
          } else {
            snprintf(aux, len - 1, "str_%08X", value);
          }
          break;
        case T_RGB:
          if (printarg) {
            red   = lt->read8(value + 1, lt->data);
            green = lt->read8(value + 2, lt->data);
            blue  = lt->read8(value + 3, lt->data);
            snprintf(aux, len - 1, "rgb{ptr_%08X %d,%d,%d}", value, red, green, blue);
          } else {
            snprintf(aux, len - 1, "rgb{ptr_%08X}", value);
          }
        break;
        case T_EVT:
          if (printarg) {
            etype = lt->read16(value, lt->data);
            ename = event_name(etype);
            if (ename) {
              snprintf(aux, len - 1, "event{ptr_%08X %d 0x%04X %s}", value, etype, etype, ename);
            } else {
              snprintf(aux, len - 1, "event{ptr_%08X %d 0x%04X}", value, etype, etype);
            }
          } else {
            snprintf(aux, len - 1, "event{ptr_%08X}", value);
          }
          break;
        case T_RCT:
          if (printarg) {
            x = lt->read16(value, lt->data);
            y = lt->read16(value + 2, lt->data);
            dx = lt->read16(value + 4, lt->data);
            dy = lt->read16(value + 6, lt->data);
            snprintf(aux, len - 1, "rect{ptr_%08X %d,%d,%d,%d}", value, x, y, dx, dy);
          } else {
            snprintf(aux, len - 1, "rect{ptr_%08X}", value);
          }
          break;
        case T_DATE:
          if (printarg) {
            sec  = lt->read16(value,      lt->data);
            min  = lt->read16(value +  2, lt->data);
            hour = lt->read16(value +  4, lt->data);
            day  = lt->read16(value +  6, lt->data);
            mon  = lt->read16(value +  8, lt->data);
            year = lt->read16(value + 10, lt->data);
            //wday = lt->read16(value + 12, lt->data);
            snprintf(aux, len - 1, "date{ptr_%08X %04d-%02d-%02d %02d:%02d:%02d}", value, year, mon, day, hour, min, sec);
          } else {
            snprintf(aux, len - 1, "date{ptr_%08X}", value);
          }
          break;
        case T_BMP:
          if (printarg) {
            width   = lt->read16(value    , lt->data);
            height  = lt->read16(value + 2, lt->data);
            version = lt->read8(value  + 9, lt->data);
            depth   = version == 0 ? 1 : lt->read8(value + 8, lt->data);
            density = version < 3 ? 72 : lt->read16(value + 14, lt->data);
            snprintf(aux, len - 1, "bmp{ptr_%08X V%u %ub %ux%u %u}", value, version, depth, width, height, density);
          } else {
            snprintf(aux, len - 1, "bmp{ptr_%08X}", value);
          }
          break;
        case T_SIG:
          if (printarg) {
            sig = 0;
            if (!(value % 2))
            switch (size) {
              case 1: sig = (int8_t)(lt->read8(value, lt->data) & 0xFF); break;
              case 2: sig = (int16_t)(lt->read16(value, lt->data) & 0xFFFF); break;
              case 4: sig = (int32_t)lt->read32(value, lt->data); break;
              default: sig = 0; break;
            }
            snprintf(aux, len - 1, "int%d{ptr_%08X %d}", isize, value, sig);
          } else {
            snprintf(aux, len - 1, "int%d{ptr_%08X}", isize, value);
          }
          break;
        case T_USIG:
          if (printarg) {
            usig = 0;
            if (!(value % 2))
            switch (size) {
              case 1: usig = lt->read8(value, lt->data); break;
              case 2: usig = lt->read16(value, lt->data); break;
              case 4: usig = lt->read32(value, lt->data); break;
              default: usig = 0; break;
            }
            snprintf(aux, len - 1, "uint%d{ptr_%08X %u}", isize, value, usig);
          } else {
            snprintf(aux, len - 1, "uint%d{ptr_%08X}", isize, value);
          }
          break;
        case T_HEX:
          if (printarg) {
            usig = 0;
            if (!(value % 2))
            switch (size) {
              case 1:
                usig = lt->read8(value, lt->data);
                snprintf(aux, len - 1, "uint8{ptr_%08X 0x%02X}", value, usig);
                break;
              case 2:
                usig = lt->read16(value, lt->data);
                snprintf(aux, len - 1, "uint16{ptr_%08X 0x%04X}", value, usig);
                break;
              case 4:
                usig = lt->read32(value, lt->data);
                snprintf(aux, len - 1, "uint32{ptr_%08X 0x%08X}", value, usig);
                break;
            }
          } else {
            snprintf(aux, len - 1, "uint%d{ptr_%08X}", isize, value);
          }
          break;
        case T_WCHR:
          if (printarg) {
            usig = 0;
            if (!(value % 2))
            usig = lt->read16(value, lt->data);
            snprintf(aux, len - 1, "wchar{ptr_%08X 0x%04X}", value, usig);
          } else {
            snprintf(aux, len - 1, "wchar{ptr_%08X}", value);
          }
          break;
        case T_ID:
          if (printarg) {
            usig = 0;
            if (!(value % 2))
            usig = lt->read32(value, lt->data);
            id2s(usig, sid);
            snprintf(aux, len - 1, "id{ptr_%08X '%s'}", value, sid);
          } else {
            snprintf(aux, len - 1, "id{ptr_%08X}", value);
          }
          break;
        case T_LOC:
          if (printarg) {
            usig = lt->read32(value, lt->data);
            snprintf(aux, len - 1, "localid{ptr_%08X localid_%08X}", value, usig);
          } else {
            snprintf(aux, len - 1, "localid{ptr_%08X}", value);
          }
          break;
        default:
          snprintf(aux, len - 1, "type_%u", type);
          break;
      }
    } else {
      snprintf(aux, len - 1, "NULL");
    }
  } else {
    switch (type) {
      case T_SIG:
        switch (size) {
          case 1: sig = (int8_t)(value & 0xFF); break;
          case 2: sig = (int16_t)(value & 0xFFFF); break;
          case 4: sig = (int32_t)value; break;
          default: sig = 0; break;
        }
        snprintf(aux, len - 1, "%d", sig);
        break;
      case T_USIG:
        if (size == 8) {
          snprintf(aux, len - 1, "double_%08X_%08X", value, value0);
        } else {
          snprintf(aux, len - 1, "%u", value);
        }
        break;
      case T_CHAR:
        if (value >= 0x20 && value < 0x7F) {
          snprintf(aux, len - 1, "'%c'", (char)value);
        } else {
          snprintf(aux, len - 1, "0x%02X", value);
        }
        break;
      case T_WCHR:
        if (value >= 0x20 && value < 0x7F) {
          snprintf(aux, len - 1, "'%c'", (char)value);
        } else {
          snprintf(aux, len - 1, "0x%04X", value);
        }
        break;
      case T_ID:
        id2s(value, sid);
        snprintf(aux, len - 1, "'%s'", sid);
        break;
      case T_LOC:
        snprintf(aux, len - 1, "localid_%08X", value);
        break;
      case T_HEX:
        switch (size) {
          case 1: snprintf(aux, len - 1, "0x%02X", value); break;
          case 2: snprintf(aux, len - 1, "0x%04X", value); break;
          case 4: snprintf(aux, len - 1, "0x%08X", value); break;
        }
        break;
    }
  }

  return aux;
}

static char *spaces(uint32_t n) {
  static char buf[256];
  uint32_t i;

  for (i = 0; i < n && i < 256; i++) {
    buf[i] = ' ';
  }
  buf[i] = 0;

  return buf;
}

static void print_params(logtrap_t *lt, trap_t *trap, uint32_t sp, char *buf, uint32_t len, int isret) {
  uint32_t value, value0, idx, i;
  char aux[256];

  idx = 0;
  buf[0] = 0;

  for (i = 0; i < trap->nargs; i++) {
    value0 = 0;
    if (trap->args[i].ptr) {
      value = lt->read32(sp + idx, lt->data);
      idx += 4;
    } else {
      switch (trap->args[i].size) {
        case 1:
          value = lt->read8(sp + idx, lt->data) & 0xFF;
          idx += 2;
          break;
        case 2:
          value = lt->read16(sp + idx, lt->data) & 0xFFFF;
          idx += 2;
          break;
        case 4:
          value = lt->read32(sp + idx, lt->data);
          idx += 4;
          break;
        case 8:
          value = lt->read32(sp + idx, lt->data);
          idx += 4;
          value0 = lt->read32(sp + idx, lt->data);
          idx += 4;
          break;
        default:
          value = 0;
          break;
      }
    }
    param_value(lt, trap->args[i].type, trap->args[i].ptr, trap->args[i].size, trap->args[i].io, value, value0, aux, sizeof(aux), isret);
    if (i > 0) strncat(buf, ", ", len - strlen(buf) - 1);
    strncat(buf, aux, len - strlen(buf) - 1);
  }
}

void logtrap_rethook(logtrap_t *lt, uint32_t pc) {
  char buf[1024], rbuf[256];
  uint32_t sp, name, value, value0, addr, selector;
  uint32_t rtype, rsize, rptr;
  uint16_t trap, rtrap, i;
  char *s;

  if (lt->stackp && pc == lt->stack[lt->stackp-1]) {
    sp = lt->getreg(logtrap_SP, lt->data);
    value = 0;
    lt->stackp--;
    trap = lt->stackt[lt->stackp];
    if (trap >= 0xA000 && trap < 0xB000) {
      rtrap = trap - 0xA000;
      selector = lt->stacksel[lt->stackp];
      rbuf[0] = 0;

      if (lt->allTraps[rtrap].numsel == 0 || selector >= lt->allTraps[rtrap].maxsel) {
        rtype = lt->allTraps[rtrap].rtype;
        rsize = lt->allTraps[rtrap].rsize;
        rptr = lt->allTraps[rtrap].rptr;
      } else {
        rtype = lt->allTraps[rtrap].selectors[selector].rtype;
        rsize = lt->allTraps[rtrap].selectors[selector].rsize;
        rptr = lt->allTraps[rtrap].selectors[selector].rptr;
      }

      if (rsize > 0) {
        if (lt->log_f) strcpy(rbuf, ": ");
        if (rtype == T_VOID || rtype == T_STR || rtype == T_BMP) {
          value = lt->getreg(logtrap_A0, lt->data);
        } else {
          value = lt->getreg(logtrap_D0, lt->data);
        }

        value0 = 0;
        switch (rsize) {
          case 1:
            value &= 0xFF;
            break;
          case 2:
            value &= 0xFFFF;
            break;
          case 8:
            addr = lt->getreg(logtrap_A0, lt->data);
            value  = lt->read32(addr, lt->data);
            value0 = lt->read32(addr+4, lt->data);
            break;
        }

        if (lt->log_f) {
          param_value(lt, rtype, rptr, rsize, 0, value, value0, &rbuf[2], sizeof(rbuf) - 2, 1);
        }
      }

      if (lt->log_f) {
        if (lt->allTraps[rtrap].numsel == 0) {
          if (lt->allTraps[rtrap].rsize == 8) sp += 4;
          print_params(lt, &lt->allTraps[rtrap], sp, buf, sizeof(buf), 1);
          logtrap_log(lt, "%08X: trap 0x%04X    %s%s(%s)%s", pc-4, trap, spaces(lt->stackp), lt->allTraps[rtrap].name, buf, rbuf);
        } else if (selector < lt->allTraps[rtrap].maxsel) {
          if (lt->allTraps[rtrap].selectors[selector].rsize == 8) sp += 4;
          print_params(lt, &lt->allTraps[rtrap].selectors[selector], sp, buf, sizeof(buf), 1);
          s = lt->allTraps[rtrap].selectors[selector].name;
          if (s == NULL) s = "unknown";
          logtrap_log(lt, "%08X: trap 0x%04X.%-2d %s%s(%s)%s", pc-4, trap, selector, spaces(lt->stackp), s, buf, rbuf);
        } else {
          logtrap_log(lt, "%08X: trap 0x%04X.%-2d %sunknown()", pc-4, trap, selector, spaces(lt->stackp));
        }
      }
    }

    switch (trap) {
      case sysTrapDmDatabaseInfo:
        // Err DmDatabaseInfo(UInt16 cardNo, LocalID dbID, Char *nameP, ...
        if (lt->appname && lt->log_f == 0 && lt->log_dbID == 0 && value == 0) {
          name = lt->read32(sp + 6, lt->data);
          if (name) {
            for (i = 0; i < sizeof(buf) - 1; i++) {
              buf[i] = lt->read8(name + i, lt->data);
              if (buf[i] == 0) break;
            }
            buf[i] = 0;
            if (strcmp(buf, lt->appname) == 0) {
              lt->log_dbID = lt->read32(sp + 2, lt->data);
              logtrap_log(lt, "Monitoring dbID 0x%08X for \"%s\"", lt->log_dbID, lt->appname);
            }
          }
        }
        break;
      case sysTrapDmOpenDatabase:
        // DmOpenRef DmOpenDatabase(UInt16 cardNo, LocalID dbID, UInt16 mode)
        if (lt->appname && lt->log_f == 0 && lt->log_dbRef == 0 && lt->log_dbID != 0 && value != 0) {
          if (lt->read32(sp + 2, lt->data) == lt->log_dbID) {
            lt->log_dbRef = value;
            logtrap_log(lt, "Monitoring dbRef 0x%08X for dbID 0x%08X", lt->log_dbRef, lt->log_dbID);
          }
        }
        break;
      case sysTrapSysAppStartup:
        // Err SysAppStartup(SysAppInfoPtr *appInfoPP, MemPtr *prevGlobalsP, MemPtr *globalsPtrP)
        if (lt->appname && lt->log_f == 0 && lt->log_dbRef != 0 && value == 0) {
          value = lt->read32(sp, lt->data);    // SysAppInfoType **
          value = lt->read32(value, lt->data); // SysAppInfoType *
          if (lt->read32(value + 16, lt->data) == lt->log_dbRef) {
            logtrap_log(lt, "Logging system calls for dbRef 0x%08X dbID 0x%08X", lt->log_dbRef, lt->log_dbID);
            lt->log_f = 1;
          }
        }
        break;
      case sysTrapSysAppExit:
        if (lt->log_f) {
          logtrap_log(lt, "Stop logging system calls for dbRef 0x%08X dbID 0x%08X", lt->log_dbRef, lt->log_dbID);
          lt->log_dbID = 0;
          lt->log_dbRef = 0;
          lt->log_f = 0;
        }
        break;
    }
  }
}

static void hex_opcodes(logtrap_t *lt, char *buf, uint32_t pc, uint32_t length) {
  char *ptr = buf;

  for (; length > 0; length -= 2) {
    sprintf(ptr, "%04x", lt->read16(pc, lt->data));
    pc += 2;
    ptr += 4;
    if (length > 2) *ptr++ = ' ';
  }
}

static void logtrap_hook(logtrap_t *lt, uint32_t pc) {
  uint32_t instruction, sp, selector;
  char *s, *spc, *elp, buf[1024];
  uint16_t trap, rtrap;
  int standalone;

  if ((pc & 0xFF000000) == 0xFF000000) {
    trap = pc & 0xFFFF;
    pc = 0;
    standalone = 1;
  } else {
    instruction = lt->read16(pc, lt->data);
    // check if it is a trap instruction
    if (instruction != 0x4E4F) return;
    trap = lt->read16(pc + 2, lt->data);
    standalone = 0;
  }

  sp = lt->getreg(logtrap_SP, lt->data);

  switch (trap) {
    case sysTrapHwrDisableDataWrites:
    case sysTrapHwrEnableDataWrites:
    case sysTrapHwrDoze:
    case sysTrapHwrDelay:
    case sysTrapHwrDockStatus:
    case sysTrapSysDoze:
    case sysTrapSysTimerWrite:
    case sysTrapSysEvGroupSignal:
    case sysTrapSysEvGroupWait:
    case sysTrapSysDisableInts:
    case sysTrapSysRestoreStatus:
    case sysTrapSysResSemaphoreReserve:
    case sysTrapSysResSemaphoreRelease:
    case sysTrapSysSemaphoreWait:
    case sysTrapSysSemaphoreSignal:
    case sysTrapSysTaskSwitching:
    case sysTrapSysGetAppInfo:
    case sysTrapMemSemaphoreReserve:
    case sysTrapMemSemaphoreRelease:
    case sysTrapAlmDisplayAlarm:
    case sysTrapAttnDoEmergencySpecialEffects:
    case sysTrapEvtDequeueKeyEvent:
    case sysTrapEvtGetSysEvent:
    case sysTrapHwrIRQ6Handler:
    case sysTrapSysKernelClockTick:
      break;

    default:
      selector = lt->getreg(logtrap_D0 + 2, lt->data);
      if (trap >= 0xA000 && trap < 0xB000) {
        rtrap = trap - 0xA000;
        if (lt->log_f) {
          spc = standalone ? "" : spaces(lt->stackp);
          elp = standalone ? "" : " ...";
          if (lt->allTraps[rtrap].numsel == 0) {
            if (lt->allTraps[rtrap].rsize == 8) sp += 4;
            print_params(lt, &lt->allTraps[rtrap], sp, buf, sizeof(buf), 0);
            logtrap_log(lt, "%08X: trap 0x%04X    %s%s(%s)%s", pc, trap, spc, lt->allTraps[rtrap].name, buf, elp);
          } else if (selector < lt->allTraps[rtrap].maxsel) {
            if (lt->allTraps[rtrap].selectors[selector].rsize == 8) sp += 4;
            print_params(lt, &lt->allTraps[rtrap].selectors[selector], sp, buf, sizeof(buf), 0);
            s = lt->allTraps[rtrap].selectors[selector].name;
            if (s == NULL) s = "unknown";
            logtrap_log(lt, "%08X: trap 0x%04X.%-2d %s%s(%s)%s ", pc, trap, selector, spc, s, buf, elp);
          } else {
            logtrap_log(lt, "%08X: trap 0x%04X.%-2d %sunknown()%s", pc, trap, selector, spc, elp);
          }
        }
      }
      if (!standalone) {
        lt->stackt[lt->stackp] = trap;
        lt->stacksel[lt->stackp] = selector;
        lt->stack[lt->stackp++] = pc + 4;
      }
      break;
  }
}

static void logtrap_hook2(logtrap_t *lt, uint32_t pc) {
  uint32_t instr_size, used, r, d, i;
  char buf[1024], buf2[128], aux[32], buf3[256];

  if (lt->disasm) {
    instr_size = m68k_disasm(&lt->dis, buf, pc, M68K_CPU_TYPE_68020, &used);
    hex_opcodes(lt, buf2, pc, instr_size);
    buf3[0] = 0;

    if (used) {
      strcpy(buf3, " (");
      for (i = 0; i < 8; i++) {
        r = logtrap_D0 + i;
        if (used & (1 << r)) {
          d = lt->getreg(r, lt->data);
          snprintf(aux, sizeof(aux)-1, " D%u=%d 0x%08X", i, d, d);
          strncat(buf3, aux, sizeof(buf3) - strlen(buf3) - 1);
        }
      }
      for (i = 0; i < 8; i++) {
        r = logtrap_A0 + i;
        if (used & (1 << r)) {
          snprintf(aux, sizeof(aux)-1, " A%u=0x%08X", i, lt->getreg(r, lt->data));
          strncat(buf3, aux, sizeof(buf3) - strlen(buf3) - 1);
        }
      }
      strncat(buf3, " )", sizeof(buf3) - strlen(buf3) - 1);
    }

    logtrap_log(lt, "%08X: %-20s: %s%s", pc, buf2, buf, buf3);
  }
}

char *logtrap_trapname(logtrap_t *lt, uint16_t trap, uint16_t *selector, int follow) {
  char *name = "unknown";
  uint32_t rtrap, d2, sp;
  uint16_t aux;

  *selector = 0xFFFF;

  if (trap >= 0xA000 && trap < 0xB000) {
    rtrap = trap - 0xA000;
    if (lt->allTraps[rtrap].numsel > 0) {
      d2 = lt->getreg(logtrap_D0 + 2, lt->data);
      if (d2 <= lt->allTraps[rtrap].maxsel) {
        name = follow ? lt->allTraps[rtrap].selectors[d2].name : lt->allTraps[rtrap].name;
        *selector = d2;
      }
    } else if (trap == sysTrapNavSelector) {
      sp = lt->getreg(logtrap_SP, lt->data);
      aux = lt->read16(sp, lt->data);
      if (aux <= NavSelectorFrmNavObjectTakeFocus) {
        name = follow ? nav_traps[aux] : lt->allTraps[rtrap].name;
        *selector = aux;
      }
    } else {
      if (lt->allTraps[rtrap].name) name = lt->allTraps[rtrap].name;
    }
  }

  return name;
}
