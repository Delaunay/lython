#pragma once

#include <string>

#include "dependencies/coz_wrap.h"
#include "dependencies/fmt.h"

#include "dtypes.h"
#include "logging/exceptions.h"

/*
 *  Buffers are special reader that keep track of current line/col and indent level
 *  they only need getc() to be defined to work properly
 *
 *  StringBuffer is made to make debugging easy (might be useful for
 *  the eval option and macro gen)
 *
 *  FileBuffer is the usual reader
 */
namespace lython {
class AbstractBuffer {
    public:
    virtual char          getc()      = 0;
    virtual const String& file_name() = 0;

    AbstractBuffer() {}

    virtual ~AbstractBuffer();

    void init() { _next_char = getc(); }

    // TODO: add a hash digest compute
    // so we can hash files with little overhead
    void consume() {
        if (_next_char == EOF)
            return;

        _col += 1;

        if (_next_char == '\n') {
            _line += 1;
            _col = 0;

            _indent     = 0;
            _empty_line = true;
            _next_char  = getc();
            return;
        }

        if (_next_char == ' ') {
            if (_empty_line)
                _indent += 1;
            _next_char = getc();
            return;
        }

        _empty_line = false;
        _next_char  = getc();
    }

    // Used to fetch a given line for error reporting
    virtual String getline(int start_line, int end_line = -1) { return ""; }

    char  peek() { return _next_char; }
    int32 line() { return _line; }
    int32 col() { return _col; }
    int32 indent() { return _indent; }
    bool  empty_line() { return _empty_line; }

    virtual void reset() {
        _next_char  = ' ';
        _line       = 1;
        _col        = 0;
        _indent     = 0;
        _empty_line = true;
        init();
    }

    private:
    char  _next_char{' '};
    int32 _line = 1;
    int32 _col  = 0;
    int32 _indent{0};
    bool  _empty_line{true};
};

class FileError: public Exception {
    public:
    template <typename... Args>
    FileError(FmtStr fmt, const Args&... args): Exception(fmt, "FileError", args...) {}
};

String read_file(String const& name);

class FileBuffer: public AbstractBuffer {
    public:
    FileBuffer(String const& name);

    ~FileBuffer() override;

    char getc() override {
        COZ_BEGIN("T::FileBuffer::getc");

        char c = char(::getc(_file));

        COZ_PROGRESS_NAMED("FileBuffer::getc");
        COZ_END("T::FileBuffer::getc");
        return c;
    }

    const String& file_name() override { return _file_name; }

    void reset() override;

    String getline(int start_line, int end_line = -1) override;

    private:
    String _file_name;
    FILE*  _file{nullptr};
};

class StringBuffer: public AbstractBuffer {
    public:
    StringBuffer(String code, String const& file = "c++ string"):
        _code(std::move(code)), _file_name(file) {
        init();
    }

    char getc() override {
        if (_pos >= _code.size())
            return EOF;

        _pos += 1;
        return _code[_pos - 1];
    }

    ~StringBuffer() override;

    const String& file_name() override { return _file_name; }

    private:
    uint32       _pos{0};
    String       _code;
    const String _file_name;

    public:
    void reset() override {
        _pos = 0;
        AbstractBuffer::reset();
    }

    String getline(int start_line, int end_line = -1) override {
        uint32 old_pos = _pos;
        //--

        String result;
        result.reserve(128);
        _pos = start_line;

        char c = getc();

        while (c != '\n') {
            result.push_back(c);
            c = getc();
        }

        // --
        _pos = old_pos;
        return result;
    }

    // helper for testing
    void read_all() {
        char c;
        do {
            c = peek();
            consume();
        } while (c);
    }

    void load_code(const std::string& code) {
        _code = code;
        _pos  = 0;
    }
};

// Quick solution but not satisfactory
class ConsoleBuffer: public AbstractBuffer {
    public:
    ConsoleBuffer(): _file_name("console") { init(); }

    char getc() override { return char(std::getchar()); }

    const String& file_name() override { return _file_name; }

    ~ConsoleBuffer() override;

    private:
    const String _file_name;
};

}  // namespace lython
