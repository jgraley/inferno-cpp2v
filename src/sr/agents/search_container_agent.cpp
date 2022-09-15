#include "search_container_agent.hpp"
#include "../search_replace.hpp" 
#include "../scr_engine.hpp" 
#include "../conjecture.hpp" 
#include "link.hpp"
#include "x_tree_database.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/symbol_operators.hpp"
#include "sym/overloads.hpp"

using namespace SR;
using namespace SYM;

//---------------------------------- SearchContainerAgent ------------------------------------    

shared_ptr<PatternQuery> SearchContainerAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    
	pq->RegisterDecision( false ); // Exclusive, please.
	pq->RegisterNormalLink( PatternLink(this, &terminus) );
    
    // Allow subclasses to further restrict
    PatternQueryRestrictions( pq );
    
    return pq;
}


TreePtr<Node> SearchContainerAgent::BuildReplaceImpl( PatternLink me_plink, 
                                                      XLink key_xlink ) 
{
    INDENT("#");
    XLink terminus_key_xlink = my_scr_engine->GetReplaceKey( PatternLink(this, &terminus) );
    ASSERT(terminus_key_xlink);// this could mean replace is being attempted on a SearchContainerAgent in an abnormal context
    TRACE( "Stuff node: Duplicating at terminus first: keynode=")
         (*(terminus))(", term=")(terminus_key_xlink)("\n");
         
    PatternLink terminus_plink(this, &terminus);
    TreePtr<Node> new_terminus_subtree = terminus_plink.GetChildAgent()->BuildReplace(terminus_plink);
    TRACE( "Stuff node: Substituting stuff");
    int c = 0;
    TreePtr<Node> t = DuplicateSubtree(key_xlink, terminus_key_xlink, new_terminus_subtree, &c);   
    ASSERT( c==1 )(terminus_key_xlink); 
    return t;
}


Graphable::Block SearchContainerAgent::GetGraphBlockInfo() const
{
    Block block;
	block.bold = true;
    block.shape = "square";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(GetTerminus()->get()), 
              list<string>{},
              list<string>{},
              phase,
              GetTerminus() );
    block.sub_blocks = { { "terminus", 
                           "", 
                           true,
                           { link } } };
    return block;
}

//---------------------------------- AnyNode ------------------------------------    

shared_ptr<ContainerInterface> AnyNodeAgent::GetContainerInterface( XLink keyer_xlink ) const
{ 
    // Note: does not do the flatten every time - instead, the FlattenNode object's range is presented
    // to the Conjecture object, which increments it only when trying alternative choice
    return make_shared<FlattenNode>( keyer_xlink.GetChildX() );
}


SYM::Over<SYM::BooleanExpression> AnyNodeAgent::SymbolicNormalLinkedQueryPRed() const
{
    PatternLink terminus_plink(this, &terminus);
    return MakeOver<ParentOperator>( MakeOver<SymbolVariable>(terminus_plink) ) ==
           MakeOver<SymbolVariable>(keyer_plink);
}


Graphable::Block AnyNodeAgent::GetGraphBlockInfo() const
{
	// The AnyNode node appears as a small circle with the text #==1 in it. The terminus block emerges from the
	// right of the circle. 1 implies the tendancy to match exactly one level. See #256.
    Block block = SearchContainerAgent::GetGraphBlockInfo();
    block.title = "AnyNode";
	block.symbol = "#=1"; // TODO this can be generated when Stuff nodes are generalised, see #256
    return block;
}

//---------------------------------- Stuff ------------------------------------    

shared_ptr<ContainerInterface> StuffAgent::GetContainerInterface( XLink keyer_xlink ) const
{    
    // Note: does not do the walk every time - instead, the Walk object's range is presented
    // to the Conjecture object, which increments it only when trying alternative choice
    return make_shared<Walk>( keyer_xlink.GetChildX(), nullptr, nullptr );
}


void StuffAgent::PatternQueryRestrictions( shared_ptr<PatternQuery> pq ) const
{
    if( recurse_restriction )
        pq->RegisterMultiplicityLink( PatternLink(this, &recurse_restriction) );
}


void StuffAgent::DecidedQueryRestrictions( DecidedQueryAgentInterface &query, ContainerInterface::iterator thistime, XLink keyer_xlink ) const
{
    // Where a recurse restriction is in use, apply it to all the recursion points
    // underlying the current iterator, thistime.
    if( recurse_restriction )
    {
        auto xpr_ss = MakeTreeNode<SubSequence>();

        // See if we are looking at a walk iterator
        const Walk::iterator *pwtt = dynamic_cast<const Walk::iterator *>(thistime.GetUnderlyingIterator());
        ASSERT(pwtt)("Failed to get Walk::iterator out of the decision iterator");    

        // Check all the nodes that we recursed through in order to get here
        for( pair<TreePtr<Node>, const TreePtrInterface *> p : pwtt->GetCurrentPath() )
        {
            xpr_ss->elts.push_back( p.second ? XLink(p.first, p.second) : keyer_xlink );
            TRACE("DQR ")(p.first)(" ")(p.second)("\n");
        }

        XLink xpr_ss_link = XLink::CreateDistinct( xpr_ss ); // Only used in after-pass
        query.RegisterMultiplicityLink( PatternLink(this, &recurse_restriction), xpr_ss_link ); // Links into X     
    }   
}                                                                                     


SYM::Over<SYM::BooleanExpression> StuffAgent::SymbolicNormalLinkedQueryPRed() const
{
    PatternLink terminus_plink(this, &terminus);
    auto expr = MakeOver<BooleanConstant>(true);
    
    expr &= MakeOver<SymbolVariable>(terminus_plink) >= 
            MakeOver<SymbolVariable>(keyer_plink);
            
    expr &= MakeOver<SymbolVariable>(terminus_plink) <= 
            MakeOver<LastDescendantOperator>( MakeOver<SymbolVariable>(keyer_plink) );
    
    return expr;
}


void StuffAgent::RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                           const SolutionMap *hypothesis_links,
                                           const XTreeDatabase *x_tree_db ) const
{
    INDENT("#");
    ASSERT( this );
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
        const XTreeDatabase::Nugget &nugget( x_tree_db->GetNugget(xlink) );       
        xlink = nugget.parent_xlink;
        
        // Putting this here excludes the terminus, as required
        TRACEC("Move to parent ")(xlink)("\n");
        xpr_ss->elts.push_front( xlink );      
    }
    
    XLink xpr_ss_link = XLink::CreateDistinct( xpr_ss ); // Only used in after-pass
    query.RegisterMultiplicityLink( PatternLink(this, &recurse_restriction), xpr_ss_link ); // Links into X    
}
    
    
Graphable::Block StuffAgent::GetGraphBlockInfo() const
{
	// The Stuff node appears as a small square with a # character inside it. The terminus block emerges from the
	// right of the circle. # is chosen (as is the name Stuff) for its addr_bounding_role to * because
	// the nodes are both able to wildcard multiple nodes in the input tree.
    Block block = SearchContainerAgent::GetGraphBlockInfo();
	block.title = "Stuff"; 
	block.symbol = "#"; 
    if( recurse_restriction )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(recurse_restriction.get()), 
                  list<string>{},
                  list<string>{},
                  phase,
                  &recurse_restriction );
        block.sub_blocks.push_back( { "recurse_restriction", 
                                      "", 
                                      false,
                                      { link } } );
    }
    return block;
}
