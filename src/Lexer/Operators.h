//===--- OperatorKinds.h - C++ Overloaded Operators -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines an enumeration for C++ overloaded operators.
///
//===----------------------------------------------------------------------===//

#ifndef LYTHON_LEXER_OPERATORS_HEADER
#define LYTHON_LEXER_OPERATORS_HEADER

namespace lython {

/// \brief Enumeration specifying the different kinds of C++ overloaded
/// operators.
enum OverloadedOperatorKind : int
{
    OO_None,                ///< Not an overloaded operator

#define OVERLOADED_OPERATOR(Name,Spelling,Token,Unary,Binary,MemberOnly) \
    OO_##Name,
#include "Operators.def"

    NUM_OVERLOADED_OPERATORS
};


}
#endif
