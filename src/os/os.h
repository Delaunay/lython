#pragma once

#include "dtypes.h"

#if BUILD_WINDOWS
#include "os/windows.h"
#endif

#if BUILD_POSIX
#include "os/posix.h"
#endif

namespace lython {

String executable_path(std::size_t bsize = 512);

}