#pragma once

#include <cstdio>
#include <string>
#include "../Types.h"

/*
 *  Buffers are special reader that keep track of current line/col and indent level
 *  they only need getc() to be defined to work properly
 *
 *  StringBuffer is made to make debugging easy (might be useful for
 *  the eval option and macro gen)
 *
 *  FileBuffer is the usual reader
 */
namespace lython{
class AbstractBuffer
{
public:

    virtual char getc() = 0;
    virtual const std::string& file_name() = 0;

    char nextc(){

        c = getc();

        if (c == '\n'){
            _line += 1;
            _col = 0;
            _indent = 0;
            _empty_line = true;
            return c;
        }

        if (c == ' '){
            if (_empty_line)
                _indent += 1;

            _col += utf8_inc(c);
            return c;
        }

        _col += utf8_inc(c);
        _empty_line = false;
        return c;
    }

    // UTF8 Char position
    uint32 utf8_inc(char cc){
        if (cc < 128 || cc >= 192)
            return 1;
        return 0;
    }

    uint32 line()      {    return _line;   }
    uint32 col()       {    return _col;    }
    uint32 indent()    {    return _indent; }
    bool empty_line() { return _empty_line; }

    operator bool(){
        return c != EOF;
    }

private:
    char   c{'%'};
    uint32 _line{1};
    uint32 _col{0};
    uint32 _indent{0};
    bool _empty_line{true};
    bool _run_once{true};
};

class FileError{
public:
    const char* what() noexcept{
        return "FileError : File does not exist";
    }
};

class FileBuffer : public AbstractBuffer
{
public:
    FileBuffer(const std::string& name):
        _file_name(name)
    {
        _file = fopen(_file_name.c_str(), "r");

        if (!_file)
            throw FileError();
    }

    ~FileBuffer(){
        fclose(_file);
    }

    virtual char getc(){
        return ::getc(_file);
    }

    virtual const std::string& file_name(){ return _file_name;  }

private:
    std::string _file_name;
    FILE*       _file{nullptr};
};

class StringBuffer: public AbstractBuffer
{
public:
    StringBuffer(std::string& code):
        _code(code), _file_name("c++ string")
    {}

    virtual char getc(){

        if (_pos >= _code.size())
            return EOF;

        _pos += 1;
        return _code[_pos - 1];
    }

    virtual const std::string& file_name(){ return _file_name;  }

private:
    uint32  _pos{0};
    std::string& _code;
    const std::string _file_name;

public:
    // helper for testing
    void read_all(){
        char c;
        do{ c = nextc();    }
        while(c);
    }

    void load_code(const std::string& code){
        _code = code;
        _pos = 0;
    }
};

// Quick solution but not satisfactory
class ConsoleBuffer: public AbstractBuffer
{
public:
    ConsoleBuffer():
        _file_name("console")
    {}

    virtual char getc(){    return std::getchar(); }

    virtual const std::string& file_name(){ return _file_name;  }

private:
    const std::string _file_name;
};

}
