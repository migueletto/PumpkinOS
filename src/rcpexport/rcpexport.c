#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#include "rgb.h"
#include "bytes.h"
#include "pumpkin.h"
#include "debug.h"

static void emitn(FileRef fileRef, char *buf, int len) {
  VFSFileWrite(fileRef, len, buf, NULL);
}

static void emit(FileRef fileRef, char *buf) {
  emitn(fileRef, buf, StrLen(buf));
}

static void emitstr(FileRef fileRef, char *buf) {
  char aux[8];
  uint8_t c;
  int i;

  emitn(fileRef, "\"", 1);
  if (buf) {
    for (i = 0; buf[i]; i++) {
      c = buf[i];
      switch (c) {
        case '\t':
          emit(fileRef, "\\t");
          break;
        case '\n':
          emit(fileRef, "\\n");
          break;
        case '\r':
          break;
        case '"':
          emit(fileRef, "\\\"");
          break;
        default:
          if (c < 32 || c > 127) {
            StrPrintF(aux, "\\x%02x", c);
            emit(fileRef, aux);
          } else {
            emitn(fileRef, &buf[i], 1);
          }
          break;
      }
    }
  }
  emitn(fileRef, "\"", 1);
}

static void emitnl(FileRef fileRef) {
  emit(fileRef, "\n");
}

static void emitField(FileRef fileRef, char *buf, FieldType *field) {
  StrPrintF(buf, "  FIELD ID %d AT (%d %d %d %d) %s", field->id,
    field->rect.topLeft.x, field->rect.topLeft.y, field->rect.extent.x, field->rect.extent.y,
    field->attr.usable ? "USABLE" : "NONUSABLE");
  emit(fileRef, buf);

  switch (field->attr.justification) {
    case leftAlign:  emit(fileRef, " LEFTALIGN");  break;
    case rightAlign: emit(fileRef, " RIGHTALIGN"); break;
  }

  emit(fileRef, field->attr.singleLine ? " SINGLELINE" : " MULTIPLELINES");
  if (field->attr.dynamicSize) emit(fileRef, " DYNAMICSIZE");
  if (field->attr.autoShift) emit(fileRef, " AUTOSHIFT");
  if (field->attr.numeric) emit(fileRef, " NUMERIC");
  if (field->attr.hasScrollBar) emit(fileRef, " HASSCROLLBAR");

  StrPrintF(buf, " MAXCHARS %d", field->maxChars);
  emit(fileRef, buf);
  emitnl(fileRef);
}

static void emitControl(FileRef fileRef, char *buf, char *type, FormObjectType *obj) {
  emit(fileRef, "  ");
  emit(fileRef, type);
  emit(fileRef, " ");
  emitstr(fileRef, obj->control->text);
  StrPrintF(buf, " ID %d AT (%d %d %d %d) %s %s", obj->control->id,
    obj->control->bounds.topLeft.x, obj->control->bounds.topLeft.y, obj->control->bounds.extent.x, obj->control->bounds.extent.y,
    obj->control->attr.usable ? "USABLE" : "NONUSABLE", obj->control->attr.leftAnchor ? "LEFTANCHOR" : "RIGHTANCHOR");
  emit(fileRef, buf);
}

static void emitGraphical(FileRef fileRef, char *buf, FormObjectType *obj) {
  if (obj->control->attr.graphical) {
    emit(fileRef, " GRAPHICAL");
    if (obj->control->bitmapID) {
      StrPrintF(buf, " BITMAPID %d", obj->control->bitmapID);
      emit(fileRef, buf);
    }
    if (obj->control->selectedBitmapID) {
      StrPrintF(buf, " SELECTEDBITMAPID %d", obj->control->selectedBitmapID);
      emit(fileRef, buf);
    }
  }
}

static void emitFrame(FileRef fileRef, FormObjectType *obj) {
  switch (obj->control->attr.frame) {
    case noButtonFrame:        emit(fileRef, " NOFRAME");   break;
    case standardButtonFrame:  emit(fileRef, " FRAME");     break;
    case boldButtonFrame:      emit(fileRef, " BOLDFRAME"); break;
    case rectangleButtonFrame: emit(fileRef, " RECTFRAME"); break;
  }
}

static void emitGroup(FileRef fileRef, char *buf, FormObjectType *obj) {
  if (obj->control->group) {
    StrPrintF(buf, " GROUP %d", obj->control->group);
    emit(fileRef, buf);
  }
}

static void emitFont(FileRef fileRef, char *buf, FormObjectType *obj) {
  StrPrintF(buf, " FONT %d", obj->control->font);
  emit(fileRef, buf);
}

