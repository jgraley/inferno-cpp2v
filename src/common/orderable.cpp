#include "orderable.hpp"
#include "trace.hpp"
#include "standard.hpp"

#include <typeinfo>
#include <typeindex>		

Orderable::Diff Orderable::OrderCompare3Way( const Orderable *l, 
                                             const Orderable *r, 
                                             OrderProperty order_property )
{
    //FTRACE(Traceable::TypeIdName(*l))(" ")(Traceable::TypeIdName(*r))("\n");
    
    type_index l_index( typeid(*l) );
    type_index r_index( typeid(*r) );
    if( l_index != r_index )
        return (l_index > r_index) ? 1 : -1;

    Diff id = l->OrderCompare3WayLocal(r, order_property);
    if( id  )
        return id;
        
    return l->OrderCompare3WayChildren(r, order_property);
}


Orderable::Diff Orderable::OrderCompare3WayLocal( const Orderable *right, 
                                                  OrderProperty order_property ) const 
{
    // Often, there are no contents to compare
    return 0; 
}


Orderable::Diff Orderable::OrderCompare3WayChildren( const Orderable *right, 
                                                     OrderProperty order_property ) const 
{
    // Sometimes we don't incorprorate children in this ordering 
    // (i.e. local ordering only is required)
    return 0; 
}


bool Orderable::OrderCompareEqual( const Orderable *l, 
                                   const Orderable *r, 
                                   OrderProperty order_property )
{
    return OrderCompare3Way( l, r, order_property ) == 0;
}                                   

