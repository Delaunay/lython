#include "libtest_fuzzer.h"


namespace lython {

Branch* mod() {
    //
    //  Python Module
    //
    return Builder::make("mod", [](Builder& self){
        self.either()
            // Module
            .group("module")
                .multiple().body().end()
                .end()
            // Interactive
            .group("interactive")
                .end()
            // Expression
            .group("expression")
                .expr()
                .end()
            // FunctionType
            .group("function_type")
                .multiple().expr().end().keyword(" -> ").expr()
                .end()
        .end();
    });
}
}