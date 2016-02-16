#pragma once

/*
 *  Types are special in that language.
 *      in C/C++ Types specify how a value must be interpreted
 *
 *      Here Types are even more specialized since it will specify
 *      how the variable will behave and how its value must be interpreted
 *
 *      i.e Type Specification is almost a language in itself
 */

/*
#include <complex>
#include "../Types.h"

namespace lython{
class Type
{};

//      Number related types
// -------------------------------
class Numeric : public Type
{};

// N integer
class Natural : public Numeric
{
    typedef uint32 type;    // machine type
};

// Z negative + postive integer
class Relatif: public Numeric
{
    typedef int32 type;
};

// Q (x/y) with x, y \in Z
class Rational: public Numeric
{
    typedef float64 type;
};

// R
class Real: public Numeric
{
    typedef float64 type;
};

// C (R + iR)
class Complex: public Numeric
{
    typedef std::complex type;
};


//          Type Operator
//  ----------------------------------

class Constraint: public  Type
{};

// Range    start <= t < end
class Range : public Constraint
{
    Range(Numeric* t, start, end);

};*/

}
