    // Now there are multiple possibilities

////  this is matlab kindof for
//    if (token() != '=')
//        RETURN_ERROR("expected '=' after for");
//    next_token();  // eat '='.

    //// C For
//    AST::Expression* start = parse_simple_expression(idt + 1);

//    if (start == 0)
//        return 0;

//    if (token() != ',')
//        RETURN_ERROR("expected ',' after for start value");

//    next_token();

//    AST::Expression *end = parse_simple_expression(idt + 1);

//    if (end == 0)
//        return 0;

//    // The step value is optional.
//    AST::Expression* step = 0;

//    if (token() == ',')
//    {
//        next_token();
//        step = parse_simple_expression(idt + 1);

//        if (step == 0)
//            return 0;
//    }

//    if (token() != tok_in)
//        RETURN_ERROR("expected 'in' after for");

//    next_token();  // eat 'in'.

//    AST::Expression *body = parse_multiline_expression(idt + 1);
//                          //parse_expression(idt + 1);

//    if (body == 0)
//        return 0;

//    return NEW_EXPR(AST::ForExpression(idname, start, end, step, body));


//    std::cout << token() << "\t"
//              << tok_identifier << "\t"
//              << int(token() != tok_identifier) << "\n";

//    _out << token() << "\t"
//              << tok_identifier << "\t"
//              << int(token() != tok_identifier) << "\n";


// prototype
    /*
    if (token() != tok_identifier)
        return error<AST::Prototype>("Expected function name in prototype");

    std::string fnname = lexer.identifier();


    // get parens
    next_token();*/
    
    
    //        int c = ' ';

//        // if new line: indentation = 0
//        if (c == '\n')
//        {
//            indent = 0;

//            // count indentation
//            while(isspace(c))
//            {
//                indent++;
//                c = buffer.getc();
//            }

//            // temp scope
//            _tscope = indent % 4;

//            // open/close scope
//            if (_tscope != _scope)
//                return tok_scope;
//        }

//        // update scope
//        _scope = _tscope;

//int& scope()    {   return _scope;   }
//int _scope;
//int _tscope;

//int indent;

        /*
        str << "\tpos: [line: " << _buffer.current_line()
            << ", col: "    << _buffer.current_col()
            << ", cursor: " << _buffer.cursor() << "]), \n";//*/
/*
template<>
class TypedExpression<int> : public Expression
{
    public:
        TypedExpression(int v):
            value(v)
        {}

        void print(ostream& str, int i = 0)
        {
            str << value;
        }

    #if LLVM_CODEGEN
        virtual llvm::Value* code_gen(Generator& g)
        {
            return llvm::ConstantFP::get(llvm::getGlobalContext(),
                                         llvm::APInt());
        }
    #endif

        int value;
};*/
