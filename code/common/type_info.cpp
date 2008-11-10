#include "type_info.hpp"

//
// Initialise all the sizes and signednesses according to the 
// current compiler - this is good for testing if the same 
// compiler is used to compile generated output.
//

#define IS_SIGNED( T ) ((T)-1 < (T)0)

const int TypeInfo::char_bits = sizeof( char ) * 8;

const bool TypeInfo::int_default_signed = IS_SIGNED(int);

const bool TypeInfo::char_default_signed = IS_SIGNED(char);

const unsigned TypeInfo::integral_bits[] =
{
    sizeof( int ) * 8,
    sizeof( short ) * 8,
    sizeof( long ) * 8,
    sizeof( long long ) * 8
};

const int TypeInfo::float_bits = sizeof( float ) * 8;

const int TypeInfo::double_bits = sizeof( double ) * 8;

const int TypeInfo::long_double_bits = sizeof( long double ) * 8;
