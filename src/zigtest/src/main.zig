const std = @import("std");

const pumpkin = @cImport({
    @cInclude("zigpumpkin.h");
    @cDefine("DEBUG_INFO", "1");
});

export fn CommandMain(c_argc: c_int, c_argv: [*c][*c]u8) c_int {
    var argc = c_argc;
    var argv = c_argv;

    var i: u32 = 0;
    while (i < argc) {
        pumpkin.debug_full("", "", 0, pumpkin.DEBUG_INFO, "zig", "arg %d: %s", i, argv[i]);
        i = i + 1;
    }

    return 0;
}
