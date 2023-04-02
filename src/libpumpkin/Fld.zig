const pumpkin = @import("pumpkin.zig");
const FieldType = pumpkin.FieldType;
const MemHandle = pumpkin.MemHandle;

const c = @cImport({
  @cInclude("zigpumpkin.h");
});

pub fn setText(fldP: *FieldType, textHandle: MemHandle, offset: u16, size: u16) void {
  c.FldSetText(fldP, textHandle, offset, size);
}

pub fn insert(fldP: *FieldType, insertChars: [*]const u8, insertLen: u16) bool {
  return c.FldInsert(fldP, insertChars, insertLen) != 0;
}

pub fn delete(fldP: *FieldType, start: u16, end: u16) void {
  c.FldDelete(fldP, start, end);
}
