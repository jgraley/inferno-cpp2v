#include "lacing.hpp"

#include "cat_relation.hpp"

using namespace SR;    

//#define TRACE_CATEGORY_RELATION

//CategoryRelation::CategoryRelation()
//{
//}


CategoryRelation::CategoryRelation( shared_ptr<Lacing> lacing_ ) :
    lacing( lacing_ )
{
}


CategoryRelation& CategoryRelation::operator=(const CategoryRelation &other)
{
    lacing = other.lacing;
    return *this;
}


bool CategoryRelation::operator() (const XLink& l_xlink, const XLink& r_xlink) const
{
	return Compare(l_xlink, r_xlink) < 0;
}


Orderable::Diff CategoryRelation::Compare(const XLink& l_xlink, const XLink& r_xlink) const
{
#ifdef TRACE_CATEGORY_RELATION
    INDENT("@");
    TRACE("l_xlink=")(l_xlink)(" r_xlink=")(r_xlink)("\n");
#endif
    TreePtr<Node> l_node = l_xlink.GetChildX();
    auto l_minimus = TreePtr<MinimusNode>::DynamicCast( l_node );
    TreePtr<Node> r_node = r_xlink.GetChildX();    
    auto r_minimus = TreePtr<MinimusNode>::DynamicCast( r_node );

    int li=-1, ri=-1;
    if( !l_minimus && !r_minimus )
    {
        li = lacing->GetOrdinalForNode( l_node );
        ri = lacing->GetOrdinalForNode( r_node );
        Orderable::Diff d = li - ri;
        // Fast path out - now super slow TODO resolve, see #642
        //Orderable::Diff d1 = lacing->OrdinalCompare( x, y );    
#ifdef TRACE_CATEGORY_RELATION
        TRACEC("both normal: %d - %d = %d\n", li, ri, d);
#endif        
#ifdef CHECK_ORDINAL_COMPARE
        ASSERT( (d>0) == (d1>0) );
        ASSERT( (d<0) == (d1<0) );
        ASSERT( (d==0) == (d1==0) );
#endif
   	    if( d )
		    return d;	
		    
        return XLink::Compare(l_xlink, r_xlink);
    }
    else if( l_minimus && r_minimus )
    {
		ASSERT( false ); // not expecting this to happen
		li = l_minimus->GetMinimusOrdinal();
		ri = r_minimus->GetMinimusOrdinal();
		return li - ri;
	}
    else if( l_minimus && !r_minimus )
	{
        li = l_minimus->GetMinimusOrdinal();
        ri = lacing->GetOrdinalForNode( r_node );
        return (li*2-1) - (ri*2); // minimus is on the left
	}
	else if( !l_minimus && r_minimus )
    {
        li = lacing->GetOrdinalForNode( l_node );
        ri = r_minimus->GetMinimusOrdinal();      
        return (li*2) - (ri*2-1); // minimus is on the right
    }
    else
    {
		ASSERTFAIL();
	}
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
    return GetTypeName() + SSPrintf("(%d)", lacing_ordinal);
}
