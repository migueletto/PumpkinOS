const std = @import("std");

const c = @cImport({
    @cInclude("zigpumpkin.h");
    @cDefine("DEBUG_INFO", "1");
});

export fn CommandMain(c_argc: c_int, c_argv: [*][*]u8) c_int {
    const argc = c_argc;
    const argv = c_argv;

    var i: u32 = 0;
    while (i < argc) {
        c.debug_full("", "", 0, c.DEBUG_INFO, "zig", "arg %d: %s", i, argv[i]);
        c.pumpkin_puts(argv[i]);
        c.pumpkin_puts("\r\n");
        i += 1;
    }

    if (argc > 1) {
      _ = c.FrmCustomAlert(10024, argv[1], "", "");
    }

    return 0;
}
