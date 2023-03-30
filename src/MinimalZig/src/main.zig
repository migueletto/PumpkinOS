const std = @import("std");
const pumpkin = @import("pumpkin");

const mainForm: u16 = 1000;
const aboutCmd: u16 = 1;
const appID: u32 = 0x4d696e5a;

fn MainFormHandleEvent(event: *pumpkin.EventType) bool {
  var handled = false;

  switch (event.eType) {
    pumpkin.eventsEnum.frmOpenEvent => {
      var formP = pumpkin.FrmGetActiveForm();
      pumpkin.FrmDrawForm(formP);
      handled = true;
    },
    pumpkin.eventsEnum.menuEvent => {
      if (event.data.menu.itemID == aboutCmd) {
        pumpkin.AbtShowAboutPumpkin(appID);
      }
    },
    else => {}
  }

  return handled;
}

fn ApplicationHandleEvent(event: *pumpkin.EventType) bool {
  var handled = false;

  switch (event.eType) {
    pumpkin.eventsEnum.frmLoadEvent => {
      if (event.data.frmLoad.formID == mainForm) {
        var formP = pumpkin.FrmInitForm(event.data.frmLoad.formID);
        pumpkin.FrmSetActiveForm(formP);
        pumpkin.FrmSetEventHandler(formP, MainFormHandleEvent);
        handled = true;
      }
    },
    else => {}
  }

  return handled;
}

fn EventLoop() void {
  // declare a default event (initialized to nilEvent)
  var event = pumpkin.EventType {};

  while (event.eType != pumpkin.eventsEnum.appStopEvent) {
    pumpkin.EvtGetEvent(&event, pumpkin.evtWaitForever);
    if (pumpkin.SysHandleEvent(&event)) continue;
    if (pumpkin.MenuHandleEvent(&event)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    _ = pumpkin.FrmDispatchEvent(&event);
  }
}

export fn PilotMain(cmd: c_ushort, cmdPBP: *void, launchFlags: c_ushort) c_uint {
  // convert cmd argument to lauchCodesEnum
  var launchCode = @intToEnum(pumpkin.lauchCodesEnum, cmd);
  _ = cmdPBP; // this argument is not used
  _ = launchFlags; // this argument is not used

  if (launchCode == pumpkin.lauchCodesEnum.sysAppLaunchCmdNormalLaunch) {
    pumpkin.FrmGotoForm(mainForm);
    EventLoop();
    pumpkin.FrmCloseAllForms();
  }

  return 0;
}
