const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

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

    lib.addIncludePath(b.path("../libpumpkin"));
    lib.addLibraryPath(b.path("../../bin"));
    lib.linkSystemLibrary("pit");
    lib.linkSystemLibrary("pumpkin");
    lib.linkLibC();
    b.installArtifact(lib);
}
