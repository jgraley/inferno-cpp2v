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


CompareResult Orderable::Compare( const Orderable *l, 
                                  const Orderable *r, 
                                  OrderProperty order_property )
{
    type_index l_index( typeid(*l) );
    type_index r_index( typeid(*r) );
    if( l_index != r_index )
        return (l_index > r_index) ? 1 : -1;
    return l->CovariantCompare(r, order_property);
}


CompareResult Orderable::CovariantCompare( const Orderable *candidate, 
                                           OrderProperty order_property ) const 
{
    return EQUAL; // usually there are no contents to compare
}
