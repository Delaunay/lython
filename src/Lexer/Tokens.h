#pragma once

#include <ostream>
#include <string>
#include <vector>
#include <algorithm>

#include "Prelexer.h"
#include "../fmt.h"
#include "../Types.h"

#define TOK_NAME_SIZE 10

/*
 *  Sexp or Syntatic Expression are Typer's Tokens.
 *      Node and Epsilon are not used during the parsing.
 */

namespace lython{

enum SexpType{
    Sexp_Sexp = -1,
    Sexp_Epsilon,
    Sexp_Block,
    Sexp_Symbol,
    Sexp_String,
    Sexp_Integer,
    Sexp_Float,
    Sexp_Node,
};


class SyntaticExpression
{
public:
    virtual ~SyntaticExpression() {}

    SyntaticExpression(uint32 line, uint32 col):
        _line(line), _col(col)
    {}

    virtual SexpType type() {   return Sexp_Sexp;  }

    virtual std::ostream& print(std::ostream& out) {    return out; }
    virtual std::ostream& debug_print(std::ostream& out) {  return out; }

    uint32 line()   {   return _line;   }
    uint32 col()    {   return _col;    }

private:
    uint32 _line;
    uint32 _col;
};

class Epsilon: public SyntaticExpression
{
public:
    Epsilon():
        SyntaticExpression(0, 0)
    {}

    std::ostream& print(std::ostream& out)       {  out << "eps";   return out; }
    std::ostream& debug_print(std::ostream& out) {  out << "eps";   return out; }

    SexpType type() {   return Sexp_Epsilon;  }
};

typedef std::shared_ptr<SyntaticExpression> Sexp;

class Block: public SyntaticExpression
{
public:
    typedef PreToken::Block PreTokenBlock;

    Block(PreToken& block):
        SyntaticExpression(block.line(), block.col()),
        _block(block.as_block())
    {}

    PreTokenBlock& block(){
        return _block;
    }

    SexpType type() {   return Sexp_Block;  }

    std::ostream& debug_print(std::ostream& out)       {
        out << MGREEN "[l:" << to_string(line(), 4) << " c:" << to_string(col(), 4) << "] " MRESET
            << align_right("Block", TOK_NAME_SIZE) << " => " << "len:" << block().size();
        return out;
    }

    std::ostream& print(std::ostream& out) {
        out << "block";
        return out;
    }

private:
    PreTokenBlock _block;
};

const std::string& get_str(Token& t){
    if (t->type() == Sexp_Symbol)
        return ((Symbol*) (t.get()))->name();
}


class Symbol: public SyntaticExpression
{
public:
    Symbol(const std::string& name, uint32 l=0, uint32 c=0):
        SyntaticExpression(l, c), _name(name)
    {}

    Symbol(char x, uint32 l=0, uint32 c=0):
        SyntaticExpression(l, c), _name(1, x)
    {}

    SexpType type() {   return Sexp_Symbol;  }
    const std::string& name() {   return _name;   }

    std::ostream& debug_print(std::ostream& out)       {
        out << MGREEN "[l:" << to_string(line(), 4) << " c:" << to_string(col(), 4) << "] " MRESET
            << align_right("Symbol", TOK_NAME_SIZE) << " => " << _name;
        return out;
    }

    std::ostream& print(std::ostream& out) {  out << "[sym]" << _name;   return out; }
private:
    std::string _name;
};

class String: public SyntaticExpression
{
public:
    String(PreToken& tok):
        SyntaticExpression(tok.line(), tok.col()), _str(tok.as_string())
    {}

    std::string& string() { return _str;    }

    SexpType type() {   return Sexp_String;  }

    std::ostream& debug_print(std::ostream& out)       {
        out << MGREEN "[l:" << to_string(line(), 4) << " c:" << to_string(col(), 4) << "] " MRESET
            << align_right("String", TOK_NAME_SIZE) << " => " << _str;
        return out;
    }

    std::ostream& print(std::ostream& out) {  out << '"' << _str << '"';   return out; }

private:
    std::string _str;
};

class Integer: public SyntaticExpression
{
public:
    Integer(int32 val, uint32 l=0, uint32 c=0):
        SyntaticExpression(l, c), _int32(val)
    {}

    int32 value()   {   return _int32;  }

    SexpType type() {   return Sexp_Integer;  }

    std::ostream& print(std::ostream& out)       {  out << _int32;   return out; }
    std::ostream& debug_print(std::ostream& out)       {
        out << MGREEN "[l:" << to_string(line(), 4) << " c:" << to_string(col(), 4) << "] " MRESET
            << align_right("Integer", TOK_NAME_SIZE) << " => " << _int32;
        return out;
    }

private:
    int32 _int32;
};

class Float: public SyntaticExpression
{
public:
    Float(float64 v, uint32 l=0, uint32 c=0):
        SyntaticExpression(l, c), _float64(v)
    {}

    float64 value() {   return _float64;    }

    SexpType type() {   return Sexp_Float;  }

    std::ostream& print(std::ostream& out)       {  out << _float64;   return out; }
    std::ostream& debug_print(std::ostream& out)       {
        out << MGREEN "[l:" << to_string(line(), 4) << " c:" << to_string(col(), 4) << "] " MRESET
            << align_right("Float", TOK_NAME_SIZE) << " => " << _float64;
        return out;
    }
private:
    float64 _float64;
};

class Node: public SyntaticExpression
{
public:
    Node(uint32 l, uint32 c):
        SyntaticExpression(l, c)
    {}

    std::vector<Sexp> children() {  return _children;   }
    Sexp parent() { return _parent; }

    SexpType type() {   return Sexp_Node;  }

    std::ostream& print(std::ostream& out)       {  out << "Node";   return out; }
    std::ostream& debug_print(std::ostream& out) {  out << "Node";   return out; }

private:

    std::vector<Sexp> _children;
    Sexp    _parent;
};

typedef Sexp Token;
}
