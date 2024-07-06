#include "libtest_fuzzer.h"


namespace lython {
    Branch* expr() {
    //
    //  Python Expression
    //
    return Builder::make("expression", [](Builder& self){
        self.either()
            .expect(bool_())
            .expect(namedexpr())
            .expect(binary())
            .expect(unary())
            .expect(compare())
            .expect(lambda())
            .expect(ifexp())
            .expect(await())
            .expect(yield())
            .expect(yield_from())
            .expect(call())
            .expect(formatted_value())
            .expect(joined_str())
            .expect(constant())
            .expect(attribute())
            .expect(subscript())
            .expect(starred())
            .expect(name())
            .expect(list())
            .expect(slice())
            .expect(tuple())
            .expect(dict())
            .expect(set())
            .expect(listcomp())
            .expect(setcomp())
            .expect(dictcomp())
            .expect(generatorexp())
        .end();
    });
}


Branch* bool_() {
    return Builder::make("bool_", [](Builder& self){ 
        self.expr().expect(boolop()).expr();
    });
}

Branch* namedexpr() {
    return Builder::make("namedexpr", [](Builder& self){ 
        self.identifier().atom(" := ").expr();
    });
}

Branch* binary() {
    return Builder::make("binary", [](Builder& self){ 
        self.expr().expect(binop()).expr();
    });
}

Branch* unary() {
    return Builder::make("unary", [](Builder& self){ 
        self.expect(unaryop()).expr();
    });
}

Branch* compare() {
    return Builder::make("compare", [](Builder& self){ 

    });
}

Branch* lambda() {
    return Builder::make("lambda", [](Builder& self){ 

    });
}

Branch* ifexp() {
    return Builder::make("ifexp", [](Builder& self){ 
        self.expr().keyword("if").expr().keyword("else").expr();
    });
}

Branch* await() {
    return Builder::make("await", [](Builder& self){ 
        self.keyword("await").expr();
    });
}

Branch* yield() {
    return Builder::make("yield", [](Builder& self){ 
        self.keyword("yield").expr();
    });
}

Branch* yield_from() {
    return Builder::make("yield_from", [](Builder& self){ 
        self.keyword("yield from").expr();
    });
}

Branch* callargs() {
   return Builder::make("callargs", [](Builder& self){
        self
            .join(", ")
                .multiple().expr().end()
                .option().atom("*").expr().end()
                .multiple().identifier().atom("=").expr().end()
                .option().atom("**").expr().end()
            .end()
        .end();
   }); 
}

Branch* call() {
    return Builder::make("call", [](Builder& self){ 
        self.identifier()
            .atom("(")
            .expect(callargs())
            .atom(")");
    });
}

Branch* formatted_value() {
    return Builder::make("formatted_value", [](Builder& self){ 

    });
}

Branch* joined_str() {
    return Builder::make("joined_str", [](Builder& self){ 

    });
}

Branch* constant() {
    return Builder::make("constant", [](Builder& self){ 

    });
}

Branch* attribute() {
    return Builder::make("attribute", [](Builder& self){ 
        self.expr().atom(".").identifier();
            
    });
}

Branch* subscript() {
    return Builder::make("subscript", [](Builder& self){ 
        self.atom("[").expr().atom("]");
    });
}

Branch* starred() {
    return Builder::make("starred", [](Builder& self){ 

    });
}

Branch* name() {
    return Builder::make("name", [](Builder& self){ 
        self.identifier();
    });
}

Branch* list() {
    return Builder::make("list", [](Builder& self){ 
        self.atom("[")
                .join(", ").expr().end()
            .atom("]");
    });
}

Branch* slice() {
    return Builder::make("slice", [](Builder& self){ 

    });
}

Branch* tuple() {
    return Builder::make("tuple", [](Builder& self){ 
        self.atom("(")
                .join(", ").expr().end()
            .atom(")");
    });
}

Branch* dict() {
    return Builder::make("dict", [](Builder& self){ 
        self
            .atom("{")
                .join(", ").expr().atom(": ").expr().end()
            .atom("}");
    });
}

Branch* set() {
    return Builder::make("set", [](Builder& self){ 
        self.atom("{")
                .join(", ").expr().end()
            .atom("}");
    });
}

Branch* listcomp() {
    return Builder::make("listcomp", [](Builder& self){ 
       
    });
}

Branch* setcomp() {
    return Builder::make("setcomp", [](Builder& self){ 

    });
}

Branch* dictcomp() {
    return Builder::make("dictcomp", [](Builder& self){ 

    });
}

Branch* generatorexp() {
    return Builder::make("generatorexp", [](Builder& self){ 

    });
}



//
//
//
Branch* boolop() { 
    return Builder::make("boolop", [](Builder& self){
        self.either()
            .keyword(" and")
            .keyword(" or")
        .end();
    });
}
Branch* binop() {
    return  Builder::make("operator", [](Builder& self){
        self.either()
            .keyword(" +")
            .keyword(" -")
            .keyword(" *")
            .keyword(" @")
            .keyword(" /")
            .keyword(" %")
            .keyword(" **")
            .keyword(" <<")
            .keyword(" >>")
            .keyword(" |")
            .keyword(" ^")
            .keyword(" &")
            .keyword(" //")
        .end();
    });
}

Branch* unaryop() {
    return Builder::make("unaryop", [](Builder& self){
        self.either()
            .keyword(" ~")
            .keyword(" !")
            .keyword(" +")
            .keyword(" -")
        .end();
    });
}


Branch* compop() { 
    return Builder::make("compop", [](Builder& self){
        self.either()
            .keyword(" ==")
            .keyword(" !=")
            .keyword(" <")
            .keyword(" <=")
            .keyword(" >")
            .keyword(" >=")
            .keyword(" is")
            .keyword(" is not")
            .keyword(" in")
            .keyword(" not in")
        .end();
    });
}
}