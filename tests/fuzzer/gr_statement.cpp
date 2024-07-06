#include "libtest_fuzzer.h"


namespace lython {

Branch* statement() {
    //
    // Pyton Statement
    //
    return Builder::make("statement", [](Builder& self){
        self.either()
            // funcrion & async
            .expect(functiondef())
            .expect(classdef())
            .expect(return_())
            .expect(del())
            .expect(assign())
            .expect(aug_assign())
            .expect(ann_assign())
            .expect(for_())
            .expect(while_())
            .expect(if_())
            .expect(with())
            .expect(match())
            .expect(raise())
            .expect(try_())
            .expect(try_star())
            .expect(assert_())
            .expect(import())
            .expect(import_from())
            .expect(global())
            .expect(nonlocal())
            .expect(expr())
            .expect(pass())
            .expect(break_())
            .expect(continue_())
        .end()
        .newline();
    });
}

Branch* args() {
   return Builder::make("args", [](Builder& self){
        self.join(", ")
            .multiple().identifier()
                .option().atom(": ").identifier()
                .option().atom(" = ").identifier()
            .end()
        .end();
   }); 
}


Branch* functiondef() {
    return Builder::make("functiondef", [](Builder& self){
        self.option()
            .multiple()
                .atom('@').expect(call()).newline()
            .end()
        .end()
        .option().keyword("async").end()
        .keyword("def").identifier().atom('(').expect(args()).atom(')')
            .option()
                .keyword(" -> ").type()
            .end()
            .atom(':')
            .newline()
            .indent()
                .either()
                    .docstring()
                    .body().end()
                    .keyword("pass")
                .end()
            .end();
    });
}

Branch* classdef() {
    return Builder::make("classdef", [](Builder& self){ 

    });
}
Branch* return_() {
    return Builder::make("return", [](Builder& self){ 
        self.keyword("return")
            .option().expr().end();
    });
}
Branch* del() {
    return Builder::make("del", [](Builder& self){ 
        self.keyword("del")
            .option().expr().end();
    });
}
Branch* assign() {
    return Builder::make("assign", [](Builder& self){ 
        self.identifier().atom(" = ").expr();
    });
}
Branch* typealias() {
    return Builder::make("typealias", [](Builder& self){ 

    });
}
Branch* aug_assign() {
    return Builder::make("aug_assign", [](Builder& self){ 
        self.identifier().expect(binop()).atom("=").expr();
    });
}
Branch* ann_assign() {
    return Builder::make("ann_assign", [](Builder& self){ 
        self.identifier().atom(": ").identifier().atom(" = ").expr();
    });
}
Branch* for_() {
    return Builder::make("for", [](Builder& self){ 

    });
}
Branch* while_() {
    return Builder::make("while", [](Builder& self){ 

    });
}
Branch* if_() {
    return Builder::make("if", [](Builder& self){ 

    });
}
Branch* with() {
    return Builder::make("with", [](Builder& self){ 

    });
}
Branch* match() {
    return Builder::make("match", [](Builder& self){ 

    });
}
Branch* raise() {
    return Builder::make("raise", [](Builder& self){ 

    });
}
Branch* try_() {
    return Builder::make("try", [](Builder& self){ 

    });
}
Branch* try_star() {
    return Builder::make("try_star", [](Builder& self){ 

    });
}
Branch* assert_() {
    return Builder::make("assert", [](Builder& self){ 
        self.keyword("assert").expr().atom(", ").string();
    });
}
Branch* import() {
    return Builder::make("import", [](Builder& self){ 
        self.keyword("import");
    });
}
Branch* import_from() {
    return Builder::make("import_from", [](Builder& self){ 
        self.keyword("from")
            .keyword("import");
    });
}
Branch* global() {
    return Builder::make("global", [](Builder& self){ 

    });
}
Branch* nonlocal() {
    return Builder::make("nonlocal", [](Builder& self){ 

    });
}
Branch* expression() {
    return Builder::make("expression", [](Builder& self){ 
        return expr();
    });
}
Branch* pass() {
    return Builder::make("pass", [](Builder& self){ 
        self.keyword("pass");
    });
}
Branch* break_() {
    return Builder::make("break", [](Builder& self){ 
        self.keyword("break");
    });
}
Branch* continue_() {
    return Builder::make("continue", [](Builder& self){ 
        self.keyword("continue");
    });
}

//
//
//

Branch* body() {
    return Builder::make("body", [](Builder& self){
        self.multiple()
            .statement()
        .end();
    });
}



}