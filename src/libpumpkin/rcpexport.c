#include <PalmOS.h>

#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "rcpexport.h"
#include "script.h"
#include "thread.h"
#include "mutex.h"
#include "rgb.h"
#include "sys.h"
#include "bytes.h"
#include "AppRegistry.h"
#include "storage.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

static void emitn(int fd, char *buf, int len) {
  sys_write(fd, (uint8_t *)buf, len);
}

static void emit(int fd, char *buf) {
  emitn(fd, buf, StrLen(buf));
}

static void emitstr(int fd, char *buf) {
  char aux[8];
  uint8_t c;
  int i;

  emitn(fd, "\"", 1);
  for (i = 0; buf[i]; i++) {
    c = buf[i];
    switch (c) {
      case '\t':
        emit(fd, "\\t");
        break;
      case '\n':
        emit(fd, "\\n");
        break;
      case '\r':
        break;
      case '"':
        emit(fd, "\\\"");
        break;
      default:
        if (c < 32 || c > 127) {
          StrPrintF(aux, "\\x%02x", c);
          emit(fd, aux);
        } else {
          emitn(fd, &buf[i], 1);
        }
        break;
    }
  }
  emitn(fd, "\"", 1);
}

static void emitnl(int fd) {
  emit(fd, "\n");
}

static void emitField(int fd, char *buf, FieldType *field) {
  StrPrintF(buf, "  FIELD ID %d AT (%d %d %d %d) %s", field->id,
    field->rect.topLeft.x, field->rect.topLeft.y, field->rect.extent.x, field->rect.extent.y,
    field->attr.usable ? "USABLE" : "NONUSABLE");
  emit(fd, buf);

  switch (field->attr.justification) {
    case leftAlign:  emit(fd, " LEFTALIGN");  break;
    case rightAlign: emit(fd, " RIGHTALIGN"); break;
  }

  emit(fd, field->attr.singleLine ? " SINGLELINE" : " MULTIPLELINES");
  if (field->attr.dynamicSize) emit(fd, " DYNAMICSIZE");
  if (field->attr.autoShift) emit(fd, " AUTOSHIFT");
  if (field->attr.numeric) emit(fd, " NUMERIC");
  if (field->attr.hasScrollBar) emit(fd, " HASSCROLLBAR");

  StrPrintF(buf, " MAXCHARS %d", field->maxChars);
  emit(fd, buf);
  emitnl(fd);
}

static void emitControl(int fd, char *buf, char *type, FormObjectType *obj) {
  emit(fd, "  ");
  emit(fd, type);
  emit(fd, " ");
  emitstr(fd, obj->control->text);
  StrPrintF(buf, " ID %d AT (%d %d %d %d) %s %s", obj->control->id,
    obj->control->bounds.topLeft.x, obj->control->bounds.topLeft.y, obj->control->bounds.extent.x, obj->control->bounds.extent.y,
    obj->control->attr.usable ? "USABLE" : "NONUSABLE", obj->control->attr.leftAnchor ? "LEFTANCHOR" : "RIGHTANCHOR");
  emit(fd, buf);
}

static void emitGraphical(int fd, char *buf, FormObjectType *obj) {
  if (obj->control->attr.graphical) {
    emit(fd, " GRAPHICAL");
    if (obj->control->bitmapID) {
      StrPrintF(buf, " BITMAPID %d", obj->control->bitmapID);
      emit(fd, buf);
    }
    if (obj->control->selectedBitmapID) {
      StrPrintF(buf, " SELECTEDBITMAPID %d", obj->control->selectedBitmapID);
      emit(fd, buf);
    }
  }
}

static void emitFrame(int fd, FormObjectType *obj) {
  switch (obj->control->attr.frame) {
    case noButtonFrame:        emit(fd, " NOFRAME");   break;
    case standardButtonFrame:  emit(fd, " FRAME");     break;
    case boldButtonFrame:      emit(fd, " BOLDFRAME"); break;
    case rectangleButtonFrame: emit(fd, " RECTFRAME"); break;
  }
}

