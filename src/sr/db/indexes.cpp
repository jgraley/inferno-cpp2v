#include "x_tree_database.hpp"
#include "../sc_relation.hpp"
#include "../sym/expression.hpp"
#include "../sym/predicate_operators.hpp"
#include "lacing.hpp"

using namespace SR;    


//#define CAT_TO_INCREMENTAL


Indexes::Indexes( const set< shared_ptr<SYM::BooleanExpression> > &clauses ) :
    plan( clauses ),
    category_ordered_index( plan.lacing )
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
    TreePtr<Node> x = x_link.GetChildX();
    auto cat_x = TreePtr<CategoryMinimaxNode>::DynamicCast( x );
    TreePtr<Node> y = y_link.GetChildX();    
    auto cat_y = TreePtr<CategoryMinimaxNode>::DynamicCast( y );

    if( !cat_x && !cat_y )
        return lacing->IsOrdinalLess( x, y );    
   
    int xi, yi;
    if( cat_x )
        xi = cat_x->GetLacingOrdinal();
    else
        xi = lacing->GetOrdinalForNode( x );
    if( cat_y )
        yi = cat_y->GetLacingOrdinal();
    else
        yi = lacing->GetOrdinalForNode( y );
    return xi < yi;   
}


Indexes::CategoryMinimaxNode::CategoryMinimaxNode( int lacing_ordinal_ ) :
    lacing_ordinal( lacing_ordinal_ )
{
}
    

Indexes::CategoryMinimaxNode::CategoryMinimaxNode() :
    lacing_ordinal( 0 )
{
}
    

int Indexes::CategoryMinimaxNode::GetLacingOrdinal() const
{
    return lacing_ordinal;
}
 

string Indexes::CategoryMinimaxNode::GetTrace() const
{
    return GetTypeName() + SSPrintf("(%d)", lacing_ordinal);
}


const Lacing *Indexes::GetLacing() const
{
    return plan.lacing.get();
}


void Indexes::ClearMonolithic()
{
    depth_first_ordered_index.clear();
#ifndef CAT_TO_INCREMENTAL
    category_ordered_index.clear();
#endif
    simple_compare_ordered_index.clear();
}


void Indexes::PrepareBuildMonolithic(DBWalk::Actions &actions)
{
	actions.indexes_in = [&](const DBWalk::WalkInfo &walk_info) -> DBCommon::DepthFirstOrderedIt
	{
		// ----------------- Update indices
		depth_first_ordered_index.push_back( walk_info.xlink );
		DBCommon::DepthFirstOrderedIt it = depth_first_ordered_index.end();
		--it; // I know this is OK because we just pushed to depth_first_ordered_index

		category_ordered_index.insert( walk_info.xlink );

		simple_compare_ordered_index.insert( walk_info.xlink );
		
		return it;
	};
}


void Indexes::PrepareDelete( DBWalk::Actions &actions )
{
	actions.indexes_in = [&](const DBWalk::WalkInfo &walk_info) -> DBCommon::DepthFirstOrderedIt
	{
#ifdef CAT_TO_INCREMENTAL
		category_ordered_index.delete( walk_info.xlink );
#endif
        
		return depth_first_ordered_index.end(); // no valid it since we deleted
	};
}


void Indexes::PrepareInsert(DBWalk::Actions &actions)
{
}

