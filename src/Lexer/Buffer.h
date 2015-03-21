#ifndef LYTHON_LEXER_BUFFER_HEADER
#define LYTHON_LEXER_BUFFER_HEADER

#include <string>
#include <iostream>

#include "../colors.h"

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
        const int& indent() const
        {}

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

        void print(std::ostream& str)
        {
            str << "===============================================================================\n"
                   "                   Buffer\n"
                   "===============================================================================\n\n";

            str << "Size   : " << _string.size() << "\n"
                << "Cursor : " << _cursor      << "\n"
                // << "Current: " << int((*this)[size() - 1]) << "\n"
                << "File   :\n"
                << "------BEG------\n" GREEN
                << _string           << RESET
                << "\n------END------\n";
        }

    protected:

        string _string;

        int _cursor;
        int _line;
        int _col;

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
        const int size() const;

        const int&    current_line () const;
        const int&    current_col  () const;
        const int&    document_line() const;
        const string& file         () const;

        void restart();
        void set_cursor(int x);

        void print(std::ostream& str)
        {
            str << "===============================================================================\n"
                   "                   Buffer\n"
                   "===============================================================================\n\n";

            str << "Size   : " << _file.size() << "\n"
                << "Cursor : " << _cursor      << "\n"
                // << "Current: " << int((*this)[size() - 1]) << "\n"
                << "File   :\n"
                << "------BEG------\n" GREEN
                << _file            << RESET
                << "\n------END------\n";
        }

        const int& indent() const   {   return _indent; }

    protected:

        int _indent;
        int _doc_line;

        string _file;

        int _cursor;
        int _line;
        int _col;
};

}

#endif
