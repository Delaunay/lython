#include "Parser.h"
#include <cassert>


#if PARSER_DBG
// Add the function to the current trace
#   define ADD_TRACE add_trace(_intern_trace, idt);
// Print partial results
//#   define PRINT_EXPR(expr) expr->print(std::cout); std::cout << "\n";
// Make the progrom print out trace info and force the program to crash
#   define CRASH_PARSER info<int>("Parser Trace Info", lexer.line(), lexer.col(), &_intern_trace, _out);\
                        _out.flush();\
                        assert(false);
// Print out the trace and erase it
#   define SHOWTRACE    if (_intern_trace.traceback.size() != 1 && _print_out){\
                        info<int>("Parser Trace Info", lexer.line(), lexer.col(), &_intern_trace, _out);}\
                        _intern_trace.erase();
#else
#   define ADD_TRACE
#   define PRINT_EXPR(expr)
#   define CRASH_PARSER "lookup how to make the compiler Scream" + 1 //
#   define SHOWTRACE
#endif

#ifndef PRINT_EXPR
#   define PRINT_EXPR(expr)
#endif

// Print expression after a successful Parsing
#define PRINT_PARSED_EXPR(expr) expr->print(_out);

// Print Evaluated expression
#define REPORT_RESULT(x) std::cout << ">> " << x << std::endl

// Use LLVM to generate Assembly code given the AST object
#if LLVM_CODEGEN
#   include <llvm/Support/raw_ostream.h>
#   define LLVM_CODE_GEN(f) \
        llvm::Function *__lf = NEW_GEN(f->code_gen(_gen));\
        std::string __s;\
        llvm::raw_string_ostream __os(__s);\
        if(__lf)\
        {\
            __lf->print(__os);\
            _out << MYELLOW << __os.str() << MRESET "\n";\
        }
#else
#   define LLVM_CODE_GEN(f)
#endif

// Cast it to the right type (takes no arguments, returns a double) so we
// can call it as a native function.
#if LLVM_JIT && LLVM_CODEGEN
#   define LLVM_CODE_EXEC(lf) void* fptr = _gen.exec_engine->getPointerToFunction(lf);\
                              double (*FP)() = (double (*)())(intptr_t)fptr;\
                              REPORT_RESULT(FP());
#else
#   define LLVM_CODE_EXEC(lf)
#endif

// Garbage Collector Macro
#define NEW_EXPR(x)     _object.new_expr(new x);
#define NEW_FUNCT(x)    _object.new_function(new x)
#define NEW_PROTO(x)    _object.new_prototype(new x)
#define NEW_GEN(x) x

// Simplify the expression if possible
#define RETURN_VECTOR(m)    if (m->expressions.size() == 1)\
                                return (m->expressions)[0];\
                            else\
                                return m;

#define RETURN_ERROR(msg)  add_trace(_intern_trace, idt); \
                           return error<AST::Expression>(msg, lexer.line(), lexer.col(), &_intern_trace, _out)

#define RETURN_ERRORP(msg) add_trace(_intern_trace, idt); \
                           return error<AST::Prototype>(msg, lexer.line(), lexer.col(), &_intern_trace, _out)

#define ERROR(msg)  add_trace(_intern_trace, idt); \
                    error<AST::Expression>(msg, lexer.line(), lexer.col(), &_intern_trace, _out)

#define ERRORP(msg) add_trace(_intern_trace, idt); \
                    error<AST::Prototype>(msg, lexer.line(), lexer.col(), &_intern_trace, _out)

#define EAT_TOK(x, err) if (token() != x) \
                            RETURN_ERROR(err); \
                        next_token()\

#define PRINT_TOK() printf("%d [%d %d] %c\n", token(), lexer.line(), lexer.col(), token())

