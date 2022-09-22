#include "x_tree_database.hpp"
#include "../sc_relation.hpp"
#include "../sym/expression.hpp"
#include "../sym/predicate_operators.hpp"
#include "lacing.hpp"

using namespace SR;    

#define CAT_TO_INCREMENTAL false

//#define TRACE_CATEGORY_RELATION

Indexes::Indexes( const set< shared_ptr<SYM::BooleanExpression> > &clauses, bool ref_ ) :
    plan( clauses ),
    category_ordered_index( plan.lacing ),
    ref( ref_ )
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
    INDENT("@");

#ifdef TRACE_CATEGORY_RELATION
    TRACE("x_link=")(x_link)(" y_link=")(y_link)("\n");
#endif
    TreePtr<Node> x = x_link.GetChildX();
    auto cat_x = TreePtr<CategoryMinimusNode>::DynamicCast( x );
    TreePtr<Node> y = y_link.GetChildX();    
    auto cat_y = TreePtr<CategoryMinimusNode>::DynamicCast( y );

    int xi, yi;
    if( !cat_x && !cat_y )
    {
#ifdef TRACE_CATEGORY_RELATION
        TRACEC("neither is minimax\n");
        xi = lacing->GetOrdinalForNode( x );
        yi = lacing->GetOrdinalForNode( y );
#else
        // Fast path out
        Orderable::Result r = lacing->OrdinalCompare( x, y );    
	    if( r != Orderable::EQUAL )
		    return r < Orderable::EQUAL;		
		return false; //x_link < y_link; 
#endif        
    }
    else if( cat_x && cat_y )
    {
		ASSERT( false ); // not expecting this to happen
		return cat_x->GetMinimusOrdinal() < cat_y->GetMinimusOrdinal();
	}
    else if( cat_x && !cat_y )
	{
        xi = cat_x->GetMinimusOrdinal();
        yi = lacing->GetOrdinalForNode( y );
        if( xi != yi )
			return xi < yi;
		return true; // minimus is on the left
	}
	else if( !cat_x && cat_y )
    {
        xi = lacing->GetOrdinalForNode( x );
        yi = cat_y->GetMinimusOrdinal();      
        if( xi != yi )
			return xi < yi;
		return false; // minimus is on the right
    }
    else
    {
		ASSERT(false);
	}
#ifdef TRACE_CATEGORY_RELATION
    TRACEC("xi=%d yi=%d result is %s\n", xi, yi, xi<yi?"true":"false");
#endif    
    return xi - yi;   
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
 

string Indexes::CategoryMinimusNode::GetTrace() const
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
    if( ref || !CAT_TO_INCREMENTAL )
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

        if( ref || !CAT_TO_INCREMENTAL )
            category_ordered_index.insert( walk_info.xlink );

		simple_compare_ordered_index.insert( walk_info.xlink );
		
		return it;
	};
}


void Indexes::PrepareDelete( DBWalk::Actions &actions )
{
	actions.indexes_in = [&](const DBWalk::WalkInfo &walk_info) -> DBCommon::DepthFirstOrderedIt
	{
		if( !ref && CAT_TO_INCREMENTAL)
        {
            //size_t n = category_ordered_index.erase( walk_info.xlink );
            size_t n = EraseExact( category_ordered_index, walk_info.xlink );
            TRACE("Erased ")(walk_info.xlink)(" from category_ordered_index; erased %u; size now %u\n", n, category_ordered_index.size());    
        }
            
        
		return depth_first_ordered_index.end(); 
	};
}


void Indexes::PrepareInsert(DBWalk::Actions &actions)
{
	actions.indexes_in = [&](const DBWalk::WalkInfo &walk_info) -> DBCommon::DepthFirstOrderedIt
	{ 
        if( !ref && CAT_TO_INCREMENTAL )
        {
            category_ordered_index.insert( walk_info.xlink );
            TRACE("Inserting ")(walk_info.xlink)(" into category_ordered_index; size now %u\n", category_ordered_index.size());    
        }

		return depth_first_ordered_index.end(); 
	};
}


void Indexes::ExpectMatching( const Indexes &mut )
{
    ASSERT( ref );
    ASSERT( !mut.ref );
    
    //ASSERT( category_ordered_index == mut.category_ordered_index )
    //      ( DiffTrace(category_ordered_index, mut.category_ordered_index) );
}
