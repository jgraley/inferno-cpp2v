#include "type_db.hpp"
#include "llvm/ADT/APFloat.h"

//
// Initialise all the sizes and signednesses of integrals according to the 
// current compiler - this is good for testing, when the same
// compiler is used to compile generated output.
//

#define IS_SIGNED( T ) ((T)-1 < (T)0)

const int TypeDb::char_bits = sizeof( char ) * 8;
const bool TypeDb::int_default_signed = IS_SIGNED(int);
const bool TypeDb::char_default_signed = IS_SIGNED(char);
const int TypeDb::integral_bits[] =
{
    sizeof( int ) * 8,
    sizeof( short ) * 8,
    sizeof( long ) * 8,
    sizeof( long long ) * 8
};

//
// Use IEEE standards for floats since reflection is too hard
//
const llvm::fltSemantics * const TypeDb::float_semantics = &llvm::APFloat::IEEEsingle;
const llvm::fltSemantics * const TypeDb::double_semantics = &llvm::APFloat::IEEEdouble;
const llvm::fltSemantics * const TypeDb::long_double_semantics = &llvm::APFloat::IEEEquad;
