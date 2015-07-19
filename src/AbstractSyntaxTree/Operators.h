#ifndef LYTHON_AST_OPERATORS_HEADER
#define LYTHON_AST_OPERATORS_HEADER

#include <unordered_map>

namespace lython
{

class Operators
{
public:
    struct Element
    {
        Element(uint8_t t = 0, bool b = true):
            pre(t), bin(b)
        {}

        uint8_t pre;
        bool    bin;
    };

    typedef std::string OpType;
    typedef std::unordered_map<OpType, Element> smap;

    Operators()
    {
        _op["="] = Element(2);  //  2;  // Assignment is Executed last
        _op["<"] = Element(10); // 10;
        _op["+"] = Element(20); // 20;
        _op["-"] = Element(20); // 20;
        _op["*"] = Element(40); // 40;
        _op["/"] = Element(40); // 40;
        _op["^"] = Element(50); // 40;
        _op["=="] = Element(2);
        _op["!="] = Element(2);
        _op["is"] = Element(2);
        _op["not"] = Element(2);
        _op["and"] = Element(2);

        // Unary Operator
        _op["++"] = Element(20, false);
        _op["--"] = Element(20, false);
        _op["return"] = Element(2, false);  // Executed last
    }

    uint8_t& operator[] (const OpType& o)
    {
        return _op[o].pre;
    }

    // return 1 if Unary
    //        2 if Binary else 0
    int is_operator(const OpType& op)
    {
        // count in log n
        if (_op.count(op) > 0)
            return _op[op].bin + 1;

        return 0;
    }

    smap& m() { return _op; }

protected:

    smap _op;

};

}

#endif
