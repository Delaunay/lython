
#include <catch2/catch_all.hpp>

#include "ast/ops.h"
#include "parser/parser.h"

#include "logging/logging.h"

using namespace lython;

TEST_CASE("Attribute") {
    String code = ""
                  "class AttributeTest:\n"
                  "    a: float = 1.1\n"
                  "    b = 2.0\n"
                  "\n"
                  "    def __init__(self):\n"
                  "        pass\n"
                  "\n"
                  "    def f(self, a, b):\n"
                  "        pass\n"
                  "\n"
                  "    class Nested:\n"
                  "        c = 3\n"
                  "\n";

    StringBuffer reader(code);
    Module       module;

    Lexer  lex(reader);
    Parser parser(lex);

    Module *mod       = parser.parse_module();
    auto    class_def = cast<ClassDef>(mod->body[0]);

    REQUIRE(class_def != nullptr);
    REQUIRE(class_def->body.size() == 5);

    auto annassign = class_def->body[0];
    auto assign    = class_def->body[1];
    auto init_fun  = class_def->body[2];
    auto fun       = class_def->body[3];
    auto nested    = class_def->body[4];

    ExprNode *ignore = nullptr;
    REQUIRE(hasattr(annassign, "a") == false);

    REQUIRE(hasattr(class_def, "a"));
    REQUIRE(equal(getattr(class_def, "a", ignore), annassign));

    REQUIRE(hasattr(class_def, "b"));
    REQUIRE(equal(getattr(class_def, "b", ignore), assign));

    REQUIRE(hasattr(class_def, "__init__"));
    REQUIRE(equal(getattr(class_def, "__init__", ignore), init_fun));

    REQUIRE(hasattr(class_def, "f"));
    REQUIRE(equal(getattr(class_def, "f", ignore), fun));

    REQUIRE(hasattr(class_def, "Nested"));
    REQUIRE(equal(getattr(class_def, "Nested", ignore), nested));

    REQUIRE(hasattr(class_def, "does_not_exist") == false);
}