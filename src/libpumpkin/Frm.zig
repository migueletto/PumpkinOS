const std = @import("std");
const pumpkin = @import("pumpkin.zig");

const EventType = pumpkin.EventType;
const FormType = pumpkin.FormType;
const ControlType = pumpkin.ControlType;
const FieldType = pumpkin.FieldType;
const ListType = pumpkin.ListType;
const TableType = pumpkin.TableType;
const GadgetType = pumpkin.GadgetType;
const ScrollBarType = pumpkin.ScrollBarType;

pub const formObjects = enum(u8) {
  fieldObj,
  controlObj,
  listObj,
  tableObj,
  bitmapObj,
  lineObj,
  frameObj,
  rectangleObj,
  labelObj,
  titleObj,
  popupObj,
  graffitiStateObj,
  gadgetObj,
  scrollBarObj,
};

pub const errorOkAlert = 10021;
pub const errorOkCancelAlert = 10022;
pub const errorCancelAlert = 10023;
pub const informationOkAlert = 10024;
pub const informationOkCancelAlert = 10025;
pub const informationCancelAlert = 10026;
pub const confirmationOkAlert = 10028;
pub const confirmationOkCancelAlert = 10029;
pub const confirmationCancelAlert = 10030;
pub const warningOkAlert = 10031;
pub const warningOkCancelAlert = 10032;
pub const warningCancelAlert = 10033;

const c = @cImport({
  @cInclude("zigpumpkin.h");
});

pub const invalidObjectId: u16 = 0xffff;

pub const eventHandlerFn = *const fn(eventP: *EventType) bool;

pub const FormMap = std.AutoHashMap(u16, eventHandlerFn);

pub fn centerDialogs(center: bool) void {
  c.FrmCenterDialogs(if (center) 1 else 0);
}

pub fn setUsable(formP: *FormType, objIndex: u16, usable: bool) void {
  c.FrmSetUsable(formP, objIndex, if (usable) 1 else 0);
}

pub fn dispatchEvent(event: *EventType) bool {
  return c.FrmDispatchEvent(event) != 0;
}

pub fn gotoForm(formId: u16) void {
  c.FrmGotoForm(formId);
}

pub fn initForm(formId: u16) ?*FormType {
  return @ptrCast(c.FrmInitForm(formId));
}

pub fn setActiveForm(formP: ?*FormType) void {
  c.FrmSetActiveForm(formP);
}

pub fn closeAllForms() void {
  c.FrmCloseAllForms();
}

pub fn setEventHandler(formP: ?*FormType, handler: ?eventHandlerFn) void {
  c.FrmSetEventHandler(formP, @ptrCast(@constCast(handler)));
}

pub fn getActiveForm() *FormType {
  return @ptrCast(c.FrmGetActiveForm());
}

pub fn getActiveFormID() u16 {
  return c.FrmGetActiveFormID();
}

pub fn drawForm(formP: *FormType) void {
  c.FrmDrawForm(formP);
}

pub fn alert(alertId: u16) u16 {
  return c.FrmAlert(alertId);
}

pub fn customAlert(alertId: u16, s1: [*]const u8, s2: [*]const u8, s3: [*]const u8) u16 {
  return c.FrmCustomAlert(alertId, s1, s2, s3);
}

pub fn doDialog(formP: *FormType) u16 {
  return c.FrmDoDialog(formP);
}

pub fn deleteForm(formP: *FormType) void {
  c.FrmDeleteForm(formP);
}

pub fn doDialogId(formId: u16) u16 {
  const formP = c.FrmInitForm(formId);
  const r = c.FrmDoDialog(formP);
  c.FrmDeleteForm(formP);
  return r;
}

pub fn popupForm(formId: u16) void {
  c.FrmPopupForm(formId);
}

pub fn returnToForm(formId: u16) void {
  c.FrmReturnToForm(formId);
}

pub fn updateForm(formId: u16, updateCode: u16) void {
  c.FrmUpdateForm(formId, updateCode);
}

pub fn help(helpMsgId: u16) void {
  c.FrmHelp(helpMsgId);
}

pub fn getLabel(formP: *FormType, labelID: u16) [*]const u8 {
  return c.FrmGetLabel(formP, labelID);
}

pub fn copyLabel(formP: *FormType, labelID: u16, newLabel: [*]const u8) void {
  c.FrmCopyLabel(formP, labelID, newLabel);
}

pub fn setFieldNum(formP: *FormType, fieldId: u16, value: f64) void {
  const objIndex: u16 = getObjectIndex(formP, fieldId, formObjects.fieldObj);
  if (objIndex != invalidObjectId) {
    const fld: *FieldType = @ptrCast(c.FrmGetObjectPtr(formP, objIndex));
    var buf: [32]u8 = undefined;
    const slice = std.fmt.bufPrint(&buf, "{d:.2}", .{ value }) catch { return; };
    const len: u16 = @intCast(slice.len);
    _ = c.FldInsert(fld, slice.ptr, len);
  }
}

