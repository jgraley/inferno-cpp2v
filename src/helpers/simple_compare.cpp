#include "simple_compare.hpp"
#include "common/trace.hpp"

#define CHECK_NEW_OLD_NODE_COMPARE

bool SimpleCompare::operator()( TreePtr<Node> x, TreePtr<Node> y )
{   
    // If we are asked to do a trivial compare, return immediately reporting success
    if( x==y )
        return true;
    
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
        return false;

    // Itemise them both and chuck out if sizes do not match
    vector< Itemiser::Element * > x_memb = x->Itemise();
    vector< Itemiser::Element * > y_memb = y->Itemise();
    if( x_memb.size() != y_memb.size() )
        return false; 
    
    for( int i=0; i<x_memb.size(); i++ )
    {
        bool r;
        ASSERT( y_memb[i] )( "itemise returned null element");
        ASSERT( x_memb[i] )( "itemise returned null element");

        if( SequenceInterface *x_seq = dynamic_cast<SequenceInterface *>(x_memb[i]) )
        {
            SequenceInterface *y_seq = dynamic_cast<SequenceInterface *>(y_memb[i]);
            if( !y_seq )
                return false;
            if( !operator()( *x_seq, *y_seq ) )
                return false;                
        }
        else if( CollectionInterface *x_col = dynamic_cast<CollectionInterface *>(x_memb[i]) )
        {
            CollectionInterface *y_col = dynamic_cast<CollectionInterface *>(y_memb[i]);
            if( !y_col )
                return false;
            if( !operator()( *x_col, *y_col ) )
                return false;                
        }
        else if( TreePtrInterface *x_ptr = dynamic_cast<TreePtrInterface *>(x_memb[i]) )
        {
            TreePtrInterface *y_ptr = dynamic_cast<TreePtrInterface *>(y_memb[i]);
            if( !y_ptr )
                return false;
            if( !operator()( *x_ptr, *y_ptr ) )
                return false;                
        }
        else
        {
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a TreePtr");
        }
    }

    // survived to the end? then we have a match.
    return true;
}


bool SimpleCompare::operator()( SequenceInterface &x, SequenceInterface &y )
{
    // Ensure the sizes are the same so we don;t go off the end
    if( x.size() != y.size() )
        return false;
    
    ContainerInterface::iterator xit, yit;
    
    // Check each element in turn
    for( xit = x.begin(), yit = y.begin(); xit != x.end(); ++xit, ++yit )
    {
        if( !operator()( *xit, *yit ) )
            return false;
    }

    // survived to the end? then we have a match.
    return true;
}


bool SimpleCompare::operator()( CollectionInterface &x, CollectionInterface &y )
{
    // Ensure the sizes are the same so we don;t go off the end
    if( x.size() != y.size() )
        return false;
    
    Collection<Node> xremaining;
    FOREACH( const TreePtrInterface &xe, x )
        xremaining.insert( xe );
    
    FOREACH( const TreePtrInterface &ye, y )
    {
        bool found = false;
        TreePtr<Node> xfound;
        ContainerInterface::iterator xit, xit_found;
        for( xit=xremaining.begin(); xit != xremaining.end(); ++xit )
        {
            const TreePtrInterface &xe = *xit;
            if( operator()( xe, ye ) )
            {
                found = true;
                xfound = xe;
                xit_found = xit;
                break;
            }
        }
        if( !found )
            return false;
        
        // Erase the element ising iterator in case of duplicates in xremaining
        xremaining.erase( xit_found );
    }

    // survived to the end? then we have a match.
    return true;
}
