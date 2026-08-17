
    case sysTrapPceNativeCall:
      palmos_PceSysTrap(sp, idx, trap);
      break;
    case sysTrapHwrGetROMToken:
      palmos_HwrSysTrap(sp, idx, trap);
      break;
    case sysTrapHostControl:
      palmos_HostSysTrap(sp, idx, trap);
      break;
    case sysTrapResLoadConstant:
      palmos_ResSysTrap(sp, idx, trap);
      break;
    case sysTrapAttnListOpen:
    case sysTrapAttnIndicatorEnable:
    case sysTrapAttnIterate:
      palmos_AttnSysTrap(sp, idx, trap);
      break;
    case sysTrapDlkGetSyncInfo:
      palmos_DlkSysTrap(sp, idx, trap);
      break;
    case sysTrapErrDisplayFileLineMsg:
    case sysTrapErrExceptionList:
    case sysTrapErrThrow:
    case sysTrapErrSetJump:
    case sysTrapErrLongJump:
    case sysTrapErrAlertCustom:
      palmos_ErrSysTrap(sp, idx, trap);
      break;
    case sysTrapFileControl:
    case sysTrapFileOpen:
    case sysTrapFileClose:
    case sysTrapFileDelete:
    case sysTrapFileReadLow:
    case sysTrapFileWrite:
    case sysTrapFileSeek:
    case sysTrapFileTell:
    case sysTrapFileTruncate:
      palmos_FileSysTrap(sp, idx, trap);
      break;
    case sysTrapFtrPtrNew:
    case sysTrapFtrPtrFree:
    case sysTrapFtrUnregister:
    case sysTrapFtrGet:
    case sysTrapFtrSet:
      palmos_FtrSysTrap(sp, idx, trap);
      break;
    case sysTrapSelectOneTime:
    case sysTrapSelectDay:
      palmos_SelectSysTrap(sp, idx, trap);
      break;
    case sysTrapDaysInMonth:
      palmos_DaysSysTrap(sp, idx, trap);
      break;
    case sysTrapDayOfWeek:
      palmos_DaySysTrap(sp, idx, trap);
      break;
    case sysTrapDateSecondsToDate:
    case sysTrapDateToDOWDMFormat:
    case sysTrapDateToAscii:
    case sysTrapDateToDays:
    case sysTrapDateDaysToDate:
    case sysTrapDateAdjust:
    case sysTrapDateTemplateToAscii:
      palmos_DateSysTrap(sp, idx, trap);
      break;
    case sysTrapTimSetSeconds:
    case sysTrapTimAdjust:
    case sysTrapTimDateTimeToSeconds:
    case sysTrapTimSecondsToDateTime:
    case sysTrapTimGetSeconds:
    case sysTrapTimGetTicks:
      palmos_TimSysTrap(sp, idx, trap);
      break;
    case sysTrapTimeToAscii:
    case sysTrapTimeZoneToAscii:
      palmos_TimeSysTrap(sp, idx, trap);
      break;
    case sysTrapFplInit:
    case sysTrapFplFree:
      palmos_FplSysTrap(sp, idx, trap);
      break;
    case sysTrapWinScreenMode:
    case sysTrapWinPalette:
    case sysTrapWinCreateWindow:
    case sysTrapWinCreateBitmapWindow:
    case sysTrapWinCreateOffscreenWindow:
    case sysTrapWinDeleteWindow:
    case sysTrapWinValidateHandle:
    case sysTrapWinInitializeWindow:
    case sysTrapWinAddWindow:
    case sysTrapWinRemoveWindow:
    case sysTrapWinSetActiveWindow:
    case sysTrapWinSetDrawWindow:
    case sysTrapWinGetDrawWindow:
    case sysTrapWinGetActiveWindow:
    case sysTrapWinGetDisplayWindow:
    case sysTrapWinGetFirstWindow:
    case sysTrapWinEnableWindow:
    case sysTrapWinDisableWindow:
    case sysTrapWinGetWindowFrameRect:
    case sysTrapWinDrawWindowFrame:
    case sysTrapWinEraseWindow:
    case sysTrapWinSaveBits:
    case sysTrapWinRestoreBits:
    case sysTrapWinCopyRectangle:
    case sysTrapWinScrollRectangle:
    case sysTrapWinGetDisplayExtent:
    case sysTrapWinGetDrawWindowBounds:
    case sysTrapWinGetBounds:
    case sysTrapWinSetBounds:
    case sysTrapWinGetWindowExtent:
    case sysTrapWinDisplayToWindowPt:
    case sysTrapWinWindowToDisplayPt:
    case sysTrapWinGetBitmap:
    case sysTrapWinGetClip:
    case sysTrapWinSetClip:
    case sysTrapWinResetClip:
    case sysTrapWinClipRectangle:
    case sysTrapWinModal:
    case sysTrapWinGetPixel:
    case sysTrapWinGetPixelRGB:
    case sysTrapWinPaintPixel:
    case sysTrapWinPaintPixels:
    case sysTrapWinDrawPixel:
    case sysTrapWinErasePixel:
    case sysTrapWinInvertPixel:
    case sysTrapWinPaintLine:
    case sysTrapWinDrawLine:
    case sysTrapWinDrawGrayLine:
    case sysTrapWinEraseLine:
    case sysTrapWinInvertLine:
    case sysTrapWinFillLine:
    case sysTrapWinPaintRectangle:
    case sysTrapWinDrawRectangle:
    case sysTrapWinEraseRectangle:
    case sysTrapWinInvertRectangle:
    case sysTrapWinFillRectangle:
    case sysTrapWinPaintRectangleFrame:
    case sysTrapWinDrawRectangleFrame:
    case sysTrapWinDrawGrayRectangleFrame:
    case sysTrapWinEraseRectangleFrame:
    case sysTrapWinInvertRectangleFrame:
    case sysTrapWinDrawBitmap:
    case sysTrapWinPaintBitmap:
    case sysTrapWinDrawChar:
    case sysTrapWinDrawChars:
    case sysTrapWinPaintChar:
    case sysTrapWinPaintChars:
    case sysTrapWinDrawInvertedChars:
    case sysTrapWinDrawTruncChars:
    case sysTrapWinEraseChars:
    case sysTrapWinInvertChars:
    case sysTrapWinSetUnderlineMode:
    case sysTrapWinPushDrawState:
    case sysTrapWinPopDrawState:
    case sysTrapWinSetDrawMode:
    case sysTrapWinSetForeColor:
    case sysTrapWinSetBackColor:
    case sysTrapWinSetTextColor:
    case sysTrapWinSetForeColorRGB:
    case sysTrapWinSetBackColorRGB:
    case sysTrapWinSetTextColorRGB:
    case sysTrapWinGetPattern:
    case sysTrapWinGetPatternType:
    case sysTrapWinSetPattern:
    case sysTrapWinSetPatternType:
    case sysTrapWinRGBToIndex:
    case sysTrapWinIndexToRGB:
    case sysTrapWinSetColors:
    case sysTrapWinScreenInit:
    case sysTrapWinScreenLock:
    case sysTrapWinScreenUnlock:
      palmos_WinSysTrap(sp, idx, trap);
      break;
    case sysTrapFntDefineFont:
    case sysTrapFntGetFont:
    case sysTrapFntSetFont:
    case sysTrapFntGetFontPtr:
    case sysTrapFntBaseLine:
    case sysTrapFntCharHeight:
    case sysTrapFntLineHeight:
    case sysTrapFntAverageCharWidth:
    case sysTrapFntCharWidth:
    case sysTrapFntWCharWidth:
    case sysTrapFntCharsWidth:
    case sysTrapFntWidthToOffset:
    case sysTrapFntCharsInWidth:
    case sysTrapFntDescenderHeight:
    case sysTrapFntLineWidth:
    case sysTrapFntWordWrap:
    case sysTrapFntWordWrapReverseNLines:
    case sysTrapFntGetScrollValues:
      palmos_FntSysTrap(sp, idx, trap);
      break;
    case sysTrapRctSetRectangle:
    case sysTrapRctInsetRectangle:
    case sysTrapRctOffsetRectangle:
    case sysTrapRctCopyRectangle:
    case sysTrapRctPtInRectangle:
    case sysTrapRctGetIntersection:
      palmos_RctSysTrap(sp, idx, trap);
      break;
    case sysTrapBmpCreate:
    case sysTrapBmpDelete:
    case sysTrapBmpCompress:
    case sysTrapBmpGetBits:
    case sysTrapBmpGetColortable:
    case sysTrapBmpSize:
    case sysTrapBmpBitsSize:
    case sysTrapBmpGetSizes:
    case sysTrapBmpColortableSize:
    case sysTrapBmpGetDimensions:
    case sysTrapBmpGetBitDepth:
    case sysTrapBmpGetNextBitmap:
      palmos_BmpSysTrap(sp, idx, trap);
      break;
    case sysTrapSecSelectViewStatus:
      palmos_SecSysTrap(sp, idx, trap);
      break;
    case sysTrapFontSelect:
      palmos_FontSysTrap(sp, idx, trap);
      break;
    case sysTrapUIColorPushTable:
    case sysTrapUIColorPopTable:
    case sysTrapUIColorSetTableEntry:
    case sysTrapUIColorGetTableEntryRGB:
    case sysTrapUIColorGetTableEntryIndex:
    case sysTrapUIPickColor:
    case sysTrapUIBrightnessAdjust:
    case sysTrapUIContrastAdjust:
      palmos_UISysTrap(sp, idx, trap);
      break;
    case sysTrapPrefGetPreferences:
    case sysTrapPrefSetPreferences:
    case sysTrapPrefGetPreference:
    case sysTrapPrefSetPreference:
    case sysTrapPrefOpenPreferenceDB:
    case sysTrapPrefOpenPreferenceDBV10:
    case sysTrapPrefSetAppPreferences:
    case sysTrapPrefSetAppPreferencesV10:
    case sysTrapPrefGetAppPreferences:
    case sysTrapPrefGetAppPreferencesV10:
      palmos_PrefSysTrap(sp, idx, trap);
      break;
    case sysTrapMemSet:
    case sysTrapMemMove:
    case sysTrapMemHandleLock:
    case sysTrapMemInit:
    case sysTrapMemKernelInit:
    case sysTrapMemInitHeapTable:
    case sysTrapMemNumCards:
    case sysTrapMemCardInfo:
    case sysTrapMemNumHeaps:
    case sysTrapMemNumRAMHeaps:
    case sysTrapMemHeapID:
    case sysTrapMemHeapDynamic:
    case sysTrapMemHeapFreeBytes:
    case sysTrapMemHeapSize:
    case sysTrapMemHeapFlags:
    case sysTrapMemHeapCompact:
    case sysTrapMemHeapInit:
    case sysTrapMemHeapFreeByOwnerID:
    case sysTrapMemChunkNew:
    case sysTrapMemChunkFree:
    case sysTrapMemPtrNew:
    case sysTrapMemPtrRecoverHandle:
    case sysTrapMemPtrFlags:
    case sysTrapMemPtrSize:
    case sysTrapMemPtrOwner:
    case sysTrapMemPtrHeapID:
    case sysTrapMemPtrDataStorage:
    case sysTrapMemPtrCardNo:
    case sysTrapMemPtrToLocalID:
    case sysTrapMemPtrSetOwner:
    case sysTrapMemPtrResize:
    case sysTrapMemPtrResetLock:
    case sysTrapMemPtrUnlock:
    case sysTrapMemHandleNew:
    case sysTrapMemHandleFree:
    case sysTrapMemHandleFlags:
    case sysTrapMemHandleSize:
    case sysTrapMemHandleOwner:
    case sysTrapMemHandleLockCount:
    case sysTrapMemHandleHeapID:
    case sysTrapMemHandleDataStorage:
    case sysTrapMemHandleCardNo:
    case sysTrapMemHandleToLocalID:
    case sysTrapMemHandleSetOwner:
    case sysTrapMemHandleResize:
    case sysTrapMemHandleUnlock:
    case sysTrapMemHandleResetLock:
    case sysTrapMemLocalIDToGlobal:
    case sysTrapMemLocalIDKind:
    case sysTrapMemLocalIDToPtr:
    case sysTrapMemLocalIDToLockedPtr:
    case sysTrapMemCmp:
    case sysTrapMemSemaphoreReserve:
    case sysTrapMemSemaphoreRelease:
    case sysTrapMemDebugMode:
    case sysTrapMemSetDebugMode:
    case sysTrapMemHeapScramble:
    case sysTrapMemHeapCheck:
      palmos_MemSysTrap(sp, idx, trap);
      break;
    case sysTrapDmDetachRecord:
    case sysTrapDmDetachResource:
    case sysTrapDmSearchResource:
    case sysTrapDmGetNextDatabaseByTypeCreator:
    case sysTrapDmInsertionSort:
    case sysTrapDmQuickSort:
    case sysTrapDmFindSortPositionV10:
    case sysTrapDmFindSortPosition:
    case sysTrapDmAttachRecord:
    case sysTrapDmSync:
    case sysTrapDmSyncDatabase:
    case sysTrapDmInit:
    case sysTrapDmCreateDatabase:
    case sysTrapDmCreateDatabaseFromImage:
    case sysTrapDmDeleteDatabase:
    case sysTrapDmNumDatabases:
    case sysTrapDmGetDatabase:
    case sysTrapDmFindDatabase:
    case sysTrapDmDatabaseInfo:
    case sysTrapDmSetDatabaseInfo:
    case sysTrapDmDatabaseSize:
    case sysTrapDmDatabaseProtect:
    case sysTrapDmOpenDatabase:
    case sysTrapDmOpenDatabaseByTypeCreator:
    case sysTrapDmOpenDBNoOverlay:
    case sysTrapDmCloseDatabase:
    case sysTrapDmNextOpenDatabase:
    case sysTrapDmOpenDatabaseInfo:
    case sysTrapDmGetAppInfoID:
    case sysTrapDmGetDatabaseLockState:
    case sysTrapDmResetRecordStates:
    case sysTrapDmGetLastErr:
    case sysTrapDmNumRecords:
    case sysTrapDmNumRecordsInCategory:
    case sysTrapDmRecordInfo:
    case sysTrapDmSetRecordInfo:
    case sysTrapDmMoveRecord:
    case sysTrapDmNewRecord:
    case sysTrapDmRemoveRecord:
    case sysTrapDmDeleteRecord:
    case sysTrapDmArchiveRecord:
    case sysTrapDmNewHandle:
    case sysTrapDmRemoveSecretRecords:
    case sysTrapDmFindRecordByID:
    case sysTrapDmQueryRecord:
    case sysTrapDmGetRecord:
    case sysTrapDmQueryNextInCategory:
    case sysTrapDmPositionInCategory:
    case sysTrapDmSeekRecordInCategory:
    case sysTrapDmResizeRecord:
    case sysTrapDmReleaseRecord:
    case sysTrapDmMoveCategory:
    case sysTrapDmDeleteCategory:
    case sysTrapDmWriteCheck:
    case sysTrapDmWrite:
    case sysTrapDmStrCopy:
    case sysTrapDmSet:
    case sysTrapDmGetResource:
    case sysTrapDmGet1Resource:
    case sysTrapDmReleaseResource:
    case sysTrapDmResizeResource:
    case sysTrapDmNextOpenResDatabase:
    case sysTrapDmFindResourceType:
    case sysTrapDmFindResource:
    case sysTrapDmNumResources:
    case sysTrapDmResourceInfo:
    case sysTrapDmSetResourceInfo:
    case sysTrapDmNewResource:
    case sysTrapDmRemoveResource:
    case sysTrapDmGetResourceIndex:
      palmos_DmSysTrap(sp, idx, trap);
      break;
    case sysTrapStrVPrintF:
    case sysTrapStrPrintF:
    case sysTrapStrCopy:
    case sysTrapStrNCopy:
    case sysTrapStrCat:
    case sysTrapStrNCat:
    case sysTrapStrLen:
    case sysTrapStrCompareAscii:
    case sysTrapStrCompare:
    case sysTrapStrNCompareAscii:
    case sysTrapStrNCompare:
    case sysTrapStrCaselessCompare:
    case sysTrapStrNCaselessCompare:
    case sysTrapStrToLower:
    case sysTrapStrIToA:
    case sysTrapStrIToH:
    case sysTrapStrLocalizeNumber:
    case sysTrapStrDelocalizeNumber:
    case sysTrapStrChr:
    case sysTrapStrStr:
    case sysTrapStrAToI:
      palmos_StrSysTrap(sp, idx, trap);
      break;
    case sysTrapFrmNewForm:
    case sysTrapFrmInitForm:
    case sysTrapFrmDeleteForm:
    case sysTrapFrmGetFormId:
    case sysTrapFrmGetFirstForm:
    case sysTrapFrmGetFormPtr:
    case sysTrapFrmGetObjectIndexFromPtr:
    case sysTrapFrmGetActiveField:
    case sysTrapFrmGotoForm:
    case sysTrapFrmUpdateForm:
    case sysTrapFrmDrawForm:
    case sysTrapFrmEraseForm:
    case sysTrapFrmVisible:
    case sysTrapFrmHideObject:
    case sysTrapFrmShowObject:
    case sysTrapFrmGetFocus:
    case sysTrapFrmSetMenu:
    case sysTrapFrmGetTitle:
    case sysTrapFrmCopyTitle:
    case sysTrapFrmSetTitle:
    case sysTrapFrmUpdateScrollers:
    case sysTrapFrmSetActiveForm:
    case sysTrapFrmSetEventHandler:
    case sysTrapFrmGetEventHandler68K:
    case sysTrapFrmSetGadgetHandler:
    case sysTrapFrmGetGadgetData:
    case sysTrapFrmSetGadgetData:
    case sysTrapFrmGetGadgetPtr68K:
    case sysTrapFrmGetWindowHandle:
    case sysTrapFrmGetFormBounds:
    case sysTrapFrmSetObjectBounds:
    case sysTrapFrmGetNumberOfObjects:
    case sysTrapFrmSetObjectPosition:
    case sysTrapFrmGetObjectId:
    case sysTrapFrmGetObjectPosition:
    case sysTrapFrmGetObjectBounds:
    case sysTrapFrmGetControlGroupSelection:
    case sysTrapFrmGetActiveForm:
    case sysTrapFrmGetActiveFormID:
    case sysTrapFrmGetObjectIndex:
    case sysTrapFrmGetObjectPtr:
    case sysTrapFrmGetObjectType:
    case sysTrapFrmGetLabel:
    case sysTrapFrmSetFocus:
    case sysTrapFrmGetControlValue:
    case sysTrapFrmSetControlValue:
    case sysTrapFrmSetControlGroupSelection:
    case sysTrapFrmDispatchEvent:
    case sysTrapFrmHandleEvent:
    case sysTrapFrmCopyLabel:
    case sysTrapFrmSaveAllForms:
    case sysTrapFrmCloseAllForms:
    case sysTrapFrmPopupForm:
    case sysTrapFrmDoDialog:
    case sysTrapFrmReturnToForm:
    case sysTrapFrmHelp:
    case sysTrapFrmCustomAlert:
    case sysTrapFrmCustomResponseAlert:
    case sysTrapFrmAlert:
    case sysTrapFrmNewBitmap:
    case sysTrapFrmNewGadget:
    case sysTrapFrmActiveState:
    case sysTrapFrmNewGsi:
    case sysTrapFrmRemoveObject:
      palmos_FrmSysTrap(sp, idx, trap);
      break;
    case sysTrapAbtShowAbout:
      palmos_AbtSysTrap(sp, idx, trap);
      break;
    case sysTrapCtlNewControl:
    case sysTrapCtlGetStyle68K:
    case sysTrapCtlGetLabel:
    case sysTrapCtlDrawControl:
    case sysTrapCtlEraseControl:
    case sysTrapCtlHideControl:
    case sysTrapCtlShowControl:
    case sysTrapCtlEnabled:
    case sysTrapCtlSetEnabled:
    case sysTrapCtlSetUsable:
    case sysTrapCtlGetValue:
    case sysTrapCtlSetValue:
    case sysTrapCtlSetLabel:
    case sysTrapCtlSetGraphics:
    case sysTrapCtlSetSliderValues:
    case sysTrapCtlGetSliderValues:
    case sysTrapCtlHitControl:
    case sysTrapCtlHandleEvent:
    case sysTrapCtlValidatePointer:
      palmos_CtlSysTrap(sp, idx, trap);
      break;
    case sysTrapLstSetDrawFunction:
    case sysTrapLstDrawList:
    case sysTrapLstEraseList:
    case sysTrapLstGetSelection:
    case sysTrapLstGetSelectionText:
    case sysTrapLstHandleEvent:
    case sysTrapLstSetHeight:
    case sysTrapLstSetPosition:
    case sysTrapLstSetSelection:
    case sysTrapLstSetListChoices:
    case sysTrapLstSetTopItem:
    case sysTrapLstMakeItemVisible:
    case sysTrapLstGetNumberOfItems:
    case sysTrapLstPopupList:
    case sysTrapLstScrollList:
    case sysTrapLstGetVisibleItems:
    case sysTrapLstGetTopItem:
      palmos_LstSysTrap(sp, idx, trap);
      break;
    case sysTrapTblSetCustomDrawProcedure:
    case sysTrapTblSetLoadDataProcedure:
    case sysTrapTblSetSaveDataProcedure:
    case sysTrapTblDrawTable:
    case sysTrapTblRedrawTable:
    case sysTrapTblEraseTable:
    case sysTrapTblHandleEvent:
    case sysTrapTblGetItemBounds:
    case sysTrapTblSelectItem:
    case sysTrapTblGetItemInt:
    case sysTrapTblSetItemInt:
    case sysTrapTblSetItemPtr:
    case sysTrapTblSetItemStyle:
    case sysTrapTblUnhighlightSelection:
    case sysTrapTblRowUsable:
    case sysTrapTblSetRowUsable:
    case sysTrapTblGetLastUsableRow:
    case sysTrapTblSetColumnUsable:
    case sysTrapTblSetRowSelectable:
    case sysTrapTblRowSelectable:
    case sysTrapTblGetNumberOfRows:
    case sysTrapTblGetBounds:
    case sysTrapTblSetBounds:
    case sysTrapTblGetRowHeight:
    case sysTrapTblSetRowHeight:
    case sysTrapTblGetColumnWidth:
    case sysTrapTblSetColumnWidth:
    case sysTrapTblGetColumnSpacing:
    case sysTrapTblSetColumnSpacing:
    case sysTrapTblFindRowID:
    case sysTrapTblFindRowData:
    case sysTrapTblGetRowID:
    case sysTrapTblSetRowID:
    case sysTrapTblGetRowData:
    case sysTrapTblSetRowData:
    case sysTrapTblRowInvalid:
    case sysTrapTblMarkRowInvalid:
    case sysTrapTblMarkTableInvalid:
    case sysTrapTblGetSelection:
    case sysTrapTblInsertRow:
    case sysTrapTblRemoveRow:
    case sysTrapTblReleaseFocus:
    case sysTrapTblEditing:
    case sysTrapTblGetCurrentField:
    case sysTrapTblGrabFocus:
    case sysTrapTblSetColumnEditIndicator:
    case sysTrapTblSetRowStaticHeight:
    case sysTrapTblHasScrollBar:
    case sysTrapTblGetItemFont:
    case sysTrapTblSetItemFont:
    case sysTrapTblGetItemPtr:
    case sysTrapTblRowMasked:
    case sysTrapTblSetRowMasked:
    case sysTrapTblSetColumnMasked:
    case sysTrapTblGetNumberOfColumns:
    case sysTrapTblGetTopRow:
    case sysTrapTblSetSelection:
      palmos_TblSysTrap(sp, idx, trap);
      break;
    case sysTrapSclSetScrollBar:
    case sysTrapSclGetScrollBar:
    case sysTrapSclDrawScrollBar:
    case sysTrapSclHandleEvent:
      palmos_SclSysTrap(sp, idx, trap);
      break;
    case sysTrapEvtEnableGraffiti:
    case sysTrapEvtResetAutoOffTimer:
    case sysTrapEvtAddUniqueEventToQueue:
    case sysTrapEvtAddEventToQueue:
    case sysTrapEvtEnqueueKey:
    case sysTrapEvtEventAvail:
    case sysTrapEvtWakeup:
    case sysTrapEvtGetEvent:
    case sysTrapEvtCopyEvent:
    case sysTrapEvtGetPen:
    case sysTrapEvtSysEventAvail:
    case sysTrapEvtFlushKeyQueue:
    case sysTrapEvtFlushPenQueue:
    case sysTrapEvtSetNullEventTick:
    case sysTrapEvtFlushNextPenStroke:
    case sysTrapEvtKeyQueueEmpty:
    case sysTrapEvtGetSilkscreenAreaList:
    case sysTrapEvtGetPenBtnList:
      palmos_EvtSysTrap(sp, idx, trap);
      break;
    case sysTrapPenResetCalibration:
    case sysTrapPenCalibrate:
    case sysTrapPenSleep:
    case sysTrapPenWake:
      palmos_PenSysTrap(sp, idx, trap);
      break;
    case sysTrapClipboardAddItem:
    case sysTrapClipboardGetItem:
      palmos_ClipboardSysTrap(sp, idx, trap);
      break;
    case sysTrapExgInit:
    case sysTrapExgConnect:
    case sysTrapExgPut:
    case sysTrapExgGet:
    case sysTrapExgAccept:
    case sysTrapExgDisconnect:
    case sysTrapExgRegisterData:
    case sysTrapExgNotifyReceiveV35:
    case sysTrapExgDBRead:
    case sysTrapExgDBWrite:
    case sysTrapExgDoDialog:
    case sysTrapExgRegisterDatatype:
    case sysTrapExgNotifyReceive:
    case sysTrapExgNotifyGoto:
    case sysTrapExgRequest:
    case sysTrapExgSetDefaultApplication:
    case sysTrapExgGetDefaultApplication:
    case sysTrapExgGetTargetApplication:
    case sysTrapExgGetRegisteredApplications:
    case sysTrapExgGetRegisteredTypes:
    case sysTrapExgNotifyPreview:
    case sysTrapExgControl:
    case sysTrapExgSend:
    case sysTrapExgReceive:
      palmos_ExgSysTrap(sp, idx, trap);
      break;
    case sysTrapLocGetNumberSeparators:
      palmos_LocSysTrap(sp, idx, trap);
      break;
    case sysTrapSndPlaySmf:
    case sysTrapSndPlaySmfResource:
    case sysTrapSndCreateMidiList:
    case sysTrapSndPlaySystemSound:
    case sysTrapSndPlayResource:
    case sysTrapSndDoCmd:
    case sysTrapSndGetDefaultVolume:
    case sysTrapSndSetDefaultVolume:
    case sysTrapSndStreamCreate:
    case sysTrapSndStreamCreateExtended:
    case sysTrapSndStreamDelete:
    case sysTrapSndStreamSetVolume:
    case sysTrapSndStreamStart:
    case sysTrapSndStreamStop:
      palmos_SndSysTrap(sp, idx, trap);
      break;
    case sysTrapGrfGetState:
    case sysTrapGrfSetState:
      palmos_GrfSysTrap(sp, idx, trap);
      break;
    case sysTrapCrc16CalcBlock:
      palmos_CrcSysTrap(sp, idx, trap);
      break;
    case sysTrapGsiInitialize:
    case sysTrapGsiSetShiftState:
    case sysTrapGsiEnable:
    case sysTrapGsiSetLocation:
      palmos_GsiSysTrap(sp, idx, trap);
      break;
    case sysTrapPrgStartDialogV31:
    case sysTrapPrgStartDialog:
    case sysTrapPrgStopDialog:
    case sysTrapPrgHandleEvent:
    case sysTrapPrgUpdateDialog:
      palmos_PrgSysTrap(sp, idx, trap);
      break;
    case sysTrapEncDigestMD5:
      palmos_EncSysTrap(sp, idx, trap);
      break;
    case sysTrapGetCharCaselessValue:
    case sysTrapGetCharSortValue:
      palmos_GetSysTrap(sp, idx, trap);
      break;
    case sysTrapAlmSetAlarm:
    case sysTrapAlmGetAlarm:
      palmos_AlmSysTrap(sp, idx, trap);
      break;
    case sysTrapFldCopy:
    case sysTrapFldCut:
    case sysTrapFldDrawField:
    case sysTrapFldEraseField:
    case sysTrapFldFreeMemory:
    case sysTrapFldGetBounds:
    case sysTrapFldGetFont:
    case sysTrapFldGetSelection:
    case sysTrapFldGetTextHandle:
    case sysTrapFldGetTextPtr:
    case sysTrapFldHandleEvent:
    case sysTrapFldPaste:
    case sysTrapFldRecalculateField:
    case sysTrapFldSetBounds:
    case sysTrapFldSetFont:
    case sysTrapFldSetText:
    case sysTrapFldSetTextHandle:
    case sysTrapFldSetTextPtr:
    case sysTrapFldSetUsable:
    case sysTrapFldSetSelection:
    case sysTrapFldGrabFocus:
    case sysTrapFldReleaseFocus:
    case sysTrapFldGetInsPtPosition:
    case sysTrapFldSetInsPtPosition:
    case sysTrapFldSetInsertionPoint:
    case sysTrapFldGetScrollPosition:
    case sysTrapFldSetScrollPosition:
    case sysTrapFldGetScrollValues:
    case sysTrapFldGetTextLength:
    case sysTrapFldScrollField:
    case sysTrapFldScrollable:
    case sysTrapFldGetVisibleLines:
    case sysTrapFldGetTextHeight:
    case sysTrapFldCalcFieldHeight:
    case sysTrapFldWordWrap:
    case sysTrapFldCompactText:
    case sysTrapFldDirty:
    case sysTrapFldSetDirty:
    case sysTrapFldGetMaxChars:
    case sysTrapFldSetMaxChars:
    case sysTrapFldInsert:
    case sysTrapFldDelete:
    case sysTrapFldUndo:
    case sysTrapFldGetTextAllocatedSize:
    case sysTrapFldSetTextAllocatedSize:
    case sysTrapFldGetAttributes:
    case sysTrapFldSetAttributes:
    case sysTrapFldSendChangeNotification:
    case sysTrapFldSendHeightChangeNotification:
    case sysTrapFldMakeFullyVisible:
    case sysTrapFldGetNumberOfBlankLines:
    case sysTrapFldSetMaxVisibleLines:
    case sysTrapFldNewField:
      palmos_FldSysTrap(sp, idx, trap);
      break;
    case sysTrapMenuInit:
    case sysTrapMenuGetActiveMenu:
    case sysTrapMenuSetActiveMenu:
    case sysTrapMenuDispose:
    case sysTrapMenuHandleEvent:
    case sysTrapMenuDrawMenu:
    case sysTrapMenuEraseStatus:
    case sysTrapMenuSetActiveMenuRscID:
    case sysTrapMenuCmdBarAddButton:
    case sysTrapMenuCmdBarGetButtonData:
    case sysTrapMenuCmdBarDisplay:
    case sysTrapMenuShowItem:
    case sysTrapMenuHideItem:
    case sysTrapMenuAddItem:
      palmos_MenuSysTrap(sp, idx, trap);
      break;
    case sysTrapInsPtInitialize:
    case sysTrapInsPtSetLocation:
    case sysTrapInsPtGetLocation:
    case sysTrapInsPtEnable:
    case sysTrapInsPtEnabled:
    case sysTrapInsPtSetHeight:
    case sysTrapInsPtGetHeight:
    case sysTrapInsPtCheckBlink:
      palmos_InsSysTrap(sp, idx, trap);
      break;
    case sysTrapKeyCurrentState:
    case sysTrapKeyRates:
    case sysTrapKeySetMask:
      palmos_KeySysTrap(sp, idx, trap);
      break;
    case sysTrapCategoryCreateListV10:
    case sysTrapCategoryCreateList:
    case sysTrapCategoryFreeListV10:
    case sysTrapCategoryFreeList:
    case sysTrapCategoryFind:
    case sysTrapCategoryGetName:
    case sysTrapCategoryEditV10:
    case sysTrapCategoryEditV20:
    case sysTrapCategoryEdit:
    case sysTrapCategorySelectV10:
    case sysTrapCategorySelect:
    case sysTrapCategoryGetNext:
    case sysTrapCategorySetTriggerLabel:
    case sysTrapCategoryTruncateName:
    case sysTrapCategoryInitialize:
    case sysTrapCategorySetName:
      palmos_CategorySysTrap(sp, idx, trap);
      break;
    case sysTrapPwdExists:
      palmos_PwdSysTrap(sp, idx, trap);
      break;
