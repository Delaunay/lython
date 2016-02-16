#ifndef KIWI_PARSER_ERROR_HEADER
#define KIWI_PARSER_ERROR_HEADER

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include "../config.h"
#include "../path_hack.h"

namespace LIBNAMESPACE {

    std::string clean_path(const std::string& path);

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
                file(i), function(j), line(k), col(c), intern(intern), indent(idt)
            {}

            void print(std::ostream& out=std::cout, int i=0)
            {
                out <<      "    " << clean_numbers(i)    << ") "
                          "File: " << clean_path(file)    <<
                         " Line: " << clean_numbers(line) << " ";

                for(int j = 0; j < indent - 1; j++)
                {
                    if (j % 2)
                        out << "|";
                    else
                        out << ":";
                }
                out << "+";
                out << "- Function: " << function         << std::endl;
            }

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

        void add(std::string i, std::string j, unsigned int k, int idt=0, bool push_to_out=false)
        {
            traceback.push_back(Element(i, j, k, 0, true, idt));
            if (idt == 1)
                out << "\n";

            if (/*push_to_out &&*/ idt < 100)
                (*traceback.rbegin()).print(out, traceback.size() - 1);
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
     *  Parser Traceback dump
     */
    void dump_format_L(Traceback* tb = 0,  std::ostream& out = std::cout);

    template<typename T>
    T* error(const char* str, const int& line, const int& col, Traceback* tb = 0,  std::ostream& out = std::cout)
    {
        out << MGREEN "\n[Ln: " << clean_numbers(line) <<
                      ", Col: " << clean_numbers(col)  << "] " MRESET "    "
                   // " /!\\ Warning "
                 MRED " /!\\ Error   " MRESET "     "<< str <<"\n";

        dump_format_L(tb, out);
        out << "\n";
        std::flush(out);

        return 0;
    }

    template<typename T>
    T* warning(const char* str, const int& line, const int& col, Traceback* tb = 0,  std::ostream& out = std::cout)
    {
        out << MGREEN "\n[Ln: " << clean_numbers(line) <<
                      ", Col: " << clean_numbers(col)  << "] " MRESET "    "
                   // " /!\\ Warning "
              MYELLOW " /!\\ Warning " MRESET "     "<< str <<"\n";

        dump_format_L(tb, out);
        out << "\n";
        std::flush(out);

        return 0;
    }

    template<typename T>
    T* info(const char* str, const int& line, const int& col, Traceback* tb = 0,  std::ostream& out = std::cout)
    {
        out << MGREEN "\n[Ln: " << clean_numbers(line) <<
                      ", Col: " << clean_numbers(col)  << "] " MRESET "    "
                   // " /!\\ Warning "
                MBLUE " /!\\ Info    " MRESET "     "<< str <<"\n";

        dump_format_L(tb, out);
        out << "\n";
        std::flush(out);

        return 0;
    }

    inline std::string clean_path(const std::string& path)
    {
        std::string r;
        std::copy(path.begin() + source_path().size() - 13, path.end(), std::back_inserter(r));
        return r;
    }

    inline std::string clean_numbers(unsigned int k)
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

}

#endif