static void saveData(MemHandle h, void *p, char *type, UInt16 resID, FileRef fileRef) {
  char buf[256], name[64];
  FileRef fileRef2;
  UInt32 size;

  debug(DEBUG_ERROR, "RCP", "unknown resource type %s id %d", type, resID);
  StrPrintF(name, "%s_%d.dat", type, resID);
  StrPrintF(buf, "DATA \"%s\" ID %d \"%s\"\n\n", type, resID, name);
  emit(fileRef, buf);
  size = MemHandleSize(h);
  VFSFileCreate(1, name);
  VFSFileOpen(1, name, vfsModeWrite, &fileRef2);
  emitn(fileRef2, p, size);
  VFSFileClose(fileRef2);
}

static void export(MemHandle h, DmResType resType, DmResID resID, FileRef fileRef) {
  char buf[256], attr[64];
  char st[8], st2[8], name[64];
  AlertTemplateType *alert;
  BitmapType *bitmap;
  Coord width, height;
  UInt32 depth, density, transparentValue;
  UInt8 red, green, blue;
  Boolean center, transp;
  FontType *font;
  FontTypeV2 *font2;
  MenuBarType *menuBar;
  MenuItemType *menuItem;
  FormType *form;
  FormObjectType obj;
  SliderControlType *slider;
  UInt16 version;
  UInt16 *u16, d, i, j, k, num, max, featNum;
  UInt32 *u32, size, creator, featVal;
  UInt8 *u8;
  char *prefix, *str;
  FileRef fileRef2;
  void *p;

  if ((p = MemHandleLock(h)) != NULL) {
    pumpkin_id2s(resType, st);
    debug(DEBUG_INFO, "RCP", "exporting resource %s %d", st, resID);

    switch (resType) {
      case alertRscType:
        /*
        ALERT ID <AlertResrouceId.n>
          [HELPID <HelpId.n>]
          [DEFAULTBUTTON <ButtonIdx.n>]
          [INFORMATION] [CONFIRMATION] [WARNING] [ERROR]
          [LOCALE <LocaleName.s>]
        BEGIN
          TITLE <Title.s>
          MESSAGE <Message.ss>
          BUTTONS <Button.s> ... <Button.s>
        END
        */
        alert = (AlertTemplateType *)p;
        StrPrintF(buf, "ALERT ID %d", resID);
        emit(fileRef, buf);
        emitnl(fileRef);
        if (alert->helpRscID) {
          StrPrintF(buf, "HELPID %d", alert->helpRscID);
          emit(fileRef, buf);
          emitnl(fileRef);
        }
        if (alert->defaultButton) {
          StrPrintF(buf, "DEFAULTBUTTON %d", alert->defaultButton);
          emit(fileRef, buf);
          emitnl(fileRef);
        }
        switch (alert->alertType) {
          case informationAlert:  emit(fileRef, "INFORMATION\n"); break;
          case confirmationAlert: emit(fileRef, "CONFIRMATION\n"); break;
          case warningAlert:      emit(fileRef, "WARNING\n"); break;
          case errorAlert:        emit(fileRef, "ERROR\n"); break;
        }
        emit(fileRef, "BEGIN\n  TITLE ");
        emitstr(fileRef, alert->title);
        emit(fileRef, "\n  MESSAGE ");
        emitstr(fileRef, alert->message);
        emit(fileRef, "\n  BUTTONS");
        for (i = 0; i < alert->numButtons; i++) {
          emit(fileRef, " ");
          emitstr(fileRef, alert->button[i]);
        }
        emit(fileRef, "\nEND\n\n");
        break;
      case formRscType:
        /*
        FORM ID <FormResourceId.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
          [FRAME] [NOFRAME]
          [MODAL]
          [SAVEBEHIND] [NOSAVEBEHIND]
          [USABLE]
          [HELPID <HelpId.n>]
          [DEFAULTBTNID <BtnId.n>]
          [MENUID <MenuId.n>]
          [LOCALE <LocaleName.s>]
        BEGIN
          <OBJECTS>
        END
        */
        center = FrmGetCenterDialogs();
        FrmCenterDialogs(false);
        form = FrmInitForm(resID);
        FrmCenterDialogs(center);
        StrPrintF(buf, "FORM ID %d AT (%d %d %d %d)\n", resID, form->window.windowBounds.topLeft.x, form->window.windowBounds.topLeft.y, form->window.windowBounds.extent.x, form->window.windowBounds.extent.x);
        emit(fileRef, buf);
        if (form->window.frameType.word == 0) {
          StrCopy(buf, "  NOFRAME\n");
          emit(fileRef, buf);
        }
        if (form->window.windowFlags.modal) {
          StrCopy(buf, "  MODAL\n");
          emit(fileRef, buf);
        }
        if (form->attr.saveBehind) {
          StrCopy(buf, "  SAVEBEHIND\n");
          emit(fileRef, buf);
        }
        if (form->attr.usable) {
          StrCopy(buf, "  USABLE\n");
          emit(fileRef, buf);
        }
        if (form->helpRscId) {
          StrPrintF(buf, "  HELPID %d\n", form->helpRscId);
          emit(fileRef, buf);
        }
        if (form->defaultButton) {
          StrPrintF(buf, "  DEFAULTBTNID %d\n", form->defaultButton);
          emit(fileRef, buf);
        }
        if (form->menuRscId) {
          StrPrintF(buf, "  MENUID %d\n", form->menuRscId);
          emit(fileRef, buf);
        }
        StrCopy(buf, "BEGIN\n");
        emit(fileRef, buf);
        for (i = 0; i < form->numObjects; i++) {
          obj = form->objects[i].object;
          switch (form->objects[i].objectType) {
            case frmFieldObj:
              // FIELD ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
              // [USABLE] [NONUSABLE] [DISABLED] [LEFTALIGN] [RIGHTALIGN]
              // [FONT <FontId.n>] [EDITABLE] [NONEDITABLE] [UNDERLINED]
              // [SINGLELINE] [MULTIPLELINES] [DYNAMICSIZE] [MAXCHARS <MaxChars.n>]
              // [AUTOSHIFT] [NUMERIC] [HASSCROLLBAR]
              emitField(fileRef, buf, obj.field);
              break;
            case frmControlObj:
              switch (obj.control->style) {
                case buttonCtl:
                  // BUTTON <Label.ss> ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
                  // [USABLE] [NONUSABLE] [DISABLED] [LEFTANCHOR] [RIGHTANCHOR]
                  // [FRAME] [NOFRAME] [BOLDFRAME] [RECTFRAME] [FONT <FontId.n>]
                  // [GRAPHICAL] [BITMAPID <BitmapId.n>] [SELECTEDBITMAPID <BitmapId.n>]
                  emitControl(fileRef, buf, "BUTTON", &obj);
                  emitFrame(fileRef, &obj);
                  emitFont(fileRef, buf, &obj);
                  emitGraphical(fileRef, buf, &obj);
                  emit(fileRef, "\n");
                  break;
                case pushButtonCtl:
                  // PUSHBUTTON <Label.ss> ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
                  // [USABLE] [NONUSABLE] [DISABLED] [LEFTANCHOR] [RIGHTANCHOR]
                  // [FONT <FontId>] [GROUP <GroupId.n>]
                  // [GRAPHICAL] [BITMAPID <BitmapId.n>] [SELECTEDBITMAPID <BitmapId.n>] 
                  emitControl(fileRef, buf, "PUSHBUTTON", &obj);
                  emitFont(fileRef, buf, &obj);
                  emitGroup(fileRef, buf, &obj);
                  emitGraphical(fileRef, buf, &obj);
                  emit(fileRef, "\n");
                  break;
                case checkboxCtl:
                  // CHECKBOX <Label.ss> ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
                  // [USABLE] [NONUSABLE] [DISABLED] [LEFTANCHOR] [RIGHTANCHOR]
                  // [FONT <FontId.n>] [GROUP <GroupId.n>] [CHECKED]
                  emitControl(fileRef, buf, "CHECKBOX", &obj);
                  emitFont(fileRef, buf, &obj);
                  emitGroup(fileRef, buf, &obj);
                  if (obj.control->attr.on) emit(fileRef, " CHECKED");
                  emit(fileRef, "\n");
                  break;
                case popupTriggerCtl:
                  // POPUPTRIGGER <Label.ss> ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
                  // [USABLE] [NONUSABLE] [DISABLED] [LEFTANCHOR] [RIGHTANCHOR]
                  // [FONT <FontId.n>]
                  // [GRAPHICAL] [BITMAPID <BitmapId.n>] [SELECTEDBITMAPID <BitmapId.n>]
                  emitControl(fileRef, buf, "POPUPTRIGGER", &obj);
                  emitFont(fileRef, buf, &obj);
                  emitGraphical(fileRef, buf, &obj);
                  emit(fileRef, "\n");
                  break;
                case selectorTriggerCtl:
                  // SELECTORTRIGGER <Label.ss> ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
                  // [USABLE] [NONUSABLE] [DISABLED] [LEFTANCHOR] [RIGHTANCHOR]
                  // [FONT <FontId.n>]
                  // [GRAPHICAL] [BITMAPID <BitmapId.n>] [SELECTEDBITMAPID <BitmapId.n>]
                  emitControl(fileRef, buf, "SELECTORTRIGGER", &obj);
                  emitFont(fileRef, buf, &obj);
                  emitGraphical(fileRef, buf, &obj);
                  emit(fileRef, "\n");
                  break;
                case repeatingButtonCtl:
                  // REPEATBUTTON <Label.ss> ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
                  // [USABLE] [NONUSABLE] [DISABLED] [LEFTANCHOR] [RIGHTANCHOR]
                  // [FRAME] [NOFRAME] [BOLDFRAME] [RECTFRAME] [FONT <FontId.n>]
                  // [GRAPHICAL] [BITMAPID <BitmapId.n>] [SELECTEDBITMAPID <BitmapId.n>]
                  emitControl(fileRef, buf, "REPEATBUTTON", &obj);
                  emitFrame(fileRef, &obj);
                  emitFont(fileRef, buf, &obj);
                  emitGraphical(fileRef, buf, &obj);
                  emit(fileRef, "\n");
                  break;
                case sliderCtl:
                case feedbackSliderCtl:
                  // SLIDER ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
                  // [USABLE] [NONUSABLE] [DISABLED] [VERTICAL] [FEEDBACK]
                  // [THUMBID <BitmapId.n>] [BACKGROUNDID <BitmapId.n>]
                  // [VALUE <Value.n>] [MIN <MinValue.n>] [MAX <MaxValue.n>] [PAGESIZE <PageSize.n>]
                  slider = (SliderControlType *)obj.control;
                  StrPrintF(buf, "  SLIDER ID %d AT (%d %d %d %d) %s", obj.control->id,
                    obj.control->bounds.topLeft.x, obj.control->bounds.topLeft.y, obj.control->bounds.extent.x, obj.control->bounds.extent.y,
                    obj.control->attr.usable ? "USABLE" : "NONUSABLE");
                  emit(fileRef, buf);
                  if (obj.control->attr.vertical) emit(fileRef, " VERTICAL");
                  if (obj.control->style == feedbackSliderCtl) emit(fileRef, " FEEDBACK");
                  if (slider->thumbID) {
                    StrPrintF(buf, " THUMBID %d", slider->thumbID);
                    emit(fileRef, buf);
                  }
                  if (slider->backgroundID) {
                    StrPrintF(buf, " BACKGROUNDID %d", slider->backgroundID);
                    emit(fileRef, buf);
                  }
                  StrPrintF(buf, " VALUE %d MIN %d MAX %d PAGESIZE %d\n", slider->value, slider->minValue, slider->maxValue, slider->pageSize);
                  emit(fileRef, buf);
                  break;
                default:
                  break;
              }
              break;
            case frmTitleObj:
              emit(fileRef, "  TITLE ");
              emitstr(fileRef, obj.title->text);
              emitnl(fileRef);
              break;
            case frmLabelObj:
              // LABEL <Label.ss> ID <Id.n> AT (<Left.p> <Top.p>)
              // [USABLE] [NONUSABLE] [FONT <FontId.n>] 
              emit(fileRef, "  LABEL ");
              emitstr(fileRef, obj.label->text);
              StrPrintF(buf, " ID %d AT (%d %d) %s FONT %d\n",
                obj.label->id, obj.label->pos.x, obj.label->pos.y, obj.label->attr.usable ? "USABLE" : "NONUSABLE", obj.label->fontID);
              emit(fileRef, buf);
              break;
            case frmBitmapObj:
              // FORMBITMAP AT (<Left.p> <Top.p>)
              // [BITMAP <BitmapId.n>] [USABLE] [NONUSABLE]
              StrPrintF(buf, "  FORMBITMAP AT (%d %d) BITMAP %d %s\n",
                obj.bitmap->pos.x, obj.bitmap->pos.y, obj.bitmap->rscID, obj.bitmap->attr.usable ? "USABLE" : "NONUSABLE");
              emit(fileRef, buf);
              break;
            case frmGadgetObj:
              // GADGET ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
              // [USABLE] [NONUSABLE] [NONEXTENDED] [EXTENDED] 
              StrPrintF(buf, "  GADGET ID %d AT (%d %d %d %d) %s %s\n",
                obj.gadget->id, obj.gadget->rect.topLeft.x, obj.gadget->rect.topLeft.y, obj.gadget->rect.extent.x, obj.gadget->rect.extent.y,
                obj.gadget->attr.usable ? "USABLE" : "NONUSABLE", obj.gadget->attr.extended ? "EXTENDED" : "NONEXTENDED");
              emit(fileRef, buf);
              break;
            case frmListObj:
              // LIST <Item.s> ... <Item.s>
              // ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
              // [USABLE] [NONUSABLE] [DISABLED] [VISIBLEITEMS <NumVisItems.n>]
              // [FONT <FontId.n>] [SEARCH] 
              StrCopy(buf, "  LIST");
              emit(fileRef, buf);
              for (j = 0; j < obj.list->numItems; j++) {
                emit(fileRef, " ");
                if (obj.list->itemsText[j] && obj.list->itemsText[j][0]) {
                  emitstr(fileRef, obj.list->itemsText[j]);
                } else {
                  emitstr(fileRef, "");
                }
              }
              StrPrintF(buf, " ID %d AT (%d %d %d %d) %s%s", obj.list->id,
                obj.list->bounds.topLeft.x, obj.list->bounds.topLeft.y, obj.list->bounds.extent.x, obj.list->bounds.extent.y,
                obj.list->attr.usable ? "USABLE" : "NONUSABLE", obj.list->attr.search ? " SEARCH" : "");
              emit(fileRef, buf);
              if (obj.list->visibleItems) {
                StrPrintF(buf, " VISIBLEITEMS %d", obj.list->visibleItems);
                emit(fileRef, buf);
              }
              StrPrintF(buf, " FONT %d\n", obj.list->font);
              emit(fileRef, buf);
              break;
            case frmTableObj:
              // TABLE ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
              // [ROWS <NumRows.n>] [COLUMNS <NumCols.n>]
              // [COLUMNWIDTHS <Col1Width.n> ... <ColNWidth.n>] 
              StrPrintF(buf, "  TABLE ID %d AT (%d %d %d %d) ROWS %d COLUMNS %d COLUMNWIDTHS",
                obj.table->id, obj.table->bounds.topLeft.x, obj.table->bounds.topLeft.y, obj.table->bounds.extent.x, obj.table->bounds.extent.y,
                obj.table->numRows, obj.table->numColumns);
              emit(fileRef, buf);
              for (j = 0; j < obj.table->numColumns; j++) {
                StrPrintF(buf, " %d", obj.table->columnAttrs[j].width);
                emit(fileRef, buf);
              }
              emitnl(fileRef);
              break;
            case frmScrollBarObj:
              // SCROLLBAR ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
              // [USABLE] [NONUSABLE] [VALUE <Value.n>] [MIN <MinValue.n>]
              // [MAX <MaxValue.n>] [PAGESIZE <PageSize.n>]
              StrPrintF(buf, "  SCROLLBAR ID %d AT (%d %d %d %d) %s VALUE %d MIN %d MAX %d PAGESIZE %d\n", obj.scrollBar->id,
                obj.scrollBar->bounds.topLeft.x, obj.scrollBar->bounds.topLeft.y, obj.scrollBar->bounds.extent.x, obj.scrollBar->bounds.extent.y,
                obj.scrollBar->attr.usable ? "USABLE" : "NONUSABLE", obj.scrollBar->value, obj.scrollBar->minValue,
                obj.scrollBar->maxValue, obj.scrollBar->pageSize);
              emit(fileRef, buf);
              break;
            case frmPopupObj:
              // POPUPLIST ID <ControlId.n> <ListId.n> 
              StrPrintF(buf, "  POPUPLIST ID %d %d\n", obj.popup->controlID, obj.popup->listID);
              emit(fileRef, buf);
              break;
            case frmGraffitiStateObj:
              // GRAFFITISTATEINDICATOR AT (<Left.p> <Top.p>)
              StrPrintF(buf, "  GRAFFITISTATEINDICATOR AT (%d %d)\n", obj.grfState->pos.x, obj.grfState->pos.y);
              emit(fileRef, buf);
            default:
              break;
          }
        }
        StrCopy(buf, "END\n\n");
        emit(fileRef, buf);
        FrmDeleteForm(form);
        break;
      case MenuRscType:
        /*
        MENU ID <MenuResourceId.n>
        [LOCALE <LocaleName.s>]
        BEGIN
          <PULLDOWNS>
        END

        PULLDOWN <PullDownTitle.s>
        BEGIN
          <MENUITEMS>
        END

        MENUITEM <MenuItem.s> ID <MenuItemId.n> [AccelChar.c]
        MENUITEM SEPARATOR [ID <MenuItemId.n>]
        */
        menuBar = (MenuBarType *)p;
        StrPrintF(buf, "MENU ID %d\nBEGIN\n", resID);
        emit(fileRef, buf);
        for (i = 0; i < menuBar->numMenus; i++) {
          emit(fileRef, "  PULLDOWN ");
          emitstr(fileRef, menuBar->menus[i].title);
          emitnl(fileRef);
          emit(fileRef, "  BEGIN");
          emitnl(fileRef);
          for (j = 0; j < menuBar->menus[i].numItems; j++) {
            menuItem = &menuBar->menus[i].items[j];
            if (menuItem->itemStr) {
              if (menuItem->itemStr[0] == '-') {
                emit(fileRef, "    MENUITEM SEPARATOR");
              } else {
                emit(fileRef, "    MENUITEM ");
                emitstr(fileRef, menuItem->itemStr);
                StrPrintF(name, " \"%c\"", menuItem->command);
                StrPrintF(buf, " ID %d%s", menuItem->id, menuItem->command ? name : "");
                emit(fileRef, buf);
              }
              emitnl(fileRef);
            }
          }
          StrCopy(buf, "  END\n");
          emit(fileRef, buf);
        }
        StrCopy(buf, "END\n\n");
        emit(fileRef, buf);
        break;
      case fontRscType:
        /*
        FONT ID <FontResourceId.n> 
        [LOCALE <LocaleName.s>] FONTID <FontId.n> <FontFileName.s>

        fontType 36864
        ascent 5
        descent 1
        */
        font = (FontType *)p;
        if (font->v == 1) {
          StrPrintF(name, "fontv1_%d.txt", resID);
          StrPrintF(buf, "FONT ID %d FONTID %d \"%s\"\n\n", resID, 128 + (resID % 10), name);
          emit(fileRef, buf);
          VFSFileCreate(1, name);
          VFSFileOpen(1, name, vfsModeWrite, &fileRef2);
          StrPrintF(buf, "fontType 36864\nascent %d\ndescent %d\n", font->ascent, font->descent);
          emit(fileRef2, buf);
          for (i = font->firstChar; i <= font->lastChar; i++) {
            if (font->width[i - font->firstChar] > 0) {
              StrPrintF(buf, "\nGLYPH %d\n", i);
              emit(fileRef2, buf);
              for (j = 0; j < font->fRectHeight; j++) {
                for (k = 0; k < font->width[i - font->firstChar]; k++) {
                  buf[k] = BmpGetPixel(font->bmp, font->column[i - font->firstChar] + k, j) ? '#' : '-';
                }
                buf[k++] = '\n';
                buf[k] = 0;
                emit(fileRef2, buf);
              }
            }
          }
          VFSFileClose(fileRef2);
        }
        break;
      case fontExtRscType:
        /*
        FONTFAMILY ID <ResId.n> FONTID <FontId.n> [LOCALE <LocaleName.s>]
        BEGIN
            FONT <FontFileName.s> DENSITY <Density.n>
            ...
        END
        */
        font2 = (FontTypeV2 *)p;
        if (font2->v == 2) {
          StrPrintF(buf, "FONTFAMILY ID %d FONTID %d\nBEGIN\n", resID, 128 + (resID % 10));
          emit(fileRef, buf);
          for (d = 0; d < font2->densityCount; d++) {
            StrPrintF(name, "fontv2_%d_%d.txt", resID, font2->densities[d].density);
            StrPrintF(buf, "  FONT \"%s\" DENSITY %d\n", name, font2->densities[d].density);
            emit(fileRef, buf);
            VFSFileCreate(1, name);
            VFSFileOpen(1, name, vfsModeWrite, &fileRef2);
            StrPrintF(buf, "fontType %d\nascent %d\ndescent %d\n", font2->densities[d].density == 72 ? 36864 : 37376, font2->ascent*(d+1), font2->descent*(d+1));
            emit(fileRef2, buf);
            for (i = font2->firstChar; i <= font2->lastChar; i++) {
              if (font2->width[i - font2->firstChar] > 0) {
                StrPrintF(buf, "\nGLYPH %d\n", i);
                emit(fileRef2, buf);
                for (j = 0; j < font2->fRectHeight*(d+1); j++) {
                  for (k = 0; k < font2->width[i - font2->firstChar]*(d+1); k++) {
                    buf[k] = BmpGetPixel(font2->bmp[d], font2->column[i - font2->firstChar]*(d+1) + k, j) ? '#' : '-';
                  }
                  buf[k++] = '\n';
                  buf[k] = 0;
                  emit(fileRef2, buf);
                }
              }
            }
            VFSFileClose(fileRef2);
          }
          StrCopy(buf, "END\n\n");
          emit(fileRef, buf);
        }
        break;
      case iconType:
      case bitmapRsc:
        /*
        BITMAP [<ResType.s>] ID <ResId.n> [LOCALE <Locale.s>]
          [<BitmapCompression>...]
        BEGIN
          BITMAP <Filename.s> BPP <Depth.n> [<BitmapAttribute>...] [DENSITY <Density.n>]
          ...
        END

        [NOCOMPRESS] [COMPRESS] [FORCECOMPRESS]
        [COMPRESSSCANLINE] [COMPRESSRLE] [COMPRESSPACKBITS] [COMPRESSBEST]
        [NOCOLORTABLE] [COLORTABLE] [BITMAPPALETTE <Filename.s>]
        [TRANSPARENT <R.n> <G.n> <B.n>] [TRANSPARENTINDEX <Index.n>]
        */
if (resID == 5516) {
debug(1, "XXX", "here");
}
        StrPrintF(buf, "%s ID %d\nBEGIN\n", resType == iconType ? "ICON" : "BITMAP", resID);
        emit(fileRef, buf);

        for (bitmap = (BitmapType *)p; bitmap;) {
          version = BmpGetVersion(bitmap);
          BmpGetDimensions(bitmap, &width, &height, NULL);
          density = BmpGetDensity(bitmap);
          depth = BmpGetBitDepth(bitmap);
          StrPrintF(name, "%s_%d_%d_%d.bmp", st, resID, depth, density);
          transp = BmpGetTransparentValue(bitmap, &transparentValue);
          attr[0] = 0;

          switch (depth) {
            case 8:
              if (transp) StrPrintF(attr, "TRANSPARENTINDEX %d ", transparentValue);
              break;
            case 16:
              if (transp) {
                red   = r565(transparentValue);
                green = g565(transparentValue);
                blue  = b565(transparentValue);
                StrPrintF(attr, "TRANSPARENT %d %d %d ", red, green, blue);
              }
              break;
          }

          switch (version) {
            case 0:
              StrPrintF(buf, "  BITMAP \"%s\" BPP %d DENSITY 72\n", name, 1);
              emit(fileRef, buf);
              pumpkin_save_bitmap(bitmap, density, 0, 0, width, height, name);
              bitmap = BmpGetNextBitmapAnyDensity(bitmap);
              break;
            case 1:
              if (depth != 0xff) {
                StrPrintF(buf, "  BITMAP \"%s\" BPP %d DENSITY 72\n", name, depth);
                emit(fileRef, buf);
                pumpkin_save_bitmap(bitmap, density, 0, 0, width, height, name);
                bitmap = BmpGetNextBitmapAnyDensity(bitmap);
              } else {
                bitmap = (BitmapType *)(((UInt8 *)bitmap) + 16);
              }
              break;
            case 2:
              StrPrintF(buf, "  BITMAP \"%s\" BPP %d %sDENSITY 72\n", name, depth, attr);
              emit(fileRef, buf);
              BmpSetTransparentValue(bitmap, kTransparencyNone);
              pumpkin_save_bitmap(bitmap, density, 0, 0, width, height, name);
              bitmap = BmpGetNextBitmapAnyDensity(bitmap);
              break;
            case 3:
              StrPrintF(buf, "  BITMAP \"%s\" BPP %d %sDENSITY %d\n", name, depth, attr, density);
              emit(fileRef, buf);
              BmpSetTransparentValue(bitmap, kTransparencyNone);
              pumpkin_save_bitmap(bitmap, density, 0, 0, width, height, name);
              bitmap = BmpGetNextBitmapAnyDensity(bitmap);
              break;
            default:
              debug(DEBUG_ERROR, "RCP", "invalid bitmap version %d", version);
              bitmap = NULL;
          }
        }
        StrCopy(buf, "END\n\n");
        emit(fileRef, buf);
        break;
      case constantRscType:
        // INTEGER ID <ResId.n> 
        // [LOCALE <LocaleName.s>] [VALUE] <Value.n>
        u32 = (UInt32 *)p;
        StrPrintF(buf, "INTEGER ID %d %u", resID, *u32);
        emit(fileRef, buf);
        emitnl(fileRef);
        emitnl(fileRef);
        break;
      case wrdListRscType:
        // WORDLIST ID <ResId.n> 
        // [LOCALE <LocaleName.s>]
        // BEGIN
        // <Word.n>
        // ...
        // END
        size = MemHandleSize(h);
        u16 = (UInt16 *)p;
        StrPrintF(buf, "WORDLIST ID %d\nBEGIN\n", resID);
        emit(fileRef, buf);
        for (i = 0; i < size/2; i++) {
          StrPrintF(buf, "  %u", u16[i]);
          emit(fileRef, buf);
          emitnl(fileRef);
        }
        emit(fileRef, "END");
        emitnl(fileRef);
        emitnl(fileRef);
        break;
      case strRsc:
        // STRING ID <StringResourceId.n> 
        // [LOCALE <LocaleName.s>] <String.ss>
        StrPrintF(buf, "STRING ID %d ", resID);
        emit(fileRef, buf);
        emitstr(fileRef, (char *)p);
        emitnl(fileRef);
        emitnl(fileRef);
        break;
      case strListRscType:
        // STRINGTABLE ID <StringTableResourceId.n> 
        // [LOCALE <LocaleName.s>] <PrefixString.ss>  ... <String.ss>
        StrPrintF(buf, "STRINGTABLE ID %d ", resID);
        emit(fileRef, buf);
        i = 0;
        i += pumpkin_getstr(&prefix, p, 0);
        emitstr(fileRef, prefix);
        i += get2b(&max, p, i);
        for (j = 0; j < max; j++) {
          if (j > 0 && (j % 16) == 0) emitnl(fileRef);
          i += pumpkin_getstr(&str, p, i);
          emit(fileRef, " ");
          emitstr(fileRef, str);
        }
        emitnl(fileRef);
        emitnl(fileRef);
        break;
      case colorTableRsc:
        // PALETTETABLE ID 10008
        // BEGIN
        //   0x00 0xff 0xff 0xff
        //   0x01 0xff 0xcc 0xff
        //   ...
        // END
        StrPrintF(buf, "PALETTETABLE ID %d\nBEGIN\n", resID);
        emit(fileRef, buf);
        get2b(&d, p, 0);
        u8 = ((UInt8 *)p) + 2;
        for (i = 0; i < d; i++) {
          StrPrintF(buf, "  0x%02X 0x%02X 0x%02X 0x%02X\n", u8[0], u8[1], u8[2], u8[3]);
          emit(fileRef, buf);
          u8 += 4;
        }
        StrCopy(buf, "END\n\n");
        emit(fileRef, buf);
        break;
      case verRsc:
        StrPrintF(buf, "VERSION ");
        emit(fileRef, buf);
        emitstr(fileRef, (char *)p);
        emitnl(fileRef);
        emitnl(fileRef);
        break;
      case 'APPL':
        get4b(&creator, p, 0);
        pumpkin_id2s(creator, st2);
        StrPrintF(buf, "APPLICATION ID %d \"%s\"\n\n", resID, st2);
        emit(fileRef, buf);
        break;
      case ainRsc:
        StrPrintF(buf, "APPLICATIONICONNAME ID %d ", resID);
        emit(fileRef, buf);
        emitstr(fileRef, (char *)p);
        emitnl(fileRef);
        emitnl(fileRef);
        break;
      case appInfoStringsRsc:
        StrPrintF(buf, "CATEGORIES ID %d\n", resID);
        emit(fileRef, buf);
        str = (char *)p;
        for (j = 0; j < 16; j++) {
          emit(fileRef, "  ");
          emitstr(fileRef, str);
          emitnl(fileRef);
          str += StrLen(str) + 1;
        }
        emitnl(fileRef);
        // what about the remainder of the tAIS resource ?
        // Address Book stores some strings after the 16 categories.
        break;
      case defaultCategoryRscType:
        StrPrintF(buf, "LAUNCHERCATEGORY ID %d \"%s\"\n\n", resID, (char *)p);
        emit(fileRef, buf);
        break;
      case sysResTFeatures:
        i = get2b(&num, p, 0);
        for (j = 0; j < num; j++) {
          i += get4b(&creator, p, i);
          i += get2b(&d, p, i);
          for (k = 0; k < d; k++) {
            i += get2b(&featNum, p, i);
            i += get4b(&featVal, p, i);
            pumpkin_id2s(creator, st2);
            StrPrintF(buf, "// feature creator '%s', number %d, value 0x%08X\n", st2, featNum, featVal);
            emit(fileRef, buf);
          }
        }
        saveData(h, p, st, resID, fileRef);
        break;
      case sysResTAppCode:
      case sysResTAppGData:
      case sysResTAppPrefs:
      case sysRsrcTypeDlib:
      case 'rloc':
        // ignore
        break;
      default:
        // DATA "locs" ID 10000 "locs.dat"
        debug(DEBUG_ERROR, "RCP", "unknown resource type %s id %d", st, resID);
        saveData(h, p, st, resID, fileRef);
        break;
    }
    MemHandleUnlock(h);
  }
}

static int rcp_export(char *prc, char *rcp) {
  LocalID dbID;
  DmOpenRef dbRef;
  FileRef fileRef;
  DmResType resType;
  DmResID resID;
  MemHandle h;
  UInt16 num, index;
  int r = -1;

  if ((dbID = DmFindDatabase(0, prc)) != 0) {
    if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
      VFSFileCreate(1, rcp);
      VFSFileOpen(1, rcp, vfsModeWrite, &fileRef);
      num = DmNumResources(dbRef);
      for (index = 0; index < num; index++) {
        if (DmResourceInfo(dbRef, index, &resType, &resID, NULL) == errNone) {
          if ((h = DmGetResourceIndex(dbRef, index)) != NULL) {
            export(h, resType, resID, fileRef);
            DmReleaseResource(h);
          }
        }
      }
      VFSFileClose(fileRef);
      DmCloseDatabase(dbRef);
      r = 0;
    }
  }

  return r;
}

int CommandMain(int argc, char *argv[]) {
  int r = -1;

  if (argc == 2) {
    r = rcp_export(argv[0], argv[1]);
  }

  return r;
}
