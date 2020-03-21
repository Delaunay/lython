#include "lexer/buffer.h"


namespace lython{
AbstractBuffer::~AbstractBuffer(){}

FileBuffer::FileBuffer(String const& name):
    _file_name(name)
{
    _file = fopen(_file_name.c_str(), "r");

    if (!_file)
        throw FileError("{}: File `{}` does not exist", _file_name);

    init();
}

FileBuffer::~FileBuffer(){
    fclose(_file);
}

StringBuffer::~StringBuffer(){}

ConsoleBuffer::~ConsoleBuffer(){}

String read_file(String const& name){
    FILE* file = fopen(name.c_str(), "r");

    if (!file)
        throw FileError("{}: File `{}` does not exist", name);

    Array<String> data;

    size_t const buffer_size = 8192;
    size_t read  = 0;
    size_t total = 0;

    do {
        data.emplace_back(buffer_size, ' ');
        auto& buffer = *(data.end() - 1);

        read = fread(&buffer[0], 1, buffer_size, file);

        total += read;
    } while (read == buffer_size);

    String aggregated(total, ' ');

    ptrdiff_t start = 0;
    for(auto& segment: data){
        std::copy(std::begin(segment), std::end(segment),
                  std::begin(aggregated) + start);

        start += segment.size();
    }

    debug("read {}", aggregated);
    return aggregated;
}

}
