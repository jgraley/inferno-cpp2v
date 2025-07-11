#include "cat_relation.hpp"

#include "lacing.hpp"
#include "relation_test.hpp"

using namespace SR;    

//#define TRACE_CATEGORY_RELATION

CategoryRelation::CategoryRelation( shared_ptr<Lacing> lacing_ ) :
    lacing( lacing_ )
{
}


CategoryRelation& CategoryRelation::operator=(const CategoryRelation &other)
{
    lacing = other.lacing;
    return *this;
}


bool CategoryRelation::operator() (KeyType l_key, KeyType r_key) const
{
    return Compare3Way(l_key, r_key) < 0;
}


Orderable::Diff CategoryRelation::Compare3Way(KeyType l_key, KeyType r_key) const
{
    TreePtr<Node> l_node = l_key;
    TreePtr<Node> r_node = r_key;    

#ifdef TRACE_CATEGORY_RELATION
    INDENT("@");
    TRACE("l_xlink=")(l_xlink)(" r_xlink=")(r_xlink)("\n");
#endif
    auto l_minimax = TreePtr<MinimusNode>::DynamicCast( l_node );
    auto r_minimax = TreePtr<MinimusNode>::DynamicCast( r_node );

    int li=-1, ri=-1;
    if( !l_minimax && !r_minimax )
    {
        Orderable::Diff d = lacing->OrdinalCompare( l_node, r_node );    

#ifdef TRACE_CATEGORY_RELATION
        TRACEC("both normal: %d - %d = %d\n", li, ri, d);
#endif        
#ifdef CHECK_ORDINAL_COMPARE
        ASSERT( (d>0) == (d1>0) );
        ASSERT( (d<0) == (d1<0) );
        ASSERT( (d==0) == (d1==0) );
#endif           
        return d ? d : TreePtr<Node>::Compare3Way( l_key, r_key );       
    }
    else if( l_minimax && r_minimax )
    {
        // This case only expected during test. Fall back to XLink to
        // keep the ordering total for the benefit of the test.
        li = l_minimax->GetMinimusOrdinal();
        ri = r_minimax->GetMinimusOrdinal();
        Orderable::Diff d = li - ri;
        d = d ? d : (int)l_minimax->GetRole() - (int)r_minimax->GetRole();
        return d ? d : TreePtr<Node>::Compare3Way( l_key, r_key );
    }
    else if( l_minimax && !r_minimax )
    {
        li = l_minimax->GetMinimusOrdinal();
        ri = lacing->GetOrdinalForNode( r_node );
        Orderable::Diff d = li - ri;
        return d ? d : (int)l_minimax->GetRole(); // minimus is on the left
    }
    else if( !l_minimax && r_minimax )
    {
        li = lacing->GetOrdinalForNode( l_node );
        ri = r_minimax->GetMinimusOrdinal();      
        Orderable::Diff d = li - ri;
        return d ? d : - (int)r_minimax->GetRole(); // minimus is on the right
    }
    else
    {
        ASSERTFAIL();
    }
}


void CategoryRelation::Test( const vector<KeyType> &keys )
{
    using namespace std::placeholders;

    TestRelationProperties<KeyType>( keys,
                                     true,
                                     "CategoryRelation",
                                     bind(&CategoryRelation::Compare3Way, *this, _1, _2), 
    [&](KeyType l, KeyType r) -> bool
    { 
        return l==r; 
    }, 
    [&](KeyType x, int randval) -> KeyType
    {
        // We wish to inject "special" nodes which will be minimax 
        // xlinks that we attempt to generate from the already-selected xlink
        // and use the provided random value to choose which minimax to gen.
        // TODO maybe we could just calculate i directly from randval and the lacing size?
        
        // Consult the lacing for lacing indices
        auto rl = lacing->TryGetRangeListForCategory( x );      
        
        // Only nodes seen in cat clauses during planning will succeed
        if( rl.empty() )
            return x; // returning x indicates fail 
            
        // Choose random minimax index
        pair<int, int> r = ToVector(rl).at((randval/2)%rl.size()); // a random range from the list of them
        int i = randval%2 ? r.first : r.second; // begin or end of range, randomly
        
        // Make minimus node (this relation always uses minimus because half-open
        TreePtr<Node> node = MakeTreeNode<SR::CategoryRelation::MinimusNode>(i);
        return node;      
    } );
}


CategoryRelation::MinimusNode::MinimusNode( int lacing_ordinal_ ) :
    lacing_ordinal( lacing_ordinal_ )
{
}
    

CategoryRelation::MinimusNode::MinimusNode() :
    lacing_ordinal( 0 )
{
}
    

int CategoryRelation::MinimusNode::GetMinimusOrdinal() const
{
    return lacing_ordinal;
}
 

string CategoryRelation::MinimusNode::GetName() const
{
    return SSPrintf("Cat::Minimus(%d)", lacing_ordinal);
}
