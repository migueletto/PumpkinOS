const std = @import("std");
const Allocator = std.mem.Allocator;
const assert = std.debug.assert;

const c = @cImport({
  @cInclude("zigpumpkin.h");
});

pub const launchCodes = enum(u16) {
  normalLaunch,
  find,
  goTo,
  syncNotify,
  timeChange,
  systemReset,
  alarmTriggered,
  displayAlarm,
  countryChange,
  syncRequestLocal,
  saveData,
  initDatabase,
  syncCallApplicationV10,
  panelCalledFromApp,
  returnFromPanel,
  lookup,
  systemLock,
  syncRequestRemote,
  handleSyncCallApp,
  addRecord,
  setServiceID,
  getServiceID,
  getServiceList,
  getServiceInfo,
  failedAppNotify,
  eventHook,
  exgReceiveData,
  exgAskUser,
};

pub const eventTypes = enum(u16) {
  nil = 0,
  penDown,
  penUp,
  penMove,
  keyDown,
  winEnter,
  winExit,
  ctlEnter,
  ctlExit,
  ctlSelect,
  ctlRepeat,
  lstEnter,
  lstSelect,
  lstExit,
  popSelect,
  fldEnter,
  fldHeightChanged,
  fldChanged,
  tblEnter,
  tblSelect,
  daySelect,
  menu,
  appStop = 22,
  frmLoad,
  frmOpen,
  frmGoto,
  frmUpdate,
  frmSave,
  frmClose,
  frmTitleEnter,
  frmTitleSelect,
  tblExit,
  sclEnter,
  sclExit,
  sclRepeat,
  tsmConfirm = 35,
  tsmFepButton,
  tsmFepMode,
  attnIndicatorEnter,
  attnIndicatorSelect,
  
  menuCmdBarOpen = 0x0800,
  menuOpen,
  menuClose,
  frmGadgetEnter,
  frmGadgetMisc,

  firstINetLib = 0x1000,
  firstWebLib = 0x1100,
  
  telAsyncReply = 0x1200, 

  keyUp = 0x4000,
  keyHold = 0x4001,
  frmObjectFocusTake = 0x4002,
  frmObjectFocusLost = 0x4003,

  winDisplayChanged = 0x4101,
  appRaise = 0x4102,

  firstLicensee = 0x5000,
  lastLicensee = 0x5FFF,

  firstUser = 0x6000,
  
  lastUser = 0x7FFF,
};

pub const PointType = extern struct {
  x: u16,
  y: u16,
};

pub const RectangleType = extern struct {
  topLeft: PointType,
  extent: PointType,
};

pub const MemHandle = *opaque {};
pub const WindowType = opaque {};
pub const ControlType = opaque {};
pub const FieldType = opaque {};
pub const ListType = opaque {};
pub const TableType = opaque {};
pub const ScrollBarType = opaque {};
pub const GadgetType = opaque {};
pub const FormType = opaque {};

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
  gadgetP: *GadgetType,
};

pub const GadgetMiscEventType = extern struct {
  gadgetID: u16,
  gadgetP: *GadgetType,
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
  eType: eventTypes = eventTypes.nil,
  dummy1: u16 = 0,
  penDown: u8 = 0,
  tapCount: u8 = 0,
  screenX: i16 = 0,
  screenY: i16 = 0,
  data: EventData = EventData { .dummy = 0 },
};

pub const Sys  = @import("Sys.zig");
pub const Evt  = @import("Evt.zig");
pub const Frm  = @import("Frm.zig");
pub const Ctl  = @import("Ctl.zig");
pub const Fld  = @import("Fld.zig");
pub const Abt  = @import("Abt.zig");
pub const Menu = @import("Menu.zig");

pub const DEBUG_ERROR: i32 = 0;
pub const DEBUG_INFO:  i32 = 1;
pub const DEBUG_TRACE: i32 = 2;

pub fn debug(level: i32, sys: [*]const u8, comptime format: []const u8, args: anytype) void {
  var buf: [1024]u8 = undefined;
  // formats the message into a zero terminated C-string
  const slice = std.fmt.bufPrintZ(&buf, format, args) catch { return; };
  // and call debug C function on PumpkinOS
  c.debug_full("", "", 0, level, sys, "%s", slice.ptr);
}

pub const PumpkinAllocator = Allocator {
  .ptr = undefined,
  .vtable = &pumpkin_allocator_vtable,
};

const pumpkin_allocator_vtable = Allocator.VTable {
  .alloc  = PumpkinAllocatorVTable.alloc,
  .resize = PumpkinAllocatorVTable.resize,
  .free   = PumpkinAllocatorVTable.free,
};

const PumpkinAllocatorVTable = struct {
  fn alloc(_: *anyopaque, len: usize, ptr_align: u8, ret_addr: usize) ?[*]u8 {
    _ = ptr_align;
    _ = ret_addr;
    debug(DEBUG_INFO, "ZigAlloc", "alloc {d}", .{ len });
    assert(len > 0);
    return c.pumpkin_heap_alloc(@intCast(u32, len), "zig");
  }

  fn resize(_: *anyopaque, buf: []u8, buf_align: u8, new_len: usize, ret_addr: usize) bool {
    _ = buf_align;
    _ = ret_addr;
    debug(DEBUG_INFO, "ZigAlloc", "resize {d} -> {d}", .{ buf.len, new_len });
    return if (new_len <= buf.len) true else false;
  }

  fn free(_: *anyopaque, buf: []u8, buf_align: u8, ret_addr: usize) void {
    _ = buf_align;
    _ = ret_addr;
    debug(DEBUG_INFO, "ZigAlloc", "free", .{});
    c.pumpkin_heap_free(buf.ptr, "zig");
  }
};
