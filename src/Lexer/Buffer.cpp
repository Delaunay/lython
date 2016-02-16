#include "Buffer.h"

#include <cstdio>
#include <cassert>

#include "../config.h"

namespace LIBNAMESPACE{

AbstractBuffer::AbstractBuffer()
{}

AbstractBuffer::~AbstractBuffer()
{}

/*! ****************************************************************************
 *
 *          StandardInputBuffer
 *
 ******************************************************************************/

StandardInputBuffer::StandardInputBuffer():
    p(0), _string(string("")), _empty_line(false)
{}

StandardInputBuffer::~StandardInputBuffer()
{}

const char& StandardInputBuffer::nextc()
{
    c = getchar();
    _string += c;
    p++;

    _col++;

    if (c == '\n')
    {
        _line++;
        _col = 0;
    }

    return c;
}

const int StandardInputBuffer::cursor() const
{
    return p;
}

/*! ****************************************************************************
 *
 *          FileBuffer
 *
 ******************************************************************************/

#ifdef _MSC_VER
#   define _CRT_SECURE_NO_WARNINGS
#endif

FileBuffer::FileBuffer(const char* str):
    _cursor(-1), _line(1), _col(1), _doc_line(0),
    _indent(0), _empty_line(true)
{
    char c = ' ';

    FILE* file = fopen(str, "r");

    assert(file != 0 && "File does not exist");

    _file = string("");

    c = getc(file);

    while(c != EOF)
    {
        if (c == '\n')
            _doc_line++;

        _file += c;

        c = getc(file);
    }

    _file += EOF;

    fclose(file);
}

FileBuffer::~FileBuffer()
{}

const char& FileBuffer::operator[] (int idx) const
{
    return _file[idx];
}

const char& FileBuffer::nextc()
{
    static bool indenting = true;

    if (_file[_cursor + 1] == ' ' && indenting)
        _indent++;

    // Handles tabs as TAB_SIZE spaces
    else if (_file[_cursor + 1] == '\t' && indenting)
        _indent += TAB_SIZE;

    else
        indenting = false;

    if (_file[_cursor + 1] == '\n')
    {
        _line++;
        _col = 1;
        _indent = 0;
        indenting = true;
        _empty_line = true;
    }
    else
    {
        _col++;

//        if (_file[_cursor + 1] != ' ')
        _empty_line = false;
    }

    if (_cursor == _file.size() - 1)
       return _file[_cursor];

    _cursor++;
    return _file[_cursor];
}

const char& FileBuffer::prevc()
{
    if (_file[_cursor - 1] = '\n')
    {
        _line--;
        _col = 0;
    }
    else
        _col--;

    _cursor--;
    return _file[_cursor];
}

const int FileBuffer::cursor() const
{
    return _cursor;
}

const size_t FileBuffer::size() const
{
    return _file.size();
}

void FileBuffer::restart()
{
    _line = 1;
    _col = 1;
    _indent = 0;
    _cursor = -1;
}

void FileBuffer::set_cursor(int x)
{
    _cursor = x;
}

const int&    FileBuffer::current_line () const {   return _line;      }
const int&    FileBuffer::current_col  () const {   return _col;       }
const int&    FileBuffer::document_line() const {   return _doc_line;  }
const string& FileBuffer::file         () const {   return _file;      }

void FileBuffer::print(std::ostream& str)
{
    str << "===============================================================================\n"
           "                   Buffer\n"
           "===============================================================================\n\n";

    str << "Size   : " << _file.size() << "\n"
        << "Cursor : " << _cursor      << "\n"
        // << "Current: " << int((*this)[size() - 1]) << "\n"
        << "File   :\n"
        << "------BEG------\n" MGREEN
        << _file            << MRESET
        << "\n------END------\n";
}


void StandardInputBuffer::print(std::ostream& str)
{
    str << "===============================================================================\n"
           "                   Buffer\n"
           "===============================================================================\n\n";

    str << "Size   : " << _string.size() << "\n"
        << "Cursor : " << _cursor      << "\n"
        // << "Current: " << int((*this)[size() - 1]) << "\n"
        << "File   :\n"
        << "------BEG------\n" MGREEN
        << _string           << MRESET
        << "\n------END------\n";
}

}
