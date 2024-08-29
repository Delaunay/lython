// #include "ast/nodes.h"


#include <string>
#include <vector>
#include <iostream>
#include <random>
#include <cassert>
#include <functional>

#include "libtest_fuzzer.h"

void test_match_value() {
    std::cout << "match_value\n"; 
    lython::Branch* def = lython::match_value();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_match_singleton() {
    std::cout << "match_singleton\n"; 
    lython::Branch* def = lython::match_singleton();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_match_sequence() {
    std::cout << "match_sequence\n"; 
    lython::Branch* def = lython::match_sequence();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_match_mapping() {
    std::cout << "match_mapping\n"; 
    lython::Branch* def = lython::match_mapping();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_match_class() {
    std::cout << "match_class\n"; 
    lython::Branch* def = lython::match_class();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_match_star() {
    std::cout << "match_star\n"; 
    lython::Branch* def = lython::match_star();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_match_as() {
    std::cout << "match_as\n"; 
    lython::Branch* def = lython::match_as();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_match_or() {
    std::cout << "match_or\n"; 
    lython::Branch* def = lython::match_or();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_statement() {
    std::cout << "statement\n"; 
    lython::Branch* def = lython::statement();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_functiondef() {
    std::cout << "functiondef\n"; 
    lython::Branch* def = lython::functiondef();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_classdef() {
    std::cout << "classdef\n"; 
    lython::Branch* def = lython::classdef();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_return_() {
    std::cout << "return_\n"; 
    lython::Branch* def = lython::return_();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_del() {
    std::cout << "del\n"; 
    lython::Branch* def = lython::del();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_assign() {
    std::cout << "assign\n"; 
    lython::Branch* def = lython::assign();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_typealias() {
    std::cout << "typealias\n"; 
    lython::Branch* def = lython::typealias();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_aug_assign() {
    std::cout << "aug_assign\n"; 
    lython::Branch* def = lython::aug_assign();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_ann_assign() {
    std::cout << "ann_assign\n"; 
    lython::Branch* def = lython::ann_assign();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_for_() {
    std::cout << "for_\n"; 
    lython::Branch* def = lython::for_();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_while_() {
    std::cout << "while_\n"; 
    lython::Branch* def = lython::while_();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_if_() {
    std::cout << "if_\n"; 
    lython::Branch* def = lython::if_();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_with() {
    std::cout << "with\n"; 
    lython::Branch* def = lython::with();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_match() {
    std::cout << "match\n"; 
    lython::Branch* def = lython::match();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_raise() {
    std::cout << "raise\n"; 
    lython::Branch* def = lython::raise();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_try_() {
    std::cout << "try_\n"; 
    lython::Branch* def = lython::try_();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_try_star() {
    std::cout << "try_star\n"; 
    lython::Branch* def = lython::try_star();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_assert_() {
    std::cout << "assert_\n"; 
    lython::Branch* def = lython::assert_();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_import() {
    std::cout << "import\n"; 
    lython::Branch* def = lython::import();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_import_from() {
    std::cout << "import_from\n"; 
    lython::Branch* def = lython::import_from();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_global() {
    std::cout << "global\n"; 
    lython::Branch* def = lython::global();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_nonlocal() {
    std::cout << "nonlocal\n"; 
    lython::Branch* def = lython::nonlocal();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_expression() {
    std::cout << "expression\n"; 
    lython::Branch* def = lython::expression();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_pass() {
    std::cout << "pass\n"; 
    lython::Branch* def = lython::pass();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_break_() {
    std::cout << "break_\n"; 
    lython::Branch* def = lython::break_();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_continue_() {
    std::cout << "continue_\n"; 
    lython::Branch* def = lython::continue_();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_expr() {
    std::cout << "expr\n"; 
    lython::Branch* def = lython::expr();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_bool_() {
    std::cout << "bool_\n"; 
    lython::Branch* def = lython::bool_();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_namedexpr() {
    std::cout << "namedexpr\n"; 
    lython::Branch* def = lython::namedexpr();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_binary() {
    std::cout << "binary\n"; 
    lython::Branch* def = lython::binary();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_unary() {
    std::cout << "unary\n"; 
    lython::Branch* def = lython::unary();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_compare() {
    std::cout << "compare\n"; 
    lython::Branch* def = lython::compare();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_lambda() {
    std::cout << "lambda\n"; 
    lython::Branch* def = lython::lambda();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_ifexp() {
    std::cout << "ifexp\n"; 
    lython::Branch* def = lython::ifexp();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_await() {
    std::cout << "await\n"; 
    lython::Branch* def = lython::await();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_yield() {
    std::cout << "yield\n"; 
    lython::Branch* def = lython::yield();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_yield_from() {
    std::cout << "yield_from\n"; 
    lython::Branch* def = lython::yield_from();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_call() {
    std::cout << "call\n"; 
    lython::Branch* def = lython::call();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_formatted_value() {
    std::cout << "formatted_value\n"; 
    lython::Branch* def = lython::formatted_value();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_joined_str() {
    std::cout << "joined_str\n"; 
    lython::Branch* def = lython::joined_str();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_constant() {
    std::cout << "constant\n"; 
    lython::Branch* def = lython::constant();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_attribute() {
    std::cout << "attribute\n"; 
    lython::Branch* def = lython::attribute();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_subscript() {
    std::cout << "subscript\n"; 
    lython::Branch* def = lython::subscript();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_starred() {
    std::cout << "starred\n"; 
    lython::Branch* def = lython::starred();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_name() {
    std::cout << "name\n"; 
    lython::Branch* def = lython::name();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_list() {
    std::cout << "list\n"; 
    lython::Branch* def = lython::list();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_slice() {
    std::cout << "slice\n"; 
    lython::Branch* def = lython::slice();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_tuple() {
    std::cout << "tuple\n"; 
    lython::Branch* def = lython::tuple();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_dict() {
    std::cout << "dict\n"; 
    lython::Branch* def = lython::dict();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_set() {
    std::cout << "set\n"; 
    lython::Branch* def = lython::set();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_listcomp() {
    std::cout << "listcomp\n"; 
    lython::Branch* def = lython::listcomp();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_setcomp() {
    std::cout << "setcomp\n"; 
    lython::Branch* def = lython::setcomp();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_dictcomp() {
    std::cout << "dictcomp\n"; 
    lython::Branch* def = lython::dictcomp();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}
void test_generatorexp() {
    std::cout << "generatorexp\n"; 
    lython::Branch* def = lython::generatorexp();
    lython::Generator gen;
    def->generate(gen, 0);
    gen.write("\n\n");
}


#include <cstring>

int main(int argc, const char* argv[]) 
{
    using namespace lython;

    auto enabled = [&](const char* name) -> bool {
        if (argc >= 2) {
            int size = std::min(
                strlen(argv[1]),
                strlen(name)
            );

            if (strncmp(argv[1], name, size) == 0) {
                return true;
            } else {
                return false;
            }
        }
        return true;
    };

#if 0
    lython::Branch* def = lython::number();
    
    for (int i = 0; i < 1; i++) {
        lython::Generator gen;
        def->generate(gen, 0);
        gen.write("\n\n");
    }
#else

    if (enabled("match_value"))     { test_match_value();   }
    if (enabled("match_singleton")) { test_match_singleton(); }
    if (enabled("match_sequence"))  { test_match_sequence(); }
    if (enabled("match_mapping"))   { test_match_mapping(); }
    if (enabled("match_class"))     { test_match_class();   }
    if (enabled("match_star"))      { test_match_star();    }
    if (enabled("match_as"))        { test_match_as();      }
    if (enabled("match_or"))        { test_match_or();      }
    if (enabled("statement"))       { test_statement();     }
    if (enabled("functiondef"))     { test_functiondef();   }
    if (enabled("classdef"))        { test_classdef();      }
    if (enabled("return_"))         { test_return_();       }
    if (enabled("del"))             { test_del();           }
    if (enabled("assign"))          { test_assign();        }
    if (enabled("typealias"))       { test_typealias();     }
    if (enabled("aug_assign"))      { test_aug_assign();    }
    if (enabled("ann_assign"))      { test_ann_assign();    }
    if (enabled("for_"))            { test_for_();          }
    if (enabled("while_"))          { test_while_();        }
    if (enabled("if_"))             { test_if_();           }
    if (enabled("with"))            { test_with();          }
    if (enabled("match"))           { test_match();         }
    if (enabled("raise"))           { test_raise();         }
    if (enabled("try_"))            { test_try_();          }
    if (enabled("try_star"))        { test_try_star();      }
    if (enabled("assert_"))         { test_assert_();       }
    if (enabled("import"))          { test_import();        }
    if (enabled("import_from"))     { test_import_from();   }
    if (enabled("global"))          { test_global();        }
    if (enabled("nonlocal"))        { test_nonlocal();      }
    if (enabled("expression"))      { test_expression();    }
    if (enabled("pass"))            { test_pass();          }
    if (enabled("break_"))          { test_break_();        }
    if (enabled("continue_"))       { test_continue_();     }
    if (enabled("expr"))            { test_expr();          }
    if (enabled("bool_"))           { test_bool_();         }
    if (enabled("namedexpr"))       { test_namedexpr();     }
    if (enabled("binary"))          { test_binary();        }
    if (enabled("unary"))           { test_unary();         }
    if (enabled("compare"))         { test_compare();       }
    if (enabled("lambda"))          { test_lambda();        }
    if (enabled("ifexp"))           { test_ifexp();         }
    if (enabled("await"))           { test_await();         }
    if (enabled("yield"))           { test_yield();         }
    if (enabled("yield_from"))      { test_yield_from();    }
    if (enabled("call"))            { test_call();          }
    if (enabled("formatted_value")) { test_formatted_value(); }
    if (enabled("joined_str"))      { test_joined_str();    }
    if (enabled("constant"))        { test_constant();      }
    if (enabled("attribute"))       { test_attribute();     }
    if (enabled("subscript"))       { test_subscript();     }
    if (enabled("starred"))         { test_starred();       }
    if (enabled("name"))            { test_name();          }
    if (enabled("list"))            { test_list();          }
    if (enabled("slice"))           { test_slice();         }
    if (enabled("tuple"))           { test_tuple();         }
    if (enabled("dict"))            { test_dict();          }
    if (enabled("set"))             { test_set();           }
    if (enabled("listcomp"))        { test_listcomp();      }
    if (enabled("setcomp"))         { test_setcomp();       }
    if (enabled("dictcomp"))        { test_dictcomp();      }
    if (enabled("generatorexp"))    { test_generatorexp();  }
#endif

    return 0;
}