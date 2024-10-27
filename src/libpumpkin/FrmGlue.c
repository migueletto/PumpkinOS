#include <PalmOS.h>

FormEventHandlerType *FrmGlueGetEventHandler(const FormType *formP) {
  return formP ? formP->handler : NULL;
}

Boolean FrmGlueGetObjectUsable(const FormType *formP, UInt16 objIndex) {
  FormObjectType obj;
  Boolean usable = false;

  if (formP && objIndex < formP->numObjects) {
    obj = formP->objects[objIndex].object;

    switch (formP->objects[objIndex].objectType) {
      case frmFieldObj:     usable = obj.field->attr.usable;     break;
      case frmControlObj:   usable = obj.control->attr.usable;   break;
      case frmLabelObj:     usable = obj.label->attr.usable;     break;
      case frmListObj:      usable = obj.list->attr.usable;      break;
      case frmTableObj:     usable = obj.table->attr.usable;     break;
      case frmGadgetObj:    usable = obj.gadget->attr.usable;    break;
      case frmScrollBarObj: usable = obj.scrollBar->attr.usable; break;
      default: break;
    }
  }

  return usable;
}

UInt16 FrmGlueGetDefaultButtonID(const FormType *formP) {
  return formP ? formP->defaultButton : 0;
}

void FrmGlueSetDefaultButtonID(FormType *formP, UInt16 defaultButton) {
  if (formP) {
    formP->defaultButton = defaultButton;
  }
}

UInt16 FrmGlueGetHelpID(const FormType *formP) {
  return formP ? formP->helpRscId : 0;
}

void FrmGlueSetHelpID(FormType *formP, UInt16 helpRscId) {
  if (formP) {
    formP->helpRscId = helpRscId;
  }
}

UInt16 FrmGlueGetMenuBarID(const FormType *formP) {
  return formP ? formP->menuRscId : 0;
}

FontID FrmGlueGetLabelFont(const FormType *formP, UInt16 labelID) {
  FormLabelType *label;
  UInt16 index;
  FontID fontID = 0;

  if (formP) {
    if ((index = FrmGetObjectIndex(formP, labelID)) != frmInvalidObjectId) {
      if (formP->objects[index].objectType == frmLabelObj) {
        label = formP->objects[index].object.label;
        fontID = label->fontID;
      }
    }
  }
    
  return fontID;
}

void FrmGlueSetLabelFont(FormType *formP, UInt16 labelID, FontID fontID) {
  FormLabelType *label;
  UInt16 index;

  if (formP) {
    if ((index = FrmGetObjectIndex(formP, labelID)) != frmInvalidObjectId) {
      if (formP->objects[index].objectType == frmLabelObj) {
        label = formP->objects[index].object.label;
        label->fontID = fontID;
      }
    }
  }
}

UInt16 FrmGlueGetObjIDFromObjPtr(void *formObjP, FormObjectKind objKind) {
  FieldType *fld;
  ControlType *ctl;
  ListType *lst;
  TableType *tbl;
  FormLabelType *lbl;
  ScrollBarType *scl;
  FormGadgetType *gad;
  UInt16 id = 0;

  if (formObjP) {
    switch (objKind) {
      case frmFieldObj:
        fld = (FieldType *)formObjP;
        id = fld->id;
        break;
      case frmControlObj:
        ctl = (ControlType *)formObjP;
        id = ctl->id;
        break;
      case frmListObj:
        lst = (ListType *)formObjP;
        id = lst->id;
        break;
      case frmTableObj:
        tbl = (TableType *)formObjP;
        id = tbl->id;
        break;
      case frmBitmapObj:
        break;
      case frmLabelObj:
        lbl = (FormLabelType *)formObjP;
        id = lbl->id;
        break;
      case frmGadgetObj:
        gad = (FormGadgetType *)formObjP;
        id = gad->id;
        break;
      case frmScrollBarObj:
        scl = (ScrollBarType *)formObjP;
        id = scl->id;
        break;
      default:
        break;
    }
  }

  return id;
}
