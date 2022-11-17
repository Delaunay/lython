const std = @import("std");

pub fn build(b: *std.build.Builder) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    const mimalloc = b.addStaticLibrary("mimalloc", null);
    mimalloc.setTarget(target);
    mimalloc.setBuildMode(mode);
    mimalloc.linkLibC();
    mimalloc.addIncludePath("dependencies/mimalloc/include");
    mimalloc.addIncludePath("dependencies/mimalloc/src");
    mimalloc.addCSourceFiles(&.{
        "dependencies/mimalloc/src/alloc-aligned.c",
        "dependencies/mimalloc/src/alloc-posix.c",
        "dependencies/mimalloc/src/alloc.c",
        "dependencies/mimalloc/src/arena.c",
        "dependencies/mimalloc/src/heap.c",
        "dependencies/mimalloc/src/init.c",
        "dependencies/mimalloc/src/options.c",
        "dependencies/mimalloc/src/os.c",
        "dependencies/mimalloc/src/page.c",
        "dependencies/mimalloc/src/random.c",
        "dependencies/mimalloc/src/segment-cache.c",
        "dependencies/mimalloc/src/segment.c",
        "dependencies/mimalloc/src/stats.c",
    }, &.{
        "-std=c17",
    });

    const spdlog = b.addStaticLibrary("spdlog", null);
    spdlog.setTarget(target);
    spdlog.setBuildMode(mode);
    spdlog.linkLibC();
    spdlog.defineCMacro("SPDLOG_COMPILED_LIB", null);
    spdlog.linkSystemLibrary("c++");
    spdlog.addIncludePath("dependencies/spdlog/include");
    spdlog.addCSourceFiles(&.{
        "dependencies/spdlog/src/async.cpp",
        "dependencies/spdlog/src/cfg.cpp",
        "dependencies/spdlog/src/color_sinks.cpp",
        "dependencies/spdlog/src/fmt.cpp",
        "dependencies/spdlog/src/spdlog.cpp",
        "dependencies/spdlog/src/stdout_sinks.cpp",
    }, &.{
        "-std=c++20",
    });

    const liblython = b.addStaticLibrary("liblython", null);
    liblython.setTarget(target);
    liblython.setBuildMode(mode);
    liblython.linkLibC();
    liblython.linkSystemLibrary("c++");
    liblython.defineCMacro("BUILD_WEBASSEMBLY", "0");
    liblython.defineCMacro("BUILD_UNIX", "1");
    liblython.defineCMacro("WITH_LOG", "0");
    liblython.defineCMacro("WITH_COZ", "0");
    liblython.defineCMacro("WITH_VALGRIND", "0");
    liblython.defineCMacro("WITH_COVERAGE", "0");
    liblython.force_pic = true;
    liblython.addIncludePath("dependencies/argparse/include");
    liblython.addIncludePath("dependencies/spdlog/include");
    liblython.addIncludePath("dependencies/mimalloc/include");
    liblython.addIncludePath("dependencies");
    liblython.addIncludePath("src/");
    liblython.addIncludePath("build/");
    // liblython.linkLibrary(spdlog);
    liblython.linkLibrary(mimalloc);
    liblython.addCSourceFiles(&.{
        "src/ast/nodes.cpp",
        "src/ast/values/native.cpp",
        "src/ast/values/generator.cpp",
        "src/ast/ops/context.cpp",
        "src/ast/ops/equality.cpp",
        "src/ast/ops/attribute.cpp",
        "src/ast/ops/print.cpp",
        "src/ast/ops/circle.cpp",
        "src/builtin/operators.cpp",
        "src/codegen/cpp/cpp_gen.cpp",
        "src/dependencies/xx_hash.cpp",
        "src/logging/logging.cpp",
        "src/lexer/lexer.cpp",
        "src/lexer/buffer.cpp",
        "src/lexer/token.cpp",
        "src/lexer/unlex.cpp",
        "src/lowering/lowering.cpp",
        "src/parser/parser.cpp",
        "src/parser/parser_ext.cpp",
        "src/parser/parsing_error.cpp",
        "src/printer/error_printer.cpp",
        "src/sema/sema.cpp",
        "src/sema/sema_import.cpp",
        "src/sema/errors.cpp",
        "src/sema/bindings.cpp",
        "src/sema/builtin.cpp",
        "src/vm/tree.cpp",
        "src/vm/garbage_collector.cpp",
        "src/utilities/allocator.cpp",
        "src/utilities/metadata.cpp",
        "src/utilities/pool.cpp",
        "src/utilities/object.cpp",
        "src/utilities/strings.cpp",
        "src/utilities/names.cpp",
    }, &.{
        // "-Wall",
        // "-W",
        // "-Wstrict-prototypes",
        // "-Wwrite-strings",
        // "-Wno-missing-field-initializers",
        "-std=c++20",
    });


    const lython = b.addExecutable("lython", null);
    lython.setTarget(target);
    lython.setBuildMode(mode);
    lython.linkLibC();
    lython.linkSystemLibrary("c++");
    // lython.linkLibrary(spdlog);
    lython.linkLibrary(mimalloc);
    lython.linkLibrary(liblython);
    lython.defineCMacro("BUILD_WEBASSEMBLY", "0");
    lython.defineCMacro("BUILD_UNIX", "1");
    lython.defineCMacro("WITH_LOG", "0");
    lython.defineCMacro("WITH_COZ", "0");
    lython.defineCMacro("WITH_VALGRIND", "0");
    lython.defineCMacro("WITH_COVERAGE", "0");

    lython.install();
    lython.addIncludePath("src/");
    lython.addIncludePath("build/");
    lython.addIncludePath("dependencies/argparse/include");
    lython.addIncludePath("dependencies/spdlog/include");
    lython.addIncludePath("dependencies/mimalloc/include");
    lython.addIncludePath("dependencies");
    lython.addCSourceFiles(&.{
        "src/cli/cli.cpp",
        "src/cli/commands/code.cpp",
        "src/cli/commands/format.cpp",
        "src/cli/commands/codegen.cpp",
        "src/cli/commands/debug.cpp",
        "src/cli/commands/doc.cpp",
        "src/cli/commands/install.cpp",
        "src/cli/commands/internal.cpp",
        "src/cli/commands/linter.cpp",
        "src/cli/commands/profile.cpp",
        "src/cli/commands/tests.cpp"
    }, &.{
        "-std=c++20",
    });

    // const run_cmd = exe.run();
    // run_cmd.step.dependOn(b.getInstallStep());
    // if (b.args) |args| {
    //     run_cmd.addArgs(args);
    // }

    // const run_step = b.step("run", "Run the app");
    // run_step.dependOn(&run_cmd.step);

    // const exe_tests = b.addTest("src/main.zig");
    // exe_tests.setTarget(target);
    // exe_tests.setBuildMode(mode);

    // const test_step = b.step("test", "Run unit tests");
    // test_step.dependOn(&exe_tests.step);
}
