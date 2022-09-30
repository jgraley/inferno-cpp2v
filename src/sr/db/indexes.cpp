#include "x_tree_database.hpp"
#include "../sc_relation.hpp"
#include "../sym/expression.hpp"
#include "../sym/predicate_operators.hpp"
#include "lacing.hpp"

#include "indexes.hpp"

using namespace SR;    

//#define TRACE_CATEGORY_RELATION

Indexes::Indexes( const set< shared_ptr<SYM::BooleanExpression> > &clauses, bool ref_ ) :
    plan( clauses ),
    category_ordered_index( plan.lacing ),
    ref( ref_ ),
    use_incremental( ref ? false : ReadArgs::use_incremental )
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


const Lacing *Indexes::GetLacing() const
{
    return plan.lacing.get();
}


void Indexes::MonolithicClear()
{
    depth_first_ordered_index.clear();
    // category_ordered_index.clear(); now incremental
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

        // category_ordered_index.insert( walk_info.xlink ); now incremental

		simple_compare_ordered_index.insert( walk_info.xlink );
		
		return it;
	};
}


void Indexes::PrepareDelete( DBWalk::Actions &actions )
{
	actions.indexes_in = [&](const DBWalk::WalkInfo &walk_info) -> DBCommon::DepthFirstOrderedIt
	{
		EraseSolo( category_ordered_index, walk_info.xlink );
        TRACEC("Erased ")(walk_info.xlink)(" from category_ordered_index; size now %u\n", category_ordered_index.size());    
        
        // Would be used by xlink_table but that's not incremental yet
		return depth_first_ordered_index.end(); 
	};
}


void Indexes::PrepareInsert(DBWalk::Actions &actions)
{
	actions.indexes_in = [&](const DBWalk::WalkInfo &walk_info) -> DBCommon::DepthFirstOrderedIt
	{ 
        category_ordered_index.insert( walk_info.xlink );
        TRACEC("Inserted ")(walk_info.xlink)(" into category_ordered_index; size now %u\n", category_ordered_index.size());    

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
