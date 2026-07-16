const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});

    const optimize = b.standardOptimizeOption(.{});

    const lib = b.addLibrary(.{
        .linkage = .dynamic,
        .name = "MinimalZig",
        .root_module = b.createModule(.{
            .root_source_file = b.path("main.zig"),
            .target = target,
            .optimize = optimize,
        })
    });

    const pumpkin = b.createModule(.{
        .root_source_file = b.path("../libpumpkin/pumpkin.zig"),
        .imports = &.{},
    });
    pumpkin.addIncludePath(b.path("../libpumpkin"));

    lib.root_module.addImport("pumpkin", pumpkin);
    lib.root_module.addIncludePath(b.path("../libpumpkin"));
    lib.root_module.addLibraryPath(b.path("../../bin"));
    lib.root_module.linkSystemLibrary("pit", .{});
    lib.root_module.linkSystemLibrary("pumpkin", .{});
    b.installArtifact(lib);
}
