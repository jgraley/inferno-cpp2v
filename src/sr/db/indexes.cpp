#include "x_tree_database.hpp"
#include "../sc_relation.hpp"
#include "../sym/expression.hpp"
#include "../sym/predicate_operators.hpp"
#include "lacing.hpp"

using namespace SR;    

//#define TRACE_CATEGORY_RELATION

Indexes::Indexes( const set< shared_ptr<SYM::BooleanExpression> > &clauses, bool ref_ ) :
    plan( clauses ),
    category_ordered_index( plan.lacing ),
    ref( ref_ ),
    incremental( ref ? false : ReadArgs::use_incremental )
{ 
}


Indexes::Plan::Plan( const set< shared_ptr<SYM::BooleanExpression> > &clauses ) :
    lacing( make_shared<Lacing>() )
{
    // Warning: there are a few places that declare an empty x_tree_db
    if( clauses.empty() )
        return;
    
    // Extract all the non-final archetypes from the IsInCategoryOperator nodes 
    // into a set so that they are uniqued by SimpleCompare equality. These
    // are the categories.
    Lacing::CategorySet categories;
    for( shared_ptr<SYM::BooleanExpression> clause : clauses )
    {
        clause->ForDepthFirstWalk( [&](const SYM::Expression *expr)
        {
            if( auto ko_expr = dynamic_cast<const SYM::IsInCategoryOperator *>(expr) )
            { 
                categories.insert( ko_expr->GetArchetypeNode() );
            }
        } );
    }

    lacing->Build(categories);
}


Indexes::CategoryRelation::CategoryRelation()
{
}


Indexes::CategoryRelation::CategoryRelation( shared_ptr<Lacing> lacing_ ) :
    lacing( lacing_ )
{
}


Indexes::CategoryRelation& Indexes::CategoryRelation::operator=(const CategoryRelation &other)
{
    lacing = other.lacing;
    return *this;
}


bool Indexes::CategoryRelation::operator() (const XLink& x_link, const XLink& y_link) const
{
#ifdef TRACE_CATEGORY_RELATION
    INDENT("@");
    TRACE("x_link=")(x_link)(" y_link=")(y_link)("\n");
#endif
    TreePtr<Node> x = x_link.GetChildX();
    auto x_minimus = TreePtr<CategoryMinimusNode>::DynamicCast( x );
    TreePtr<Node> y = y_link.GetChildX();    
    auto y_minimus = TreePtr<CategoryMinimusNode>::DynamicCast( y );

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


Indexes::CategoryMinimusNode::CategoryMinimusNode( int lacing_ordinal_ ) :
    lacing_ordinal( lacing_ordinal_ )
{
}
    

Indexes::CategoryMinimusNode::CategoryMinimusNode() :
    lacing_ordinal( 0 )
{
}
    

int Indexes::CategoryMinimusNode::GetMinimusOrdinal() const
{
    return lacing_ordinal;
}
 

string Indexes::CategoryMinimusNode::GetName() const
{
    return GetTypeName() + SSPrintf("(%d)", lacing_ordinal);
}


const Lacing *Indexes::GetLacing() const
{
    return plan.lacing.get();
}


void Indexes::MonolithicClear()
{
    depth_first_ordered_index.clear();
    if( !incremental )
        category_ordered_index.clear();
    simple_compare_ordered_index.clear();
}


void Indexes::PrepareMonolithicBuild(DBWalk::Actions &actions)
{
	actions.indexes_in = [&](const DBWalk::WalkInfo &walk_info) -> DBCommon::DepthFirstOrderedIt
	{
		// ----------------- Update indices
		depth_first_ordered_index.push_back( walk_info.xlink );
		DBCommon::DepthFirstOrderedIt it = depth_first_ordered_index.end();
		--it; // I know this is OK because we just pushed to depth_first_ordered_index

        if( !incremental )
            category_ordered_index.insert( walk_info.xlink );

		simple_compare_ordered_index.insert( walk_info.xlink );
		
		return it;
	};
}


void Indexes::PrepareDelete( DBWalk::Actions &actions )
{
	actions.indexes_in = [&](const DBWalk::WalkInfo &walk_info) -> DBCommon::DepthFirstOrderedIt
	{
		if( incremental )
        {
            EraseSolo( category_ordered_index, walk_info.xlink );
            TRACEC("Erased ")(walk_info.xlink)(" from category_ordered_index; size now %u\n", category_ordered_index.size());    
        }            
        
        // Would be used by xlink_table but that's not incremental yet
		return depth_first_ordered_index.end(); 
	};
}


void Indexes::PrepareInsert(DBWalk::Actions &actions)
{
	actions.indexes_in = [&](const DBWalk::WalkInfo &walk_info) -> DBCommon::DepthFirstOrderedIt
	{ 
        if( incremental )
        {
            category_ordered_index.insert( walk_info.xlink );
            TRACEC("Inserted ")(walk_info.xlink)(" into category_ordered_index; size now %u\n", category_ordered_index.size());    
        }

        // Would be used by xlink_table but that's not incremental yet
		return depth_first_ordered_index.end();
	};
}


void Indexes::Dump() const
{
    TRACE("category_ordered_index:\n")(category_ordered_index)("\n");
}


void Indexes::ExpectMatching( const Indexes &mut )
{
    ASSERT( ref );
    ASSERT( !mut.ref );
    
    ASSERT( category_ordered_index == mut.category_ordered_index )
          ( DiffTrace(category_ordered_index, mut.category_ordered_index) )
          ( mut.category_ordered_index );
}
