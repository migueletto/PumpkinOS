    case sysTrapAccessorDispatch: {
    switch (sel) {
    case 0: {
      const ControlType *ctlP = sys_va_arg(ap, ControlType *);
      ControlStyleType ret = CtlGlueGetControlStyle(ctlP);
      *iret = ret;
      }
      break;

    case 1: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 lineNum = sys_va_arg(ap, UInt32);
      UInt16 *startP = sys_va_arg(ap, UInt16 *);
      UInt16 *lengthP = sys_va_arg(ap, UInt16 *);
      Boolean ret = FldGlueGetLineInfo(fldP, lineNum, startP, lengthP);
      *iret = ret;
      }
      break;

    case 2: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, UInt32);
      Boolean ret = FrmGlueGetObjectUsable(formP, objIndex);
      *iret = ret;
      }
      break;

    case 3: {
      const BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      BitmapCompressionType ret = BmpGlueGetCompressionType(bitmapP);
      *iret = ret;
      }
      break;

    case 4: {
      const BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      UInt32 *transparentValueP = sys_va_arg(ap, UInt32 *);
      Boolean ret = BmpGlueGetTransparentValue(bitmapP, transparentValueP);
      *iret = ret;
      }
      break;

    case 5: {
      BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      UInt32 transparentValue = sys_va_arg(ap, UInt32);
      BmpGlueSetTransparentValue(bitmapP, transparentValue);
      }
      break;

    case 6: {
      const ControlType *ctlP = sys_va_arg(ap, ControlType *);
      FontID ret = CtlGlueGetFont(ctlP);
      *iret = ret;
      }
      break;

    case 7: {
      ControlType *ctlP = sys_va_arg(ap, ControlType *);
      FontID fontID = sys_va_arg(ap, FontID);
      CtlGlueSetFont(ctlP, fontID);
      }
      break;

    case 8: {
      const ControlType *ctlP = sys_va_arg(ap, ControlType *);
      DmResID *bitmapID = sys_va_arg(ap, DmResID *);
      DmResID *selectedBitmapID = sys_va_arg(ap, DmResID *);
      CtlGlueGetGraphics(ctlP, bitmapID, selectedBitmapID);
      }
      break;

    case 9: {
      void **formPP = sys_va_arg(ap, void **);
      UInt16 ID = sys_va_arg(ap, UInt32);
      ControlStyleType style = sys_va_arg(ap, ControlStyleType);
      DmResID thumbID = sys_va_arg(ap, UInt32);
      DmResID backgroundID = sys_va_arg(ap, UInt32);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      Coord width = sys_va_arg(ap, Int32);
      Coord height = sys_va_arg(ap, Int32);
      UInt16 minValue = sys_va_arg(ap, UInt32);
      UInt16 maxValue = sys_va_arg(ap, UInt32);
      UInt16 pageSize = sys_va_arg(ap, UInt32);
      UInt16 value = sys_va_arg(ap, UInt32);
      SliderControlType *ret = CtlGlueNewSliderControl(formPP, ID, style, thumbID, backgroundID, x, y, width, height, minValue, maxValue, pageSize, value);
      *pret = (void *)ret;
      }
      break;

    case 10: {
      ControlType *ctlP = sys_va_arg(ap, ControlType *);
      Boolean leftAnchor = sys_va_arg(ap, UInt32);
      CtlGlueSetLeftAnchor(ctlP, leftAnchor);
      }
      break;

    case 11: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 ret = FrmGlueGetDefaultButtonID(formP);
      *iret = ret;
      }
      break;

    case 12: {
      FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 defaultButton = sys_va_arg(ap, UInt32);
      FrmGlueSetDefaultButtonID(formP, defaultButton);
      }
      break;

    case 13: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 ret = FrmGlueGetHelpID(formP);
      *iret = ret;
      }
      break;

    case 14: {
      FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 helpRscID = sys_va_arg(ap, UInt32);
      FrmGlueSetHelpID(formP, helpRscID);
      }
      break;

    case 15: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 ret = FrmGlueGetMenuBarID(formP);
      *iret = ret;
      }
      break;

    case 16: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 labelID = sys_va_arg(ap, UInt32);
      FontID ret = FrmGlueGetLabelFont(formP, labelID);
      *iret = ret;
      }
      break;

    case 17: {
      FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 labelID = sys_va_arg(ap, UInt32);
      FontID fontID = sys_va_arg(ap, FontID);
      FrmGlueSetLabelFont(formP, labelID, fontID);
      }
      break;

    case 18: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      FormEventHandlerType *ret = FrmGlueGetEventHandler(formP);
      *pret = (void *)ret;
      }
      break;

    case 19: {
      const ListType *listP = sys_va_arg(ap, ListType *);
      FontID ret = LstGlueGetFont(listP);
      *iret = ret;
      }
      break;

    case 20: {
      ListType *listP = sys_va_arg(ap, ListType *);
      FontID fontID = sys_va_arg(ap, FontID);
      LstGlueSetFont(listP, fontID);
      }
      break;

    case 21: {
      const ListType *listP = sys_va_arg(ap, ListType *);
      Char **ret = LstGlueGetItemsText(listP);
      *pret = (void *)ret;
      }
      break;

    case 22: {
      ListType *listP = sys_va_arg(ap, ListType *);
      Boolean incrementalSearch = sys_va_arg(ap, UInt32);
      LstGlueSetIncrementalSearch(listP, incrementalSearch);
      }
      break;

    case 23: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 column = sys_va_arg(ap, Int32);
      Boolean ret = TblGlueGetColumnMasked(tableP, column);
      *iret = ret;
      }
      break;

    case 24: {
      const WinHandle winH = sys_va_arg(ap, WinHandle);
      FrameType ret = WinGlueGetFrameType(winH);
      *iret = ret;
      }
      break;

    case 25: {
      WinHandle winH = sys_va_arg(ap, WinHandle);
      FrameType frame = sys_va_arg(ap, UInt32);
      WinGlueSetFrameType(winH, frame);
      }
      break;

    case 26: {
      void *formObjP = sys_va_arg(ap, void *);
      FormObjectKind objKind = sys_va_arg(ap, FormObjectKind);
      UInt16 ret = FrmGlueGetObjIDFromObjPtr(formObjP, objKind);
      *iret = ret;
      }
      break;

    case 27: {
      ListType *listP = sys_va_arg(ap, ListType *);
      void *ret = LstGlueGetDrawFunction(listP);
      *pret = (void *)ret;
      }
      break;

    case 28: {
      ControlType *controlP = sys_va_arg(ap, ControlType *);
      Boolean ret = CtlGlueIsGraphical(controlP);
      *iret = ret;
      }
      break;

    case 29: {
      ControlType *controlP = sys_va_arg(ap, ControlType *);
      ButtonFrameType frameStyle = sys_va_arg(ap, ButtonFrameType);
      CtlGlueSetFrameStyle(controlP, frameStyle);
      }
      break;

    case 30: {
      FontDefaultType inFontType = sys_va_arg(ap, FontDefaultType);
      FontID ret = FntGlueGetDefaultFontID(inFontType);
      *iret = ret;
      }
      break;

    case 31: {
      char *iDstString = sys_va_arg(ap, char *);
      const char *iSrcString = sys_va_arg(ap, char *);
      FontID iFont = sys_va_arg(ap, FontID);
      Coord iMaxWidth = sys_va_arg(ap, Int32);
      Boolean iAddEllipsis = sys_va_arg(ap, UInt32);
      Boolean ret = FntGlueTruncateString(iDstString, iSrcString, iFont, iMaxWidth, iAddEllipsis);
      *iret = ret;
      }
      break;

    case 32: {
      WChar iChar = sys_va_arg(ap, UInt32);
      Int16 ret = FntGlueWCharWidth(iChar);
      *iret = ret;
      }
      break;

    case 33: {
      const Char *charsP = sys_va_arg(ap, Char *);
      UInt16 length = sys_va_arg(ap, UInt32);
      Int16 pixelWidth = sys_va_arg(ap, Int32);
      Boolean *leadingEdge = sys_va_arg(ap, Boolean *);
      Int16 *truncWidth = sys_va_arg(ap, Int16 *);
      Int16 ret = FntGlueWidthToOffset(charsP, length, pixelWidth, leadingEdge, truncWidth);
      *iret = ret;
      }
      break;

    case 34: {
      WChar inChar = sys_va_arg(ap, UInt32);
      WChar ret = TxtGlueUpperChar(inChar);
      *iret = ret;
      }
      break;

    case 35: {
      WChar inChar = sys_va_arg(ap, UInt32);
      WChar ret = TxtGlueLowerChar(inChar);
      *iret = ret;
      }
      break;

    }
    }
    break;

    case sysTrapCncMgrDispatch: {
    switch (sel) {
    case sysTrapCncMgrProfileCloseDB: {
      Err ret = CncProfileCloseDB();
      *iret = ret;
      }
      break;

    case sysTrapCncMgrProfileCount: {
      UInt16 *profilesCountP = sys_va_arg(ap, UInt16 *);
      Err ret = CncProfileCount(profilesCountP);
      *iret = ret;
      }
      break;

    case sysTrapCncMgrProfileCreate: {
      CncProfileID *profileIdP = sys_va_arg(ap, CncProfileID *);
      Err ret = CncProfileCreate(profileIdP);
      *iret = ret;
      }
      break;

    case sysTrapCncMgrProfileDelete: {
      CncProfileID profileId = sys_va_arg(ap, CncProfileID);
      Err ret = CncProfileDelete(profileId);
      *iret = ret;
      }
      break;

    case sysTrapCncMgrProfileGetCurrent: {
      CncProfileID *profileIdP = sys_va_arg(ap, CncProfileID *);
      Err ret = CncProfileGetCurrent(profileIdP);
      *iret = ret;
      }
      break;

    case sysTrapCncMgrProfileGetIDFromIndex: {
      UInt16 index = sys_va_arg(ap, UInt32);
      CncProfileID *profileIdP = sys_va_arg(ap, CncProfileID *);
      Err ret = CncProfileGetIDFromIndex(index, profileIdP);
      *iret = ret;
      }
      break;

    case sysTrapCncMgrProfileGetIDFromName: {
      const Char *profileNameP = sys_va_arg(ap, Char *);
      CncProfileID *profileIdP = sys_va_arg(ap, CncProfileID *);
      Err ret = CncProfileGetIDFromName(profileNameP, profileIdP);
      *iret = ret;
      }
      break;

    case sysTrapCncMgrProfileGetIndex: {
      CncProfileID profileId = sys_va_arg(ap, CncProfileID);
      UInt16 *indexP = sys_va_arg(ap, UInt16 *);
      Err ret = CncProfileGetIndex(profileId, indexP);
      *iret = ret;
      }
      break;

    case sysTrapCncMgrProfileOpenDB: {
      Err ret = CncProfileOpenDB();
      *iret = ret;
      }
      break;

    case sysTrapCncMgrProfileSetCurrent: {
      CncProfileID profileId = sys_va_arg(ap, CncProfileID);
      Err ret = CncProfileSetCurrent(profileId);
      *iret = ret;
      }
      break;

    case sysTrapCncMgrProfileSettingGet: {
      CncProfileID profileId = sys_va_arg(ap, CncProfileID);
      UInt16 paramId = sys_va_arg(ap, UInt32);
      void *paramBufferP = sys_va_arg(ap, void *);
      UInt16 *ioParamSizeP = sys_va_arg(ap, UInt16 *);
      Err ret = CncProfileSettingGet(profileId, paramId, paramBufferP, ioParamSizeP);
      *iret = ret;
      }
      break;

    case sysTrapCncMgrProfileSettingSet: {
      CncProfileID iProfileId = sys_va_arg(ap, CncProfileID);
      UInt16 paramId = sys_va_arg(ap, UInt32);
      const void *paramBufferP = sys_va_arg(ap, void *);
      UInt16 paramSize = sys_va_arg(ap, UInt32);
      Err ret = CncProfileSettingSet(iProfileId, paramId, paramBufferP, paramSize);
      *iret = ret;
      }
      break;

    }
    }
    break;

    case sysTrapExpansionMgr: {
    switch (sel) {
    case expCardGetSerialPort: {
      UInt16 slotRefNum = sys_va_arg(ap, UInt32);
      UInt32 *portP = sys_va_arg(ap, UInt32 *);
      Err ret = ExpCardGetSerialPort(slotRefNum, portP);
      *iret = ret;
      }
      break;

    case expCardInfo: {
      UInt16 slotRefNum = sys_va_arg(ap, UInt32);
      ExpCardInfoType *infoP = sys_va_arg(ap, ExpCardInfoType *);
      Err ret = ExpCardInfo(slotRefNum, infoP);
      *iret = ret;
      }
      break;

    case expCardInserted: {
      UInt16 slotRefNum = sys_va_arg(ap, UInt32);
      Err ret = ExpCardInserted(slotRefNum);
      *iret = ret;
      }
      break;

    case expCardPresent: {
      UInt16 slotRefNum = sys_va_arg(ap, UInt32);
      Err ret = ExpCardPresent(slotRefNum);
      *iret = ret;
      }
      break;

    case expCardRemoved: {
      UInt16 slotRefNum = sys_va_arg(ap, UInt32);
      Err ret = ExpCardRemoved(slotRefNum);
      *iret = ret;
      }
      break;

    case expInit: {
      Err ret = ExpInit();
      *iret = ret;
      }
      break;

    case expSlotDriverInstall: {
      UInt32 dbCreator = sys_va_arg(ap, UInt32);
      UInt16 *slotLibRefNumP = sys_va_arg(ap, UInt16 *);
      Err ret = ExpSlotDriverInstall(dbCreator, slotLibRefNumP);
      *iret = ret;
      }
      break;

    case expSlotDriverRemove: {
      UInt16 slotLibRefNum = sys_va_arg(ap, UInt32);
      Err ret = ExpSlotDriverRemove(slotLibRefNum);
      *iret = ret;
      }
      break;

    case expSlotEnumerate: {
      UInt16 *slotRefNumP = sys_va_arg(ap, UInt16 *);
      UInt32 *slotIteratorP = sys_va_arg(ap, UInt32 *);
      Err ret = ExpSlotEnumerate(slotRefNumP, slotIteratorP);
      *iret = ret;
      }
      break;

    case expSlotLibFind: {
      UInt16 slotRefNum = sys_va_arg(ap, UInt32);
      UInt16 *slotLibRefNum = sys_va_arg(ap, UInt16 *);
      Err ret = ExpSlotLibFind(slotRefNum, slotLibRefNum);
      *iret = ret;
      }
      break;

    case expSlotRegister: {
      UInt16 slotLibRefNum = sys_va_arg(ap, UInt32);
      UInt16 *slotRefNum = sys_va_arg(ap, UInt16 *);
      Err ret = ExpSlotRegister(slotLibRefNum, slotRefNum);
      *iret = ret;
      }
      break;

    case expSlotUnregister: {
      UInt16 slotRefNum = sys_va_arg(ap, UInt32);
      Err ret = ExpSlotUnregister(slotRefNum);
      *iret = ret;
      }
      break;

    }
    }
    break;

    case sysTrapFlpDispatch: {
    switch (sel) {
    case sysFloatAToF: {
      const Char *s = sys_va_arg(ap, Char *);
      FlpDouble ret = FlpAToF(s);
      uint64_t *d = (uint64_t *)(&ret);
      *iret = *d;
      }
      break;

    case sysFloatFToA: {
      FlpDouble a = sys_va_arg(ap, FlpDouble);
      Char *s = sys_va_arg(ap, Char *);
      Err ret = FlpFToA(a, s);
      *iret = ret;
      }
      break;

    case sysFloatVersion: {
      UInt32 ret = FlpVersion();
      *iret = ret;
      }
      break;

    }
    }
    break;

    case sysTrapFlpEmDispatch: {
    switch (sel) {
    case sysFloatEm_d_add: {
      FlpDouble a = sys_va_arg(ap, FlpDouble);
      FlpDouble b = sys_va_arg(ap, FlpDouble);
      FlpDouble ret = _d_add(a, b);
      uint64_t *d = (uint64_t *)(&ret);
      *iret = *d;
      }
      break;

    }
    }
    break;

    case sysTrapHighDensityDispatch: {
    switch (sel) {
    case HDSelectorBmpCreateBitmapV3: {
      const BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      UInt16 density = sys_va_arg(ap, UInt32);
      const void *bitsP = sys_va_arg(ap, void *);
      const ColorTableType *colorTableP = sys_va_arg(ap, ColorTableType *);
      BitmapTypeV3 *ret = BmpCreateBitmapV3(bitmapP, density, bitsP, colorTableP);
      *pret = (void *)ret;
      }
      break;

    case HDSelectorBmpGetCompressionType: {
      const BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      BitmapCompressionType ret = BmpGetCompressionType(bitmapP);
      *iret = ret;
      }
      break;

    case HDSelectorBmpGetDensity: {
      const BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      UInt16 ret = BmpGetDensity(bitmapP);
      *iret = ret;
      }
      break;

    case HDSelectorBmpGetNextBitmapAnyDensity: {
      BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      BitmapType *ret = BmpGetNextBitmapAnyDensity(bitmapP);
      *pret = (void *)ret;
      }
      break;

    case HDSelectorBmpGetTransparentValue: {
      const BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      UInt32 *transparentValueP = sys_va_arg(ap, UInt32 *);
      Boolean ret = BmpGetTransparentValue(bitmapP, transparentValueP);
      *iret = ret;
      }
      break;

    case HDSelectorBmpGetVersion: {
      const BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      UInt8 ret = BmpGetVersion(bitmapP);
      *iret = ret;
      }
      break;

    case HDSelectorBmpSetDensity: {
      BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      UInt16 density = sys_va_arg(ap, UInt32);
      Err ret = BmpSetDensity(bitmapP, density);
      *iret = ret;
      }
      break;

    case HDSelectorBmpSetTransparentValue: {
      BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      UInt32 transparentValue = sys_va_arg(ap, UInt32);
      BmpSetTransparentValue(bitmapP, transparentValue);
      }
      break;

    case HDSelectorEvtGetPenNative: {
      WinHandle winH = sys_va_arg(ap, WinHandle);
      Int16 *pScreenX = sys_va_arg(ap, Int16 *);
      Int16 *pScreenY = sys_va_arg(ap, Int16 *);
      Boolean *pPenDown = sys_va_arg(ap, Boolean *);
      EvtGetPenNative(winH, pScreenX, pScreenY, pPenDown);
      }
      break;

    case HDSelectorWinGetCoordinateSystem: {
      UInt16 ret = WinGetCoordinateSystem();
      *iret = ret;
      }
      break;

    case HDSelectorWinGetScalingMode: {
      UInt32 ret = WinGetScalingMode();
      *iret = ret;
      }
      break;

    case HDSelectorWinGetSupportedDensity: {
      UInt16 *densityP = sys_va_arg(ap, UInt16 *);
      Err ret = WinGetSupportedDensity(densityP);
      *iret = ret;
      }
      break;

    case HDSelectorWinPaintRoundedRectangleFrame: {
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      Coord width = sys_va_arg(ap, Int32);
      Coord cornerRadius = sys_va_arg(ap, Int32);
      Coord shadowWidth = sys_va_arg(ap, Int32);
      WinPaintRoundedRectangleFrame(rP, width, cornerRadius, shadowWidth);
      }
      break;

    case HDSelectorWinPaintTiledBitmap: {
      BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      RectangleType *rectP = sys_va_arg(ap, RectangleType *);
      WinPaintTiledBitmap(bitmapP, rectP);
      }
      break;

    case HDSelectorWinScaleCoord: {
      Coord coord = sys_va_arg(ap, Int32);
      Boolean ceiling = sys_va_arg(ap, UInt32);
      Coord ret = WinScaleCoord(coord, ceiling);
      *iret = ret;
      }
      break;

    case HDSelectorWinScalePoint: {
      PointType *pointP = sys_va_arg(ap, PointType *);
      Boolean ceiling = sys_va_arg(ap, UInt32);
      WinScalePoint(pointP, ceiling);
      }
      break;

    case HDSelectorWinScaleRectangle: {
      RectangleType *rectP = sys_va_arg(ap, RectangleType *);
      WinScaleRectangle(rectP);
      }
      break;

    case HDSelectorWinScreenGetAttribute: {
      WinScreenAttrType selector = sys_va_arg(ap, WinScreenAttrType);
      UInt32 *attrP = sys_va_arg(ap, UInt32 *);
      Err ret = WinScreenGetAttribute(selector, attrP);
      *iret = ret;
      }
      break;

    case HDSelectorWinSetCoordinateSystem: {
      UInt16 coordSys = sys_va_arg(ap, UInt32);
      UInt16 ret = WinSetCoordinateSystem(coordSys);
      *iret = ret;
      }
      break;

    case HDSelectorWinSetScalingMode: {
      UInt32 mode = sys_va_arg(ap, UInt32);
      UInt32 ret = WinSetScalingMode(mode);
      *iret = ret;
      }
      break;

    case HDSelectorWinUnscaleCoord: {
      Coord coord = sys_va_arg(ap, Int32);
      Boolean ceiling = sys_va_arg(ap, UInt32);
      Coord ret = WinUnscaleCoord(coord, ceiling);
      *iret = ret;
      }
      break;

    case HDSelectorWinUnscalePoint: {
      PointType *pointP = sys_va_arg(ap, PointType *);
      Boolean ceiling = sys_va_arg(ap, UInt32);
      WinUnscalePoint(pointP, ceiling);
      }
      break;

    case HDSelectorWinUnscaleRectangle: {
      RectangleType *rectP = sys_va_arg(ap, RectangleType *);
      WinUnscaleRectangle(rectP);
      }
      break;

    }
    }
    break;

    case sysTrapHostControl: {
    switch (sel) {
    case hostSelectorAscTime: {
      const HostTmType *a = sys_va_arg(ap, HostTmType *);
      char *ret = HostAscTime(a);
      *pret = (void *)ret;
      }
      break;

    case hostSelectorCTime: {
      const HostTimeType *a = sys_va_arg(ap, HostTimeType *);
      char *ret = HostCTime(a);
      *pret = (void *)ret;
      }
      break;

    case hostSelectorClock: {
      HostClockType ret = HostClock();
      *iret = ret;
      }
      break;

    case hostSelectorCloseDir: {
      HostDIRType *a = sys_va_arg(ap, HostDIRType *);
      long ret = HostCloseDir(a);
      *iret = ret;
      }
      break;

    case hostSelectorDbgClearDataBreak: {
      HostErr ret = HostDbgClearDataBreak();
      *iret = ret;
      }
      break;

    case hostSelectorDbgSetDataBreak: {
      UInt32 addr = sys_va_arg(ap, UInt32);
      UInt32 size = sys_va_arg(ap, UInt32);
      HostErr ret = HostDbgSetDataBreak(addr, size);
      *iret = ret;
      }
      break;

    case hostSelectorErrNo: {
      long ret = HostErrNo();
      *iret = ret;
      }
      break;

    case hostSelectorExgLibAccept: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      void *exgSocketP = sys_va_arg(ap, void *);
      Err ret = HostExgLibAccept(libRefNum, exgSocketP);
      *iret = ret;
      }
      break;

    case hostSelectorExgLibClose: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      Err ret = HostExgLibClose(libRefNum);
      *iret = ret;
      }
      break;

    case hostSelectorExgLibConnect: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      void *exgSocketP = sys_va_arg(ap, void *);
      Err ret = HostExgLibConnect(libRefNum, exgSocketP);
      *iret = ret;
      }
      break;

    case hostSelectorExgLibControl: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt16 op = sys_va_arg(ap, UInt32);
      void *valueP = sys_va_arg(ap, void *);
      UInt16 *valueLenP = sys_va_arg(ap, UInt16 *);
      Err ret = HostExgLibControl(libRefNum, op, valueP, valueLenP);
      *iret = ret;
      }
      break;

    case hostSelectorExgLibDisconnect: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      void *exgSocketP = sys_va_arg(ap, void *);
      Err error = sys_va_arg(ap, UInt32);
      Err ret = HostExgLibDisconnect(libRefNum, exgSocketP, error);
      *iret = ret;
      }
      break;

    case hostSelectorExgLibGet: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      void *exgSocketP = sys_va_arg(ap, void *);
      Err ret = HostExgLibGet(libRefNum, exgSocketP);
      *iret = ret;
      }
      break;

    case hostSelectorExgLibHandleEvent: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      void *eventP = sys_va_arg(ap, void *);
      Boolean ret = HostExgLibHandleEvent(libRefNum, eventP);
      *iret = ret;
      }
      break;

    case hostSelectorExgLibOpen: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      Err ret = HostExgLibOpen(libRefNum);
      *iret = ret;
      }
      break;

    case hostSelectorExgLibPut: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      void *exgSocketP = sys_va_arg(ap, void *);
      Err ret = HostExgLibPut(libRefNum, exgSocketP);
      *iret = ret;
      }
      break;

    case hostSelectorExgLibReceive: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      void *exgSocketP = sys_va_arg(ap, void *);
      void *bufP = sys_va_arg(ap, void *);
      const UInt32 bufSize = sys_va_arg(ap, UInt32);
      Err *errP = sys_va_arg(ap, Err *);
      UInt32 ret = HostExgLibReceive(libRefNum, exgSocketP, bufP, bufSize, errP);
      *iret = ret;
      }
      break;

    case hostSelectorExgLibRequest: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      void *exgSocketP = sys_va_arg(ap, void *);
      Err ret = HostExgLibRequest(libRefNum, exgSocketP);
      *iret = ret;
      }
      break;

    case hostSelectorExgLibSend: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      void *exgSocketP = sys_va_arg(ap, void *);
      const void * const bufP = sys_va_arg(ap, void * const);
      const UInt32 bufLen = sys_va_arg(ap, UInt32);
      Err *errP = sys_va_arg(ap, Err *);
      UInt32 ret = HostExgLibSend(libRefNum, exgSocketP, bufP, bufLen, errP);
      *iret = ret;
      }
      break;

    case hostSelectorExgLibSleep: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      Err ret = HostExgLibSleep(libRefNum);
      *iret = ret;
      }
      break;

    case hostSelectorExgLibWake: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      Err ret = HostExgLibWake(libRefNum);
      *iret = ret;
      }
      break;

    case hostSelectorExportFile: {
      const char *fileName = sys_va_arg(ap, char *);
      long cardNum = sys_va_arg(ap, long);
      const char *dbName = sys_va_arg(ap, char *);
      HostErrType ret = HostExportFile(fileName, cardNum, dbName);
      *iret = ret;
      }
      break;

    case hostSelectorFClose: {
      HostFILEType *fileP = sys_va_arg(ap, HostFILEType *);
      long ret = HostFClose(fileP);
      *iret = ret;
      }
      break;

    case hostSelectorFEOF: {
      HostFILEType *fileP = sys_va_arg(ap, HostFILEType *);
      long ret = HostFEOF(fileP);
      *iret = ret;
      }
      break;

    case hostSelectorFError: {
      HostFILEType *fileP = sys_va_arg(ap, HostFILEType *);
      long ret = HostFError(fileP);
      *iret = ret;
      }
      break;

    case hostSelectorFFlush: {
      HostFILEType *fileP = sys_va_arg(ap, HostFILEType *);
      long ret = HostFFlush(fileP);
      *iret = ret;
      }
      break;

    case hostSelectorFGetC: {
      HostFILEType *fileP = sys_va_arg(ap, HostFILEType *);
      long ret = HostFGetC(fileP);
      *iret = ret;
      }
      break;

    case hostSelectorFGetPos: {
      HostFILEType *fileP = sys_va_arg(ap, HostFILEType *);
      long *posP = sys_va_arg(ap, long *);
      long ret = HostFGetPos(fileP, posP);
      *iret = ret;
      }
      break;

    case hostSelectorFGetS: {
      char *s = sys_va_arg(ap, char *);
      long n = sys_va_arg(ap, long);
      HostFILEType *fileP = sys_va_arg(ap, HostFILEType *);
      char *ret = HostFGetS(s, n, fileP);
      *pret = (void *)ret;
      }
      break;

    case hostSelectorFOpen: {
      const char *name = sys_va_arg(ap, char *);
      const char *mode = sys_va_arg(ap, char *);
      HostFILEType *ret = HostFOpen(name, mode);
      *pret = (void *)ret;
      }
      break;

    case hostSelectorFPrintF: {
      HostFILEType *fileP = sys_va_arg(ap, HostFILEType *);
      const char *fmt = sys_va_arg(ap, char *);
      long ret = HostFPrintF(fileP, fmt, ap);
      *iret = ret;
      }
      break;

    case hostSelectorFPutC: {
      long c = sys_va_arg(ap, long);
      HostFILEType *fileP = sys_va_arg(ap, HostFILEType *);
      long ret = HostFPutC(c, fileP);
      *iret = ret;
      }
      break;

    case hostSelectorFPutS: {
      const char *s = sys_va_arg(ap, char *);
      HostFILEType *fileP = sys_va_arg(ap, HostFILEType *);
      long ret = HostFPutS(s, fileP);
      *iret = ret;
      }
      break;

    case hostSelectorFRead: {
      void *buffer = sys_va_arg(ap, void *);
      long size = sys_va_arg(ap, long);
      long count = sys_va_arg(ap, long);
      HostFILEType *fileP = sys_va_arg(ap, HostFILEType *);
      long ret = HostFRead(buffer, size, count, fileP);
      *iret = ret;
      }
      break;

    case hostSelectorFReopen: {
      const char *name = sys_va_arg(ap, char *);
      const char *mode = sys_va_arg(ap, char *);
      HostFILEType *fileP = sys_va_arg(ap, HostFILEType *);
      HostFILEType *ret = HostFReopen(name, mode, fileP);
      *pret = (void *)ret;
      }
      break;

    case hostSelectorFScanF: {
      HostFILEType *fileP = sys_va_arg(ap, HostFILEType *);
      const char *fmt = sys_va_arg(ap, char *);
      long ret = HostFScanF(fileP, fmt, ap);
      *iret = ret;
      }
      break;

    case hostSelectorFSeek: {
      HostFILEType *fileP = sys_va_arg(ap, HostFILEType *);
      long offset = sys_va_arg(ap, long);
      long origin = sys_va_arg(ap, long);
      long ret = HostFSeek(fileP, offset, origin);
      *iret = ret;
      }
      break;

    case hostSelectorFSetPos: {
      HostFILEType *fileP = sys_va_arg(ap, HostFILEType *);
      long *pos = sys_va_arg(ap, long *);
      long ret = HostFSetPos(fileP, pos);
      *iret = ret;
      }
      break;

    case hostSelectorFTell: {
      HostFILEType *fileP = sys_va_arg(ap, HostFILEType *);
      long ret = HostFTell(fileP);
      *iret = ret;
      }
      break;

    case hostSelectorFWrite: {
      const void *buffer = sys_va_arg(ap, void *);
      long size = sys_va_arg(ap, long);
      long count = sys_va_arg(ap, long);
      HostFILEType *fileP = sys_va_arg(ap, HostFILEType *);
      long ret = HostFWrite(buffer, size, count, fileP);
      *iret = ret;
      }
      break;

    case hostSelectorFree: {
      void *p = sys_va_arg(ap, void *);
      HostFree(p);
      }
      break;

    case hostSelectorGMTime: {
      const HostTimeType *a = sys_va_arg(ap, HostTimeType *);
      HostTmType *ret = HostGMTime(a);
      *pret = (void *)ret;
      }
      break;

    case hostSelectorGestalt: {
      long gestSel = sys_va_arg(ap, long);
      long *response = sys_va_arg(ap, long *);
      HostErrType ret = HostGestalt(gestSel, response);
      *iret = ret;
      }
      break;

    case hostSelectorGetDirectory: {
      const char *prompt = sys_va_arg(ap, char *);
      const char *defaultDir = sys_va_arg(ap, char *);
      const char *ret = HostGetDirectory(prompt, defaultDir);
      *pret = (void *)ret;
      }
      break;

    case hostSelectorGetEnv: {
      const char *nameP = sys_va_arg(ap, char *);
      char *ret = HostGetEnv(nameP);
      *pret = (void *)ret;
      }
      break;

    case hostSelectorGetFile: {
      const char *prompt = sys_va_arg(ap, char *);
      const char *defaultDir = sys_va_arg(ap, char *);
      const char *ret = HostGetFile(prompt, defaultDir);
      *pret = (void *)ret;
      }
      break;

    case hostSelectorGetFileAttr: {
      const char *a = sys_va_arg(ap, char *);
      long *b = sys_va_arg(ap, long *);
      long ret = HostGetFileAttr(a, b);
      *iret = ret;
      }
      break;

    case hostSelectorGetHostID: {
      HostIDType ret = HostGetHostID();
      *iret = ret;
      }
      break;

    case hostSelectorGetHostPlatform: {
      HostPlatformType ret = HostGetHostPlatform();
      *iret = ret;
      }
      break;

    case hostSelectorGetHostVersion: {
      Int32 ret = HostGetHostVersion();
      *iret = ret;
      }
      break;

    case hostSelectorGetPreference: {
      const char *a = sys_va_arg(ap, char *);
      char *b = sys_va_arg(ap, char *);
      HostBoolType ret = HostGetPreference(a, b);
      *iret = ret;
      }
      break;

    case hostSelectorGremlinCounter: {
      long ret = HostGremlinCounter();
      *iret = ret;
      }
      break;

    case hostSelectorGremlinIsRunning: {
      HostBoolType ret = HostGremlinIsRunning();
      *iret = ret;
      }
      break;

    case hostSelectorGremlinLimit: {
      long ret = HostGremlinLimit();
      *iret = ret;
      }
      break;

    case hostSelectorGremlinNew: {
      const HostGremlinInfoType *a = sys_va_arg(ap, HostGremlinInfoType *);
      HostErrType ret = HostGremlinNew(a);
      *iret = ret;
      }
      break;

    case hostSelectorGremlinNumber: {
      long ret = HostGremlinNumber();
      *iret = ret;
      }
      break;

    case hostSelectorImportFile: {
      const char *fileName = sys_va_arg(ap, char *);
      long cardNum = sys_va_arg(ap, long);
      HostErrType ret = HostImportFile(fileName, cardNum);
      *iret = ret;
      }
      break;

    case hostSelectorImportFileWithID: {
      const char *fileName = sys_va_arg(ap, char *);
      long cardNum = sys_va_arg(ap, long);
      LocalID *newIDP = sys_va_arg(ap, LocalID *);
      HostErrType ret = HostImportFileWithID(fileName, cardNum, newIDP);
      *iret = ret;
      }
      break;

    case hostSelectorIsCallingTrap: {
      HostBoolType ret = HostIsCallingTrap();
      *iret = ret;
      }
      break;

    case hostSelectorIsSelectorImplemented: {
      long selector = sys_va_arg(ap, long);
      HostBoolType ret = HostIsSelectorImplemented(selector);
      *iret = ret;
      }
      break;

    case hostSelectorLocalTime: {
      const HostTimeType *a = sys_va_arg(ap, HostTimeType *);
      HostTmType *ret = HostLocalTime(a);
      *pret = (void *)ret;
      }
      break;

    case hostSelectorLogFile: {
      HostFILEType *ret = HostLogFile();
      *pret = (void *)ret;
      }
      break;

    case hostSelectorMalloc: {
      long size = sys_va_arg(ap, long);
      void *ret = HostMalloc(size);
      *pret = (void *)ret;
      }
      break;

    case hostSelectorMkDir: {
      const char *a = sys_va_arg(ap, char *);
      long ret = HostMkDir(a);
      *iret = ret;
      }
      break;

    case hostSelectorMkTime: {
      HostTmType *a = sys_va_arg(ap, HostTmType *);
      HostTimeType ret = HostMkTime(a);
      *iret = ret;
      }
      break;

    case hostSelectorOpenDir: {
      const char *a = sys_va_arg(ap, char *);
      HostDIRType *ret = HostOpenDir(a);
      *pret = (void *)ret;
      }
      break;

    case hostSelectorProfileCleanup: {
      HostErrType ret = HostProfileCleanup();
      *iret = ret;
      }
      break;

    case hostSelectorProfileDetailFn: {
      void *addr = sys_va_arg(ap, void *);
      HostBoolType logDetails = sys_va_arg(ap, HostBoolType);
      HostErrType ret = HostProfileDetailFn(addr, logDetails);
      *iret = ret;
      }
      break;

    case hostSelectorProfileDump: {
      const char *filenameP = sys_va_arg(ap, char *);
      HostErrType ret = HostProfileDump(filenameP);
      *iret = ret;
      }
      break;

    case hostSelectorProfileGetCycles: {
      long ret = HostProfileGetCycles();
      *iret = ret;
      }
      break;

    case hostSelectorProfileInit: {
      long maxCalls = sys_va_arg(ap, long);
      long maxDepth = sys_va_arg(ap, long);
      HostErrType ret = HostProfileInit(maxCalls, maxDepth);
      *iret = ret;
      }
      break;

    case hostSelectorProfileStart: {
      HostErrType ret = HostProfileStart();
      *iret = ret;
      }
      break;

    case hostSelectorProfileStop: {
      HostErrType ret = HostProfileStop();
      *iret = ret;
      }
      break;

    case hostSelectorPutFile: {
      const char *prompt = sys_va_arg(ap, char *);
      const char *defaultDir = sys_va_arg(ap, char *);
      const char *defaultName = sys_va_arg(ap, char *);
      const char *ret = HostPutFile(prompt, defaultDir, defaultName);
      *pret = (void *)ret;
      }
      break;

    case hostSelectorReadDir: {
      HostDIRType *a = sys_va_arg(ap, HostDIRType *);
      HostDirEntType *ret = HostReadDir(a);
      *pret = (void *)ret;
      }
      break;

    case hostSelectorRealloc: {
      void *p = sys_va_arg(ap, void *);
      long size = sys_va_arg(ap, long);
      void *ret = HostRealloc(p, size);
      *pret = (void *)ret;
      }
      break;

    case hostSelectorRemove: {
      const char *nameP = sys_va_arg(ap, char *);
      long ret = HostRemove(nameP);
      *iret = ret;
      }
      break;

    case hostSelectorRename: {
      const char *oldNameP = sys_va_arg(ap, char *);
      const char *newNameP = sys_va_arg(ap, char *);
      long ret = HostRename(oldNameP, newNameP);
      *iret = ret;
      }
      break;

    case hostSelectorRmDir: {
      const char *a = sys_va_arg(ap, char *);
      long ret = HostRmDir(a);
      *iret = ret;
      }
      break;

    case hostSelectorSaveScreen: {
      const char *fileName = sys_va_arg(ap, char *);
      HostErrType ret = HostSaveScreen(fileName);
      *iret = ret;
      }
      break;

    case hostSelectorSessionClose: {
      const char *saveFileName = sys_va_arg(ap, char *);
      HostErrType ret = HostSessionClose(saveFileName);
      *iret = ret;
      }
      break;

    case hostSelectorSessionCreate: {
      const char *device = sys_va_arg(ap, char *);
      long ramSize = sys_va_arg(ap, long);
      const char *romPath = sys_va_arg(ap, char *);
      HostErrType ret = HostSessionCreate(device, ramSize, romPath);
      *iret = ret;
      }
      break;

    case hostSelectorSessionOpen: {
      const char *psfFileName = sys_va_arg(ap, char *);
      HostErrType ret = HostSessionOpen(psfFileName);
      *iret = ret;
      }
      break;

    case hostSelectorSessionQuit: {
      HostErrType ret = HostSessionQuit();
      *iret = ret;
      }
      break;

    case hostSelectorSessionSave: {
      const char *saveFileName = sys_va_arg(ap, char *);
      HostBoolType ret = HostSessionSave(saveFileName);
      *iret = ret;
      }
      break;

    case hostSelectorSetFileAttr: {
      const char *a = sys_va_arg(ap, char *);
      long b = sys_va_arg(ap, long);
      long ret = HostSetFileAttr(a, b);
      *iret = ret;
      }
      break;

    case hostSelectorSetLogFileSize: {
      long a = sys_va_arg(ap, long);
      HostSetLogFileSize(a);
      }
      break;

    case hostSelectorSetPreference: {
      const char *a = sys_va_arg(ap, char *);
      const char *b = sys_va_arg(ap, char *);
      HostSetPreference(a, b);
      }
      break;

    case hostSelectorSignalResume: {
      HostErrType ret = HostSignalResume();
      *iret = ret;
      }
      break;

    case hostSelectorSignalSend: {
      HostSignalType signalNumber = sys_va_arg(ap, HostSignalType);
      HostErrType ret = HostSignalSend(signalNumber);
      *iret = ret;
      }
      break;

    case hostSelectorSignalWait: {
      long timeout = sys_va_arg(ap, long);
      HostSignalType *signalNumber = sys_va_arg(ap, HostSignalType *);
      HostErrType ret = HostSignalWait(timeout, signalNumber);
      *iret = ret;
      }
      break;

    case hostSelectorSlotHasCard: {
      long slotNo = sys_va_arg(ap, long);
      HostBoolType ret = HostSlotHasCard(slotNo);
      *iret = ret;
      }
      break;

    case hostSelectorSlotMax: {
      long ret = HostSlotMax();
      *iret = ret;
      }
      break;

    case hostSelectorSlotRoot: {
      long slotNo = sys_va_arg(ap, long);
      const char *ret = HostSlotRoot(slotNo);
      *pret = (void *)ret;
      }
      break;

    case hostSelectorStat: {
      const char *a = sys_va_arg(ap, char *);
      HostStatType *b = sys_va_arg(ap, HostStatType *);
      long ret = HostStat(a, b);
      *iret = ret;
      }
      break;

    case hostSelectorStrFTime: {
      char *a = sys_va_arg(ap, char *);
      HostSizeType b = sys_va_arg(ap, HostSizeType);
      const char *c = sys_va_arg(ap, char *);
      const HostTmType *d = sys_va_arg(ap, HostTmType *);
      HostSizeType ret = HostStrFTime(a, b, c, d);
      *iret = ret;
      }
      break;

    case hostSelectorTime: {
      HostTimeType *a = sys_va_arg(ap, HostTimeType *);
      HostTimeType ret = HostTime(a);
      *iret = ret;
      }
      break;

    case hostSelectorTmpFile: {
      HostFILEType *ret = HostTmpFile();
      *pret = (void *)ret;
      }
      break;

    case hostSelectorTmpNam: {
      char *nameP = sys_va_arg(ap, char *);
      char *ret = HostTmpNam(nameP);
      *pret = (void *)ret;
      }
      break;

    case hostSelectorTraceClose: {
      HostTraceClose();
      }
      break;

    case hostSelectorTraceInit: {
      HostTraceInit();
      }
      break;

    case hostSelectorTraceOutputB: {
      UInt16 a = sys_va_arg(ap, UInt32);
      const void *b = sys_va_arg(ap, void *);
      HostSizeType c = sys_va_arg(ap, HostSizeType);
      HostTraceOutputB(a, b, c);
      }
      break;

    case hostSelectorTraceOutputT: {
      UInt16 a = sys_va_arg(ap, UInt32);
      const char *b = sys_va_arg(ap, char *);
      HostTraceOutputT(a, b, ap);
      }
      break;

    case hostSelectorTraceOutputTL: {
      UInt16 a = sys_va_arg(ap, UInt32);
      const char *b = sys_va_arg(ap, char *);
      HostTraceOutputTL(a, b, ap);
      }
      break;

    case hostSelectorTraceOutputVT: {
      UInt16 a = sys_va_arg(ap, UInt32);
      const char *b = sys_va_arg(ap, char *);
      char *va = sys_va_arg(ap, char *);
      HostTraceOutputVT(a, b, va);
      }
      break;

    case hostSelectorTraceOutputVTL: {
      UInt16 a = sys_va_arg(ap, UInt32);
      const char *b = sys_va_arg(ap, char *);
      char *va = sys_va_arg(ap, char *);
      HostTraceOutputVTL(a, b, va);
      }
      break;

    case hostSelectorTruncate: {
      const char *a = sys_va_arg(ap, char *);
      long b = sys_va_arg(ap, long);
      long ret = HostTruncate(a, b);
      *iret = ret;
      }
      break;

    case hostSelectorUTime: {
      const char *a = sys_va_arg(ap, char *);
      HostUTimeType *b = sys_va_arg(ap, HostUTimeType *);
      long ret = HostUTime(a, b);
      *iret = ret;
      }
      break;

    }
    }
    break;

    case sysTrapIntlDispatch: {
    switch (sel) {
    case intlIntlGetRoutineAddress: {
      IntlSelector inSelector = sys_va_arg(ap, UInt32);
      void *ret = IntlGetRoutineAddress(inSelector);
      *pret = (void *)ret;
      }
      break;

    case intlIntlSetRoutineAddress: {
      IntlSelector iSelector = sys_va_arg(ap, UInt32);
      void *iProcPtr = sys_va_arg(ap, void *);
      Err ret = IntlSetRoutineAddress(iSelector, iProcPtr);
      *iret = ret;
      }
      break;

    case intlTxtByteAttr: {
      UInt8 inByte = sys_va_arg(ap, UInt32);
      UInt8 ret = TxtByteAttr(inByte);
      *iret = ret;
      }
      break;

    case intlTxtCaselessCompare: {
      const Char *s1 = sys_va_arg(ap, Char *);
      UInt16 s1Len = sys_va_arg(ap, UInt32);
      UInt16 *s1MatchLen = sys_va_arg(ap, UInt16 *);
      const Char *s2 = sys_va_arg(ap, Char *);
      UInt16 s2Len = sys_va_arg(ap, UInt32);
      UInt16 *s2MatchLen = sys_va_arg(ap, UInt16 *);
      Int16 ret = TxtCaselessCompare(s1, s1Len, s1MatchLen, s2, s2Len, s2MatchLen);
      *iret = ret;
      }
      break;

    case intlTxtCharAttr: {
      WChar inChar = sys_va_arg(ap, UInt32);
      UInt16 ret = TxtCharAttr(inChar);
      *iret = ret;
      }
      break;

    case intlTxtCharBounds: {
      const Char *inText = sys_va_arg(ap, Char *);
      UInt32 inOffset = sys_va_arg(ap, UInt32);
      UInt32 *outStart = sys_va_arg(ap, UInt32 *);
      UInt32 *outEnd = sys_va_arg(ap, UInt32 *);
      WChar ret = TxtCharBounds(inText, inOffset, outStart, outEnd);
      *iret = ret;
      }
      break;

    case intlTxtCharEncoding: {
      WChar inChar = sys_va_arg(ap, UInt32);
      CharEncodingType ret = TxtCharEncoding(inChar);
      *iret = ret;
      }
      break;

    case intlTxtCharIsValid: {
      WChar inChar = sys_va_arg(ap, UInt32);
      Boolean ret = TxtCharIsValid(inChar);
      *iret = ret;
      }
      break;

    case intlTxtCharSize: {
      WChar inChar = sys_va_arg(ap, UInt32);
      UInt16 ret = TxtCharSize(inChar);
      *iret = ret;
      }
      break;

    case intlTxtCharWidth: {
      WChar inChar = sys_va_arg(ap, UInt32);
      Int16 ret = TxtCharWidth(inChar);
      *iret = ret;
      }
      break;

    case intlTxtCharXAttr: {
      WChar inChar = sys_va_arg(ap, UInt32);
      UInt16 ret = TxtCharXAttr(inChar);
      *iret = ret;
      }
      break;

    case intlTxtCompare: {
      const Char *s1 = sys_va_arg(ap, Char *);
      UInt16 s1Len = sys_va_arg(ap, UInt32);
      UInt16 *s1MatchLen = sys_va_arg(ap, UInt16 *);
      const Char *s2 = sys_va_arg(ap, Char *);
      UInt16 s2Len = sys_va_arg(ap, UInt32);
      UInt16 *s2MatchLen = sys_va_arg(ap, UInt16 *);
      Int16 ret = TxtCompare(s1, s1Len, s1MatchLen, s2, s2Len, s2MatchLen);
      *iret = ret;
      }
      break;

    case intlTxtConvertEncoding: {
      Boolean newConversion = sys_va_arg(ap, UInt32);
      TxtConvertStateType *ioStateP = sys_va_arg(ap, TxtConvertStateType *);
      const Char *srcTextP = sys_va_arg(ap, Char *);
      UInt16 *ioSrcBytes = sys_va_arg(ap, UInt16 *);
      CharEncodingType srcEncoding = sys_va_arg(ap, UInt32);
      Char *dstTextP = sys_va_arg(ap, Char *);
      UInt16 *ioDstBytes = sys_va_arg(ap, UInt16 *);
      CharEncodingType dstEncoding = sys_va_arg(ap, UInt32);
      const Char *substitutionStr = sys_va_arg(ap, Char *);
      UInt16 substitutionLen = sys_va_arg(ap, UInt32);
      Err ret = TxtConvertEncoding(newConversion, ioStateP, srcTextP, ioSrcBytes, srcEncoding, dstTextP, ioDstBytes, dstEncoding, substitutionStr, substitutionLen);
      *iret = ret;
      }
      break;

    case intlTxtEncodingName: {
      CharEncodingType inEncoding = sys_va_arg(ap, UInt32);
      const Char *ret = TxtEncodingName(inEncoding);
      *pret = (void *)ret;
      }
      break;

    case intlTxtFindString: {
      const Char *inSourceStr = sys_va_arg(ap, Char *);
      const Char *inTargetStr = sys_va_arg(ap, Char *);
      UInt32 *outPos = sys_va_arg(ap, UInt32 *);
      UInt16 *outLength = sys_va_arg(ap, UInt16 *);
      Boolean ret = TxtFindString(inSourceStr, inTargetStr, outPos, outLength);
      *iret = ret;
      }
      break;

    case intlTxtGetChar: {
      const Char *inText = sys_va_arg(ap, Char *);
      UInt32 inOffset = sys_va_arg(ap, UInt32);
      WChar ret = TxtGetChar(inText, inOffset);
      *iret = ret;
      }
      break;

    case intlTxtGetNextChar: {
      const Char *inText = sys_va_arg(ap, Char *);
      UInt32 inOffset = sys_va_arg(ap, UInt32);
      WChar *outChar = sys_va_arg(ap, WChar *);
      UInt16 ret = TxtGetNextChar(inText, inOffset, outChar);
      *iret = ret;
      }
      break;

    case intlTxtGetPreviousChar: {
      const Char *inText = sys_va_arg(ap, Char *);
      UInt32 inOffset = sys_va_arg(ap, UInt32);
      WChar *outChar = sys_va_arg(ap, WChar *);
      UInt16 ret = TxtGetPreviousChar(inText, inOffset, outChar);
      *iret = ret;
      }
      break;

    case intlTxtGetTruncationOffset: {
      const Char *inText = sys_va_arg(ap, Char *);
      UInt32 inOffset = sys_va_arg(ap, UInt32);
      UInt32 ret = TxtGetTruncationOffset(inText, inOffset);
      *iret = ret;
      }
      break;

    case intlTxtGetWordWrapOffset: {
      const Char *iTextP = sys_va_arg(ap, Char *);
      UInt32 iOffset = sys_va_arg(ap, UInt32);
      UInt32 ret = TxtGetWordWrapOffset(iTextP, iOffset);
      *iret = ret;
      }
      break;

    case intlTxtMaxEncoding: {
      CharEncodingType a = sys_va_arg(ap, UInt32);
      CharEncodingType b = sys_va_arg(ap, UInt32);
      CharEncodingType ret = TxtMaxEncoding(a, b);
      *iret = ret;
      }
      break;

    case intlTxtNameToEncoding: {
      const Char *iEncodingName = sys_va_arg(ap, Char *);
      CharEncodingType ret = TxtNameToEncoding(iEncodingName);
      *iret = ret;
      }
      break;

    case intlTxtParamString: {
      const Char *inTemplate = sys_va_arg(ap, Char *);
      const Char *param0 = sys_va_arg(ap, Char *);
      const Char *param1 = sys_va_arg(ap, Char *);
      const Char *param2 = sys_va_arg(ap, Char *);
      const Char *param3 = sys_va_arg(ap, Char *);
      Char *ret = TxtParamString(inTemplate, param0, param1, param2, param3);
      *pret = (void *)ret;
      }
      break;

    case intlTxtReplaceStr: {
      Char *ioStr = sys_va_arg(ap, Char *);
      UInt16 inMaxLen = sys_va_arg(ap, UInt32);
      const Char *inParamStr = sys_va_arg(ap, Char *);
      UInt16 inParamNum = sys_va_arg(ap, UInt32);
      UInt16 ret = TxtReplaceStr(ioStr, inMaxLen, inParamStr, inParamNum);
      *iret = ret;
      }
      break;

    case intlTxtSetNextChar: {
      Char *ioText = sys_va_arg(ap, Char *);
      UInt32 inOffset = sys_va_arg(ap, UInt32);
      WChar inChar = sys_va_arg(ap, UInt32);
      UInt16 ret = TxtSetNextChar(ioText, inOffset, inChar);
      *iret = ret;
      }
      break;

    case intlTxtStrEncoding: {
      const Char *inStr = sys_va_arg(ap, Char *);
      CharEncodingType ret = TxtStrEncoding(inStr);
      *iret = ret;
      }
      break;

    case intlTxtTransliterate: {
      const Char *inSrcText = sys_va_arg(ap, Char *);
      UInt16 inSrcLength = sys_va_arg(ap, UInt32);
      Char *outDstText = sys_va_arg(ap, Char *);
      UInt16 *ioDstLength = sys_va_arg(ap, UInt16 *);
      TranslitOpType inOp = sys_va_arg(ap, UInt32);
      Err ret = TxtTransliterate(inSrcText, inSrcLength, outDstText, ioDstLength, inOp);
      *iret = ret;
      }
      break;

    case intlTxtWordBounds: {
      const Char *inText = sys_va_arg(ap, Char *);
      UInt32 inLength = sys_va_arg(ap, UInt32);
      UInt32 inOffset = sys_va_arg(ap, UInt32);
      UInt32 *outStart = sys_va_arg(ap, UInt32 *);
      UInt32 *outEnd = sys_va_arg(ap, UInt32 *);
      Boolean ret = TxtWordBounds(inText, inLength, inOffset, outStart, outEnd);
      *iret = ret;
      }
      break;

    case intlTxtUpperChar: {
      WChar inChar = sys_va_arg(ap, UInt32);
      WChar ret = TxtUpperChar(inChar);
      *iret = ret;
      }
      break;

    case intlTxtLowerChar: {
      WChar inChar = sys_va_arg(ap, UInt32);
      WChar ret = TxtLowerChar(inChar);
      *iret = ret;
      }
      break;

    }
    }
    break;

    case sysTrapLmDispatch: {
    switch (sel) {
    case lmGetLocaleSetting: {
      UInt16 iLocaleIndex = sys_va_arg(ap, UInt32);
      LmLocaleSettingChoice iChoice = sys_va_arg(ap, UInt32);
      void *oValue = sys_va_arg(ap, void *);
      UInt16 iValueSize = sys_va_arg(ap, UInt32);
      Err ret = LmGetLocaleSetting(iLocaleIndex, iChoice, oValue, iValueSize);
      *iret = ret;
      }
      break;

    case lmGetNumLocales: {
      UInt16 ret = LmGetNumLocales();
      *iret = ret;
      }
      break;

    case lmLocaleToIndex: {
      const LmLocaleType *iLocale = sys_va_arg(ap, LmLocaleType *);
      UInt16 *oLocaleIndex = sys_va_arg(ap, UInt16 *);
      Err ret = LmLocaleToIndex(iLocale, oLocaleIndex);
      *iret = ret;
      }
      break;

    }
    }
    break;

    case sysTrapOmDispatch: {
    switch (sel) {
    case omGetCurrentLocale: {
      LmLocaleType *currentLocale = sys_va_arg(ap, LmLocaleType *);
      OmGetCurrentLocale(currentLocale);
      }
      break;

    case omGetIndexedLocale: {
      UInt16 localeIndex = sys_va_arg(ap, UInt32);
      LmLocaleType *theLocale = sys_va_arg(ap, LmLocaleType *);
      Err ret = OmGetIndexedLocale(localeIndex, theLocale);
      *iret = ret;
      }
      break;

    case omGetNextSystemLocale: {
      Boolean iNewSearch = sys_va_arg(ap, UInt32);
      OmSearchStateType *ioStateInfoP = sys_va_arg(ap, OmSearchStateType *);
      LmLocaleType *oLocaleP = sys_va_arg(ap, LmLocaleType *);
      Err ret = OmGetNextSystemLocale(iNewSearch, ioStateInfoP, oLocaleP);
      *iret = ret;
      }
      break;

    case omGetRoutineAddress: {
      OmSelector inSelector = sys_va_arg(ap, UInt32);
      void *ret = OmGetRoutineAddress(inSelector);
      *pret = (void *)ret;
      }
      break;

    case omGetSystemLocale: {
      LmLocaleType *systemLocale = sys_va_arg(ap, LmLocaleType *);
      OmGetSystemLocale(systemLocale);
      }
      break;

    case omLocaleToOverlayDBName: {
      const Char *baseDBName = sys_va_arg(ap, Char *);
      const LmLocaleType *targetLocale = sys_va_arg(ap, LmLocaleType *);
      Char *overlayDBName = sys_va_arg(ap, Char *);
      Err ret = OmLocaleToOverlayDBName(baseDBName, targetLocale, overlayDBName);
      *iret = ret;
      }
      break;

    case omOverlayDBNameToLocale: {
      const Char *overlayDBName = sys_va_arg(ap, Char *);
      LmLocaleType *overlayLocale = sys_va_arg(ap, LmLocaleType *);
      Err ret = OmOverlayDBNameToLocale(overlayDBName, overlayLocale);
      *iret = ret;
      }
      break;

    case omSetSystemLocale: {
      const LmLocaleType *systemLocale = sys_va_arg(ap, LmLocaleType *);
      Err ret = OmSetSystemLocale(systemLocale);
      *iret = ret;
      }
      break;

    }
    }
    break;

    case sysTrapPinsDispatch: {
    switch (sel) {
    case pinFrmGetDIAPolicyAttr: {
      FormPtr formP = sys_va_arg(ap, FormPtr);
      UInt16 ret = FrmGetDIAPolicyAttr(formP);
      *iret = ret;
      }
      break;

    case pinFrmSetDIAPolicyAttr: {
      FormPtr formP = sys_va_arg(ap, FormPtr);
      UInt16 diaPolicy = sys_va_arg(ap, UInt32);
      Err ret = FrmSetDIAPolicyAttr(formP, diaPolicy);
      *iret = ret;
      }
      break;

    case pinPINGetInputAreaState: {
      UInt16 ret = PINGetInputAreaState();
      *iret = ret;
      }
      break;

    case pinPINGetInputTriggerState: {
      UInt16 ret = PINGetInputTriggerState();
      *iret = ret;
      }
      break;

    case pinPINSetInputAreaState: {
      UInt16 state = sys_va_arg(ap, UInt32);
      Err ret = PINSetInputAreaState(state);
      *iret = ret;
      }
      break;

    case pinPINSetInputTriggerState: {
      UInt16 state = sys_va_arg(ap, UInt32);
      Err ret = PINSetInputTriggerState(state);
      *iret = ret;
      }
      break;

    case pinStatGetAttribute: {
      UInt16 selector = sys_va_arg(ap, UInt32);
      UInt32 *dataP = sys_va_arg(ap, UInt32 *);
      Err ret = StatGetAttribute(selector, dataP);
      *iret = ret;
      }
      break;

    case pinStatHide: {
      Err ret = StatHide();
      *iret = ret;
      }
      break;

    case pinStatShow: {
      Err ret = StatShow();
      *iret = ret;
      }
      break;

    case pinSysGetOrientation: {
      UInt16 ret = SysGetOrientation();
      *iret = ret;
      }
      break;

    case pinSysGetOrientationTriggerState: {
      UInt16 ret = SysGetOrientationTriggerState();
      *iret = ret;
      }
      break;

    case pinSysSetOrientation: {
      UInt16 orientation = sys_va_arg(ap, UInt32);
      Err ret = SysSetOrientation(orientation);
      *iret = ret;
      }
      break;

    case pinSysSetOrientationTriggerState: {
      UInt16 triggerState = sys_va_arg(ap, UInt32);
      Err ret = SysSetOrientationTriggerState(triggerState);
      *iret = ret;
      }
      break;

    case pinWinSetConstraintsSize: {
      WinHandle winH = sys_va_arg(ap, WinHandle);
      Coord minH = sys_va_arg(ap, Int32);
      Coord prefH = sys_va_arg(ap, Int32);
      Coord maxH = sys_va_arg(ap, Int32);
      Coord minW = sys_va_arg(ap, Int32);
      Coord prefW = sys_va_arg(ap, Int32);
      Coord maxW = sys_va_arg(ap, Int32);
      Err ret = WinSetConstraintsSize(winH, minH, prefH, maxH, minW, prefW, maxW);
      *iret = ret;
      }
      break;

    }
    }
    break;

    case sysTrapSerialDispatch: {
    switch (sel) {
    case sysSerialClearErr: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      Err ret = SrmClearErr(portId);
      *iret = ret;
      }
      break;

    case sysSerialClose: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      Err ret = SrmClose(portId);
      *iret = ret;
      }
      break;

    case sysSerialControl: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      UInt16 op = sys_va_arg(ap, UInt32);
      void *valueP = sys_va_arg(ap, void *);
      UInt16 *valueLenP = sys_va_arg(ap, UInt16 *);
      Err ret = SrmControl(portId, op, valueP, valueLenP);
      *iret = ret;
      }
      break;

    case sysSerialCustomControl: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      UInt16 opCode = sys_va_arg(ap, UInt32);
      UInt32 creator = sys_va_arg(ap, UInt32);
      void *valueP = sys_va_arg(ap, void *);
      UInt16 *valueLenP = sys_va_arg(ap, UInt16 *);
      Err ret = SrmCustomControl(portId, opCode, creator, valueP, valueLenP);
      *iret = ret;
      }
      break;

    case sysSerialGetDeviceCount: {
      UInt16 *numOfDevicesP = sys_va_arg(ap, UInt16 *);
      Err ret = SrmGetDeviceCount(numOfDevicesP);
      *iret = ret;
      }
      break;

    case sysSerialGetDeviceInfo: {
      UInt32 deviceID = sys_va_arg(ap, UInt32);
      DeviceInfoType *deviceInfoP = sys_va_arg(ap, DeviceInfoType *);
      Err ret = SrmGetDeviceInfo(deviceID, deviceInfoP);
      *iret = ret;
      }
      break;

    case sysSerialGetStatus: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      UInt32 *statusFieldP = sys_va_arg(ap, UInt32 *);
      UInt16 *lineErrsP = sys_va_arg(ap, UInt16 *);
      Err ret = SrmGetStatus(portId, statusFieldP, lineErrsP);
      *iret = ret;
      }
      break;

    case sysSerialInstall: {
      Err ret = SerialMgrInstall();
      *iret = ret;
      }
      break;

    case sysSerialOpen: {
      UInt32 port = sys_va_arg(ap, UInt32);
      UInt32 baud = sys_va_arg(ap, UInt32);
      UInt16 *newPortIdP = sys_va_arg(ap, UInt16 *);
      Err ret = SrmOpen(port, baud, newPortIdP);
      *iret = ret;
      }
      break;

    case sysSerialOpenBkgnd: {
      UInt32 port = sys_va_arg(ap, UInt32);
      UInt32 baud = sys_va_arg(ap, UInt32);
      UInt16 *newPortIdP = sys_va_arg(ap, UInt16 *);
      Err ret = SrmOpenBackground(port, baud, newPortIdP);
      *iret = ret;
      }
      break;

    case sysSerialOpenBkgndV4: {
      UInt32 port = sys_va_arg(ap, UInt32);
      SrmOpenConfigType *configP = sys_va_arg(ap, SrmOpenConfigType *);
      UInt16 configSize = sys_va_arg(ap, UInt32);
      UInt16 *newPortIdP = sys_va_arg(ap, UInt16 *);
      Err ret = SrmExtOpenBackground(port, configP, configSize, newPortIdP);
      *iret = ret;
      }
      break;

    case sysSerialOpenV4: {
      UInt32 port = sys_va_arg(ap, UInt32);
      SrmOpenConfigType *configP = sys_va_arg(ap, SrmOpenConfigType *);
      UInt16 configSize = sys_va_arg(ap, UInt32);
      UInt16 *newPortIdP = sys_va_arg(ap, UInt16 *);
      Err ret = SrmExtOpen(port, configP, configSize, newPortIdP);
      *iret = ret;
      }
      break;

    case sysSerialPrimeWakeupHandler: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      UInt16 minBytes = sys_va_arg(ap, UInt32);
      Err ret = SrmPrimeWakeupHandler(portId, minBytes);
      *iret = ret;
      }
      break;

    case sysSerialRcvWindowClose: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      UInt32 bytesPulled = sys_va_arg(ap, UInt32);
      Err ret = SrmReceiveWindowClose(portId, bytesPulled);
      *iret = ret;
      }
      break;

    case sysSerialRcvWindowOpen: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      UInt8 * *bufPP = sys_va_arg(ap, UInt8 * *);
      UInt32 *sizeP = sys_va_arg(ap, UInt32 *);
      Err ret = SrmReceiveWindowOpen(portId, bufPP, sizeP);
      *iret = ret;
      }
      break;

    case sysSerialReceive: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      void *rcvBufP = sys_va_arg(ap, void *);
      UInt32 count = sys_va_arg(ap, UInt32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      UInt32 ret = SrmReceive(portId, rcvBufP, count, timeout, errP);
      *iret = ret;
      }
      break;

    case sysSerialReceiveCheck: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      UInt32 *numBytesP = sys_va_arg(ap, UInt32 *);
      Err ret = SrmReceiveCheck(portId, numBytesP);
      *iret = ret;
      }
      break;

    case sysSerialReceiveFlush: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err ret = SrmReceiveFlush(portId, timeout);
      *iret = ret;
      }
      break;

    case sysSerialReceiveWait: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      UInt32 bytes = sys_va_arg(ap, UInt32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err ret = SrmReceiveWait(portId, bytes, timeout);
      *iret = ret;
      }
      break;

    case sysSerialSend: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      const void *bufP = sys_va_arg(ap, void *);
      UInt32 count = sys_va_arg(ap, UInt32);
      Err *errP = sys_va_arg(ap, Err *);
      UInt32 ret = SrmSend(portId, bufP, count, errP);
      *iret = ret;
      }
      break;

    case sysSerialSendCheck: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      UInt32 *numBytesP = sys_va_arg(ap, UInt32 *);
      Err ret = SrmSendCheck(portId, numBytesP);
      *iret = ret;
      }
      break;

    case sysSerialSendFlush: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      Err ret = SrmSendFlush(portId);
      *iret = ret;
      }
      break;

    case sysSerialSendWait: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      Err ret = SrmSendWait(portId);
      *iret = ret;
      }
      break;

    case sysSerialSetRcvBuffer: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      void *bufP = sys_va_arg(ap, void *);
      UInt16 bufSize = sys_va_arg(ap, UInt32);
      Err ret = SrmSetReceiveBuffer(portId, bufP, bufSize);
      *iret = ret;
      }
      break;

    case sysSerialSetWakeupHandler: {
      UInt16 portId = sys_va_arg(ap, UInt32);
      WakeupHandlerProcPtr procP = sys_va_arg(ap, WakeupHandlerProcPtr);
      UInt32 refCon = sys_va_arg(ap, UInt32);
      Err ret = SrmSetWakeupHandler(portId, procP, refCon);
      *iret = ret;
      }
      break;

    case sysSerialSleep: {
      Err ret = SrmSleep();
      *iret = ret;
      }
      break;

    case sysSerialWake: {
      Err ret = SrmWake();
      *iret = ret;
      }
      break;

    }
    }
    break;

    case sysTrapTsmDispatch: {
    switch (sel) {
    case tsmGetFepMode: {
      void *nullParam = sys_va_arg(ap, void *);
      TsmFepModeType ret = TsmGetFepMode(nullParam);
      *iret = ret;
      }
      break;

    case tsmSetFepMode: {
      void *nullParam = sys_va_arg(ap, void *);
      TsmFepModeType inNewMode = sys_va_arg(ap, UInt32);
      TsmFepModeType ret = TsmSetFepMode(nullParam, inNewMode);
      *iret = ret;
      }
      break;

    }
    }
    break;

    case sysTrapUdaMgrDispatch: {
    switch (sel) {
    case sysUdaControl: {
      UDAObjectType *ioObject = sys_va_arg(ap, UDAObjectType *);
      UInt16 parameter = sys_va_arg(ap, UInt32);
      Err ret = UDAControl(ioObject, parameter, ap);
      *iret = ret;
      }
      break;

    case sysUdaExchangeReaderNew: {
      ExgSocketType *socket = sys_va_arg(ap, ExgSocketType *);
      UDAReaderType *ret = UDAExchangeReaderNew(socket);
      *pret = (void *)ret;
      }
      break;

    case sysUdaExchangeWriterNew: {
      ExgSocketType *socket = sys_va_arg(ap, ExgSocketType *);
      UDABufferSize bufferSize = sys_va_arg(ap, UInt32);
      UDAWriterType *ret = UDAExchangeWriterNew(socket, bufferSize);
      *pret = (void *)ret;
      }
      break;

    case sysUdaMemoryReaderNew: {
      const UInt8 *bufferP = sys_va_arg(ap, UInt8 *);
      UDABufferSize bufferSizeInBytes = sys_va_arg(ap, UInt32);
      UDAReaderType *ret = UDAMemoryReaderNew(bufferP, bufferSizeInBytes);
      *pret = (void *)ret;
      }
      break;

    }
    }
    break;

    case sysTrapVFSMgr: {
    switch (sel) {
    case vfsTrapCustomControl: {
      UInt32 fsCreator = sys_va_arg(ap, UInt32);
      UInt32 apiCreator = sys_va_arg(ap, UInt32);
      UInt16 apiSelector = sys_va_arg(ap, UInt32);
      void *valueP = sys_va_arg(ap, void *);
      UInt16 *valueLenP = sys_va_arg(ap, UInt16 *);
      Err ret = VFSCustomControl(fsCreator, apiCreator, apiSelector, valueP, valueLenP);
      *iret = ret;
      }
      break;

    case vfsTrapDirCreate: {
      UInt16 volRefNum = sys_va_arg(ap, UInt32);
      const Char *dirNameP = sys_va_arg(ap, Char *);
      Err ret = VFSDirCreate(volRefNum, dirNameP);
      *iret = ret;
      }
      break;

    case vfsTrapDirEntryEnumerate: {
      FileRef dirRef = sys_va_arg(ap, FileRef);
      UInt32 *dirEntryIteratorP = sys_va_arg(ap, UInt32 *);
      FileInfoType *infoP = sys_va_arg(ap, FileInfoType *);
      Err ret = VFSDirEntryEnumerate(dirRef, dirEntryIteratorP, infoP);
      *iret = ret;
      }
      break;

    case vfsTrapExportDatabaseToFile: {
      UInt16 volRefNum = sys_va_arg(ap, UInt32);
      const Char *pathNameP = sys_va_arg(ap, Char *);
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      Err ret = VFSExportDatabaseToFile(volRefNum, pathNameP, cardNo, dbID);
      *iret = ret;
      }
      break;

    case vfsTrapExportDatabaseToFileCustom: {
      UInt16 volRefNum = sys_va_arg(ap, UInt32);
      const Char *pathNameP = sys_va_arg(ap, Char *);
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      VFSExportProcPtr exportProcP = sys_va_arg(ap, VFSExportProcPtr);
      void *userDataP = sys_va_arg(ap, void *);
      Err ret = VFSExportDatabaseToFileCustom(volRefNum, pathNameP, cardNo, dbID, exportProcP, userDataP);
      *iret = ret;
      }
      break;

    case vfsTrapFileClose: {
      FileRef fileRef = sys_va_arg(ap, FileRef);
      Err ret = VFSFileClose(fileRef);
      *iret = ret;
      }
      break;

    case vfsTrapFileCreate: {
      UInt16 volRefNum = sys_va_arg(ap, UInt32);
      const Char *pathNameP = sys_va_arg(ap, Char *);
      Err ret = VFSFileCreate(volRefNum, pathNameP);
      *iret = ret;
      }
      break;

    case vfsTrapFileDBGetRecord: {
      FileRef ref = sys_va_arg(ap, FileRef);
      UInt16 recIndex = sys_va_arg(ap, UInt32);
      MemHandle *recHP = sys_va_arg(ap, MemHandle *);
      UInt8 *recAttrP = sys_va_arg(ap, UInt8 *);
      UInt32 *uniqueIDP = sys_va_arg(ap, UInt32 *);
      Err ret = VFSFileDBGetRecord(ref, recIndex, recHP, recAttrP, uniqueIDP);
      *iret = ret;
      }
      break;

    case vfsTrapFileDBGetResource: {
      FileRef ref = sys_va_arg(ap, FileRef);
      DmResType type = sys_va_arg(ap, DmResType);
      DmResID resID = sys_va_arg(ap, UInt32);
      MemHandle *resHP = sys_va_arg(ap, MemHandle *);
      Err ret = VFSFileDBGetResource(ref, type, resID, resHP);
      *iret = ret;
      }
      break;

    case vfsTrapFileDBInfo: {
      FileRef ref = sys_va_arg(ap, FileRef);
      Char *nameP = sys_va_arg(ap, Char *);
      UInt16 *attributesP = sys_va_arg(ap, UInt16 *);
      UInt16 *versionP = sys_va_arg(ap, UInt16 *);
      UInt32 *crDateP = sys_va_arg(ap, UInt32 *);
      UInt32 *modDateP = sys_va_arg(ap, UInt32 *);
      UInt32 *bckUpDateP = sys_va_arg(ap, UInt32 *);
      UInt32 *modNumP = sys_va_arg(ap, UInt32 *);
      MemHandle *appInfoHP = sys_va_arg(ap, MemHandle *);
      MemHandle *sortInfoHP = sys_va_arg(ap, MemHandle *);
      UInt32 *typeP = sys_va_arg(ap, UInt32 *);
      UInt32 *creatorP = sys_va_arg(ap, UInt32 *);
      UInt16 *numRecordsP = sys_va_arg(ap, UInt16 *);
      Err ret = VFSFileDBInfo(ref, nameP, attributesP, versionP, crDateP, modDateP, bckUpDateP, modNumP, appInfoHP, sortInfoHP, typeP, creatorP, numRecordsP);
      *iret = ret;
      }
      break;

    case vfsTrapFileDelete: {
      UInt16 volRefNum = sys_va_arg(ap, UInt32);
      const Char *pathNameP = sys_va_arg(ap, Char *);
      Err ret = VFSFileDelete(volRefNum, pathNameP);
      *iret = ret;
      }
      break;

    case vfsTrapFileEOF: {
      FileRef fileRef = sys_va_arg(ap, FileRef);
      Err ret = VFSFileEOF(fileRef);
      *iret = ret;
      }
      break;

    case vfsTrapFileGetAttributes: {
      FileRef fileRef = sys_va_arg(ap, FileRef);
      UInt32 *attributesP = sys_va_arg(ap, UInt32 *);
      Err ret = VFSFileGetAttributes(fileRef, attributesP);
      *iret = ret;
      }
      break;

    case vfsTrapFileGetDate: {
      FileRef fileRef = sys_va_arg(ap, FileRef);
      UInt16 whichDate = sys_va_arg(ap, UInt32);
      UInt32 *dateP = sys_va_arg(ap, UInt32 *);
      Err ret = VFSFileGetDate(fileRef, whichDate, dateP);
      *iret = ret;
      }
      break;

    case vfsTrapFileOpen: {
      UInt16 volRefNum = sys_va_arg(ap, UInt32);
      const Char *pathNameP = sys_va_arg(ap, Char *);
      UInt16 openMode = sys_va_arg(ap, UInt32);
      FileRef *fileRefP = sys_va_arg(ap, FileRef *);
      Err ret = VFSFileOpen(volRefNum, pathNameP, openMode, fileRefP);
      *iret = ret;
      }
      break;

    case vfsTrapFileRead: {
      FileRef fileRef = sys_va_arg(ap, FileRef);
      UInt32 numBytes = sys_va_arg(ap, UInt32);
      void *bufP = sys_va_arg(ap, void *);
      UInt32 *numBytesReadP = sys_va_arg(ap, UInt32 *);
      Err ret = VFSFileRead(fileRef, numBytes, bufP, numBytesReadP);
      *iret = ret;
      }
      break;

    case vfsTrapFileReadData: {
      FileRef fileRef = sys_va_arg(ap, FileRef);
      UInt32 numBytes = sys_va_arg(ap, UInt32);
      void *bufBaseP = sys_va_arg(ap, void *);
      UInt32 offset = sys_va_arg(ap, UInt32);
      UInt32 *numBytesReadP = sys_va_arg(ap, UInt32 *);
      Err ret = VFSFileReadData(fileRef, numBytes, bufBaseP, offset, numBytesReadP);
      *iret = ret;
      }
      break;

    case vfsTrapFileRename: {
      UInt16 volRefNum = sys_va_arg(ap, UInt32);
      const Char *pathNameP = sys_va_arg(ap, Char *);
      const Char *newNameP = sys_va_arg(ap, Char *);
      Err ret = VFSFileRename(volRefNum, pathNameP, newNameP);
      *iret = ret;
      }
      break;

    case vfsTrapFileResize: {
      FileRef fileRef = sys_va_arg(ap, FileRef);
      UInt32 newSize = sys_va_arg(ap, UInt32);
      Err ret = VFSFileResize(fileRef, newSize);
      *iret = ret;
      }
      break;

    case vfsTrapFileSeek: {
      FileRef fileRef = sys_va_arg(ap, FileRef);
      FileOrigin origin = sys_va_arg(ap, UInt32);
      Int32 offset = sys_va_arg(ap, Int32);
      Err ret = VFSFileSeek(fileRef, origin, offset);
      *iret = ret;
      }
      break;

    case vfsTrapFileSetAttributes: {
      FileRef fileRef = sys_va_arg(ap, FileRef);
      UInt32 attributes = sys_va_arg(ap, UInt32);
      Err ret = VFSFileSetAttributes(fileRef, attributes);
      *iret = ret;
      }
      break;

    case vfsTrapFileSetDate: {
      FileRef fileRef = sys_va_arg(ap, FileRef);
      UInt16 whichDate = sys_va_arg(ap, UInt32);
      UInt32 date = sys_va_arg(ap, UInt32);
      Err ret = VFSFileSetDate(fileRef, whichDate, date);
      *iret = ret;
      }
      break;

    case vfsTrapFileSize: {
      FileRef fileRef = sys_va_arg(ap, FileRef);
      UInt32 *fileSizeP = sys_va_arg(ap, UInt32 *);
      Err ret = VFSFileSize(fileRef, fileSizeP);
      *iret = ret;
      }
      break;

    case vfsTrapFileTell: {
      FileRef fileRef = sys_va_arg(ap, FileRef);
      UInt32 *filePosP = sys_va_arg(ap, UInt32 *);
      Err ret = VFSFileTell(fileRef, filePosP);
      *iret = ret;
      }
      break;

    case vfsTrapFileWrite: {
      FileRef fileRef = sys_va_arg(ap, FileRef);
      UInt32 numBytes = sys_va_arg(ap, UInt32);
      const void *dataP = sys_va_arg(ap, void *);
      UInt32 *numBytesWrittenP = sys_va_arg(ap, UInt32 *);
      Err ret = VFSFileWrite(fileRef, numBytes, dataP, numBytesWrittenP);
      *iret = ret;
      }
      break;

    case vfsTrapGetDefaultDirectory: {
      UInt16 volRefNum = sys_va_arg(ap, UInt32);
      const Char *fileTypeStr = sys_va_arg(ap, Char *);
      Char *pathStr = sys_va_arg(ap, Char *);
      UInt16 *bufLenP = sys_va_arg(ap, UInt16 *);
      Err ret = VFSGetDefaultDirectory(volRefNum, fileTypeStr, pathStr, bufLenP);
      *iret = ret;
      }
      break;

    case vfsTrapImportDatabaseFromFile: {
      UInt16 volRefNum = sys_va_arg(ap, UInt32);
      const Char *pathNameP = sys_va_arg(ap, Char *);
      UInt16 *cardNoP = sys_va_arg(ap, UInt16 *);
      LocalID *dbIDP = sys_va_arg(ap, LocalID *);
      Err ret = VFSImportDatabaseFromFile(volRefNum, pathNameP, cardNoP, dbIDP);
      *iret = ret;
      }
      break;

    case vfsTrapImportDatabaseFromFileCustom: {
      UInt16 volRefNum = sys_va_arg(ap, UInt32);
      const Char *pathNameP = sys_va_arg(ap, Char *);
      UInt16 *cardNoP = sys_va_arg(ap, UInt16 *);
      LocalID *dbIDP = sys_va_arg(ap, LocalID *);
      VFSImportProcPtr importProcP = sys_va_arg(ap, VFSImportProcPtr);
      void *userDataP = sys_va_arg(ap, void *);
      Err ret = VFSImportDatabaseFromFileCustom(volRefNum, pathNameP, cardNoP, dbIDP, importProcP, userDataP);
      *iret = ret;
      }
      break;

    case vfsTrapInit: {
      Err ret = VFSInit();
      *iret = ret;
      }
      break;

    case vfsTrapInstallFSLib: {
      UInt32 creator = sys_va_arg(ap, UInt32);
      UInt16 *fsLibRefNumP = sys_va_arg(ap, UInt16 *);
      Err ret = VFSInstallFSLib(creator, fsLibRefNumP);
      *iret = ret;
      }
      break;

    case vfsTrapRegisterDefaultDirectory: {
      const Char *fileTypeStr = sys_va_arg(ap, Char *);
      UInt32 mediaType = sys_va_arg(ap, UInt32);
      const Char *pathStr = sys_va_arg(ap, Char *);
      Err ret = VFSRegisterDefaultDirectory(fileTypeStr, mediaType, pathStr);
      *iret = ret;
      }
      break;

    case vfsTrapRemoveFSLib: {
      UInt16 fsLibRefNum = sys_va_arg(ap, UInt32);
      Err ret = VFSRemoveFSLib(fsLibRefNum);
      *iret = ret;
      }
      break;

    case vfsTrapUnregisterDefaultDirectory: {
      const Char *fileTypeStr = sys_va_arg(ap, Char *);
      UInt32 mediaType = sys_va_arg(ap, UInt32);
      Err ret = VFSUnregisterDefaultDirectory(fileTypeStr, mediaType);
      *iret = ret;
      }
      break;

    case vfsTrapVolumeEnumerate: {
      UInt16 *volRefNumP = sys_va_arg(ap, UInt16 *);
      UInt32 *volIteratorP = sys_va_arg(ap, UInt32 *);
      Err ret = VFSVolumeEnumerate(volRefNumP, volIteratorP);
      *iret = ret;
      }
      break;

    case vfsTrapVolumeFormat: {
      UInt8 flags = sys_va_arg(ap, UInt32);
      UInt16 fsLibRefNum = sys_va_arg(ap, UInt32);
      VFSAnyMountParamPtr vfsMountParamP = sys_va_arg(ap, VFSAnyMountParamPtr);
      Err ret = VFSVolumeFormat(flags, fsLibRefNum, vfsMountParamP);
      *iret = ret;
      }
      break;

    case vfsTrapVolumeGetLabel: {
      UInt16 volRefNum = sys_va_arg(ap, UInt32);
      Char *labelP = sys_va_arg(ap, Char *);
      UInt16 bufLen = sys_va_arg(ap, UInt32);
      Err ret = VFSVolumeGetLabel(volRefNum, labelP, bufLen);
      *iret = ret;
      }
      break;

    case vfsTrapVolumeInfo: {
      UInt16 volRefNum = sys_va_arg(ap, UInt32);
      VolumeInfoType *volInfoP = sys_va_arg(ap, VolumeInfoType *);
      Err ret = VFSVolumeInfo(volRefNum, volInfoP);
      *iret = ret;
      }
      break;

    case vfsTrapVolumeMount: {
      UInt8 flags = sys_va_arg(ap, UInt32);
      UInt16 fsLibRefNum = sys_va_arg(ap, UInt32);
      VFSAnyMountParamPtr vfsMountParamP = sys_va_arg(ap, VFSAnyMountParamPtr);
      Err ret = VFSVolumeMount(flags, fsLibRefNum, vfsMountParamP);
      *iret = ret;
      }
      break;

    case vfsTrapVolumeSetLabel: {
      UInt16 volRefNum = sys_va_arg(ap, UInt32);
      const Char *labelP = sys_va_arg(ap, Char *);
      Err ret = VFSVolumeSetLabel(volRefNum, labelP);
      *iret = ret;
      }
      break;

    case vfsTrapVolumeSize: {
      UInt16 volRefNum = sys_va_arg(ap, UInt32);
      UInt32 *volumeUsedP = sys_va_arg(ap, UInt32 *);
      UInt32 *volumeTotalP = sys_va_arg(ap, UInt32 *);
      Err ret = VFSVolumeSize(volRefNum, volumeUsedP, volumeTotalP);
      *iret = ret;
      }
      break;

    case vfsTrapVolumeUnmount: {
      UInt16 volRefNum = sys_va_arg(ap, UInt32);
      Err ret = VFSVolumeUnmount(volRefNum);
      *iret = ret;
      }
      break;

    }
    }
    break;

    case sysLibTrapClose: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      Err ret = SysLibClose(refNum);
      *iret = ret;
      }
      break;

    case sysLibTrapOpen: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      Err ret = SysLibOpen(refNum);
      *iret = ret;
      }
      break;

    case sysLibTrapSleep: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      Err ret = SysLibSleep(refNum);
      *iret = ret;
      }
      break;

    case sysLibTrapWake: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      Err ret = SysLibWake(refNum);
      *iret = ret;
      }
      break;

    case sysTrapAbtShowAbout: {
      UInt32 creator = sys_va_arg(ap, UInt32);
      AbtShowAbout(creator);
      }
      break;

    case sysTrapAlmAlarmCallback: {
      AlmAlarmCallback();
      }
      break;

    case sysTrapAlmCancelAll: {
      AlmCancelAll();
      }
      break;

    case sysTrapAlmDisplayAlarm: {
      Boolean okToDisplay = sys_va_arg(ap, UInt32);
      Boolean ret = AlmDisplayAlarm(okToDisplay);
      *iret = ret;
      }
      break;

    case sysTrapAlmEnableNotification: {
      Boolean enable = sys_va_arg(ap, UInt32);
      AlmEnableNotification(enable);
      }
      break;

    case sysTrapAlmGetAlarm: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      UInt32 *refP = sys_va_arg(ap, UInt32 *);
      UInt32 ret = AlmGetAlarm(cardNo, dbID, refP);
      *iret = ret;
      }
      break;

    case sysTrapAlmInit: {
      Err ret = AlmInit();
      *iret = ret;
      }
      break;

    case sysTrapAlmSetAlarm: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      UInt32 ref = sys_va_arg(ap, UInt32);
      UInt32 alarmSeconds = sys_va_arg(ap, UInt32);
      Boolean quiet = sys_va_arg(ap, UInt32);
      Err ret = AlmSetAlarm(cardNo, dbID, ref, alarmSeconds, quiet);
      *iret = ret;
      }
      break;

    case sysTrapAlmTimeChange: {
      AlmTimeChange();
      }
      break;

    case sysTrapAttnDoSpecialEffects: {
      AttnFlagsType flags = sys_va_arg(ap, AttnFlagsType);
      Err ret = AttnDoSpecialEffects(flags);
      *iret = ret;
      }
      break;

    case sysTrapAttnForgetIt: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      UInt32 userData = sys_va_arg(ap, UInt32);
      Boolean ret = AttnForgetIt(cardNo, dbID, userData);
      *iret = ret;
      }
      break;

    case sysTrapAttnGetAttention: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      UInt32 userData = sys_va_arg(ap, UInt32);
      AttnCallbackProc *callbackFnP = sys_va_arg(ap, AttnCallbackProc *);
      AttnLevelType level = sys_va_arg(ap, UInt32);
      AttnFlagsType flags = sys_va_arg(ap, AttnFlagsType);
      UInt16 nagRateInSeconds = sys_va_arg(ap, UInt32);
      UInt16 nagRepeatLimit = sys_va_arg(ap, UInt32);
      Err ret = AttnGetAttention(cardNo, dbID, userData, callbackFnP, level, flags, nagRateInSeconds, nagRepeatLimit);
      *iret = ret;
      }
      break;

    case sysTrapAttnGetCounts: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      UInt16 *insistentCountP = sys_va_arg(ap, UInt16 *);
      UInt16 *subtleCountP = sys_va_arg(ap, UInt16 *);
      UInt16 ret = AttnGetCounts(cardNo, dbID, insistentCountP, subtleCountP);
      *iret = ret;
      }
      break;

    case sysTrapAttnIndicatorEnable: {
      Boolean enableIt = sys_va_arg(ap, UInt32);
      AttnIndicatorEnable(enableIt);
      }
      break;

    case sysTrapAttnIndicatorEnabled: {
      Boolean ret = AttnIndicatorEnabled();
      *iret = ret;
      }
      break;

    case sysTrapAttnIterate: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      UInt32 iterationData = sys_va_arg(ap, UInt32);
      AttnIterate(cardNo, dbID, iterationData);
      }
      break;

    case sysTrapAttnListOpen: {
      AttnListOpen();
      }
      break;

    case sysTrapAttnUpdate: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      UInt32 userData = sys_va_arg(ap, UInt32);
      AttnCallbackProc *callbackFnP = sys_va_arg(ap, AttnCallbackProc *);
      AttnFlagsType *flagsP = sys_va_arg(ap, AttnFlagsType *);
      UInt16 *nagRateInSecondsP = sys_va_arg(ap, UInt16 *);
      UInt16 *nagRepeatLimitP = sys_va_arg(ap, UInt16 *);
      Boolean ret = AttnUpdate(cardNo, dbID, userData, callbackFnP, flagsP, nagRateInSecondsP, nagRepeatLimitP);
      *iret = ret;
      }
      break;

    case sysTrapBmpBitsSize: {
      const BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      UInt16 ret = BmpBitsSize(bitmapP);
      *iret = ret;
      }
      break;

    case sysTrapBmpColortableSize: {
      const BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      UInt16 ret = BmpColortableSize(bitmapP);
      *iret = ret;
      }
      break;

    case sysTrapBmpCompress: {
      BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      BitmapCompressionType compType = sys_va_arg(ap, BitmapCompressionType);
      Err ret = BmpCompress(bitmapP, compType);
      *iret = ret;
      }
      break;

    case sysTrapBmpCreate: {
      Coord width = sys_va_arg(ap, Int32);
      Coord height = sys_va_arg(ap, Int32);
      UInt8 depth = sys_va_arg(ap, UInt32);
      ColorTableType *colortableP = sys_va_arg(ap, ColorTableType *);
      UInt16 *error = sys_va_arg(ap, UInt16 *);
      BitmapType *ret = BmpCreate(width, height, depth, colortableP, error);
      *pret = (void *)ret;
      }
      break;

    case sysTrapBmpDelete: {
      BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      Err ret = BmpDelete(bitmapP);
      *iret = ret;
      }
      break;

    case sysTrapBmpGetBitDepth: {
      const BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      UInt8 ret = BmpGetBitDepth(bitmapP);
      *iret = ret;
      }
      break;

    case sysTrapBmpGetBits: {
      BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      void *ret = BmpGetBits(bitmapP);
      *pret = (void *)ret;
      }
      break;

    case sysTrapBmpGetColortable: {
      BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      ColorTableType *ret = BmpGetColortable(bitmapP);
      *pret = (void *)ret;
      }
      break;

    case sysTrapBmpGetDimensions: {
      const BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      Coord *widthP = sys_va_arg(ap, Coord *);
      Coord *heightP = sys_va_arg(ap, Coord *);
      UInt16 *rowBytesP = sys_va_arg(ap, UInt16 *);
      BmpGetDimensions(bitmapP, widthP, heightP, rowBytesP);
      }
      break;

    case sysTrapBmpGetNextBitmap: {
      BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      BitmapType *ret = BmpGetNextBitmap(bitmapP);
      *pret = (void *)ret;
      }
      break;

    case sysTrapBmpGetSizes: {
      const BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      UInt32 *dataSizeP = sys_va_arg(ap, UInt32 *);
      UInt32 *headerSizeP = sys_va_arg(ap, UInt32 *);
      BmpGetSizes(bitmapP, dataSizeP, headerSizeP);
      }
      break;

    case sysTrapBmpSize: {
      const BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      UInt16 ret = BmpSize(bitmapP);
      *iret = ret;
      }
      break;

    case sysTrapCategoryCreateList: {
      DmOpenRef db = sys_va_arg(ap, DmOpenRef);
      ListType *listP = sys_va_arg(ap, ListType *);
      UInt16 currentCategory = sys_va_arg(ap, UInt32);
      Boolean showAll = sys_va_arg(ap, UInt32);
      Boolean showUneditables = sys_va_arg(ap, UInt32);
      UInt8 numUneditableCategories = sys_va_arg(ap, UInt32);
      UInt32 editingStrID = sys_va_arg(ap, UInt32);
      Boolean resizeList = sys_va_arg(ap, UInt32);
      CategoryCreateList(db, listP, currentCategory, showAll, showUneditables, numUneditableCategories, editingStrID, resizeList);
      }
      break;

    case sysTrapCategoryCreateListV10: {
      DmOpenRef db = sys_va_arg(ap, DmOpenRef);
      ListType *lst = sys_va_arg(ap, ListType *);
      UInt16 currentCategory = sys_va_arg(ap, UInt32);
      Boolean showAll = sys_va_arg(ap, UInt32);
      CategoryCreateListV10(db, lst, currentCategory, showAll);
      }
      break;

    case sysTrapCategoryEdit: {
      DmOpenRef db = sys_va_arg(ap, DmOpenRef);
      UInt16 *category = sys_va_arg(ap, UInt16 *);
      UInt32 titleStrID = sys_va_arg(ap, UInt32);
      UInt8 numUneditableCategories = sys_va_arg(ap, UInt32);
      Boolean ret = CategoryEdit(db, category, titleStrID, numUneditableCategories);
      *iret = ret;
      }
      break;

    case sysTrapCategoryEditV10: {
      DmOpenRef db = sys_va_arg(ap, DmOpenRef);
      UInt16 *category = sys_va_arg(ap, UInt16 *);
      Boolean ret = CategoryEditV10(db, category);
      *iret = ret;
      }
      break;

    case sysTrapCategoryEditV20: {
      DmOpenRef db = sys_va_arg(ap, DmOpenRef);
      UInt16 *category = sys_va_arg(ap, UInt16 *);
      UInt32 titleStrID = sys_va_arg(ap, UInt32);
      Boolean ret = CategoryEditV20(db, category, titleStrID);
      *iret = ret;
      }
      break;

    case sysTrapCategoryFind: {
      DmOpenRef db = sys_va_arg(ap, DmOpenRef);
      const Char *name = sys_va_arg(ap, Char *);
      UInt16 ret = CategoryFind(db, name);
      *iret = ret;
      }
      break;

    case sysTrapCategoryFreeList: {
      DmOpenRef db = sys_va_arg(ap, DmOpenRef);
      ListType *listP = sys_va_arg(ap, ListType *);
      Boolean showAll = sys_va_arg(ap, UInt32);
      UInt32 editingStrID = sys_va_arg(ap, UInt32);
      CategoryFreeList(db, listP, showAll, editingStrID);
      }
      break;

    case sysTrapCategoryFreeListV10: {
      DmOpenRef db = sys_va_arg(ap, DmOpenRef);
      ListType *lst = sys_va_arg(ap, ListType *);
      CategoryFreeListV10(db, lst);
      }
      break;

    case sysTrapCategoryGetName: {
      DmOpenRef db = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      Char *name = sys_va_arg(ap, Char *);
      CategoryGetName(db, index, name);
      }
      break;

    case sysTrapCategoryGetNext: {
      DmOpenRef db = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      UInt16 ret = CategoryGetNext(db, index);
      *iret = ret;
      }
      break;

    case sysTrapCategoryInitialize: {
      AppInfoPtr appInfoP = sys_va_arg(ap, AppInfoPtr);
      UInt16 localizedAppInfoStrID = sys_va_arg(ap, UInt32);
      CategoryInitialize(appInfoP, localizedAppInfoStrID);
      }
      break;

    case sysTrapCategorySelect: {
      DmOpenRef db = sys_va_arg(ap, DmOpenRef);
      const FormType *frm = sys_va_arg(ap, FormType *);
      UInt16 ctlID = sys_va_arg(ap, UInt32);
      UInt16 lstID = sys_va_arg(ap, UInt32);
      Boolean title = sys_va_arg(ap, UInt32);
      UInt16 *categoryP = sys_va_arg(ap, UInt16 *);
      Char *categoryName = sys_va_arg(ap, Char *);
      UInt8 numUneditableCategories = sys_va_arg(ap, UInt32);
      UInt32 editingStrID = sys_va_arg(ap, UInt32);
      Boolean ret = CategorySelect(db, frm, ctlID, lstID, title, categoryP, categoryName, numUneditableCategories, editingStrID);
      *iret = ret;
      }
      break;

    case sysTrapCategorySelectV10: {
      DmOpenRef db = sys_va_arg(ap, DmOpenRef);
      const FormType *frm = sys_va_arg(ap, FormType *);
      UInt16 ctlID = sys_va_arg(ap, UInt32);
      UInt16 lstID = sys_va_arg(ap, UInt32);
      Boolean title = sys_va_arg(ap, UInt32);
      UInt16 *categoryP = sys_va_arg(ap, UInt16 *);
      Char *categoryName = sys_va_arg(ap, Char *);
      Boolean ret = CategorySelectV10(db, frm, ctlID, lstID, title, categoryP, categoryName);
      *iret = ret;
      }
      break;

    case sysTrapCategorySetName: {
      DmOpenRef db = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      const Char *nameP = sys_va_arg(ap, Char *);
      CategorySetName(db, index, nameP);
      }
      break;

    case sysTrapCategorySetTriggerLabel: {
      ControlType *ctl = sys_va_arg(ap, ControlType *);
      Char *name = sys_va_arg(ap, Char *);
      CategorySetTriggerLabel(ctl, name);
      }
      break;

    case sysTrapCategoryTruncateName: {
      Char *name = sys_va_arg(ap, Char *);
      UInt16 maxWidth = sys_va_arg(ap, UInt32);
      CategoryTruncateName(name, maxWidth);
      }
      break;

    case sysTrapClipboardAddItem: {
      const ClipboardFormatType format = sys_va_arg(ap, ClipboardFormatType);
      const void *ptr = sys_va_arg(ap, void *);
      UInt16 length = sys_va_arg(ap, UInt32);
      ClipboardAddItem(format, ptr, length);
      }
      break;

    case sysTrapClipboardAppendItem: {
      const ClipboardFormatType format = sys_va_arg(ap, ClipboardFormatType);
      const void *ptr = sys_va_arg(ap, void *);
      UInt16 length = sys_va_arg(ap, UInt32);
      Err ret = ClipboardAppendItem(format, ptr, length);
      *iret = ret;
      }
      break;

    case sysTrapClipboardGetItem: {
      const ClipboardFormatType format = sys_va_arg(ap, ClipboardFormatType);
      UInt16 *length = sys_va_arg(ap, UInt16 *);
      MemHandle ret = ClipboardGetItem(format, length);
      *pret = (void *)ret;
      }
      break;

    case sysTrapCncAddProfile: {
      Char *name = sys_va_arg(ap, Char *);
      UInt32 port = sys_va_arg(ap, UInt32);
      UInt32 baud = sys_va_arg(ap, UInt32);
      UInt16 volume = sys_va_arg(ap, UInt32);
      UInt16 handShake = sys_va_arg(ap, UInt32);
      const Char *initString = sys_va_arg(ap, Char *);
      const Char *resetString = sys_va_arg(ap, Char *);
      Boolean isModem = sys_va_arg(ap, UInt32);
      Boolean isPulse = sys_va_arg(ap, UInt32);
      Err ret = CncAddProfile(name, port, baud, volume, handShake, initString, resetString, isModem, isPulse);
      *iret = ret;
      }
      break;

    case sysTrapCncDeleteProfile: {
      const Char *name = sys_va_arg(ap, Char *);
      Err ret = CncDeleteProfile(name);
      *iret = ret;
      }
      break;

    case sysTrapCncGetProfileInfo: {
      Char *name = sys_va_arg(ap, Char *);
      UInt32 *port = sys_va_arg(ap, UInt32 *);
      UInt32 *baud = sys_va_arg(ap, UInt32 *);
      UInt16 *volume = sys_va_arg(ap, UInt16 *);
      UInt16 *handShake = sys_va_arg(ap, UInt16 *);
      Char *initString = sys_va_arg(ap, Char *);
      Char *resetString = sys_va_arg(ap, Char *);
      Boolean *isModem = sys_va_arg(ap, Boolean *);
      Boolean *isPulse = sys_va_arg(ap, Boolean *);
      Err ret = CncGetProfileInfo(name, port, baud, volume, handShake, initString, resetString, isModem, isPulse);
      *iret = ret;
      }
      break;

    case sysTrapCncGetProfileList: {
      Char * * *nameListPPP = sys_va_arg(ap, Char * * *);
      UInt16 *countP = sys_va_arg(ap, UInt16 *);
      Err ret = CncGetProfileList(nameListPPP, countP);
      *iret = ret;
      }
      break;

    case sysTrapCrc16CalcBlock: {
      const void *bufP = sys_va_arg(ap, void *);
      UInt16 count = sys_va_arg(ap, UInt32);
      UInt16 crc = sys_va_arg(ap, UInt32);
      UInt16 ret = Crc16CalcBlock(bufP, count, crc);
      *iret = ret;
      }
      break;

    case sysTrapCtlDrawControl: {
      ControlType *controlP = sys_va_arg(ap, ControlType *);
      CtlDrawControl(controlP);
      }
      break;

    case sysTrapCtlEnabled: {
      const ControlType *controlP = sys_va_arg(ap, ControlType *);
      Boolean ret = CtlEnabled(controlP);
      *iret = ret;
      }
      break;

    case sysTrapCtlEraseControl: {
      ControlType *controlP = sys_va_arg(ap, ControlType *);
      CtlEraseControl(controlP);
      }
      break;

    case sysTrapCtlGetLabel: {
      const ControlType *controlP = sys_va_arg(ap, ControlType *);
      const Char *ret = CtlGetLabel(controlP);
      *pret = (void *)ret;
      }
      break;

    case sysTrapCtlGetSliderValues: {
      const ControlType *ctlP = sys_va_arg(ap, ControlType *);
      UInt16 *minValueP = sys_va_arg(ap, UInt16 *);
      UInt16 *maxValueP = sys_va_arg(ap, UInt16 *);
      UInt16 *pageSizeP = sys_va_arg(ap, UInt16 *);
      UInt16 *valueP = sys_va_arg(ap, UInt16 *);
      CtlGetSliderValues(ctlP, minValueP, maxValueP, pageSizeP, valueP);
      }
      break;

    case sysTrapCtlGetValue: {
      const ControlType *controlP = sys_va_arg(ap, ControlType *);
      Int16 ret = CtlGetValue(controlP);
      *iret = ret;
      }
      break;

    case sysTrapCtlHandleEvent: {
      ControlType *controlP = sys_va_arg(ap, ControlType *);
      EventType *pEvent = sys_va_arg(ap, EventType *);
      Boolean ret = CtlHandleEvent(controlP, pEvent);
      *iret = ret;
      }
      break;

    case sysTrapCtlHideControl: {
      ControlType *controlP = sys_va_arg(ap, ControlType *);
      CtlHideControl(controlP);
      }
      break;

    case sysTrapCtlHitControl: {
      const ControlType *controlP = sys_va_arg(ap, ControlType *);
      CtlHitControl(controlP);
      }
      break;

    case sysTrapCtlNewControl: {
      void * *formPP = sys_va_arg(ap, void * *);
      UInt16 ID = sys_va_arg(ap, UInt32);
      ControlStyleType style = sys_va_arg(ap, ControlStyleType);
      const Char *textP = sys_va_arg(ap, Char *);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      Coord width = sys_va_arg(ap, Int32);
      Coord height = sys_va_arg(ap, Int32);
      FontID font = sys_va_arg(ap, FontID);
      UInt8 group = sys_va_arg(ap, UInt32);
      Boolean leftAnchor = sys_va_arg(ap, UInt32);
      ControlType *ret = CtlNewControl(formPP, ID, style, textP, x, y, width, height, font, group, leftAnchor);
      *pret = (void *)ret;
      }
      break;

    case sysTrapCtlNewGraphicControl: {
      void * *formPP = sys_va_arg(ap, void * *);
      UInt16 ID = sys_va_arg(ap, UInt32);
      ControlStyleType style = sys_va_arg(ap, ControlStyleType);
      DmResID bitmapID = sys_va_arg(ap, UInt32);
      DmResID selectedBitmapID = sys_va_arg(ap, UInt32);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      Coord width = sys_va_arg(ap, Int32);
      Coord height = sys_va_arg(ap, Int32);
      UInt8 group = sys_va_arg(ap, UInt32);
      Boolean leftAnchor = sys_va_arg(ap, UInt32);
      GraphicControlType *ret = CtlNewGraphicControl(formPP, ID, style, bitmapID, selectedBitmapID, x, y, width, height, group, leftAnchor);
      *pret = (void *)ret;
      }
      break;

    case sysTrapCtlNewSliderControl: {
      void * *formPP = sys_va_arg(ap, void * *);
      UInt16 ID = sys_va_arg(ap, UInt32);
      ControlStyleType style = sys_va_arg(ap, ControlStyleType);
      DmResID thumbID = sys_va_arg(ap, UInt32);
      DmResID backgroundID = sys_va_arg(ap, UInt32);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      Coord width = sys_va_arg(ap, Int32);
      Coord height = sys_va_arg(ap, Int32);
      UInt16 minValue = sys_va_arg(ap, UInt32);
      UInt16 maxValue = sys_va_arg(ap, UInt32);
      UInt16 pageSize = sys_va_arg(ap, UInt32);
      UInt16 value = sys_va_arg(ap, UInt32);
      SliderControlType *ret = CtlNewSliderControl(formPP, ID, style, thumbID, backgroundID, x, y, width, height, minValue, maxValue, pageSize, value);
      *pret = (void *)ret;
      }
      break;

    case sysTrapCtlSetEnabled: {
      ControlType *controlP = sys_va_arg(ap, ControlType *);
      Boolean usable = sys_va_arg(ap, UInt32);
      CtlSetEnabled(controlP, usable);
      }
      break;

    case sysTrapCtlSetGraphics: {
      ControlType *ctlP = sys_va_arg(ap, ControlType *);
      DmResID newBitmapID = sys_va_arg(ap, UInt32);
      DmResID newSelectedBitmapID = sys_va_arg(ap, UInt32);
      CtlSetGraphics(ctlP, newBitmapID, newSelectedBitmapID);
      }
      break;

    case sysTrapCtlSetLabel: {
      ControlType *controlP = sys_va_arg(ap, ControlType *);
      const Char *newLabel = sys_va_arg(ap, Char *);
      CtlSetLabel(controlP, newLabel);
      }
      break;

    case sysTrapCtlSetSliderValues: {
      ControlType *ctlP = sys_va_arg(ap, ControlType *);
      const UInt16 *minValueP = sys_va_arg(ap, UInt16 *);
      const UInt16 *maxValueP = sys_va_arg(ap, UInt16 *);
      const UInt16 *pageSizeP = sys_va_arg(ap, UInt16 *);
      const UInt16 *valueP = sys_va_arg(ap, UInt16 *);
      CtlSetSliderValues(ctlP, minValueP, maxValueP, pageSizeP, valueP);
      }
      break;

    case sysTrapCtlSetUsable: {
      ControlType *controlP = sys_va_arg(ap, ControlType *);
      Boolean usable = sys_va_arg(ap, UInt32);
      CtlSetUsable(controlP, usable);
      }
      break;

    case sysTrapCtlSetValue: {
      ControlType *controlP = sys_va_arg(ap, ControlType *);
      Int16 newValue = sys_va_arg(ap, Int32);
      CtlSetValue(controlP, newValue);
      }
      break;

    case sysTrapCtlShowControl: {
      ControlType *controlP = sys_va_arg(ap, ControlType *);
      CtlShowControl(controlP);
      }
      break;

    case sysTrapCtlValidatePointer: {
      const ControlType *controlP = sys_va_arg(ap, ControlType *);
      Boolean ret = CtlValidatePointer(controlP);
      *iret = ret;
      }
      break;

    case sysTrapDateAdjust: {
      DateType *dateP = sys_va_arg(ap, DateType *);
      Int32 adjustment = sys_va_arg(ap, Int32);
      DateAdjust(dateP, adjustment);
      }
      break;

    case sysTrapDateDaysToDate: {
      UInt32 days = sys_va_arg(ap, UInt32);
      DateType *dateP = sys_va_arg(ap, DateType *);
      DateDaysToDate(days, dateP);
      }
      break;

    case sysTrapDateSecondsToDate: {
      UInt32 seconds = sys_va_arg(ap, UInt32);
      DateType *dateP = sys_va_arg(ap, DateType *);
      DateSecondsToDate(seconds, dateP);
      }
      break;

    case sysTrapDateTemplateToAscii: {
      const Char *templateP = sys_va_arg(ap, Char *);
      UInt8 months = sys_va_arg(ap, UInt32);
      UInt8 days = sys_va_arg(ap, UInt32);
      UInt16 years = sys_va_arg(ap, UInt32);
      Char *stringP = sys_va_arg(ap, Char *);
      Int16 stringLen = sys_va_arg(ap, Int32);
      UInt16 ret = DateTemplateToAscii(templateP, months, days, years, stringP, stringLen);
      *iret = ret;
      }
      break;

    case sysTrapDateToAscii: {
      UInt8 months = sys_va_arg(ap, UInt32);
      UInt8 days = sys_va_arg(ap, UInt32);
      UInt16 years = sys_va_arg(ap, UInt32);
      DateFormatType dateFormat = sys_va_arg(ap, DateFormatType);
      Char *pString = sys_va_arg(ap, Char *);
      DateToAscii(months, days, years, dateFormat, pString);
      }
      break;

    case sysTrapDateToDOWDMFormat: {
      UInt8 months = sys_va_arg(ap, UInt32);
      UInt8 days = sys_va_arg(ap, UInt32);
      UInt16 years = sys_va_arg(ap, UInt32);
      DateFormatType dateFormat = sys_va_arg(ap, DateFormatType);
      Char *pString = sys_va_arg(ap, Char *);
      DateToDOWDMFormat(months, days, years, dateFormat, pString);
      }
      break;

    case sysTrapDateToDays: {
      DateType date = sys_va_arg(ap, DateType);
      UInt32 ret = DateToDays(date);
      *iret = ret;
      }
      break;

    case sysTrapDayDrawDaySelector: {
      const DaySelectorType *selectorP = sys_va_arg(ap, DaySelectorType *);
      DayDrawDaySelector(selectorP);
      }
      break;

    case sysTrapDayDrawDays: {
      const DaySelectorType *selectorP = sys_va_arg(ap, DaySelectorType *);
      DayDrawDays(selectorP);
      }
      break;

    case sysTrapDayHandleEvent: {
      DaySelectorType *selectorP = sys_va_arg(ap, DaySelectorType *);
      const EventType *pEvent = sys_va_arg(ap, EventType *);
      Boolean ret = DayHandleEvent(selectorP, pEvent);
      *iret = ret;
      }
      break;

    case sysTrapDayOfMonth: {
      Int16 month = sys_va_arg(ap, Int32);
      Int16 day = sys_va_arg(ap, Int32);
      Int16 year = sys_va_arg(ap, Int32);
      Int16 ret = DayOfMonth(month, day, year);
      *iret = ret;
      }
      break;

    case sysTrapDayOfWeek: {
      Int16 month = sys_va_arg(ap, Int32);
      Int16 day = sys_va_arg(ap, Int32);
      Int16 year = sys_va_arg(ap, Int32);
      Int16 ret = DayOfWeek(month, day, year);
      *iret = ret;
      }
      break;

    case sysTrapDaysInMonth: {
      Int16 month = sys_va_arg(ap, Int32);
      Int16 year = sys_va_arg(ap, Int32);
      Int16 ret = DaysInMonth(month, year);
      *iret = ret;
      }
      break;

    case sysTrapDlkControl: {
      DlkCtlEnum op = sys_va_arg(ap, DlkCtlEnum);
      void *param1P = sys_va_arg(ap, void *);
      void *param2P = sys_va_arg(ap, void *);
      Err ret = DlkControl(op, param1P, param2P);
      *iret = ret;
      }
      break;

    case sysTrapDlkDispatchRequest: {
      DlkServerSessionPtr sessP = sys_va_arg(ap, DlkServerSessionPtr);
      Err ret = DlkDispatchRequest(sessP);
      *iret = ret;
      }
      break;

    case sysTrapDlkGetSyncInfo: {
      UInt32 *succSyncDateP = sys_va_arg(ap, UInt32 *);
      UInt32 *lastSyncDateP = sys_va_arg(ap, UInt32 *);
      DlkSyncStateType *syncStateP = sys_va_arg(ap, DlkSyncStateType *);
      Char *nameBufP = sys_va_arg(ap, Char *);
      Char *logBufP = sys_va_arg(ap, Char *);
      Int32 *logLenP = sys_va_arg(ap, Int32 *);
      Err ret = DlkGetSyncInfo(succSyncDateP, lastSyncDateP, syncStateP, nameBufP, logBufP, logLenP);
      *iret = ret;
      }
      break;

    case sysTrapDlkSetLogEntry: {
      const Char *textP = sys_va_arg(ap, Char *);
      Int16 textLen = sys_va_arg(ap, Int32);
      Boolean append = sys_va_arg(ap, UInt32);
      DlkSetLogEntry(textP, textLen, append);
      }
      break;

    case sysTrapDlkStartServer: {
      DlkServerParamPtr paramP = sys_va_arg(ap, DlkServerParamPtr);
      Err ret = DlkStartServer(paramP);
      *iret = ret;
      }
      break;

    case sysTrapDmArchiveRecord: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      Err ret = DmArchiveRecord(dbP, index);
      *iret = ret;
      }
      break;

    case sysTrapDmAttachRecord: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 *atP = sys_va_arg(ap, UInt16 *);
      MemHandle newH = sys_va_arg(ap, MemHandle);
      MemHandle *oldHP = sys_va_arg(ap, MemHandle *);
      Err ret = DmAttachRecord(dbP, atP, newH, oldHP);
      *iret = ret;
      }
      break;

    case sysTrapDmAttachResource: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      MemHandle newH = sys_va_arg(ap, MemHandle);
      DmResType resType = sys_va_arg(ap, DmResType);
      DmResID resID = sys_va_arg(ap, UInt32);
      Err ret = DmAttachResource(dbP, newH, resType, resID);
      *iret = ret;
      }
      break;

    case sysTrapDmCloseDatabase: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      Err ret = DmCloseDatabase(dbP);
      *iret = ret;
      }
      break;

    case sysTrapDmCreateDatabase: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      const Char *nameP = sys_va_arg(ap, Char *);
      UInt32 creator = sys_va_arg(ap, UInt32);
      UInt32 type = sys_va_arg(ap, UInt32);
      Boolean resDB = sys_va_arg(ap, UInt32);
      Err ret = DmCreateDatabase(cardNo, nameP, creator, type, resDB);
      *iret = ret;
      }
      break;

    case sysTrapDmCreateDatabaseFromImage: {
      MemPtr bufferP = sys_va_arg(ap, MemPtr);
      Err ret = DmCreateDatabaseFromImage(bufferP);
      *iret = ret;
      }
      break;

    case sysTrapDmDatabaseInfo: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      Char *nameP = sys_va_arg(ap, Char *);
      UInt16 *attributesP = sys_va_arg(ap, UInt16 *);
      UInt16 *versionP = sys_va_arg(ap, UInt16 *);
      UInt32 *crDateP = sys_va_arg(ap, UInt32 *);
      UInt32 *modDateP = sys_va_arg(ap, UInt32 *);
      UInt32 *bckUpDateP = sys_va_arg(ap, UInt32 *);
      UInt32 *modNumP = sys_va_arg(ap, UInt32 *);
      LocalID *appInfoIDP = sys_va_arg(ap, LocalID *);
      LocalID *sortInfoIDP = sys_va_arg(ap, LocalID *);
      UInt32 *typeP = sys_va_arg(ap, UInt32 *);
      UInt32 *creatorP = sys_va_arg(ap, UInt32 *);
      Err ret = DmDatabaseInfo(cardNo, dbID, nameP, attributesP, versionP, crDateP, modDateP, bckUpDateP, modNumP, appInfoIDP, sortInfoIDP, typeP, creatorP);
      *iret = ret;
      }
      break;

    case sysTrapDmDatabaseProtect: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      Boolean protect = sys_va_arg(ap, UInt32);
      Err ret = DmDatabaseProtect(cardNo, dbID, protect);
      *iret = ret;
      }
      break;

    case sysTrapDmDatabaseSize: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      UInt32 *numRecordsP = sys_va_arg(ap, UInt32 *);
      UInt32 *totalBytesP = sys_va_arg(ap, UInt32 *);
      UInt32 *dataBytesP = sys_va_arg(ap, UInt32 *);
      Err ret = DmDatabaseSize(cardNo, dbID, numRecordsP, totalBytesP, dataBytesP);
      *iret = ret;
      }
      break;

    case sysTrapDmDeleteCategory: {
      DmOpenRef dbR = sys_va_arg(ap, DmOpenRef);
      UInt16 categoryNum = sys_va_arg(ap, UInt32);
      Err ret = DmDeleteCategory(dbR, categoryNum);
      *iret = ret;
      }
      break;

    case sysTrapDmDeleteDatabase: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      Err ret = DmDeleteDatabase(cardNo, dbID);
      *iret = ret;
      }
      break;

    case sysTrapDmDeleteRecord: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      Err ret = DmDeleteRecord(dbP, index);
      *iret = ret;
      }
      break;

    case sysTrapDmDetachRecord: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      MemHandle *oldHP = sys_va_arg(ap, MemHandle *);
      Err ret = DmDetachRecord(dbP, index, oldHP);
      *iret = ret;
      }
      break;

    case sysTrapDmDetachResource: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      MemHandle *oldHP = sys_va_arg(ap, MemHandle *);
      Err ret = DmDetachResource(dbP, index, oldHP);
      *iret = ret;
      }
      break;

    case sysTrapDmFindDatabase: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      const Char *nameP = sys_va_arg(ap, Char *);
      LocalID ret = DmFindDatabase(cardNo, nameP);
      *iret = ret;
      }
      break;

    case sysTrapDmFindRecordByID: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt32 uniqueID = sys_va_arg(ap, UInt32);
      UInt16 *indexP = sys_va_arg(ap, UInt16 *);
      Err ret = DmFindRecordByID(dbP, uniqueID, indexP);
      *iret = ret;
      }
      break;

    case sysTrapDmFindResource: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      DmResType resType = sys_va_arg(ap, DmResType);
      DmResID resID = sys_va_arg(ap, UInt32);
      MemHandle resH = sys_va_arg(ap, MemHandle);
      UInt16 ret = DmFindResource(dbP, resType, resID, resH);
      *iret = ret;
      }
      break;

    case sysTrapDmFindResourceType: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      DmResType resType = sys_va_arg(ap, DmResType);
      UInt16 typeIndex = sys_va_arg(ap, UInt32);
      UInt16 ret = DmFindResourceType(dbP, resType, typeIndex);
      *iret = ret;
      }
      break;

    case sysTrapDmFindSortPosition: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      void *newRecord = sys_va_arg(ap, void *);
      SortRecordInfoPtr newRecordInfo = sys_va_arg(ap, SortRecordInfoPtr);
      DmComparF *compar = sys_va_arg(ap, DmComparF *);
      Int16 other = sys_va_arg(ap, Int32);
      UInt16 ret = DmFindSortPosition(dbP, newRecord, newRecordInfo, compar, other);
      *iret = ret;
      }
      break;

    case sysTrapDmFindSortPositionV10: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      void *newRecord = sys_va_arg(ap, void *);
      DmComparF *compar = sys_va_arg(ap, DmComparF *);
      Int16 other = sys_va_arg(ap, Int32);
      UInt16 ret = DmFindSortPositionV10(dbP, newRecord, compar, other);
      *iret = ret;
      }
      break;

    case sysTrapDmGet1Resource: {
      DmResType type = sys_va_arg(ap, DmResType);
      DmResID resID = sys_va_arg(ap, UInt32);
      MemHandle ret = DmGet1Resource(type, resID);
      *pret = (void *)ret;
      }
      break;

    case sysTrapDmGetAppInfoID: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      LocalID ret = DmGetAppInfoID(dbP);
      *iret = ret;
      }
      break;

    case sysTrapDmGetDatabase: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      UInt16 index = sys_va_arg(ap, UInt32);
      LocalID ret = DmGetDatabase(cardNo, index);
      *iret = ret;
      }
      break;

    case sysTrapDmGetDatabaseLockState: {
      DmOpenRef dbR = sys_va_arg(ap, DmOpenRef);
      UInt8 *highest = sys_va_arg(ap, UInt8 *);
      UInt32 *count = sys_va_arg(ap, UInt32 *);
      UInt32 *busy = sys_va_arg(ap, UInt32 *);
      DmGetDatabaseLockState(dbR, highest, count, busy);
      }
      break;

    case sysTrapDmGetLastErr: {
      Err ret = DmGetLastErr();
      *iret = ret;
      }
      break;

    case sysTrapDmGetNextDatabaseByTypeCreator: {
      Boolean newSearch = sys_va_arg(ap, UInt32);
      DmSearchStatePtr stateInfoP = sys_va_arg(ap, DmSearchStatePtr);
      UInt32 type = sys_va_arg(ap, UInt32);
      UInt32 creator = sys_va_arg(ap, UInt32);
      Boolean onlyLatestVers = sys_va_arg(ap, UInt32);
      UInt16 *cardNoP = sys_va_arg(ap, UInt16 *);
      LocalID *dbIDP = sys_va_arg(ap, LocalID *);
      Err ret = DmGetNextDatabaseByTypeCreator(newSearch, stateInfoP, type, creator, onlyLatestVers, cardNoP, dbIDP);
      *iret = ret;
      }
      break;

    case sysTrapDmGetRecord: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      MemHandle ret = DmGetRecord(dbP, index);
      *pret = (void *)ret;
      }
      break;

    case sysTrapDmGetResource: {
      DmResType type = sys_va_arg(ap, DmResType);
      DmResID resID = sys_va_arg(ap, UInt32);
      MemHandle ret = DmGetResource(type, resID);
      *pret = (void *)ret;
      }
      break;

    case sysTrapDmGetResourceIndex: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      MemHandle ret = DmGetResourceIndex(dbP, index);
      *pret = (void *)ret;
      }
      break;

    case sysTrapDmInit: {
      Err ret = DmInit();
      *iret = ret;
      }
      break;

    case sysTrapDmInsertionSort: {
      DmOpenRef dbR = sys_va_arg(ap, DmOpenRef);
      DmComparF *compar = sys_va_arg(ap, DmComparF *);
      Int16 other = sys_va_arg(ap, Int32);
      Err ret = DmInsertionSort(dbR, compar, other);
      *iret = ret;
      }
      break;

    case sysTrapDmMoveCategory: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 toCategory = sys_va_arg(ap, UInt32);
      UInt16 fromCategory = sys_va_arg(ap, UInt32);
      Boolean dirty = sys_va_arg(ap, UInt32);
      Err ret = DmMoveCategory(dbP, toCategory, fromCategory, dirty);
      *iret = ret;
      }
      break;

    case sysTrapDmMoveRecord: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 from = sys_va_arg(ap, UInt32);
      UInt16 to = sys_va_arg(ap, UInt32);
      Err ret = DmMoveRecord(dbP, from, to);
      *iret = ret;
      }
      break;

    case sysTrapDmNewHandle: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt32 size = sys_va_arg(ap, UInt32);
      MemHandle ret = DmNewHandle(dbP, size);
      *pret = (void *)ret;
      }
      break;

    case sysTrapDmNewRecord: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 *atP = sys_va_arg(ap, UInt16 *);
      UInt32 size = sys_va_arg(ap, UInt32);
      MemHandle ret = DmNewRecord(dbP, atP, size);
      *pret = (void *)ret;
      }
      break;

    case sysTrapDmNewResource: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      DmResType resType = sys_va_arg(ap, DmResType);
      DmResID resID = sys_va_arg(ap, UInt32);
      UInt32 size = sys_va_arg(ap, UInt32);
      MemHandle ret = DmNewResource(dbP, resType, resID, size);
      *pret = (void *)ret;
      }
      break;

    case sysTrapDmNextOpenDatabase: {
      DmOpenRef currentP = sys_va_arg(ap, DmOpenRef);
      DmOpenRef ret = DmNextOpenDatabase(currentP);
      *pret = (void *)ret;
      }
      break;

    case sysTrapDmNextOpenResDatabase: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      DmOpenRef ret = DmNextOpenResDatabase(dbP);
      *pret = (void *)ret;
      }
      break;

    case sysTrapDmNumDatabases: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      UInt16 ret = DmNumDatabases(cardNo);
      *iret = ret;
      }
      break;

    case sysTrapDmNumRecords: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 ret = DmNumRecords(dbP);
      *iret = ret;
      }
      break;

    case sysTrapDmNumRecordsInCategory: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 category = sys_va_arg(ap, UInt32);
      UInt16 ret = DmNumRecordsInCategory(dbP, category);
      *iret = ret;
      }
      break;

    case sysTrapDmNumResources: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 ret = DmNumResources(dbP);
      *iret = ret;
      }
      break;

    case sysTrapDmOpenDBNoOverlay: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      UInt16 mode = sys_va_arg(ap, UInt32);
      DmOpenRef ret = DmOpenDBNoOverlay(cardNo, dbID, mode);
      *pret = (void *)ret;
      }
      break;

    case sysTrapDmOpenDatabase: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      UInt16 mode = sys_va_arg(ap, UInt32);
      DmOpenRef ret = DmOpenDatabase(cardNo, dbID, mode);
      *pret = (void *)ret;
      }
      break;

    case sysTrapDmOpenDatabaseByTypeCreator: {
      UInt32 type = sys_va_arg(ap, UInt32);
      UInt32 creator = sys_va_arg(ap, UInt32);
      UInt16 mode = sys_va_arg(ap, UInt32);
      DmOpenRef ret = DmOpenDatabaseByTypeCreator(type, creator, mode);
      *pret = (void *)ret;
      }
      break;

    case sysTrapDmOpenDatabaseInfo: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      LocalID *dbIDP = sys_va_arg(ap, LocalID *);
      UInt16 *openCountP = sys_va_arg(ap, UInt16 *);
      UInt16 *modeP = sys_va_arg(ap, UInt16 *);
      UInt16 *cardNoP = sys_va_arg(ap, UInt16 *);
      Boolean *resDBP = sys_va_arg(ap, Boolean *);
      Err ret = DmOpenDatabaseInfo(dbP, dbIDP, openCountP, modeP, cardNoP, resDBP);
      *iret = ret;
      }
      break;

    case sysTrapDmPositionInCategory: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      UInt16 category = sys_va_arg(ap, UInt32);
      UInt16 ret = DmPositionInCategory(dbP, index, category);
      *iret = ret;
      }
      break;

    case sysTrapDmQueryNextInCategory: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 *indexP = sys_va_arg(ap, UInt16 *);
      UInt16 category = sys_va_arg(ap, UInt32);
      MemHandle ret = DmQueryNextInCategory(dbP, indexP, category);
      *pret = (void *)ret;
      }
      break;

    case sysTrapDmQueryRecord: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      MemHandle ret = DmQueryRecord(dbP, index);
      *pret = (void *)ret;
      }
      break;

    case sysTrapDmQuickSort: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      DmComparF *compar = sys_va_arg(ap, DmComparF *);
      Int16 other = sys_va_arg(ap, Int32);
      Err ret = DmQuickSort(dbP, compar, other);
      *iret = ret;
      }
      break;

    case sysTrapDmRecordInfo: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      UInt16 *attrP = sys_va_arg(ap, UInt16 *);
      UInt32 *uniqueIDP = sys_va_arg(ap, UInt32 *);
      LocalID *chunkIDP = sys_va_arg(ap, LocalID *);
      Err ret = DmRecordInfo(dbP, index, attrP, uniqueIDP, chunkIDP);
      *iret = ret;
      }
      break;

    case sysTrapDmReleaseRecord: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      Boolean dirty = sys_va_arg(ap, UInt32);
      Err ret = DmReleaseRecord(dbP, index, dirty);
      *iret = ret;
      }
      break;

    case sysTrapDmReleaseResource: {
      MemHandle resourceH = sys_va_arg(ap, MemHandle);
      Err ret = DmReleaseResource(resourceH);
      *iret = ret;
      }
      break;

    case sysTrapDmRemoveRecord: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      Err ret = DmRemoveRecord(dbP, index);
      *iret = ret;
      }
      break;

    case sysTrapDmRemoveResource: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      Err ret = DmRemoveResource(dbP, index);
      *iret = ret;
      }
      break;

    case sysTrapDmRemoveSecretRecords: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      Err ret = DmRemoveSecretRecords(dbP);
      *iret = ret;
      }
      break;

    case sysTrapDmResetRecordStates: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      Err ret = DmResetRecordStates(dbP);
      *iret = ret;
      }
      break;

    case sysTrapDmResizeRecord: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      UInt32 newSize = sys_va_arg(ap, UInt32);
      MemHandle ret = DmResizeRecord(dbP, index, newSize);
      *pret = (void *)ret;
      }
      break;

    case sysTrapDmResizeResource: {
      MemHandle resourceH = sys_va_arg(ap, MemHandle);
      UInt32 newSize = sys_va_arg(ap, UInt32);
      MemHandle ret = DmResizeResource(resourceH, newSize);
      *pret = (void *)ret;
      }
      break;

    case sysTrapDmResourceInfo: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      DmResType *resTypeP = sys_va_arg(ap, DmResType *);
      DmResID *resIDP = sys_va_arg(ap, DmResID *);
      LocalID *chunkLocalIDP = sys_va_arg(ap, LocalID *);
      Err ret = DmResourceInfo(dbP, index, resTypeP, resIDP, chunkLocalIDP);
      *iret = ret;
      }
      break;

    case sysTrapDmSearchRecord: {
      MemHandle recH = sys_va_arg(ap, MemHandle);
      DmOpenRef *dbPP = sys_va_arg(ap, DmOpenRef *);
      UInt16 ret = DmSearchRecord(recH, dbPP);
      *iret = ret;
      }
      break;

    case sysTrapDmSearchResource: {
      DmResType resType = sys_va_arg(ap, DmResType);
      DmResID resID = sys_va_arg(ap, UInt32);
      MemHandle resH = sys_va_arg(ap, MemHandle);
      DmOpenRef *dbPP = sys_va_arg(ap, DmOpenRef *);
      UInt16 ret = DmSearchResource(resType, resID, resH, dbPP);
      *iret = ret;
      }
      break;

    case sysTrapDmSeekRecordInCategory: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 *indexP = sys_va_arg(ap, UInt16 *);
      UInt16 offset = sys_va_arg(ap, UInt32);
      Int16 direction = sys_va_arg(ap, Int32);
      UInt16 category = sys_va_arg(ap, UInt32);
      Err ret = DmSeekRecordInCategory(dbP, indexP, offset, direction, category);
      *iret = ret;
      }
      break;

    case sysTrapDmSet: {
      void *recordP = sys_va_arg(ap, void *);
      UInt32 offset = sys_va_arg(ap, UInt32);
      UInt32 bytes = sys_va_arg(ap, UInt32);
      UInt8 value = sys_va_arg(ap, UInt32);
      Err ret = DmSet(recordP, offset, bytes, value);
      *iret = ret;
      }
      break;

    case sysTrapDmSetDatabaseInfo: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      const Char *nameP = sys_va_arg(ap, Char *);
      UInt16 *attributesP = sys_va_arg(ap, UInt16 *);
      UInt16 *versionP = sys_va_arg(ap, UInt16 *);
      UInt32 *crDateP = sys_va_arg(ap, UInt32 *);
      UInt32 *modDateP = sys_va_arg(ap, UInt32 *);
      UInt32 *bckUpDateP = sys_va_arg(ap, UInt32 *);
      UInt32 *modNumP = sys_va_arg(ap, UInt32 *);
      LocalID *appInfoIDP = sys_va_arg(ap, LocalID *);
      LocalID *sortInfoIDP = sys_va_arg(ap, LocalID *);
      UInt32 *typeP = sys_va_arg(ap, UInt32 *);
      UInt32 *creatorP = sys_va_arg(ap, UInt32 *);
      Err ret = DmSetDatabaseInfo(cardNo, dbID, nameP, attributesP, versionP, crDateP, modDateP, bckUpDateP, modNumP, appInfoIDP, sortInfoIDP, typeP, creatorP);
      *iret = ret;
      }
      break;

    case sysTrapDmSetRecordInfo: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      UInt16 *attrP = sys_va_arg(ap, UInt16 *);
      UInt32 *uniqueIDP = sys_va_arg(ap, UInt32 *);
      Err ret = DmSetRecordInfo(dbP, index, attrP, uniqueIDP);
      *iret = ret;
      }
      break;

    case sysTrapDmSetResourceInfo: {
      DmOpenRef dbP = sys_va_arg(ap, DmOpenRef);
      UInt16 index = sys_va_arg(ap, UInt32);
      DmResType *resTypeP = sys_va_arg(ap, DmResType *);
      DmResID *resIDP = sys_va_arg(ap, DmResID *);
      Err ret = DmSetResourceInfo(dbP, index, resTypeP, resIDP);
      *iret = ret;
      }
      break;

    case sysTrapDmStrCopy: {
      void *recordP = sys_va_arg(ap, void *);
      UInt32 offset = sys_va_arg(ap, UInt32);
      const Char *srcP = sys_va_arg(ap, Char *);
      Err ret = DmStrCopy(recordP, offset, srcP);
      *iret = ret;
      }
      break;

    case sysTrapDmWrite: {
      void *recordP = sys_va_arg(ap, void *);
      UInt32 offset = sys_va_arg(ap, UInt32);
      const void *srcP = sys_va_arg(ap, void *);
      UInt32 bytes = sys_va_arg(ap, UInt32);
      Err ret = DmWrite(recordP, offset, srcP, bytes);
      *iret = ret;
      }
      break;

    case sysTrapDmWriteCheck: {
      void *recordP = sys_va_arg(ap, void *);
      UInt32 offset = sys_va_arg(ap, UInt32);
      UInt32 bytes = sys_va_arg(ap, UInt32);
      Err ret = DmWriteCheck(recordP, offset, bytes);
      *iret = ret;
      }
      break;

    case sysTrapECFixedDiv: {
      Int32 lhs = sys_va_arg(ap, Int32);
      Int32 rhs = sys_va_arg(ap, Int32);
      FixedType ret = ECFixedDiv(lhs, rhs);
      *iret = ret;
      }
      break;

    case sysTrapECFixedMul: {
      Int32 lhs = sys_va_arg(ap, Int32);
      Int32 rhs = sys_va_arg(ap, Int32);
      FixedType ret = ECFixedMul(lhs, rhs);
      *iret = ret;
      }
      break;

    case sysTrapEncDES: {
      UInt8 *srcP = sys_va_arg(ap, UInt8 *);
      UInt8 *keyP = sys_va_arg(ap, UInt8 *);
      UInt8 *dstP = sys_va_arg(ap, UInt8 *);
      Boolean encrypt = sys_va_arg(ap, UInt32);
      Err ret = EncDES(srcP, keyP, dstP, encrypt);
      *iret = ret;
      }
      break;

    case sysTrapEncDigestMD4: {
      UInt8 *strP = sys_va_arg(ap, UInt8 *);
      UInt16 strLen = sys_va_arg(ap, UInt32);
      UInt8 *digestP = sys_va_arg(ap, UInt8 *);
      Err ret = EncDigestMD4(strP, strLen, digestP);
      *iret = ret;
      }
      break;

    case sysTrapEncDigestMD5: {
      UInt8 *strP = sys_va_arg(ap, UInt8 *);
      UInt16 strLen = sys_va_arg(ap, UInt32);
      UInt8 *digestP = sys_va_arg(ap, UInt8 *);
      Err ret = EncDigestMD5(strP, strLen, digestP);
      *iret = ret;
      }
      break;

    case sysTrapErrAlertCustom: {
      Err errCode = sys_va_arg(ap, UInt32);
      Char *errMsgP = sys_va_arg(ap, Char *);
      Char *preMsgP = sys_va_arg(ap, Char *);
      Char *postMsgP = sys_va_arg(ap, Char *);
      UInt16 ret = ErrAlertCustom(errCode, errMsgP, preMsgP, postMsgP);
      *iret = ret;
      }
      break;

    case sysTrapErrDisplayFileLineMsg: {
      const Char * const filename = sys_va_arg(ap, Char * const);
      UInt16 lineNo = sys_va_arg(ap, UInt32);
      const Char * const msg = sys_va_arg(ap, Char * const);
      ErrDisplayFileLineMsg(filename, lineNo, msg);
      }
      break;

    case sysTrapErrExceptionList: {
      MemPtr *ret = ErrExceptionList();
      *pret = (void *)ret;
      }
      break;

    case sysTrapErrLongJump: {
      ErrJumpBufP buf = sys_va_arg(ap, ErrJumpBufP);
      Int16 result = sys_va_arg(ap, Int32);
      ErrLongJump((void *)buf, result);
      }
      break;

    case sysTrapErrSetJump: {
      ErrJumpBufP buf = sys_va_arg(ap, ErrJumpBufP);
      Int16 ret = ErrSetJump((void *)buf);
      *iret = ret;
      }
      break;

    case sysTrapErrThrow: {
      Int32 err = sys_va_arg(ap, Int32);
      ErrThrow(err);
      }
      break;

    case sysTrapEvtAddEventToQueue: {
      const EventType *event = sys_va_arg(ap, EventType *);
      EvtAddEventToQueue(event);
      }
      break;

    case sysTrapEvtAddUniqueEventToQueue: {
      const EventType *eventP = sys_va_arg(ap, EventType *);
      UInt32 id = sys_va_arg(ap, UInt32);
      Boolean inPlace = sys_va_arg(ap, UInt32);
      EvtAddUniqueEventToQueue(eventP, id, inPlace);
      }
      break;

    case sysTrapEvtCopyEvent: {
      const EventType *source = sys_va_arg(ap, EventType *);
      EventType *dest = sys_va_arg(ap, EventType *);
      EvtCopyEvent(source, dest);
      }
      break;

    case sysTrapEvtDequeueKeyEvent: {
      SysEventType *eventP = sys_va_arg(ap, SysEventType *);
      UInt16 peek = sys_va_arg(ap, UInt32);
      Err ret = EvtDequeueKeyEvent(eventP, peek);
      *iret = ret;
      }
      break;

    case sysTrapEvtDequeuePenPoint: {
      PointType *retP = sys_va_arg(ap, PointType *);
      Err ret = EvtDequeuePenPoint(retP);
      *iret = ret;
      }
      break;

    case sysTrapEvtDequeuePenStrokeInfo: {
      PointType *startPtP = sys_va_arg(ap, PointType *);
      PointType *endPtP = sys_va_arg(ap, PointType *);
      Err ret = EvtDequeuePenStrokeInfo(startPtP, endPtP);
      *iret = ret;
      }
      break;

    case sysTrapEvtEnableGraffiti: {
      Boolean enable = sys_va_arg(ap, UInt32);
      EvtEnableGraffiti(enable);
      }
      break;

    case sysTrapEvtEnqueueKey: {
      WChar ascii = sys_va_arg(ap, UInt32);
      UInt16 keycode = sys_va_arg(ap, UInt32);
      UInt16 modifiers = sys_va_arg(ap, UInt32);
      Err ret = EvtEnqueueKey(ascii, keycode, modifiers);
      *iret = ret;
      }
      break;

    case sysTrapEvtEnqueuePenPoint: {
      PointType *ptP = sys_va_arg(ap, PointType *);
      Err ret = EvtEnqueuePenPoint(ptP);
      *iret = ret;
      }
      break;

    case sysTrapEvtEventAvail: {
      Boolean ret = EvtEventAvail();
      *iret = ret;
      }
      break;

    case sysTrapEvtFlushKeyQueue: {
      Err ret = EvtFlushKeyQueue();
      *iret = ret;
      }
      break;

    case sysTrapEvtFlushNextPenStroke: {
      Err ret = EvtFlushNextPenStroke();
      *iret = ret;
      }
      break;

    case sysTrapEvtFlushPenQueue: {
      Err ret = EvtFlushPenQueue();
      *iret = ret;
      }
      break;

    case sysTrapEvtGetEvent: {
      EventType *event = sys_va_arg(ap, EventType *);
      Int32 timeout = sys_va_arg(ap, Int32);
      EvtGetEvent(event, timeout);
      }
      break;

    case sysTrapEvtGetPen: {
      Int16 *pScreenX = sys_va_arg(ap, Int16 *);
      Int16 *pScreenY = sys_va_arg(ap, Int16 *);
      Boolean *pPenDown = sys_va_arg(ap, Boolean *);
      EvtGetPen(pScreenX, pScreenY, pPenDown);
      }
      break;

    case sysTrapEvtGetPenBtnList: {
      UInt16 *numButtons = sys_va_arg(ap, UInt16 *);
      const PenBtnInfoType *ret = EvtGetPenBtnList(numButtons);
      *pret = (void *)ret;
      }
      break;

    case sysTrapEvtGetSilkscreenAreaList: {
      UInt16 *numAreas = sys_va_arg(ap, UInt16 *);
      const SilkscreenAreaType *ret = EvtGetSilkscreenAreaList(numAreas);
      *pret = (void *)ret;
      }
      break;

    case sysTrapEvtGetSysEvent: {
      SysEventType *eventP = sys_va_arg(ap, SysEventType *);
      Int32 timeout = sys_va_arg(ap, Int32);
      EvtGetSysEvent(eventP, timeout);
      }
      break;

    case sysTrapEvtKeyQueueEmpty: {
      Boolean ret = EvtKeyQueueEmpty();
      *iret = ret;
      }
      break;

    case sysTrapEvtKeyQueueSize: {
      UInt32 ret = EvtKeyQueueSize();
      *iret = ret;
      }
      break;

    case sysTrapEvtPenQueueSize: {
      UInt32 ret = EvtPenQueueSize();
      *iret = ret;
      }
      break;

    case sysTrapEvtProcessSoftKeyStroke: {
      PointType *startPtP = sys_va_arg(ap, PointType *);
      PointType *endPtP = sys_va_arg(ap, PointType *);
      Err ret = EvtProcessSoftKeyStroke(startPtP, endPtP);
      *iret = ret;
      }
      break;

    case sysTrapEvtResetAutoOffTimer: {
      Err ret = EvtResetAutoOffTimer();
      *iret = ret;
      }
      break;

    case sysTrapEvtSetAutoOffTimer: {
      EvtSetAutoOffCmd cmd = sys_va_arg(ap, EvtSetAutoOffCmd);
      UInt16 timeout = sys_va_arg(ap, UInt32);
      Err ret = EvtSetAutoOffTimer(cmd, timeout);
      *iret = ret;
      }
      break;

    case sysTrapEvtSetKeyQueuePtr: {
      MemPtr keyQueueP = sys_va_arg(ap, MemPtr);
      UInt32 size = sys_va_arg(ap, UInt32);
      Err ret = EvtSetKeyQueuePtr(keyQueueP, size);
      *iret = ret;
      }
      break;

    case sysTrapEvtSetNullEventTick: {
      UInt32 tick = sys_va_arg(ap, UInt32);
      Boolean ret = EvtSetNullEventTick(tick);
      *iret = ret;
      }
      break;

    case sysTrapEvtSetPenQueuePtr: {
      MemPtr penQueueP = sys_va_arg(ap, MemPtr);
      UInt32 size = sys_va_arg(ap, UInt32);
      Err ret = EvtSetPenQueuePtr(penQueueP, size);
      *iret = ret;
      }
      break;

    case sysTrapEvtSysEventAvail: {
      Boolean ignorePenUps = sys_va_arg(ap, UInt32);
      Boolean ret = EvtSysEventAvail(ignorePenUps);
      *iret = ret;
      }
      break;

    case sysTrapEvtSysInit: {
      Err ret = EvtSysInit();
      *iret = ret;
      }
      break;

    case sysTrapEvtWakeup: {
      Err ret = EvtWakeup();
      *iret = ret;
      }
      break;

    case sysTrapEvtWakeupWithoutNilEvent: {
      Err ret = EvtWakeupWithoutNilEvent();
      *iret = ret;
      }
      break;

    case sysTrapExgAccept: {
      ExgSocketType *socketP = sys_va_arg(ap, ExgSocketType *);
      Err ret = ExgAccept(socketP);
      *iret = ret;
      }
      break;

    case sysTrapExgConnect: {
      ExgSocketType *socketP = sys_va_arg(ap, ExgSocketType *);
      Err ret = ExgConnect(socketP);
      *iret = ret;
      }
      break;

    case sysTrapExgControl: {
      ExgSocketType *socketP = sys_va_arg(ap, ExgSocketType *);
      UInt16 op = sys_va_arg(ap, UInt32);
      void *valueP = sys_va_arg(ap, void *);
      UInt16 *valueLenP = sys_va_arg(ap, UInt16 *);
      Err ret = ExgControl(socketP, op, valueP, valueLenP);
      *iret = ret;
      }
      break;

    case sysTrapExgDBRead: {
      ExgDBReadProcPtr readProcP = sys_va_arg(ap, ExgDBReadProcPtr);
      ExgDBDeleteProcPtr deleteProcP = sys_va_arg(ap, ExgDBDeleteProcPtr);
      void *userDataP = sys_va_arg(ap, void *);
      LocalID *dbIDP = sys_va_arg(ap, LocalID *);
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      Boolean *needResetP = sys_va_arg(ap, Boolean *);
      Boolean keepDates = sys_va_arg(ap, UInt32);
      Err ret = ExgDBRead(readProcP, deleteProcP, userDataP, dbIDP, cardNo, needResetP, keepDates);
      *iret = ret;
      }
      break;

    case sysTrapExgDBWrite: {
      ExgDBWriteProcPtr writeProcP = sys_va_arg(ap, ExgDBWriteProcPtr);
      void *userDataP = sys_va_arg(ap, void *);
      const char *nameP = sys_va_arg(ap, char *);
      LocalID dbID = sys_va_arg(ap, LocalID);
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      Err ret = ExgDBWrite(writeProcP, userDataP, nameP, dbID, cardNo);
      *iret = ret;
      }
      break;

    case sysTrapExgDisconnect: {
      ExgSocketType *socketP = sys_va_arg(ap, ExgSocketType *);
      Err error = sys_va_arg(ap, UInt32);
      Err ret = ExgDisconnect(socketP, error);
      *iret = ret;
      }
      break;

    case sysTrapExgDoDialog: {
      ExgSocketType *socketP = sys_va_arg(ap, ExgSocketType *);
      ExgDialogInfoType *infoP = sys_va_arg(ap, ExgDialogInfoType *);
      Err *errP = sys_va_arg(ap, Err *);
      Boolean ret = ExgDoDialog(socketP, infoP, errP);
      *iret = ret;
      }
      break;

    case sysTrapExgGet: {
      ExgSocketType *socketP = sys_va_arg(ap, ExgSocketType *);
      Err ret = ExgGet(socketP);
      *iret = ret;
      }
      break;

    case sysTrapExgGetDefaultApplication: {
      UInt32 *creatorIDP = sys_va_arg(ap, UInt32 *);
      UInt16 id = sys_va_arg(ap, UInt32);
      const Char *dataTypeP = sys_va_arg(ap, Char *);
      Err ret = ExgGetDefaultApplication(creatorIDP, id, dataTypeP);
      *iret = ret;
      }
      break;

    case sysTrapExgGetRegisteredApplications: {
      UInt32 * *creatorIDsP = sys_va_arg(ap, UInt32 * *);
      UInt32 *numAppsP = sys_va_arg(ap, UInt32 *);
      Char * *namesP = sys_va_arg(ap, Char * *);
      Char * *descriptionsP = sys_va_arg(ap, Char * *);
      UInt16 id = sys_va_arg(ap, UInt32);
      const Char *dataTypeP = sys_va_arg(ap, Char *);
      Err ret = ExgGetRegisteredApplications(creatorIDsP, numAppsP, namesP, descriptionsP, id, dataTypeP);
      *iret = ret;
      }
      break;

    case sysTrapExgGetRegisteredTypes: {
      Char * *dataTypesP = sys_va_arg(ap, Char * *);
      UInt32 *sizeP = sys_va_arg(ap, UInt32 *);
      UInt16 id = sys_va_arg(ap, UInt32);
      Err ret = ExgGetRegisteredTypes(dataTypesP, sizeP, id);
      *iret = ret;
      }
      break;

    case sysTrapExgGetTargetApplication: {
      ExgSocketType *socketP = sys_va_arg(ap, ExgSocketType *);
      Boolean unwrap = sys_va_arg(ap, UInt32);
      UInt32 *creatorIDP = sys_va_arg(ap, UInt32 *);
      Char *descriptionP = sys_va_arg(ap, Char *);
      UInt32 descriptionSize = sys_va_arg(ap, UInt32);
      Err ret = ExgGetTargetApplication(socketP, unwrap, creatorIDP, descriptionP, descriptionSize);
      *iret = ret;
      }
      break;

    case sysTrapExgInit: {
      Err ret = ExgInit();
      *iret = ret;
      }
      break;

    case sysTrapExgNotifyGoto: {
      ExgSocketType *socketP = sys_va_arg(ap, ExgSocketType *);
      UInt16 flags = sys_va_arg(ap, UInt32);
      Err ret = ExgNotifyGoto(socketP, flags);
      *iret = ret;
      }
      break;

    case sysTrapExgNotifyPreview: {
      ExgPreviewInfoType *infoP = sys_va_arg(ap, ExgPreviewInfoType *);
      Err ret = ExgNotifyPreview(infoP);
      *iret = ret;
      }
      break;

    case sysTrapExgNotifyReceive: {
      ExgSocketType *socketP = sys_va_arg(ap, ExgSocketType *);
      UInt16 flags = sys_va_arg(ap, UInt32);
      Err ret = ExgNotifyReceive(socketP, flags);
      *iret = ret;
      }
      break;

    case sysTrapExgNotifyReceiveV35: {
      ExgSocketType *socketP = sys_va_arg(ap, ExgSocketType *);
      Err ret = ExgNotifyReceiveV35(socketP);
      *iret = ret;
      }
      break;

    case sysTrapExgPut: {
      ExgSocketType *socketP = sys_va_arg(ap, ExgSocketType *);
      Err ret = ExgPut(socketP);
      *iret = ret;
      }
      break;

    case sysTrapExgReceive: {
      ExgSocketType *socketP = sys_va_arg(ap, ExgSocketType *);
      void *bufP = sys_va_arg(ap, void *);
      UInt32 bufLen = sys_va_arg(ap, UInt32);
      Err *err = sys_va_arg(ap, Err *);
      UInt32 ret = ExgReceive(socketP, bufP, bufLen, err);
      *iret = ret;
      }
      break;

    case sysTrapExgRegisterData: {
      UInt32 creatorID = sys_va_arg(ap, UInt32);
      UInt16 id = sys_va_arg(ap, UInt32);
      const Char *dataTypesP = sys_va_arg(ap, Char *);
      Err ret = ExgRegisterData(creatorID, id, dataTypesP);
      *iret = ret;
      }
      break;

    case sysTrapExgRegisterDatatype: {
      UInt32 creatorID = sys_va_arg(ap, UInt32);
      UInt16 id = sys_va_arg(ap, UInt32);
      const Char *dataTypesP = sys_va_arg(ap, Char *);
      const Char *descriptionsP = sys_va_arg(ap, Char *);
      UInt16 flags = sys_va_arg(ap, UInt32);
      Err ret = ExgRegisterDatatype(creatorID, id, dataTypesP, descriptionsP, flags);
      *iret = ret;
      }
      break;

    case sysTrapExgRequest: {
      ExgSocketType *socketP = sys_va_arg(ap, ExgSocketType *);
      Err ret = ExgRequest(socketP);
      *iret = ret;
      }
      break;

    case sysTrapExgSend: {
      ExgSocketType *socketP = sys_va_arg(ap, ExgSocketType *);
      const void *bufP = sys_va_arg(ap, void *);
      UInt32 bufLen = sys_va_arg(ap, UInt32);
      Err *err = sys_va_arg(ap, Err *);
      UInt32 ret = ExgSend(socketP, bufP, bufLen, err);
      *iret = ret;
      }
      break;

    case sysTrapExgSetDefaultApplication: {
      UInt32 creatorID = sys_va_arg(ap, UInt32);
      UInt16 id = sys_va_arg(ap, UInt32);
      const Char *dataTypeP = sys_va_arg(ap, Char *);
      Err ret = ExgSetDefaultApplication(creatorID, id, dataTypeP);
      *iret = ret;
      }
      break;

    case sysTrapFileClose: {
      FileHand stream = sys_va_arg(ap, FileHand);
      Err ret = FileClose(stream);
      *iret = ret;
      }
      break;

    case sysTrapFileControl: {
      FileOpEnum op = sys_va_arg(ap, FileOpEnum);
      FileHand stream = sys_va_arg(ap, FileHand);
      void *valueP = sys_va_arg(ap, void *);
      Int32 *valueLenP = sys_va_arg(ap, Int32 *);
      Err ret = FileControl(op, stream, valueP, valueLenP);
      *iret = ret;
      }
      break;

    case sysTrapFileDelete: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      const Char *nameP = sys_va_arg(ap, Char *);
      Err ret = FileDelete(cardNo, nameP);
      *iret = ret;
      }
      break;

    case sysTrapFileOpen: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      const Char *nameP = sys_va_arg(ap, Char *);
      UInt32 type = sys_va_arg(ap, UInt32);
      UInt32 creator = sys_va_arg(ap, UInt32);
      UInt32 openMode = sys_va_arg(ap, UInt32);
      Err *errP = sys_va_arg(ap, Err *);
      FileHand ret = FileOpen(cardNo, nameP, type, creator, openMode, errP);
      *pret = (void *)ret;
      }
      break;

    case sysTrapFileReadLow: {
      FileHand stream = sys_va_arg(ap, FileHand);
      void *baseP = sys_va_arg(ap, void *);
      Int32 offset = sys_va_arg(ap, Int32);
      Boolean dataStoreBased = sys_va_arg(ap, UInt32);
      Int32 objSize = sys_va_arg(ap, Int32);
      Int32 numObj = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      Int32 ret = FileReadLow(stream, baseP, offset, dataStoreBased, objSize, numObj, errP);
      *iret = ret;
      }
      break;

    case sysTrapFileSeek: {
      FileHand stream = sys_va_arg(ap, FileHand);
      Int32 offset = sys_va_arg(ap, Int32);
      FileOriginEnum origin = sys_va_arg(ap, FileOriginEnum);
      Err ret = FileSeek(stream, offset, origin);
      *iret = ret;
      }
      break;

    case sysTrapFileTell: {
      FileHand stream = sys_va_arg(ap, FileHand);
      Int32 *fileSizeP = sys_va_arg(ap, Int32 *);
      Err *errP = sys_va_arg(ap, Err *);
      Int32 ret = FileTell(stream, fileSizeP, errP);
      *iret = ret;
      }
      break;

    case sysTrapFileTruncate: {
      FileHand stream = sys_va_arg(ap, FileHand);
      Int32 newSize = sys_va_arg(ap, Int32);
      Err ret = FileTruncate(stream, newSize);
      *iret = ret;
      }
      break;

    case sysTrapFileWrite: {
      FileHand stream = sys_va_arg(ap, FileHand);
      const void *dataP = sys_va_arg(ap, void *);
      Int32 objSize = sys_va_arg(ap, Int32);
      Int32 numObj = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      Int32 ret = FileWrite(stream, dataP, objSize, numObj, errP);
      *iret = ret;
      }
      break;

    case sysTrapFind: {
      GoToParamsPtr goToP = sys_va_arg(ap, GoToParamsPtr);
      Find(goToP);
      }
      break;

    case sysTrapFindDrawHeader: {
      FindParamsPtr findParams = sys_va_arg(ap, FindParamsPtr);
      Char const *title = sys_va_arg(ap, Char const *);
      Boolean ret = FindDrawHeader(findParams, title);
      *iret = ret;
      }
      break;

    case sysTrapFindGetLineBounds: {
      const FindParamsType *findParams = sys_va_arg(ap, FindParamsType *);
      RectanglePtr r = sys_va_arg(ap, RectanglePtr);
      FindGetLineBounds(findParams, r);
      }
      break;

    case sysTrapFindSaveMatch: {
      FindParamsPtr findParams = sys_va_arg(ap, FindParamsPtr);
      UInt16 recordNum = sys_va_arg(ap, UInt32);
      UInt16 pos = sys_va_arg(ap, UInt32);
      UInt16 fieldNum = sys_va_arg(ap, UInt32);
      UInt32 appCustom = sys_va_arg(ap, UInt32);
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      Boolean ret = FindSaveMatch(findParams, recordNum, pos, fieldNum, appCustom, cardNo, dbID);
      *iret = ret;
      }
      break;

    case sysTrapFindStrInStr: {
      Char const *strToSearch = sys_va_arg(ap, Char const *);
      Char const *strToFind = sys_va_arg(ap, Char const *);
      UInt16 *posP = sys_va_arg(ap, UInt16 *);
      Boolean ret = FindStrInStr(strToSearch, strToFind, posP);
      *iret = ret;
      }
      break;

    case sysTrapFldCalcFieldHeight: {
      const Char *chars = sys_va_arg(ap, Char *);
      UInt16 maxWidth = sys_va_arg(ap, UInt32);
      UInt16 ret = FldCalcFieldHeight(chars, maxWidth);
      *iret = ret;
      }
      break;

    case sysTrapFldCompactText: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      FldCompactText(fldP);
      }
      break;

    case sysTrapFldCopy: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      FldCopy(fldP);
      }
      break;

    case sysTrapFldCut: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      FldCut(fldP);
      }
      break;

    case sysTrapFldDelete: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 start = sys_va_arg(ap, UInt32);
      UInt16 end = sys_va_arg(ap, UInt32);
      FldDelete(fldP, start, end);
      }
      break;

    case sysTrapFldDirty: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      Boolean ret = FldDirty(fldP);
      *iret = ret;
      }
      break;

    case sysTrapFldDrawField: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      FldDrawField(fldP);
      }
      break;

    case sysTrapFldEraseField: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      FldEraseField(fldP);
      }
      break;

    case sysTrapFldFreeMemory: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      FldFreeMemory(fldP);
      }
      break;

    case sysTrapFldGetAttributes: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      FieldAttrPtr attrP = sys_va_arg(ap, FieldAttrPtr);
      FldGetAttributes(fldP, attrP);
      }
      break;

    case sysTrapFldGetBounds: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      RectanglePtr rect = sys_va_arg(ap, RectanglePtr);
      FldGetBounds(fldP, rect);
      }
      break;

    case sysTrapFldGetFont: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      FontID ret = FldGetFont(fldP);
      *iret = ret;
      }
      break;

    case sysTrapFldGetInsPtPosition: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 ret = FldGetInsPtPosition(fldP);
      *iret = ret;
      }
      break;

    case sysTrapFldGetMaxChars: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 ret = FldGetMaxChars(fldP);
      *iret = ret;
      }
      break;

    case sysTrapFldGetNumberOfBlankLines: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 ret = FldGetNumberOfBlankLines(fldP);
      *iret = ret;
      }
      break;

    case sysTrapFldGetScrollPosition: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 ret = FldGetScrollPosition(fldP);
      *iret = ret;
      }
      break;

    case sysTrapFldGetScrollValues: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 *scrollPosP = sys_va_arg(ap, UInt16 *);
      UInt16 *textHeightP = sys_va_arg(ap, UInt16 *);
      UInt16 *fieldHeightP = sys_va_arg(ap, UInt16 *);
      FldGetScrollValues(fldP, scrollPosP, textHeightP, fieldHeightP);
      }
      break;

    case sysTrapFldGetSelection: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 *startPosition = sys_va_arg(ap, UInt16 *);
      UInt16 *endPosition = sys_va_arg(ap, UInt16 *);
      FldGetSelection(fldP, startPosition, endPosition);
      }
      break;

    case sysTrapFldGetTextAllocatedSize: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 ret = FldGetTextAllocatedSize(fldP);
      *iret = ret;
      }
      break;

    case sysTrapFldGetTextHandle: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      MemHandle ret = FldGetTextHandle(fldP);
      *pret = (void *)ret;
      }
      break;

    case sysTrapFldGetTextHeight: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 ret = FldGetTextHeight(fldP);
      *iret = ret;
      }
      break;

    case sysTrapFldGetTextLength: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 ret = FldGetTextLength(fldP);
      *iret = ret;
      }
      break;

    case sysTrapFldGetTextPtr: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      Char *ret = FldGetTextPtr(fldP);
      *pret = (void *)ret;
      }
      break;

    case sysTrapFldGetVisibleLines: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 ret = FldGetVisibleLines(fldP);
      *iret = ret;
      }
      break;

    case sysTrapFldGrabFocus: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      FldGrabFocus(fldP);
      }
      break;

    case sysTrapFldHandleEvent: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      EventType *eventP = sys_va_arg(ap, EventType *);
      Boolean ret = FldHandleEvent(fldP, eventP);
      *iret = ret;
      }
      break;

    case sysTrapFldInsert: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      const Char *insertChars = sys_va_arg(ap, Char *);
      UInt16 insertLen = sys_va_arg(ap, UInt32);
      Boolean ret = FldInsert(fldP, insertChars, insertLen);
      *iret = ret;
      }
      break;

    case sysTrapFldMakeFullyVisible: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      Boolean ret = FldMakeFullyVisible(fldP);
      *iret = ret;
      }
      break;

    case sysTrapFldNewField: {
      void * *formPP = sys_va_arg(ap, void * *);
      UInt16 id = sys_va_arg(ap, UInt32);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      Coord width = sys_va_arg(ap, Int32);
      Coord height = sys_va_arg(ap, Int32);
      FontID font = sys_va_arg(ap, FontID);
      UInt32 maxChars = sys_va_arg(ap, UInt32);
      Boolean editable = sys_va_arg(ap, UInt32);
      Boolean underlined = sys_va_arg(ap, UInt32);
      Boolean singleLine = sys_va_arg(ap, UInt32);
      Boolean dynamicSize = sys_va_arg(ap, UInt32);
      JustificationType justification = sys_va_arg(ap, JustificationType);
      Boolean autoShift = sys_va_arg(ap, UInt32);
      Boolean hasScrollBar = sys_va_arg(ap, UInt32);
      Boolean numeric = sys_va_arg(ap, UInt32);
      FieldType *ret = FldNewField(formPP, id, x, y, width, height, font, maxChars, editable, underlined, singleLine, dynamicSize, justification, autoShift, hasScrollBar, numeric);
      *pret = (void *)ret;
      }
      break;

    case sysTrapFldPaste: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      FldPaste(fldP);
      }
      break;

    case sysTrapFldRecalculateField: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      Boolean redraw = sys_va_arg(ap, UInt32);
      FldRecalculateField(fldP, redraw);
      }
      break;

    case sysTrapFldReleaseFocus: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      FldReleaseFocus(fldP);
      }
      break;

    case sysTrapFldScrollField: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 linesToScroll = sys_va_arg(ap, UInt32);
      WinDirectionType direction = sys_va_arg(ap, WinDirectionType);
      FldScrollField(fldP, linesToScroll, direction);
      }
      break;

    case sysTrapFldScrollable: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      WinDirectionType direction = sys_va_arg(ap, WinDirectionType);
      Boolean ret = FldScrollable(fldP, direction);
      *iret = ret;
      }
      break;

    case sysTrapFldSendChangeNotification: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      FldSendChangeNotification(fldP);
      }
      break;

    case sysTrapFldSendHeightChangeNotification: {
      const FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 pos = sys_va_arg(ap, UInt32);
      Int16 numLines = sys_va_arg(ap, Int32);
      FldSendHeightChangeNotification(fldP, pos, numLines);
      }
      break;

    case sysTrapFldSetAttributes: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      const FieldAttrType *attrP = sys_va_arg(ap, FieldAttrType *);
      FldSetAttributes(fldP, attrP);
      }
      break;

    case sysTrapFldSetBounds: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      FldSetBounds(fldP, rP);
      }
      break;

    case sysTrapFldSetDirty: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      Boolean dirty = sys_va_arg(ap, UInt32);
      FldSetDirty(fldP, dirty);
      }
      break;

    case sysTrapFldSetFont: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      FontID fontID = sys_va_arg(ap, FontID);
      FldSetFont(fldP, fontID);
      }
      break;

    case sysTrapFldSetInsPtPosition: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 pos = sys_va_arg(ap, UInt32);
      FldSetInsPtPosition(fldP, pos);
      }
      break;

    case sysTrapFldSetInsertionPoint: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 pos = sys_va_arg(ap, UInt32);
      FldSetInsertionPoint(fldP, pos);
      }
      break;

    case sysTrapFldSetMaxChars: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 maxChars = sys_va_arg(ap, UInt32);
      FldSetMaxChars(fldP, maxChars);
      }
      break;

    case sysTrapFldSetMaxVisibleLines: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt8 maxLines = sys_va_arg(ap, UInt32);
      FldSetMaxVisibleLines(fldP, maxLines);
      }
      break;

    case sysTrapFldSetScrollPosition: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 pos = sys_va_arg(ap, UInt32);
      FldSetScrollPosition(fldP, pos);
      }
      break;

    case sysTrapFldSetSelection: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 startPosition = sys_va_arg(ap, UInt32);
      UInt16 endPosition = sys_va_arg(ap, UInt32);
      FldSetSelection(fldP, startPosition, endPosition);
      }
      break;

    case sysTrapFldSetText: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      MemHandle textHandle = sys_va_arg(ap, MemHandle);
      UInt16 offset = sys_va_arg(ap, UInt32);
      UInt16 size = sys_va_arg(ap, UInt32);
      FldSetText(fldP, textHandle, offset, size);
      }
      break;

    case sysTrapFldSetTextAllocatedSize: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      UInt16 allocatedSize = sys_va_arg(ap, UInt32);
      FldSetTextAllocatedSize(fldP, allocatedSize);
      }
      break;

    case sysTrapFldSetTextHandle: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      MemHandle textHandle = sys_va_arg(ap, MemHandle);
      FldSetTextHandle(fldP, textHandle);
      }
      break;

    case sysTrapFldSetTextPtr: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      Char *textP = sys_va_arg(ap, Char *);
      FldSetTextPtr(fldP, textP);
      }
      break;

    case sysTrapFldSetUsable: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      Boolean usable = sys_va_arg(ap, UInt32);
      FldSetUsable(fldP, usable);
      }
      break;

    case sysTrapFldUndo: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      FldUndo(fldP);
      }
      break;

    case sysTrapFldWordWrap: {
      const Char *chars = sys_va_arg(ap, Char *);
      Int16 maxWidth = sys_va_arg(ap, Int32);
      UInt16 ret = FldWordWrap(chars, maxWidth);
      *iret = ret;
      }
      break;

    case sysTrapFntAverageCharWidth: {
      Int16 ret = FntAverageCharWidth();
      *iret = ret;
      }
      break;

    case sysTrapFntBaseLine: {
      Int16 ret = FntBaseLine();
      *iret = ret;
      }
      break;

    case sysTrapFntCharHeight: {
      Int16 ret = FntCharHeight();
      *iret = ret;
      }
      break;

    case sysTrapFntCharWidth: {
      Char ch = sys_va_arg(ap, Int32);
      Int16 ret = FntCharWidth(ch);
      *iret = ret;
      }
      break;

    case sysTrapFntCharsInWidth: {
      Char const *string = sys_va_arg(ap, Char const *);
      Int16 *stringWidthP = sys_va_arg(ap, Int16 *);
      Int16 *stringLengthP = sys_va_arg(ap, Int16 *);
      Boolean *fitWithinWidth = sys_va_arg(ap, Boolean *);
      FntCharsInWidth(string, stringWidthP, stringLengthP, fitWithinWidth);
      }
      break;

    case sysTrapFntCharsWidth: {
      Char const *chars = sys_va_arg(ap, Char const *);
      Int16 len = sys_va_arg(ap, Int32);
      Int16 ret = FntCharsWidth(chars, len);
      *iret = ret;
      }
      break;

    case sysTrapFntDefineFont: {
      FontID font = sys_va_arg(ap, FontID);
      FontType *fontP = sys_va_arg(ap, FontType *);
      Err ret = FntDefineFont(font, fontP);
      *iret = ret;
      }
      break;

    case sysTrapFntDescenderHeight: {
      Int16 ret = FntDescenderHeight();
      *iret = ret;
      }
      break;

    case sysTrapFntGetFont: {
      FontID ret = FntGetFont();
      *iret = ret;
      }
      break;

    case sysTrapFntGetFontPtr: {
      FontType *ret = FntGetFontPtr();
      *pret = (void *)ret;
      }
      break;

    case sysTrapFntGetScrollValues: {
      Char const *chars = sys_va_arg(ap, Char const *);
      UInt16 width = sys_va_arg(ap, UInt32);
      UInt16 scrollPos = sys_va_arg(ap, UInt32);
      UInt16 *linesP = sys_va_arg(ap, UInt16 *);
      UInt16 *topLine = sys_va_arg(ap, UInt16 *);
      FntGetScrollValues(chars, width, scrollPos, linesP, topLine);
      }
      break;

    case sysTrapFntLineHeight: {
      Int16 ret = FntLineHeight();
      *iret = ret;
      }
      break;

    case sysTrapFntLineWidth: {
      Char const *pChars = sys_va_arg(ap, Char const *);
      UInt16 length = sys_va_arg(ap, UInt32);
      Int16 ret = FntLineWidth(pChars, length);
      *iret = ret;
      }
      break;

    case sysTrapFntSetFont: {
      FontID font = sys_va_arg(ap, FontID);
      FontID ret = FntSetFont(font);
      *iret = ret;
      }
      break;

    case sysTrapFntWCharWidth: {
      WChar iChar = sys_va_arg(ap, UInt32);
      Int16 ret = FntWCharWidth(iChar);
      *iret = ret;
      }
      break;

    case sysTrapFntWidthToOffset: {
      Char const *pChars = sys_va_arg(ap, Char const *);
      UInt16 length = sys_va_arg(ap, UInt32);
      Int16 pixelWidth = sys_va_arg(ap, Int32);
      Boolean *leadingEdge = sys_va_arg(ap, Boolean *);
      Int16 *truncWidth = sys_va_arg(ap, Int16 *);
      Int16 ret = FntWidthToOffset(pChars, length, pixelWidth, leadingEdge, truncWidth);
      *iret = ret;
      }
      break;

    case sysTrapFntWordWrap: {
      Char const *chars = sys_va_arg(ap, Char const *);
      UInt16 maxWidth = sys_va_arg(ap, UInt32);
      UInt16 ret = FntWordWrap(chars, maxWidth);
      *iret = ret;
      }
      break;

    case sysTrapFntWordWrapReverseNLines: {
      Char const * const chars = sys_va_arg(ap, Char const * const);
      UInt16 maxWidth = sys_va_arg(ap, UInt32);
      UInt16 *linesToScrollP = sys_va_arg(ap, UInt16 *);
      UInt16 *scrollPosP = sys_va_arg(ap, UInt16 *);
      FntWordWrapReverseNLines(chars, maxWidth, linesToScrollP, scrollPosP);
      }
      break;

    case sysTrapFontSelect: {
      FontID fontID = sys_va_arg(ap, FontID);
      FontID ret = FontSelect(fontID);
      *iret = ret;
      }
      break;

    case sysTrapFrmActiveState: {
      FormActiveStateType *stateP = sys_va_arg(ap, FormActiveStateType *);
      Boolean save = sys_va_arg(ap, UInt32);
      Err ret = FrmActiveState(stateP, save);
      *iret = ret;
      }
      break;

    case sysTrapFrmAddSpaceForObject: {
      FormType * *formPP = sys_va_arg(ap, FormType * *);
      MemPtr *objectPP = sys_va_arg(ap, MemPtr *);
      FormObjectKind objectKind = sys_va_arg(ap, FormObjectKind);
      UInt16 objectSize = sys_va_arg(ap, UInt32);
      Err ret = FrmAddSpaceForObject(formPP, objectPP, objectKind, objectSize);
      *iret = ret;
      }
      break;

    case sysTrapFrmAlert: {
      UInt16 alertId = sys_va_arg(ap, UInt32);
      UInt16 ret = FrmAlert(alertId);
      *iret = ret;
      }
      break;

    case sysTrapFrmCloseAllForms: {
      FrmCloseAllForms();
      }
      break;

    case sysTrapFrmCopyLabel: {
      FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 labelID = sys_va_arg(ap, UInt32);
      const Char *newLabel = sys_va_arg(ap, Char *);
      FrmCopyLabel(formP, labelID, newLabel);
      }
      break;

    case sysTrapFrmCopyTitle: {
      FormType *formP = sys_va_arg(ap, FormType *);
      const Char *newTitle = sys_va_arg(ap, Char *);
      FrmCopyTitle(formP, newTitle);
      }
      break;

    case sysTrapFrmCustomAlert: {
      UInt16 alertId = sys_va_arg(ap, UInt32);
      const Char *s1 = sys_va_arg(ap, Char *);
      const Char *s2 = sys_va_arg(ap, Char *);
      const Char *s3 = sys_va_arg(ap, Char *);
      UInt16 ret = FrmCustomAlert(alertId, s1, s2, s3);
      *iret = ret;
      }
      break;

    case sysTrapFrmCustomResponseAlert: {
      UInt16 alertId = sys_va_arg(ap, UInt32);
      const Char *s1 = sys_va_arg(ap, Char *);
      const Char *s2 = sys_va_arg(ap, Char *);
      const Char *s3 = sys_va_arg(ap, Char *);
      Char *entryStringBuf = sys_va_arg(ap, Char *);
      Int16 entryStringBufLength = sys_va_arg(ap, Int32);
      FormCheckResponseFuncPtr callback = sys_va_arg(ap, FormCheckResponseFuncPtr);
      UInt16 ret = FrmCustomResponseAlert(alertId, s1, s2, s3, entryStringBuf, entryStringBufLength, callback);
      *iret = ret;
      }
      break;

    case sysTrapFrmDeleteForm: {
      FormType *formP = sys_va_arg(ap, FormType *);
      FrmDeleteForm(formP);
      }
      break;

    case sysTrapFrmDispatchEvent: {
      EventType *eventP = sys_va_arg(ap, EventType *);
      Boolean ret = FrmDispatchEvent(eventP);
      *iret = ret;
      }
      break;

    case sysTrapFrmDoDialog: {
      FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 ret = FrmDoDialog(formP);
      *iret = ret;
      }
      break;

    case sysTrapFrmDrawForm: {
      FormType *formP = sys_va_arg(ap, FormType *);
      FrmDrawForm(formP);
      }
      break;

    case sysTrapFrmEraseForm: {
      FormType *formP = sys_va_arg(ap, FormType *);
      FrmEraseForm(formP);
      }
      break;

    case sysTrapFrmGetActiveField: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      FieldType *ret = FrmGetActiveField(formP);
      *pret = (void *)ret;
      }
      break;

    case sysTrapFrmGetActiveForm: {
      FormType *ret = FrmGetActiveForm();
      *pret = (void *)ret;
      }
      break;

    case sysTrapFrmGetActiveFormID: {
      UInt16 ret = FrmGetActiveFormID();
      *iret = ret;
      }
      break;

    case sysTrapFrmGetControlGroupSelection: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt8 groupNum = sys_va_arg(ap, UInt32);
      UInt16 ret = FrmGetControlGroupSelection(formP, groupNum);
      *iret = ret;
      }
      break;

    case sysTrapFrmGetControlValue: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, UInt32);
      Int16 ret = FrmGetControlValue(formP, objIndex);
      *iret = ret;
      }
      break;

    case sysTrapFrmGetFirstForm: {
      FormType *ret = FrmGetFirstForm();
      *pret = (void *)ret;
      }
      break;

    case sysTrapFrmGetFocus: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 ret = FrmGetFocus(formP);
      *iret = ret;
      }
      break;

    case sysTrapFrmGetFormBounds: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      RectangleType *rP = sys_va_arg(ap, RectangleType *);
      FrmGetFormBounds(formP, rP);
      }
      break;

    case sysTrapFrmGetFormId: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 ret = FrmGetFormId(formP);
      *iret = ret;
      }
      break;

    case sysTrapFrmGetFormPtr: {
      UInt16 formId = sys_va_arg(ap, UInt32);
      FormType *ret = FrmGetFormPtr(formId);
      *pret = (void *)ret;
      }
      break;

    case sysTrapFrmGetGadgetData: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, UInt32);
      void *ret = FrmGetGadgetData(formP, objIndex);
      *pret = (void *)ret;
      }
      break;

    case sysTrapFrmGetLabel: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 labelID = sys_va_arg(ap, UInt32);
      const Char *ret = FrmGetLabel(formP, labelID);
      *pret = (void *)ret;
      }
      break;

    case sysTrapFrmGetNumberOfObjects: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 ret = FrmGetNumberOfObjects(formP);
      *iret = ret;
      }
      break;

    case sysTrapFrmGetObjectBounds: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, UInt32);
      RectangleType *rP = sys_va_arg(ap, RectangleType *);
      FrmGetObjectBounds(formP, objIndex, rP);
      }
      break;

    case sysTrapFrmGetObjectId: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, UInt32);
      UInt16 ret = FrmGetObjectId(formP, objIndex);
      *iret = ret;
      }
      break;

    case sysTrapFrmGetObjectIndex: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 objID = sys_va_arg(ap, UInt32);
      UInt16 ret = FrmGetObjectIndex(formP, objID);
      *iret = ret;
      }
      break;

    case sysTrapFrmGetObjectIndexFromPtr: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      void *objP = sys_va_arg(ap, void *);
      UInt16 ret = FrmGetObjectIndexFromPtr(formP, objP);
      *iret = ret;
      }
      break;

    case sysTrapFrmGetObjectPosition: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, UInt32);
      Coord *x = sys_va_arg(ap, Coord *);
      Coord *y = sys_va_arg(ap, Coord *);
      FrmGetObjectPosition(formP, objIndex, x, y);
      }
      break;

    case sysTrapFrmGetObjectPtr: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, UInt32);
      void *ret = FrmGetObjectPtr(formP, objIndex);
      *pret = (void *)ret;
      }
      break;

    case sysTrapFrmGetObjectType: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, UInt32);
      FormObjectKind ret = FrmGetObjectType(formP, objIndex);
      *iret = ret;
      }
      break;

    case sysTrapFrmGetTitle: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      const Char *ret = FrmGetTitle(formP);
      *pret = (void *)ret;
      }
      break;

    case sysTrapFrmGetUserModifiedState: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      Boolean ret = FrmGetUserModifiedState(formP);
      *iret = ret;
      }
      break;

    case sysTrapFrmGetWindowHandle: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      WinHandle ret = FrmGetWindowHandle(formP);
      *pret = (void *)ret;
      }
      break;

    case sysTrapFrmGotoForm: {
      UInt16 formId = sys_va_arg(ap, UInt32);
      FrmGotoForm(formId);
      }
      break;

    case sysTrapFrmHandleEvent: {
      FormType *formP = sys_va_arg(ap, FormType *);
      EventType *eventP = sys_va_arg(ap, EventType *);
      Boolean ret = FrmHandleEvent(formP, eventP);
      *iret = ret;
      }
      break;

    case sysTrapFrmHelp: {
      UInt16 helpMsgId = sys_va_arg(ap, UInt32);
      FrmHelp(helpMsgId);
      }
      break;

    case sysTrapFrmHideObject: {
      FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, UInt32);
      FrmHideObject(formP, objIndex);
      }
      break;

    case sysTrapFrmInitForm: {
      UInt16 rscID = sys_va_arg(ap, UInt32);
      FormType *ret = FrmInitForm(rscID);
      *pret = (void *)ret;
      }
      break;

    case sysTrapFrmNewBitmap: {
      FormType * *formPP = sys_va_arg(ap, FormType * *);
      UInt16 ID = sys_va_arg(ap, UInt32);
      UInt16 rscID = sys_va_arg(ap, UInt32);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      FormBitmapType *ret = FrmNewBitmap(formPP, ID, rscID, x, y);
      *pret = (void *)ret;
      }
      break;

    case sysTrapFrmNewForm: {
      UInt16 formID = sys_va_arg(ap, UInt32);
      const Char *titleStrP = sys_va_arg(ap, Char *);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      Coord width = sys_va_arg(ap, Int32);
      Coord height = sys_va_arg(ap, Int32);
      Boolean modal = sys_va_arg(ap, UInt32);
      UInt16 defaultButton = sys_va_arg(ap, UInt32);
      UInt16 helpRscID = sys_va_arg(ap, UInt32);
      UInt16 menuRscID = sys_va_arg(ap, UInt32);
      FormType *ret = FrmNewForm(formID, titleStrP, x, y, width, height, modal, defaultButton, helpRscID, menuRscID);
      *pret = (void *)ret;
      }
      break;

    case sysTrapFrmNewGadget: {
      FormType * *formPP = sys_va_arg(ap, FormType * *);
      UInt16 id = sys_va_arg(ap, UInt32);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      Coord width = sys_va_arg(ap, Int32);
      Coord height = sys_va_arg(ap, Int32);
      FormGadgetType *ret = FrmNewGadget(formPP, id, x, y, width, height);
      *pret = (void *)ret;
      }
      break;

    case sysTrapFrmNewGsi: {
      FormType * *formPP = sys_va_arg(ap, FormType * *);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      FrmGraffitiStateType *ret = FrmNewGsi(formPP, x, y);
      *pret = (void *)ret;
      }
      break;

    case sysTrapFrmNewLabel: {
      FormType * *formPP = sys_va_arg(ap, FormType * *);
      UInt16 ID = sys_va_arg(ap, UInt32);
      const Char *textP = sys_va_arg(ap, Char *);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      FontID font = sys_va_arg(ap, FontID);
      FormLabelType *ret = FrmNewLabel(formPP, ID, textP, x, y, font);
      *pret = (void *)ret;
      }
      break;

    case sysTrapFrmPointInTitle: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      Boolean ret = FrmPointInTitle(formP, x, y);
      *iret = ret;
      }
      break;

    case sysTrapFrmPopupForm: {
      UInt16 formId = sys_va_arg(ap, UInt32);
      FrmPopupForm(formId);
      }
      break;

    case sysTrapFrmRemoveObject: {
      FormType * *formPP = sys_va_arg(ap, FormType * *);
      UInt16 objIndex = sys_va_arg(ap, UInt32);
      Err ret = FrmRemoveObject(formPP, objIndex);
      *iret = ret;
      }
      break;

    case sysTrapFrmReturnToForm: {
      UInt16 formId = sys_va_arg(ap, UInt32);
      FrmReturnToForm(formId);
      }
      break;

    case sysTrapFrmSaveAllForms: {
      FrmSaveAllForms();
      }
      break;

    case sysTrapFrmSetActiveForm: {
      FormType *formP = sys_va_arg(ap, FormType *);
      FrmSetActiveForm(formP);
      }
      break;

    case sysTrapFrmSetCategoryLabel: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, UInt32);
      Char *newLabel = sys_va_arg(ap, Char *);
      FrmSetCategoryLabel(formP, objIndex, newLabel);
      }
      break;

    case sysTrapFrmSetControlGroupSelection: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt8 groupNum = sys_va_arg(ap, UInt32);
      UInt16 controlID = sys_va_arg(ap, UInt32);
      FrmSetControlGroupSelection(formP, groupNum, controlID);
      }
      break;

    case sysTrapFrmSetControlValue: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, UInt32);
      Int16 newValue = sys_va_arg(ap, Int32);
      FrmSetControlValue(formP, objIndex, newValue);
      }
      break;

    case sysTrapFrmSetEventHandler: {
      FormType *formP = sys_va_arg(ap, FormType *);
      FormEventHandlerType *handler = sys_va_arg(ap, FormEventHandlerType *);
      FrmSetEventHandler(formP, handler);
      }
      break;

    case sysTrapFrmSetFocus: {
      FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 fieldIndex = sys_va_arg(ap, UInt32);
      FrmSetFocus(formP, fieldIndex);
      }
      break;

    case sysTrapFrmSetGadgetData: {
      FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, UInt32);
      const void *data = sys_va_arg(ap, void *);
      FrmSetGadgetData(formP, objIndex, data);
      }
      break;

    case sysTrapFrmSetGadgetHandler: {
      FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, UInt32);
      FormGadgetHandlerType *attrP = sys_va_arg(ap, FormGadgetHandlerType *);
      FrmSetGadgetHandler(formP, objIndex, attrP);
      }
      break;

    case sysTrapFrmSetMenu: {
      FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 menuRscID = sys_va_arg(ap, UInt32);
      FrmSetMenu(formP, menuRscID);
      }
      break;

    case sysTrapFrmSetNotUserModified: {
      FormType *formP = sys_va_arg(ap, FormType *);
      FrmSetNotUserModified(formP);
      }
      break;

    case sysTrapFrmSetObjectBounds: {
      FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, UInt32);
      const RectangleType *bounds = sys_va_arg(ap, RectangleType *);
      FrmSetObjectBounds(formP, objIndex, bounds);
      }
      break;

    case sysTrapFrmSetObjectPosition: {
      FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, UInt32);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      FrmSetObjectPosition(formP, objIndex, x, y);
      }
      break;

    case sysTrapFrmSetTitle: {
      FormType *formP = sys_va_arg(ap, FormType *);
      Char *newTitle = sys_va_arg(ap, Char *);
      FrmSetTitle(formP, newTitle);
      }
      break;

    case sysTrapFrmShowObject: {
      FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 objIndex = sys_va_arg(ap, UInt32);
      FrmShowObject(formP, objIndex);
      }
      break;

    case sysTrapFrmUpdateForm: {
      UInt16 formId = sys_va_arg(ap, UInt32);
      UInt16 updateCode = sys_va_arg(ap, UInt32);
      FrmUpdateForm(formId, updateCode);
      }
      break;

    case sysTrapFrmUpdateScrollers: {
      FormType *formP = sys_va_arg(ap, FormType *);
      UInt16 upIndex = sys_va_arg(ap, UInt32);
      UInt16 downIndex = sys_va_arg(ap, UInt32);
      Boolean scrollableUp = sys_va_arg(ap, UInt32);
      Boolean scrollableDown = sys_va_arg(ap, UInt32);
      FrmUpdateScrollers(formP, upIndex, downIndex, scrollableUp, scrollableDown);
      }
      break;

    case sysTrapFrmValidatePtr: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      Boolean ret = FrmValidatePtr(formP);
      *iret = ret;
      }
      break;

    case sysTrapFrmVisible: {
      const FormType *formP = sys_va_arg(ap, FormType *);
      Boolean ret = FrmVisible(formP);
      *iret = ret;
      }
      break;

    case sysTrapFtrGet: {
      UInt32 creator = sys_va_arg(ap, UInt32);
      UInt16 featureNum = sys_va_arg(ap, UInt32);
      UInt32 *valueP = sys_va_arg(ap, UInt32 *);
      Err ret = FtrGet(creator, featureNum, valueP);
      *iret = ret;
      }
      break;

    case sysTrapFtrGetByIndex: {
      UInt16 index = sys_va_arg(ap, UInt32);
      Boolean romTable = sys_va_arg(ap, UInt32);
      UInt32 *creatorP = sys_va_arg(ap, UInt32 *);
      UInt16 *numP = sys_va_arg(ap, UInt16 *);
      UInt32 *valueP = sys_va_arg(ap, UInt32 *);
      Err ret = FtrGetByIndex(index, romTable, creatorP, numP, valueP);
      *iret = ret;
      }
      break;

    case sysTrapFtrInit: {
      Err ret = FtrInit();
      *iret = ret;
      }
      break;

    case sysTrapFtrPtrFree: {
      UInt32 creator = sys_va_arg(ap, UInt32);
      UInt16 featureNum = sys_va_arg(ap, UInt32);
      Err ret = FtrPtrFree(creator, featureNum);
      *iret = ret;
      }
      break;

    case sysTrapFtrPtrNew: {
      UInt32 creator = sys_va_arg(ap, UInt32);
      UInt16 featureNum = sys_va_arg(ap, UInt32);
      UInt32 size = sys_va_arg(ap, UInt32);
      void * *newPtrP = sys_va_arg(ap, void * *);
      Err ret = FtrPtrNew(creator, featureNum, size, newPtrP);
      *iret = ret;
      }
      break;

    case sysTrapFtrPtrResize: {
      UInt32 creator = sys_va_arg(ap, UInt32);
      UInt16 featureNum = sys_va_arg(ap, UInt32);
      UInt32 newSize = sys_va_arg(ap, UInt32);
      void * *newPtrP = sys_va_arg(ap, void * *);
      Err ret = FtrPtrResize(creator, featureNum, newSize, newPtrP);
      *iret = ret;
      }
      break;

    case sysTrapFtrSet: {
      UInt32 creator = sys_va_arg(ap, UInt32);
      UInt16 featureNum = sys_va_arg(ap, UInt32);
      UInt32 newValue = sys_va_arg(ap, UInt32);
      Err ret = FtrSet(creator, featureNum, newValue);
      *iret = ret;
      }
      break;

    case sysTrapFtrUnregister: {
      UInt32 creator = sys_va_arg(ap, UInt32);
      UInt16 featureNum = sys_va_arg(ap, UInt32);
      Err ret = FtrUnregister(creator, featureNum);
      *iret = ret;
      }
      break;

    case sysTrapGrfAddMacro: {
      const Char *nameP = sys_va_arg(ap, Char *);
      UInt8 *macroDataP = sys_va_arg(ap, UInt8 *);
      UInt16 dataLen = sys_va_arg(ap, UInt32);
      Err ret = GrfAddMacro(nameP, macroDataP, dataLen);
      *iret = ret;
      }
      break;

    case sysTrapGrfAddPoint: {
      PointType *pt = sys_va_arg(ap, PointType *);
      Err ret = GrfAddPoint(pt);
      *iret = ret;
      }
      break;

    case sysTrapGrfBeginStroke: {
      const PointType *startPtP = sys_va_arg(ap, PointType *);
      const RectangleType *boundsP = sys_va_arg(ap, RectangleType *);
      Boolean liveInk = sys_va_arg(ap, UInt32);
      Err ret = GrfBeginStroke(startPtP, boundsP, liveInk);
      *iret = ret;
      }
      break;

    case sysTrapGrfCleanState: {
      Err ret = GrfCleanState();
      *iret = ret;
      }
      break;

    case sysTrapGrfDeleteMacro: {
      UInt16 index = sys_va_arg(ap, UInt32);
      Err ret = GrfDeleteMacro(index);
      *iret = ret;
      }
      break;

    case sysTrapGrfFieldChange: {
      Boolean resetState = sys_va_arg(ap, UInt32);
      UInt16 *characterToDelete = sys_va_arg(ap, UInt16 *);
      Err ret = GrfFieldChange(resetState, characterToDelete);
      *iret = ret;
      }
      break;

    case sysTrapGrfFilterPoints: {
      Err ret = GrfFilterPoints();
      *iret = ret;
      }
      break;

    case sysTrapGrfFindBranch: {
      UInt16 flags = sys_va_arg(ap, UInt32);
      Err ret = GrfFindBranch(flags);
      *iret = ret;
      }
      break;

    case sysTrapGrfFlushPoints: {
      Err ret = GrfFlushPoints();
      *iret = ret;
      }
      break;

    case sysTrapGrfFree: {
      Err ret = GrfFree();
      *iret = ret;
      }
      break;

    case sysTrapGrfGetAndExpandMacro: {
      Char *nameP = sys_va_arg(ap, Char *);
      UInt8 *macroDataP = sys_va_arg(ap, UInt8 *);
      UInt16 *dataLenP = sys_va_arg(ap, UInt16 *);
      Err ret = GrfGetAndExpandMacro(nameP, macroDataP, dataLenP);
      *iret = ret;
      }
      break;

    case sysTrapGrfGetGlyphMapping: {
      UInt16 glyphID = sys_va_arg(ap, UInt32);
      UInt16 *flagsP = sys_va_arg(ap, UInt16 *);
      void *dataPtrP = sys_va_arg(ap, void *);
      UInt16 *dataLenP = sys_va_arg(ap, UInt16 *);
      UInt16 *uncertainLenP = sys_va_arg(ap, UInt16 *);
      Err ret = GrfGetGlyphMapping(glyphID, flagsP, dataPtrP, dataLenP, uncertainLenP);
      *iret = ret;
      }
      break;

    case sysTrapGrfGetMacro: {
      Char *nameP = sys_va_arg(ap, Char *);
      UInt8 *macroDataP = sys_va_arg(ap, UInt8 *);
      UInt16 *dataLenP = sys_va_arg(ap, UInt16 *);
      Err ret = GrfGetMacro(nameP, macroDataP, dataLenP);
      *iret = ret;
      }
      break;

    case sysTrapGrfGetMacroName: {
      UInt16 index = sys_va_arg(ap, UInt32);
      Char *nameP = sys_va_arg(ap, Char *);
      Err ret = GrfGetMacroName(index, nameP);
      *iret = ret;
      }
      break;

    case sysTrapGrfGetNumPoints: {
      UInt16 *numPtsP = sys_va_arg(ap, UInt16 *);
      Err ret = GrfGetNumPoints(numPtsP);
      *iret = ret;
      }
      break;

    case sysTrapGrfGetPoint: {
      UInt16 index = sys_va_arg(ap, UInt32);
      PointType *pointP = sys_va_arg(ap, PointType *);
      Err ret = GrfGetPoint(index, pointP);
      *iret = ret;
      }
      break;

    case sysTrapGrfGetState: {
      Boolean *capsLockP = sys_va_arg(ap, Boolean *);
      Boolean *numLockP = sys_va_arg(ap, Boolean *);
      UInt16 *tempShiftP = sys_va_arg(ap, UInt16 *);
      Boolean *autoShiftedP = sys_va_arg(ap, Boolean *);
      Err ret = GrfGetState(capsLockP, numLockP, tempShiftP, autoShiftedP);
      *iret = ret;
      }
      break;

    case sysTrapGrfInit: {
      Err ret = GrfInit();
      *iret = ret;
      }
      break;

    case sysTrapGrfInitState: {
      Err ret = GrfInitState();
      *iret = ret;
      }
      break;

    case sysTrapGrfMatch: {
      UInt16 *flagsP = sys_va_arg(ap, UInt16 *);
      void *dataPtrP = sys_va_arg(ap, void *);
      UInt16 *dataLenP = sys_va_arg(ap, UInt16 *);
      UInt16 *uncertainLenP = sys_va_arg(ap, UInt16 *);
      GrfMatchInfoPtr matchInfoP = sys_va_arg(ap, GrfMatchInfoPtr);
      Err ret = GrfMatch(flagsP, dataPtrP, dataLenP, uncertainLenP, matchInfoP);
      *iret = ret;
      }
      break;

    case sysTrapGrfMatchGlyph: {
      GrfMatchInfoPtr matchInfoP = sys_va_arg(ap, GrfMatchInfoPtr);
      Int16 maxUnCertainty = sys_va_arg(ap, Int32);
      UInt16 maxMatches = sys_va_arg(ap, UInt32);
      Err ret = GrfMatchGlyph(matchInfoP, maxUnCertainty, maxMatches);
      *iret = ret;
      }
      break;

    case sysTrapGrfProcessStroke: {
      const PointType *startPtP = sys_va_arg(ap, PointType *);
      const PointType *endPtP = sys_va_arg(ap, PointType *);
      Boolean upShift = sys_va_arg(ap, UInt32);
      Err ret = GrfProcessStroke(startPtP, endPtP, upShift);
      *iret = ret;
      }
      break;

    case sysTrapGrfSetState: {
      Boolean capsLock = sys_va_arg(ap, UInt32);
      Boolean numLock = sys_va_arg(ap, UInt32);
      Boolean upperShift = sys_va_arg(ap, UInt32);
      Err ret = GrfSetState(capsLock, numLock, upperShift);
      *iret = ret;
      }
      break;

    case sysTrapGsiEnable: {
      const Boolean enableIt = sys_va_arg(ap, UInt32);
      GsiEnable(enableIt);
      }
      break;

    case sysTrapGsiEnabled: {
      Boolean ret = GsiEnabled();
      *iret = ret;
      }
      break;

    case sysTrapGsiInitialize: {
      GsiInitialize();
      }
      break;

    case sysTrapGsiSetLocation: {
      const Int16 x = sys_va_arg(ap, Int32);
      const Int16 y = sys_va_arg(ap, Int32);
      GsiSetLocation(x, y);
      }
      break;

    case sysTrapGsiSetShiftState: {
      const UInt16 lockFlags = sys_va_arg(ap, UInt32);
      const UInt16 tempShift = sys_va_arg(ap, UInt32);
      GsiSetShiftState(lockFlags, tempShift);
      }
      break;

    case sysTrapHwrGetROMToken: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      UInt32 token = sys_va_arg(ap, UInt32);
      UInt8 * *dataP = sys_va_arg(ap, UInt8 * *);
      UInt16 *sizeP = sys_va_arg(ap, UInt16 *);
      Err ret = SysGetROMToken(cardNo, token, dataP, sizeP);
      *iret = ret;
      }
      break;

    case sysTrapInsPtCheckBlink: {
      InsPtCheckBlink();
      }
      break;

    case sysTrapInsPtEnable: {
      Boolean enableIt = sys_va_arg(ap, UInt32);
      InsPtEnable(enableIt);
      }
      break;

    case sysTrapInsPtEnabled: {
      Boolean ret = InsPtEnabled();
      *iret = ret;
      }
      break;

    case sysTrapInsPtGetHeight: {
      Int16 ret = InsPtGetHeight();
      *iret = ret;
      }
      break;

    case sysTrapInsPtGetLocation: {
      Int16 *x = sys_va_arg(ap, Int16 *);
      Int16 *y = sys_va_arg(ap, Int16 *);
      InsPtGetLocation(x, y);
      }
      break;

    case sysTrapInsPtInitialize: {
      InsPtInitialize();
      }
      break;

    case sysTrapInsPtSetHeight: {
      const Int16 height = sys_va_arg(ap, Int32);
      InsPtSetHeight(height);
      }
      break;

    case sysTrapInsPtSetLocation: {
      const Int16 x = sys_va_arg(ap, Int32);
      const Int16 y = sys_va_arg(ap, Int32);
      InsPtSetLocation(x, y);
      }
      break;

    case sysTrapKbdDraw: {
      KeyboardStatus *ks = sys_va_arg(ap, KeyboardStatus *);
      Boolean keyTopsOnly = sys_va_arg(ap, UInt32);
      Boolean ignoreModifiers = sys_va_arg(ap, UInt32);
      KbdDraw(ks, keyTopsOnly, ignoreModifiers);
      }
      break;

    case sysTrapKbdErase: {
      KeyboardStatus *ks = sys_va_arg(ap, KeyboardStatus *);
      KbdErase(ks);
      }
      break;

    case sysTrapKbdGetLayout: {
      const KeyboardStatus *ks = sys_va_arg(ap, KeyboardStatus *);
      UInt16 ret = KbdGetLayout(ks);
      *iret = ret;
      }
      break;

    case sysTrapKbdGetPosition: {
      const KeyboardStatus *ks = sys_va_arg(ap, KeyboardStatus *);
      PointType *p = sys_va_arg(ap, PointType *);
      KbdGetPosition(ks, p);
      }
      break;

    case sysTrapKbdGetShiftState: {
      const KeyboardStatus *ks = sys_va_arg(ap, KeyboardStatus *);
      UInt16 ret = KbdGetShiftState(ks);
      *iret = ret;
      }
      break;

    case sysTrapKbdHandleEvent: {
      KeyboardStatus *ks = sys_va_arg(ap, KeyboardStatus *);
      EventType *pEvent = sys_va_arg(ap, EventType *);
      Boolean ret = KbdHandleEvent(ks, pEvent);
      *iret = ret;
      }
      break;

    case sysTrapKbdSetLayout: {
      KeyboardStatus *ks = sys_va_arg(ap, KeyboardStatus *);
      UInt16 layout = sys_va_arg(ap, UInt32);
      KbdSetLayout(ks, layout);
      }
      break;

    case sysTrapKbdSetPosition: {
      KeyboardStatus *ks = sys_va_arg(ap, KeyboardStatus *);
      const PointType *p = sys_va_arg(ap, PointType *);
      KbdSetPosition(ks, p);
      }
      break;

    case sysTrapKbdSetShiftState: {
      KeyboardStatus *ks = sys_va_arg(ap, KeyboardStatus *);
      UInt16 shiftState = sys_va_arg(ap, UInt32);
      KbdSetShiftState(ks, shiftState);
      }
      break;

    case sysTrapKeyCurrentState: {
      UInt32 ret = KeyCurrentState();
      *iret = ret;
      }
      break;

    case sysTrapKeyRates: {
      Boolean set = sys_va_arg(ap, UInt32);
      UInt16 *initDelayP = sys_va_arg(ap, UInt16 *);
      UInt16 *periodP = sys_va_arg(ap, UInt16 *);
      UInt16 *doubleTapDelayP = sys_va_arg(ap, UInt16 *);
      Boolean *queueAheadP = sys_va_arg(ap, Boolean *);
      Err ret = KeyRates(set, initDelayP, periodP, doubleTapDelayP, queueAheadP);
      *iret = ret;
      }
      break;

    case sysTrapKeySetMask: {
      UInt32 keyMask = sys_va_arg(ap, UInt32);
      UInt32 ret = KeySetMask(keyMask);
      *iret = ret;
      }
      break;

    case sysTrapKeyboardStatusFree: {
      KeyboardStatus *ks = sys_va_arg(ap, KeyboardStatus *);
      KeyboardStatusFree(ks);
      }
      break;

    case sysTrapKeyboardStatusNew: {
      UInt16 keyboardID = sys_va_arg(ap, UInt32);
      KeyboardStatus *ret = KeyboardStatusNew(keyboardID);
      *pret = (void *)ret;
      }
      break;

    case sysTrapLocGetNumberSeparators: {
      NumberFormatType numberFormat = sys_va_arg(ap, NumberFormatType);
      Char *thousandSeparator = sys_va_arg(ap, Char *);
      Char *decimalSeparator = sys_va_arg(ap, Char *);
      LocGetNumberSeparators(numberFormat, thousandSeparator, decimalSeparator);
      }
      break;

    case sysTrapLstDrawList: {
      ListType *listP = sys_va_arg(ap, ListType *);
      LstDrawList(listP);
      }
      break;

    case sysTrapLstEraseList: {
      ListType *listP = sys_va_arg(ap, ListType *);
      LstEraseList(listP);
      }
      break;

    case sysTrapLstGetNumberOfItems: {
      const ListType *listP = sys_va_arg(ap, ListType *);
      Int16 ret = LstGetNumberOfItems(listP);
      *iret = ret;
      }
      break;

    case sysTrapLstGetSelection: {
      const ListType *listP = sys_va_arg(ap, ListType *);
      Int16 ret = LstGetSelection(listP);
      *iret = ret;
      }
      break;

    case sysTrapLstGetSelectionText: {
      const ListType *listP = sys_va_arg(ap, ListType *);
      Int16 itemNum = sys_va_arg(ap, Int32);
      Char *ret = LstGetSelectionText(listP, itemNum);
      *pret = (void *)ret;
      }
      break;

    case sysTrapLstGetTopItem: {
      const ListType *listP = sys_va_arg(ap, ListType *);
      Int16 ret = LstGetTopItem(listP);
      *iret = ret;
      }
      break;

    case sysTrapLstGetVisibleItems: {
      const ListType *listP = sys_va_arg(ap, ListType *);
      Int16 ret = LstGetVisibleItems(listP);
      *iret = ret;
      }
      break;

    case sysTrapLstHandleEvent: {
      ListType *listP = sys_va_arg(ap, ListType *);
      const EventType *eventP = sys_va_arg(ap, EventType *);
      Boolean ret = LstHandleEvent(listP, eventP);
      *iret = ret;
      }
      break;

    case sysTrapLstMakeItemVisible: {
      ListType *listP = sys_va_arg(ap, ListType *);
      Int16 itemNum = sys_va_arg(ap, Int32);
      LstMakeItemVisible(listP, itemNum);
      }
      break;

    case sysTrapLstNewList: {
      void * *formPP = sys_va_arg(ap, void * *);
      UInt16 id = sys_va_arg(ap, UInt32);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      Coord width = sys_va_arg(ap, Int32);
      Coord height = sys_va_arg(ap, Int32);
      FontID font = sys_va_arg(ap, FontID);
      Int16 visibleItems = sys_va_arg(ap, Int32);
      Int16 triggerId = sys_va_arg(ap, Int32);
      Err ret = LstNewList(formPP, id, x, y, width, height, font, visibleItems, triggerId);
      *iret = ret;
      }
      break;

    case sysTrapLstPopupList: {
      ListType *listP = sys_va_arg(ap, ListType *);
      Int16 ret = LstPopupList(listP);
      *iret = ret;
      }
      break;

    case sysTrapLstScrollList: {
      ListType *listP = sys_va_arg(ap, ListType *);
      WinDirectionType direction = sys_va_arg(ap, WinDirectionType);
      Int16 itemCount = sys_va_arg(ap, Int32);
      Boolean ret = LstScrollList(listP, direction, itemCount);
      *iret = ret;
      }
      break;

    case sysTrapLstSetDrawFunction: {
      ListType *listP = sys_va_arg(ap, ListType *);
      ListDrawDataFuncPtr func = sys_va_arg(ap, ListDrawDataFuncPtr);
      LstSetDrawFunction(listP, func);
      }
      break;

    case sysTrapLstSetHeight: {
      ListType *listP = sys_va_arg(ap, ListType *);
      Int16 visibleItems = sys_va_arg(ap, Int32);
      LstSetHeight(listP, visibleItems);
      }
      break;

    case sysTrapLstSetListChoices: {
      ListType *listP = sys_va_arg(ap, ListType *);
      Char * *itemsText = sys_va_arg(ap, Char * *);
      Int16 numItems = sys_va_arg(ap, Int32);
      LstSetListChoices(listP, itemsText, numItems);
      }
      break;

    case sysTrapLstSetPosition: {
      ListType *listP = sys_va_arg(ap, ListType *);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      LstSetPosition(listP, x, y);
      }
      break;

    case sysTrapLstSetSelection: {
      ListType *listP = sys_va_arg(ap, ListType *);
      Int16 itemNum = sys_va_arg(ap, Int32);
      LstSetSelection(listP, itemNum);
      }
      break;

    case sysTrapLstSetTopItem: {
      ListType *listP = sys_va_arg(ap, ListType *);
      Int16 itemNum = sys_va_arg(ap, Int32);
      LstSetTopItem(listP, itemNum);
      }
      break;

    case sysTrapMdmDial: {
      MdmInfoPtr modemP = sys_va_arg(ap, MdmInfoPtr);
      Char *okDialP = sys_va_arg(ap, Char *);
      Char *userInitP = sys_va_arg(ap, Char *);
      Char *phoneNumP = sys_va_arg(ap, Char *);
      Err ret = MdmDial(modemP, okDialP, userInitP, phoneNumP);
      *iret = ret;
      }
      break;

    case sysTrapMdmHangUp: {
      MdmInfoPtr modemP = sys_va_arg(ap, MdmInfoPtr);
      Err ret = MdmHangUp(modemP);
      *iret = ret;
      }
      break;

    case sysTrapMemCardFormat: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      const Char *cardNameP = sys_va_arg(ap, Char *);
      const Char *manufNameP = sys_va_arg(ap, Char *);
      const Char *ramStoreNameP = sys_va_arg(ap, Char *);
      Err ret = MemCardFormat(cardNo, cardNameP, manufNameP, ramStoreNameP);
      *iret = ret;
      }
      break;

    case sysTrapMemCardInfo: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      Char *cardNameP = sys_va_arg(ap, Char *);
      Char *manufNameP = sys_va_arg(ap, Char *);
      UInt16 *versionP = sys_va_arg(ap, UInt16 *);
      UInt32 *crDateP = sys_va_arg(ap, UInt32 *);
      UInt32 *romSizeP = sys_va_arg(ap, UInt32 *);
      UInt32 *ramSizeP = sys_va_arg(ap, UInt32 *);
      UInt32 *freeBytesP = sys_va_arg(ap, UInt32 *);
      Err ret = MemCardInfo(cardNo, cardNameP, manufNameP, versionP, crDateP, romSizeP, ramSizeP, freeBytesP);
      *iret = ret;
      }
      break;

    case sysTrapMemChunkFree: {
      MemPtr chunkDataP = sys_va_arg(ap, MemPtr);
      Err ret = MemChunkFree(chunkDataP);
      *iret = ret;
      }
      break;

    case sysTrapMemChunkNew: {
      UInt16 heapID = sys_va_arg(ap, UInt32);
      UInt32 size = sys_va_arg(ap, UInt32);
      UInt16 attr = sys_va_arg(ap, UInt32);
      MemPtr ret = MemChunkNew(heapID, size, attr);
      *pret = (void *)ret;
      }
      break;

    case sysTrapMemCmp: {
      const void *s1 = sys_va_arg(ap, void *);
      const void *s2 = sys_va_arg(ap, void *);
      Int32 numBytes = sys_va_arg(ap, Int32);
      Int16 ret = MemCmp(s1, s2, numBytes);
      *iret = ret;
      }
      break;

    case sysTrapMemDebugMode: {
      UInt16 ret = MemDebugMode();
      *iret = ret;
      }
      break;

    case sysTrapMemHandleCardNo: {
      MemHandle h = sys_va_arg(ap, MemHandle);
      UInt16 ret = MemHandleCardNo(h);
      *iret = ret;
      }
      break;

    case sysTrapMemHandleDataStorage: {
      MemHandle h = sys_va_arg(ap, MemHandle);
      Boolean ret = MemHandleDataStorage(h);
      *iret = ret;
      }
      break;

    case sysTrapMemHandleFlags: {
      MemHandle h = sys_va_arg(ap, MemHandle);
      UInt16 ret = MemHandleFlags(h);
      *iret = ret;
      }
      break;

    case sysTrapMemHandleFree: {
      MemHandle h = sys_va_arg(ap, MemHandle);
      Err ret = MemHandleFree(h);
      *iret = ret;
      }
      break;

    case sysTrapMemHandleHeapID: {
      MemHandle h = sys_va_arg(ap, MemHandle);
      UInt16 ret = MemHandleHeapID(h);
      *iret = ret;
      }
      break;

    case sysTrapMemHandleLock: {
      MemHandle h = sys_va_arg(ap, MemHandle);
      MemPtr ret = MemHandleLock(h);
      *pret = (void *)ret;
      }
      break;

    case sysTrapMemHandleLockCount: {
      MemHandle h = sys_va_arg(ap, MemHandle);
      UInt16 ret = MemHandleLockCount(h);
      *iret = ret;
      }
      break;

    case sysTrapMemHandleNew: {
      UInt32 size = sys_va_arg(ap, UInt32);
      MemHandle ret = MemHandleNew(size);
      *pret = (void *)ret;
      }
      break;

    case sysTrapMemHandleOwner: {
      MemHandle h = sys_va_arg(ap, MemHandle);
      UInt16 ret = MemHandleOwner(h);
      *iret = ret;
      }
      break;

    case sysTrapMemHandleResetLock: {
      MemHandle h = sys_va_arg(ap, MemHandle);
      Err ret = MemHandleResetLock(h);
      *iret = ret;
      }
      break;

    case sysTrapMemHandleResize: {
      MemHandle h = sys_va_arg(ap, MemHandle);
      UInt32 newSize = sys_va_arg(ap, UInt32);
      Err ret = MemHandleResize(h, newSize);
      *iret = ret;
      }
      break;

    case sysTrapMemHandleSetOwner: {
      MemHandle h = sys_va_arg(ap, MemHandle);
      UInt16 owner = sys_va_arg(ap, UInt32);
      Err ret = MemHandleSetOwner(h, owner);
      *iret = ret;
      }
      break;

    case sysTrapMemHandleSize: {
      MemHandle h = sys_va_arg(ap, MemHandle);
      UInt32 ret = MemHandleSize(h);
      *iret = ret;
      }
      break;

    case sysTrapMemHandleToLocalID: {
      MemHandle h = sys_va_arg(ap, MemHandle);
      LocalID ret = MemHandleToLocalID(h);
      *iret = ret;
      }
      break;

    case sysTrapMemHandleUnlock: {
      MemHandle h = sys_va_arg(ap, MemHandle);
      Err ret = MemHandleUnlock(h);
      *iret = ret;
      }
      break;

    case sysTrapMemHeapCheck: {
      UInt16 heapID = sys_va_arg(ap, UInt32);
      Err ret = MemHeapCheck(heapID);
      *iret = ret;
      }
      break;

    case sysTrapMemHeapCompact: {
      UInt16 heapID = sys_va_arg(ap, UInt32);
      Err ret = MemHeapCompact(heapID);
      *iret = ret;
      }
      break;

    case sysTrapMemHeapDynamic: {
      UInt16 heapID = sys_va_arg(ap, UInt32);
      Boolean ret = MemHeapDynamic(heapID);
      *iret = ret;
      }
      break;

    case sysTrapMemHeapFlags: {
      UInt16 heapID = sys_va_arg(ap, UInt32);
      UInt16 ret = MemHeapFlags(heapID);
      *iret = ret;
      }
      break;

    case sysTrapMemHeapFreeByOwnerID: {
      UInt16 heapID = sys_va_arg(ap, UInt32);
      UInt16 ownerID = sys_va_arg(ap, UInt32);
      Err ret = MemHeapFreeByOwnerID(heapID, ownerID);
      *iret = ret;
      }
      break;

    case sysTrapMemHeapFreeBytes: {
      UInt16 heapID = sys_va_arg(ap, UInt32);
      UInt32 *freeP = sys_va_arg(ap, UInt32 *);
      UInt32 *maxP = sys_va_arg(ap, UInt32 *);
      Err ret = MemHeapFreeBytes(heapID, freeP, maxP);
      *iret = ret;
      }
      break;

    case sysTrapMemHeapID: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      UInt16 heapIndex = sys_va_arg(ap, UInt32);
      UInt16 ret = MemHeapID(cardNo, heapIndex);
      *iret = ret;
      }
      break;

    case sysTrapMemHeapInit: {
      UInt16 heapID = sys_va_arg(ap, UInt32);
      Int16 numHandles = sys_va_arg(ap, Int32);
      Boolean initContents = sys_va_arg(ap, UInt32);
      Err ret = MemHeapInit(heapID, numHandles, initContents);
      *iret = ret;
      }
      break;

    case sysTrapMemHeapScramble: {
      UInt16 heapID = sys_va_arg(ap, UInt32);
      Err ret = MemHeapScramble(heapID);
      *iret = ret;
      }
      break;

    case sysTrapMemHeapSize: {
      UInt16 heapID = sys_va_arg(ap, UInt32);
      UInt32 ret = MemHeapSize(heapID);
      *iret = ret;
      }
      break;

    case sysTrapMemInit: {
      Err ret = MemInit();
      *iret = ret;
      }
      break;

    case sysTrapMemInitHeapTable: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      Err ret = MemInitHeapTable(cardNo);
      *iret = ret;
      }
      break;

    case sysTrapMemKernelInit: {
      Err ret = MemKernelInit();
      *iret = ret;
      }
      break;

    case sysTrapMemLocalIDKind: {
      LocalID local = sys_va_arg(ap, LocalID);
      LocalIDKind ret = MemLocalIDKind(local);
      *iret = ret;
      }
      break;

    case sysTrapMemLocalIDToGlobal: {
      LocalID local = sys_va_arg(ap, LocalID);
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      MemPtr ret = MemLocalIDToGlobal(local, cardNo);
      *pret = (void *)ret;
      }
      break;

    case sysTrapMemLocalIDToLockedPtr: {
      LocalID local = sys_va_arg(ap, LocalID);
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      MemPtr ret = MemLocalIDToLockedPtr(local, cardNo);
      *pret = (void *)ret;
      }
      break;

    case sysTrapMemLocalIDToPtr: {
      LocalID local = sys_va_arg(ap, LocalID);
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      MemPtr ret = MemLocalIDToPtr(local, cardNo);
      *pret = (void *)ret;
      }
      break;

    case sysTrapMemMove: {
      void *dstP = sys_va_arg(ap, void *);
      const void *sP = sys_va_arg(ap, void *);
      Int32 numBytes = sys_va_arg(ap, Int32);
      Err ret = MemMove(dstP, sP, numBytes);
      *iret = ret;
      }
      break;

    case sysTrapMemNumCards: {
      UInt16 ret = MemNumCards();
      *iret = ret;
      }
      break;

    case sysTrapMemNumHeaps: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      UInt16 ret = MemNumHeaps(cardNo);
      *iret = ret;
      }
      break;

    case sysTrapMemNumRAMHeaps: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      UInt16 ret = MemNumRAMHeaps(cardNo);
      *iret = ret;
      }
      break;

    case sysTrapMemPtrCardNo: {
      MemPtr p = sys_va_arg(ap, MemPtr);
      UInt16 ret = MemPtrCardNo(p);
      *iret = ret;
      }
      break;

    case sysTrapMemPtrDataStorage: {
      MemPtr p = sys_va_arg(ap, MemPtr);
      Boolean ret = MemPtrDataStorage(p);
      *iret = ret;
      }
      break;

    case sysTrapMemPtrFlags: {
      MemPtr p = sys_va_arg(ap, MemPtr);
      UInt16 ret = MemPtrFlags(p);
      *iret = ret;
      }
      break;

    case sysTrapMemPtrHeapID: {
      MemPtr p = sys_va_arg(ap, MemPtr);
      UInt16 ret = MemPtrHeapID(p);
      *iret = ret;
      }
      break;

    case sysTrapMemPtrNew: {
      UInt32 size = sys_va_arg(ap, UInt32);
      MemPtr ret = MemPtrNew(size);
      *pret = (void *)ret;
      }
      break;

    case sysTrapMemPtrOwner: {
      MemPtr p = sys_va_arg(ap, MemPtr);
      UInt16 ret = MemPtrOwner(p);
      *iret = ret;
      }
      break;

    case sysTrapMemPtrRecoverHandle: {
      MemPtr p = sys_va_arg(ap, MemPtr);
      MemHandle ret = MemPtrRecoverHandle(p);
      *pret = (void *)ret;
      }
      break;

    case sysTrapMemPtrResetLock: {
      MemPtr p = sys_va_arg(ap, MemPtr);
      Err ret = MemPtrResetLock(p);
      *iret = ret;
      }
      break;

    case sysTrapMemPtrResize: {
      MemPtr p = sys_va_arg(ap, MemPtr);
      UInt32 newSize = sys_va_arg(ap, UInt32);
      Err ret = MemPtrResize(p, newSize);
      *iret = ret;
      }
      break;

    case sysTrapMemPtrSetOwner: {
      MemPtr p = sys_va_arg(ap, MemPtr);
      UInt16 owner = sys_va_arg(ap, UInt32);
      Err ret = MemPtrSetOwner(p, owner);
      *iret = ret;
      }
      break;

    case sysTrapMemPtrSize: {
      MemPtr p = sys_va_arg(ap, MemPtr);
      UInt32 ret = MemPtrSize(p);
      *iret = ret;
      }
      break;

    case sysTrapMemPtrToLocalID: {
      MemPtr p = sys_va_arg(ap, MemPtr);
      LocalID ret = MemPtrToLocalID(p);
      *iret = ret;
      }
      break;

    case sysTrapMemPtrUnlock: {
      MemPtr p = sys_va_arg(ap, MemPtr);
      Err ret = MemPtrUnlock(p);
      *iret = ret;
      }
      break;

    case sysTrapMemSemaphoreRelease: {
      Boolean writeAccess = sys_va_arg(ap, UInt32);
      Err ret = MemSemaphoreRelease(writeAccess);
      *iret = ret;
      }
      break;

    case sysTrapMemSemaphoreReserve: {
      Boolean writeAccess = sys_va_arg(ap, UInt32);
      Err ret = MemSemaphoreReserve(writeAccess);
      *iret = ret;
      }
      break;

    case sysTrapMemSet: {
      void *dstP = sys_va_arg(ap, void *);
      Int32 numBytes = sys_va_arg(ap, Int32);
      UInt8 value = sys_va_arg(ap, UInt32);
      Err ret = MemSet(dstP, numBytes, value);
      *iret = ret;
      }
      break;

    case sysTrapMemSetDebugMode: {
      UInt16 flags = sys_va_arg(ap, UInt32);
      Err ret = MemSetDebugMode(flags);
      *iret = ret;
      }
      break;

    case sysTrapMemStoreInfo: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      UInt16 storeNumber = sys_va_arg(ap, UInt32);
      UInt16 *versionP = sys_va_arg(ap, UInt16 *);
      UInt16 *flagsP = sys_va_arg(ap, UInt16 *);
      Char *nameP = sys_va_arg(ap, Char *);
      UInt32 *crDateP = sys_va_arg(ap, UInt32 *);
      UInt32 *bckUpDateP = sys_va_arg(ap, UInt32 *);
      UInt32 *heapListOffsetP = sys_va_arg(ap, UInt32 *);
      UInt32 *initCodeOffset1P = sys_va_arg(ap, UInt32 *);
      UInt32 *initCodeOffset2P = sys_va_arg(ap, UInt32 *);
      LocalID *databaseDirIDP = sys_va_arg(ap, LocalID *);
      Err ret = MemStoreInfo(cardNo, storeNumber, versionP, flagsP, nameP, crDateP, bckUpDateP, heapListOffsetP, initCodeOffset1P, initCodeOffset2P, databaseDirIDP);
      *iret = ret;
      }
      break;

    case sysTrapMemStoreSetInfo: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      UInt16 storeNumber = sys_va_arg(ap, UInt32);
      UInt16 *versionP = sys_va_arg(ap, UInt16 *);
      UInt16 *flagsP = sys_va_arg(ap, UInt16 *);
      Char *nameP = sys_va_arg(ap, Char *);
      UInt32 *crDateP = sys_va_arg(ap, UInt32 *);
      UInt32 *bckUpDateP = sys_va_arg(ap, UInt32 *);
      UInt32 *heapListOffsetP = sys_va_arg(ap, UInt32 *);
      UInt32 *initCodeOffset1P = sys_va_arg(ap, UInt32 *);
      UInt32 *initCodeOffset2P = sys_va_arg(ap, UInt32 *);
      LocalID *databaseDirIDP = sys_va_arg(ap, LocalID *);
      Err ret = MemStoreSetInfo(cardNo, storeNumber, versionP, flagsP, nameP, crDateP, bckUpDateP, heapListOffsetP, initCodeOffset1P, initCodeOffset2P, databaseDirIDP);
      *iret = ret;
      }
      break;

    case sysTrapMenuAddItem: {
      UInt16 positionId = sys_va_arg(ap, UInt32);
      UInt16 id = sys_va_arg(ap, UInt32);
      Char cmd = sys_va_arg(ap, Int32);
      const Char *textP = sys_va_arg(ap, Char *);
      Err ret = MenuAddItem(positionId, id, cmd, textP);
      *iret = ret;
      }
      break;

    case sysTrapMenuCmdBarAddButton: {
      UInt8 where = sys_va_arg(ap, UInt32);
      UInt16 bitmapId = sys_va_arg(ap, UInt32);
      MenuCmdBarResultType resultType = sys_va_arg(ap, MenuCmdBarResultType);
      UInt32 result = sys_va_arg(ap, UInt32);
      Char *nameP = sys_va_arg(ap, Char *);
      Err ret = MenuCmdBarAddButton(where, bitmapId, resultType, result, nameP);
      *iret = ret;
      }
      break;

    case sysTrapMenuCmdBarDisplay: {
      MenuCmdBarDisplay();
      }
      break;

    case sysTrapMenuCmdBarGetButtonData: {
      Int16 buttonIndex = sys_va_arg(ap, Int32);
      UInt16 *bitmapIdP = sys_va_arg(ap, UInt16 *);
      MenuCmdBarResultType *resultTypeP = sys_va_arg(ap, MenuCmdBarResultType *);
      UInt32 *resultP = sys_va_arg(ap, UInt32 *);
      Char *nameP = sys_va_arg(ap, Char *);
      Boolean ret = MenuCmdBarGetButtonData(buttonIndex, bitmapIdP, resultTypeP, resultP, nameP);
      *iret = ret;
      }
      break;

    case sysTrapMenuDispose: {
      MenuBarType *menuP = sys_va_arg(ap, MenuBarType *);
      MenuDispose(menuP);
      }
      break;

    case sysTrapMenuDrawMenu: {
      MenuBarType *menuP = sys_va_arg(ap, MenuBarType *);
      MenuDrawMenu(menuP);
      }
      break;

    case sysTrapMenuEraseStatus: {
      MenuBarType *menuP = sys_va_arg(ap, MenuBarType *);
      MenuEraseStatus(menuP);
      }
      break;

    case sysTrapMenuGetActiveMenu: {
      MenuBarType *ret = MenuGetActiveMenu();
      *pret = (void *)ret;
      }
      break;

    case sysTrapMenuHandleEvent: {
      MenuBarType *menuP = sys_va_arg(ap, MenuBarType *);
      EventType *event = sys_va_arg(ap, EventType *);
      UInt16 *error = sys_va_arg(ap, UInt16 *);
      Boolean ret = MenuHandleEvent(menuP, event, error);
      *iret = ret;
      }
      break;

    case sysTrapMenuHideItem: {
      UInt16 id = sys_va_arg(ap, UInt32);
      Boolean ret = MenuHideItem(id);
      *iret = ret;
      }
      break;

    case sysTrapMenuInit: {
      UInt16 resourceId = sys_va_arg(ap, UInt32);
      MenuBarType *ret = MenuInit(resourceId);
      *pret = (void *)ret;
      }
      break;

    case sysTrapMenuSetActiveMenu: {
      MenuBarType *menuP = sys_va_arg(ap, MenuBarType *);
      MenuBarType *ret = MenuSetActiveMenu(menuP);
      *pret = (void *)ret;
      }
      break;

    case sysTrapMenuSetActiveMenuRscID: {
      UInt16 resourceId = sys_va_arg(ap, UInt32);
      MenuSetActiveMenuRscID(resourceId);
      }
      break;

    case sysTrapMenuShowItem: {
      UInt16 id = sys_va_arg(ap, UInt32);
      Boolean ret = MenuShowItem(id);
      *iret = ret;
      }
      break;

    case sysTrapPceNativeCall: {
      NativeFuncType *nativeFuncP = sys_va_arg(ap, NativeFuncType *);
      void *userDataP = sys_va_arg(ap, void *);
      UInt32 ret = PceNativeCall(nativeFuncP, userDataP);
      *iret = ret;
      }
      break;

    case sysTrapPenCalibrate: {
      PointType *digTopLeftP = sys_va_arg(ap, PointType *);
      PointType *digBotRightP = sys_va_arg(ap, PointType *);
      PointType *scrTopLeftP = sys_va_arg(ap, PointType *);
      PointType *scrBotRightP = sys_va_arg(ap, PointType *);
      Err ret = PenCalibrate(digTopLeftP, digBotRightP, scrTopLeftP, scrBotRightP);
      *iret = ret;
      }
      break;

    case sysTrapPenClose: {
      Err ret = PenClose();
      *iret = ret;
      }
      break;

    case sysTrapPenGetRawPen: {
      PointType *penP = sys_va_arg(ap, PointType *);
      Err ret = PenGetRawPen(penP);
      *iret = ret;
      }
      break;

    case sysTrapPenOpen: {
      Err ret = PenOpen();
      *iret = ret;
      }
      break;

    case sysTrapPenRawToScreen: {
      PointType *penP = sys_va_arg(ap, PointType *);
      Err ret = PenRawToScreen(penP);
      *iret = ret;
      }
      break;

    case sysTrapPenResetCalibration: {
      Err ret = PenResetCalibration();
      *iret = ret;
      }
      break;

    case sysTrapPenScreenToRaw: {
      PointType *penP = sys_va_arg(ap, PointType *);
      Err ret = PenScreenToRaw(penP);
      *iret = ret;
      }
      break;

    case sysTrapPenSleep: {
      Err ret = PenSleep();
      *iret = ret;
      }
      break;

    case sysTrapPenWake: {
      Err ret = PenWake();
      *iret = ret;
      }
      break;

    case sysTrapPhoneNumberLookup: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      PhoneNumberLookup(fldP);
      }
      break;

    case sysTrapPhoneNumberLookupCustom: {
      FieldType *fldP = sys_va_arg(ap, FieldType *);
      AddrLookupParamsType *params = sys_va_arg(ap, AddrLookupParamsType *);
      Boolean useClipboard = sys_va_arg(ap, UInt32);
      PhoneNumberLookupCustom(fldP, params, useClipboard);
      }
      break;

    case sysTrapPrefGetAppPreferences: {
      UInt32 creator = sys_va_arg(ap, UInt32);
      UInt16 id = sys_va_arg(ap, UInt32);
      void *prefs = sys_va_arg(ap, void *);
      UInt16 *prefsSize = sys_va_arg(ap, UInt16 *);
      Boolean saved = sys_va_arg(ap, UInt32);
      Int16 ret = PrefGetAppPreferences(creator, id, prefs, prefsSize, saved);
      *iret = ret;
      }
      break;

    case sysTrapPrefGetAppPreferencesV10: {
      UInt32 type = sys_va_arg(ap, UInt32);
      Int16 version = sys_va_arg(ap, Int32);
      void *prefs = sys_va_arg(ap, void *);
      UInt16 prefsSize = sys_va_arg(ap, UInt32);
      Boolean ret = PrefGetAppPreferencesV10(type, version, prefs, prefsSize);
      *iret = ret;
      }
      break;

    case sysTrapPrefGetPreference: {
      SystemPreferencesChoice choice = sys_va_arg(ap, SystemPreferencesChoice);
      UInt32 ret = PrefGetPreference(choice);
      *iret = ret;
      }
      break;

    case sysTrapPrefGetPreferences: {
      SystemPreferencesPtr p = sys_va_arg(ap, SystemPreferencesPtr);
      PrefGetPreferences(p);
      }
      break;

    case sysTrapPrefOpenPreferenceDB: {
      Boolean saved = sys_va_arg(ap, UInt32);
      DmOpenRef ret = PrefOpenPreferenceDB(saved);
      *pret = (void *)ret;
      }
      break;

    case sysTrapPrefOpenPreferenceDBV10: {
      DmOpenRef ret = PrefOpenPreferenceDBV10();
      *pret = (void *)ret;
      }
      break;

    case sysTrapPrefSetAppPreferences: {
      UInt32 creator = sys_va_arg(ap, UInt32);
      UInt16 id = sys_va_arg(ap, UInt32);
      Int16 version = sys_va_arg(ap, Int32);
      const void *prefs = sys_va_arg(ap, void *);
      UInt16 prefsSize = sys_va_arg(ap, UInt32);
      Boolean saved = sys_va_arg(ap, UInt32);
      PrefSetAppPreferences(creator, id, version, prefs, prefsSize, saved);
      }
      break;

    case sysTrapPrefSetAppPreferencesV10: {
      UInt32 creator = sys_va_arg(ap, UInt32);
      Int16 version = sys_va_arg(ap, Int32);
      void *prefs = sys_va_arg(ap, void *);
      UInt16 prefsSize = sys_va_arg(ap, UInt32);
      PrefSetAppPreferencesV10(creator, version, prefs, prefsSize);
      }
      break;

    case sysTrapPrefSetPreference: {
      SystemPreferencesChoice choice = sys_va_arg(ap, SystemPreferencesChoice);
      UInt32 value = sys_va_arg(ap, UInt32);
      PrefSetPreference(choice, value);
      }
      break;

    case sysTrapPrefSetPreferences: {
      SystemPreferencesPtr p = sys_va_arg(ap, SystemPreferencesPtr);
      PrefSetPreferences(p);
      }
      break;

    case sysTrapPrgHandleEvent: {
      ProgressType *prgGP = sys_va_arg(ap, ProgressType *);
      EventType *eventP = sys_va_arg(ap, EventType *);
      Boolean ret = PrgHandleEvent(prgGP, eventP);
      *iret = ret;
      }
      break;

    case sysTrapPrgStartDialog: {
      const Char *title = sys_va_arg(ap, Char *);
      PrgCallbackFunc textCallback = sys_va_arg(ap, PrgCallbackFunc);
      void *userDataP = sys_va_arg(ap, void *);
      ProgressType *ret = PrgStartDialog(title, textCallback, userDataP);
      *pret = (void *)ret;
      }
      break;

    case sysTrapPrgStartDialogV31: {
      const Char *title = sys_va_arg(ap, Char *);
      PrgCallbackFunc textCallback = sys_va_arg(ap, PrgCallbackFunc);
      ProgressType *ret = PrgStartDialogV31(title, textCallback);
      *pret = (void *)ret;
      }
      break;

    case sysTrapPrgStopDialog: {
      ProgressType *prgP = sys_va_arg(ap, ProgressType *);
      Boolean force = sys_va_arg(ap, UInt32);
      PrgStopDialog(prgP, force);
      }
      break;

    case sysTrapPrgUpdateDialog: {
      ProgressType *prgGP = sys_va_arg(ap, ProgressType *);
      UInt16 err = sys_va_arg(ap, UInt32);
      UInt16 stage = sys_va_arg(ap, UInt32);
      const Char *messageP = sys_va_arg(ap, Char *);
      Boolean updateNow = sys_va_arg(ap, UInt32);
      PrgUpdateDialog(prgGP, err, stage, messageP, updateNow);
      }
      break;

    case sysTrapPwdExists: {
      Boolean ret = PwdExists();
      *iret = ret;
      }
      break;

    case sysTrapPwdRemove: {
      PwdRemove();
      }
      break;

    case sysTrapPwdSet: {
      Char *oldPassword = sys_va_arg(ap, Char *);
      Char *newPassword = sys_va_arg(ap, Char *);
      PwdSet(oldPassword, newPassword);
      }
      break;

    case sysTrapPwdVerify: {
      Char *string = sys_va_arg(ap, Char *);
      Boolean ret = PwdVerify(string);
      *iret = ret;
      }
      break;

    case sysTrapRctCopyRectangle: {
      const RectangleType *srcRectP = sys_va_arg(ap, RectangleType *);
      RectangleType *dstRectP = sys_va_arg(ap, RectangleType *);
      RctCopyRectangle(srcRectP, dstRectP);
      }
      break;

    case sysTrapRctGetIntersection: {
      const RectangleType *r1P = sys_va_arg(ap, RectangleType *);
      const RectangleType *r2P = sys_va_arg(ap, RectangleType *);
      RectangleType *r3P = sys_va_arg(ap, RectangleType *);
      RctGetIntersection(r1P, r2P, r3P);
      }
      break;

    case sysTrapRctInsetRectangle: {
      RectangleType *rP = sys_va_arg(ap, RectangleType *);
      Coord insetAmt = sys_va_arg(ap, Int32);
      RctInsetRectangle(rP, insetAmt);
      }
      break;

    case sysTrapRctOffsetRectangle: {
      RectangleType *rP = sys_va_arg(ap, RectangleType *);
      Coord deltaX = sys_va_arg(ap, Int32);
      Coord deltaY = sys_va_arg(ap, Int32);
      RctOffsetRectangle(rP, deltaX, deltaY);
      }
      break;

    case sysTrapRctPtInRectangle: {
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      Boolean ret = RctPtInRectangle(x, y, rP);
      *iret = ret;
      }
      break;

    case sysTrapRctSetRectangle: {
      RectangleType *rP = sys_va_arg(ap, RectangleType *);
      Coord left = sys_va_arg(ap, Int32);
      Coord top = sys_va_arg(ap, Int32);
      Coord width = sys_va_arg(ap, Int32);
      Coord height = sys_va_arg(ap, Int32);
      RctSetRectangle(rP, left, top, width, height);
      }
      break;

    case sysTrapResLoadConstant: {
      UInt16 rscID = sys_va_arg(ap, UInt32);
      UInt32 ret = ResLoadConstant(rscID);
      *iret = ret;
      }
      break;

    case sysTrapResLoadForm: {
      UInt16 rscID = sys_va_arg(ap, UInt32);
      void *ret = ResLoadForm(rscID);
      *pret = (void *)ret;
      }
      break;

    case sysTrapResLoadMenu: {
      UInt16 rscID = sys_va_arg(ap, UInt32);
      void *ret = ResLoadMenu(rscID);
      *pret = (void *)ret;
      }
      break;

    case sysTrapSclDrawScrollBar: {
      ScrollBarType *bar = sys_va_arg(ap, ScrollBarType *);
      SclDrawScrollBar(bar);
      }
      break;

    case sysTrapSclGetScrollBar: {
      const ScrollBarType *bar = sys_va_arg(ap, ScrollBarType *);
      Int16 *valueP = sys_va_arg(ap, Int16 *);
      Int16 *minP = sys_va_arg(ap, Int16 *);
      Int16 *maxP = sys_va_arg(ap, Int16 *);
      Int16 *pageSizeP = sys_va_arg(ap, Int16 *);
      SclGetScrollBar(bar, valueP, minP, maxP, pageSizeP);
      }
      break;

    case sysTrapSclHandleEvent: {
      ScrollBarType *bar = sys_va_arg(ap, ScrollBarType *);
      const EventType *event = sys_va_arg(ap, EventType *);
      Boolean ret = SclHandleEvent(bar, event);
      *iret = ret;
      }
      break;

    case sysTrapSclSetScrollBar: {
      ScrollBarType *bar = sys_va_arg(ap, ScrollBarType *);
      Int16 value = sys_va_arg(ap, Int32);
      Int16 min = sys_va_arg(ap, Int32);
      Int16 max = sys_va_arg(ap, Int32);
      Int16 pageSize = sys_va_arg(ap, Int32);
      SclSetScrollBar(bar, value, min, max, pageSize);
      }
      break;

    case sysTrapSecSelectViewStatus: {
      privateRecordViewEnum ret = SecSelectViewStatus();
      *iret = ret;
      }
      break;

    case sysTrapSecVerifyPW: {
      privateRecordViewEnum newSecLevel = sys_va_arg(ap, privateRecordViewEnum);
      Boolean ret = SecVerifyPW(newSecLevel);
      *iret = ret;
      }
      break;

    case sysTrapSelectDay: {
      const SelectDayType selectDayBy = sys_va_arg(ap, SelectDayType);
      Int16 *month = sys_va_arg(ap, Int16 *);
      Int16 *day = sys_va_arg(ap, Int16 *);
      Int16 *year = sys_va_arg(ap, Int16 *);
      const Char *title = sys_va_arg(ap, Char *);
      Boolean ret = SelectDay(selectDayBy, month, day, year, title);
      *iret = ret;
      }
      break;

    case sysTrapSelectDayV10: {
      Int16 *month = sys_va_arg(ap, Int16 *);
      Int16 *day = sys_va_arg(ap, Int16 *);
      Int16 *year = sys_va_arg(ap, Int16 *);
      const Char *title = sys_va_arg(ap, Char *);
      Boolean ret = SelectDayV10(month, day, year, title);
      *iret = ret;
      }
      break;

    case sysTrapSelectOneTime: {
      Int16 *hour = sys_va_arg(ap, Int16 *);
      Int16 *minute = sys_va_arg(ap, Int16 *);
      const Char *titleP = sys_va_arg(ap, Char *);
      Boolean ret = SelectOneTime(hour, minute, titleP);
      *iret = ret;
      }
      break;

    case sysTrapSelectTime: {
      TimeType *startTimeP = sys_va_arg(ap, TimeType *);
      TimeType *EndTimeP = sys_va_arg(ap, TimeType *);
      Boolean untimed = sys_va_arg(ap, UInt32);
      const Char *titleP = sys_va_arg(ap, Char *);
      Int16 startOfDay = sys_va_arg(ap, Int32);
      Int16 endOfDay = sys_va_arg(ap, Int32);
      Int16 startOfDisplay = sys_va_arg(ap, Int32);
      Boolean ret = SelectTime(startTimeP, EndTimeP, untimed, titleP, startOfDay, endOfDay, startOfDisplay);
      *iret = ret;
      }
      break;

    case sysTrapSelectTimeV33: {
      TimeType *startTimeP = sys_va_arg(ap, TimeType *);
      TimeType *EndTimeP = sys_va_arg(ap, TimeType *);
      Boolean untimed = sys_va_arg(ap, UInt32);
      const Char *titleP = sys_va_arg(ap, Char *);
      Int16 startOfDay = sys_va_arg(ap, Int32);
      Boolean ret = SelectTimeV33(startTimeP, EndTimeP, untimed, titleP, startOfDay);
      *iret = ret;
      }
      break;

    case sysTrapSelectTimeZone: {
      Int16 *ioTimeZoneP = sys_va_arg(ap, Int16 *);
      LmLocaleType *ioLocaleInTimeZoneP = sys_va_arg(ap, LmLocaleType *);
      const Char *titleP = sys_va_arg(ap, Char *);
      Boolean showTimes = sys_va_arg(ap, UInt32);
      Boolean anyLocale = sys_va_arg(ap, UInt32);
      Boolean ret = SelectTimeZone(ioTimeZoneP, ioLocaleInTimeZoneP, titleP, showTimes, anyLocale);
      *iret = ret;
      }
      break;

    case sysTrapSerReceiveISP: {
      Boolean ret = SerReceiveISP();
      *iret = ret;
      }
      break;

    case sysTrapSlkClose: {
      Err ret = SlkClose();
      *iret = ret;
      }
      break;

    case sysTrapSlkCloseSocket: {
      UInt16 socket = sys_va_arg(ap, UInt32);
      Err ret = SlkCloseSocket(socket);
      *iret = ret;
      }
      break;

    case sysTrapSlkFlushSocket: {
      UInt16 socket = sys_va_arg(ap, UInt32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err ret = SlkFlushSocket(socket, timeout);
      *iret = ret;
      }
      break;

    case sysTrapSlkOpen: {
      Err ret = SlkOpen();
      *iret = ret;
      }
      break;

    case sysTrapSlkOpenSocket: {
      UInt16 portID = sys_va_arg(ap, UInt32);
      UInt16 *socketP = sys_va_arg(ap, UInt16 *);
      Boolean staticSocket = sys_va_arg(ap, UInt32);
      Err ret = SlkOpenSocket(portID, socketP, staticSocket);
      *iret = ret;
      }
      break;

    case sysTrapSlkProcessRPC: {
      SlkPktHeaderPtr headerP = sys_va_arg(ap, SlkPktHeaderPtr);
      void *bodyP = sys_va_arg(ap, void *);
      Err ret = SlkProcessRPC(headerP, bodyP);
      *iret = ret;
      }
      break;

    case sysTrapSlkReceivePacket: {
      UInt16 socket = sys_va_arg(ap, UInt32);
      Boolean andOtherSockets = sys_va_arg(ap, UInt32);
      SlkPktHeaderPtr headerP = sys_va_arg(ap, SlkPktHeaderPtr);
      void *bodyP = sys_va_arg(ap, void *);
      UInt16 bodySize = sys_va_arg(ap, UInt32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err ret = SlkReceivePacket(socket, andOtherSockets, headerP, bodyP, bodySize, timeout);
      *iret = ret;
      }
      break;

    case sysTrapSlkSendPacket: {
      SlkPktHeaderPtr headerP = sys_va_arg(ap, SlkPktHeaderPtr);
      SlkWriteDataPtr writeList = sys_va_arg(ap, SlkWriteDataPtr);
      Err ret = SlkSendPacket(headerP, writeList);
      *iret = ret;
      }
      break;

    case sysTrapSlkSetSocketListener: {
      UInt16 socket = sys_va_arg(ap, UInt32);
      SlkSocketListenPtr socketP = sys_va_arg(ap, SlkSocketListenPtr);
      Err ret = SlkSetSocketListener(socket, socketP);
      *iret = ret;
      }
      break;

    case sysTrapSlkSocketRefNum: {
      UInt16 socket = sys_va_arg(ap, UInt32);
      UInt16 *portIDP = sys_va_arg(ap, UInt16 *);
      Err ret = SlkSocketPortID(socket, portIDP);
      *iret = ret;
      }
      break;

    case sysTrapSlkSocketSetTimeout: {
      UInt16 socket = sys_va_arg(ap, UInt32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err ret = SlkSocketSetTimeout(socket, timeout);
      *iret = ret;
      }
      break;

    case sysTrapSlkSysPktDefaultResponse: {
      SlkPktHeaderPtr headerP = sys_va_arg(ap, SlkPktHeaderPtr);
      void *bodyP = sys_va_arg(ap, void *);
      Err ret = SlkSysPktDefaultResponse(headerP, bodyP);
      *iret = ret;
      }
      break;

    case sysTrapSndCreateMidiList: {
      UInt32 creator = sys_va_arg(ap, UInt32);
      Boolean multipleDBs = sys_va_arg(ap, UInt32);
      UInt16 *wCountP = sys_va_arg(ap, UInt16 *);
      MemHandle *entHP = sys_va_arg(ap, MemHandle *);
      Boolean ret = SndCreateMidiList(creator, multipleDBs, wCountP, entHP);
      *iret = ret;
      }
      break;

    case sysTrapSndDoCmd: {
      void *channelP = sys_va_arg(ap, void *);
      SndCommandPtr cmdP = sys_va_arg(ap, SndCommandPtr);
      Boolean noWait = sys_va_arg(ap, UInt32);
      Err ret = SndDoCmd(channelP, cmdP, noWait);
      *iret = ret;
      }
      break;

    case sysTrapSndGetDefaultVolume: {
      UInt16 *alarmAmpP = sys_va_arg(ap, UInt16 *);
      UInt16 *sysAmpP = sys_va_arg(ap, UInt16 *);
      UInt16 *masterAmpP = sys_va_arg(ap, UInt16 *);
      SndGetDefaultVolume(alarmAmpP, sysAmpP, masterAmpP);
      }
      break;

    case sysTrapSndInit: {
      Err ret = SndInit();
      *iret = ret;
      }
      break;

    case sysTrapSndInterruptSmfIrregardless: {
      Err ret = SndInterruptSmfIrregardless();
      *iret = ret;
      }
      break;

    case sysTrapSndPlayResource: {
      SndPtr sndP = sys_va_arg(ap, SndPtr);
      Int32 volume = sys_va_arg(ap, Int32);
      UInt32 flags = sys_va_arg(ap, UInt32);
      Err ret = SndPlayResource(sndP, volume, flags);
      *iret = ret;
      }
      break;

    case sysTrapSndPlaySmf: {
      void *chanP = sys_va_arg(ap, void *);
      SndSmfCmdEnum cmd = sys_va_arg(ap, UInt32);
      UInt8 *smfP = sys_va_arg(ap, UInt8 *);
      SndSmfOptionsType *selP = sys_va_arg(ap, SndSmfOptionsType *);
      SndSmfChanRangeType *chanRangeP = sys_va_arg(ap, SndSmfChanRangeType *);
      SndSmfCallbacksType *callbacksP = sys_va_arg(ap, SndSmfCallbacksType *);
      Boolean bNoWait = sys_va_arg(ap, UInt32);
      Err ret = SndPlaySmf(chanP, cmd, smfP, selP, chanRangeP, callbacksP, bNoWait);
      *iret = ret;
      }
      break;

    case sysTrapSndPlaySmfIrregardless: {
      void *chanP = sys_va_arg(ap, void *);
      SndSmfCmdEnum cmd = sys_va_arg(ap, UInt32);
      UInt8 *smfP = sys_va_arg(ap, UInt8 *);
      SndSmfOptionsType *selP = sys_va_arg(ap, SndSmfOptionsType *);
      SndSmfChanRangeType *chanRangeP = sys_va_arg(ap, SndSmfChanRangeType *);
      SndSmfCallbacksType *callbacksP = sys_va_arg(ap, SndSmfCallbacksType *);
      Boolean bNoWait = sys_va_arg(ap, UInt32);
      Err ret = SndPlaySmfIrregardless(chanP, cmd, smfP, selP, chanRangeP, callbacksP, bNoWait);
      *iret = ret;
      }
      break;

    case sysTrapSndPlaySmfResource: {
      UInt32 resType = sys_va_arg(ap, UInt32);
      Int16 resID = sys_va_arg(ap, Int32);
      SystemPreferencesChoice volumeSelector = sys_va_arg(ap, SystemPreferencesChoice);
      Err ret = SndPlaySmfResource(resType, resID, volumeSelector);
      *iret = ret;
      }
      break;

    case sysTrapSndPlaySmfResourceIrregardless: {
      UInt32 resType = sys_va_arg(ap, UInt32);
      Int16 resID = sys_va_arg(ap, Int32);
      SystemPreferencesChoice volumeSelector = sys_va_arg(ap, SystemPreferencesChoice);
      Err ret = SndPlaySmfResourceIrregardless(resType, resID, volumeSelector);
      *iret = ret;
      }
      break;

    case sysTrapSndPlaySystemSound: {
      SndSysBeepType beepID = sys_va_arg(ap, UInt32);
      SndPlaySystemSound(beepID);
      }
      break;

    case sysTrapSndSetDefaultVolume: {
      UInt16 *alarmAmpP = sys_va_arg(ap, UInt16 *);
      UInt16 *sysAmpP = sys_va_arg(ap, UInt16 *);
      UInt16 *defAmpP = sys_va_arg(ap, UInt16 *);
      SndSetDefaultVolume(alarmAmpP, sysAmpP, defAmpP);
      }
      break;

    case sysTrapSndStreamCreate: {
      SndStreamRef *channel = sys_va_arg(ap, SndStreamRef *);
      SndStreamMode mode = sys_va_arg(ap, Int32);
      UInt32 samplerate = sys_va_arg(ap, UInt32);
      SndSampleType type = sys_va_arg(ap, Int32);
      SndStreamWidth width = sys_va_arg(ap, Int32);
      SndStreamBufferCallback func = sys_va_arg(ap, SndStreamBufferCallback);
      void *userdata = sys_va_arg(ap, void *);
      UInt32 buffsize = sys_va_arg(ap, UInt32);
      Boolean armNative = sys_va_arg(ap, UInt32);
      Err ret = SndStreamCreate(channel, mode, samplerate, type, width, func, userdata, buffsize, armNative);
      *iret = ret;
      }
      break;

    case sysTrapSndStreamCreateExtended: {
      SndStreamRef *channel = sys_va_arg(ap, SndStreamRef *);
      SndStreamMode mode = sys_va_arg(ap, Int32);
      SndFormatType format = sys_va_arg(ap, SndFormatType);
      UInt32 samplerate = sys_va_arg(ap, UInt32);
      SndSampleType type = sys_va_arg(ap, Int32);
      SndStreamWidth width = sys_va_arg(ap, Int32);
      SndStreamVariableBufferCallback func = sys_va_arg(ap, SndStreamVariableBufferCallback);
      void *userdata = sys_va_arg(ap, void *);
      UInt32 buffsize = sys_va_arg(ap, UInt32);
      Boolean armNative = sys_va_arg(ap, UInt32);
      Err ret = SndStreamCreateExtended(channel, mode, format, samplerate, type, width, func, userdata, buffsize, armNative);
      *iret = ret;
      }
      break;

    case sysTrapSndStreamDelete: {
      SndStreamRef channel = sys_va_arg(ap, SndStreamRef);
      Err ret = SndStreamDelete(channel);
      *iret = ret;
      }
      break;

    case sysTrapSndStreamDeviceControl: {
      SndStreamRef channel = sys_va_arg(ap, SndStreamRef);
      Int32 cmd = sys_va_arg(ap, Int32);
      void *param = sys_va_arg(ap, void *);
      Int32 size = sys_va_arg(ap, Int32);
      Err ret = SndStreamDeviceControl(channel, cmd, param, size);
      *iret = ret;
      }
      break;

    case sysTrapSndStreamGetPan: {
      SndStreamRef channel = sys_va_arg(ap, SndStreamRef);
      Int32 *panposition = sys_va_arg(ap, Int32 *);
      Err ret = SndStreamGetPan(channel, panposition);
      *iret = ret;
      }
      break;

    case sysTrapSndStreamGetVolume: {
      SndStreamRef channel = sys_va_arg(ap, SndStreamRef);
      Int32 *volume = sys_va_arg(ap, Int32 *);
      Err ret = SndStreamGetVolume(channel, volume);
      *iret = ret;
      }
      break;

    case sysTrapSndStreamPause: {
      SndStreamRef channel = sys_va_arg(ap, SndStreamRef);
      Boolean pause = sys_va_arg(ap, UInt32);
      Err ret = SndStreamPause(channel, pause);
      *iret = ret;
      }
      break;

    case sysTrapSndStreamSetPan: {
      SndStreamRef channel = sys_va_arg(ap, SndStreamRef);
      Int32 panposition = sys_va_arg(ap, Int32);
      Err ret = SndStreamSetPan(channel, panposition);
      *iret = ret;
      }
      break;

    case sysTrapSndStreamSetVolume: {
      SndStreamRef channel = sys_va_arg(ap, SndStreamRef);
      Int32 volume = sys_va_arg(ap, Int32);
      Err ret = SndStreamSetVolume(channel, volume);
      *iret = ret;
      }
      break;

    case sysTrapSndStreamStart: {
      SndStreamRef channel = sys_va_arg(ap, SndStreamRef);
      Err ret = SndStreamStart(channel);
      *iret = ret;
      }
      break;

    case sysTrapSndStreamStop: {
      SndStreamRef channel = sys_va_arg(ap, SndStreamRef);
      Err ret = SndStreamStop(channel);
      *iret = ret;
      }
      break;

    case sysTrapStrAToI: {
      const Char *str = sys_va_arg(ap, Char *);
      Int32 ret = StrAToI(str);
      *iret = ret;
      }
      break;

    case sysTrapStrCaselessCompare: {
      const Char *s1 = sys_va_arg(ap, Char *);
      const Char *s2 = sys_va_arg(ap, Char *);
      Int16 ret = StrCaselessCompare(s1, s2);
      *iret = ret;
      }
      break;

    case sysTrapStrCat: {
      Char *dst = sys_va_arg(ap, Char *);
      const Char *src = sys_va_arg(ap, Char *);
      Char *ret = StrCat(dst, src);
      *pret = (void *)ret;
      }
      break;

    case sysTrapStrChr: {
      const Char *str = sys_va_arg(ap, Char *);
      WChar chr = sys_va_arg(ap, UInt32);
      Char *ret = StrChr(str, chr);
      *pret = (void *)ret;
      }
      break;

    case sysTrapStrCompare: {
      const Char *s1 = sys_va_arg(ap, Char *);
      const Char *s2 = sys_va_arg(ap, Char *);
      Int16 ret = StrCompare(s1, s2);
      *iret = ret;
      }
      break;

    case sysTrapStrCompareAscii: {
      const Char *s1 = sys_va_arg(ap, Char *);
      const Char *s2 = sys_va_arg(ap, Char *);
      Int16 ret = StrCompareAscii(s1, s2);
      *iret = ret;
      }
      break;

    case sysTrapStrCopy: {
      Char *dst = sys_va_arg(ap, Char *);
      const Char *src = sys_va_arg(ap, Char *);
      Char *ret = StrCopy(dst, src);
      *pret = (void *)ret;
      }
      break;

    case sysTrapStrDelocalizeNumber: {
      Char *s = sys_va_arg(ap, Char *);
      Char thousandSeparator = sys_va_arg(ap, Int32);
      Char decimalSeparator = sys_va_arg(ap, Int32);
      Char *ret = StrDelocalizeNumber(s, thousandSeparator, decimalSeparator);
      *pret = (void *)ret;
      }
      break;

    case sysTrapStrIToA: {
      Char *s = sys_va_arg(ap, Char *);
      Int32 i = sys_va_arg(ap, Int32);
      Char *ret = StrIToA(s, i);
      *pret = (void *)ret;
      }
      break;

    case sysTrapStrIToH: {
      Char *s = sys_va_arg(ap, Char *);
      UInt32 i = sys_va_arg(ap, UInt32);
      Char *ret = StrIToH(s, i);
      *pret = (void *)ret;
      }
      break;

    case sysTrapStrLen: {
      const Char *src = sys_va_arg(ap, Char *);
      UInt16 ret = StrLen(src);
      *iret = ret;
      }
      break;

    case sysTrapStrLocalizeNumber: {
      Char *s = sys_va_arg(ap, Char *);
      Char thousandSeparator = sys_va_arg(ap, Int32);
      Char decimalSeparator = sys_va_arg(ap, Int32);
      Char *ret = StrLocalizeNumber(s, thousandSeparator, decimalSeparator);
      *pret = (void *)ret;
      }
      break;

    case sysTrapStrNCaselessCompare: {
      const Char *s1 = sys_va_arg(ap, Char *);
      const Char *s2 = sys_va_arg(ap, Char *);
      Int32 n = sys_va_arg(ap, Int32);
      Int16 ret = StrNCaselessCompare(s1, s2, n);
      *iret = ret;
      }
      break;

    case sysTrapStrNCat: {
      Char *dst = sys_va_arg(ap, Char *);
      const Char *src = sys_va_arg(ap, Char *);
      Int16 n = sys_va_arg(ap, Int32);
      Char *ret = StrNCat(dst, src, n);
      *pret = (void *)ret;
      }
      break;

    case sysTrapStrNCompare: {
      const Char *s1 = sys_va_arg(ap, Char *);
      const Char *s2 = sys_va_arg(ap, Char *);
      Int32 n = sys_va_arg(ap, Int32);
      Int16 ret = StrNCompare(s1, s2, n);
      *iret = ret;
      }
      break;

    case sysTrapStrNCompareAscii: {
      const Char *s1 = sys_va_arg(ap, Char *);
      const Char *s2 = sys_va_arg(ap, Char *);
      Int32 n = sys_va_arg(ap, Int32);
      Int16 ret = StrNCompareAscii(s1, s2, n);
      *iret = ret;
      }
      break;

    case sysTrapStrNCopy: {
      Char *dst = sys_va_arg(ap, Char *);
      const Char *src = sys_va_arg(ap, Char *);
      Int16 n = sys_va_arg(ap, Int32);
      Char *ret = StrNCopy(dst, src, n);
      *pret = (void *)ret;
      }
      break;

    case sysTrapStrPrintF: {
      Char *s = sys_va_arg(ap, Char *);
      const Char *formatStr = sys_va_arg(ap, Char *);
      Int16 ret = StrPrintF(s, formatStr, ap);
      *iret = ret;
      }
      break;

    case sysTrapStrStr: {
      const Char *str = sys_va_arg(ap, Char *);
      const Char *token = sys_va_arg(ap, Char *);
      Char *ret = StrStr(str, token);
      *pret = (void *)ret;
      }
      break;

    case sysTrapStrToLower: {
      Char *dst = sys_va_arg(ap, Char *);
      const Char *src = sys_va_arg(ap, Char *);
      Char *ret = StrToLower(dst, src);
      *pret = (void *)ret;
      }
      break;

    case sysTrapStrVPrintF: {
      Char *s = sys_va_arg(ap, Char *);
      const Char *formatStr = sys_va_arg(ap, Char *);
      Int16 ret = StrVPrintF(s, formatStr, ap);
      *iret = ret;
      }
      break;

    case sysTrapSysAppExit: {
      SysAppInfoPtr appInfoP = sys_va_arg(ap, SysAppInfoPtr);
      MemPtr prevGlobalsP = sys_va_arg(ap, MemPtr);
      MemPtr globalsP = sys_va_arg(ap, MemPtr);
      Err ret = SysAppExit(appInfoP, prevGlobalsP, globalsP);
      *iret = ret;
      }
      break;

    case sysTrapSysAppLaunch: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      UInt16 launchFlags = sys_va_arg(ap, UInt32);
      UInt16 cmd = sys_va_arg(ap, UInt32);
      MemPtr cmdPBP = sys_va_arg(ap, MemPtr);
      UInt32 *resultP = sys_va_arg(ap, UInt32 *);
      Err ret = SysAppLaunch(cardNo, dbID, launchFlags, cmd, cmdPBP, resultP);
      *iret = ret;
      }
      break;

    case sysTrapSysAppLauncherDialog: {
      SysAppLauncherDialog();
      }
      break;

    case sysTrapSysAppStartup: {
      SysAppInfoPtr *appInfoPP = sys_va_arg(ap, SysAppInfoPtr *);
      MemPtr *prevGlobalsP = sys_va_arg(ap, MemPtr *);
      MemPtr *globalsPtrP = sys_va_arg(ap, MemPtr *);
      Err ret = SysAppStartup(appInfoPP, prevGlobalsP, globalsPtrP);
      *iret = ret;
      }
      break;

    case sysTrapSysBatteryDialog: {
      SysBatteryDialog();
      }
      break;

    case sysTrapSysBatteryInfo: {
      Boolean set = sys_va_arg(ap, UInt32);
      UInt16 *warnThresholdP = sys_va_arg(ap, UInt16 *);
      UInt16 *criticalThresholdP = sys_va_arg(ap, UInt16 *);
      Int16 *maxTicksP = sys_va_arg(ap, Int16 *);
      SysBatteryKind *kindP = sys_va_arg(ap, SysBatteryKind *);
      Boolean *pluggedIn = sys_va_arg(ap, Boolean *);
      UInt8 *percentP = sys_va_arg(ap, UInt8 *);
      UInt16 ret = SysBatteryInfo(set, warnThresholdP, criticalThresholdP, maxTicksP, kindP, pluggedIn, percentP);
      *iret = ret;
      }
      break;

    case sysTrapSysBatteryInfoV20: {
      Boolean set = sys_va_arg(ap, UInt32);
      UInt16 *warnThresholdP = sys_va_arg(ap, UInt16 *);
      UInt16 *criticalThresholdP = sys_va_arg(ap, UInt16 *);
      Int16 *maxTicksP = sys_va_arg(ap, Int16 *);
      SysBatteryKind *kindP = sys_va_arg(ap, SysBatteryKind *);
      Boolean *pluggedIn = sys_va_arg(ap, Boolean *);
      UInt16 ret = SysBatteryInfoV20(set, warnThresholdP, criticalThresholdP, maxTicksP, kindP, pluggedIn);
      *iret = ret;
      }
      break;

    case sysTrapSysBinarySearch: {
      void const *baseP = sys_va_arg(ap, void const *);
      UInt16 numOfElements = sys_va_arg(ap, UInt32);
      Int16 width = sys_va_arg(ap, Int32);
      SearchFuncPtr searchF = sys_va_arg(ap, SearchFuncPtr);
      void const *searchData = sys_va_arg(ap, void const *);
      Int32 other = sys_va_arg(ap, Int32);
      Int32 *position = sys_va_arg(ap, Int32 *);
      Boolean findFirst = sys_va_arg(ap, UInt32);
      Boolean ret = SysBinarySearch(baseP, numOfElements, width, searchF, searchData, other, position, findFirst);
      *iret = ret;
      }
      break;

    case sysTrapSysBroadcastActionCode: {
      UInt16 cmd = sys_va_arg(ap, UInt32);
      MemPtr cmdPBP = sys_va_arg(ap, MemPtr);
      Err ret = SysBroadcastActionCode(cmd, cmdPBP);
      *iret = ret;
      }
      break;

    case sysTrapSysColdBoot: {
      void *card0P = sys_va_arg(ap, void *);
      UInt32 card0Size = sys_va_arg(ap, UInt32);
      void *card1P = sys_va_arg(ap, void *);
      UInt32 card1Size = sys_va_arg(ap, UInt32);
      UInt32 sysCardHeaderOffset = sys_va_arg(ap, UInt32);
      SysColdBoot(card0P, card0Size, card1P, card1Size, sysCardHeaderOffset);
      }
      break;

    case sysTrapSysCopyStringResource: {
      Char *string = sys_va_arg(ap, Char *);
      Int16 theID = sys_va_arg(ap, Int32);
      SysCopyStringResource(string, theID);
      }
      break;

    case sysTrapSysCreateDataBaseList: {
      UInt32 type = sys_va_arg(ap, UInt32);
      UInt32 creator = sys_va_arg(ap, UInt32);
      UInt16 *dbCount = sys_va_arg(ap, UInt16 *);
      MemHandle *dbIDs = sys_va_arg(ap, MemHandle *);
      Boolean lookupName = sys_va_arg(ap, UInt32);
      Boolean ret = SysCreateDataBaseList(type, creator, dbCount, dbIDs, lookupName);
      *iret = ret;
      }
      break;

    case sysTrapSysCreatePanelList: {
      UInt16 *panelCount = sys_va_arg(ap, UInt16 *);
      MemHandle *panelIDs = sys_va_arg(ap, MemHandle *);
      Boolean ret = SysCreatePanelList(panelCount, panelIDs);
      *iret = ret;
      }
      break;

    case sysTrapSysCurAppDatabase: {
      UInt16 *cardNoP = sys_va_arg(ap, UInt16 *);
      LocalID *dbIDP = sys_va_arg(ap, LocalID *);
      Err ret = SysCurAppDatabase(cardNoP, dbIDP);
      *iret = ret;
      }
      break;

    case sysTrapSysDisableInts: {
      UInt16 ret = SysDisableInts();
      *iret = ret;
      }
      break;

    case sysTrapSysDoze: {
      Boolean onlyNMI = sys_va_arg(ap, UInt32);
      SysDoze(onlyNMI);
      }
      break;

    case sysTrapSysErrString: {
      Err err = sys_va_arg(ap, UInt32);
      Char *strP = sys_va_arg(ap, Char *);
      UInt16 maxLen = sys_va_arg(ap, UInt32);
      Char *ret = SysErrString(err, strP, maxLen);
      *pret = (void *)ret;
      }
      break;

    case sysTrapSysEvGroupCreate: {
      UInt32 *evIDP = sys_va_arg(ap, UInt32 *);
      UInt32 *tagP = sys_va_arg(ap, UInt32 *);
      UInt32 init = sys_va_arg(ap, UInt32);
      Err ret = SysEvGroupCreate(evIDP, tagP, init);
      *iret = ret;
      }
      break;

    case sysTrapSysEvGroupRead: {
      UInt32 evID = sys_va_arg(ap, UInt32);
      UInt32 *valueP = sys_va_arg(ap, UInt32 *);
      Err ret = SysEvGroupRead(evID, valueP);
      *iret = ret;
      }
      break;

    case sysTrapSysEvGroupSignal: {
      UInt32 evID = sys_va_arg(ap, UInt32);
      UInt32 mask = sys_va_arg(ap, UInt32);
      UInt32 value = sys_va_arg(ap, UInt32);
      Int32 type = sys_va_arg(ap, Int32);
      Err ret = SysEvGroupSignal(evID, mask, value, type);
      *iret = ret;
      }
      break;

    case sysTrapSysEvGroupWait: {
      UInt32 evID = sys_va_arg(ap, UInt32);
      UInt32 mask = sys_va_arg(ap, UInt32);
      UInt32 value = sys_va_arg(ap, UInt32);
      Int32 matchType = sys_va_arg(ap, Int32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err ret = SysEvGroupWait(evID, mask, value, matchType, timeout);
      *iret = ret;
      }
      break;

    case sysTrapSysFatalAlert: {
      const Char *msg = sys_va_arg(ap, Char *);
      UInt16 ret = SysFatalAlert(msg);
      *iret = ret;
      }
      break;

    case sysTrapSysFatalAlertInit: {
      SysFatalAlertInit();
      }
      break;

    case sysTrapSysFormPointerArrayToStrings: {
      Char *c = sys_va_arg(ap, Char *);
      Int16 stringCount = sys_va_arg(ap, Int32);
      MemHandle ret = SysFormPointerArrayToStrings(c, stringCount);
      *pret = (void *)ret;
      }
      break;

    case sysTrapSysGetOSVersionString: {
      Char *ret = SysGetOSVersionString();
      *pret = (void *)ret;
      }
      break;

    case sysTrapSysGetStackInfo: {
      MemPtr *startPP = sys_va_arg(ap, MemPtr *);
      MemPtr *endPP = sys_va_arg(ap, MemPtr *);
      Boolean ret = SysGetStackInfo(startPP, endPP);
      *iret = ret;
      }
      break;

    case sysTrapSysGetTrapAddress: {
      UInt16 trapNum = sys_va_arg(ap, UInt32);
      void *ret = SysGetTrapAddress(trapNum);
      *pret = (void *)ret;
      }
      break;

    case sysTrapSysGraffitiReferenceDialog: {
      ReferenceType referenceType = sys_va_arg(ap, ReferenceType);
      SysGraffitiReferenceDialog(referenceType);
      }
      break;

    case sysTrapSysHandleEvent: {
      EventPtr eventP = sys_va_arg(ap, EventPtr);
      Boolean ret = SysHandleEvent(eventP);
      *iret = ret;
      }
      break;

    case sysTrapSysInit: {
      SysInit();
      }
      break;

    case sysTrapSysInsertionSort: {
      void *baseP = sys_va_arg(ap, void *);
      UInt16 numOfElements = sys_va_arg(ap, UInt32);
      Int16 width = sys_va_arg(ap, Int32);
      CmpFuncPtr comparF = sys_va_arg(ap, CmpFuncPtr);
      Int32 other = sys_va_arg(ap, Int32);
      SysInsertionSort(baseP, numOfElements, width, comparF, other);
      }
      break;

    case sysTrapSysKernelInfo: {
      void *paramP = sys_va_arg(ap, void *);
      Err ret = SysKernelInfo(paramP);
      *iret = ret;
      }
      break;

    case sysTrapSysKeyboardDialog: {
      KeyboardType kbd = sys_va_arg(ap, KeyboardType);
      SysKeyboardDialog(kbd);
      }
      break;

    case sysTrapSysKeyboardDialogV10: {
      SysKeyboardDialogV10();
      }
      break;

    case sysTrapSysLCDBrightness: {
      Boolean set = sys_va_arg(ap, UInt32);
      UInt8 newBrightnessLevel = sys_va_arg(ap, UInt32);
      UInt8 ret = SysLCDBrightness(set, newBrightnessLevel);
      *iret = ret;
      }
      break;

    case sysTrapSysLCDContrast: {
      Boolean set = sys_va_arg(ap, UInt32);
      UInt8 newContrastLevel = sys_va_arg(ap, UInt32);
      UInt8 ret = SysLCDContrast(set, newContrastLevel);
      *iret = ret;
      }
      break;

    case sysTrapSysLaunchConsole: {
      Err ret = SysLaunchConsole();
      *iret = ret;
      }
      break;

    case sysTrapSysLibFind: {
      const Char *nameP = sys_va_arg(ap, Char *);
      UInt16 *refNumP = sys_va_arg(ap, UInt16 *);
      Err ret = SysLibFind(nameP, refNumP);
      *iret = ret;
      }
      break;

    case sysTrapSysLibInstall: {
      SysLibEntryProcPtr libraryP = sys_va_arg(ap, SysLibEntryProcPtr);
      UInt16 *refNumP = sys_va_arg(ap, UInt16 *);
      Err ret = SysLibInstall(libraryP, refNumP);
      *iret = ret;
      }
      break;

    case sysTrapSysLibLoad: {
      UInt32 libType = sys_va_arg(ap, UInt32);
      UInt32 libCreator = sys_va_arg(ap, UInt32);
      UInt16 *refNumP = sys_va_arg(ap, UInt16 *);
      Err ret = SysLibLoad(libType, libCreator, refNumP);
      *iret = ret;
      }
      break;

    case sysTrapSysLibRemove: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      Err ret = SysLibRemove(refNum);
      *iret = ret;
      }
      break;

    case sysTrapSysLibTblEntry: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      SysLibTblEntryType *ret = SysLibTblEntry(refNum);
      *pret = (void *)ret;
      }
      break;

    case sysTrapSysMailboxCreate: {
      UInt32 *mbIDP = sys_va_arg(ap, UInt32 *);
      UInt32 *tagP = sys_va_arg(ap, UInt32 *);
      UInt32 depth = sys_va_arg(ap, UInt32);
      Err ret = SysMailboxCreate(mbIDP, tagP, depth);
      *iret = ret;
      }
      break;

    case sysTrapSysMailboxDelete: {
      UInt32 mbID = sys_va_arg(ap, UInt32);
      Err ret = SysMailboxDelete(mbID);
      *iret = ret;
      }
      break;

    case sysTrapSysMailboxFlush: {
      UInt32 mbID = sys_va_arg(ap, UInt32);
      Err ret = SysMailboxFlush(mbID);
      *iret = ret;
      }
      break;

    case sysTrapSysMailboxSend: {
      UInt32 mbID = sys_va_arg(ap, UInt32);
      void *msgP = sys_va_arg(ap, void *);
      UInt32 wAck = sys_va_arg(ap, UInt32);
      Err ret = SysMailboxSend(mbID, msgP, wAck);
      *iret = ret;
      }
      break;

    case sysTrapSysMailboxWait: {
      UInt32 mbID = sys_va_arg(ap, UInt32);
      void *msgP = sys_va_arg(ap, void *);
      UInt32 priority = sys_va_arg(ap, UInt32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err ret = SysMailboxWait(mbID, msgP, priority, timeout);
      *iret = ret;
      }
      break;

    case sysTrapSysNewOwnerID: {
      UInt16 ret = SysNewOwnerID();
      *iret = ret;
      }
      break;

    case sysTrapSysNotifyRegister: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      UInt32 notifyType = sys_va_arg(ap, UInt32);
      SysNotifyProcPtr callbackP = sys_va_arg(ap, SysNotifyProcPtr);
      Int8 priority = sys_va_arg(ap, Int32);
      void *userDataP = sys_va_arg(ap, void *);
      Err ret = SysNotifyRegister(cardNo, dbID, notifyType, callbackP, priority, userDataP);
      *iret = ret;
      }
      break;

    case sysTrapSysNotifyBroadcast: {
      SysNotifyParamType *notify = sys_va_arg(ap, SysNotifyParamType *);
      Err ret = SysNotifyBroadcast(notify);
      *iret = ret;
      }
      break;

    case sysTrapSysNotifyBroadcastDeferred: {
      SysNotifyParamType *notify = sys_va_arg(ap, SysNotifyParamType *);
      Int16 paramSize = sys_va_arg(ap, Int32);
      Err ret = SysNotifyBroadcastDeferred(notify, paramSize);
      *iret = ret;
      }
      break;

    case sysTrapSysNotifyBroadcastFromInterrupt: {
      UInt32 notifyType = sys_va_arg(ap, UInt32);
      UInt32 broadcaster = sys_va_arg(ap, UInt32);
      void *notifyDetailsP = sys_va_arg(ap, void *);
      Err ret = SysNotifyBroadcastFromInterrupt(notifyType, broadcaster, notifyDetailsP);
      *iret = ret;
      }
      break;

    case sysTrapSysNotifyUnregister: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      UInt32 notifyType = sys_va_arg(ap, UInt32);
      Int8 priority = sys_va_arg(ap, Int32);
      Err ret = SysNotifyUnregister(cardNo, dbID, notifyType, priority);
      *iret = ret;
      }
      break;

    case sysTrapSysQSort: {
      void *baseP = sys_va_arg(ap, void *);
      UInt16 numOfElements = sys_va_arg(ap, UInt32);
      Int16 width = sys_va_arg(ap, Int32);
      CmpFuncPtr comparF = sys_va_arg(ap, CmpFuncPtr);
      Int32 other = sys_va_arg(ap, Int32);
      SysQSort(baseP, numOfElements, width, comparF, other);
      }
      break;

    case sysTrapSysRandom: {
      Int32 newSeed = sys_va_arg(ap, Int32);
      Int16 ret = SysRandom(newSeed);
      *iret = ret;
      }
      break;

    case sysTrapSysResSemaphoreCreate: {
      UInt32 *smIDP = sys_va_arg(ap, UInt32 *);
      UInt32 *tagP = sys_va_arg(ap, UInt32 *);
      Err ret = SysResSemaphoreCreate(smIDP, tagP);
      *iret = ret;
      }
      break;

    case sysTrapSysResSemaphoreDelete: {
      UInt32 smID = sys_va_arg(ap, UInt32);
      Err ret = SysResSemaphoreDelete(smID);
      *iret = ret;
      }
      break;

    case sysTrapSysResSemaphoreRelease: {
      UInt32 smID = sys_va_arg(ap, UInt32);
      Err ret = SysResSemaphoreRelease(smID);
      *iret = ret;
      }
      break;

    case sysTrapSysResSemaphoreReserve: {
      UInt32 smID = sys_va_arg(ap, UInt32);
      UInt32 priority = sys_va_arg(ap, UInt32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err ret = SysResSemaphoreReserve(smID, priority, timeout);
      *iret = ret;
      }
      break;

    case sysTrapSysReset: {
      SysReset();
      }
      break;

    case sysTrapSysRestoreStatus: {
      UInt16 status = sys_va_arg(ap, UInt32);
      SysRestoreStatus(status);
      }
      break;

    case sysTrapSysSemaphoreCreate: {
      UInt32 *smIDP = sys_va_arg(ap, UInt32 *);
      UInt32 *tagP = sys_va_arg(ap, UInt32 *);
      Int32 initValue = sys_va_arg(ap, Int32);
      Err ret = SysSemaphoreCreate(smIDP, tagP, initValue);
      *iret = ret;
      }
      break;

    case sysTrapSysSemaphoreDelete: {
      UInt32 smID = sys_va_arg(ap, UInt32);
      Err ret = SysSemaphoreDelete(smID);
      *iret = ret;
      }
      break;

    case sysTrapSysSemaphoreSet: {
      UInt32 smID = sys_va_arg(ap, UInt32);
      Err ret = SysSemaphoreSet(smID);
      *iret = ret;
      }
      break;

    case sysTrapSysSemaphoreSignal: {
      UInt32 smID = sys_va_arg(ap, UInt32);
      Err ret = SysSemaphoreSignal(smID);
      *iret = ret;
      }
      break;

    case sysTrapSysSemaphoreWait: {
      UInt32 smID = sys_va_arg(ap, UInt32);
      UInt32 priority = sys_va_arg(ap, UInt32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err ret = SysSemaphoreWait(smID, priority, timeout);
      *iret = ret;
      }
      break;

    case sysTrapSysSetA5: {
      UInt32 newValue = sys_va_arg(ap, UInt32);
      UInt32 ret = SysSetA5(newValue);
      *iret = ret;
      }
      break;

    case sysTrapSysSetAutoOffTime: {
      UInt16 seconds = sys_va_arg(ap, UInt32);
      UInt16 ret = SysSetAutoOffTime(seconds);
      *iret = ret;
      }
      break;

    case sysTrapSysSetPerformance: {
      UInt32 *sysClockP = sys_va_arg(ap, UInt32 *);
      UInt16 *cpuDutyP = sys_va_arg(ap, UInt16 *);
      Err ret = SysSetPerformance(sysClockP, cpuDutyP);
      *iret = ret;
      }
      break;

    case sysTrapSysSetTrapAddress: {
      UInt16 trapNum = sys_va_arg(ap, UInt32);
      void *procP = sys_va_arg(ap, void *);
      Err ret = SysSetTrapAddress(trapNum, procP);
      *iret = ret;
      }
      break;

    case sysTrapSysSleep: {
      Boolean untilReset = sys_va_arg(ap, UInt32);
      Boolean emergency = sys_va_arg(ap, UInt32);
      SysSleep(untilReset, emergency);
      }
      break;

    case sysTrapSysStringByIndex: {
      UInt16 resID = sys_va_arg(ap, UInt32);
      UInt16 index = sys_va_arg(ap, UInt32);
      Char *strP = sys_va_arg(ap, Char *);
      UInt16 maxLen = sys_va_arg(ap, UInt32);
      Char *ret = SysStringByIndex(resID, index, strP, maxLen);
      *pret = (void *)ret;
      }
      break;

    case sysTrapSysTaskCreate: {
      UInt32 *taskIDP = sys_va_arg(ap, UInt32 *);
      UInt32 *creator = sys_va_arg(ap, UInt32 *);
      ProcPtr codeP = sys_va_arg(ap, ProcPtr);
      MemPtr stackP = sys_va_arg(ap, MemPtr);
      UInt32 stackSize = sys_va_arg(ap, UInt32);
      UInt32 attr = sys_va_arg(ap, UInt32);
      UInt32 priority = sys_va_arg(ap, UInt32);
      UInt32 tSlice = sys_va_arg(ap, UInt32);
      Err ret = SysTaskCreate(taskIDP, creator, codeP, stackP, stackSize, attr, priority, tSlice);
      *iret = ret;
      }
      break;

    case sysTrapSysTaskDelay: {
      Int32 delay = sys_va_arg(ap, Int32);
      Err ret = SysTaskDelay(delay);
      *iret = ret;
      }
      break;

    case sysTrapSysTaskDelete: {
      UInt32 taskID = sys_va_arg(ap, UInt32);
      UInt32 priority = sys_va_arg(ap, UInt32);
      Err ret = SysTaskDelete(taskID, priority);
      *iret = ret;
      }
      break;

    case sysTrapSysTaskID: {
      UInt32 ret = SysTaskID();
      *iret = ret;
      }
      break;

    case sysTrapSysTaskResume: {
      UInt32 taskID = sys_va_arg(ap, UInt32);
      Err ret = SysTaskResume(taskID);
      *iret = ret;
      }
      break;

    case sysTrapSysTaskSetTermProc: {
      UInt32 taskID = sys_va_arg(ap, UInt32);
      SysTermProcPtr termProcP = sys_va_arg(ap, SysTermProcPtr);
      Err ret = SysTaskSetTermProc(taskID, termProcP);
      *iret = ret;
      }
      break;

    case sysTrapSysTaskSuspend: {
      UInt32 taskID = sys_va_arg(ap, UInt32);
      Err ret = SysTaskSuspend(taskID);
      *iret = ret;
      }
      break;

    case sysTrapSysTaskSwitching: {
      Boolean enable = sys_va_arg(ap, UInt32);
      Err ret = SysTaskSwitching(enable);
      *iret = ret;
      }
      break;

    case sysTrapSysTaskTrigger: {
      UInt32 taskID = sys_va_arg(ap, UInt32);
      Err ret = SysTaskTrigger(taskID);
      *iret = ret;
      }
      break;

    case sysTrapSysTaskWait: {
      Int32 timeout = sys_va_arg(ap, Int32);
      Err ret = SysTaskWait(timeout);
      *iret = ret;
      }
      break;

    case sysTrapSysTaskWaitClr: {
      SysTaskWaitClr();
      }
      break;

    case sysTrapSysTaskWake: {
      UInt32 taskID = sys_va_arg(ap, UInt32);
      Err ret = SysTaskWake(taskID);
      *iret = ret;
      }
      break;

    case sysTrapSysTicksPerSecond: {
      UInt16 ret = SysTicksPerSecond();
      *iret = ret;
      }
      break;

    case sysTrapSysTimerCreate: {
      UInt32 *timerIDP = sys_va_arg(ap, UInt32 *);
      UInt32 *tagP = sys_va_arg(ap, UInt32 *);
      SysTimerProcPtr timerProc = sys_va_arg(ap, SysTimerProcPtr);
      UInt32 periodicDelay = sys_va_arg(ap, UInt32);
      UInt32 param = sys_va_arg(ap, UInt32);
      Err ret = SysTimerCreate(timerIDP, tagP, timerProc, periodicDelay, param);
      *iret = ret;
      }
      break;

    case sysTrapSysTimerDelete: {
      UInt32 timerID = sys_va_arg(ap, UInt32);
      Err ret = SysTimerDelete(timerID);
      *iret = ret;
      }
      break;

    case sysTrapSysTimerRead: {
      UInt32 timerID = sys_va_arg(ap, UInt32);
      UInt32 *valueP = sys_va_arg(ap, UInt32 *);
      Err ret = SysTimerRead(timerID, valueP);
      *iret = ret;
      }
      break;

    case sysTrapSysTimerWrite: {
      UInt32 timerID = sys_va_arg(ap, UInt32);
      UInt32 value = sys_va_arg(ap, UInt32);
      Err ret = SysTimerWrite(timerID, value);
      *iret = ret;
      }
      break;

    case sysTrapSysTranslateKernelErr: {
      Err err = sys_va_arg(ap, UInt32);
      Err ret = SysTranslateKernelErr(err);
      *iret = ret;
      }
      break;

    case sysTrapSysUIAppSwitch: {
      UInt16 cardNo = sys_va_arg(ap, UInt32);
      LocalID dbID = sys_va_arg(ap, LocalID);
      UInt16 cmd = sys_va_arg(ap, UInt32);
      MemPtr cmdPBP = sys_va_arg(ap, MemPtr);
      Err ret = SysUIAppSwitch(cardNo, dbID, cmd, cmdPBP);
      *iret = ret;
      }
      break;

    case sysTrapSysUIBusy: {
      Boolean set = sys_va_arg(ap, UInt32);
      Boolean value = sys_va_arg(ap, UInt32);
      UInt16 ret = SysUIBusy(set, value);
      *iret = ret;
      }
      break;

    case sysTrapSysUILaunch: {
      SysUILaunch();
      }
      break;

    case sysTrapSysUnimplemented: {
      SysUnimplemented();
      }
      break;

    case sysTrapTblDrawTable: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      TblDrawTable(tableP);
      }
      break;

    case sysTrapTblEditing: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Boolean ret = TblEditing(tableP);
      *iret = ret;
      }
      break;

    case sysTrapTblEraseTable: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      TblEraseTable(tableP);
      }
      break;

    case sysTrapTblFindRowData: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      UIntPtr data = sys_va_arg(ap, UIntPtr);
      Int16 *rowP = sys_va_arg(ap, Int16 *);
      Boolean ret = TblFindRowData(tableP, data, rowP);
      *iret = ret;
      }
      break;

    case sysTrapTblFindRowID: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      UInt16 id = sys_va_arg(ap, UInt32);
      Int16 *rowP = sys_va_arg(ap, Int16 *);
      Boolean ret = TblFindRowID(tableP, id, rowP);
      *iret = ret;
      }
      break;

    case sysTrapTblGetBounds: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      RectangleType *rP = sys_va_arg(ap, RectangleType *);
      TblGetBounds(tableP, rP);
      }
      break;

    case sysTrapTblGetColumnSpacing: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 column = sys_va_arg(ap, Int32);
      Coord ret = TblGetColumnSpacing(tableP, column);
      *iret = ret;
      }
      break;

    case sysTrapTblGetColumnWidth: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 column = sys_va_arg(ap, Int32);
      Coord ret = TblGetColumnWidth(tableP, column);
      *iret = ret;
      }
      break;

    case sysTrapTblGetCurrentField: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      FieldType *ret = TblGetCurrentField(tableP);
      *pret = (void *)ret;
      }
      break;

    case sysTrapTblGetItemBounds: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Int16 column = sys_va_arg(ap, Int32);
      RectangleType *rP = sys_va_arg(ap, RectangleType *);
      TblGetItemBounds(tableP, row, column, rP);
      }
      break;

    case sysTrapTblGetItemFont: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Int16 column = sys_va_arg(ap, Int32);
      FontID ret = TblGetItemFont(tableP, row, column);
      *iret = ret;
      }
      break;

    case sysTrapTblGetItemInt: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Int16 column = sys_va_arg(ap, Int32);
      Int16 ret = TblGetItemInt(tableP, row, column);
      *iret = ret;
      }
      break;

    case sysTrapTblGetItemPtr: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Int16 column = sys_va_arg(ap, Int32);
      void *ret = TblGetItemPtr(tableP, row, column);
      *pret = (void *)ret;
      }
      break;

    case sysTrapTblGetLastUsableRow: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 ret = TblGetLastUsableRow(tableP);
      *iret = ret;
      }
      break;

    case sysTrapTblGetNumberOfColumns: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 ret = TblGetNumberOfColumns(tableP);
      *iret = ret;
      }
      break;

    case sysTrapTblGetNumberOfRows: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 ret = TblGetNumberOfRows(tableP);
      *iret = ret;
      }
      break;

    case sysTrapTblGetRowData: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      UIntPtr ret = TblGetRowData(tableP, row);
      *iret = ret;
      }
      break;

    case sysTrapTblGetRowHeight: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Coord ret = TblGetRowHeight(tableP, row);
      *iret = ret;
      }
      break;

    case sysTrapTblGetRowID: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      UInt16 ret = TblGetRowID(tableP, row);
      *iret = ret;
      }
      break;

    case sysTrapTblGetSelection: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 *rowP = sys_va_arg(ap, Int16 *);
      Int16 *columnP = sys_va_arg(ap, Int16 *);
      Boolean ret = TblGetSelection(tableP, rowP, columnP);
      *iret = ret;
      }
      break;

    case sysTrapTblGetTopRow: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 ret = TblGetTopRow(tableP);
      *iret = ret;
      }
      break;

    case sysTrapTblGrabFocus: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Int16 column = sys_va_arg(ap, Int32);
      TblGrabFocus(tableP, row, column);
      }
      break;

    case sysTrapTblHandleEvent: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      EventType *event = sys_va_arg(ap, EventType *);
      Boolean ret = TblHandleEvent(tableP, event);
      *iret = ret;
      }
      break;

    case sysTrapTblHasScrollBar: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Boolean hasScrollBar = sys_va_arg(ap, UInt32);
      TblHasScrollBar(tableP, hasScrollBar);
      }
      break;

    case sysTrapTblInsertRow: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      TblInsertRow(tableP, row);
      }
      break;

    case sysTrapTblMarkRowInvalid: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      TblMarkRowInvalid(tableP, row);
      }
      break;

    case sysTrapTblMarkTableInvalid: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      TblMarkTableInvalid(tableP);
      }
      break;

    case sysTrapTblRedrawTable: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      TblRedrawTable(tableP);
      }
      break;

    case sysTrapTblReleaseFocus: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      TblReleaseFocus(tableP);
      }
      break;

    case sysTrapTblRemoveRow: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      TblRemoveRow(tableP, row);
      }
      break;

    case sysTrapTblRowInvalid: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Boolean ret = TblRowInvalid(tableP, row);
      *iret = ret;
      }
      break;

    case sysTrapTblRowMasked: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Boolean ret = TblRowMasked(tableP, row);
      *iret = ret;
      }
      break;

    case sysTrapTblRowSelectable: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Boolean ret = TblRowSelectable(tableP, row);
      *iret = ret;
      }
      break;

    case sysTrapTblRowUsable: {
      const TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Boolean ret = TblRowUsable(tableP, row);
      *iret = ret;
      }
      break;

    case sysTrapTblSelectItem: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Int16 column = sys_va_arg(ap, Int32);
      TblSelectItem(tableP, row, column);
      }
      break;

    case sysTrapTblSetBounds: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      TblSetBounds(tableP, rP);
      }
      break;

    case sysTrapTblSetColumnEditIndicator: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 column = sys_va_arg(ap, Int32);
      Boolean editIndicator = sys_va_arg(ap, UInt32);
      TblSetColumnEditIndicator(tableP, column, editIndicator);
      }
      break;

    case sysTrapTblSetColumnMasked: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 column = sys_va_arg(ap, Int32);
      Boolean masked = sys_va_arg(ap, UInt32);
      TblSetColumnMasked(tableP, column, masked);
      }
      break;

    case sysTrapTblSetColumnSpacing: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 column = sys_va_arg(ap, Int32);
      Coord spacing = sys_va_arg(ap, Int32);
      TblSetColumnSpacing(tableP, column, spacing);
      }
      break;

    case sysTrapTblSetColumnUsable: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 column = sys_va_arg(ap, Int32);
      Boolean usable = sys_va_arg(ap, UInt32);
      TblSetColumnUsable(tableP, column, usable);
      }
      break;

    case sysTrapTblSetColumnWidth: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 column = sys_va_arg(ap, Int32);
      Coord width = sys_va_arg(ap, Int32);
      TblSetColumnWidth(tableP, column, width);
      }
      break;

    case sysTrapTblSetCustomDrawProcedure: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 column = sys_va_arg(ap, Int32);
      TableDrawItemFuncPtr drawCallback = sys_va_arg(ap, TableDrawItemFuncPtr);
      TblSetCustomDrawProcedure(tableP, column, drawCallback);
      }
      break;

    case sysTrapTblSetItemFont: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Int16 column = sys_va_arg(ap, Int32);
      FontID fontID = sys_va_arg(ap, FontID);
      TblSetItemFont(tableP, row, column, fontID);
      }
      break;

    case sysTrapTblSetItemInt: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Int16 column = sys_va_arg(ap, Int32);
      Int16 value = sys_va_arg(ap, Int32);
      TblSetItemInt(tableP, row, column, value);
      }
      break;

    case sysTrapTblSetItemPtr: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Int16 column = sys_va_arg(ap, Int32);
      void *value = sys_va_arg(ap, void *);
      TblSetItemPtr(tableP, row, column, value);
      }
      break;

    case sysTrapTblSetItemStyle: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Int16 column = sys_va_arg(ap, Int32);
      TableItemStyleType type = sys_va_arg(ap, TableItemStyleType);
      TblSetItemStyle(tableP, row, column, type);
      }
      break;

    case sysTrapTblSetLoadDataProcedure: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 column = sys_va_arg(ap, Int32);
      TableLoadDataFuncPtr loadDataCallback = sys_va_arg(ap, TableLoadDataFuncPtr);
      TblSetLoadDataProcedure(tableP, column, loadDataCallback);
      }
      break;

    case sysTrapTblSetRowData: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      UIntPtr data = sys_va_arg(ap, UIntPtr);
      TblSetRowData(tableP, row, data);
      }
      break;

    case sysTrapTblSetRowHeight: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Coord height = sys_va_arg(ap, Int32);
      TblSetRowHeight(tableP, row, height);
      }
      break;

    case sysTrapTblSetRowID: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      UInt16 id = sys_va_arg(ap, UInt32);
      TblSetRowID(tableP, row, id);
      }
      break;

    case sysTrapTblSetRowMasked: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Boolean masked = sys_va_arg(ap, UInt32);
      TblSetRowMasked(tableP, row, masked);
      }
      break;

    case sysTrapTblSetRowSelectable: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Boolean selectable = sys_va_arg(ap, UInt32);
      TblSetRowSelectable(tableP, row, selectable);
      }
      break;

    case sysTrapTblSetRowStaticHeight: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Boolean staticHeight = sys_va_arg(ap, UInt32);
      TblSetRowStaticHeight(tableP, row, staticHeight);
      }
      break;

    case sysTrapTblSetRowUsable: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Boolean usable = sys_va_arg(ap, UInt32);
      TblSetRowUsable(tableP, row, usable);
      }
      break;

    case sysTrapTblSetSaveDataProcedure: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 column = sys_va_arg(ap, Int32);
      TableSaveDataFuncPtr saveDataCallback = sys_va_arg(ap, TableSaveDataFuncPtr);
      TblSetSaveDataProcedure(tableP, column, saveDataCallback);
      }
      break;

    case sysTrapTblSetSelection: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      Int16 row = sys_va_arg(ap, Int32);
      Int16 column = sys_va_arg(ap, Int32);
      TblSetSelection(tableP, row, column);
      }
      break;

    case sysTrapTblUnhighlightSelection: {
      TableType *tableP = sys_va_arg(ap, TableType *);
      TblUnhighlightSelection(tableP);
      }
      break;

    case sysTrapTimAdjust: {
      DateTimeType *dateTimeP = sys_va_arg(ap, DateTimeType *);
      Int32 adjustment = sys_va_arg(ap, Int32);
      TimAdjust(dateTimeP, adjustment);
      }
      break;

    case sysTrapTimDateTimeToSeconds: {
      const DateTimeType *dateTimeP = sys_va_arg(ap, DateTimeType *);
      UInt32 ret = TimDateTimeToSeconds(dateTimeP);
      *iret = ret;
      }
      break;

    case sysTrapTimGetSeconds: {
      UInt32 ret = TimGetSeconds();
      *iret = ret;
      }
      break;

    case sysTrapTimGetTicks: {
      UInt32 ret = TimGetTicks();
      *iret = ret;
      }
      break;

    case sysTrapTimInit: {
      Err ret = TimInit();
      *iret = ret;
      }
      break;

    case sysTrapTimSecondsToDateTime: {
      UInt32 seconds = sys_va_arg(ap, UInt32);
      DateTimeType *dateTimeP = sys_va_arg(ap, DateTimeType *);
      TimSecondsToDateTime(seconds, dateTimeP);
      }
      break;

    case sysTrapTimSetSeconds: {
      UInt32 seconds = sys_va_arg(ap, UInt32);
      TimSetSeconds(seconds);
      }
      break;

    case sysTrapTimTimeZoneToUTC: {
      UInt32 seconds = sys_va_arg(ap, UInt32);
      Int16 timeZone = sys_va_arg(ap, Int32);
      Int16 daylightSavingAdjustment = sys_va_arg(ap, Int32);
      UInt32 ret = TimTimeZoneToUTC(seconds, timeZone, daylightSavingAdjustment);
      *iret = ret;
      }
      break;

    case sysTrapTimUTCToTimeZone: {
      UInt32 seconds = sys_va_arg(ap, UInt32);
      Int16 timeZone = sys_va_arg(ap, Int32);
      Int16 daylightSavingAdjustment = sys_va_arg(ap, Int32);
      UInt32 ret = TimUTCToTimeZone(seconds, timeZone, daylightSavingAdjustment);
      *iret = ret;
      }
      break;

    case sysTrapTimeToAscii: {
      UInt8 hours = sys_va_arg(ap, UInt32);
      UInt8 minutes = sys_va_arg(ap, UInt32);
      TimeFormatType timeFormat = sys_va_arg(ap, TimeFormatType);
      Char *pString = sys_va_arg(ap, Char *);
      TimeToAscii(hours, minutes, timeFormat, pString);
      }
      break;

    case sysTrapTimeZoneToAscii: {
      Int16 timeZone = sys_va_arg(ap, Int32);
      const LmLocaleType *localeP = sys_va_arg(ap, LmLocaleType *);
      Char *string = sys_va_arg(ap, Char *);
      TimeZoneToAscii(timeZone, localeP, string);
      }
      break;

    case sysTrapUIBrightnessAdjust: {
      UIBrightnessAdjust();
      }
      break;

    case sysTrapUIColorGetTableEntryIndex: {
      UIColorTableEntries which = sys_va_arg(ap, UIColorTableEntries);
      IndexedColorType ret = UIColorGetTableEntryIndex(which);
      *iret = ret;
      }
      break;

    case sysTrapUIColorGetTableEntryRGB: {
      UIColorTableEntries which = sys_va_arg(ap, UIColorTableEntries);
      RGBColorType *rgbP = sys_va_arg(ap, RGBColorType *);
      UIColorGetTableEntryRGB(which, rgbP);
      }
      break;

    case sysTrapUIColorPopTable: {
      Err ret = UIColorPopTable();
      *iret = ret;
      }
      break;

    case sysTrapUIColorPushTable: {
      Err ret = UIColorPushTable();
      *iret = ret;
      }
      break;

    case sysTrapUIColorSetTableEntry: {
      UIColorTableEntries which = sys_va_arg(ap, UIColorTableEntries);
      const RGBColorType *rgbP = sys_va_arg(ap, RGBColorType *);
      Err ret = UIColorSetTableEntry(which, rgbP);
      *iret = ret;
      }
      break;

    case sysTrapUIContrastAdjust: {
      UIContrastAdjust();
      }
      break;

    case sysTrapUIPickColor: {
      IndexedColorType *indexP = sys_va_arg(ap, IndexedColorType *);
      RGBColorType *rgbP = sys_va_arg(ap, RGBColorType *);
      UIPickColorStartType start = sys_va_arg(ap, UInt32);
      const Char *titleP = sys_va_arg(ap, Char *);
      const Char *tipP = sys_va_arg(ap, Char *);
      Boolean ret = UIPickColor(indexP, rgbP, start, titleP, tipP);
      *iret = ret;
      }
      break;

    case sysTrapWinAddWindow: {
      WinHandle winHandle = sys_va_arg(ap, WinHandle);
      WinAddWindow(winHandle);
      }
      break;

    case sysTrapWinClipRectangle: {
      RectangleType *rP = sys_va_arg(ap, RectangleType *);
      WinClipRectangle(rP);
      }
      break;

    case sysTrapWinCopyRectangle: {
      WinHandle srcWin = sys_va_arg(ap, WinHandle);
      WinHandle dstWin = sys_va_arg(ap, WinHandle);
      const RectangleType *srcRect = sys_va_arg(ap, RectangleType *);
      Coord destX = sys_va_arg(ap, Int32);
      Coord destY = sys_va_arg(ap, Int32);
      WinDrawOperation mode = sys_va_arg(ap, WinDrawOperation);
      WinCopyRectangle(srcWin, dstWin, srcRect, destX, destY, mode);
      }
      break;

    case sysTrapWinCreateBitmapWindow: {
      BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      UInt16 *error = sys_va_arg(ap, UInt16 *);
      WinHandle ret = WinCreateBitmapWindow(bitmapP, error);
      *pret = (void *)ret;
      }
      break;

    case sysTrapWinCreateOffscreenWindow: {
      Coord width = sys_va_arg(ap, Int32);
      Coord height = sys_va_arg(ap, Int32);
      WindowFormatType format = sys_va_arg(ap, WindowFormatType);
      UInt16 *error = sys_va_arg(ap, UInt16 *);
      WinHandle ret = WinCreateOffscreenWindow(width, height, format, error);
      *pret = (void *)ret;
      }
      break;

    case sysTrapWinCreateWindow: {
      const RectangleType *bounds = sys_va_arg(ap, RectangleType *);
      FrameType frame = sys_va_arg(ap, UInt32);
      Boolean modal = sys_va_arg(ap, UInt32);
      Boolean focusable = sys_va_arg(ap, UInt32);
      UInt16 *error = sys_va_arg(ap, UInt16 *);
      WinHandle ret = WinCreateWindow(bounds, frame, modal, focusable, error);
      *pret = (void *)ret;
      }
      break;

    case sysTrapWinDeleteWindow: {
      WinHandle winHandle = sys_va_arg(ap, WinHandle);
      Boolean eraseIt = sys_va_arg(ap, UInt32);
      WinDeleteWindow(winHandle, eraseIt);
      }
      break;

    case sysTrapWinDisableWindow: {
      WinHandle winHandle = sys_va_arg(ap, WinHandle);
      WinDisableWindow(winHandle);
      }
      break;

    case sysTrapWinDisplayToWindowPt: {
      Coord *extentX = sys_va_arg(ap, Coord *);
      Coord *extentY = sys_va_arg(ap, Coord *);
      WinDisplayToWindowPt(extentX, extentY);
      }
      break;

    case sysTrapWinDrawBitmap: {
      BitmapPtr bitmapP = sys_va_arg(ap, BitmapPtr);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      WinDrawBitmap(bitmapP, x, y);
      }
      break;

    case sysTrapWinDrawChar: {
      WChar theChar = sys_va_arg(ap, UInt32);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      WinDrawChar(theChar, x, y);
      }
      break;

    case sysTrapWinDrawChars: {
      const Char *chars = sys_va_arg(ap, Char *);
      Int16 len = sys_va_arg(ap, Int32);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      WinDrawChars(chars, len, x, y);
      }
      break;

    case sysTrapWinDrawGrayLine: {
      Coord x1 = sys_va_arg(ap, Int32);
      Coord y1 = sys_va_arg(ap, Int32);
      Coord x2 = sys_va_arg(ap, Int32);
      Coord y2 = sys_va_arg(ap, Int32);
      WinDrawGrayLine(x1, y1, x2, y2);
      }
      break;

    case sysTrapWinDrawGrayRectangleFrame: {
      FrameType frame = sys_va_arg(ap, UInt32);
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      WinDrawGrayRectangleFrame(frame, rP);
      }
      break;

    case sysTrapWinDrawInvertedChars: {
      const Char *chars = sys_va_arg(ap, Char *);
      Int16 len = sys_va_arg(ap, Int32);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      WinDrawInvertedChars(chars, len, x, y);
      }
      break;

    case sysTrapWinDrawLine: {
      Coord x1 = sys_va_arg(ap, Int32);
      Coord y1 = sys_va_arg(ap, Int32);
      Coord x2 = sys_va_arg(ap, Int32);
      Coord y2 = sys_va_arg(ap, Int32);
      WinDrawLine(x1, y1, x2, y2);
      }
      break;

    case sysTrapWinDrawPixel: {
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      WinDrawPixel(x, y);
      }
      break;

    case sysTrapWinDrawRectangle: {
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      UInt16 cornerDiam = sys_va_arg(ap, UInt32);
      WinDrawRectangle(rP, cornerDiam);
      }
      break;

    case sysTrapWinDrawRectangleFrame: {
      FrameType frame = sys_va_arg(ap, UInt32);
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      WinDrawRectangleFrame(frame, rP);
      }
      break;

    case sysTrapWinDrawTruncChars: {
      const Char *chars = sys_va_arg(ap, Char *);
      Int16 len = sys_va_arg(ap, Int32);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      Coord maxWidth = sys_va_arg(ap, Int32);
      WinDrawTruncChars(chars, len, x, y, maxWidth);
      }
      break;

    case sysTrapWinDrawWindowFrame: {
      WinDrawWindowFrame();
      }
      break;

    case sysTrapWinEnableWindow: {
      WinHandle winHandle = sys_va_arg(ap, WinHandle);
      WinEnableWindow(winHandle);
      }
      break;

    case sysTrapWinEraseChars: {
      const Char *chars = sys_va_arg(ap, Char *);
      Int16 len = sys_va_arg(ap, Int32);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      WinEraseChars(chars, len, x, y);
      }
      break;

    case sysTrapWinEraseLine: {
      Coord x1 = sys_va_arg(ap, Int32);
      Coord y1 = sys_va_arg(ap, Int32);
      Coord x2 = sys_va_arg(ap, Int32);
      Coord y2 = sys_va_arg(ap, Int32);
      WinEraseLine(x1, y1, x2, y2);
      }
      break;

    case sysTrapWinErasePixel: {
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      WinErasePixel(x, y);
      }
      break;

    case sysTrapWinEraseRectangle: {
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      UInt16 cornerDiam = sys_va_arg(ap, UInt32);
      WinEraseRectangle(rP, cornerDiam);
      }
      break;

    case sysTrapWinEraseRectangleFrame: {
      FrameType frame = sys_va_arg(ap, UInt32);
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      WinEraseRectangleFrame(frame, rP);
      }
      break;

    case sysTrapWinEraseWindow: {
      WinEraseWindow();
      }
      break;

    case sysTrapWinFillLine: {
      Coord x1 = sys_va_arg(ap, Int32);
      Coord y1 = sys_va_arg(ap, Int32);
      Coord x2 = sys_va_arg(ap, Int32);
      Coord y2 = sys_va_arg(ap, Int32);
      WinFillLine(x1, y1, x2, y2);
      }
      break;

    case sysTrapWinFillRectangle: {
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      UInt16 cornerDiam = sys_va_arg(ap, UInt32);
      WinFillRectangle(rP, cornerDiam);
      }
      break;

    case sysTrapWinGetActiveWindow: {
      WinHandle ret = WinGetActiveWindow();
      *pret = (void *)ret;
      }
      break;

    case sysTrapWinGetBitmap: {
      WinHandle winHandle = sys_va_arg(ap, WinHandle);
      BitmapType *ret = WinGetBitmap(winHandle);
      *pret = (void *)ret;
      }
      break;

    case sysTrapWinGetBounds: {
      WinHandle winH = sys_va_arg(ap, WinHandle);
      RectangleType *rP = sys_va_arg(ap, RectangleType *);
      WinGetBounds(winH, rP);
      }
      break;

    case sysTrapWinGetClip: {
      RectangleType *rP = sys_va_arg(ap, RectangleType *);
      WinGetClip(rP);
      }
      break;

    case sysTrapWinGetDisplayExtent: {
      Coord *extentX = sys_va_arg(ap, Coord *);
      Coord *extentY = sys_va_arg(ap, Coord *);
      WinGetDisplayExtent(extentX, extentY);
      }
      break;

    case sysTrapWinGetDisplayWindow: {
      WinHandle ret = WinGetDisplayWindow();
      *pret = (void *)ret;
      }
      break;

    case sysTrapWinGetDrawWindow: {
      WinHandle ret = WinGetDrawWindow();
      *pret = (void *)ret;
      }
      break;

    case sysTrapWinGetDrawWindowBounds: {
      RectangleType *rP = sys_va_arg(ap, RectangleType *);
      WinGetDrawWindowBounds(rP);
      }
      break;

    case sysTrapWinGetFirstWindow: {
      WinHandle ret = WinGetFirstWindow();
      *pret = (void *)ret;
      }
      break;

    case sysTrapWinGetFramesRectangle: {
      FrameType frame = sys_va_arg(ap, UInt32);
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      RectangleType *obscuredRect = sys_va_arg(ap, RectangleType *);
      WinGetFramesRectangle(frame, rP, obscuredRect);
      }
      break;

    case sysTrapWinGetPattern: {
      CustomPatternType *patternP = sys_va_arg(ap, CustomPatternType *);
      WinGetPattern(patternP);
      }
      break;

    case sysTrapWinGetPatternType: {
      PatternType ret = WinGetPatternType();
      *iret = ret;
      }
      break;

    case sysTrapWinGetPixel: {
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      IndexedColorType ret = WinGetPixel(x, y);
      *iret = ret;
      }
      break;

    case sysTrapWinGetPixelRGB: {
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      RGBColorType *rgbP = sys_va_arg(ap, RGBColorType *);
      Err ret = WinGetPixelRGB(x, y, rgbP);
      *iret = ret;
      }
      break;

    case sysTrapWinGetWindowExtent: {
      Coord *extentX = sys_va_arg(ap, Coord *);
      Coord *extentY = sys_va_arg(ap, Coord *);
      WinGetWindowExtent(extentX, extentY);
      }
      break;

    case sysTrapWinGetWindowFrameRect: {
      WinHandle winHandle = sys_va_arg(ap, WinHandle);
      RectangleType *r = sys_va_arg(ap, RectangleType *);
      WinGetWindowFrameRect(winHandle, r);
      }
      break;

    case sysTrapWinIndexToRGB: {
      IndexedColorType i = sys_va_arg(ap, UInt32);
      RGBColorType *rgbP = sys_va_arg(ap, RGBColorType *);
      WinIndexToRGB(i, rgbP);
      }
      break;

    case sysTrapWinInitializeWindow: {
      WinHandle winHandle = sys_va_arg(ap, WinHandle);
      WinInitializeWindow(winHandle);
      }
      break;

    case sysTrapWinInvertChars: {
      const Char *chars = sys_va_arg(ap, Char *);
      Int16 len = sys_va_arg(ap, Int32);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      WinInvertChars(chars, len, x, y);
      }
      break;

    case sysTrapWinInvertLine: {
      Coord x1 = sys_va_arg(ap, Int32);
      Coord y1 = sys_va_arg(ap, Int32);
      Coord x2 = sys_va_arg(ap, Int32);
      Coord y2 = sys_va_arg(ap, Int32);
      WinInvertLine(x1, y1, x2, y2);
      }
      break;

    case sysTrapWinInvertPixel: {
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      WinInvertPixel(x, y);
      }
      break;

    case sysTrapWinInvertRectangle: {
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      UInt16 cornerDiam = sys_va_arg(ap, UInt32);
      WinInvertRectangle(rP, cornerDiam);
      }
      break;

    case sysTrapWinInvertRectangleFrame: {
      FrameType frame = sys_va_arg(ap, UInt32);
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      WinInvertRectangleFrame(frame, rP);
      }
      break;

    case sysTrapWinModal: {
      WinHandle winHandle = sys_va_arg(ap, WinHandle);
      Boolean ret = WinModal(winHandle);
      *iret = ret;
      }
      break;

    case sysTrapWinMoveWindowAddr: {
      WindowType *oldLocationP = sys_va_arg(ap, WindowType *);
      WindowType *newLocationP = sys_va_arg(ap, WindowType *);
      WinMoveWindowAddr(oldLocationP, newLocationP);
      }
      break;

    case sysTrapWinPaintBitmap: {
      BitmapType *bitmapP = sys_va_arg(ap, BitmapType *);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      WinPaintBitmap(bitmapP, x, y);
      }
      break;

    case sysTrapWinPaintChar: {
      WChar theChar = sys_va_arg(ap, UInt32);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      WinPaintChar(theChar, x, y);
      }
      break;

    case sysTrapWinPaintChars: {
      const Char *chars = sys_va_arg(ap, Char *);
      Int16 len = sys_va_arg(ap, Int32);
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      WinPaintChars(chars, len, x, y);
      }
      break;

    case sysTrapWinPaintLine: {
      Coord x1 = sys_va_arg(ap, Int32);
      Coord y1 = sys_va_arg(ap, Int32);
      Coord x2 = sys_va_arg(ap, Int32);
      Coord y2 = sys_va_arg(ap, Int32);
      WinPaintLine(x1, y1, x2, y2);
      }
      break;

    case sysTrapWinPaintLines: {
      UInt16 numLines = sys_va_arg(ap, UInt32);
      WinLineType *lines = sys_va_arg(ap, WinLineType *);
      WinPaintLines(numLines, lines);
      }
      break;

    case sysTrapWinPaintPixel: {
      Coord x = sys_va_arg(ap, Int32);
      Coord y = sys_va_arg(ap, Int32);
      WinPaintPixel(x, y);
      }
      break;

    case sysTrapWinPaintPixels: {
      UInt16 numPoints = sys_va_arg(ap, UInt32);
      PointType *pts = sys_va_arg(ap, PointType *);
      WinPaintPixels(numPoints, pts);
      }
      break;

    case sysTrapWinPaintRectangle: {
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      UInt16 cornerDiam = sys_va_arg(ap, UInt32);
      WinPaintRectangle(rP, cornerDiam);
      }
      break;

    case sysTrapWinPaintRectangleFrame: {
      FrameType frame = sys_va_arg(ap, UInt32);
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      WinPaintRectangleFrame(frame, rP);
      }
      break;

    case sysTrapWinPalette: {
      UInt8 operation = sys_va_arg(ap, UInt32);
      Int16 startIndex = sys_va_arg(ap, Int32);
      UInt16 paletteEntries = sys_va_arg(ap, UInt32);
      RGBColorType *tableP = sys_va_arg(ap, RGBColorType *);
      Err ret = WinPalette(operation, startIndex, paletteEntries, tableP);
      *iret = ret;
      }
      break;

    case sysTrapWinPopDrawState: {
      WinPopDrawState();
      }
      break;

    case sysTrapWinPushDrawState: {
      WinPushDrawState();
      }
      break;

    case sysTrapWinRGBToIndex: {
      const RGBColorType *rgbP = sys_va_arg(ap, RGBColorType *);
      IndexedColorType ret = WinRGBToIndex(rgbP);
      *iret = ret;
      }
      break;

    case sysTrapWinRemoveWindow: {
      WinHandle winHandle = sys_va_arg(ap, WinHandle);
      WinRemoveWindow(winHandle);
      }
      break;

    case sysTrapWinResetClip: {
      WinResetClip();
      }
      break;

    case sysTrapWinRestoreBits: {
      WinHandle winHandle = sys_va_arg(ap, WinHandle);
      Coord destX = sys_va_arg(ap, Int32);
      Coord destY = sys_va_arg(ap, Int32);
      WinRestoreBits(winHandle, destX, destY);
      }
      break;

    case sysTrapWinSaveBits: {
      const RectangleType *source = sys_va_arg(ap, RectangleType *);
      UInt16 *error = sys_va_arg(ap, UInt16 *);
      WinHandle ret = WinSaveBits(source, error);
      *pret = (void *)ret;
      }
      break;

    case sysTrapWinScreenInit: {
      WinScreenInit();
      }
      break;

    case sysTrapWinScreenLock: {
      WinLockInitType initMode = sys_va_arg(ap, WinLockInitType);
      UInt8 *ret = WinScreenLock(initMode);
      *pret = (void *)ret;
      }
      break;

    case sysTrapWinScreenMode: {
      WinScreenModeOperation operation = sys_va_arg(ap, WinScreenModeOperation);
      UInt32 *widthP = sys_va_arg(ap, UInt32 *);
      UInt32 *heightP = sys_va_arg(ap, UInt32 *);
      UInt32 *depthP = sys_va_arg(ap, UInt32 *);
      Boolean *enableColorP = sys_va_arg(ap, Boolean *);
      Err ret = WinScreenMode(operation, widthP, heightP, depthP, enableColorP);
      *iret = ret;
      }
      break;

    case sysTrapWinScreenUnlock: {
      WinScreenUnlock();
      }
      break;

    case sysTrapWinScrollRectangle: {
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      WinDirectionType direction = sys_va_arg(ap, WinDirectionType);
      Coord distance = sys_va_arg(ap, Int32);
      RectangleType *vacatedP = sys_va_arg(ap, RectangleType *);
      WinScrollRectangle(rP, direction, distance, vacatedP);
      }
      break;

    case sysTrapWinSetActiveWindow: {
      WinHandle winHandle = sys_va_arg(ap, WinHandle);
      WinSetActiveWindow(winHandle);
      }
      break;

    case sysTrapWinSetBackColor: {
      IndexedColorType backColor = sys_va_arg(ap, UInt32);
      IndexedColorType ret = WinSetBackColor(backColor);
      *iret = ret;
      }
      break;

    case sysTrapWinSetBackColorRGB: {
      const RGBColorType *newRgbP = sys_va_arg(ap, RGBColorType *);
      RGBColorType *prevRgbP = sys_va_arg(ap, RGBColorType *);
      WinSetBackColorRGB(newRgbP, prevRgbP);
      }
      break;

    case sysTrapWinSetBounds: {
      WinHandle winHandle = sys_va_arg(ap, WinHandle);
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      WinSetBounds(winHandle, rP);
      }
      break;

    case sysTrapWinSetClip: {
      const RectangleType *rP = sys_va_arg(ap, RectangleType *);
      WinSetClip(rP);
      }
      break;

    case sysTrapWinSetColors: {
      const RGBColorType *newForeColorP = sys_va_arg(ap, RGBColorType *);
      RGBColorType *oldForeColorP = sys_va_arg(ap, RGBColorType *);
      const RGBColorType *newBackColorP = sys_va_arg(ap, RGBColorType *);
      RGBColorType *oldBackColorP = sys_va_arg(ap, RGBColorType *);
      WinSetColors(newForeColorP, oldForeColorP, newBackColorP, oldBackColorP);
      }
      break;

    case sysTrapWinSetDrawMode: {
      WinDrawOperation newMode = sys_va_arg(ap, WinDrawOperation);
      WinDrawOperation ret = WinSetDrawMode(newMode);
      *iret = ret;
      }
      break;

    case sysTrapWinSetDrawWindow: {
      WinHandle winHandle = sys_va_arg(ap, WinHandle);
      WinHandle ret = WinSetDrawWindow(winHandle);
      *pret = (void *)ret;
      }
      break;

    case sysTrapWinSetForeColor: {
      IndexedColorType foreColor = sys_va_arg(ap, UInt32);
      IndexedColorType ret = WinSetForeColor(foreColor);
      *iret = ret;
      }
      break;

    case sysTrapWinSetForeColorRGB: {
      const RGBColorType *newRgbP = sys_va_arg(ap, RGBColorType *);
      RGBColorType *prevRgbP = sys_va_arg(ap, RGBColorType *);
      WinSetForeColorRGB(newRgbP, prevRgbP);
      }
      break;

    case sysTrapWinSetPattern: {
      const CustomPatternType *patternP = sys_va_arg(ap, CustomPatternType *);
      WinSetPattern(patternP);
      }
      break;

    case sysTrapWinSetPatternType: {
      PatternType newPattern = sys_va_arg(ap, PatternType);
      WinSetPatternType(newPattern);
      }
      break;

    case sysTrapWinSetTextColor: {
      IndexedColorType textColor = sys_va_arg(ap, UInt32);
      IndexedColorType ret = WinSetTextColor(textColor);
      *iret = ret;
      }
      break;

    case sysTrapWinSetTextColorRGB: {
      const RGBColorType *newRgbP = sys_va_arg(ap, RGBColorType *);
      RGBColorType *prevRgbP = sys_va_arg(ap, RGBColorType *);
      WinSetTextColorRGB(newRgbP, prevRgbP);
      }
      break;

    case sysTrapWinSetUnderlineMode: {
      UnderlineModeType mode = sys_va_arg(ap, UnderlineModeType);
      UnderlineModeType ret = WinSetUnderlineMode(mode);
      *iret = ret;
      }
      break;

    case sysTrapWinValidateHandle: {
      WinHandle winHandle = sys_va_arg(ap, WinHandle);
      Boolean ret = WinValidateHandle(winHandle);
      *iret = ret;
      }
      break;

    case sysTrapWinWindowToDisplayPt: {
      Coord *extentX = sys_va_arg(ap, Coord *);
      Coord *extentY = sys_va_arg(ap, Coord *);
      WinWindowToDisplayPt(extentX, extentY);
      }
      break;

