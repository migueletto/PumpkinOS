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

pub fn getValue(controlP: *ControlType) u16 {
  return c.CtlGetValue(controlP);
}

pub fn setValue(controlP: *ControlType, newValue: u16) void {
  c.CtlSetValue(controlP, newValue);
}

pub fn setSliderValues(controlP: *ControlType, minValue: *u16, maxValue: *u16, pageSize: *u16, value: *u16) void {
  c.CtlSetSliderValues(controlP, minValue, maxValue, pageSize, value);
}

pub fn getSliderValues(controlP: *ControlType, minValue: *u16, maxValue: *u16, pageSize: *u16, value: *u16) void {
  c.CtlGetSliderValues(controlP, minValue, maxValue, pageSize, value);
}

pub fn getLabel(controlP: *ControlType) [*]const u8 {
  return c.CtlGetLabel(controlP);
}

pub fn setLabel(controlP: *ControlType, newLabel: [*]const u8) void {
  c.CtlSetLabel(controlP, newLabel);
}

pub fn updateGroup(controlP: *ControlType, value: bool) void {
  c.CtlUpdateGroup(controlP, if (value) 1 else 0);
}

pub fn setGraphics(controlP: *ControlType, newBitmapID: u16, newSelectedBitmapID: u16) void {
  c.CtlSetGraphics(controlP, newBitmapID, newSelectedBitmapID);
}
