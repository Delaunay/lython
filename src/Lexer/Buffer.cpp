#include "Buffer.h"

namespace lython{
AbstractBuffer::~AbstractBuffer(){}

FileBuffer::~FileBuffer(){
    fclose(_file);
}

StringBuffer::~StringBuffer(){}

ConsoleBuffer::~ConsoleBuffer(){}

}
