# ===============================================================================
# Project Source
# ===============================================================================

# Includes
# ========

INCLUDE_DIRECTORIES(.)

# INCLUDE_DIRECTORIES(../dependencies/fmt/include)
INCLUDE_DIRECTORIES(../dependencies)
INCLUDE_DIRECTORIES(mimalloc)

# Libraries
# =========
SET(LIBRARIES fmt::fmt)

IF(WITH_VALGRIND)
    MESSAGE(STATUS "Disabling mimalloc")
    ADD_DEFINITIONS(-DWITH_VALGRIND)
ELSE()
    LIST(APPEND LIBRARIES mimalloc-static)
ENDIF(WITH_VALGRIND)

IF(WITH_LOG)
    # LIST(APPEND LIBRARIES spdlog::spdlog)
ENDIF(WITH_LOG)

IF(WITH_COZ)
    LIST(APPEND LIBRARIES dl)
    ADD_COMPILE_OPTIONS(-g3 -gdwarf-3)
    MESSAGE(STATUS "Compiling for coz")
ENDIF(WITH_COZ)

# Source
SET(ADD_HEADERS
    ast/nodes.h
    ast/ops.h

    # ast/values/native.h
    # ast/values/generator.h
    utilities/magic.h
    wasm/interface.cpp

    # ast/values/constant.h
    ast/values/value.h
    builtin/operators.h
    compatibility/compatibility.h
    codegen/cpp/cpp_gen.h
    codegen/clang/clang_gen.h
    codegen/llvm/llvm_gen.h
    codegen/llvm/llvm_jit.h
    lexer/lexer.h
    lexer/buffer.h
    lexer/token.h
    lexer/unlex.h
    lowering/lowering.h
    parser/parser.h
    parser/parsing_error.h
    parser/format_spec.h
    sema/sema.h
    sema/importlib.h
    stdlib/garbage.cpp
    stdlib/garbage_linux.cpp
    stdlib/garbage_windows.cpp
    vm/tree.h
    vm/vm.h

    # vm/garbage_collector.h
    utilities/names.h
    utilities/object.h
    utilities/optional.h
    utilities/pool.h
    utilities/trie.h
    utilities/stack.h
    utilities/allocator.h
    utilities/metadata.h
    utilities/stopwatch.h
    utilities/strings.h
    utilities/guard.h
    dtypes.h
    "${BUILDDIR}/revision_data.h"
)

FIND_PACKAGE(Python)

ADD_CUSTOM_COMMAND(
    OUTPUT generated.cpp
    COMMAND ${CMAKE_COMMAND} -E rm -f generated.cpp
    COMMAND Python::Interpreter
    ARGS ${CMAKE_SOURCE_DIR}/lython/header/header.py ${CMAKE_SOURCE_DIR}/src/ast generated.cpp
)

SET(ADD_SOURCE
    ast/nodes.cpp

    generated.cpp

    # ast/values/native.cpp
    ast/values/value.cpp
    ast/ops/context.cpp
    ast/ops/equality.cpp
    ast/ops/attribute.cpp
    ast/ops/print.cpp
    ast/ops/circle.cpp
    builtin/operators.cpp
    codegen/cpp/cpp_gen.cpp
    codegen/clang/clang_gen.cpp
    codegen/llvm/llvm_gen.cpp
    codegen/llvm/llvm_jit.cpp
    codegen/llvm/llvm_emit.cpp
    dependencies/xx_hash.cpp
    lexer/lexer.cpp
    lexer/buffer.cpp
    lexer/token.cpp
    lexer/unlex.cpp
    lowering/lowering.cpp
    lowering/SSA.cpp
    parser/parser.cpp
    parser/parser_ext.cpp
    parser/parsing_error.cpp
    parser/format_spec.cpp
    printer/error_printer.cpp
    sema/sema.cpp
    sema/sema_import.cpp
    sema/errors.cpp
    sema/bindings.cpp
    sema/builtin.cpp
    sema/importlib.cpp
    vm/tree.cpp
    vm/vm.cpp

    # vm/garbage_collector.cpp
    utilities/allocator.cpp
    utilities/metadata.cpp
    utilities/pool.cpp
    utilities/object.cpp
    utilities/strings.cpp
    utilities/names.cpp
)

# Configure Platforms
# ==============================================
IF(BUILD_WINDOWS)
ENDIF()

IF(BUILD_POSIX AND !BUILD_WEBASSEMBLY)
    ADD_COMPILE_OPTIONS(-mavx2)
ENDIF()

IF(BUILD_WEBASSEMBLY)
    LIST(APPEND LIBRARIES embind)
    ADD_COMPILE_OPTIONS(-sEXCEPTION_CATCHING_ALLOWED=YES)
    ADD_LINK_OPTIONS(-sEXCEPTION_CATCHING_ALLOWED=YES)
    # add_definitions()
    # add_definitions("-s LINKABLE=1")
    # add_definitions("-s EXPORT_ALL=1")
ENDIF()

# Defintions
ADD_DEFINITIONS(-DBUILD_WEBASSEMBLY=${BUILD_WEBASSEMBLY})
ADD_DEFINITIONS(-DBUILD_POSIX=${BUILD_POSIX})
ADD_DEFINITIONS(-DBUILD_WINDOWS=${BUILD_WINDOWS})
ADD_DEFINITIONS(-DWITH_LLVM=${WITH_LLVM})
ADD_DEFINITIONS(-DWITH_LLVM_CODEGEN=${WITH_LLVM})
ADD_DEFINITIONS(-DWITH_CLANG_CODEGEN=0)