pub fn getFieldNum(formP: *FormType, fieldId: u16) f64 {
  var value: f64 = 0;
  const objIndex: u16 = getObjectIndex(formP, fieldId, formObjects.fieldObj);
  if (objIndex != invalidObjectId) {
    const fld: *FieldType = @ptrCast(c.FrmGetObjectPtr(formP, objIndex));
    var str = c.FldGetTextPtr(fld);
    const len = c.StrLen(str);
    if (len > 0) {
      value = std.fmt.parseFloat(f64, str[0..len]) catch 0.0;
    }
  }

  return value;
}

pub fn setControlValue(formP: *FormType, objIndex: u16, newValue: i16) void {
  c.FrmSetControlValue(formP, objIndex, newValue);
}

pub fn setControlGroupSelection(formP: *FormType, groupNum: u8, controlID: u16) void {
  c.FrmSetControlGroupSelection(formP, groupNum, controlID);
}

pub fn getControlGroupSelection(formP: *FormType, groupNum: u8) u16 {
  return c.FrmGetControlGroupSelection(formP, groupNum);
}

pub fn getObjectIndex(formP: *FormType, controlId: u16, requiredObjType: formObjects) u16 {
  const objIndex: u16 = c.FrmGetObjectIndex(formP, controlId);
  if (objIndex == invalidObjectId) return objIndex;
  const objTypeU8: u8 = c.FrmGetObjectType(formP, objIndex);
  const objType: formObjects = @enumFromInt(objTypeU8);
  return if (objType == requiredObjType) objIndex else invalidObjectId;
}

pub fn getControl(formP: *FormType, controlId: u16) ?*ControlType {
  const objIndex: u16 = getObjectIndex(formP, controlId, formObjects.controlObj);
  return if (objIndex != invalidObjectId) @ptrCast(c.FrmGetObjectPtr(formP, objIndex)) else null;
}

pub fn getField(formP: *FormType, controlId: u16) ?*FieldType {
  const objIndex: u16 = getObjectIndex(formP, controlId, formObjects.fieldObj);
  return if (objIndex != invalidObjectId) @ptrCast(c.FrmGetObjectPtr(formP, objIndex)) else null;
}

pub fn getList(formP: *FormType, controlId: u16) ?*ListType {
  const objIndex: u16 = getObjectIndex(formP, controlId, formObjects.listObj);
  return if (objIndex != invalidObjectId) @ptrCast(c.FrmGetObjectPtr(formP, objIndex)) else null;
}

pub fn getTable(formP: *FormType, controlId: u16) ?*TableType {
  const objIndex: u16 = getObjectIndex(formP, controlId, formObjects.tableObj);
  return if (objIndex != invalidObjectId) @ptrCast(c.FrmGetObjectPtr(formP, objIndex)) else null;
}

pub fn getGadget(formP: *FormType, controlId: u16) ?*GadgetType {
  const objIndex: u16 = getObjectIndex(formP, controlId, formObjects.gadgetObj);
  return if (objIndex != invalidObjectId) @ptrCast(c.FrmGetObjectPtr(formP, objIndex)) else null;
}

pub fn getScrollBar(formP: *FormType, controlId: u16) ?*ScrollBarType {
  const objIndex: u16 = getObjectIndex(formP, controlId, formObjects.scrollBarObj);
  return if (objIndex != invalidObjectId) @ptrCast(c.FrmGetObjectPtr(formP, objIndex)) else null;
}

pub fn simpleFrmOpenHandler() bool {
  const formP = pumpkin.Frm.getActiveForm();
  pumpkin.Frm.drawForm(formP);
  return true;
}

pub fn eventLoop(formMap: *FormMap, timeout: i32) void {
  var event = pumpkin.EventType {};

  while (true) {
    pumpkin.Evt.getEvent(&event, timeout);
    if (pumpkin.Sys.handleEvent(&event)) continue;
    if (pumpkin.Menu.handleEvent(&event)) continue;
    if (event.eType == pumpkin.eventTypes.frmLoad) {
      const eventHandler = formMap.get(event.data.frmLoad.formID);
      if (eventHandler != null) {
        const formP = initForm(event.data.frmLoad.formID);
        if (formP != null) {
          setActiveForm(formP);
          setEventHandler(formP, eventHandler);
        }
        continue;
      }
    }
    _ = dispatchEvent(&event); // ignore return value
    if (event.eType == pumpkin.eventTypes.appStop) break;
  }
}

pub fn normalLaunchMain(firstForm: u16, formMap: *FormMap, timeout: i32) void {
  gotoForm(firstForm);
  eventLoop(formMap, timeout);
  closeAllForms();
}
