#include "tide/ast_input.h"


namespace lython {
    InputState& InputState::state() {
        static InputState state;
        return state;
     }
}