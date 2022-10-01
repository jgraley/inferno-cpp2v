#include "simple_compare.hpp"
#include "common/trace.hpp"


SimpleCompare::SimpleCompare( Orderable::OrderProperty order_property_ ) :
    order_property( order_property_ )
{
}


SimpleCompare &SimpleCompare::operator=(const SimpleCompare &other)
{
    order_property = other.order_property;
    return *this;
}


Orderable::Result SimpleCompare::Compare( TreePtr<Node> l, TreePtr<Node> r ) const
{   
    // Inputs must be non-NULL (though we do handle NULL in itemise, see below)
    ASSERT(l);
    ASSERT(r);

    //FTRACE("SC::Compare ")(x)(" - ")(y)("\n");

    // If we are asked to do a trivial compare, return immediately reporting success
    if( l==r )
        return Orderable::EQUAL;
        
    // Local comparison deals with node type and value if there is one
    Orderable::Result cr = Node::OrderCompare(l.get(), r.get(), order_property);
    
    if( cr != Orderable::EQUAL )
        return cr;

    // Itemise them both and chuck out if sizes do not match
    vector< Itemiser::Element * > l_items = l->Itemise();
    vector< Itemiser::Element * > r_items = r->Itemise();
    int sd = (int)(l_items.size()) - (int)(r_items.size());
    if( sd != Orderable::EQUAL )
        return sd; 
    
    for( int i=0; i<l_items.size(); i++ )
    {
        ASSERT( l_items[i] )( "itemise returned null element");
        ASSERT( r_items[i] )( "itemise returned null element");

        if( SequenceInterface *l_seq = dynamic_cast<SequenceInterface *>(l_items[i]) )
        {
            SequenceInterface *r_seq = dynamic_cast<SequenceInterface *>(r_items[i]);
            ASSERT( r_seq );
            Orderable::Result d = Compare( *l_seq, *r_seq );
            if( d != Orderable::EQUAL )
                return d;                
        }
        else if( CollectionInterface *l_col = dynamic_cast<CollectionInterface *>(l_items[i]) )
        {
            CollectionInterface *r_col = dynamic_cast<CollectionInterface *>(r_items[i]);
            ASSERT( r_col );
            Orderable::Result d = Compare( *l_col, *r_col );
            if( d != Orderable::EQUAL )
                return d;                
        }
        else if( TreePtrInterface *l_singular = dynamic_cast<TreePtrInterface *>(l_items[i]) )
        {
            TreePtrInterface *r_singular = dynamic_cast<TreePtrInterface *>(r_items[i]);
            ASSERT( r_singular );
            
            // MakeValueArchetype() can generate nodes with NULL pointers (eg in PointerIs)
            // and these get SimpleCompared even though they are not allowed in input trees.
            // In this case, order on the non-null-ness of x and y.
            if( !(TreePtr<Node>)*l_singular || !(TreePtr<Node>)*r_singular )
                return (int)(!(TreePtr<Node>)*r_singular) - (int)(!(TreePtr<Node>)*l_singular);                
            
            // Both non-null, so we are allowed to recurse
            Orderable::Result d = Compare( (TreePtr<Node>)*l_singular, (TreePtr<Node>)*r_singular );
            if( d != Orderable::EQUAL )
                return d;                
        }
        else
        {
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a TreePtr");
        }
    }

    // survived to the end? then we have a match.
    return Orderable::EQUAL;
}


Orderable::Result SimpleCompare::Compare( SequenceInterface &l, SequenceInterface &r ) const
{
    // Ensure the sizes are the same so we don;t go off the end
    int sd = (int)(l.size()) - (int)(r.size());
    if( sd != Orderable::EQUAL )
        return sd;
    
    ContainerInterface::iterator lit, rit;
    
    // Check each element in turn
    for( lit = l.begin(), rit = r.begin(); lit != l.end(); ++lit, ++rit )
    {
        Orderable::Result d = Compare( (TreePtr<Node>)*lit, (TreePtr<Node>)*rit );
        if( d != Orderable::EQUAL )
            return d;
    }

    // survived to the end? then we have a match.
    return Orderable::EQUAL;
}


Orderable::Result SimpleCompare::Compare( CollectionInterface &l, CollectionInterface &r ) const
{
    // Ensure the sizes are the same so we don't go off the end
    int sd = (int)(l.size()) - (int)(r.size());
    if( sd != Orderable::EQUAL )
        return sd;
        
    // Use this object so our ordering is used.
    Orderered l_ordering = GetOrdering(l);
    Orderered r_ordering = GetOrdering(r);
    
    return LexicographicalCompare(l_ordering, r_ordering, *this);
}


bool SimpleCompare::operator()( TreePtr<Node> l, TreePtr<Node> r ) const
{
    //FTRACE("SC::operator() ")(xl)(" - ")(yl)("\n");
    return Compare(l, r) < Orderable::EQUAL;
}


SimpleCompare::Orderered SimpleCompare::GetOrdering( ContainerInterface &c ) const
{
    Orderered ordered( *this );
    for( const TreePtrInterface &e : c )
    	ordered.insert( (TreePtr<Node>)e );
    return ordered; // hoping for a "move"
}
