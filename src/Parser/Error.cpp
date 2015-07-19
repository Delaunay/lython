#include "Error.h"

namespace LIBNAMESPACE {

std::string clean_numbers(unsigned int k)
{
    std::string s;

    if (k < 10)
        s += "   "; // "000"
    else if (k < 100)
        s += "  ";
    else if (k < 1000)
        s += " ";

    return s + std::to_string(k);
}

void dump_format_L(Traceback* tb,  std::ostream& out, std::string color)
{
    int old_indent = 0,
    current_indent = 0;

    if (tb != nullptr)
        for(int i = 0; i < tb->traceback.size(); ++i)
        {
            if (tb->intern(i))
            {
                out <<      "    "         << clean_numbers(i)           <<") "
                          "File: " << color << clean_path(tb->file(i))    << MRESET
                         " Line: " << color << clean_numbers(tb->line(i)) << MRESET << " ";

                current_indent = tb->indent(i);

                if (current_indent > old_indent)
                {
                    for(int j = 0; j < old_indent; j++)
                        out << " ";

                    out << "L";

                    for (int j = old_indent + 1; j < current_indent; j++)
                        out << "_";
                }
                else
                {
                    for (int j = 0; j < current_indent; j++)
                        out << "_";
                }

                old_indent = tb->indent(i);

                out << " Function: " << color << tb->function(i)            << MRESET "\n";
            }
            else
                for(int i = 0; i < tb->traceback.size(); ++i)
                    out << "[Ln: " MBLUE << clean_numbers(tb->line(i)) << MRESET
                         ", Col: " MBLUE << clean_numbers(tb->col(i))  << MRESET "]\n";
        }
}
}
