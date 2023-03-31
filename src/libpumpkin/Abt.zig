const pumpkin = @import("pumpkin.zig");
const EventType = pumpkin.EventType;

const c = @cImport({
  @cInclude("zigpumpkin.h");
});

pub fn showAboutPumpkin() void {
  var creator: u32 = c.pumpkin_get_app_creator();
  c.AbtShowAboutPumpkin(creator);
}
