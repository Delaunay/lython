#ifndef LYTHON_AST_OPERATORS_HEADER
#define LYTHON_AST_OPERATORS_HEADER

#include <unordered_map>

class Operators
{
public:
    typedef char OpType;

    Operators()
    {
        _op['<'] = 10;
        _op['+'] = 20;
        _op['-'] = 20;
        _op['*'] = 40;
    }

    const int& operator[] (const OpType& o)
    {
        return _op[o];
    }

protected:

    std::unordered_map<OpType, int> _op;

};

#endif
