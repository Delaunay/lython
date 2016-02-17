#pragma once

#include <unordered_map>
#include "../Types.h"

namespace lython{

struct Precedence
{
public:
    Precedence(int32 l, int32 r):
        _ll(l), _lr(r)
    {}
    int32 ll();
    int32 lr();

private:
    int32 _ll;
    int32 _lr;
};

typedef std::unordered_map<std::string, Precedence> Grammar;

Grammar& default_grammar();

}
