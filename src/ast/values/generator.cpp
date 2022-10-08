#include "generator.h"
#include "sema/builtin.h"
#include "vm/tree.h"

namespace lython {

ConstantValue Generator::__next__(struct TreeEvaluator& vm2) {
    //
    // TODO we need to detech when we reach
    // vm.exec(bo)

    /*
        int           depth = 0;
        TreeEvaluator vm(scope);

        for (StmtNode* stmt: generator->body) {
            vm.exec(stmt, depth + 1);

            if (vm.has_exceptions()) {
                break;
            }

            if (vm.yielding) {
                break;
            }

            // We are returning
            if (vm.return_value != nullptr) {
                break;
            }
        }

        vm.raise<StopIteration>();
        */
    return ConstantValue::none();
}
}  // namespace lython