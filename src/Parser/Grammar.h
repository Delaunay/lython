#pragma once

#include <unordered_map>
#include "../Types.h"

namespace lython{

struct Precedence
{
public:
    Precedence(int32 l=-1, int32 r=-1):
        _ll(l), _lr(r)
    {}
    int32 ll();
    int32 lr();

    operator bool(){
        return _ll != -1 || _lr != -1;
    }

private:
    int32 _ll;
    int32 _lr;
};

typedef std::unordered_map<std::string, Precedence> Grammar;

Grammar& default_grammar();

}
