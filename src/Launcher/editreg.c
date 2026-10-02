#include <PalmOS.h>

#include "RegistryMgr.h"
#include "resource.h"
#include "debug.h"

static UInt16 parseVersion(char *text) {
  int major, minor;
  UInt16 version = 0;

  if (text && sys_sscanf(text, "%u.%u", &major, &minor) == 2) {
    version = major * 10 + minor;
  }

  return version;
}

static void showPage(FormType *frm, UInt16 page, Boolean visible) {
  UInt16 num, index, id, firstId, lastId;

  firstId = 1000 + page * 1000 + 1;
  lastId  = 1000 + page * 1000 + 999;
  num = FrmGetNumberOfObjects(frm);

  for (index = 0; index < num; index++) {
    id = FrmGetObjectId(frm, index);
    if (id > 2000 && id < 10000) {
      if (FrmGetObjectType(frm, index) == frmListObj) continue;
      if (!(id >= firstId && id <= lastId)) {
        if (visible) {
          FrmHideObject(frm, index);
        } else {
          FrmSetUsable(frm, index, false);
        }
      }
    }
  }

  for (index = 0; index < num; index++) {
    id = FrmGetObjectId(frm, index);
    if (id > 2000 && id < 10000) {
      if (FrmGetObjectType(frm, index) == frmListObj) continue;
      if (id >= firstId && id <= lastId) {
        if (visible) {
          FrmShowObject(frm, index);
        } else {
          FrmSetUsable(frm, index, true);
        }
      }
    }
  }
}

static Boolean eventHandler(EventType *event) {
  FormType *frm;
  ControlType *ctl;
  UInt16 *page, index, osversion, density, depth;
  char *text;
  Boolean handled = false;

  switch (event->eType) {
    case ctlSelectEvent:
      frm = FrmGetActiveForm();
      index = FrmGetObjectIndex(frm, regDummyGad);
      page = (UInt16 *)FrmGetGadgetData(frm, index);

      switch (event->data.ctlSelect.controlID) {
        case regPage1Ctl:
          if (*page != 1) {
            *page = 1;
            pumpkin_dirty_region_mode(dirtyRegionBegin);
            showPage(frm, *page, true);
            pumpkin_dirty_region_mode(dirtyRegionEnd);
          }
          break;
        case regPage2Ctl:
          if (*page != 2) {
            *page = 2;
            pumpkin_dirty_region_mode(dirtyRegionBegin);
            showPage(frm, *page, true);
            pumpkin_dirty_region_mode(dirtyRegionEnd);
          }
          break;
      }
      break;
    case popSelectEvent:
      if (event->data.popSelect.listID == osList) {
        if ((text = LstGetSelectionText(event->data.popSelect.listP, event->data.popSelect.selection)) != NULL) {
          if ((osversion = parseVersion(text)) > 0) {
            // set density according to OS version
            if (osversion < 50) {
              density = singleCtl;
            } else {
              density = doubleCtl;
            }

            // set depth according to OS version
            if (osversion < 30) {
              depth = depth1Ctl;
            } else if (osversion < 35) {
              depth = depth4Ctl;
            } else if (osversion < 50) {
              depth = depth8Ctl;
            } else {
              depth = depth16Ctl;
            }

            // update control values
            frm = FrmGetActiveForm();
            index = FrmGetObjectIndex(frm, density);
            ctl = FrmGetObjectPtr(frm, index);
            CtlSetValue(ctl, 1);
            index = FrmGetObjectIndex(frm, depth);
            ctl = FrmGetObjectPtr(frm, index);
            CtlSetValue(ctl, 1);
          }
        }
      }
      break;
    default:
      break;
  }

  return handled;
}

