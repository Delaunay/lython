
#include <catch2/catch.hpp>

#include "ast/ops.h"
#include "parser/parser.h"

#include "logging/logging.h"

using namespace lython;

TEST_CASE("Attribute") {
    String code = ""
                  "class AttributeTest:\n"
                  "    a: float = 1.1\n"
                  "    b: float = 2.0\n"
                  ""
                  "    def __init__(self):\n"
                  "        pass\n"
                  ""
                  "    def f(self, a, b):\n"
                  "        pass\n"
                  "";

    StringBuffer reader(code);
    Module       module;

    Lexer  lex(reader);
    Parser parser(lex);

    Module *mod       = parser.parse_module();
    auto    class_def = cast<ClassDef>(mod->body[0]);

    REQUIRE(class_def != nullptr);
    REQUIRE(class_def->body.size() == 4);

    auto annassign = class_def->body[0];
    auto assign    = class_def->body[1];
    auto init_fun  = class_def->body[2];
    auto fun       = class_def->body[3];

    REQUIRE(hasattr(class_def, "a"));
    REQUIRE(equal(getattr(class_def, "a"), annassign));

    REQUIRE(hasattr(class_def, "b"));
    REQUIRE(equal(getattr(class_def, "b"), assign));

    REQUIRE(hasattr(class_def, "__init__"));
    REQUIRE(equal(getattr(class_def, "__init__"), init_fun));

    REQUIRE(hasattr(class_def, "f"));
    REQUIRE(equal(getattr(class_def, "f"), fun));

    REQUIRE(hasattr(class_def, "does_not_exist") == false);
}