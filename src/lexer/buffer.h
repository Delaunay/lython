#pragma once

#include <cstdio>
#include <string>

#include "dtypes.h"
#include "logging/logging.h"

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
    virtual const String& file_name() = 0;

    AbstractBuffer(){}

    virtual ~AbstractBuffer();

    void init() { _next_char = getc(); }
    
    void consume(){
        if (_next_char == EOF)
            return;

        _col += 1;

        if (_next_char == '\n'){
            _line += 1;
            _col = 0;

            _indent = 0;
            _empty_line = true;
            _next_char = getc();
            return;
        }

        if (_next_char  == ' '){
            if (_empty_line)
                _indent += 1;
            _next_char = getc();
            return;
        }

        _empty_line = false;
        _next_char = getc();
    }

    char  peek()        {   return _next_char;   }
    int32 line()        {   return _line;        }
    int32 col()         {   return _col;         }
    int32 indent()      {	return _indent;      }
    bool  empty_line()  {	return _empty_line;  }

    virtual void reset(){
        _next_char = ' ';
        _line = 1;
        _col = 0;
        _indent = 0;
        _empty_line = true;
        init();
    }

private:
    char  _next_char{' '};
    int32 _line=1;
    int32 _col=0;
    int32 _indent{0};
    bool  _empty_line{true};
};

class FileError: public Exception {
public:
    template<typename ... Args>
    FileError(const char* fmt, const Args& ... args) :
        Exception(fmt, "FileError", args...)
    {}
};

String read_file(String const& name);

class FileBuffer : public AbstractBuffer
{
public:
    FileBuffer(String const& name);

    ~FileBuffer() override;

    char getc() override {
        return char(::getc(_file));
    }

    const String& file_name() override { return _file_name;  }

private:
    String _file_name;
    FILE*  _file{nullptr};
};

class StringBuffer: public AbstractBuffer
{
public:
    StringBuffer(String code, String const& file = "c++ string"):
        _code(std::move(code)), _file_name(file)
    {
        init();
    }

    char getc() override{
        if (_pos >= _code.size())
            return EOF;

        _pos += 1;
        return _code[_pos - 1];
    }

    ~StringBuffer() override;

    const String& file_name() override { return _file_name;  }

private:
    uint32  _pos{0};
    String _code;
    const String _file_name;

public:
    void reset() override {
        _pos = 0;
        AbstractBuffer::reset();
    }

    // helper for testing
    void read_all(){
        char c;
        do{ c = peek(); consume();    }
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
    {
        init();
    }

    char getc() override {  return char(std::getchar()); }

    const String& file_name() override { return _file_name;  }

    ~ConsoleBuffer() override;

private:
    const String _file_name;
};

}
