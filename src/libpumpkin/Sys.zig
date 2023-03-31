const pumpkin = @import("pumpkin.zig");
const EventType = pumpkin.EventType;

const c = @cImport({
  @cInclude("zigpumpkin.h");
});

pub fn handleEvent(event: *EventType) bool {
  return c.SysHandleEvent(event) != 0;
}
