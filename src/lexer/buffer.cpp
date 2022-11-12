#include "lexer/buffer.h"

#define __STDC_WANT_LIB_EXT1__   1
#define __STDC_WANT_SECURE_LIB__ 1

#include <cstdio>

#ifndef __linux__
#    define __STDC_LIB_EXT1__ 1
#endif

namespace lython {

FILE* internal_fopen(String filename) {
    FILE* file;

#if (defined __STDC_LIB_EXT1__) && __STDC_LIB_EXT1__
    auto err = fopen_s(&file, filename.c_str(), "r");
    if (err != 0) {
        throw FileError("{}: File `{}` does not exist", filename);
    }
#else
    file = fopen(filename.c_str(), "r");

    if (!file) {
        throw FileError("{}: File `{}` does not exist", filename);
    }
#endif

    return file;
}

AbstractBuffer::~AbstractBuffer() {}

FileBuffer::FileBuffer(String const& name): _file_name(name) {

    _file = internal_fopen(_file_name);

    init();
}

FileBuffer::~FileBuffer() { fclose(_file); }

void FileBuffer::reset() {
    fseek(_file, 0, SEEK_SET);
    AbstractBuffer::reset();
}

String FileBuffer::getline(int start_line, int end_line) {
    fpos_t pos;
    fgetpos(_file, &pos);
    //--

    String result;
    result.reserve(128);
    fseek(_file, start_line, SEEK_SET);

    char c = fgetc(_file);

    while (c != '\n') {
        result.push_back(c);
        c = fgetc(_file);
    }

    // --
    fsetpos(_file, &pos);
    return result;
}

StringBuffer::~StringBuffer() {}

ConsoleBuffer::~ConsoleBuffer() {}

String read_file(String const& name) {
    FILE* file = internal_fopen(name);

    if (!file)
        throw FileError("{}: File `{}` does not exist", name);

    Array<String> data;

    size_t const buffer_size = 8192;
    size_t       read        = 0;
    size_t       total       = 0;

    do {
        data.emplace_back(buffer_size, ' ');
        auto& buffer = *(data.end() - 1);

        read = fread(&buffer[0], 1, buffer_size, file);

        total += read;
    } while (read == buffer_size);

    String aggregated(total, ' ');

    ptrdiff_t start = 0;
    for (auto& segment: data) {
        std::copy(std::begin(segment), std::end(segment), std::begin(aggregated) + start);

        start += segment.size();
    }

    debug("read {}", aggregated);
    return aggregated;
}

}  // namespace lython
