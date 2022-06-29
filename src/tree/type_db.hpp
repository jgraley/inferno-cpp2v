
#ifndef TYPE_DB_HPP
#define TYPE_DB_HPP

#include "llvm/ADT/APFloat.h"

struct TypeDb
{
    static const int char_bits;
    static const bool int_default_signed;
    static const bool char_default_signed;
    static const int integral_bits[];
    static const llvm::fltSemantics * const float_semantics;
    static const llvm::fltSemantics * const double_semantics;    
    static const llvm::fltSemantics * const long_double_semantics;    
};

#endif

