#include "type_data.hpp"
#include "llvm/ADT/APFloat.h"

//
// Initialise all the sizes and signednesses of integrals according to the 
// current compiler - this is good for testing, when the same
// compiler is used to compile generated output.
//

#define IS_SIGNED( T ) ((T)-1 < (T)0)

const int TypeDb::char_bits = sizeof( char ) * 8;
const bool TypeDb::char_default_signed = IS_SIGNED(char);
const int TypeDb::integral_bits[] =
{
    sizeof( int ) * 8,      // TSW_undefined
    sizeof( short ) * 8,    // TSW_short
    sizeof( long ) * 8,     // TSW_long
    sizeof( long long ) * 8 // TSW_longlong
};

const int TypeDb::size_t_bits = sizeof( size_t ) * 8;

//
// Use IEEE standards for floats since reflection is too hard
//
const llvm::fltSemantics * const TypeDb::float_semantics = &llvm::APFloat::IEEEsingle;
const llvm::fltSemantics * const TypeDb::double_semantics = &llvm::APFloat::IEEEdouble;
const llvm::fltSemantics * const TypeDb::long_double_semantics = &llvm::APFloat::IEEEquad;
