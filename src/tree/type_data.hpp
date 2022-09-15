
#ifndef TYPE_DATA_HPP
#define TYPE_DATA_HPP

#include "llvm/ADT/APFloat.h"

struct TypeDb
{
    static const int char_bits;
    static const bool char_default_signed;
    static const int integral_bits[]; // indexed by the LLVM TSW value (type-specifier enum)
    static const int size_t_bits; // always unsigned
    static const llvm::fltSemantics * const float_semantics;
    static const llvm::fltSemantics * const double_semantics;    
    static const llvm::fltSemantics * const long_double_semantics;    
};

#endif

