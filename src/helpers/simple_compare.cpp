#include "simple_compare.hpp"
#include "common/trace.hpp"


SimpleCompare::SimpleCompare( Matcher::Ordering ordering_ ) :
    ordering( ordering_ )
{
}


CompareResult SimpleCompare::Compare( TreePtr<Node> x, TreePtr<Node> y )
{   
    // Inputs must be non-NULL (though we do handle NULL in itemise, see below)
    ASSERT(x);
    ASSERT(y);

    //FTRACE("SC::Compare ")(x)(" - ")(y)("\n");

    // If we are asked to do a trivial compare, return immediately reporting success
    if( x==y )
        return EQUAL;
        
    // Local comparison deals with node type and value if there is one
    CompareResult cr = Node::Compare(x.get(), y.get(), ordering);
    
    if( cr != EQUAL )
        return cr;

    // Itemise them both and chuck out if sizes do not match
    vector< Itemiser::Element * > x_memb = x->Itemise();
    vector< Itemiser::Element * > y_memb = y->Itemise();
    int sd = (int)(x_memb.size()) - (int)(y_memb.size());
    if( sd != EQUAL )
        return sd; 
    
    for( int i=0; i<x_memb.size(); i++ )
    {
        bool r;
        ASSERT( y_memb[i] )( "itemise returned null element");
        ASSERT( x_memb[i] )( "itemise returned null element");

        if( SequenceInterface *x_seq = dynamic_cast<SequenceInterface *>(x_memb[i]) )
        {
            SequenceInterface *y_seq = dynamic_cast<SequenceInterface *>(y_memb[i]);
            ASSERT( y_seq );
            CompareResult cr = Compare( *x_seq, *y_seq );
            if( cr != EQUAL )
                return cr;                
        }
        else if( CollectionInterface *x_col = dynamic_cast<CollectionInterface *>(x_memb[i]) )
        {
            CollectionInterface *y_col = dynamic_cast<CollectionInterface *>(y_memb[i]);
            ASSERT( y_col );
            CompareResult cr = Compare( *x_col, *y_col );
            if( cr != EQUAL )
                return cr;                
        }
        else if( TreePtrInterface *x_ptr = dynamic_cast<TreePtrInterface *>(x_memb[i]) )
        {
            TreePtrInterface *y_ptr = dynamic_cast<TreePtrInterface *>(y_memb[i]);
            ASSERT( y_ptr );
            
            // MakeValueArchitype() can generate nodes with NULL pointers (eg in PointerIs)
            // and these get SimpleCompared even though they are not allowed in input trees.
            // In this case, order on the non-null-ness of x and y.
            if( !(TreePtr<Node>)*x_ptr || !(TreePtr<Node>)*y_ptr )
                return (int)(!(TreePtr<Node>)*y_ptr) - (int)(!(TreePtr<Node>)*x_ptr);                
            
            // Both non-null, so we are allowed to recurse
            CompareResult cr = Compare( (TreePtr<Node>)*x_ptr, (TreePtr<Node>)*y_ptr );
            if( cr != EQUAL )
                return cr;                
        }
        else
        {
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a TreePtr");
        }
    }

    // survived to the end? then we have a match.
    return EQUAL;
}


CompareResult SimpleCompare::Compare( SequenceInterface &x, SequenceInterface &y )
{
    // Ensure the sizes are the same so we don;t go off the end
    int sd = (int)(x.size()) - (int)(y.size());
    if( sd != EQUAL )
        return sd;
    
    ContainerInterface::iterator xit, yit;
    
    // Check each element in turn
    for( xit = x.begin(), yit = y.begin(); xit != x.end(); ++xit, ++yit )
    {
        CompareResult cr = Compare( (TreePtr<Node>)*xit, (TreePtr<Node>)*yit );
        if( cr != EQUAL )
            return cr;
    }

    // survived to the end? then we have a match.
    return EQUAL;
}


CompareResult SimpleCompare::Compare( CollectionInterface &x, CollectionInterface &y )
{
    // Ensure the sizes are the same so we don't go off the end
    int sd = (int)(x.size()) - (int)(y.size());
    if( sd != EQUAL )
        return sd;
        
    // Use this object so our ordering is used.
    Ordered xo = GetOrdered(x);
    Ordered yo = GetOrdered(y);

    // Compare them (will use SimpleCompare)
    return (int)(xo > yo) - (int)(xo < yo);
}


bool SimpleCompare::operator()( TreePtr<Node> xl, TreePtr<Node> yl )
{
    //FTRACE("SC::operator() ")(xl)(" - ")(yl)("\n");
    return Compare(xl, yl) < EQUAL;
}


SimpleCompare::Ordered SimpleCompare::GetOrdered( ContainerInterface &c )
{
    Ordered ordered( *this );
    FOREACH( const TreePtrInterface &e, c )
    	ordered.insert( (TreePtr<Node>)e );
    return ordered; // hoping for a "move"
}
