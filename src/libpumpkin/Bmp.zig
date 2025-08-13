const pumpkin = @import("pumpkin.zig");
const BitmapType = pumpkin.BitmapType;
const MemHandle = pumpkin.MemHandle;
const Dm = pumpkin.Dm;

const c = @cImport({
  @cInclude("zigpumpkin.h");
});

fn isBitmap(handle: MemHandle) bool {
  var resType: u32 = 0;
  var resID: u16 = 0;
  return c.DmResourceType(handle, &resType, &resID) == 0 and resType == pumpkin.bitmapRsc;
}

pub fn get(resID: u16) MemHandle {
  const handle = Dm.getResource(pumpkin.bitmapRsc, resID);
  if (isBitmap(handle)) {
    return handle;
  }
  _ = Dm.releaseResource(handle);
  return null;
  
}

pub fn release(handle: MemHandle) u16 {
  return Dm.releaseResource(handle);
}

pub fn lock(handle: MemHandle) ?*BitmapType {
  return if (isBitmap(handle)) @ptrCast(c.MemHandleLock(handle)) else null;
}

pub fn unlock(handle: MemHandle) void {
  pumpkin.Mem.handleUnlock(handle);
}

pub fn paint(handle: MemHandle, x: i16, y: i16) void {
  if (isBitmap(handle)) {
    const bmp = c.MemHandleLock(handle);
    c.WinPaintBitmap(bmp, x, y);
    c.MemHandleUnlock(handle);
  }
}

pub fn dimensions(handle: MemHandle, width: *i16, height: *i16) void {
  if (isBitmap(handle)) {
    const bmp = c.MemHandleLock(handle);
    var rowBytes: u16 = 0;
    c.BmpGetDimensions(bmp, width, height, &rowBytes);
    c.MemHandleUnlock(handle);
  }
}
