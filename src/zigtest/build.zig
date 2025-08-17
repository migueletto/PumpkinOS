const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const os = b.option([]const u8, "OS", "OS") orelse "";

    const mod = b.addModule("pumpkin", .{
        .root_source_file = b.path("../libpumpkin/pumpkin.zig"),
        .target = target,
    });

    const lib = b.addLibrary(.{
        .name = "zigtest",
        .linkage = .dynamic,
        .root_module = b.createModule(.{
            .root_source_file = b.path("main.zig"),
            .target = target,
            .optimize = optimize,
            .imports = &.{
                .{ .name = "pumpkin", .module = mod },
            },
        }),
    });

    lib.root_module.addIncludePath(b.path("../libpumpkin"));
    lib.root_module.addLibraryPath(b.path("../../bin"));
    if (std.mem.eql(u8, os, "Windows")) {
      lib.root_module.linkSystemLibrary("libpit", .{});
      lib.root_module.linkSystemLibrary("libpumpkin", .{});
    } else {
      lib.root_module.linkSystemLibrary("pit", .{});
      lib.root_module.linkSystemLibrary("pumpkin", .{});
    }
    lib.linkLibC();
    b.installArtifact(lib);
}
