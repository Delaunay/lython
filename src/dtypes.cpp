#include "dtypes.h"

// Explicit template instantiations for faster compilation
namespace lython {

template class AllocatorCPU<char>;

template class std::basic_string<char, std::char_traits<char>, AllocatorCPU<char>>;

}  // namespace lython