static void emitGroup(int fd, char *buf, FormObjectType *obj) {
  if (obj->control->group) {
    StrPrintF(buf, " GROUP %d", obj->control->group);
    emit(fd, buf);
  }
}

static void emitFont(int fd, char *buf, FormObjectType *obj) {
  StrPrintF(buf, " FONT %d", obj->control->font);
  emit(fd, buf);
}

static void export(MemHandle h, DmResType resType, DmResID resID, int fd) {
  char buf[256], attr[64];
  char st[8], name[64];
  AlertTemplateType *alert;
  BitmapType *bitmap, *next;
  BitmapTypeV0 *bitmapV0;
  BitmapTypeV1 *bitmapV1;
  BitmapTypeV2 *bitmapV2;
  BitmapTypeV3 *bitmapV3;
  Coord width, height;
  UInt32 depth, density, transparentValue;
  UInt8 red, green, blue;
  Boolean transp;
  FontType *font;
  FontTypeV2 *font2;
  MenuBarType *menuBar;
  MenuItemType *menuItem;
  FormType *form;
  FormObjectType obj;
  SliderControlType *slider;
  UInt16 *u16, d, i, j, k, max;
  UInt32 *u32, size;
  char *prefix, *str;
  int fd2;
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
        emit(fd, buf);
        emitnl(fd);
        if (alert->helpRscID) {
          StrPrintF(buf, "HELPID %d", alert->helpRscID);
          emit(fd, buf);
          emitnl(fd);
        }
        if (alert->defaultButton) {
          StrPrintF(buf, "DEFAULTBUTTON %d", alert->defaultButton);
          emit(fd, buf);
          emitnl(fd);
        }
        switch (alert->alertType) {
          case informationAlert:  emit(fd, "INFORMATION\n"); break;
          case confirmationAlert: emit(fd, "CONFIRMATION\n"); break;
          case warningAlert:      emit(fd, "WARNING\n"); break;
          case errorAlert:        emit(fd, "ERROR\n"); break;
        }
        emit(fd, "BEGIN\n  TITLE ");
        emitstr(fd, alert->title);
        emit(fd, "\n  MESSAGE ");
        emitstr(fd, alert->message);
        emit(fd, "\n  BUTTONS");
        for (i = 0; i < alert->numButtons; i++) {
          emit(fd, " ");
          emitstr(fd, alert->button[i]);
        }
        emit(fd, "\nEND\n\n");
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
        //form = (FormType *)p;
        form = FrmInitForm(resID);
        StrPrintF(buf, "FORM ID %d AT (%d %d %d %d)\n", resID, form->window.windowBounds.topLeft.x, form->window.windowBounds.topLeft.y, form->window.windowBounds.extent.x, form->window.windowBounds.extent.x);
        emit(fd, buf);
        if (form->window.frameType.word == 0) {
          StrCopy(buf, "  NOFRAME\n");
          emit(fd, buf);
        }
        if (form->window.windowFlags.modal) {
          StrCopy(buf, "  MODAL\n");
          emit(fd, buf);
        }
        if (form->attr.saveBehind) {
          StrCopy(buf, "  SAVEBEHIND\n");
          emit(fd, buf);
        }
        if (form->attr.usable) {
          StrCopy(buf, "  USABLE\n");
          emit(fd, buf);
        }
        if (form->helpRscId) {
          StrPrintF(buf, "  HELPID %d\n", form->helpRscId);
          emit(fd, buf);
        }
        if (form->defaultButton) {
          StrPrintF(buf, "  DEFAULTBTNID %d\n", form->defaultButton);
          emit(fd, buf);
        }
        if (form->menuRscId) {
          StrPrintF(buf, "  MENUID %d\n", form->menuRscId);
          emit(fd, buf);
        }
        StrCopy(buf, "BEGIN\n");
        emit(fd, buf);
        for (i = 0; i < form->numObjects; i++) {
          obj = form->objects[i].object;
          switch (form->objects[i].objectType) {
            case frmFieldObj:
              // FIELD ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
              // [USABLE] [NONUSABLE] [DISABLED] [LEFTALIGN] [RIGHTALIGN]
              // [FONT <FontId.n>] [EDITABLE] [NONEDITABLE] [UNDERLINED]
              // [SINGLELINE] [MULTIPLELINES] [DYNAMICSIZE] [MAXCHARS <MaxChars.n>]
              // [AUTOSHIFT] [NUMERIC] [HASSCROLLBAR]
              emitField(fd, buf, obj.field);
              break;
            case frmControlObj:
              switch (obj.control->style) {
                case buttonCtl:
                  // BUTTON <Label.ss> ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
                  // [USABLE] [NONUSABLE] [DISABLED] [LEFTANCHOR] [RIGHTANCHOR]
                  // [FRAME] [NOFRAME] [BOLDFRAME] [RECTFRAME] [FONT <FontId.n>]
                  // [GRAPHICAL] [BITMAPID <BitmapId.n>] [SELECTEDBITMAPID <BitmapId.n>]
                  emitControl(fd, buf, "BUTTON", &obj);
                  emitFrame(fd, &obj);
                  emitFont(fd, buf, &obj);
                  emitGraphical(fd, buf, &obj);
                  emit(fd, "\n");
                  break;
                case pushButtonCtl:
                  // PUSHBUTTON <Label.ss> ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
                  // [USABLE] [NONUSABLE] [DISABLED] [LEFTANCHOR] [RIGHTANCHOR]
                  // [FONT <FontId>] [GROUP <GroupId.n>]
                  // [GRAPHICAL] [BITMAPID <BitmapId.n>] [SELECTEDBITMAPID <BitmapId.n>] 
                  emitControl(fd, buf, "PUSHBUTTON", &obj);
                  emitFont(fd, buf, &obj);
                  emitGroup(fd, buf, &obj);
                  emitGraphical(fd, buf, &obj);
                  emit(fd, "\n");
                  break;
                case checkboxCtl:
                  // CHECKBOX <Label.ss> ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
                  // [USABLE] [NONUSABLE] [DISABLED] [LEFTANCHOR] [RIGHTANCHOR]
                  // [FONT <FontId.n>] [GROUP <GroupId.n>] [CHECKED]
                  emitControl(fd, buf, "CHECKBOX", &obj);
                  emitFont(fd, buf, &obj);
                  emitGroup(fd, buf, &obj);
                  if (obj.control->attr.on) emit(fd, " CHECKED");
                  emit(fd, "\n");
                  break;
                case popupTriggerCtl:
                  // POPUPTRIGGER <Label.ss> ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
                  // [USABLE] [NONUSABLE] [DISABLED] [LEFTANCHOR] [RIGHTANCHOR]
                  // [FONT <FontId.n>]
                  // [GRAPHICAL] [BITMAPID <BitmapId.n>] [SELECTEDBITMAPID <BitmapId.n>]
                  emitControl(fd, buf, "POPUPTRIGGER", &obj);
                  emitFont(fd, buf, &obj);
                  emitGraphical(fd, buf, &obj);
                  emit(fd, "\n");
                  break;
                case selectorTriggerCtl:
                  // SELECTORTRIGGER <Label.ss> ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
                  // [USABLE] [NONUSABLE] [DISABLED] [LEFTANCHOR] [RIGHTANCHOR]
                  // [FONT <FontId.n>]
                  // [GRAPHICAL] [BITMAPID <BitmapId.n>] [SELECTEDBITMAPID <BitmapId.n>]
                  emitControl(fd, buf, "SELECTORTRIGGER", &obj);
                  emitFont(fd, buf, &obj);
                  emitGraphical(fd, buf, &obj);
                  emit(fd, "\n");
                  break;
                case repeatingButtonCtl:
                  // REPEATBUTTON <Label.ss> ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
                  // [USABLE] [NONUSABLE] [DISABLED] [LEFTANCHOR] [RIGHTANCHOR]
                  // [FRAME] [NOFRAME] [BOLDFRAME] [RECTFRAME] [FONT <FontId.n>]
                  // [GRAPHICAL] [BITMAPID <BitmapId.n>] [SELECTEDBITMAPID <BitmapId.n>]
                  emitControl(fd, buf, "REPEATBUTTON", &obj);
                  emitFrame(fd, &obj);
                  emitFont(fd, buf, &obj);
                  emitGraphical(fd, buf, &obj);
                  emit(fd, "\n");
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
                  emit(fd, buf);
                  if (obj.control->attr.vertical) emit(fd, " VERTICAL");
                  if (obj.control->style == feedbackSliderCtl) emit(fd, " FEEDBACK");
                  if (slider->thumbID) {
                    StrPrintF(buf, " THUMBID %d", slider->thumbID);
                    emit(fd, buf);
                  }
                  if (slider->backgroundID) {
                    StrPrintF(buf, " BACKGROUNDID %d", slider->backgroundID);
                    emit(fd, buf);
                  }
                  StrPrintF(buf, " VALUE %d MIN %d MAX %d PAGESIZE %d\n", slider->value, slider->minValue, slider->maxValue, slider->pageSize);
                  emit(fd, buf);
                  break;
                default:
                  break;
              }
              break;
            case frmTitleObj:
              emit(fd, "  TITLE ");
              emitstr(fd, obj.title->text);
              emitnl(fd);
              break;
            case frmLabelObj:
              // LABEL <Label.ss> ID <Id.n> AT (<Left.p> <Top.p>)
              // [USABLE] [NONUSABLE] [FONT <FontId.n>] 
              emit(fd, "  LABEL ");
              emitstr(fd, obj.label->text);
              StrPrintF(buf, " ID %d AT (%d %d) %s FONT %d\n",
                obj.label->id, obj.label->pos.x, obj.label->pos.y, obj.label->attr.usable ? "USABLE" : "NONUSABLE", obj.label->fontID);
              emit(fd, buf);
              break;
            case frmBitmapObj:
              // FORMBITMAP AT (<Left.p> <Top.p>)
              // [BITMAP <BitmapId.n>] [USABLE] [NONUSABLE]
              StrPrintF(buf, "  FORMBITMAP AT (%d %d) BITMAP %d %s\n",
                obj.bitmap->pos.x, obj.bitmap->pos.y, obj.bitmap->rscID, obj.bitmap->attr.usable ? "USABLE" : "NONUSABLE");
              emit(fd, buf);
              break;
            case frmGadgetObj:
              // GADGET ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
              // [USABLE] [NONUSABLE] [NONEXTENDED] [EXTENDED] 
              StrPrintF(buf, "  GADGET ID %d AT (%d %d %d %d) %s %s\n",
                obj.gadget->id, obj.gadget->rect.topLeft.x, obj.gadget->rect.topLeft.y, obj.gadget->rect.extent.x, obj.gadget->rect.extent.y,
                obj.gadget->attr.usable ? "USABLE" : "NONUSABLE", obj.gadget->attr.extended ? "EXTENDED" : "NONEXTENDED");
              emit(fd, buf);
              break;
            case frmListObj:
              // LIST <Item.s> ... <Item.s>
              // ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
              // [USABLE] [NONUSABLE] [DISABLED] [VISIBLEITEMS <NumVisItems.n>]
              // [FONT <FontId.n>] [SEARCH] 
              StrCopy(buf, "  LIST");
              emit(fd, buf);
              for (j = 0; j < obj.list->numItems; j++) {
                emit(fd, " ");
                if (obj.list->itemsText[j] && obj.list->itemsText[j][0]) {
                  emitstr(fd, obj.list->itemsText[j]);
                } else {
                  emitstr(fd, "");
                }
              }
              StrPrintF(buf, " ID %d AT (%d %d %d %d) %s%s", obj.list->id,
                obj.list->bounds.topLeft.x, obj.list->bounds.topLeft.y, obj.list->bounds.extent.x, obj.list->bounds.extent.y,
                obj.list->attr.usable ? "USABLE" : "NONUSABLE", obj.list->attr.search ? " SEARCH" : "");
              emit(fd, buf);
              if (obj.list->visibleItems) {
                StrPrintF(buf, " VISIBLEITEMS %d", obj.list->visibleItems);
                emit(fd, buf);
              }
              StrPrintF(buf, " FONT %d\n", obj.list->font);
              emit(fd, buf);
              break;
            case frmTableObj:
              // TABLE ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
              // [ROWS <NumRows.n>] [COLUMNS <NumCols.n>]
              // [COLUMNWIDTHS <Col1Width.n> ... <ColNWidth.n>] 
              StrPrintF(buf, "  TABLE ID %d AT (%d %d %d %d) ROWS %d COLUMNS %d COLUMNWIDTHS",
                obj.table->id, obj.table->bounds.topLeft.x, obj.table->bounds.topLeft.y, obj.table->bounds.extent.x, obj.table->bounds.extent.y,
                obj.table->numRows, obj.table->numColumns);
              emit(fd, buf);
              for (j = 0; j < obj.table->numColumns; j++) {
                StrPrintF(buf, " %d", obj.table->columnAttrs[j].width);
                emit(fd, buf);
              }
              emitnl(fd);
              break;
            case frmScrollBarObj:
              // SCROLLBAR ID <Id.n> AT (<Left.p> <Top.p> <Width.p> <Height.p>)
              // [USABLE] [NONUSABLE] [VALUE <Value.n>] [MIN <MinValue.n>]
              // [MAX <MaxValue.n>] [PAGESIZE <PageSize.n>]
              StrPrintF(buf, "  SCROLLBAR ID %d AT (%d %d %d %d) %s VALUE %d MIN %d MAX %d PAGESIZE %d\n", obj.scrollBar->id,
                obj.scrollBar->bounds.topLeft.x, obj.scrollBar->bounds.topLeft.y, obj.scrollBar->bounds.extent.x, obj.scrollBar->bounds.extent.y,
                obj.scrollBar->attr.usable ? "USABLE" : "NONUSABLE", obj.scrollBar->value, obj.scrollBar->minValue,
                obj.scrollBar->maxValue, obj.scrollBar->pageSize);
              emit(fd, buf);
              break;
            case frmPopupObj:
              // POPUPLIST ID <ControlId.n> <ListId.n> 
              StrPrintF(buf, "  POPUPLIST ID %d %d\n", obj.popup->controlID, obj.popup->listID);
              emit(fd, buf);
              break;
            case frmGraffitiStateObj:
              // GRAFFITISTATEINDICATOR AT (<Left.p> <Top.p>)
              StrPrintF(buf, "  GRAFFITISTATEINDICATOR AT (%d %d)\n", obj.grfState->pos.x, obj.grfState->pos.y);
              emit(fd, buf);
            default:
              break;
          }
        }
        StrCopy(buf, "END\n\n");
        emit(fd, buf);
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
        emit(fd, buf);
        for (i = 0; i < menuBar->numMenus; i++) {
          emit(fd, "  PULLDOWN ");
          emitstr(fd, menuBar->menus[i].title);
          emitnl(fd);
          emit(fd, "  BEGIN");
          emitnl(fd);
          for (j = 0; j < menuBar->menus[i].numItems; j++) {
            menuItem = &menuBar->menus[i].items[j];
            if (menuItem->itemStr) {
              if (menuItem->itemStr[0] == '-') {
                emit(fd, "    MENUITEM SEPARATOR");
              } else {
                emit(fd, "    MENUITEM ");
                emitstr(fd, menuItem->itemStr);
                StrPrintF(name, " \"%c\"", menuItem->command);
                StrPrintF(buf, " ID %d%s", menuItem->id, menuItem->command ? name : "");
                emit(fd, buf);
              }
              emitnl(fd);
            }
          }
          StrCopy(buf, "  END\n");
          emit(fd, buf);
        }
        StrCopy(buf, "END\n\n");
        emit(fd, buf);
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
          emit(fd, buf);
          fd2 = sys_create(name, SYS_WRITE|SYS_TRUNC, 0644);
          StrPrintF(buf, "fontType 36864\nascent %d\ndescent %d\n", font->ascent, font->descent);
          emit(fd2, buf);
          for (i = font->firstChar; i <= font->lastChar; i++) {
            if (font->width[i - font->firstChar] > 0) {
              StrPrintF(buf, "\nGLYPH %d\n", i);
              emit(fd2, buf);
              for (j = 0; j < font->fRectHeight; j++) {
                for (k = 0; k < font->width[i - font->firstChar]; k++) {
                  buf[k] = BmpGetPixel(font->bmp, font->column[i - font->firstChar] + k, j) ? '#' : '-';
                }
                buf[k++] = '\n';
                buf[k] = 0;
                emit(fd2, buf);
              }
            }
          }
          sys_close(fd2);
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
          emit(fd, buf);
          for (d = 0; d < font2->densityCount; d++) {
            StrPrintF(name, "fontv2_%d_%d.txt", resID, font2->densities[d].density);
            StrPrintF(buf, "  FONT \"%s\" DENSITY %d\n", name, font2->densities[d].density);
            emit(fd, buf);
            fd2 = sys_create(name, SYS_WRITE|SYS_TRUNC, 0644);
            StrPrintF(buf, "fontType %d\nascent %d\ndescent %d\n", font2->densities[d].density == 72 ? 36864 : 37376, font2->ascent*(d+1), font2->descent*(d+1));
            emit(fd2, buf);
            for (i = font2->firstChar; i <= font2->lastChar; i++) {
              if (font2->width[i - font2->firstChar] > 0) {
                StrPrintF(buf, "\nGLYPH %d\n", i);
                emit(fd2, buf);
                for (j = 0; j < font2->fRectHeight*(d+1); j++) {
                  for (k = 0; k < font2->width[i - font2->firstChar]*(d+1); k++) {
                    buf[k] = BmpGetPixel(font2->bmp[d], font2->column[i - font2->firstChar]*(d+1) + k, j) ? '#' : '-';
                  }
                  buf[k++] = '\n';
                  buf[k] = 0;
                  emit(fd2, buf);
                }
              }
            }
            sys_close(fd2);
          }
          StrCopy(buf, "END\n\n");
          emit(fd, buf);
        }
        break;
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
        StrPrintF(buf, "BITMAP ID %d\nBEGIN\n", resID);
        emit(fd, buf);

        for (bitmap = (BitmapType *)p; bitmap;) {
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

          switch (bitmap->version) {
            case 0:
              StrPrintF(buf, "  BITMAP \"%s\" BPP %d DENSITY 72\n", name, 1);
              emit(fd, buf);
              bitmapV0 = (BitmapTypeV0 *)bitmap;
              next = bitmapV0->next;
              bitmapV0->next = NULL;
              pumpkin_save_bitmap(bitmap, density, 0, 0, width, height, name);
              bitmap = next;
              break;
            case 1:
              StrPrintF(buf, "  BITMAP \"%s\" BPP %d DENSITY 72\n", name, depth);
              emit(fd, buf);
              bitmapV1 = (BitmapTypeV1 *)bitmap;
              next = bitmapV1->next;
              bitmapV1->next = NULL;
              pumpkin_save_bitmap(bitmap, density, 0, 0, width, height, name);
              bitmap = next;
              break;
            case 2:
              StrPrintF(buf, "  BITMAP \"%s\" BPP %d %sDENSITY 72\n", name, depth, attr);
              emit(fd, buf);
              bitmapV2 = (BitmapTypeV2 *)bitmap;
              bitmapV2->flags.hasTransparency = 0;
              next = bitmapV2->next;
              bitmapV2->next = NULL;
              pumpkin_save_bitmap(bitmap, density, 0, 0, width, height, name);
              bitmap = next;
              break;
            case 3:
              StrPrintF(buf, "  BITMAP \"%s\" BPP %d %sDENSITY %d\n", name, depth, attr, density);
              emit(fd, buf);
              bitmapV3 = (BitmapTypeV3 *)bitmap;
              bitmapV3->flags.hasTransparency = 0;
              next = bitmapV3->next;
              bitmapV3->next = NULL;
              pumpkin_save_bitmap(bitmap, density, 0, 0, width, height, name);
              bitmap = next;
              break;
            default:
              debug(DEBUG_ERROR, "RCP", "invalid bitmap version %d", bitmap->version);
              bitmap = NULL;
          }
        }
        StrCopy(buf, "END\n\n");
        emit(fd, buf);
        break;
      case constantRscType:
        // INTEGER ID <ResId.n> 
        // [LOCALE <LocaleName.s>] [VALUE] <Value.n>
        u32 = (UInt32 *)p;
        StrPrintF(buf, "INTEGER ID %d %u", resID, *u32);
        emit(fd, buf);
        emitnl(fd);
        emitnl(fd);
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
        emit(fd, buf);
        for (i = 0; i < size/2; i++) {
          StrPrintF(buf, "  %u", u16[i]);
          emit(fd, buf);
          emitnl(fd);
        }
        emit(fd, "END");
        emitnl(fd);
        emitnl(fd);
        break;
      case strRsc:
        // STRING ID <StringResourceId.n> 
        // [LOCALE <LocaleName.s>] <String.ss>
        StrPrintF(buf, "STRING ID %d ", resID);
        emit(fd, buf);
        emitstr(fd, (char *)p);
        emitnl(fd);
        emitnl(fd);
        break;
      case strListRscType:
        // STRINGTABLE ID <StringTableResourceId.n> 
        // [LOCALE <LocaleName.s>] <PrefixString.ss>  ... <String.ss>
        StrPrintF(buf, "STRINGTABLE ID %d ", resID);
        emit(fd, buf);
        i = 0;
        i += pumpkin_getstr(&prefix, p, 0);
        emitstr(fd, prefix);
        i += get2b(&max, p, i);
        for (j = 0; j < max; j++) {
          if (j > 0 && (j % 16) == 0) emitnl(fd);
          i += pumpkin_getstr(&str, p, i);
          emit(fd, " ");
          emitstr(fd, str);
        }
        emitnl(fd);
        emitnl(fd);
        break;
      default:
        debug(DEBUG_ERROR, "RCP", "unknown resource type %s", st);
        break;
    }
    MemHandleUnlock(h);
  }
}

