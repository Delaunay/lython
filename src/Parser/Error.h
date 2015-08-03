#ifndef KIWI_PARSER_ERROR_HEADER
#define KIWI_PARSER_ERROR_HEADER

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include "../config.h"
#include "../path_hack.h"

namespace LIBNAMESPACE {

    // remove everything before src/
    #define clean_path(path)    std::string(path).erase(0, std::string(SOURCE_PATH).size() - 13)

    // reserve sufficient space for the number 9999
    std::string clean_numbers(unsigned int k);

    // Shortcut
    #define add_trace(tb, idt) tb.add(__FILE__, __FUNCTION__, __LINE__, idt/*, true*/)

    class Traceback
    {
    public:

        struct Element
        {
            Element(std::string i, std::string j, unsigned int k, unsigned int c = 0,
                    bool intern = true, int idt=0):
                file(i), function(j), line(k), col(c), intern(intern),
                indent(idt)
            {}

            std::string file;
            std::string function;
            unsigned int line;

            unsigned int col;
            std::string  time;
            bool         intern;
            int          indent;

        };

        Traceback():
            out(std::cout)
        {}

        void add(std::string i, std::string j, unsigned int k, int idt=0, bool push_to_out=true)
        {
            traceback.push_back(Element(i, j, k, 0, true, idt));

            if (push_to_out && idt < 100)
            {
                out <<      "    "         << clean_numbers(traceback.size()) << ") "
                          "File: " MYELLOW << clean_path(i)    << MRESET
                         " Line: " MYELLOW << clean_numbers(k) << MRESET;

                for (int m = 0, n = idt; m < n; m++)
                    out << " ";

                out << " Function: " MYELLOW << j          << MRESET "\n";
            }
        }

        void add_extern(std::string file, std::string function, unsigned int line, unsigned col)
        {
            traceback.push_back(Element(file, function, line, col, false));
        }

        void erase()
        {
            traceback.erase(traceback.begin(), traceback.end());
        }

        const std::string&  file(int k)      {   return traceback[k].file;}
        const std::string&  function(int k)  {   return traceback[k].function;}
        const unsigned int& line(int k)      {   return traceback[k].line;}
        const unsigned int& col(int k)       {   return traceback[k].col;}
        const std::string&  time(int k)      {   return traceback[k].time;  }
        const bool          intern(int k)    {   return traceback[k].intern;    }
        const int&          indent(int k)    {   return traceback[k].indent;    }

        void flush()    {   out.flush(); }

        std::vector<Element> traceback;
        std::ostream& out;
    };


    /*!
     *  Error dump, can be dump in any ostream
     */

    template<typename T>
    T* error(const char* str, const int& line, const int& col, Traceback* tb = 0,  std::ostream& out = std::cout)
    {
        out << MGREEN "\n[Ln: " << clean_numbers(line) <<
                      ", Col: " << clean_numbers(col)  << "] " MRESET "    "
                   // " /!\\ Warning "
                 MRED " /!\\ Error   " MRESET "     "<< str <<"\n";

        if (tb != nullptr)
            for(int i = 0; i < tb->traceback.size(); ++i)
            {
                if (tb->intern(i))
                {
                    out <<      "    "         << clean_numbers(i)           <<") "
                              "File: " MYELLOW << clean_path(tb->file(i))    << MRESET
                             " Line: " MYELLOW << clean_numbers(tb->line(i)) << MRESET;

                    for (int j = 0, n = tb->indent(i); j < n; j++)
                        out << " ";

                    out << " Function: " MYELLOW << tb->function(i)            << MRESET "\n";
                }

                else
                    for(int i = 0; i < tb->traceback.size(); ++i)
                        out <<    "    "      << clean_numbers(i)           <<") "
                                "File: " MRED << clean_path(tb->file(i))    << MRESET
                                " [Ln: " MRED << clean_numbers(tb->line(i)) << MRESET
                               ", Col: " MRED << clean_numbers(tb->col(i))  << MRESET "]"
                           " Function: " MRED << tb->function(i)            << MRESET "\n";
            }

        out << "\n";

        return 0;
    }

