#include "lacing.hpp"

#include "cat_relation.hpp"

using namespace SR;    

//#define TRACE_CATEGORY_RELATION

CategoryRelation::CategoryRelation()
{
}


CategoryRelation::CategoryRelation( shared_ptr<Lacing> lacing_ ) :
    lacing( lacing_ )
{
}


CategoryRelation& CategoryRelation::operator=(const CategoryRelation &other)
{
    lacing = other.lacing;
    return *this;
}


bool CategoryRelation::operator() (const XLink& x_link, const XLink& y_link) const
{
#ifdef TRACE_CATEGORY_RELATION
    INDENT("@");
    TRACE("x_link=")(x_link)(" y_link=")(y_link)("\n");
#endif
    TreePtr<Node> x = x_link.GetChildX();
    auto x_minimus = TreePtr<MinimusNode>::DynamicCast( x );
    TreePtr<Node> y = y_link.GetChildX();    
    auto y_minimus = TreePtr<MinimusNode>::DynamicCast( y );

    int xi=-1, yi=-1;
    if( !x_minimus && !y_minimus )
    {
        xi = lacing->GetOrdinalForNode( x );
        yi = lacing->GetOrdinalForNode( y );
        Orderable::Result ro = xi - yi;
        // Fast path out
        //Orderable::Result r = lacing->OrdinalCompare( x, y );    
#ifdef TRACE_CATEGORY_RELATION
        TRACEC("both normal: %d - %d = %d\n", xi, yi, r);
#endif        
#ifdef CHECK_ORDINAL_COMPARE
        ASSERT( (r>0) == (ro>0) );
        ASSERT( (r<0) == (ro<0) );
        ASSERT( (r==0) == (ro==0) );
#endif
   	    if( ro != Orderable::EQUAL )
		    return ro < Orderable::EQUAL;	
		    
        bool res = x_link.operator<(y_link);
#ifdef TRACE_CATEGORY_RELATION
        TRACEC("strong cat order: ")(res)("\n");
#endif
        return res; 
    }
    else if( x_minimus && y_minimus )
    {
		ASSERT( false ); // not expecting this to happen
		return x_minimus->GetMinimusOrdinal() < y_minimus->GetMinimusOrdinal();
	}
    else if( x_minimus && !y_minimus )
	{
        xi = x_minimus->GetMinimusOrdinal();
        yi = lacing->GetOrdinalForNode( y );
        bool res;
        if( xi != yi )
			res = xi < yi;
		else
		    res = true; // minimus is on the left
#ifdef TRACE_CATEGORY_RELATION
        TRACEC("left is minimus: %d♭ - %d = %d result: ", xi, yi, xi-yi)(res)("\n");
#endif
		return res;
	}
	else if( !x_minimus && y_minimus )
    {
        xi = lacing->GetOrdinalForNode( x );
        yi = y_minimus->GetMinimusOrdinal();      
        bool res;
        if( xi != yi )
			res = xi < yi;
		else
		    res = false; // minimus is on the right
#ifdef TRACE_CATEGORY_RELATION
        TRACEC("right is minimus: %d - %d♭ = %d result: ", xi, yi, xi-yi)(res)("\n");
#endif
		return res;
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
