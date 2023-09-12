#if BUILD_WINDOWS
#include "os/os.h"

#include <libloaderapi.h>

namespace lython {

String executable_path(std::size_t bsize) {
    String path(bsize, ' ');
    DWORD size = GetModuleFileNameA(NULL, path.data(), bsize);
    path.resize(size);
    return path;
}

}
#endif