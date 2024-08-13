#include <iostream>
#include "ast/nodes.h"


namespace lython {

template<typename T>
void generate_doc(char const*  type_name, char const* function_name) {

    int type_id = meta::type_id<T>();
    meta::ClassMetadata& meta = meta::classmeta<T>();

    std::cout << fmt::format("{} {} {}\n", type_name, meta.name, function_name);
    std::cout << fmt::format(" type_id = {:4d} weakref = {:4d}\n", 
        meta.type_id, meta.weakref_type_id);

    std::cout << fmt::format(" size    = {:4d} trivial = {:4d}\n", meta.size, meta.is_trivially_copyable);

    std::cout << fmt::format(" deleter = {} copier  = {} printer = {}\n", 
                    (void*)meta.deleter, (void*)meta.copier, (void*)meta.printer
    );
    std::cout << fmt::format(" hasher  = {} ref     = {} assign  = {}\n", 
            (void*)meta.hasher, (void*)meta.ref, (void*)meta.assign);

    std::cout << " stat\n";
    std::cout << fmt::format("  all={} free={} \n", meta.stat.allocated, meta.stat.deallocated);
    
    // std::cout << "  " << meta.stat.allocated << "\n";
    // std::cout << "  " << meta.stat.deallocated << "\n";
    // std::cout << "  " << meta.stat.bytes << "\n";
    // std::cout << "  " << meta.stat.size_alloc << "\n";
    // std::cout << "  " << meta.stat.size_free << "\n";
    // std::cout << "  " << meta.stat.startup_count << "\n";

    if (meta.members.size()){
        std::cout << " member\n";
        for(meta::Property& mem: meta.members) {
            std::cout << fmt::format("  {}\n", mem.name);
            std::cout << fmt::format("  {}\n", mem.type);
            std::cout << fmt::format("  {}\n", mem.offset);
            std::cout << fmt::format("  {}\n", mem.size);
            std::cout << fmt::format("  {}\n", (void*)mem.native);
            std::cout << fmt::format("  {}\n", (void*)mem.method);
        }
    }
    std::cout << "\n";    
}


void generate_all() {
    #define FUNCTION_GEN(name, fun, rtype)\
        generate_doc<name>(#name, #fun);

    #define X(name, _)
    #define SSECTION(name)
    #define EXPR(name, fun)  FUNCTION_GEN(name, fun, ExprRet)
    #define STMT(name, fun)  FUNCTION_GEN(name, fun, StmtRet)
    #define MOD(name, fun)   FUNCTION_GEN(name, fun, ModRet)
    #define MATCH(name, fun) FUNCTION_GEN(name, fun, PatRet)
    #define VM(name, fun)    FUNCTION_GEN(name, fun, StmtRet)

    NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH, VM)

    #undef X
    #undef SSECTION
    #undef EXPR
    #undef STMT
    #undef MOD
    #undef MATCH
    #undef VM
}

}

int main(int argc, const char* argv[]) 
{
    lython::generate_all();

    return 0;
}