static void exportType(int fd, DmOpenRef dbRef, DmResType resType) {
  UInt16 typeIndex, index;
  DmResID resID;
  MemHandle h;

  for (typeIndex = 0; typeIndex < 0x8000; typeIndex++) {
    if ((index = DmFindResourceType(dbRef, resType, typeIndex)) == 0xffff) break;
    if ((h = DmGetResourceIndex(dbRef, index)) != NULL) {
      if (DmResourceType(h, &resType, &resID) == errNone) {
        export(h, resType, resID, fd);
      }
      DmReleaseResource(h);
    }
  }
}

int rcp_export(char *prc, char *rcp) {
  LocalID dbID;
  DmOpenRef dbRef;
  int fd, r = -1;

  if ((dbID = DmFindDatabase(0, prc)) != 0) {
    if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
      fd = sys_create(rcp, SYS_WRITE|SYS_TRUNC, 0644);
      exportType(fd, dbRef, formRscType);
      exportType(fd, dbRef, MenuRscType);
      exportType(fd, dbRef, alertRscType);
      exportType(fd, dbRef, bitmapRsc);
      exportType(fd, dbRef, fontRscType);
      exportType(fd, dbRef, fontExtRscType);
      exportType(fd, dbRef, strRsc);
      exportType(fd, dbRef, strListRscType);
      exportType(fd, dbRef, constantRscType);
      exportType(fd, dbRef, wrdListRscType);
      sys_close(fd);
      DmCloseDatabase(dbRef);
      r = 0;
    }
  }

  return r;
}
