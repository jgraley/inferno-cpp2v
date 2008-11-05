#include "type_info.hpp"

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


