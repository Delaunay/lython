#include "interpreter.h"
#include "ast/visitor.h"

#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

namespace lython {

Value builtin_sin(Array<Value>& args);
Value builtin_max(Array<Value>& args);
Value builtin_div(Array<Value>& args);
Value builtin_mult(Array<Value>& args);


struct InterpreterImpl: public ConstVisitor<InterpreterImpl, Value>{
    //! Execution environment
    Array<Value>  env;
    Array<String> str;

    //! Builtin function defined in C++
    Dict<String, Value::BuiltinImpl> builtins = {
        {"max", builtin_max},
        {"sin", builtin_sin},
        {"/", builtin_div},
        {"*", builtin_mult},
    };

    void push(Value v, String const& name = ""){
        // debug("{} = {}", name, v);
        env.push_back(v);
        str.push_back(name);
    }

    void pop(std::ptrdiff_t n){
        debug("{}", n);
        env.erase(env.begin() + n, env.end());
        str.erase(str.begin() + n, str.end());
    }

    std::ostream& dump_env(std::ostream& out){
        out << String(50, '-') << '\n';
        out << str.size() << " " << env.size() << '\n';
        for(auto i = 0ul; i < env.size(); ++i){
            out << fmt::format("{:4d} | {:20} | {} \n", i, str[i], env[i]);
        }

        out << String(50, '-') << '\n';
        return out;
    }

    InterpreterImpl(Module& m){
        // Eval the module en create the environment for the interpreter
        for(int i = 0; i < m.size(); ++i){
            debug("{}", m.get_name(i).c_str());
            Expression exp = m[i];
            auto v = eval(exp);
            push(v, m.get_name(i));
        }
        debug("Module evaluated (env: {})", env.size());

        debug("Dumping env");
        dump_env(std::cout);
        debug("---");
    }

    Value eval(Expression const expr, std::size_t d=0){
        return visit(expr, d);
    }

    Value imported_expression(ImportedExpr_t imp, std::size_t d){
        // we need to memoize this
        Value module = eval(imp->import, d + 1);


        return String("fake_import");
    }

    Value import(Import_t import, size_t depth){
        // TODO tweak research paths
        static String search_path = "/home/setepenre/work/lython/code";

        // String path = import->path;
        // Look for <path> in module lookup path
        // load module object and return
        String file_path;

        // in the current state there are different way this could be implemented
        // what we should do is parse the module-import during the parsing phase
        // because we need the information for type checking
        // so we have to do it there NOT here like I am doing.
        // because the module is already parsed we only need to eval the module
        // which means the Import is actually a Module and ImportExpr is a referencce
        // into that Module

        FileBuffer reader(file_path);
        Lexer lex(reader);
        Module module;
        Parser par(reader, &module);

        // import <path_1>...<path_n>
        // insert  path_1 as an module object into the current environment
        if (import->imports.size() == 0){

        } else {
            // from a.b.c import f
            // insert f as an imported expression

            for(auto& imp: import->imports){
                String name;

                if (imp.import_name){
                    name = imp.import_name.str();
                } else {
                    name = imp.export_name.str();
                }
            }
        }

        return Value("none");
    }

    Array<Value> eval(Array<Expression> const & exprs, size_t depth){
        trace_start(depth, "eval_args");

        Array<Value> values;
        values.reserve(exprs.size());

        for (auto& expr: exprs)
            values.push_back(eval(expr, depth));

        return values;
    }

    Value undefined(Node_t, std::size_t){
        return Value("undefined");
    }

    Value parameter(Parameter_t, std::size_t) {
        return Value("parameter");
    }

    Value unary(UnaryOperator_t, size_t) {
        return Value("unary");
    }

    Value binary(BinaryOperator_t bin, std::size_t d) {
        trace_start(d, "{} {} {}", bin->lhs, bin->op, bin->rhs);

        // StringDatabase::instance().report(std::cout);

        // retrieval attribute
        if (bin->op == get_string(".")){
            auto obj = eval(bin->lhs, d);
            debug("retrieve attribute {}", obj);
            assert(obj.tag == ValueKind::obj_object, "retrieve attribute");
            value::Struct& data = *obj.get<value::Struct*>();
            return data.get_attributes(bin->rhs.ref<AST::Ref>()->name);
        }

        // assignment
        if (bin->op == get_string("=")){
            switch(bin->lhs.kind()){
            // assign to variable
            case AST::NodeKind::KReference: {
                debug("Assign to a Reference");

                auto rhs = eval(bin->rhs, d);
                push(rhs, bin->lhs.ref<AST::Ref>()->name.str());
                return Value("none");
            }
            // assign to object
            case AST::NodeKind::KBinaryOperator: {
                debug("Assign to an object attribute");

                auto new_value = eval(bin->rhs, d);

                AST::BinaryOperator const* attr = bin->lhs.ref<AST::BinaryOperator>();
                assert(attr->op == get_string("."), "Expected getattr operator");

                // fetch the p in p.x
                auto obj = eval(attr->lhs, d);
                auto attr_name = attr->rhs.ref<AST::Ref>()->name;
                assert(obj.tag == ValueKind::obj_object, "Assign to an object");

                // set x to lhs
                value::Struct& data = *obj.get<value::Struct*>();
                data.set_attribute(attr_name, new_value);

                // dump_env(std::cout);
                return Value("none");
            }

            default:
                return Value("Unsupported");
            }
        } else {
            auto lhs = eval(bin->lhs, d);
            auto rhs = eval(bin->rhs, d);
        }

        return Value("binary");
    }

