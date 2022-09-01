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


Orderable::Result SimpleCompare::Compare( TreePtr<Node> x, TreePtr<Node> y ) const
{   
    // Inputs must be non-NULL (though we do handle NULL in itemise, see below)
    ASSERT(x);
    ASSERT(y);

    //FTRACE("SC::Compare ")(x)(" - ")(y)("\n");

    // If we are asked to do a trivial compare, return immediately reporting success
    if( x==y )
        return Orderable::EQUAL;
        
    // Local comparison deals with node type and value if there is one
    Orderable::Result cr = Node::OrderCompare(x.get(), y.get(), order_property);
    
    if( cr != Orderable::EQUAL )
        return cr;

    // Itemise them both and chuck out if sizes do not match
    vector< Itemiser::Element * > x_items = x->Itemise();
    vector< Itemiser::Element * > y_items = y->Itemise();
    int sd = (int)(x_items.size()) - (int)(y_items.size());
    if( sd != Orderable::EQUAL )
        return sd; 
    
    for( int i=0; i<x_items.size(); i++ )
    {
        bool r;
        ASSERT( y_items[i] )( "itemise returned null element");
        ASSERT( x_items[i] )( "itemise returned null element");

        if( SequenceInterface *x_seq = dynamic_cast<SequenceInterface *>(x_items[i]) )
        {
            SequenceInterface *y_seq = dynamic_cast<SequenceInterface *>(y_items[i]);
            ASSERT( y_seq );
            Orderable::Result cr = Compare( *x_seq, *y_seq );
            if( cr != Orderable::EQUAL )
                return cr;                
        }
        else if( CollectionInterface *x_col = dynamic_cast<CollectionInterface *>(x_items[i]) )
        {
            CollectionInterface *y_col = dynamic_cast<CollectionInterface *>(y_items[i]);
            ASSERT( y_col );
            Orderable::Result cr = Compare( *x_col, *y_col );
            if( cr != Orderable::EQUAL )
                return cr;                
        }
        else if( TreePtrInterface *x_singular = dynamic_cast<TreePtrInterface *>(x_items[i]) )
        {
            TreePtrInterface *y_singular = dynamic_cast<TreePtrInterface *>(y_items[i]);
            ASSERT( y_singular );
            
            // MakeValueArchetype() can generate nodes with NULL pointers (eg in PointerIs)
            // and these get SimpleCompared even though they are not allowed in input trees.
            // In this case, order on the non-null-ness of x and y.
            if( !(TreePtr<Node>)*x_singular || !(TreePtr<Node>)*y_singular )
                return (int)(!(TreePtr<Node>)*y_singular) - (int)(!(TreePtr<Node>)*x_singular);                
            
            // Both non-null, so we are allowed to recurse
            Orderable::Result cr = Compare( (TreePtr<Node>)*x_singular, (TreePtr<Node>)*y_singular );
            if( cr != Orderable::EQUAL )
                return cr;                
        }
        else
        {
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a TreePtr");
        }
    }

    // survived to the end? then we have a match.
    return Orderable::EQUAL;
}


Orderable::Result SimpleCompare::Compare( SequenceInterface &x, SequenceInterface &y ) const
{
    // Ensure the sizes are the same so we don;t go off the end
    int sd = (int)(x.size()) - (int)(y.size());
    if( sd != Orderable::EQUAL )
        return sd;
    
    ContainerInterface::iterator xit, yit;
    
    // Check each element in turn
    for( xit = x.begin(), yit = y.begin(); xit != x.end(); ++xit, ++yit )
    {
        Orderable::Result cr = Compare( (TreePtr<Node>)*xit, (TreePtr<Node>)*yit );
        if( cr != Orderable::EQUAL )
            return cr;
    }

    // survived to the end? then we have a match.
    return Orderable::EQUAL;
}


Orderable::Result SimpleCompare::Compare( CollectionInterface &x, CollectionInterface &y ) const
{
    // Ensure the sizes are the same so we don't go off the end
    int sd = (int)(x.size()) - (int)(y.size());
    if( sd != Orderable::EQUAL )
        return sd;
        
    // Use this object so our ordering is used.
    Orderered xo = GetOrdering(x);
    Orderered yo = GetOrdering(y);
    
    return LexicographicalCompare(xo, yo, *this);
}


bool SimpleCompare::operator()( TreePtr<Node> xl, TreePtr<Node> yl ) const
{
    //FTRACE("SC::operator() ")(xl)(" - ")(yl)("\n");
    return Compare(xl, yl) < Orderable::EQUAL;
}


SimpleCompare::Orderered SimpleCompare::GetOrdering( ContainerInterface &c ) const
{
    Orderered ordered( *this );
    for( const TreePtrInterface &e : c )
    	ordered.insert( (TreePtr<Node>)e );
    return ordered; // hoping for a "move"
}