    template<typename T>
    T* warning(const char* str, const int& line, const int& col, Traceback* tb = 0,  std::ostream& out = std::cout)
    {
        out << MGREEN "\n[Ln: " << clean_numbers(line) <<
                      ", Col: " << clean_numbers(col)  << "] " MRESET "    "
                   // " /!\\ Warning "
              MYELLOW " /!\\ Warning " MRESET "     "<< str <<"\n";

        if (tb != nullptr)
            for(int i = 0; i < tb->traceback.size(); ++i)
            {
                if (tb->intern(i))
                {
                    out <<      "    "         << clean_numbers(i)           <<") "
                              "File: " MYELLOW << clean_path(tb->file(i))    << MRESET
                             " Line: " MYELLOW << clean_numbers(tb->line(i)) << MRESET;

                    for (int j = 0, n = tb->indent(i); j < n; j++)
                        out << " ";

                    out << " Function: " MYELLOW << tb->function(i)            << MRESET "\n";
                }

                else
                    for(int i = 0; i < tb->traceback.size(); ++i)
                        out <<    "    "         << clean_numbers(i)           <<") "
                                "File: " MYELLOW << clean_path(tb->file(i))    << MRESET
                                " [Ln: " MYELLOW << clean_numbers(tb->line(i)) << MRESET
                               ", Col: " MYELLOW << clean_numbers(tb->col(i))  << MRESET "]"
                           " Function: " MYELLOW << tb->function(i)            << MRESET "\n";
            }

        out << "\n";

        return 0;
    }

    void dump_format_L(Traceback* tb = 0,  std::ostream& out = std::cout, std::string color = MYELLOW);

    template<typename T>
    T* info(const char* str, const int& line, const int& col, Traceback* tb = 0,  std::ostream& out = std::cout)
    {
        out << MGREEN "\n[Ln: " << clean_numbers(line) <<
                      ", Col: " << clean_numbers(col)  << "] " MRESET "    "
                   // " /!\\ Warning "
                MBLUE " /!\\ Info    " MRESET "     "<< str <<"\n";
        int old_indent = 0,
            current_indent = 0;

        if (tb != nullptr)
            for(int i = 0; i < tb->traceback.size(); ++i)
            {
                if (tb->intern(i))
                {
                    out <<      "    "         << clean_numbers(i)           <<") "
                              "File: " MYELLOW << clean_path(tb->file(i))    << MRESET
                             " Line: " MYELLOW << clean_numbers(tb->line(i)) << MRESET << " ";

                    current_indent = tb->indent(i);

                    if (current_indent > old_indent)
                    {
                        for(int j = 0; j < old_indent; j++)
                        {
                            if (j % 2)
                                out << "|";
                            else
                                out << ":";
                        }
                        out << "+";

                        for (int j = old_indent + 1; j < current_indent; j++)
                            out << "-";
                    }
                    else
                    {
                        for (int j = 0; j < current_indent - 1; j++)
                        {
                            if (j % 2)
                                out << "|";
                            else
                                out << ":";
                        }
                        out << "+";
                    }

                    old_indent = tb->indent(i);

                    out << "- Function: " MYELLOW << tb->function(i)            << MRESET "\n";
                }
                else
                    for(int i = 0; i < tb->traceback.size(); ++i)
                        out << "[Ln: " MBLUE << clean_numbers(tb->line(i)) << MRESET
                             ", Col: " MBLUE << clean_numbers(tb->col(i))  << MRESET "]\n";

//                        out <<    "    "       << clean_numbers(i)           <<") "
//                                "File: " MBLUE << clean_path(tb->file(i))    << MRESET
//                                "
//                           " Function: " MBLUE << tb->function(i)            << MRESET "\n";
            }

        out << "\n";

        return 0;
    }
}

#endif
