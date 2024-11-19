#include "libtest_fuzzer.h"



namespace lython {
    struct Branch;

    Branch* match_value() ;
    Branch* match_singleton();
    Branch* match_sequence();
    Branch* match_mapping();
    Branch* match_class();
    Branch* match_star() ;
    Branch* match_as() ;
    Branch* match_or() ;

    Branch* pattern() {
        return Builder::make("pattern", [](Builder& self){
            self.either()
                .expect(match_value())
                .expect(match_singleton())
                .expect(match_sequence())
                .expect(match_mapping())
                .expect(match_class())
                .expect(match_star())
                .expect(match_as())
                .expect(match_or())
            .end();
        });
    }


    Branch* match_value() {
        return Builder::make("match_value", [](Builder& self) {
            self.expr();
        });
    }

    Branch* match_singleton() {
        return Builder::make("match_singleton", [](Builder& self) {
            return self.expect(constant());
        });
    }

    Branch* match_sequence() {
        return Builder::make("match_sequence", [](Builder& self) {
            self.atom("[")
                .join(", ")
                    .one_or_more(5).expect(pattern()).end()
            .atom("]");
        });
    }
    
    Branch* match_mapping() {
        return Builder::make("match_mapping", [](Builder& self) {
            self.atom("{")
                .join(", ")
                    .one_or_more(5)
                        .group("mapping")
                            .expr().atom(":").expect(pattern())
                        .end()
                    .end()
                    // rest of the mapping
                    .option()
                        .identifier()
                    .end()
            .atom("}");
        });
    }

    Branch* match_class() {
        return Builder::make("match_class", [](Builder& self) {
            self.identifier()
                .atom("(")
                .one_or_more(5).expect(pattern()).end()
                .atom(")");
        });
    }

    Branch* match_star() {
        return Builder::make("match_star", [](Builder& self) {
            self.atom("*").identifier();
        });
    }

    Branch* match_as() {
        return Builder::make("match_as", [](Builder& self) {
            self.expect(pattern()).keyword("as").identifier();
        });
    }

    Branch* match_or() {
        return Builder::make("match_or", [](Builder& self) {
            self.one_or_more(5).expect(pattern()).keyword("or").end().expect(pattern());
        });
    }
}