    Value sequential(SeqBlock_t seq, std::size_t depth) {
        trace_start(depth, "seq_block");

        size_t n = size_t(std::max(int(seq->blocks.size()) - 1, 0));

        for(size_t i = 0; i < n; ++i)
            eval(seq->blocks[i], depth + 1);

        // returns last line of sequential block
        return eval(seq->blocks[n], depth);
    }

    Value unparsed(UnparsedBlock_t, std::size_t) {
        return Value("unparsed");
    }

    Value statement(Statement_t stmt, std::size_t depth) {
        trace_start(depth, "{} {}", stmt->statement, stmt->expr);
        return eval(stmt->expr, depth);
    }

    Value reference(Reference_t ref, std::size_t depth) {
        auto n = env.size() - std::size_t(ref->index);
        if (n >= env.size()){
            debug("{}: {}, length={} | size={} | fetch={}",
                  ref->name,
                  ref->index,
                  ref->length,
                  env.size(),
                  n);
        }

        auto r = env[n];
        trace_start(depth, "{}: {}, length={} | size={} | fetch={} => {}",
                    ref->name,
                    ref->index,
                    ref->length,
                    env.size(),
                    n,
                    r);
        return r;
    }

    Value builtin(Builtin_t blt, std::size_t depth) {
        trace_start(depth, "{}", blt->name);
        auto fun = builtins[blt->name.str()];
        return Value(fun, Array<Value>());
    }

    Value value(Value_t val, std::size_t depth) {
        trace_start(depth, "value {}", val->value);
        return val->value;
    }

    Value struct_type(Struct_t cstruct, std::size_t) {
        return Value(cstruct);
    }

    Value call(Call_t call, std::size_t depth) {
        trace_start(depth, "{}", call->function);
        Value closure = eval(call->function, depth);
        switch (closure.tag) {
        // standard function
        case ValueKind::obj_closure: return fun_call(closure, call, depth);

        // Calling a struct type
        case ValueKind::obj_class: return struct_call(closure, call, depth);
        }

        return closure;
    }

    Value struct_call(Value closure, Call_t call, std::size_t depth){
        trace_start(depth, "struct constructor");

        AST::Struct const* cstruct = closure.get<value::Class*>()->fun;
        Value v = new_object(cstruct);
        value::Struct& data = *v.get<value::Struct*>();

        assert(call->arguments.size() == cstruct->attributes.size(),
               "arguments should match attributes");

        // for all positional arguments
        auto n = call->arguments.size();

        for(auto i = 0u; i < n; ++i){
            data.set_attribute(i, eval(call->arguments[i], depth));
        }

        for(auto& item: call->kwargs){
           data.set_attribute(item.first, eval(item.second, depth));
        }

        // v.v_object.attributes;
        return v;
    }

    Value fun_call(Value closure, Call_t call, std::size_t depth){
        trace_start(depth, "fun_call");

        auto on = std::ptrdiff_t(env.size());

        for (auto& expr: call->arguments)
            push(eval(expr, depth));

        Value returned_value = eval_closure(closure, depth);

        pop(on);
        // debug("return {}", returned_value);
        return returned_value;
    }

    Value arrow(Arrow_t aw, std::size_t d) {
        // this should not be called at runtime
        // Note that there is a compile time interpreter running as well
        return Value("arrow");
    }

    Value type(Type_t type, std::size_t) {
        return Value(type->name);
    }

    Value function(Function_t fun, std::size_t) {
        // make a closure out a function
        return Value(fun, env);
    }

    // Helpers
    // -------
    Value eval_closure(Value fun, std::size_t depth){
        trace_start(depth, "{}", fun);

        if (fun.tag == ValueKind::obj_closure){
            value::Closure* clo = fun.get<value::Closure*>();

            if (!clo->fun){
                assert(clo->builtin, "Closure is undefined");
                auto v = clo->builtin(env);
                return v;
            } else {
                const AST::Function* fun_decl = fun.get<value::Closure*>()->fun;
                assert(fun_decl, "fun should be defined");
                return eval(fun_decl->body, depth);
            }
        }


        return Value("NotImplemented");
    }
};

Interpreter::Interpreter(Module& m):
    ptr(new InterpreterImpl(m))
{}

Interpreter::~Interpreter(){
    delete ptr;
}

Value Interpreter::eval(Expression const expr){
    return ptr->eval(expr);
}

Value builtin_sin(Array<Value>& args){
    assert(args.size() == 2, "expected 1 arguments");

    auto v = args[1].get<float64>();

    // std::cout << "sin(" << v << ")";
    return std::sin(v);
}


Value builtin_max(Array<Value>& args){
    debug("calling max");
    assert(args.size() >= 3, "expected 2 arguments");

    auto n = args.size();

    auto a = args[n - 2];
    auto b = args[n - 1];

    return std::max(a.get<float64>(), b.get<float64>());
}


Value builtin_div(Array<Value>& args){
    assert(args.size() >= 3, "expected 2 arguments");

    auto a = args[1].get<float64>();
    auto b = args[2].get<float64>();

    //std::cout << "div(" << a << ", " << b << ")";
    return a / b;
}


Value builtin_mult(Array<Value>& args){
    assert(args.size() >= 3, "expected 2 arguments");

    auto a = args[1].get<float64>();
    auto b = args[2].get<float64>();

    //std::cout << "mult(" << a << ", " << b << ")";
    return a * b;
}

}
