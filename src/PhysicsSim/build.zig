const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});

    const optimize = b.standardOptimizeOption(.{});

    const lib = b.addSharedLibrary(.{
        .name = "PhysicsSim",
        .root_source_file = .{ .path = "main.zig" },
        .target = target,
        .optimize = optimize,
    });

    const pumpkin = b.createModule(.{
        .source_file = .{ .path = "../libpumpkin/pumpkin.zig"},
        .dependencies = &.{},
    });

    const space = b.createModule(.{
        .source_file = .{ .path = "../libchipmunk/space.zig"},
        .dependencies = &.{},
    });

    lib.addModule("pumpkin", pumpkin);
    lib.addModule("space", space);

    lib.addIncludePath("../libpumpkin");
    lib.addIncludePath("../libchipmunk/chipmunk");
    lib.addIncludePath(".");
    lib.addLibraryPath("../../bin");
    lib.linkSystemLibraryName("pit");
    lib.linkSystemLibraryName("pumpkin");
    lib.linkSystemLibraryName("chipmunk");
    lib.linkSystemLibraryName("pluto");
    lib.install();
}
