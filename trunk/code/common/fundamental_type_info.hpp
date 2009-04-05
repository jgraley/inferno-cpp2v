
#ifndef FUNDAMENTAL_TYPE_INFO_HPP
#define FUNDAMENTAL_TYPE_INFO_HPP

#include "llvm/ADT/APFloat.h"

struct TypeInfo
{
    static const int char_bits;
    static const bool int_default_signed;
    static const bool char_default_signed;
    static const unsigned integral_bits[];
    static const llvm::fltSemantics *(floating_semantics[]);
    static const int float_bits;
    static const int double_bits;    
    static const int long_double_bits;    
};

#endif

