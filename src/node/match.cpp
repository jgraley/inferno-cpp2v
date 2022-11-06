#include "match.hpp"
#include "common/common.hpp"
#include <typeinfo>
#include <string>
#include <typeindex>		

bool Matcher::IsLocalMatch( const Matcher &candidate ) const
{
    if( IsSubcategory( candidate ) )
        return IsLocalMatchCovariant(candidate);
    else
        return false;
        
}


bool Matcher::IsLocalMatchCovariant( const Matcher &candidate ) const
{
    // Default local matching criterion checks only the type of the candidate. If the
    // candidate's class is a (non-strict) subclass of this class, we have a match.
    return true;
}


Matcher::~Matcher()
{
}


bool Matcher::IsEquivalentCategory( const Matcher &l, const Matcher &r )
{
    // IsSubcategory() is a weak subcategory, so two nodes represent equivalent 
    // category if and only if each is a subcategory of the other.
    return l.IsSubcategory(r) && r.IsSubcategory(l);
}
