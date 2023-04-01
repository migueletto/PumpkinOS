const pumpkin = @import("pumpkin.zig");

const EventType = pumpkin.EventType;
const FormType = pumpkin.FormType;
const ControlType = pumpkin.ControlType;

const formObjects = enum(u8) {
  fieldObj,
  controlObj,
  listObj,
  tableObj,
  bitmapObj,
  lineObj,
  FrameObj,
  rectangleObj,
  labelObj,
  titleObj,
  popupObj,
  graffitiStateObj,
  gadgetObj,
  scrollBarObj,
};

const c = @cImport({
  @cInclude("zigpumpkin.h");
});

pub const eventHandlerFn = *const fn(eventP: *EventType) bool;

pub const formMapperFn = *const fn(formId: u16) eventHandlerFn;

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

pub fn initForm(formId: u16) *FormType {
  return @ptrCast(*FormType, c.FrmInitForm(formId));
}

pub fn setActiveForm(formP: *FormType) void {
  c.FrmSetActiveForm(formP);
}

pub fn closeAllForms() void {
  c.FrmCloseAllForms();
}

pub fn setEventHandler(formP: *FormType, handler: eventHandlerFn) void {
  c.FrmSetEventHandler(formP, @ptrCast(*void, @constCast(handler)));
}

pub fn getActiveForm() *FormType {
  return @ptrCast(*FormType, c.FrmGetActiveForm());
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

pub fn setControlValue(formP: *FormType, objIndex: u16, newValue: i16) void {
  c.FrmSetControlValue(formP, objIndex, newValue);
}

pub fn setControlGroupSelection(formP: *FormType, groupNum: u8, controlID: u16) void {
  c.FrmSetControlGroupSelection(formP, groupNum, controlID);
}

pub fn getControlGroupSelection(formP: *FormType, groupNum: u8) u16 {
  return c.FrmGetControlGroupSelection(formP, groupNum);
}

pub fn getControl(formP: *FormType, controlId: u16) ?*ControlType {
  var objIndex: u16 = c.FrmGetObjectIndex(formP, controlId);
  if (objIndex == 0xffff) return null;
  var objTypeU8: u8 = c.FrmGetObjectType(formP, objIndex);
  var objType: formObjects = @intToEnum(formObjects, objTypeU8);
  if (objType != formObjects.controlObj) return null;
  return @ptrCast(*ControlType, c.FrmGetObjectPtr(formP, objIndex));
}

pub fn nullEventHandler(event: *pumpkin.EventType) bool {
  _ = event;
  return false;
}

pub fn eventLoop(appFormMapper: formMapperFn) void {
  var event = pumpkin.EventType {};

  while (true) {
    pumpkin.Evt.getEvent(&event, pumpkin.Evt.waitForever);
    if (pumpkin.Sys.handleEvent(&event)) continue;
    if (pumpkin.Menu.handleEvent(&event)) continue;
    if (event.eType == pumpkin.eventTypes.frmLoad) {
      var eventHandler = appFormMapper(event.data.frmLoad.formID);
      if (eventHandler != nullEventHandler) {
        var formP = initForm(event.data.frmLoad.formID);
        setActiveForm(formP);
        setEventHandler(formP, eventHandler);
        continue;
      }
    }
    _ = dispatchEvent(&event); // ignore return value
    if (event.eType == pumpkin.eventTypes.appStop) break;
  }
}

pub fn normalLaunchMain(firstForm: u16, appFormMapper: formMapperFn) void {
  gotoForm(firstForm);
  eventLoop(appFormMapper);
  closeAllForms();
}
