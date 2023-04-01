// module "pumpkin" is the bridge between Zig and PumpkinOS
const pumpkin = @import("pumpkin");

const mainForm: u16 = 1000;
const aboutCmd: u16 = 1;

fn simpleFrmOpenHandler() bool {
  var formP = pumpkin.Frm.getActiveForm();
  pumpkin.Frm.drawForm(formP);
  return true;
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
    pumpkin.eventTypes.frmOpen => simpleFrmOpenHandler(),
    pumpkin.eventTypes.menu    => menuHandler(event),
    else => false
  };
}

fn formMapper(formId: u16) ?pumpkin.Frm.eventHandlerFn {
  return switch (formId) {
    mainForm => mainFormEventHandler,
    else => null,
  };
}

export fn PilotMain(cmd: c_ushort, cmdPBP: *void, launchFlags: c_ushort) c_uint {
  // convert cmd argument to enum launchCodes
  var launchCode = @intToEnum(pumpkin.launchCodes, cmd);
  _ = cmdPBP; // not used
  _ = launchFlags; // not used

  if (launchCode == pumpkin.launchCodes.normalLaunch) {
    pumpkin.Frm.normalLaunchMain(mainForm, formMapper);
  }

  return 0;
}
