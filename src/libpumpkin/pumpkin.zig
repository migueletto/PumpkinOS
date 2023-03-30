const c = @cImport({
  @cInclude("zigpumpkin.h");
});

pub const lauchCodesEnum = enum(u16) {
  sysAppLaunchCmdNormalLaunch,
  sysAppLaunchCmdFind,
  sysAppLaunchCmdGoTo,
  sysAppLaunchCmdSyncNotify,
};

pub const evtWaitForever: i32 = -1;

pub const eventsEnum = enum(u16) {
  nilEvent = 0,
  penDownEvent,
  penUpEvent,
  penMoveEvent,
  keyDownEvent,
  winEnterEvent,
  winExitEvent,
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
  appStopEvent = 22,
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
  tsmConfirmEvent = 35,
  tsmFepButtonEvent,
  tsmFepModeEvent,
  attnIndicatorEnterEvent,
  attnIndicatorSelectEvent,
  
  menuCmdBarOpenEvent = 0x0800,
  menuOpenEvent,
  menuCloseEvent,
  frmGadgetEnterEvent,
  frmGadgetMiscEvent,

  firstINetLibEvent = 0x1000,
  firstWebLibEvent = 0x1100,
  
  telAsyncReplyEvent = 0x1200, 

  keyUpEvent = 0x4000,
  keyHoldEvent = 0x4001,
  frmObjectFocusTakeEvent = 0x4002,
  frmObjectFocusLostEvent = 0x4003,

  winDisplayChangedEvent = 0x4101,
  appRaiseEvent = 0x4102,

  firstLicenseeEvent = 0x5000,
  lastLicenseeEvent = 0x5FFF,

  firstUserEvent = 0x6000,
  
  lastUserEvent  = 0x7FFF,
};

pub const PointType = extern struct {
  x: u16,
  y: u16,
};

pub const RectangleType = extern struct {
  topLeft: PointType,
  extent: PointType,
};

pub const WindowType = opaque {};
pub const ControlType = opaque {};
pub const FieldType = opaque {};
pub const ListType = opaque {};
pub const TableType = opaque {};
pub const ScrollBarType = opaque {};
pub const FormGadgetType = opaque {};
pub const FormType = opaque {};

const FormEventHandlerType = *const fn(eventP: *EventType) bool;

pub const GenericEventType = extern struct {
  datum: [8]u16,
};

pub const PenUpEventType = extern struct {
  start: PointType,
  end: PointType,
};

pub const KeyDownEventType = extern struct {
  chr: u16,
  keyCode: u16,
  modifiers: u16,
};

pub const KeyUpEventType = extern struct {
  chr: u16,
  keyCode: u16,
  modifiers: u16,
};

pub const WinEnterEventType = extern struct {
  enterWindow: *WindowType,
  exitWindow: *WindowType,
};

pub const WinExitEventType = extern struct {
  enterWindow: *WindowType,
  exitWindow: *WindowType,
};

pub const CtlEnterEventType = extern struct {
  controlID: u16,
  pControl: *ControlType,
};

pub const CtlSelectEventType = extern struct {
  controlID: u16,
  pControl: *ControlType,
  on: u8,
  reserved1: u8,
  value: u16,
};

pub const CtlRepeatEventType = extern struct {
  controlID: u16,
  pControl: *ControlType,
  time: u32,
  value: u16,
};

pub const CtlExitEventType = extern struct {
  controlID: u16,
  pControl: *ControlType,
};

pub const FldHeightChangedEventType = extern struct {
  fieldID: u16,
  pField: *FieldType,
  newHeight: i16,
  currentPos: u16,
};

pub const FldChangedEventType = extern struct {
  fieldID: u16,
  pField: *FieldType,
};

pub const FldExitEventType = extern struct {
  fieldID: u16,
  pField: *FieldType,
};

pub const LstEnterEventType = extern struct {
  listID: u16,
  pList: *ListType,
  selection: i16,
};

pub const LstExitEventType = extern struct {
  listID: u16,
  pList: *ListType,
};

pub const LstSelectEventType = extern struct {
  listID: u16,
  pList: *ListType,
  selection: i16,
};

pub const TblEnterEventType = extern struct {
  tableID: u16,
  pTable: *TableType,
  row: i16,
  column: i16,
};

pub const TblExitEventType = extern struct {
  tableID: u16,
  pTable: *TableType,
  row: i16,
  column: i16,
};

pub const TblSelectEventType = extern struct {
  tableID: u16,
  pTable: *TableType,
  row: i16,
  column: i16,
};

pub const FrmLoadEventType = extern struct {
  formID: u16,
};

pub const FrmOpenEventType = extern struct {
  formID: u16,
};

pub const FrmGotoEventType = extern struct {
  formID: u16,
  recordNum: u16,
  matchPos: u16,
  matchLen: u16,
  matchFieldNum: u16,
  matchCustom: u32,
};

pub const FrmCloseEventType = extern struct {
  formID: u16,
};

pub const FrmUpdateEventType = extern struct {
  formID: u16,
  updateCode: u16,
};

pub const FrmTitleEnterEventType = extern struct {
  formID: u16,
};

pub const FrmTitleSelectEventType = extern struct {
  formID: u16,
};

pub const MenuEventType = extern struct {
  itemID: u16,
};

pub const PopSelectEventType = extern struct {
  controlID: u16,
  controlP: *ControlType,
  listID: u16,
  listP: *ListType,
  selection: i16,
  priorSelection: i16,
};

