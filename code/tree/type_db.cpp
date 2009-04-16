#include "type_db.hpp"
#include "llvm/ADT/APFloat.h"

//
// Initialise all the sizes and signednesses according to the 
// current compiler - this is good for testing if the same 
// compiler is used to compile generated output.
//

#define IS_SIGNED( T ) ((T)-1 < (T)0)

const int TypeDb::char_bits = sizeof( char ) * 8;

const bool TypeDb::int_default_signed = IS_SIGNED(int);

const bool TypeDb::char_default_signed = IS_SIGNED(char);

const unsigned TypeDb::integral_bits[] =
{
    sizeof( int ) * 8,
    sizeof( short ) * 8,
    sizeof( long ) * 8,
    sizeof( long long ) * 8
};

const llvm::fltSemantics *(TypeDb::floating_semantics[]) =
{
    &llvm::APFloat::IEEEdouble,
    &llvm::APFloat::IEEEsingle, // pretend float == short double
    &llvm::APFloat::IEEEquad,
    NULL
};

const int TypeDb::float_bits = sizeof( float ) * 8;

const int TypeDb::double_bits = sizeof( double ) * 8;

const int TypeDb::long_double_bits = sizeof( long double ) * 8;
