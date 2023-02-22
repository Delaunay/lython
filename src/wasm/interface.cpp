#include <emscripten/bind.h>

// Buffer
EMSCRIPTEN_BINDINGS(my_class_example) {
    class_<StringBuffer>("StringBuffer")
        .constructor<int, std::string>()
        .function("incrementX", &MyClass::incrementX)
        .property("x", &MyClass::getX, &MyClass::setX)
        .class_function("getStringFromInstance", &MyClass::getStringFromInstance);
}

// Lexer
EMSCRIPTEN_BINDINGS(my_class_example) {
    class_<Lexer>("Lexer")
        .constructor<int, std::string>()
        .function("incrementX", &MyClass::incrementX)
        .property("x", &MyClass::getX, &MyClass::setX)
        .class_function("getStringFromInstance", &MyClass::getStringFromInstance);
}

// Parser
EMSCRIPTEN_BINDINGS(my_class_example) {
    class_<Parser>("Parser")
        .constructor<int, std::string>()
        .function("incrementX", &MyClass::incrementX)
        .property("x", &MyClass::getX, &MyClass::setX)
        .class_function("getStringFromInstance", &MyClass::getStringFromInstance);
}

// SEMA
EMSCRIPTEN_BINDINGS(my_class_example) {
    class_<SemanticAnalyser>("SemanticAnalyser")
        .constructor<int, std::string>()
        .function("incrementX", &MyClass::incrementX)
        .property("x", &MyClass::getX, &MyClass::setX)
        .class_function("getStringFromInstance", &MyClass::getStringFromInstance);
}

// VM
EMSCRIPTEN_BINDINGS(my_class_example) {
    class_<TreeEvaluator>("TreeEvaluator")
        .constructor<int, std::string>()
        .function("incrementX", &MyClass::incrementX)
        .property("x", &MyClass::getX, &MyClass::setX)
        .class_function("getStringFromInstance", &MyClass::getStringFromInstance);
}
