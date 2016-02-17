#ifndef PRELEXER_H
#define PRELEXER_H

#include "Buffer.h"


#include <unordered_set>
#include <cstdio>
#include <boost/variant.hpp>


namespace lython{

    enum PreTokenType
    {
        pretok_pretok,
        pretok_prestring,
        pretok_preblock
    };

    class PreToken
    {
    public:
        typedef std::vector<PreToken> Block;
        typedef boost::variant<std::string, Block> PreTokenData;

        PreTokenType type()      {   return _type;                            }
        std::string& as_string() {   return boost::get<std::string>(_data);   }
        Block&       as_block()  {   return boost::get<Block>(_data);         }

        PreToken(PreTokenType t, const std::string& str, uint32 line=0, uint32 col=0):
            _type(t), _data(str), _line(line), _col(col)
        {}

        PreToken(Block& block, uint32 line=0, uint32 col=0):
            _type(pretok_preblock), _data(block), _line(line), _col(col)
        {}

        // default
        PreToken(uint32 line=0, uint32 col=0):
            _type(pretok_pretok), _line(line), _col(col)
        {}

        uint32 line()   {   return _line;   }
        uint32 col()    {   return _col;    }

        uint32 line_begin() {
            if (type() == pretok_preblock){
                if (as_block().size() > 0)
                    return as_block()[0].line();
                return line();
            }
            return line();
        }

        uint32 line_end(){ return line(); }

    private:
        PreTokenType _type;
        PreTokenData _data;

        uint32 _line;
        uint32 _col;

    public:
        std::ostream& debug_print(std::ostream& out);
        std::ostream& print(std::ostream& out);
    };

    class Prelexer
    {
    public:
        Prelexer(AbstractBuffer& reader):
            _reader(reader)
        {}

        // shortcuts
        const std::string& file_name()   {   return _reader.file_name();  }
        uint32 line()      {    return _reader.line();      }
        uint32 col()       {    return _reader.col();       }
        uint32 indent()    {    return _reader.indent();    }
        char   nextc()     {    return _reader.nextc();     }
        bool   empty_line(){    return _reader.empty_line();}

        // Error Printing
        void print_error(const char* msg, int err_code){
            printf("[Prelexer ERROR] [l:%d c:%d]  EC %d: %s\n",
                   line(), col(), err_code, msg);
        }

        void eat_invisible()
        {
            while(c == ' ' || c == '\n' && c != EOF){
                c = nextc();
            }
        }

        // prelexing
        PreToken next_pretoken(){
            if (_run_once){
                c = nextc();    // get first char
                _run_once = false;
            }

            // eat white space
            eat_invisible();

            // eat comment
            if (c == '%'){
                while(c != '\n' && c != EOF){
                    c = nextc();
                }
                eat_invisible();   // eat '\n'
            }

            // PreString
            if (c == '"'){
                std::string str;
                c = nextc(); // eat '"'

                while(c != '"' && c != EOF){

                    if (c == '\\'){
                        c = nextc();

                        switch(c){
                            case 't': str.push_back('\t'); break;
                            case 'n': str.push_back('\n'); break;
                            case 'r': str.push_back('\r'); break;
                            default:
                                print_error("Unimplemented unicode escape", 3);
                                break;
                        }
                    }

                    if (c == EOF)
                        print_error("Unterminated string", 1);

                    str.push_back(c);
                    c = nextc();
                }

                PreToken tok = make_prestring(str);
                c = nextc(); // eat '"'
                eat_invisible();
                return tok;
            }

            // PreBlock
            if (c == '{'){
                PreToken::Block block;
                c = nextc(); // eat '{'

                while (c != '}' && c != EOF){
                    // we don't want to start reading an empty pretoken
                    eat_invisible();
                    block.push_back(next_pretoken());
                }

                if (c == EOF)
                    print_error("Unmatched closing brace", 2);

                PreToken tok = make_preblock(block);
                c = nextc(); // eat '}'
                eat_invisible();
                return tok;
            }

            // PreToken
            std::string str;

            while(_stop_char.count(c) == 0 && c != EOF){
                str.push_back(c);
                c = nextc();
            }

            if (str.size() == 0){
                eat_invisible();
                return next_pretoken();
            }

            PreToken tok = make_pretoken(str);
            eat_invisible();
            return tok;
        }

        PreToken make_prestring(const std::string& str){
            return PreToken(pretok_prestring, str, line(), col());
        }

        PreToken make_preblock(PreToken::Block& block){
            return PreToken(block, line(), col());
        }

        PreToken make_pretoken(const std::string& str){
            return PreToken(pretok_pretok, str, line(), col());
        }

        operator bool(){
            return bool(_reader);
        }

    private:
        AbstractBuffer& _reader;
        char c;
        bool _run_once{true};
        std::unordered_set<char> _stop_char{' ', '\t', '\n', '\r','%', '"', '{', '}'};

    public:
        // print all pretokens and their info
        std::ostream& debug_print(std::ostream& out);
        std::ostream& print(std::ostream& out);
    };
}

#endif // PRELEXER_H
