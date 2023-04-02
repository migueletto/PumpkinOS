// module "pumpkin" is the bridge between Zig and PumpkinOS
const pumpkin = @import("pumpkin");
const Frm = pumpkin.Frm;

const std = @import("std");

const mainForm: u16 = 1000;
const aboutCmd: u16 = 1;
const clickButton: u16 = 2000;

fn controlHandler(event: *pumpkin.EventType) bool {
  if (event.data.ctlSelect.controlID == clickButton) {
    _ = Frm.customAlert(Frm.informationOkAlert,
          "For information on the Zig language, check https://ziglang.org/", "", "");
    return true;
  }
  return false;
}

fn menuHandler(event: *pumpkin.EventType) bool {
  if (event.data.menu.itemID == aboutCmd) {
    pumpkin.Abt.showAboutPumpkin();
    return true;
  }
  return false;
}

fn mainFormEventHandler(event: *pumpkin.EventType) bool {
  return switch (event.eType) {
    pumpkin.eventTypes.frmOpen   => Frm.simpleFrmOpenHandler(),
    pumpkin.eventTypes.ctlSelect => controlHandler(event),
    pumpkin.eventTypes.menu      => menuHandler(event),
    else => false,
  };
}

export fn PilotMain(cmd: c_ushort, cmdPBP: *void, launchFlags: c_ushort) c_uint {
  // convert cmd argument to enum launchCodes
  var launchCode = @intToEnum(pumpkin.launchCodes, cmd);
  _ = cmdPBP; // not used
  _ = launchFlags; // not used

  if (launchCode == pumpkin.launchCodes.normalLaunch) {
    var map = Frm.FormMap.init(pumpkin.PumpkinAllocator);
    defer map.deinit();
    map.put(mainForm, mainFormEventHandler) catch { return 0; };
    Frm.normalLaunchMain(mainForm, &map, pumpkin.Evt.waitForever);
  }

  return 0;
}
