const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});

    const optimize = b.standardOptimizeOption(.{});

    const lib = b.addSharedLibrary(.{
        .name = "MinimalZig",
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    const pumpkin = b.createModule(.{
        .source_file = .{ .path = "../libpumpkin/pumpkin.zig"},
        .dependencies = &.{},
    });

    lib.addModule("pumpkin", pumpkin);

    lib.addIncludePath(".");
    lib.addLibraryPath("../../bin");
    lib.linkSystemLibraryName("pit");
    lib.linkSystemLibraryName("pumpkin");
    lib.install();
}
