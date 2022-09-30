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
    Module       module;

    Lexer  lex(reader);
    Parser parser(lex);

    Module* mod = parser.parse_module();
    assert(mod->body.size() > 0, "Should parse more than one expression");

    auto data = str(mod);

    delete mod;
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
    {"Delete", 0, 2},       {"Assign", 0, 1},      {"Assign", 1, 1},      {"Assign", 1, 3},
    {"Assign", 2, 1},       {"Assign", 3, 1},      {"Assign", 4, 1},      {"Assign", 5, 1},
    {"Assign", 6, 1},       {"Assign", 7, 1},      {"Assign", 8, 1},      {"Assign", 9, 1},
    {"Assign", 10, 1},      {"Assign", 11, 1},     {"Assign", 12, 1},     {"Assign", 13, 1},
    {"Assign", 14, 1},      {"Assign", 14, 3},     {"Assign", 14, 5},     {"AugAssign", 0, 1},
    {"AugAssign", 1, 1},    {"AnnAssign", 3, 1},   {"AnnAssign", 2, 1},   {"AnnAssign", 0, 1},
    {"AnnAssign", 1, 1},    {"For", 0, 8},         {"For", 0, 9},         {"For", 0, 10},
    {"For", 0, 15},         {"For", 1, 16},        {"While", 0, 6},       {"While", 0, 7},
    {"While", 0, 8},        {"While", 0, 13},      {"If", 0, 6},          {"If", 0, 7},
    {"If", 0, 8},           {"If", 0, 14},         {"If", 0, 15},         {"If", 0, 16},
    {"If", 0, 21},          {"With", 0, 12},       {"Raise", 0, 1},       {"Raise", 0, 2},
    {"Raise", 1, 1},        {"Try", 0, 15},        {"Try", 0, 16},        {"Try", 0, 17},
    {"Try", 0, 22},         {"Try", 0, 23},        {"Try", 0, 24},        {"Try", 0, 29},
    {"Assert", 1, 2},       {"Import", 0, 2},      {"Import", 0, 4},      {"Import", 0, 6},
    {"Import", 0, 8},       {"Import", 1, 2},      {"Import", 0, 10},     {"Import", 0, 12},
    {"ImportFrom", 0, 6},   {"ImportFrom", 1, 4},  {"ImportFrom", 0, 8},  {"ImportFrom", 0, 10},
    {"ImportFrom", 0, 12},  {"ImportFrom", 1, 12}, {"ImportFrom", 1, 10}, {"ImportFrom", 1, 8},
    {"ImportFrom", 1, 6},   {"ImportFrom", 1, 16}, {"ImportFrom", 1, 14}, {"Match", 0, 15},
    {"Match", 0, 16},       {"Match", 0, 17},      {"Match", 0, 25},      {"Match", 0, 26},
    {"Match", 0, 27},       {"Match", 0, 28},      {"Match", 0, 36},      {"Match", 0, 37},
    {"Match", 0, 38},       {"Match", 0, 39},      {"Match", 0, 54},      {"Match", 0, 55},
    {"Match", 0, 57},       {"Match", 0, 59},      {"Match", 0, 60},      {"Match", 0, 61},
    {"Match", 0, 69},       {"Match", 0, 70},      {"Match", 1, 12},      {"Match", 1, 13},
    {"Match", 1, 14},       {"ClassDef", 1, 10},   {"ClassDef", 1, 11},   {"ClassDef", 1, 16},
    {"ClassDef", 1, 17},    {"ClassDef", 1, 20},   {"ClassDef", 1, 21},   {"ClassDef", 1, 22},
    {"ClassDef", 1, 31},    {"ClassDef", 1, 33},   {"ClassDef", 1, 35},   {"ClassDef", 1, 36},
    {"IfExp", 0, 1},        {"IfExp", 0, 3}};

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
    Module       module;

    Lexer        lex(reader);
    Array<Token> toks = lex.extract_token();

    for (int i = 1; i < toks.size() - 1; i++) {
        Array<Token> tokens(std::begin(toks), std::begin(toks) + i);
        ReplayLexer  lexer(tokens);

        Parser parser(lexer);
        auto   expr = [&]() {
            Module* mod = parser.parse_module();
            show_debug(name, j, i, test.code, tokens, mod);
        };

        if (!allowed({name, j, i})) {
            CHECK_THROWS_AS(expr(), SyntaxError);
        } else {
            try {
                Module* mod = parser.parse_module();
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

void run_testcase(String const& name, Array<TestCase> cases) {
    info("Testing {}", name);
    for (auto& c: cases) {
        REQUIRE(strip(parse_it(c.code)) == strip(c.code));
        info("<<<<<<<<<<<<<<<<<<<<<<<< DONE");
    }
}

#define GENTEST(name)                                               \
    TEMPLATE_TEST_CASE("Parser_Success_" #name, #name, name) {      \
        run_testcase(str(nodekind<TestType>()), name##_examples()); \
    }                                                               \
    TEMPLATE_TEST_CASE("Parser_Failure_" #name, #name, name) {      \
        run_partials(str(nodekind<TestType>()), name##_examples()); \
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