pub const SclEnterEventType = extern struct {
  scrollBarID: u16,
  pScrollBar: *ScrollBarType,
};

pub const SclExitEventType = extern struct {
  scrollBarID: u16,
  pScrollBar: *ScrollBarType,
  value: i16,
  newValue: i16,
};

pub const SclRepeatEventType = extern struct {
  scrollBarID: u16,
  pScrollBar: *ScrollBarType,
  value: i16,
  newValue: i16,
  time: i32,
};

pub const MenuCmdBarOpenEventType = extern struct {
  preventFieldButtons: u8,
  reserved: u8,
};

pub const MenuOpenEventType = extern struct {
  menuRscID: u16,
  cause: i16,
};

pub const GadgetEnterEventType = extern struct {
  gadgetID: u16,
  gadgetP: *FormGadgetType,
};

pub const GadgetMiscEventType = extern struct {
  gadgetID: u16,
  gadgetP: *FormGadgetType,
  selector: u16,
  dataP: *void,
};

pub const WinDisplayChangedEventType = extern struct {
  newBounds: RectangleType,
};

pub const EventData = extern union {
  dummy: u32,
  generic: GenericEventType,
  penUp: PenUpEventType,
  keyDown: KeyDownEventType,
  keyUp: KeyUpEventType,
  winEnter: WinEnterEventType,
  winExit: WinExitEventType,
  ctlEnter: CtlEnterEventType,
  ctlSelect: CtlSelectEventType,
  ctlRepeat: CtlRepeatEventType,
  ctlExit: CtlExitEventType,
  fldHeightChanged: FldHeightChangedEventType,
  fldChanged: FldChangedEventType,
  fldExit: FldExitEventType,
  lstEnter: LstEnterEventType,
  lstSelect: LstSelectEventType,
  lstExit: LstExitEventType,
  tblEnter: TblEnterEventType,
  tblExit: TblExitEventType,
  tblSelect: TblSelectEventType,
  frmLoad: FrmLoadEventType,
  frmOpen: FrmOpenEventType,
  frmGoto: FrmGotoEventType,
  frmUpdate: FrmUpdateEventType,
  frmClose: FrmCloseEventType,
  frmTitleEnter: FrmTitleEnterEventType,
  frmTitleSelect: FrmTitleSelectEventType,
  menu: MenuEventType,
  popSelect: PopSelectEventType,
  sclEnter: SclEnterEventType,
  sclExit: SclExitEventType,
  sclRepeat: SclRepeatEventType,
  menuCmdBarOpen: MenuCmdBarOpenEventType,
  menuOpen: MenuOpenEventType,
  gadgetEnter: GadgetEnterEventType,
  gadgetMisc: GadgetMiscEventType,
  winDisplayChanged: WinDisplayChangedEventType,
};

pub const EventType = extern struct {
  eType: eventsEnum = eventsEnum.nilEvent,
  dummy1: u16 = 0,
  penDown: u8 = 0,
  tapCount: u8 = 0,
  screenX: i16 = 0,
  screenY: i16 = 0,
  data: EventData = EventData { .dummy = 0 },
};

pub fn EvtGetEvent(event: *EventType, timeout: i32) void {
  c.EvtGetEvent(event, timeout);
  //c.debug_full("", "", 0, 1, "zig", "EvtGetEvent offset eType    %d", @intCast(i32, @offsetOf(EventType, "eType")));
  //c.debug_full("", "", 0, 1, "zig", "EvtGetEvent offset penDown  %d", @intCast(i32, @offsetOf(EventType, "penDown")));
  //c.debug_full("", "", 0, 1, "zig", "EvtGetEvent offset tapCount %d", @intCast(i32, @offsetOf(EventType, "tapCount")));
  //c.debug_full("", "", 0, 1, "zig", "EvtGetEvent offset screenX  %d", @intCast(i32, @offsetOf(EventType, "screenX")));
  //c.debug_full("", "", 0, 1, "zig", "EvtGetEvent offset screenY  %d", @intCast(i32, @offsetOf(EventType, "screenY")));
  //c.debug_full("", "", 0, 1, "zig", "EvtGetEvent offset data     %d", @intCast(i32, @offsetOf(EventType, "data")));
}

pub fn SysHandleEvent(event: *EventType) bool {
  return c.SysHandleEvent(event) != 0;
}

pub fn MenuHandleEvent(event: *EventType) bool {
  return c.MenuHandleEvent(null, event, null) != 0;
}

pub fn FrmDispatchEvent(event: *EventType) bool {
  return c.FrmDispatchEvent(event) != 0;
}

pub fn FrmGotoForm(formId: u16) void {
  c.FrmGotoForm(formId);
}

pub fn FrmInitForm(formId: u16) *FormType {
  return @ptrCast(*FormType, c.FrmInitForm(formId));
}

pub fn FrmSetActiveForm(formP: *FormType) void {
  c.FrmSetActiveForm(formP);
}

pub fn FrmCloseAllForms() void {
  c.FrmCloseAllForms();
}

pub fn FrmSetEventHandler(formP: *FormType, handler: FormEventHandlerType) void {
  c.FrmSetEventHandler(formP, @ptrCast(*void, @constCast(handler)));
}

pub fn FrmGetActiveForm() *FormType {
  return @ptrCast(*FormType, c.FrmGetActiveForm());
}

pub fn FrmDrawForm(formP: *FormType) void {
  c.FrmDrawForm(formP);
}

pub fn AbtShowAboutPumpkin(creator: u32) void {
  c.AbtShowAboutPumpkin(creator);
}
