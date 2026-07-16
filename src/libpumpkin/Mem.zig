const pumpkin = @import("pumpkin.zig");
const MemHandle = pumpkin.MemHandle;

const c = @import("c");

pub fn handleLock(handle: MemHandle) *void {
  return @ptrCast(c.MemHandleLock(handle));
}

pub fn handleUnlock(handle: MemHandle) void {
  c.MemHandleUnlock(handle);
}

pub fn handleSize(handle: MemHandle) u32 {
  return c.MemHandleSize(handle);
}
