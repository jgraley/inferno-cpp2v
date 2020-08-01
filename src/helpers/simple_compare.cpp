#include "simple_compare.hpp"
#include "common/trace.hpp"

void SimpleCompare::operator()( TreePtr<Node> x, TreePtr<Node> y )
{
    INDENT("=");
    
    // If we are asked to do a trivial compare, return immediately reporting success
    if( x==y )
        return;
    
    // Local comparison deals with node type (or any overloaded matching rule)
    // Try both ways to explicitly disallow wildcarding (this fn guaranteed symmetrical)
    x->CheckLocalMatch(y.get());
    y->CheckLocalMatch(x.get());

    // Itemise them both and chuck out if sizes do not match
    vector< Itemiser::Element * > x_memb = x->Itemise();
    vector< Itemiser::Element * > y_memb = y->Itemise();
    if( x_memb.size() != y_memb.size() )
        throw ItemiseSizeMismatch();
    
    for( int i=0; i<x_memb.size(); i++ )
    {
        bool r;
        ASSERT( y_memb[i] )( "itemise returned null element");
        ASSERT( x_memb[i] )( "itemise returned null element");

        if( SequenceInterface *x_seq = dynamic_cast<SequenceInterface *>(x_memb[i]) )
        {
            SequenceInterface *y_seq = dynamic_cast<SequenceInterface *>(y_memb[i]);
            if( !y_seq )
                throw MemberKindMismatch();
            operator()( *x_seq, *y_seq );
        }
        else if( CollectionInterface *x_col = dynamic_cast<CollectionInterface *>(x_memb[i]) )
        {
            CollectionInterface *y_col = dynamic_cast<CollectionInterface *>(y_memb[i]);
            if( !y_col )
                throw MemberKindMismatch();
            operator()( *x_col, *y_col );
        }
        else if( TreePtrInterface *x_ptr = dynamic_cast<TreePtrInterface *>(x_memb[i]) )
        {
            TreePtrInterface *y_ptr = dynamic_cast<TreePtrInterface *>(y_memb[i]);
            if( !y_ptr )
                throw MemberKindMismatch();
            operator()( *x_ptr, *y_ptr );
        }
        else
        {
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a TreePtr");
        }
    }

    // survived to the end? then we have a match.
}


void SimpleCompare::operator()( SequenceInterface &x, SequenceInterface &y )
{
    // Ensure the sizes are the same so we don;t go off the end
    if( x.size() != y.size() )
        throw SequenceSizeMismatch();
    
    ContainerInterface::iterator xit, yit;
    
    // Check each element in turn
    for( xit = x.begin(), yit = y.begin(); xit != x.end(); ++xit, ++yit )
    {
        operator()( *xit, *yit );
    }

    // survived to the end? then we have a match.
}


void SimpleCompare::operator()( CollectionInterface &x, CollectionInterface &y )
{
    // Ensure the sizes are the same so we don;t go off the end
    if( x.size() != y.size() )
        throw CollectionSizeMismatch();
    
    Collection<Node> xremaining;
    FOREACH( const TreePtrInterface &xe, x )
        xremaining.insert( xe );

    ContainerInterface::iterator xit, yit;
    FOREACH( const TreePtrInterface &ye, y )
    {
        bool found = false;
        TreePtr<Node> xfound;
        FOREACH( const TreePtrInterface &xe, xremaining )
        {
            try
            {
                operator()( xe, ye );
                found = true;
                xfound = xe;
                break;
            }
            catch( Mismatch & )
            {
                continue;
            }
        }
        if( !found )
            throw CollectionContentMismatch();
        
        // Try to erase the element
        int ner = xremaining.erase( xfound );
        ASSERT( ner == 1 )("Erase erased %d elements, huh?\n", ner );
    }

    // survived to the end? then we have a match.
}
