#include "star_agent.hpp"
#include "../subcontainers.hpp" 
#include "../search_replace.hpp" 
#include "link.hpp"
#include "sym/primary_expressions.hpp"

using namespace SR;
using namespace SYM;

shared_ptr<PatternQuery> StarAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
    if( *GetRestriction() )
    {
        pq->RegisterMultiplicityLink( PatternLink(this, GetRestriction()) );
    }

    return pq;
}


// NOTE this is a DecidedCompare() not DecidedCompareImpl() so some of the AgentCommon 
// stuff has to be done explicitly in here.
void StarAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                     XLink keyer_xlink ) const
{
    INDENT("*");
    ASSERT(keyer_xlink);
                
    auto x_ci = dynamic_cast<ContainerInterface *>(keyer_xlink.GetChildX().get());
    auto x_sc = TreePtr<SubContainer>::DynamicCast(keyer_xlink.GetChildX());

    // Nodes passed to StarAgent::RunDecidedQueryImpl() must be a SubContainer, since * matches multiple things
    if( !( x_sc && x_ci ) )
        throw NotASubcontainerMismatch();
    
    // Check pre-restriction
    TRACE("StarAgent pre-res\n");
    FOREACH( const TreePtrInterface &xe, *x_ci )
    {
        if( !IsLocalMatch( ((TreePtr<Node>)xe).get() ) )
            throw PreRestrictionMismatch();
    }
     
    if( *GetRestriction() )
    {
        TRACE("StarAgent pattern, size is %d\n", x_ci->size());
        // Apply pattern restriction - will be at least as strict as pre-restriction
        query.RegisterMultiplicityLink( PatternLink(this, GetRestriction()), keyer_xlink ); // Links into X
    }
}                       


bool StarAgent::ImplHasNLQ() const
{
    return true;
}


SYM::Over<SYM::BooleanExpression> StarAgent::SymbolicNormalLinkedQueryImpl() const
{
    auto keyer_expr = MakeOver<SymbolVariable>(keyer_plink);
    return MakeOver<SubcontainerKindOfOperator>(GetArchetypeNode(), keyer_expr);
}


void StarAgent::RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                          const SolutionMap *hypothesis_links,
                                          const TheKnowledge *knowledge ) const
{ 
    // This agent has no normal links, so just do this to populate query
    XLink keyer_xlink = hypothesis_links->at(keyer_plink);
    RunDecidedQueryImpl( query, keyer_xlink ); 
}


TreePtr<Node> StarAgent::BuildReplaceImpl( PatternLink me_plink, 
                                           TreePtr<Node> key_node ) 
{
    INDENT("*");
    ASSERT( key_node );
    ContainerInterface *psc = dynamic_cast<ContainerInterface *>(key_node.get());
    ASSERT( psc )("Star node ")(*this)(" keyed to ")(*key_node)(" which should implement ContainerInterface");  
    TRACE("Walking container length %d\n", psc->size() );
    
    if( auto sc = dynamic_cast<SubContainer *>(key_node.get()) )    
        TRACE("SubContainer found ")(sc->GetContentsTrace())("\n");
        
    TreePtr<SubContainer> dest;
    ContainerInterface *dest_container;
    if( dynamic_cast<SequenceInterface *>(key_node.get()) )
        dest = TreePtr<SubSequence>(new SubSequence);
    else if( dynamic_cast<CollectionInterface *>(key_node.get()) )
        dest = TreePtr<SubCollection>(new SubCollection);
    else
        ASSERT(0)("Please add new kind of Star");
    
    dest_container = dynamic_cast<ContainerInterface *>(dest.get());
    FOREACH( const TreePtrInterface &pp, *psc )
    {
        TRACE("Building ")(pp)("\n");
        TreePtr<Node> nn = DuplicateSubtree( (TreePtr<Node>)pp );
        dest_container->insert( nn );
    }
    
    return dest;
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
 

shared_ptr<BooleanResult> StarAgent::SubcontainerKindOfOperator::Evaluate( const EvalKit &kit,
                                                                           const list<shared_ptr<SymbolResult>> &op_results ) const
{
    ASSERT( op_results.size()==1 );        
    shared_ptr<SymbolResult> ra = OnlyElementOf(op_results);

    if( ra->cat == SymbolResult::UNDEFINED )
        return make_shared<BooleanResult>( BooleanResult::UNDEFINED );

    auto x_ci = dynamic_cast<ContainerInterface *>(ra->xlink.GetChildX().get());
    auto x_sc = TreePtr<SubContainer>::DynamicCast(ra->xlink.GetChildX());

    // Nodes passed to StarAgent::RunDecidedQueryImpl() must be a SubContainer, since * matches multiple things
    if( !( x_sc && x_ci ) )
        return make_shared<BooleanResult>( BooleanResult::FALSE );
    
    // Check pre-restriction
    bool matches = true;
    FOREACH( const TreePtrInterface &xe, *x_ci )
        matches = matches & archetype_node->IsLocalMatch( ((TreePtr<Node>)xe).get() );            

    return make_shared<BooleanResult>( matches ? BooleanResult::TRUE : BooleanResult::FALSE );
}                                   

         
string StarAgent::SubcontainerKindOfOperator::Render() const
{
    return "SubcontainerKindOf<" + archetype_node->GetTypeName() + ">(" + a->Render() + ")"; 
}

