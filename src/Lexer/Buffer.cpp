#include "Buffer.h"

#include <cstdio>

namespace lython{

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
    p(0), _string(string(""))
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

FileBuffer::FileBuffer(const char* str):
    _cursor(-1), _line(1), _col(0), _doc_line(0),
    _indent(0)
{
    char c = ' ';

    FILE* file = fopen(str, "r");

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
    if (_file[_cursor + 1] == ' ')
        _indent++;

    if (_file[_cursor + 1] == '\n')
    {
        _line++;
        _col = 0;
        _indent = 0;
    }
    else
        _col++;

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

const int FileBuffer::size() const
{
    return _file.size();
}

void FileBuffer::restart()
{
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

}
