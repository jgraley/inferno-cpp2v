#include "orderable.hpp"

#include <typeinfo>
#include <typeindex>		

Orderable::Result Orderable::OrderCompare( const Orderable *l, 
                                           const Orderable *r, 
                                           OrderProperty order_property )
{
    type_index l_index( typeid(*l) );
    type_index r_index( typeid(*r) );
    if( l_index != r_index )
        return (l_index > r_index) ? 1 : -1;

    Result ir = l->OrderCompareInternals(r, order_property);
    if( ir != EQUAL )
        return ir;
        
    return l->OrderCompareChildren(r, order_property);
}


Orderable::Result Orderable::OrderCompareInternals( const Orderable *candidate, 
                                                    OrderProperty order_property ) const 
{
    // Often, there are no contents to compare
    return EQUAL; 
}


Orderable::Result Orderable::OrderCompareChildren( const Orderable *candidate, 
                                                   OrderProperty order_property ) const 
{
    // Sometimes we don't incorprorate children in this ordering 
    // (i.e. local ordering only is required)
    return EQUAL; 
}
