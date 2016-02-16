#include "Error.h"

namespace LIBNAMESPACE {


void dump_format_L(Traceback* tb,  std::ostream& out)
{
    if (tb != nullptr)
        for(int i = 0; i < tb->traceback.size(); ++i)
        {
            if (tb->intern(i))
                tb->traceback[i].print(out, i);
            else
                for(int i = 0; i < tb->traceback.size(); ++i)
                    out << "[Ln: " << clean_numbers(tb->line(i)) << MRESET
                         ", Col: " << clean_numbers(tb->col(i))  << MRESET "]\n";

        }
}
}
