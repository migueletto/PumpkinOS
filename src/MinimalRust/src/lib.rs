#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(unused_imports)]
#![allow(unused_macros)]

use std::ffi::c_void;
use std::mem;

#[path = "../../libpit/libpit.rs"]
pub mod libpit;
use libpit::*;

#[path = "../../libpumpkin/libpumpkin.rs"]
pub mod libpumpkin;
use libpumpkin::*;

const mainForm: u16 = 1000;
const aboutCmd: u16 = 1;
const clickButton: u16 = 2000;

#[unsafe(no_mangle)]
extern "C" fn MainFormHandleEvent(event: &EventType) -> bool {
  let handled: bool;

  match event.eType {
    EventEnum::frmOpenEvent => {
      let frm = frm_get_active_form();
      frm_draw_form(frm);
      handled = true;
    },
    EventEnum::menuEvent => {
      let itemID: u16;
      unsafe { itemID = event.data.menu.itemID; }
      if itemID == aboutCmd {
        show_about_pumpkin();
      }
      handled = true;
    },
    EventEnum::ctlSelectEvent => {
      let controlID: u16;
      unsafe { controlID = event.data.ctlSelect.controlID; }
      if controlID == clickButton {
        frm_custom_alert(informationOkAlert, "For information on the Rust language, check https://rustlang.org/", "", "");
      }
      handled = true;
    },
    _ => { handled = false; }
  }

  return handled;
}

fn app_handle_event(event: &EventType) -> bool {
  let handled: bool;

  match event.eType {
    EventEnum::frmLoadEvent => {
      let formID: u16;
      unsafe { formID = event.data.frmLoad.formID; }
      let frm = frm_init_form(formID);
      frm_set_active_form(frm);
      frm_set_event_handler(frm, MainFormHandleEvent);
      handled = true;
    },
    _ => { handled = false; }
  }
  
  return handled;
}

fn event_loop() {
  loop {
    let event: EventType = evt_get_event(waitForever);

    if !sys_handle_event(&event) && !menu_handle_event(&event) && !app_handle_event(&event) {
      frm_dispatch_event(&event);
      if event.eType == EventEnum::appStopEvent {
        break;
      }
    }
  }
}

#[unsafe(no_mangle)]
pub extern "C" fn PilotMain(cmd: u16, _cmd_pbp: *mut c_void, _launch_flags: u16) -> i32 {
  if cmd == sysAppLaunchCmdNormalLaunch {
    frm_goto_form(mainForm);
    event_loop();
  }

  return 0;
}