# ADD_DEFINITIONS(-DWIN32=${WIN32})
ADD_DEFINITIONS(-DWITH_LOG=${WITH_LOG})
ADD_DEFINITIONS(-DWITH_COZ=${WITH_COZ})
ADD_DEFINITIONS(-DFMT_USE_CONSTEXPR=0)

# Compile
# ==================

# Support library for utilities
ADD_LIBRARY(liblogging logging/backtrace.cpp logging/logger.cpp)
TARGET_LINK_LIBRARIES(liblogging ${LIBRARIES})

# Core language libraries
ADD_LIBRARY(liblython ${SRC_LIST} ${ADD_HEADERS} ${ADD_SOURCE})
TARGET_LINK_LIBRARIES(liblython ${LIBRARIES} ${LLVM_LIRARIES})
ADD_DEPENDENCIES(liblython ZLIB::ZLIB)

ADD_LIBRARY(stblyb stdlib/siphash.cpp)
SET_PROPERTY(TARGET stblyb PROPERTY CXX_STANDARD ${LY_CXX_STANDARD})

# CLI
SET(ADD_CLI_SOURCE
    cli/cli.cpp
    cli/commands/code.cpp
    cli/commands/format.cpp
    cli/commands/codegen.cpp
    cli/commands/debug.cpp
    cli/commands/doc.cpp
    cli/commands/install.cpp
    cli/commands/internal.cpp
    cli/commands/repl.cpp
    cli/commands/linter.cpp
    cli/commands/profile.cpp
    cli/commands/tests.cpp
    cli/commands/vm.cpp
)
ADD_EXECUTABLE(lython ${ADD_CLI_SOURCE})
TARGET_LINK_LIBRARIES(lython liblython liblogging argparse::argparse ${LIBRARIES} ${LLVM_LIRARIES})

SET_PROPERTY(TARGET lython PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
SET_PROPERTY(TARGET lython PROPERTY CMAKE_CXX_STANDARD ${LY_CXX_STANDARD})

IF(UNIX AND WITH_COVERAGE)
    setup_target_for_coverage_gcovr_html(
        NAME coverage
        EXECUTABLE ctest --timeout 10 --output-on-failure
        BASE_DIRECTORY "."
        DEPENDS test
        EXCLUDE "dependencies/*" "src/tide/dependencies/*"
    )
ENDIF()



# ADD_CUSTOM_COMMAND(
#     OUTPUT generated.cpp
#     COMMAND ${CMAKE_COMMAND} -E rm -f generated.cpp
#     COMMAND Python::Interpreter
#     ARGS ${CMAKE_SOURCE_DIR}/lython/header/header.py ${CMAKE_SOURCE_DIR}/src/ast generated.cpp
# )

# emcc -lembind -o library.js -Wl,--whole-archive library.a -Wl,--no-whole-archive



IF (BUILD_WEBASSEMBLY)
    # How can I make this better
    #
    # /home/newton/.conan2/p/b/fmte41f475119a6f/p/lib/libfmt.a
    # $<TARGET_PROPERTY:LINK_LIBRARIES>
    #
    get_target_property(FMT_LIBRARIES fmt::fmt INTERFACE_LINK_LIBRARIES)
    message(STATUS "FMT_LIBRARIES:" $<fmt::fmt>)

    ADD_CUSTOM_COMMAND(
        TARGET liblython
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        COMMAND emcc -lembind -o liblython.js -Wl,--whole-archive 
            ${CMAKE_BINARY_DIR}/lib/libliblogging.a 
            ${CMAKE_BINARY_DIR}/lib/libliblython.a  
            /home/newton/.conan2/p/b/fmte41f475119a6f/p/lib/libfmt.a
            -Wl,--no-whole-archive
        COMMAND_EXPAND_LISTS
        VERBATIM
    )

    # ADD_EXECUTABLE(liblython.js nothing.cpp)
    # # target_compile_options(liblython.js PRIVATE)
    # TARGET_LINK_LIBRARIES(liblython.js liblython ${LIBRARIES} embind)

    # ADD_LIBRARY()
ENDIF()


# SET (CUDA_SUPPORT
# utilities/allocator.cu
# )

# Compile as a library to avoid double compilation
# ADD_SUBDIRECTORY(cuda)
# ADD_SUBDIRECTORY(math)

# if not windows
# stdc++fs
# endif

# FUNCTION(CLANG_STATIC_ANALYSIS target)
# GET_TARGET_PROPERTY(SRCs ${target} SOURCES)
# ADD_LIBRARY(${target}_analyze OBJECT EXCLUDE_FROM_ALL ${SRCs})
# TARGET_INCLUDE_DIRECTORIES(
# ${target}_analyze PUBLIC
# $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
# )
# SET_TARGET_PROPERTIES(${target}_analyze PROPERTIES
# COMPILE_OPTIONS "--analyze"
# EXCLUDE_FROM_DEFAULT_BUILD true)
# ENDFUNCTION()
add_subdirectory(tide)

# Plugin Example
ADD_LIBRARY(PluginExample SHARED plugin/example.cpp)
TARGET_LINK_LIBRARIES(PluginExample ${LIBRARIES})
