const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});

    const optimize = b.standardOptimizeOption(.{});

    const lib = b.addSharedLibrary(.{
        .name = "zigtest",
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    lib.addIncludePath(".");
    lib.addLibraryPath("../../bin");
    lib.linkSystemLibraryName("pit");
    lib.install();
}
