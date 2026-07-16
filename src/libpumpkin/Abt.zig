const pumpkin = @import("pumpkin.zig");
const EventType = pumpkin.EventType;

const c = @import("c");

pub fn showAboutPumpkin() void {
  const creator: u32 = c.pumpkin_get_app_creator();
  c.AbtShowAboutPumpkin(creator);
}
