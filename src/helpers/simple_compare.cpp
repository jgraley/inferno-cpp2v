#include "simple_compare.hpp"
#include "common/trace.hpp"

//#define CHECK_NEW_OLD_NODE_COMPARE

CompareResult SimpleCompare::Compare( TreePtr<Node> x, TreePtr<Node> y )
{   
    // If we are asked to do a trivial compare, return immediately reporting success
    if( x==y )
        return EQUAL;
    
    // Local comparison deals with node type and value if there is one
    CompareResult cr = Node::Compare(x.get(), y.get());
    
#ifdef CHECK_NEW_OLD_NODE_COMPARE
    bool new_uneq = (cr != EQUAL);
    bool xlmy = x->IsLocalMatch(y.get());
    bool ylmx = y->IsLocalMatch(x.get());
    bool old_uneq = (!xlmy || !ylmx);
    ASSERT( new_uneq==old_uneq )("x: ")(*x)("\n")
                                ("y: ")(*y)("\n")
                                ("x.LM(y): ")(xlmy)("\n")
                                ("y.LM(x): ")(ylmx)("\n")
                                ("Compare: %d\n", cr);
#endif
    
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
            CompareResult cr = Compare( *x_ptr, *y_ptr );
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
        CompareResult cr = Compare( *xit, *yit );
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
    
    auto sc_less = [this]( const TreePtrInterface *px, const TreePtrInterface *py )
    {
        return Compare(*px, *py) < EQUAL;
    };
    typedef set<const TreePtrInterface *, decltype(sc_less)> SCOrderedPtrs;
    SCOrderedPtrs x_ptrs(sc_less), y_ptrs(sc_less);

    // Fill up the sets of pointers
    FOREACH( const TreePtrInterface &xe, x )
        x_ptrs.insert( &xe );
    FOREACH( const TreePtrInterface &ye, x )
        y_ptrs.insert( &ye );

    // Compare them (will use SimpleCompare)
    return (int)(x_ptrs > y_ptrs) - (int)(x_ptrs < y_ptrs);
}

