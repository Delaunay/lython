const std = @import("std");

pub fn build(b: *std.Build) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    // const mode = b.standardReleaseOptions();
    const mode = b.standardOptimizeOption(.{});

    const mimalloc = b.addStaticLibrary(.{
        .name = "mimalloc", 
        .target = target,
        .optimize = mode,
    });
    mimalloc.linkLibC();
    mimalloc.addIncludePath(.{ .path="dependencies/mimalloc/include"});
    mimalloc.addIncludePath(.{ .path="dependencies/mimalloc/src"});
    mimalloc.addCSourceFiles(.{ 
        .files = &.{
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
        },
        .flags = &.{
            // "-std=c++17",
        },
    });

    const spdlog = b.addStaticLibrary(.{
        .name = "spdlog", 
        .target = target,
        .optimize = mode,
    });
    spdlog.linkLibC();
    spdlog.defineCMacro("SPDLOG_COMPILED_LIB", "1");
    spdlog.linkSystemLibrary("c++");
    spdlog.addIncludePath(.{ .path="dependencies/spdlog/include"});
    spdlog.addCSourceFiles(.{
        .files = &.{
            "dependencies/spdlog/src/async.cpp",
            "dependencies/spdlog/src/cfg.cpp",
            "dependencies/spdlog/src/color_sinks.cpp",
            "dependencies/spdlog/src/fmt.cpp",
            "dependencies/spdlog/src/spdlog.cpp",
            "dependencies/spdlog/src/stdout_sinks.cpp",
        }, 
        .flags = &.{
            "-std=c++17",
        },
    });

    // logging does not work
    const with_log = "0";

    const liblython = b.addStaticLibrary(.{
        .name = "lython", 
        .target = target,
        .optimize = mode,
    });
    //liblython.global_base = 6560;
    //liblython.rdynamic = true;
    //liblython.import_memory = true;
    //liblython.stack_size = std.wasm.page_size;
    liblython.linkLibC();
    liblython.linkSystemLibrary("c++");
    liblython.defineCMacro("BUILD_WEBASSEMBLY", "1");
    liblython.defineCMacro("BUILD_UNIX", "1");
    liblython.defineCMacro("WITH_LOG", with_log);
    liblython.defineCMacro("WITH_COZ", "0");
    liblython.defineCMacro("WITH_VALGRIND", "0");
    liblython.defineCMacro("WITH_COVERAGE", "0");
    liblython.defineCMacro("FMT_USE_CONSTEXPR", "0");
    // liblython.force_pic = true;
    liblython.addIncludePath(.{ .path="dependencies/argparse/include"});
    liblython.addIncludePath(.{ .path="dependencies/spdlog/include"});
    liblython.addIncludePath(.{ .path="dependencies/mimalloc/include"});
    liblython.addIncludePath(.{ .path="dependencies"});
    liblython.addIncludePath(.{ .path="src/"});
    liblython.addIncludePath(.{ .path="build/"});
    liblython.linkLibrary(mimalloc);
    if (std.mem.eql(u8, with_log, "1")) {
        liblython.linkLibrary(spdlog);
    }
    b.installArtifact(liblython);
    liblython.addCSourceFiles(.{
        .files = &.{
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
            "src/parser/format_spec.cpp",
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
        }, 
        .flags = &.{
            // "-Wall",
            // "-W",
            // "-Wstrict-prototypes",
            // "-Wwrite-strings",
            // "-Wno-missing-field-initializers",
            "-std=c++17",
        },
    });


    // const lython = b.addExecutable(.{
    //     .name = "lython", 
    //     .target = target,
    //     .optimize = mode,
    // });
    // lython.linkLibC();
    // lython.linkSystemLibrary("c++");
    // if (std.mem.eql(u8, with_log, "1")) {
    //     lython.linkLibrary(spdlog);
    // }
    // lython.linkLibrary(mimalloc);
    // lython.linkLibrary(liblython);
    // lython.defineCMacro("BUILD_WEBASSEMBLY", "1");
    // lython.defineCMacro("BUILD_UNIX", "1");
    // lython.defineCMacro("WITH_LOG", with_log);
    // lython.defineCMacro("WITH_COZ", "0");
    // lython.defineCMacro("WITH_VALGRIND", "0");
    // lython.defineCMacro("WITH_COVERAGE", "0");
    // lython.defineCMacro("FMT_USE_CONSTEXPR", "0");

    
    // // lython.installArtifact();
    // b.installArtifact(lython);
    // lython.addIncludePath(.{ .path="src/"});
    // lython.addIncludePath(.{ .path="build/"});
    // lython.addIncludePath(.{ .path="dependencies/argparse/include"});
    // lython.addIncludePath(.{ .path="dependencies/spdlog/include"});
    // lython.addIncludePath(.{ .path="dependencies/mimalloc/include"});
    // lython.addIncludePath(.{ .path="dependencies"});
    // lython.addCSourceFiles(.{
    //     .files = &.{
    //         "src/cli/cli.cpp",
    //         "src/cli/commands/code.cpp",
    //         "src/cli/commands/format.cpp",
    //         "src/cli/commands/codegen.cpp",
    //         "src/cli/commands/debug.cpp",
    //         "src/cli/commands/doc.cpp",
    //         "src/cli/commands/install.cpp",
    //         "src/cli/commands/internal.cpp",
    //         "src/cli/commands/linter.cpp",
    //         "src/cli/commands/profile.cpp",
    //         "src/cli/commands/tests.cpp",
    //         "src/cli/commands/vm.cpp"
    //     }, 
    //     .flags = &.{
    //         "-std=c++17",
    //     }
    // });
}
