const pumpkin = @import("pumpkin.zig");
const EventType = pumpkin.EventType;

const c = @import("c");

pub fn handleEvent(event: *EventType) bool {
  return c.SysHandleEvent(event) != 0;
}
