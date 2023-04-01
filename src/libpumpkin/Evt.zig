const pumpkin = @import("pumpkin.zig");
const EventType = pumpkin.EventType;

const c = @cImport({
  @cInclude("zigpumpkin.h");
});

pub const waitForever: i32 = -1;

pub fn getEvent(event: *EventType, timeout: i32) void {
  c.EvtGetEvent(event, timeout);
  //c.debug_full("", "", 0, 1, "zig", "EvtGetEvent offset eType    %d", @intCast(i32, @offsetOf(EventType, "eType")));
  //c.debug_full("", "", 0, 1, "zig", "EvtGetEvent offset penDown  %d", @intCast(i32, @offsetOf(EventType, "penDown")));
  //c.debug_full("", "", 0, 1, "zig", "EvtGetEvent offset tapCount %d", @intCast(i32, @offsetOf(EventType, "tapCount")));
  //c.debug_full("", "", 0, 1, "zig", "EvtGetEvent offset screenX  %d", @intCast(i32, @offsetOf(EventType, "screenX")));
  //c.debug_full("", "", 0, 1, "zig", "EvtGetEvent offset screenY  %d", @intCast(i32, @offsetOf(EventType, "screenY")));
  //c.debug_full("", "", 0, 1, "zig", "EvtGetEvent offset data     %d", @intCast(i32, @offsetOf(EventType, "data")));
}

pub fn addEventToQueue(event: *EventType) void {
  c.EvtAddEventToQueue(event);
}

pub fn copyEvent(source: *EventType, dest: *EventType) void {
  c.EvtCopyEvent(source, dest);
}

pub fn eventAvailable() bool {
  return c.EvtEventAvail() != 0;
}
