const pumpkin = @import("pumpkin.zig");

const c = @cImport({
  @cInclude("zigpumpkin.h");
});

pub fn dimensions(width: *i16, height: *i16) void {
  c.WinGetDisplayExtent(width, height);
}