namespace LIBNAMESPACE
{

const int& Parser::token() const
{
    return _token;
}

const string& Parser::identifier() const
{
    return lexer.identifier();
}

const int& Parser::next_token()
{
    _token = lexer.get_token();
    _past_token[_tk_idx % HST_SIZE] = _token;
    _tk_idx++;

    return _token;
}

Parser::Parser(Lexer& lex, std::ostream& out):
    lexer(lex), _token(0), _tk_idx(0),
    _print_out(true), _out(out), _indent(0),
    _past_token(vector<int>(HST_SIZE))
#if LLVM_CODEGEN
    , _gen(_op, _object)
#endif
{}

AST::Expression* Parser::parse_number_expression(int idt)
{
    ADD_TRACE

    AST::Expression* expr = NEW_EXPR(AST::FloatExpression(lexer.value()));
    next_token();

    PRINT_EXPR(expr)
    return expr;
}

AST::Expression* Parser::parse_string_expression(int idt)
{
    ADD_TRACE

    AST::Expression* expr = NEW_EXPR(AST::StringExpression(lexer.identifier()));
    next_token();

    PRINT_EXPR(expr)
    return expr;
}

// Expression inside parenthesis
AST::Expression* Parser::parse_parent_expression(int idt)
{
    ADD_TRACE
    next_token();   // Eat '('

    AST::Expression* expr = parse_simple_expression(idt + 1);
                          //parse_expression(idt + 1);

    if (!expr){
        return 0;
    }

    // Eat ')'
    if (token() == ')')
    {
        next_token();
        PRINT_EXPR(expr)
        return expr;
    }

    RETURN_ERROR("Expected ')'");
}

// identifier '(' expression* ')'
#define PARENS_OPEN(x) ((x == '(') || (x == '['))
#define PARENS_CLOSE(x) ((x == ')') || (x == ']'))


AST::Expression* Parser::parse_identifier_expression(int idt)
{
    ADD_TRACE

    string idname = lexer.identifier();
    next_token();

    // if it is not a function then it is a Variable
    if (!PARENS_OPEN(token())){
        return NEW_EXPR(AST::VariableExpression(idname));
    }

    char c = token();
    next_token(); // '('

    AST::CallExpression::Arguments args;

    if (!PARENS_CLOSE(token()))
    {
        // extract arguments
        while(1)
        {
            AST::Expression* arg = parse_simple_expression(idt + 1);
                                 //parse_expression(idt + 1);

            if (!arg)
                return 0;

            args.push_back(arg);

            if (PARENS_CLOSE(token())){
                break;
            }

            if (token() != ','){
                RETURN_ERROR("Expected ')' or ',' in argument list");
            }

            next_token();
        }
    }

    next_token(); // ')'

    return NEW_EXPR(AST::CallExpression(idname, args, c));
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
AST::Expression* Parser::parse_primary(int idt)
{
    ADD_TRACE

    while (token() == tok_newline)
        next_token();

    switch(token())
    {
    case tok_identifier: // - 4
        return parse_identifier_expression(idt + 1);

    case tok_number:     // - 5
        return parse_number_expression(idt + 1);

    case tok_string_lit:
        return parse_string_expression(idt + 1);

    case '(':
        return parse_parent_expression(idt + 1);

    case tok_indent:
        return parse_multiline_expression(idt + 1); //parse_expression(idt + 1);

    case tok_if:
        return parse_if_expression(idt + 1);

    case tok_for:
        return parse_for_expression(idt + 1);

    case tok_else:
    case tok_def:
        return 0;

    case tok_desindent:
        next_token();
        return 0;

    default:
        printf("%d [%d %d] %c\n", token(), lexer.line(), lexer.col(), token());
        RETURN_ERROR("Unknown token when expecting an expression");
    }
}

const int Parser::precendence()
{
    string op;

    if (token() == tok_identifier)
        op = identifier();
    else
        op += token();

    int tokpre = _op[op]; //binpre[token()];

    if (tokpre <= 0)
        return -1;

    return tokpre;
}

AST::Expression* Parser::parse_simple_expression(int idt)
{
    ADD_TRACE

    AST::Expression* lhs = parse_unary(idt + 1);

    PRINT_EXPR(lhs)

    // Complex Expression cant be inside BinOp
    // Such as For Expression, Class, function definition
    if (lhs->complex)
        return lhs;

    if (!lhs)
        return 0;

    return parse_bin_op_rhs(0, lhs, idt + 1);
}

AST::Expression* Parser::parse_bin_op_rhs(int exppre, AST::Expression* lhs, int idt)
{
    while(1 && token())
    {
        ADD_TRACE

        int tokpre = precendence();

        if (tokpre < exppre)
            return lhs;

        string binop;

        // Bin op is an identifier
        if (token() == tok_identifier)
            binop = identifier();

        // Bin op is a character
        else
            binop += token();

        next_token();

        // Check if it is as binop
        // this the line of code that allow b++ <===
        if (_op.is_operator(binop) == 1){
            return NEW_EXPR(AST::UnaryExpression(binop, lhs));
        }

        AST::Expression* rhs = parse_unary(idt + 1);

        if (!rhs)
            return 0;

        int nextpre = precendence();

        if (tokpre < nextpre)
        {
            rhs = parse_bin_op_rhs(tokpre + 1, rhs, idt + 1);

            if (rhs == 0){
                return 0;
            }
        }

        lhs = NEW_EXPR(AST::BinaryExpression(binop, lhs, rhs));
        PRINT_EXPR(lhs)
    }
}

AST::Expression* Parser::parse_if_expression(int idt)
{
    ADD_TRACE

    next_token();

    AST::Expression* cond = parse_simple_expression(idt + 1);

    if (!cond){
        return 0;
    }

    if (token() != ':'){
        RETURN_ERROR("expected ':'");
    }

    next_token();

    AST::Expression* then = parse_multiline_expression(idt + 1);
                          //parse_expression(idt + 1);

    if (then == 0)
        return 0;

    if (token() != tok_else){
        RETURN_ERROR("expected 'else' statement");
    }
    next_token();

    if (token() != ':'){
        RETURN_ERROR("expected ':'");
    }
    next_token();

    AST::Expression* el = parse_multiline_expression(idt + 1);
                        //parse_expression(idt + 1);

    if (!el){
        return 0;
    }

    PRINT_EXPR(el)
    return NEW_EXPR(AST::IfExpression(cond, then, el));
}

AST::Prototype* Parser::parse_prototype(int idt)
{
    ADD_TRACE

    string fnname;
    unsigned kind = 0;
    unsigned prec = 30;

    switch(token())
    {
    case tok_identifier:
        fnname = identifier();
        kind = 0;
        next_token();
        break;

    case tok_unary:
        next_token();

        if (!isascii(token())){
            RETURN_ERRORP("Expected unary operator");
        }

        fnname = "unary";
        fnname += (char) token();

        kind = 1;

        next_token();
        break;

    case tok_binary:

        next_token();

        if (!isascii(token())){
            RETURN_ERRORP("Expected binary operator");
        }

        fnname = "binary";
        fnname += (char)token();
        kind = 2;

        next_token();

        // Read the precedence if present.
        if (token() == tok_number)
        {
            if (lexer.value() < 1 || lexer.value() > 100){
                RETURN_ERRORP("Invalid precedecnce: must be 1..100");
            }

            prec = (unsigned) lexer.value();

            next_token();
        }
        break;
//    default:
    }

    if (token() != '('){
        RETURN_ERRORP("Expected '(' in prototype");
    }

    // Read the list of argument names.
    AST::Prototype::Arguments argname;

    next_token();

    // extract arguments
    while (token() == tok_identifier || token() == ',')
    {
        if (token() == ',')
            next_token();
        else
        {
            AST::Prototype::add_param(argname, lexer.identifier());
            //argname.push_back(lexer.identifier());
            next_token();
        }
    }

    // close parens
    if (token() != ')'){
        RETURN_ERRORP("Expected ')' in prototype");
    }

    // success.
    next_token();  // eat ')'

    // TODO  ':' is optional =(
    if (token() == ':'){
        next_token(); // eat ':'

        if (token() == tok_newline) // eat '\n'
            next_token();
    }
    else{
        RETURN_ERRORP("Expected ':' in prototype");
    }

    // User define Operators
    if (kind && argname.size() != kind){
        RETURN_ERRORP("Invalid number of operands for operator");
    }

    return NEW_PROTO(AST::Prototype(fnname, argname, kind != 0, prec));
}

AST::Function* Parser::parse_definition(int idt)
{
    ADD_TRACE

    // Previous token is 'def'
    next_token();

    // get the prototype
    AST::Prototype* proto = parse_prototype(idt + 1);

    if (proto == 0){
        return 0;
    }

    // get the function body
    AST::Expression* e = parse_multiline_expression(idt + 1);

    if (e){
        return NEW_FUNCT(AST::Function(proto, e));
    }

    return 0;
}

AST::Prototype* Parser::parse_extern(int idt)
{
    ADD_TRACE
    next_token();

    return parse_prototype(idt + 1);
}

AST::Function* Parser::parse_top_level_expression(int idt)
{
    ADD_TRACE

    AST::Expression* e = parse_multiline_expression(idt + 1);
                       //parse_expression(idt + 1);

    if (e)
    {
        AST::Prototype* proto = NEW_PROTO(AST::Prototype("",
                                          AST::Prototype::Arguments()));

        return NEW_FUNCT(AST::Function(proto, e));
    }

    return 0;
}

AST::Expression* Parser::parse_for_expression(int idt)
{
    ADD_TRACE

    next_token();  // eat the for.

    if (int(token() != tok_identifier)){
        RETURN_ERROR("expected identifier after for");
    }

    std::string idname = identifier();
    next_token();  // eat identifier.

    //// Python For
    if (int(token() != tok_in)){
        std::string er_msg = "expected 'in' after " + identifier();
        RETURN_ERROR(er_msg.c_str());
    }

    next_token();  // eat 'in'.

    // range(0, 1):
    AST::Expression *start = parse_simple_expression(idt + 1);

    if (token() != ':'){
        RETURN_ERROR("expected ':'");
    }

    next_token();

    // Eat newlines
    while (token() == tok_newline)
        next_token();

    // Expect New Scope
    if (token() != tok_indent){
        RETURN_ERROR("expected indent block after for");
    }

    next_token();

    AST::Expression* body = parse_multiline_expression(idt + 1);

    return NEW_EXPR(AST::ForExpression(idname, start, body));
}


AST::Expression* Parser::parse_unary(int idt)
{
    ADD_TRACE

    // If the current token is not an operator, it must be a primary expr.
    if ( token() == '(' ||
         token() == ',' ||
        (token() == tok_identifier && _op.is_operator(identifier()) != 1) ||
         token() != tok_identifier)
    {
        return parse_primary(idt + 1);
    }

    // If this is a unary operator, read it.
    string opc;

    if (token() == tok_identifier)
        opc = identifier();
    else
        opc += token();

    next_token();

    // it not unary anymore
    AST::Expression *operand = parse_simple_expression(idt + 1);
                             //parse_primary(idt + 1);
                             //parse_unary(idt + 1);

    if (operand){
        return NEW_EXPR(AST::UnaryExpression(opc, operand));
    }

    return 0;
}

#include<iostream>

void Parser::parse()
{
    if (_print_out)
       _out << "===============================================================================\n"
               "                   Parser\n"
               "===============================================================================\n\n";

    if (_tk_idx == 0)
        next_token();

    int idt = 1;

    while(1)
    {
        ADD_TRACE

        switch (token())
        {
            case tok_eof:
                return;

            //case ';':
            case tok_desindent:
            case tok_indent:
            case tok_newline:
                next_token();
                break;  // ignore top-level semicolons.

            case tok_def:
                handle_definition(idt + 1);
                break;

            case tok_class:
                handle_class(idt + 1);
                break;

            case tok_extern:
                handle_extern(idt + 1);
                break;

            default:
                handle_top_level_expression(idt + 1);
                break;
        }

        SHOWTRACE
    }


}

void Parser::handle_definition(int idt)
{
    ADD_TRACE

    AST::Function* f = parse_definition(idt + 1);

    if(f)
    {
        PRINT_PARSED_EXPR(f)
        LLVM_CODE_GEN(f)
    }
    else{
        next_token();
    }

}

void Parser::handle_class(int idt)
{
    ADD_TRACE

    AST::Expression* f = parse_class(idt + 1);

    if (f)
    {
        PRINT_PARSED_EXPR(f)
        LLVM_CODE_GEN(f)
    }
    else
    {
        next_token();
    }
}

void Parser::handle_extern(int idt)
{
    ADD_TRACE

    AST::Prototype* f = parse_extern(idt + 1);

    if (f)
    {
        PRINT_PARSED_EXPR(f)
        LLVM_CODE_GEN(f)
    }
    else
    {
        next_token();
    }
}

void Parser::handle_top_level_expression(int idt)
{
    ADD_TRACE

    AST::Function* f = parse_top_level_expression(idt + 1);

    if (f)
    {
        PRINT_PARSED_EXPR(f)
        LLVM_CODE_GEN(f)

        #if LLVM_JIT && LLVM_CODEGEN
            LLVM_CODE_EXEC(__lf)
        #endif
    }
    else{
        next_token();
    }
}

AST::Expression* Parser::parse_variable_expression(int idt)
{
    ADD_TRACE

    next_token();  // eat the var.

    std::vector<std::pair<std::string, AST::Expression*> > varnames;

    // At least one variable name is required.
    if (token() != tok_identifier){
        RETURN_ERROR("expected identifier after var");
    }

    while(1)
    {
        std::string name = identifier();

        next_token();  // eat identifier.

        // Read the optional initializer.
        AST::Expression *init = 0;

        if (token() == '=')
        {
           next_token(); // eat the '='.

           init = parse_simple_expression(idt + 1);

           if (init == 0)
               return 0;
        }

        varnames.push_back(std::make_pair(name, init));

        // End of var list, exit loop.
        if (token() != ',')
            break;

        next_token(); // eat the ','.

        if (token() != tok_identifier){
           RETURN_ERROR("expected identifier list after var");
        }
    }

    // At this point, we have to have 'in'.
    if (token() != tok_in){
        RETURN_ERROR("expected 'in' keyword after 'var'");
    }

    next_token();  // eat 'in'.

    AST::Expression *body = parse_simple_expression(idt + 1);

    if (body == 0)
        return 0;

    return NEW_EXPR(AST::MutableVariableExpression(varnames, body));
}


AST::Expression* Parser::parse_multiline_expression(int idt)
{
    ADD_TRACE

    // eat indent token
    if (token() == tok_indent)
        next_token();

    // Save all expression
    AST::MultilineExpression* m = (AST::MultilineExpression*) NEW_EXPR(AST::MultilineExpression());

    while(1)
    {
        m->add(parse_simple_expression(idt + 1));

        while(token() == tok_newline)
            next_token();

        if (token() == tok_indent)
        {
            m->add(parse_multiline_expression(idt + 1)/*parse_expression()*/);
        }
        else if (token() == tok_desindent || token() == tok_eof)
        {
            // Eat Token
            next_token();
            RETURN_VECTOR(m)
        }
        // dont eat else tok we need it
        else if (token() == tok_else)
        {
            RETURN_VECTOR(m)
        }
    }
}

AST::Expression* Parser::parse_class(int idt)
{
    ADD_TRACE

    // eat class token
    next_token();

    if (token() != tok_identifier){
        RETURN_ERROR("expected class name after 'class'");
    }

    int class_indent = indent();

    AST::ClassExpression* c = (AST::ClassExpression*)
            NEW_EXPR(AST::ClassExpression(identifier()));


    // eat name identifier
    next_token();

    // Skip () no inheritance yet
    if (token() == '(')
    {
        next_token();

        while(token() != ')')
            next_token();

        // eat ')'
        next_token();
    }

    // eat ':'
    if (token() != ':'){
        RETURN_ERROR("expected class class_name():\\n'");
    }
    next_token();

    while(token() == tok_newline)
        next_token();

    // WTF, empty class
    if (token() != tok_indent)
    {
        next_token();
        return c;
    }

    next_token();

    // while(token() != tok_desindent) <= does not work
    //                  because only one desindent is sent evenif two indent level
    //                  are lost
    while(indent() > class_indent)
    {
        if (token() == tok_def)
        {
            AST::Function* f = parse_definition(idt + 1);
            c->add_method(f->prototype->name, f);
        }
        else if (token() == tok_identifier)
        {
            AST::Expression* e = parse_simple_expression(idt + 1);

            if (e->etype != AST::Type_BinaryExpression){
                RETURN_ERROR("expected attribute assignment");
            }

            AST::BinaryExpression* be = (AST::BinaryExpression*) e;

            c->add_attr(((AST::VariableExpression*)be->lhs)->name, be->lhs);

            while (token() == tok_newline)
                next_token();
        }
        else
        {
            break;
        }
    }

    return c;
}

}