Boolean editRegistry(FormType *frm, UInt32 creator, char *name) {
  RegOsType regOS, *regOsP;
  RegDisplayType regDisp, *regDispP;
  RegHeapType regHeap, *regHeapP;
  RegRunFlagsType regRunFlags, *regRunFlagsP;
  RegFlagsType regFlags, *regFlagsP;
  ListType *lst;
  ControlType *ctl;
  UInt32 regSize;
  UInt16 page, osversion, density, depth, heapSize, heapAlign, index, id, num, i;
  char buf[16], *text;
  Boolean littleEndian, enableSound, fastScreenWrite, armScreenWrite, lenientMemCheck;
  Boolean handspringExt, r = false;

  FrmSetTitle(frm, name);

  // set 1st page
  index = FrmGetObjectIndex(frm, regPage1Ctl);
  ctl = FrmGetObjectPtr(frm, index);
  CtlSetValue(ctl, 1);
  index = FrmGetObjectIndex(frm, regDummyGad);
  page = 1;
  FrmSetGadgetData(frm, index, &page);
  showPage(frm, page, false);

  regOsP = pumpkin_reg_get(creator, regOsID, &regSize);
  osversion = regOsP ? regOsP->version : pumpkin_get_default_osversion();

  regDispP = pumpkin_reg_get(creator, regDisplayID, &regSize);
  density = regDispP ? regDispP->density : pumpkin_get_density();
  depth = regDispP ? (regDispP->depth & 0x7FFF) : pumpkin_get_depth();
  littleEndian = regDispP ? ((regDispP->depth & 0x8000) == 0x8000) : false;

  regHeapP = pumpkin_reg_get(creator, regHeapID, &regSize);
  heapSize = regHeapP ? regHeapP->heapSize : 8;
  heapAlign = regHeapP ? regHeapP->heapAlign : 0;

  regRunFlagsP = pumpkin_reg_get(creator, regRunFlagsID, &regSize);
  enableSound = regRunFlagsP ? (regRunFlagsP->flags & regRunFlagSound) == regRunFlagSound : 0;

  regFlagsP = pumpkin_reg_get(creator, regFlagsID, &regSize);
  fastScreenWrite = regFlagsP ? regFlagsP->flags & regFlagFastScreenWrite : false;
  armScreenWrite  = regFlagsP ? regFlagsP->flags & regFlagARMScreenWrite  : false;
  lenientMemCheck = regFlagsP ? regFlagsP->flags & regFlagLenientMemCheck : false;
  handspringExt   = regFlagsP ? regFlagsP->flags & regFlagHandspringExt : false;

  // set OS version
  index = FrmGetObjectIndex(frm, osList);
  lst = FrmGetObjectPtr(frm, index);
  num = LstGetNumberOfItems(lst);
  StrNPrintF(buf, sizeof(buf)-1, "%u.%u", osversion / 10, osversion % 10);
  for (i = 0; i < num; i++) {
    text = LstGetSelectionText(lst, i);
    if (!StrCompare(buf, text)) break;
  }
  if (i == num) i = num - 1;
  LstSetSelection(lst, i);
  index = FrmGetObjectIndex(frm, osCtl);
  ctl = FrmGetObjectPtr(frm, index);
  CtlSetLabel(ctl, LstGetSelectionText(lst, i));

  // set density
  index = FrmGetObjectIndex(frm, density == kDensityLow ? singleCtl : doubleCtl);
  ctl = FrmGetObjectPtr(frm, index);
  CtlSetValue(ctl, 1);

  // set depth
  switch (depth) {
    case  1: id = depth1Ctl;  break;
    case  2: id = depth2Ctl;  break;
    case  4: id = depth4Ctl;  break;
    case  8: id = depth8Ctl;  break;
    case 16: id = depth16Ctl; break;
    default: id = depth16Ctl; break;
  }
  if (id == depth16Ctl && littleEndian) {
    id = depth16leCtl;
  }
  index = FrmGetObjectIndex(frm, id);
  ctl = FrmGetObjectPtr(frm, index);
  CtlSetValue(ctl, 1);

  // set heap size
  switch (heapSize) {
    case  8: id = heap8Ctl;  break;
    case 16: id = heap16Ctl; break;
    case 32: id = heap32Ctl; break;
    case 64: id = heap64Ctl; break;
    default: id = heap8Ctl;  break;
  }
  index = FrmGetObjectIndex(frm, id);
  ctl = FrmGetObjectPtr(frm, index);
  CtlSetValue(ctl, 1);

  // set sound
  index = FrmGetObjectIndex(frm, enableSoundCtl);
  ctl = FrmGetObjectPtr(frm, index);
  CtlSetValue(ctl, enableSound);

  // set fastScreenWrite
  index = FrmGetObjectIndex(frm, fastScreenWriteCtl);
  ctl = FrmGetObjectPtr(frm, index);
  CtlSetValue(ctl, fastScreenWrite);

  // set armScreenWrite
  index = FrmGetObjectIndex(frm, armScreenWriteCtl);
  ctl = FrmGetObjectPtr(frm, index);
  CtlSetValue(ctl, armScreenWrite);

  // set lenientMemCheck
  index = FrmGetObjectIndex(frm, lenientMemCheckCtl);
  ctl = FrmGetObjectPtr(frm, index);
  CtlSetValue(ctl, lenientMemCheck);

  // set handspringExt
  index = FrmGetObjectIndex(frm, handspringExtCtl);
  ctl = FrmGetObjectPtr(frm, index);
  CtlSetValue(ctl, handspringExt);

  FrmSetEventHandler(frm, eventHandler);
  if (FrmDoDialog(frm) == regOkBtn) {
    // update OS version
    index = FrmGetObjectIndex(frm, osList);
    lst = FrmGetObjectPtr(frm, index);
    i = LstGetSelection(lst);
    if ((text = LstGetSelectionText(lst, i)) != NULL) {
      if ((regOS.version = parseVersion(text)) > 0) {
        pumpkin_reg_set(creator, regOsID, &regOS, sizeof(RegOsType));
      }
    }

    // update density
    index = FrmGetObjectIndex(frm, singleCtl);
    ctl = FrmGetObjectPtr(frm, index);
    regDisp.density = CtlGetValue(ctl) ? kDensityLow : kDensityDouble; 

    // update depth
    for (i = depth1Ctl; i <= depth16Ctl; i++) {
      index = FrmGetObjectIndex(frm, i);
      ctl = FrmGetObjectPtr(frm, index);
      if (CtlGetValue(ctl)) break;
    }
    switch (i) {
      case depth1Ctl:    regDisp.depth =  1; break;
      case depth2Ctl:    regDisp.depth =  2; break;
      case depth4Ctl:    regDisp.depth =  4; break;
      case depth8Ctl:    regDisp.depth =  8; break;
      case depth16Ctl:   regDisp.depth = 16; break;
      case depth16leCtl: regDisp.depth = 16 | 0x8000; break;
      default:           regDisp.depth = 16; break;
    }

    pumpkin_reg_set(creator, regDisplayID, &regDisp, sizeof(RegDisplayType));

    // update heap size
    for (i = heap8Ctl; i <= heap64Ctl; i++) {
      index = FrmGetObjectIndex(frm, i);
      ctl = FrmGetObjectPtr(frm, index);
      if (CtlGetValue(ctl)) break;
    }
    switch (i) {
      case heap8Ctl:    regHeap.heapSize =  8; break;
      case heap16Ctl:   regHeap.heapSize = 16; break;
      case heap32Ctl:   regHeap.heapSize = 32; break;
      case heap64Ctl:   regHeap.heapSize = 64; break;
      default:          regHeap.heapSize =  8; break;
    }

    regHeap.heapAlign = heapAlign;
    pumpkin_reg_set(creator, regHeapID, &regHeap, sizeof(RegHeapType));

    // update sound
    regRunFlags.flags = regRunFlagsP ? regRunFlagsP->flags : 0;
    index = FrmGetObjectIndex(frm, enableSoundCtl);
    ctl = FrmGetObjectPtr(frm, index);
    if (CtlGetValue(ctl)) {
      regRunFlags.flags |= regRunFlagSound;
    } else {
      regRunFlags.flags &= ~regRunFlagSound;
    }
    pumpkin_reg_set(creator, regRunFlagsID, &regRunFlags, sizeof(RegRunFlagsType));

    // update fastScreenWrite, armScreenWrite and lenientMemCheck
    regFlags.flags = regFlagsP ? regFlagsP->flags : 0;

    index = FrmGetObjectIndex(frm, fastScreenWriteCtl);
    ctl = FrmGetObjectPtr(frm, index);
    if (CtlGetValue(ctl)) {
      regFlags.flags |= regFlagFastScreenWrite;
    } else {
      regFlags.flags &= ~regFlagFastScreenWrite;
    }

    index = FrmGetObjectIndex(frm, armScreenWriteCtl);
    ctl = FrmGetObjectPtr(frm, index);
    if (CtlGetValue(ctl)) {
      regFlags.flags |= regFlagARMScreenWrite;
    } else {
      regFlags.flags &= ~regFlagARMScreenWrite;
    }

    index = FrmGetObjectIndex(frm, lenientMemCheckCtl);
    ctl = FrmGetObjectPtr(frm, index);
    if (CtlGetValue(ctl)) {
      regFlags.flags |= regFlagLenientMemCheck;
    } else {
      regFlags.flags &= ~regFlagLenientMemCheck;
    }

    index = FrmGetObjectIndex(frm, handspringExtCtl);
    ctl = FrmGetObjectPtr(frm, index);
    if (CtlGetValue(ctl)) {
      regFlags.flags |= regFlagHandspringExt;
    } else {
      regFlags.flags &= ~regFlagHandspringExt;
    }

    pumpkin_reg_set(creator, regFlagsID, &regFlags, sizeof(RegFlagsType));

    r = true;
  }

  return r;
}
