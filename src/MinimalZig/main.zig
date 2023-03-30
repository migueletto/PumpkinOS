// module "pumpkin" is the bridge between Zig and PumpkinOS
const pumpkin = @import("pumpkin");

const mainForm: u16 = 1000;
const aboutCmd: u16 = 1;
const appID: u32 = 0x4d696e5a;

fn MainFormHandleEvent(event: *pumpkin.EventType) bool {
  return switch (event.eType) {
    pumpkin.eventsEnum.frmOpenEvent => blk: {
      var formP = pumpkin.FrmGetActiveForm();
      pumpkin.FrmDrawForm(formP);
      break :blk true;
    },
    pumpkin.eventsEnum.menuEvent => blk: {
      if (event.data.menu.itemID == aboutCmd) {
        pumpkin.AbtShowAboutPumpkin(appID);
      }
      break :blk true;
    },
    else => false
  };
}

fn ApplicationHandleEvent(event: *pumpkin.EventType) bool {
  return switch (event.eType) {
    pumpkin.eventsEnum.frmLoadEvent => blk: {
      if (event.data.frmLoad.formID == mainForm) {
        var formP = pumpkin.FrmInitForm(event.data.frmLoad.formID);
        pumpkin.FrmSetActiveForm(formP);
        pumpkin.FrmSetEventHandler(formP, MainFormHandleEvent);
      }
      break :blk true;
    },
    else => false
  };
}

fn EventLoop() void {
  // declare a default event (initialized to nilEvent)
  var event = pumpkin.EventType {};

  while (true) {
    pumpkin.EvtGetEvent(&event, pumpkin.evtWaitForever);
    if (pumpkin.SysHandleEvent(&event)) continue;
    if (pumpkin.MenuHandleEvent(&event)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    _ = pumpkin.FrmDispatchEvent(&event); // ignore return value
    if (event.eType == pumpkin.eventsEnum.appStopEvent) break;
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
