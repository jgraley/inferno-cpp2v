#include "depth_agent.hpp"
#include "../search_replace.hpp" 
#include "../scr_engine.hpp" 
#include "../conjecture.hpp" 
#include "link.hpp"
#include "db/x_tree_database.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/symbol_operators.hpp"
#include "sym/lazy_eval.hpp"

using namespace SR;
using namespace SYM;

//---------------------------------- DepthAgent ------------------------------------    

shared_ptr<PatternQuery> DepthAgent::GetPatternQuery() const
{
	ASSERT( terminus )("Terminus has not been specified for ")(*this);
    auto pq = make_shared<PatternQuery>();
    
    pq->RegisterDecision( false ); // Exclusive, please.
    pq->RegisterNormalLink( PatternLink(this, &terminus) );
    
    // Allow subclasses to further restrict
    PatternQueryRestrictions( pq );
    
    return pq;
}


Agent::ReplacePatchPtr DepthAgent::GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                                   PatternLink me_plink, 
                                                   XLink key_xlink ) 
{
    INDENT("#");
    
    // Recurse at the terminus
    PatternLink terminus_plink(this, &terminus);
    list<Agent::ReplacePatchPtr> child_commands;
    child_commands.push_back( terminus_plink.GetChildAgent()->GenReplaceLayout(kit, terminus_plink) );

    // Make a tree zone for the nodes we covered
    XLink terminus_key_xlink = my_scr_engine->GetReplaceKey( terminus_plink );
    ASSERT(terminus_key_xlink);// this could mean replace is being attempted on a DepthAgent in an abnormal context
    auto new_zone = TreeZone( key_xlink, vector<XLink>{terminus_key_xlink} );   

    return make_shared<TreePatch>( new_zone, move(child_commands) );
}


Graphable::NodeBlock DepthAgent::GetGraphBlockInfo() const
{
    NodeBlock block;
    block.bold = true;
    block.shape = "square";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetTerminus()->get()), 
              list<string>{},
              list<string>{},
              phase,
              GetTerminus() );
    block.item_blocks = { { "terminus", 
                           "", 
                           true,
                           { link } } };
    return block;
}

//---------------------------------- ChildAgent ------------------------------------    

SYM::Lazy<SYM::BooleanExpression> ChildAgent::SymbolicNormalLinkedQueryPRed() const
{
    PatternLink terminus_plink(this, &terminus);
    return MakeLazy<ParentOperator>( MakeLazy<SymbolVariable>(terminus_plink) ) ==
           MakeLazy<SymbolVariable>(keyer_plink);
}


Graphable::NodeBlock ChildAgent::GetGraphBlockInfo() const
{
    // The Child node appears as a small circle with the text #==1 in it. The terminus block emerges from the
    // right of the circle. 1 implies the tendancy to match exactly one level. See #256.
    NodeBlock block = DepthAgent::GetGraphBlockInfo();
    block.title = "Child";
    block.symbol = "#=1"; // TODO this can be generated when Stuff nodes are generalised, see #256
    return block;
}

//---------------------------------- StuffAgent ------------------------------------    

void StuffAgent::PatternQueryRestrictions( shared_ptr<PatternQuery> pq ) const
{
    if( recurse_restriction )
        pq->RegisterMultiplicityLink( PatternLink(this, &recurse_restriction) );
}                                                      


SYM::Lazy<SYM::BooleanExpression> StuffAgent::SymbolicNormalLinkedQueryPRed() const
{
    PatternLink terminus_plink(this, &terminus);
    auto expr = MakeLazy<BooleanConstant>(true);
    
    expr &= MakeLazy<SymbolVariable>(terminus_plink) >= 
            MakeLazy<SymbolVariable>(keyer_plink);
            
    expr &= MakeLazy<SymbolVariable>(terminus_plink) <= 
            MakeLazy<LastDescendantOperator>( MakeLazy<SymbolVariable>(keyer_plink) );
    
    return expr;
}


void StuffAgent::RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                           const SolutionMap *hypothesis_links,
                                           const XTreeDatabase *x_tree_db ) const
{
    INDENT("#");
    ASSERTTHIS();
    ASSERT( terminus )("Stuff node without terminus, seems pointless, if there's a reason for it remove this assert");

    if( !recurse_restriction )
        return;

    XLink keyer_xlink = hypothesis_links->at(keyer_plink);
    TRACE("SearchContainer agent ")(*this)(" terminus pattern is ")(*(terminus))(" at ")(keyer_xlink)("\n");
    
    PatternLink terminus_plink(this, &terminus);
    XLink req_terminus_xlink = hypothesis_links->at(terminus_plink); 
    
    XLink xlink = req_terminus_xlink;
    auto xpr_ss = MakeTreeNode<SubSequence>();
    while(xlink != keyer_xlink)
    {       
        xlink = x_tree_db->TryGetParentXLink(xlink);
        
        // Putting this here excludes the terminus, as required
        TRACEC("Move to parent ")(xlink)("\n");
        xpr_ss->elts.push_front( xlink );      
    }
    
    XLink xpr_ss_link = XLink::CreateDistinct( xpr_ss ); // Only used in after-pass
    query.RegisterMultiplicityLink( PatternLink(this, &recurse_restriction), xpr_ss_link ); // Links into X    
}
    
    
Graphable::NodeBlock StuffAgent::GetGraphBlockInfo() const
{
    // The Stuff node appears as a small square with a # character inside it. The terminus block emerges from the
    // right of the circle. # is chosen (as is the name Stuff) for its addr_bounding_role to * because
    // the nodes are both able to wildcard multiple nodes in the input tree.
    NodeBlock block = DepthAgent::GetGraphBlockInfo();
    block.title = "Stuff"; 
    block.symbol = "#"; 
    if( recurse_restriction )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(recurse_restriction.get()), 
                  list<string>{},
                  list<string>{},
                  phase,
                  &recurse_restriction );
        block.item_blocks.push_back( { "recurse_restriction", 
                                      "", 
                                      false,
                                      { link } } );
    }
    return block;
}
