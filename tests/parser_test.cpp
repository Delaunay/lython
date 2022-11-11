#include "cases.h"
#include "samples.h"

#include <catch2/catch.hpp>
#include <sstream>

#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "logging/logging.h"
#include "parser/parser.h"
#include "utilities/strings.cpp"

using namespace lython;

inline String parse_it(String code) {
    StringBuffer reader(code);

    Lexer  lex(reader);
    Parser parser(lex);

    auto mod = Unique<Module>(parser.parse_module());
    assert(mod->body.size() > 0, "Should parse more than one expression");

    std::cout << std::string(80, '-') << '\n';
    std::cout << "Parsing Diag\n";
    std::cout << std::string(80, '-') << '\n';
    parser.show_diagnostics(std::cout);
    std::cout << std::string(80, '-') << '\n';

    auto data = str(mod.get());
    return data;
}

#define TEST_PARSING(code) \
    SECTION(#code) { REQUIRE(strip(parse_it(code())) == strip(code())); }

TEST_CASE("Parser") {
    CODE_SAMPLES(TEST_PARSING);
    IMPORT_TEST(TEST_PARSING);
}

TEST_CASE("Parser_Ext_IfExp") { REQUIRE(parse_it("d = if a: b else c") == "d = b if a else c"); }

struct AllowEntry {
    String name;
    int    j;
    int    i;

    bool operator==(AllowEntry const& v) const { return name == v.name && i == v.i && j == v.j; }
};

// List of Fuzzed examples that works
// mostly because so much is missing that the expression becomes valid again
Array<AllowEntry> allow_list = {
    {"Inline", 0, 9},       {"Inline", 0, 8},      {"Inline", 0, 7},      {"Inline", 0, 5},
    {"Inline", 0, 4},       {"Inline", 0, 3},      {"Inline", 0, 1},      {"Match", 2, 27},
    {"Match", 2, 14},       {"Match", 2, 13},      {"Match", 2, 12},      {"Match", 1, 25},
    {"BoolOp", 0, 1},       {"BoolOp", 1, 1},      {"BoolOp", 2, 1},      {"BoolOp", 2, 3},
    {"NamedExpr", 0, 1},    {"NamedExpr", 0, 3},   {"BinOp", 0, 1},       {"BinOp", 1, 1},
    {"BinOp", 2, 1},        {"BinOp", 3, 1},       {"BinOp", 4, 1},       {"Yield", 0, 1},
    {"Yield", 1, 2},        {"Yield", 1, 1},       {"YieldFrom", 0, 1},   {"Compare", 0, 1},
    {"Compare", 0, 3},      {"Compare", 0, 5},     {"Compare", 1, 1},     {"Compare", 2, 1},
    {"Compare", 3, 1},      {"Compare", 4, 1},     {"Call", 0, 1},        {"Attribute", 0, 1},
    {"Subscript", 0, 1},    {"TupleExpr", 0, 1},   {"TupleExpr", 0, 3},   {"TupleExpr", 1, 1},
    {"TupleExpr", 0, 7},    {"TupleExpr", 1, 7},   {"TupleExpr", 2, 1},   {"TupleExpr", 2, 3},
    {"TupleExpr", 2, 5},    {"TupleExpr", 2, 7},   {"TupleExpr", 2, 9},   {"Slice", 0, 1},
    {"FunctionDef", 0, 30}, {"Return", 0, 1},      {"Return", 1, 2},      {"Return", 1, 1},
    {"Return", 2, 1},       {"Return", 2, 2},      {"Return", 3, 1},      {"Return", 3, 2},
    {"Return", 3, 4},       {"Return", 3, 6},      {"Delete", 0, 2},      {"Assign", 0, 1},
    {"Assign", 1, 1},       {"Assign", 1, 3},      {"Assign", 2, 1},      {"Assign", 3, 1},
    {"Assign", 4, 1},       {"Assign", 5, 1},      {"Assign", 6, 1},      {"Assign", 7, 1},
    {"Assign", 8, 1},       {"Assign", 9, 1},      {"Assign", 10, 1},     {"Assign", 11, 1},
    {"Assign", 12, 1},      {"Assign", 13, 1},     {"Assign", 14, 1},     {"Assign", 14, 3},
    {"Assign", 14, 5},      {"AugAssign", 0, 1},   {"AugAssign", 1, 1},   {"AnnAssign", 3, 1},
    {"AnnAssign", 0, 3},    {"AnnAssign", 2, 1},   {"AnnAssign", 0, 1},   {"AnnAssign", 1, 1},
    {"AnnAssign", 3, 3},    {"AnnAssign", 2, 3},   {"AnnAssign", 1, 3},   {"For", 0, 8},
    {"For", 0, 9},          {"For", 0, 10},        {"For", 1, 16},        {"For", 0, 14},
    {"For", 0, 13},         {"For", 0, 12},        {"For", 0, 19},        {"For", 0, 11},
    {"While", 0, 6},        {"While", 0, 7},       {"While", 0, 8},       {"While", 0, 13},
    {"If", 0, 6},           {"If", 0, 7},          {"If", 0, 8},          {"If", 0, 14},
    {"If", 0, 15},          {"If", 0, 16},         {"If", 0, 21},         {"With", 0, 12},
    {"Raise", 0, 1},        {"Raise", 0, 2},       {"Raise", 1, 1},       {"Try", 0, 15},
    {"Try", 0, 16},         {"Try", 0, 17},        {"Try", 0, 22},        {"Try", 0, 23},
    {"Try", 0, 24},         {"Try", 0, 29},        {"Assert", 1, 2},      {"Import", 0, 2},
    {"Import", 0, 4},       {"Import", 0, 6},      {"Import", 0, 8},      {"Import", 1, 2},
    {"Import", 0, 10},      {"Import", 0, 12},     {"ImportFrom", 0, 6},  {"ImportFrom", 1, 4},
    {"ImportFrom", 0, 8},   {"ImportFrom", 0, 10}, {"ImportFrom", 0, 12}, {"ImportFrom", 1, 12},
    {"ImportFrom", 1, 10},  {"ImportFrom", 1, 8},  {"ImportFrom", 1, 6},  {"ImportFrom", 1, 16},
    {"ImportFrom", 1, 14},  {"Match", 0, 15},      {"Match", 0, 16},      {"Match", 0, 17},
    {"Match", 0, 25},       {"Match", 0, 26},      {"Match", 0, 27},      {"Match", 0, 28},
    {"Match", 0, 36},       {"Match", 0, 37},      {"Match", 0, 38},      {"Match", 0, 39},
    {"Match", 0, 54},       {"Match", 0, 55},      {"Match", 0, 57},      {"Match", 0, 59},
    {"Match", 0, 60},       {"Match", 0, 61},      {"Match", 0, 69},      {"Match", 0, 70},
    {"Match", 1, 12},       {"Match", 1, 13},      {"Match", 1, 14},      {"ClassDef", 1, 10},
    {"ClassDef", 1, 11},    {"ClassDef", 1, 16},   {"ClassDef", 1, 17},   {"ClassDef", 1, 20},
    {"ClassDef", 1, 21},    {"ClassDef", 1, 22},   {"ClassDef", 1, 31},   {"ClassDef", 1, 33},
    {"ClassDef", 1, 35},    {"ClassDef", 1, 36},   {"ClassDef", 1, 14},   {"ClassDef", 1, 8},
    {"IfExp", 0, 1},        {"IfExp", 0, 3},

    {"With", 1, 12},        {"With", 1, 14},       {"With", 1, 16},       {"With", 1, 17},
    {"With", 1, 18},        {"With", 1, 20},       {"With", 1, 22},       {"With", 1, 23},
    {"With", 1, 24},        {"With", 1, 26},       {"With", 1, 28}};

bool allowed(AllowEntry const& v) {
    for (auto& entry: allow_list) {
        if (entry == v) {
            return true;
        }
    }

    return false;
}

void show_debug(
    String const& name, int j, int i, String const& code, Array<Token> const& tokens, Module* mod) {
    std::cout << "\n=========Module: " << name << " x " << j << " x " << i << "\n";
    std::cout << code;
    std::cout << "\n=======\n";
    for (auto& tok: tokens) {
        tok.print(std::cout);
    }
    std::cout << "\n=======\n";
    if (mod) {
        auto dumps = str(mod);
        std::cout << "\n=======\n";
        std::cout << dumps;
        std::cout << "\n==========\n";
    }
}

// Runs code through the parser but remove some tokens
void run_partial(String const& name, int j, TestCase const& test) {
    StringBuffer reader(test.code);
    Lexer        lex(reader);
    Array<Token> toks = lex.extract_token();

    for (int i = 1; i < toks.size() - 1; i++) {
        Array<Token> tokens(std::begin(toks), std::begin(toks) + i);
        ReplayLexer  lexer(tokens);

        Parser parser(lexer);
        auto   expr = [&]() {
            Module mod;
            parser.parse_to_module(&mod);

            if (parser.has_errors()) {
                parser.show_diagnostics(std::cout);
                throw SyntaxError();
            }
            show_debug(name, j, i, test.code, tokens, &mod);
        };

        if (!allowed({name, j, i})) {
            CHECK_THROWS_AS(expr(), SyntaxError);
        } else {
            try {
                Module mod;
                parser.parse_to_module(&mod);

                if (parser.has_errors()) {
                    parser.show_diagnostics(std::cout);
                    throw SyntaxError();
                }
            } catch (SyntaxError const& err) {
                show_debug(name, j, i, test.code, tokens, nullptr);
                throw err;
            }
        }
    }
}

void run_partials(String const& name, Array<TestCase> cases) {
    info("Testing {}", name);
    for (int i = 0; i < cases.size(); i++) {
        auto& c = cases[i];
        run_partial(name, i, c);
        info("<<<<<<<<<<<<<<<<<<<<<<<< DONE");
    }
}

using Transformer = std::function<String(String const& code)>;

String identity(String const& code) { return code; }

// insert comments at the end of each lines
String insert_comment(String const& code) {
    auto lines = split('\n', code);

    int          i = 1;
    StringStream ss;

    ss << "# c0 another tok\n";
    for (auto& line: lines) {
        ss << line;

        if (line.size() > 0) {
            ss << "   ";
        }

        ss << "# c" << i << " another tok\n";
        i += 1;
    }

    return ss.str();
}

struct FormatException {
    String name;
    int    i;
    String format;
    bool   operator==(FormatException const& v) const { return name == v.name && i == v.i; }
};

String classdef_format = "# c0 another tok\n"
                         "class Name:   # c1 another tok\n"
                         "    x: i32 = 0   # c2 another tok\n"
                         "    y: i32 = 1   # c3 another tok\n"
                         "    z = 1.2   # c4 another tok\n"
                         "\n"
                         "    # c5 another tok\n"
                         "\n"
                         "    def __init__(self):   # c6 another tok\n"
                         "        self.x = 2   # c7 another tok\n"
                         "\n"
                         "\n"
                         "# c8 another tok\n";

Array<FormatException> fmt_exceptions = {{"ClassDef", 1, classdef_format}};

String const* get_exception(Array<FormatException> const& exceptions, FormatException const& v) {
    for (auto& entry: exceptions) {
        if (entry == v) {
            return &entry.format;
        }
    }
    return nullptr;
}

void run_testcase(String const&                 name,
                  Array<TestCase>               cases,
                  Transformer                   Transformer = identity,
                  Array<FormatException> const& exceptions  = Array<FormatException>()) {
    int i = 0;
    for (auto& c: cases) {
        String const* fmt = get_exception(exceptions, FormatException{name, i});

        info("Testing {} - {}", name, i);
        String new_code = Transformer(c.code);

        String parsed   = strip(parse_it(new_code));
        String original = strip(new_code);
        bool   equal    = parsed == original;

        if (!equal) {
            error("\n`{}`\n`{}`", parsed, original);
        }

        if (fmt != nullptr) {
            original = strip(*fmt);
        }

        REQUIRE(parsed == original);
        info("<<<<<<<<<<<<<<<<<<<<<<<< DONE");
        i += 1;
    }
}

#define GENTEST(name)                                                                      \
    TEMPLATE_TEST_CASE("Parser_Success_" #name, #name, name) {                             \
        run_testcase(str(nodekind<TestType>()), name##_examples());                        \
    }                                                                                      \
    TEMPLATE_TEST_CASE("Parser_Comment_" #name, #name, name) {                             \
        run_testcase(                                                                      \
            str(nodekind<TestType>()), name##_examples(), insert_comment, fmt_exceptions); \
    }                                                                                      \
    TEMPLATE_TEST_CASE("Parser_Failure_" #name, #name, name) {                             \
        run_partials(str(nodekind<TestType>()), name##_examples());                        \
    }

#define X(name, _)
#define SSECTION(name)
#define EXPR(name, _) GENTEST(name)
#define STMT(name, _) GENTEST(name)
#define MOD(name, _)
#define MATCH(name, _)

NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef GENTEST
