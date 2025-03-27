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


// Inputs must be non-NULL (though we do handle NULL in itemise, see below)
bool SimpleCompare::operator()( TreePtr<Node> l, TreePtr<Node> r ) const
{
    //FTRACE("SC::operator() ")(l)(" - ")(r)("\n");
    return Compare3Way(l, r) < 0;
}


Orderable::Diff SimpleCompare::Compare3Way( TreePtr<Node> l, TreePtr<Node> r ) const
{   
    ASSERT( l );
    ASSERT( r );
    return Compare3Way( *l, *r );
}
    
    
bool SimpleCompare::operator()( const Node &l, const Node &r ) const
{
    //FTRACE("SC::operator() ")(l)(" - ")(r)("\n");
    return Compare3Way(l, r) < 0;
}


Orderable::Diff SimpleCompare::Compare3Way( const Node &l, const Node &r ) const
{   
    //INDENT("-");
    //FTRACEC("SC::Compare ")(l)(" - ")(r)("\n");

    // If we are asked to do a trivial compare, return immediately reporting success
    if( &l==&r )
        return 0;
        
    // Local comparison deals with node type and value if there is one
    if( Orderable::Diff cd = Node::OrderCompare3Way(l, r, order_property) )
        return cd;

    // Itemise them both and chuck out if sizes do not match
    vector< Itemiser::Element * > l_items = l.Itemise();
    vector< Itemiser::Element * > r_items = r.Itemise();
    int sd = (int)(l_items.size()) - (int)(r_items.size());
    if( sd != 0 )
        return sd; 
    
    for( vector< Itemiser::Element * >::size_type i=0; i<l_items.size(); i++ )
    {
        ASSERT( l_items[i] )( "itemise returned null element");
        ASSERT( r_items[i] )( "itemise returned null element");

        if( SequenceInterface *l_seq = dynamic_cast<SequenceInterface *>(l_items[i]) )
        {
            SequenceInterface *r_seq = dynamic_cast<SequenceInterface *>(r_items[i]);
            ASSERT( r_seq );
            if( Orderable::Diff d = Compare3Way( *l_seq, *r_seq ) )            
                return d;                
        }
        else if( CollectionInterface *l_col = dynamic_cast<CollectionInterface *>(l_items[i]) )
        {
            CollectionInterface *r_col = dynamic_cast<CollectionInterface *>(r_items[i]);
            ASSERT( r_col );
            if( Orderable::Diff d = Compare3Way( *l_col, *r_col ) )
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
            if( Orderable::Diff d = Compare3Way( *(TreePtr<Node>)*l_singular, *(TreePtr<Node>)*r_singular ) )
                return d;                
        }
        else
        {
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a TreePtr");
        }
    }

    // survived to the end? then we have a match.
    return 0;
}


Orderable::Diff SimpleCompare::Compare3Way( SequenceInterface &l, SequenceInterface &r ) const
{
    // Ensure the sizes are the same so we don;t go off the end
    int sd = (int)(l.size()) - (int)(r.size());
    if( sd != 0 )
        return sd;
    
    ContainerInterface::iterator lit, rit;
    
    // Check each element in turn
    for( lit = l.begin(), rit = r.begin(); lit != l.end(); ++lit, ++rit )
    {
        if( Orderable::Diff d = Compare3Way( *(TreePtr<Node>)*lit, *(TreePtr<Node>)*rit ) )       
            return d;
    }

    // survived to the end? then we have a match.
    return 0;
}


Orderable::Diff SimpleCompare::Compare3Way( CollectionInterface &l, CollectionInterface &r ) const
{
    // Ensure the sizes are the same so we don't go off the end
    if( int sd = (int)(l.size()) - (int)(r.size()) )
        return sd;
        
    // Use this object so our ordering is used.
    NodeOrdering l_ordering = GetNodeOrdering(l);
    NodeOrdering r_ordering = GetNodeOrdering(r);
    
    return LexicographicalCompare(l_ordering, r_ordering, *this);
}


SimpleCompare::NodeOrdering SimpleCompare::GetNodeOrdering( ContainerInterface &c ) const
{
    NodeOrdering ordering( *this );
    for( const TreePtrInterface &e : c )
        ordering.insert( *(TreePtr<Node>)e );
    return ordering; 
}


SimpleCompare::TreePtrOrdering SimpleCompare::GetTreePtrOrdering( ContainerInterface &c ) const
{
    TreePtrOrdering ordering( *this );
    for( const TreePtrInterface &e : c )
        ordering.insert( (TreePtr<Node>)e );
    return ordering; 
}
