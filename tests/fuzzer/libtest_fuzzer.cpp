// #include "ast/nodes.h"


#include <string>
#include <vector>
#include <iostream>
#include <random>
#include <cassert>
#include <functional>

#include "libtest_fuzzer.h"

int main() 
{
    using namespace lython;

    lython::Branch* def = lython::bool_();
    
    for (int i = 0; i < 1; i++) {
        lython::Generator gen;
        def->generate(gen);
        gen.write("\n\n");
    }

    return 0;
}