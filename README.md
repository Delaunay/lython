# 'Python' Parser (Heavily Modified Kaleidoscope)

Lython is based upon the Kaleidoscope tutorial. This is why you may find code
that are not relevant anymore (especially from the code generation part of lython
which I have not yet modified)


## Implementation Specific Modification
* Language Implementation broke down into modules
    * Lexer / Buffer
        * Lexer does not (always) eat '\n' 
		* new tok_indent and tok_desindent
        * Buffer from file
        * Buffer from standard inputs
        * debug print: print tokenized text 
    * Parser
        * Object Manager: Keeps tracks of dynamic alloc (for good conscience)
		* Parser Traceback make it easier to debug the parser
		* multiline parsing
		* [] call
    * Abstract Syntax Tree
        * Expression can be printed for debuging purposes
    * Generator
        * class holding LLVM class for generation/Execution
		* Optional
    
* New Error Reporting (still basic)


    [Ln: 4, Col: 1]      /!\ Error     Expected ':' in prototype 
        0) File: Parser\Parser.cpp Line:  582 Function: parse
        1) File: Parser\Parser.cpp Line:  682 Function:  handle_top_level_expression
        2) File: Parser\Parser.cpp Line:  467 Function:   parse_top_level_expression
        3) File: Parser\Parser.cpp Line:  191 Function:    parse_expression
        4) File: Parser\Parser.cpp Line:  548 Function:     parse_unary
        5) File: Parser\Parser.cpp Line:  548 Function:     parse_unary
    
* no more global variables 
* Dynamic alloc are freed, (LLVM allocs are not yet freed)
* new code style (can be good or bad ^_^)


Design
=======

## Tokenize source code
  
code -*- my_source -*-
    
    def myfunction(x, y, z):
        (z + y) * x
        
token -*- my tokens -*-
    
    tokens = [
        0001: (tok: [-2, '�'], def: 'def')
        0002: (tok: [-4, '�'], ide: 'myfunction')
        0003: (tok: [40, '('], chr: '(')
        0004: (tok: [-4, '�'], ide: 'x')
        0005: (tok: [44, ','], chr: ',')
        0006: (tok: [-4, '�'], ide: 'y')
        0007: (tok: [44, ','], chr: ',')
        0008: (tok: [-4, '�'], ide: 'z')
        0009: (tok: [41, ')'], chr: ')')
        0010: (tok: [58, ':'], chr: ':')
        0011: (tok: [40, '('], chr: '(')
        0012: (tok: [-4, '�'], ide: 'x')
        0013: (tok: [43, '+'], chr: '+')
        0014: (tok: [-4, '�'], ide: 'y')
        0015: (tok: [41, ')'], chr: ')')
        0016: (tok: [42, '*'], chr: '*')
        0017: (tok: [-4, '�'], ide: 'z')
        0018: (tok: [59, ';'], chr: ';')
        0019: (tok: [-1, '�'], chr: '�')
    ] (size: 19)

## Syntax Tree

Implements memory representation of expression sources,
    
code -*- my_source -*-

    def myfunction(x, y, z):
        (z + y) * x
        
C++ -*- parsing -*-

    AST::Prototype::Arguments args;
    args.push_back("x");
    args.push_back("y");
    args.push_back("z");

    AST::Prototype pt("myfunction", args, false, 0);

    AST::VariableExpression x("x");
    AST::VariableExpression y("y");
    AST::VariableExpression z("z");

    AST::BinaryExpression op1('+', &z, &y);
    AST::BinaryExpression op2('*', &op1, &x);

    AST::Expression* body = &op2;

    AST::Function mf(&pt, body);

    mf.print(std::cout);

Expression Pretty Print -*- std::cout -*-

    fn myfunction (x, y, z)
        '*'('+'(z, y), x)
        
    implements method to generate IR code from the expression.
    
IR Print -*- std::cout -*-

    define double @myfunction(double %x, double %y, double %z) {
    entry:
      %addtmp = fadd double %z, %y
      %multmp = fmul double %addtmp, %x
      ret double %multmp
    }


## Parser
    
Takes token and returns memory representation of the source as Expression
   