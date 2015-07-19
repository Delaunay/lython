#ifndef LYTHON_LEXER_BUFFER_HEADER
#define LYTHON_LEXER_BUFFER_HEADER

#include <string>
#include <iostream>

#ifdef _MSC_VER
#   define _CRT_SECURE_NO_WARNINGS
#endif

using namespace std;

namespace lython{

class AbstractBuffer
{
    public:
         AbstractBuffer();
        ~AbstractBuffer();

        virtual const char& nextc() = 0;
        virtual const int cursor() const = 0;    // postion

        virtual void restart()
        {}

        virtual void set_cursor(int x)
        {

        }

        virtual const int&    current_line () const = 0;
        virtual const int&    current_col  () const = 0;
        virtual const string& file         () const = 0;
        virtual const int&    indent       () const = 0;
        virtual const bool& is_line_empty() const  = 0;

        // virtual const bool end() = 0;
};

class StandardInputBuffer : public AbstractBuffer
{
    public:
        // TODO
         StandardInputBuffer();
        ~StandardInputBuffer();

        const char& nextc();
        const int cursor() const;

        const int&    current_line () const {   return _line;     }
        const int&    current_col  () const {   return _col;      }
        const string& file         () const {   return _string;     }
        const int&    indent       () const {   return _indent;     }
        const bool&   is_line_empty() const {   return _empty_line;}

        void print(std::ostream& str);

    protected:

        string _string;

        int _cursor;
        int _line;
        int _col;
        int _indent;
        bool _empty_line;

        char c;
        int p;
};

class FileBuffer : public AbstractBuffer
{
    public:
         FileBuffer(const char* str);
        ~FileBuffer();

        const char& operator[] (int idx) const;

        const char& nextc();
        const char& prevc();

        const int cursor() const;
        const size_t size() const;

        const int&    current_line () const;
        const int&    current_col  () const;
        const int&    document_line() const;
        const string& file         () const;

        void restart();
        void set_cursor(int x);

        void print(std::ostream& str);

        const int& indent() const   {   return _indent; }
        const bool& is_line_empty() const { return _empty_line;}

    protected:

        int _indent;
        int _doc_line;
        bool _empty_line;

        string _file;

        int _cursor;
        int _line;
        int _col;
};

}

#endif
