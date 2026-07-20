use std::ffi::CString;

#[link(name = "pit", kind = "dylib")]
unsafe extern "C" {
  fn debugva_full(file: *const i8, func: *const i8, line: u32, level: u32, sys: *const i8, s: *const i8);
  fn debug_bytes_full(file: *const i8, func: *const i8, line: u32, level: u32, sys: *const i8, buf: *const u8, len: i32);
}

pub fn debug_str(level: u32, sys: &str, s: String) {
  let file_cs = CString::new(file!()).unwrap();
  let func_cs = CString::new("unknown").unwrap();
  let sys_cs = CString::new(sys).unwrap_or(CString::new("").unwrap());
  let s_cs = CString::new(s).unwrap_or(CString::new("").unwrap());
  unsafe {
    debugva_full(file_cs.as_ptr(), func_cs.as_ptr(), line!(), level, sys_cs.as_ptr(), s_cs.as_ptr());
  }
}

pub fn debug_bytes(level: u32, sys: &str, buf: *const u8, len: i32) {
  let file_cs = CString::new(file!()).unwrap();
  let func_cs = CString::new("unknown").unwrap();
  let sys_cs = CString::new(sys).unwrap_or(CString::new("").unwrap());
  unsafe {
    debug_bytes_full(file_cs.as_ptr(), func_cs.as_ptr(), line!(), level, sys_cs.as_ptr(), buf, len);
  }
}

macro_rules! debug {
  ($level:expr, $sys:expr, $($args:tt)*) => {
    debug_str($level, $sys, format!($($args)*));
  };
}

pub(crate) use debug;
