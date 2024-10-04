#include "star_agent.hpp"
#include "../subcontainers.hpp" 
#include "../search_replace.hpp" 
#include "../scr_engine.hpp" 
#include "link.hpp"
#include "sym/symbol_operators.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/result.hpp"
#include "up/commands.hpp"

using namespace SR;
using namespace SYM;

shared_ptr<PatternQuery> StarAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    if( *GetRestriction() )
    {
        pq->RegisterMultiplicityLink( PatternLink(this, GetRestriction()) );
    }

    return pq;
}


SYM::Lazy<SYM::BooleanExpression> StarAgent::SymbolicNormalLinkedQuery() const
{
    if( ShouldGenerateCategoryClause() )
    {
        auto keyer_expr = MakeLazy<SymbolVariable>(keyer_plink);
        return MakeLazy<IsSubcontainerInCategoryOperator>(GetArchetypeNode(), keyer_expr);
    }
    else
    {
        return MakeLazy<BooleanConstant>(true);
    }    
}


void StarAgent::RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                          const SolutionMap *hypothesis_links,
                                          const XTreeDatabase *x_tree_db ) const
{ 
    // This agent has no normal links, so just do this to populate query
    XLink keyer_xlink = hypothesis_links->at(keyer_plink);
    INDENT("*");
    ASSERT(keyer_xlink);
                
    auto x_ci = dynamic_cast<ContainerInterface *>(keyer_xlink.GetChildX().get());
    auto x_sc = TreePtr<SubContainer>::DynamicCast(keyer_xlink.GetChildX());

    // Nodes must be a SubContainer, since * matches multiple things
    if( !( x_sc && x_ci ) )
        throw NotASubcontainerMismatch();
    
    // Check pre-restriction
    TRACE("StarAgent pre-res\n");
    for( const TreePtrInterface &xe : *x_ci )
    {
        if( !IsPreRestrictionMatch( (TreePtr<Node>)xe ) )
            throw PreRestrictionMismatch();
    }
     
    if( *GetRestriction() )
    {
        TRACE("StarAgent pattern, size is %d\n", x_ci->size());
        // Apply pattern restriction - will be at least as strict as pre-restriction
        query.RegisterMultiplicityLink( PatternLink(this, GetRestriction()), keyer_xlink ); // Links into X
    }
}


Agent::FreeZoneExprPtr StarAgent::GenFreeZoneExprImpl( const ReplaceKit &kit, 
                                                  PatternLink me_plink, 
                                                  XLink key_xlink ) 
{
    INDENT("*");
    ASSERT( key_xlink );
    TreePtr<Node> key_node = key_xlink.GetChildX();
    
    // Key needs to implement ContainerInterface
    ContainerInterface *key_container = dynamic_cast<ContainerInterface *>(key_node.get());
    ASSERT( key_container )("Star node ")(*this)(" keyed to ")(*key_node)(" which should implement ContainerInterface");  
    
    // Make a subcontainer of the corresponding type
    TreePtr<SubContainer> dest;
    if( dynamic_cast<SequenceInterface *>(key_container) )
        dest = MakeTreeNode<SubSequence>();
    else if( dynamic_cast<CollectionInterface *>(key_container) )
        dest = MakeTreeNode<SubCollection>();
    else
        ASSERT(0)("Please add new kind of container");
    
    list<Agent::FreeZoneExprPtr> child_commands;
    FreeZone zone = FreeZone::CreateSubtree(dest);

    TRACE("Walking container length %d\n", key_container->size() );
    ContainerInterface *dest_container = dynamic_cast<ContainerInterface *>(dest.get());
    for( const TreePtrInterface &key_elt : *key_container )
    {
        ContainerInterface::iterator dest_it = dest_container->insert( ContainerTerminus::MakePlaceholder() );
        zone.AddTerminus( make_shared<ContainerTerminus>(dest_container, dest_it) );    

        TreeZone child_zone = TreeZone::CreateSubtree( kit.x_tree_db, XLink(key_node, &key_elt) );
	    child_commands.push_back( make_shared<PopulateTreeZoneOperator>(move(child_zone)) );
    }

    return make_shared<PopulateFreeZoneOperator>( move(zone), move(child_commands) );    
}


Graphable::Block StarAgent::GetGraphBlockInfo() const
{
	// The Star node appears as a small circle with a * character inside it. * is chosen for its role in 
	// filename wildcarding, which is semantically equiviant only when used in a Sequence.
    Block block;
	block.bold = true;
	block.title = "Star";
	block.symbol = "*";
	block.shape = "circle";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    if( *GetRestriction() )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetRestriction()->get()), 
                                                  list<string>{},
                                                  list<string>{},
                                                  phase,
                                                  GetRestriction() );
        block.sub_blocks.push_back( { "restriction", 
                                      "", 
                                      false,
                                      { link } } );
    }
    return block;
}
 

unique_ptr<BooleanResult> StarAgent::IsSubcontainerInCategoryOperator::Evaluate( const EvalKit &kit,
                                                                                 list<unique_ptr<SymbolicResult>> &&op_results ) const
{
    ASSERT( op_results.size()==1 );        
    unique_ptr<SymbolicResult> ra = OnlyElementOf(move(op_results));

    if( !ra->IsDefinedAndUnique() )
        return make_unique<BooleanResult>( false );

    auto x_ci = dynamic_cast<ContainerInterface *>(ra->GetOnlyXLink().GetChildX().get());
    auto x_sc = TreePtr<SubContainer>::DynamicCast(ra->GetOnlyXLink().GetChildX());

    // Nodes must be a SubContainer, since * matches multiple things
    if( !( x_sc && x_ci ) )
        return make_unique<BooleanResult>( false );
    
    // Check pre-restriction
    bool matches = true;
    for( const TreePtrInterface &xe : *x_ci )
        matches = matches & archetype_node->IsSubcategory( *(TreePtr<Node>)xe );            

    return make_unique<BooleanResult>( matches );
}                                   

         
string StarAgent::IsSubcontainerInCategoryOperator::RenderNF() const
{
    return "SubcontainerCAT<" + archetype_node->GetTypeName() + ">(" + a->Render() + ")"; 
}

