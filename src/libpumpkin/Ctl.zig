const pumpkin = @import("pumpkin.zig");
const ControlType = pumpkin.ControlType;

const c = @cImport({
  @cInclude("zigpumpkin.h");
});

pub fn showControl(controlP: *ControlType) void {
  c.CtlShowControl(controlP);
}

pub fn hideControl(controlP: *ControlType) void {
  c.CtlHideControl(controlP);
}
