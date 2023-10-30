#if BUILD_POSIX
#include "os/os.h"

#include <unistd.h>

namespace lython {

String executable_path(std::size_t bsize) {
    String path(bsize, ' ');
    ssize_t size = readlink("/proc/self/exe", path.data(), bsize);
    path.resize(size);
    return path;
}

}
#endif