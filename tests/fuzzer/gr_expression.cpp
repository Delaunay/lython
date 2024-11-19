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
        // (<expr> <op>)+ <expr>
        self.one_or_more(4)
                .group("comp")
                    .expr().expect(compop())
                .end()
            .end()
        .expr();
    });
}

Branch* lambda() {
    return Builder::make("lambda", [](Builder& self){ 
        // lambda args: expr
        self.atom("lambda")
        .expect(callargs())
        .atom(":")
        .expr()
        ;
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
        // <expr>*, <*<expr>>?, (<identifier> = <expr>)*, <**expr>?
        //  call(1, 2, 3, *args, key=4, key2=5, **kwargs)
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
        self.atom("{")
        .expr()
        .atom(":")
        .expr()
        .atom("}");
    });
}

Branch* joined_str() {
    return Builder::make("joined_str", [](Builder& self){ 
        self.atom("f\"")
            .join("")
                .multiple(10)
                    .group("fmt_str")
                        .either()
                            .string()
                            .formatted_value()
                        .end()
                    .end()
                .end()
            .end()
        .atom("\"");
    });
}

Branch* digit() {
    return Builder::make("digit", [](Builder& self){
        self.either()
            .atom("0")
            .atom("1")
            .atom("2")
            .atom("3")
            .atom("4")
            .atom("5")
            .atom("6")
            .atom("7")
            .atom("8")
            .atom("9")
        .end();
    });
}

Branch* number() {
    // Number with no leading 0
    // we do not want to generate 0000123
    // FIXME: but currently it cannot generate just 0
    return Builder::make("number", [](Builder& self){
        self
        .either()
            .atom("0")
            .group("non_zero")
                .either()
                    .atom("1")
                    .atom("2")
                    .atom("3")
                    .atom("4")
                    .atom("5")
                    .atom("6")
                    .atom("7")
                    .atom("8")
                    .atom("9")
                .end()
                .multiple().expect(digit()).end()
            .end()
        .end();
    });
}

Branch* octal() {
    return Builder::make("octal", [](Builder& self){
        // Max octal = 37777777777 (11 digits)
        self.atom("0o")
            .one_or_more(11)
                .either()
                    .atom("0")
                    .atom("1")
                    .atom("2")
                    .atom("3")
                    .atom("4")
                    .atom("5")
                    .atom("6")
                    .atom("7")
                .end()
            .end();
    });
}

Branch* hex() {
    return Builder::make("hex", [](Builder& self){
        self.atom("0x")
            .one_or_more(8)
                .either()
                    .expect(digit())
                    .atom("a")
                    .atom("b")
                    .atom("c")
                    .atom("d")
                    .atom("e")
                    .atom("f")
                .end()
            .end();
    });
}

Branch* binary_num() {
    return Builder::make("binary_num", [](Builder& self){
        self.atom("0b")
            .one_or_more(32)
                .either()
                    .atom("0")
                    .atom("1")
                .end()
            .end();
    });
}

Branch* float_() {
    return Builder::make("float_", [](Builder& self){
        self.expect(number()).atom(".").expect(number());
    });
}

Branch* scientific() {
    return Builder::make("scientific", [](Builder& self){
        self.either()
            .expect(float_())
            .expect(number())
        .end()
            .atom("e")
            .option().either().atom("+").atom("-").end().end()
            .expect(number());
    });
}

Branch* constant() {
    return Builder::make("constant", [](Builder& self){ 
        self.either()
            // number
            .expect(number())
            // Oct number       0o1
            .expect(octal())
            // Hex number       0x1
            .expect(hex())
            // Binary number    0b1
            .expect(binary_num())
            // string
            .string()
            // float
            .expect(float_())
            // Scientific notation
            .expect(scientific())
            // bool
            .atom("True")
            .atom("False")
            .atom("None")
            // tuples with constant
            .group("tuple_constant")
                .atom("(")
                    .join(", ")
                        .expect(constant())
                    .end()
                .atom(")")
        .end();
    });
}

Branch* attribute() {
    return Builder::make("attribute", [](Builder& self){ 
        self.expr().atom(".").identifier();
            
    });
}

Branch* subscript() {
    return Builder::make("subscript", [](Builder& self){ 
        self.atom("[")
            .either()
                .expr()
                .slice()
            .end()
        .atom("]");
    });
}

Branch* starred() {
    return Builder::make("starred", [](Builder& self){ 
        self.atom("*").identifier();
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
                .join(", ")
                    .expr()    
                .end()
            .atom("]");
    });
}

Branch* slice() {
    return Builder::make("slice", [](Builder& self){ 
        // Slice(expr? lower, expr? upper, expr? step)
        self.expr()                              // lower
            .option().atom(":").expr().end()     // step
            .option().atom(":").expr().end();    // upper  
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
        self.atom("{")
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
        // [ expr for a in expr (if expr)+]
        self.atom("[")
            .expr().atom("for").expr().atom("in").expr()
            .option()
                .multiple(4)
                    .group("filter").atom("if").expr()
                .end()
            .end()
       .atom("]");
    });
}

Branch* setcomp() {
    return Builder::make("setcomp", [](Builder& self){ 
        self.atom("{")
            .expr().atom("for").expr().atom("in").expr()
            .option()
                .multiple(4)
                    .group("filter").atom("if").expr()
                .end()
            .end()
       .atom("}");
    });
}

Branch* dictcomp() {
    return Builder::make("dictcomp", [](Builder& self){ 
        self.atom("{")
            .expr().atom(":").expr().atom("for").expr().atom("in").expr()
            .option()
                .multiple(4)
                    .group("filter").atom("if").expr()
                .end()
            .end()
       .atom("}");
    });
}

Branch* generatorexp() {
    return Builder::make("generatorexp", [](Builder& self){ 
        self.atom("(")
            .expr().atom("for").expr().atom("in").expr()
            .option()
                .multiple(4)
                    .group("filter").atom("if").expr()
                .end()
            .end()
       .atom(")");
    });
}



//
//
//
Branch* boolop() { 
    return Builder::make("boolop", [](Builder& self){
        self.either()
            .atom(" and")
            .atom(" or")
        .end();
    });
}
Branch* binop() {
    return  Builder::make("operator", [](Builder& self){
        self.either()
            .atom(" +")
            .atom(" -")
            .atom(" *")
            .atom(" @")
            .atom(" /")
            .atom(" %")
            .atom(" **")
            .atom(" <<")
            .atom(" >>")
            .atom(" |")
            .atom(" ^")
            .atom(" &")
            .atom(" //")
        .end();
    });
}

Branch* unaryop() {
    return Builder::make("unaryop", [](Builder& self){
        self.either()
            .atom(" ~")
            .atom(" !")
            .atom(" +")
            .atom(" -")
        .end();
    });
}


Branch* compop() { 
    return Builder::make("compop", [](Builder& self){
        self.either()
            .atom(" ==")
            .atom(" !=")
            .atom(" <")
            .atom(" <=")
            .atom(" >")
            .atom(" >=")
            .atom(" is")
            .atom(" is not")
            .atom(" in")
            .atom(" not in")
        .end();
    });
}
}