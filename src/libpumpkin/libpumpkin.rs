use std::ffi::CString;

pub const sysAppLaunchCmdNormalLaunch: u16 = 0;

pub const informationOkAlert: u16 = 10024;

pub const waitForever: i32 = -1;

#[derive(PartialEq, Eq, Debug)]
#[repr(u16)]
pub enum EventEnum {
  nilEvent = 0,                           // system level
  penDownEvent,                           // system level
  penUpEvent,                             // system level
  penMoveEvent,                           // system level
  keyDownEvent,                           // system level
  winEnterEvent,                          // system level
  winExitEvent,                           // system level
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
  appStopEvent = 22,                      // system level
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

  tsmConfirmEvent = 35,           // system level
  tsmFepButtonEvent,              // system level
  tsmFepModeEvent,                // system level
  attnIndicatorEnterEvent,        // for attention manager's indicator
  attnIndicatorSelectEvent,       // for attention manager's indicator

  // add future UI level events in this numeric space
  // to save room for new system level events
  menuCmdBarOpenEvent = 0x0800,
  menuOpenEvent,
  menuCloseEvent,
  frmGadgetEnterEvent,
  frmGadgetMiscEvent,

  // <chg 2-25-98 RM> Equates added for library events
  firstINetLibEvent = 0x1000,
  firstWebLibEvent = 0x1100,

  // GFa, 07/20/01 : integrated the bellagio telephony events.
  telAsyncReplyEvent = 0x1200,

  // Can't add these to the system event range because PACE won't pass them through,
  // add them to the licensee range here:
  keyUpEvent              = 0x4000,
  keyHoldEvent            = 0x4001,
  frmObjectFocusTakeEvent = 0x4002,
  frmObjectFocusLostEvent = 0x4003,

  winDisplayChangedEvent  = 0x4101,       // defined below for compatibility
  appRaiseEvent = 0x4102,
  penDownRightEvent = 0x4103,
  modKeyDownEvent = 0x4104,
  modKeyUpEvent = 0x4105,
  appWidgetEvent = 0x4106,

  // BGT, 06/24/2003 Clarify the range reserved for licensees
  firstLicenseeEvent      = 0x5000,
  lastLicenseeEvent       = 0x5FFF,

  // <chg 10/9/98 SCL> Changed firstUserEvent from 32767 (0x7FFF) to 0x6000
  // Enums are signed ints, so 32767 technically only allowed for ONE event.
  firstUserEvent = 0x6000,

  lastUserEvent  = 0x7FFF
}

#[derive(Debug, Copy, Clone)]
#[repr(C)]
pub struct CtlSelectData {
  pub controlID: u16
}

#[derive(Debug, Copy, Clone)]
#[repr(C)]
pub struct FrmLoadData {
  pub formID: u16
}

#[derive(Debug, Copy, Clone)]
#[repr(C)]
pub struct MenuData {
  pub itemID: u16
}

#[repr(C)]
pub union EventData {
  pub ctlSelect: CtlSelectData,
  pub frmLoad: FrmLoadData,
  pub menu: MenuData,
  buffer: [u8; 24]
}

#[repr(C)]
pub struct EventType {
  pub eType: EventEnum,
  pub penDown: bool,
  pub tapCount: u8,
  pub screenX: i16,
  pub screenY: i16,
  _pad: [u8; 8],
  pub data: EventData
}

#[repr(C)]
pub struct FormType {
  _private: [u8; 0]
}

#[link(name = "pumpkin", kind = "dylib")]
unsafe extern "C" {
  fn FrmGotoForm(form_id: u16);
  fn EvtGetEvent(event: &mut EventType, timeout: i32);
  fn SysHandleEvent(event: &EventType) -> bool;
  fn MenuHandleEvent(a: *const std::ffi::c_void, event: &EventType, err: &mut i16) -> bool;
  fn FrmDispatchEvent(event: &EventType) -> bool;
  fn FrmSetEventHandler(frm: *mut FormType, f: extern "C" fn(&EventType) -> bool);
  fn FrmInitForm(formId: u16) -> *mut FormType;
  fn FrmDrawForm(frm: *mut FormType);
  fn FrmGetActiveForm() -> *mut FormType;
  fn FrmSetActiveForm(frm: *mut FormType);
  fn FrmCustomAlert(alertID: u16, s1: *const i8, s2: *const i8, s3: *const i8);
  fn AbtShowAboutPumpkin(creator: u32);
  fn pumpkin_get_app_creator() -> u32;
}

// to show the bytes of a EventType struct:
// let buf = &event as *const EventType as *const u8;
// debug_bytes(1, "Rust", buf, 32);

pub fn evt_get_event(timeout: i32) -> EventType {
  let mut event: EventType = unsafe { std::mem::zeroed() };

  unsafe {
    EvtGetEvent(&mut event, timeout);
  }

  return event;
}

pub fn sys_handle_event(event: &EventType) -> bool {
  let r: bool;

  unsafe {
    r = SysHandleEvent(event);
  }

  return r;
}

pub fn menu_handle_event(event: &EventType) -> bool {
  let r: bool;
  let mut err: i16 = 0;

  unsafe {
    r = MenuHandleEvent(std::ptr::null(), event, &mut err);
  }

  return r;
}

pub fn frm_goto_form(form_id: u16) {
  unsafe {
    FrmGotoForm(form_id);
  }
}

pub fn frm_dispatch_event(event: &EventType) -> bool {
  let r: bool;

  unsafe {
    r = FrmDispatchEvent(event);
  }

  return r;
}

pub fn frm_init_form(formId: u16) -> *mut FormType {
  let frm: *mut FormType;

  unsafe {
    frm = FrmInitForm(formId);
  }

  return frm;
}

pub fn frm_draw_form(frm: *mut FormType) {
  unsafe {
    FrmDrawForm(frm);
  }
}

pub fn frm_get_active_form() -> *mut FormType {
  let frm: *mut FormType;

  unsafe {
    frm = FrmGetActiveForm();
  }

  return frm;
}

pub fn frm_set_active_form(frm: *mut FormType) {
  unsafe {
    FrmSetActiveForm(frm);
  }
}

pub fn frm_set_event_handler(frm: *mut FormType, f: extern "C" fn(&EventType) -> bool) {
  unsafe {
    FrmSetEventHandler(frm, f);
  }
}

pub fn frm_custom_alert(alertID: u16, s1: &str, s2: &str, s3: &str) {
  let s1_cs = CString::new(s1).unwrap_or(CString::new("").unwrap());
  let s2_cs = CString::new(s2).unwrap_or(CString::new("").unwrap());
  let s3_cs = CString::new(s3).unwrap_or(CString::new("").unwrap());
  unsafe {
    FrmCustomAlert(alertID, s1_cs.as_ptr(), s2_cs.as_ptr(), s3_cs.as_ptr());
  }
}

pub fn show_about_pumpkin() {
  unsafe {
    let creator = pumpkin_get_app_creator();
    AbtShowAboutPumpkin(creator);
  }
}
