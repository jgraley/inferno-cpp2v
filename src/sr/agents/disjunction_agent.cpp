#include "disjunction_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"
#include "sym/lambdas.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/primary_expressions.hpp"
#include "sym/symbol_operators.hpp"

using namespace SR;
using namespace SYM;

void DisjunctionAgent::SCRConfigure( const SCREngine *e,
                                     Phase phase )
{
    AgentCommon::SCRConfigure(e, phase);

    options = make_shared< Collection<Node> >();
    FOREACH( const TreePtrInterface &p, GetDisjuncts() )
        options->insert( p );
}


shared_ptr<PatternQuery> DisjunctionAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
    pq->RegisterDecision(false); // Exclusive, please
    for( CollectionInterface::iterator pit = GetDisjuncts().begin(); pit != GetDisjuncts().end(); ++pit )                 
    {
        const TreePtrInterface *p = &*pit; 
	    pq->RegisterNormalLink( PatternLink(this, p) );
    }
    
    return pq;
}


void DisjunctionAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                            XLink x ) const
{
    INDENT("∨");
    ASSERT( !GetDisjuncts().empty() ); // must be at least one thing!
    
    // Check pre-restriction
    if( !IsLocalMatch( x.GetChildX().get() ) )
        throw PreRestrictionMismatch();

    // We register a decision that actually chooses between our agents - that
    // is, the disjuncts for the OR operation.
    ContainerInterface::iterator choice_it = query.RegisterDecision( options, false );
    FOREACH( const TreePtrInterface &p, GetDisjuncts() )                 
    {
        PatternLink plink(this, &p);

        // Context is normal because all patterns must match
        if( *plink.GetPatternPtr() == *choice_it ) // Is this the pattern that was chosen?
        {
            // Yes, so supply the "real" x for this link. We'll really
            // test x against this pattern.
            query.RegisterNormalLink( plink, x ); // Link into X
        }
        else
        {
            // No, so just make sure this link matches (overall AND-rule
            // applies, reducing the outcome to that of the normal 
            // link registered above).
            query.RegisterNormalLink( plink, XLink::MMAX_Link ); // Link into MMAX
        }
    }
}


bool DisjunctionAgent::ImplHasSNLQ() const
{    
    return true;
}


void DisjunctionAgent::RunNormalLinkedQueryImpl( const SolutionMap *hypothesis_links,
                                                 const TheKnowledge *knowledge ) const
{ 
    // Only handles the case where keyer is not MMAX and pre-restriction already handled
    INDENT("∨");
    if( hypothesis_links->count(keyer_plink)==0 )
        throw IncompleteQuery();
    XLink keyer_xlink;
    keyer_xlink = hypothesis_links->at(keyer_plink);
        
    // Loop over the disjuncts for this disjunction and collect the links 
    // that are not MMAX. Also take note of missing children.
    list<XLink> non_mmax_residuals;
    FOREACH( const TreePtrInterface &p, GetDisjuncts() )           
    {
        PatternLink plink(this, &p);
        if( hypothesis_links->count(plink)==0 )
            throw IncompleteQuery();
        XLink xlink = hypothesis_links->at(plink); 
        ASSERT( xlink );
        if( xlink != XLink::MMAX_Link )
            non_mmax_residuals.push_back( xlink );
    }
    
    // Choose a checking strategy based on the number of non-MMAX residuals we saw. 
    // It should be 1.
    switch( non_mmax_residuals.size() )
    {
    case 0:
        // All were MMAX
        throw NoOptionsMatchedMismatch();    
        break;        
        
    case 1:
        {
            // This is the correct number of non-MMAX. If we have a base, check against it.
            XLink taken_option_x_link = OnlyElementOf(non_mmax_residuals);
            if( taken_option_x_link != keyer_xlink )
                throw TakenOptionMismatch();  
            break;        
        }
        
    default: // 2 or more
        // It's never OK to have more than one non-MMAX (strict MMAX rules).
        throw MMAXRequiredOnUntakenOptionMismatch();        
        break;
    }
}


SYM::Over<SYM::BooleanExpression> DisjunctionAgent::SymbolicNormalLinkedQueryImpl() const
{
    auto mmax_expr = MakeOver<SymbolConstant>(XLink::MMAX_Link);
    auto keyer_expr = MakeOver<SymbolVariable>(keyer_plink);
    
    list< shared_ptr<BooleanExpression> > is_mmax_exprs, is_keyer_exprs;
    FOREACH( const TreePtrInterface &p, GetDisjuncts() )           
    {
        PatternLink c_plink(this, &p);
        auto c_expr = MakeOver<SymbolVariable>(c_plink);
        is_mmax_exprs.push_back( c_expr==mmax_expr );
        is_keyer_exprs.push_back( c_expr==keyer_expr );
    }
          
    auto non_mmax_case_expr = MakeOver<BooleanConstant>(true);
    if( ReadArgs::split_disjunctions )
    {
        ASSERT( GetDisjuncts().size() == 2 )
              ("Got %d choices; to support more than 2 disjuncts, enable SplitDisjunctions; fewer than 2 not allowed", GetDisjuncts().size());
        // This is actually the only part that's hard with more than 2 disjuncts
        non_mmax_case_expr &= is_mmax_exprs.front() & is_keyer_exprs.back() | is_mmax_exprs.back() & is_keyer_exprs.front();
    }
    else
    {
        set<PatternLink> nlq_plinks = ToSetSolo( keyer_and_normal_plinks );
        auto nlq_lambda = [this](const Expression::EvalKit &kit)
        {
            RunNormalLinkedQueryImpl( kit.hypothesis_links,
                                      kit.knowledge ); // throws on mismatch   
        };
        non_mmax_case_expr &= MakeOver<BooleanLambda>(nlq_plinks, nlq_lambda, GetTrace()+".DisjuncArb()");	             
    }
        
    non_mmax_case_expr &= SymbolicPreRestriction(); // Don't forget the pre-restriction, applies in non-MMAX-keyer case
    
    return MakeOver<BooleanConditionalOperator>( keyer_expr == mmax_expr, 
                                                 MakeOver<AndOperator>( is_mmax_exprs ),
                                                 non_mmax_case_expr );
}


Graphable::Block DisjunctionAgent::GetGraphBlockInfo() const
{
	// The Disjunction node appears as a diamond with a ∨ character inside it. The affected subtrees are 
	// on the right.
	// NOTE this node controls the action of the search engine in Inferno search/replace. It is not 
    // a node that represents a boolean operation in the program being processed. Those nodes would 
    // appear as rounded rectangles with the name at the top. Their names may be found in
	// src/tree/operator_db.txt  
    Block block;
	block.bold = true;
    block.title = "Disjunction";
	block.symbol = string("∨");
	block.shape = "diamond";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    block.sub_blocks = { { "patterns", 
                           "", 
                           true,
                           {} } };
    FOREACH( const TreePtrInterface &p, GetDisjuncts() )
    {
        auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(p.get()),
                  list<string>{},
                  list<string>{},
                  phase,
                  &p );
        block.sub_blocks.front().links.push_back( link );
    }

    return block;
}
