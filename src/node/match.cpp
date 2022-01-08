#include "match.hpp"
#include "common/common.hpp"
#include <typeinfo>
#include <string>
#include <typeindex>		

bool Matcher::IsLocalMatch( const Matcher *candidate ) const
{
    // Default local matching criterion checks only the type of the candidate. If the
    // candidate's class is a (non-strict) subclass of this class, we have a match.
    return IsSubclass( candidate );
}


Matcher::~Matcher()
{
}
