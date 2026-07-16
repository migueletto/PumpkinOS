const pumpkin = @import("pumpkin.zig");
const MemHandle = pumpkin.MemHandle;

const c = @import("c");

pub fn getResource(resType: u32, resID: u16) MemHandle {
  return @ptrCast(c.DmGetResource(resType, resID));
}

pub fn releaseResource(handle: MemHandle) u16 {
  return c.DmReleaseResource(handle);
}
