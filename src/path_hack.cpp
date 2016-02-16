#include "path_hack.h"

namespace LIBNAMESPACE{

    const std::string& source_path()
    {
        static const std::string s(__FILE__);
        return s;
    }
}
