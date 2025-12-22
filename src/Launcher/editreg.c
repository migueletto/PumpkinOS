#include <PalmOS.h>

#include "RegistryMgr.h"
#include "resource.h"

static UInt16 parseVersion(char *text) {
  int major, minor;
  UInt16 version = 0;

  if (text && sys_sscanf(text, "%u.%u", &major, &minor) == 2) {
    version = major * 10 + minor;
  }

  return version;
}

static Boolean eventHandler(EventType *event) {
  FormType *frm;
  ControlType *ctl;
  UInt16 index, osversion, density, depth;
  char *text;

  switch (event->eType) {
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

  return false;
}

Boolean editRegistry(FormType *frm, UInt32 creator, char *name) {
  RegOsType regOS, *regOsP;
  RegDisplayType regDisp, *regDispP;
  RegDisplayEndianType regEnd, *regEndP;
  RegSoundType regSnd, *regSndP;
  ListType *lst;
  ControlType *ctl;
  UInt32 regSize;
  UInt16 osversion, density, depth, littleEndian, enableSound, index, id, num, i;
  char buf[16], *text;
  Boolean r = false;

  FrmSetTitle(frm, name);

  regOsP = pumpkin_reg_get(creator, regOsID, &regSize);
  osversion = regOsP ? regOsP->version : pumpkin_get_default_osversion();

  regDispP = pumpkin_reg_get(creator, regDisplayID, &regSize);
  density = regDispP ? regDispP->density : pumpkin_get_density();
  depth = regDispP ? regDispP->depth : pumpkin_get_depth();

  regEndP = pumpkin_reg_get(creator, regEndianID, &regSize);
  littleEndian = regEndP ? regEndP->littleEndian : 0;

  regSndP = pumpkin_reg_get(creator, regSoundID, &regSize);
  enableSound = regSndP ? regSndP->enableSound : 0;

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

  // set sound
  index = FrmGetObjectIndex(frm, enableSoundCtl);
  ctl = FrmGetObjectPtr(frm, index);
  CtlSetValue(ctl, enableSound);

  FrmSetEventHandler(frm, eventHandler);
  if (FrmDoDialog(frm) == okBtn) {
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
    regEnd.littleEndian = 0;
    switch (i) {
      case depth1Ctl:    regDisp.depth =  1; break;
      case depth2Ctl:    regDisp.depth =  2; break;
      case depth4Ctl:    regDisp.depth =  4; break;
      case depth8Ctl:    regDisp.depth =  8; break;
      case depth16Ctl:   regDisp.depth = 16; break;
      case depth16leCtl: regDisp.depth = 16; regEnd.littleEndian = 1; break;
      default:           regDisp.depth = 16; break;
    }

    pumpkin_reg_set(creator, regDisplayID, &regDisp, sizeof(RegDisplayType));
    pumpkin_reg_set(creator, regEndianID, &regEnd, sizeof(RegDisplayEndianType));

    // update sound
    index = FrmGetObjectIndex(frm, enableSoundCtl);
    ctl = FrmGetObjectPtr(frm, index);
    regSnd.enableSound = CtlGetValue(ctl) ? 1 : 0; 
    pumpkin_reg_set(creator, regSoundID, &regSnd, sizeof(RegSoundType));

    r = true;
  }

  return r;
}
