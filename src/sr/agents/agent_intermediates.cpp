#include "agent_intermediates.hpp"

#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "scr_engine.hpp"
#include "and_rule_engine.hpp"
#include "link.hpp"

// Temporary
#include "tree/cpptree.hpp"

#include "transform_of_agent.hpp"
#include "sym/lambdas.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/comparison_operators.hpp"
#include "sym/primary_expressions.hpp"
#include "sym/clutch.hpp"

#include <stdexcept>

using namespace SR;
using namespace SYM;

//---------------------------------- DefaultMMAXAgent ------------------------------------    

void DefaultMMAXAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                            XLink keyer_xlink ) const
{
    if( keyer_xlink == XLink::MMAX_Link )
    {
        // Magic Match Anything node: all normal children also match anything
        // This is just to keep normal-domain solver happy, so we 
        // only need normals. 
        for( PatternLink plink : pattern_query->GetNormalLinks() )       
            query.RegisterNormalLink( plink, keyer_xlink );
    }   
    else
    {
        RunDecidedQueryMMed( query, keyer_xlink );
    }
}


void DefaultMMAXAgent::RunNormalLinkedQueryImpl( const SolutionMap *hypothesis_links,
                                                 const TheKnowledge *knowledge ) const
{
    exception_ptr eptr;
    try
    {
        // This function must not "protect" client agents from eg partial queries.        
        RunNormalLinkedQueryMMed( hypothesis_links, knowledge );
    }
    catch( ::Mismatch & )
    {
        eptr = current_exception();
    }
    
    bool all_non_mmax = true;
    bool all_mmax = true;
    for( PatternLink plink : keyer_and_normal_plinks ) 
    {
        if( hypothesis_links->count(plink) > 0 )
        {
            if( hypothesis_links->at(plink) == XLink::MMAX_Link )
                all_non_mmax = false;
            else
                all_mmax = false;                    
        }
    }   

    if( all_mmax )
        return; // Done: all are MMAX            
    
    if( !all_non_mmax )
        throw MMAXPropagationMismatch(); // Mismatch: mixed MMAX and non-MMAX
    
    if (eptr)
       std::rethrow_exception(eptr);
}


void DefaultMMAXAgent::RunNormalLinkedQueryMMed( const SolutionMap *hypothesis_links,
                                                 const TheKnowledge *knowledge ) const                                      
{                      
    ASSERTFAIL("Please implement RunNormalLinkedQueryMMed()\n");
}                     


SYM::Over<SYM::BooleanExpression> DefaultMMAXAgent::SymbolicNormalLinkedQueryImpl() const
{    
    auto mmax_expr = MakeOver<SymbolConstant>(SR::XLink::MMAX_Link);
    ClutchRewriter mmax_rewriter( mmax_expr );
    shared_ptr<BooleanExpression> original_expr = SymbolicNormalLinkedQueryMMed();
    return mmax_rewriter.ApplyDistributed( original_expr );
}

                               
SYM::Over<SYM::BooleanExpression> DefaultMMAXAgent::SymbolicNormalLinkedQueryMMed() const                                      
{                      
	set<PatternLink> nlq_plinks = ToSetSolo( keyer_and_normal_plinks );
	auto nlq_lambda = [this](const Expression::EvalKit &kit)
	{
		RunNormalLinkedQueryMMed( kit.hypothesis_links,
						    	  kit.knowledge ); // throws on mismatch   
	};
	return MakeOver<BooleanLambda>(nlq_plinks, nlq_lambda, GetTrace()+".NLQMMed()");	
}                     

//---------------------------------- PreRestrictedAgent ------------------------------------    

void PreRestrictedAgent::RunDecidedQueryMMed( DecidedQueryAgentInterface &query,
                                              XLink keyer_xlink ) const
{
    // Check pre-restriction
    if( !IsPreRestrictionMatch(keyer_xlink) )
        throw PreRestrictionMismatch();
            
    RunDecidedQueryPRed( query, keyer_xlink );
}


void PreRestrictedAgent::RunNormalLinkedQueryMMed( const SolutionMap *hypothesis_links,
                                                   const TheKnowledge *knowledge ) const
{
    // This function must not "protect" client agents from eg partial queries. 
    RunNormalLinkedQueryPRed( hypothesis_links, knowledge );

    // Baseless query strategy: don't check pre-restriction
    bool based = (hypothesis_links->count(keyer_plink) == 1);
    if( based )
    { 
        // Check pre-restriction
        XLink keyer_xlink = hypothesis_links->at(keyer_plink);
        if( !IsPreRestrictionMatch(keyer_xlink) )
            throw PreRestrictionMismatch();
    }
}

                               
void PreRestrictedAgent::RunNormalLinkedQueryPRed( const SolutionMap *hypothesis_links,
                                                   const TheKnowledge *knowledge ) const                                      
{                      
    ASSERTFAIL("Please implement RunNormalLinkedQueryPRed()\n");
}                     


SYM::Over<SYM::BooleanExpression> PreRestrictedAgent::SymbolicNormalLinkedQueryMMed() const
{
    return SymbolicPreRestriction() & SymbolicNormalLinkedQueryPRed();
}

                               
SYM::Over<SYM::BooleanExpression> PreRestrictedAgent::SymbolicNormalLinkedQueryPRed() const                                      
{                      
	set<PatternLink> nlq_plinks = ToSetSolo( keyer_and_normal_plinks );
	auto nlq_lambda = [this](const Expression::EvalKit &kit)
	{
		RunNormalLinkedQueryPRed( kit.hypothesis_links,
						    	  kit.knowledge ); // throws on mismatch   
	};
	return MakeOver<BooleanLambda>(nlq_plinks, nlq_lambda, GetTrace()+".NLQPRed()");	
}                     

//---------------------------------- TeleportAgent ------------------------------------    

void TeleportAgent::RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                         XLink keyer_xlink ) const
{
    INDENT("T");
    
    auto op = [&](XLink keyer_xlink) -> map<PatternLink, XLink>
    {
        map<PatternLink, XLink> tp_links = RunTeleportQuery( keyer_xlink );
        
        // We will uniquify the link against the domain and then cache it against keyer_xlink
        
        for( pair<PatternLink, XLink> p : tp_links )
            p.second = master_scr_engine->UniquifyDomainExtension(p.second); // in-place
                   
        return tp_links;
    };
    
    map<PatternLink, XLink> cached_links = cache( keyer_xlink, op );
    for( LocatedLink cached_link : cached_links )
    {   
        ASSERT( cached_link );
        query.RegisterNormalLink( (PatternLink)cached_link, (XLink)cached_link );
    }    
}                                    


set<XLink> TeleportAgent::ExpandNormalDomain( const unordered_set<XLink> &keyer_xlinks )
{
    set<XLink> extra_xlinks;
    for( XLink keyer_xlink : keyer_xlinks )
    {
        if( keyer_xlink == XLink::MMAX_Link )
            continue; // MMAX at base never expands domain because all child patterns are also MMAX
        if( !IsPreRestrictionMatch(keyer_xlink) )
            continue; // Failed pre-restriction so can't expand domain

        try
        {
            shared_ptr<DecidedQuery> query = CreateDecidedQuery();
            RunDecidedQueryPRed( *query, keyer_xlink );
           
            for( LocatedLink extra_link : query->GetNormalLinks() )
                extra_xlinks.insert( (XLink)extra_link );
        }
        catch( ::Mismatch & ) {}
    }
    return extra_xlinks;
}


void TeleportAgent::Reset()
{
    AgentCommon::Reset();
    cache.Reset();
}

//---------------------------------- SearchLeafAgent ------------------------------------    

shared_ptr<PatternQuery> SearchLeafAgent::GetPatternQuery() const
{ 
    return make_shared<PatternQuery>(this);
}


void SearchLeafAgent::RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                           XLink keyer_xlink ) const
{
}

//---------------------------------- SpecialBase ------------------------------------    

bool SpecialBase::IsNonTrivialPreRestriction(const TreePtrInterface *ptr)
{
    if( ptr )		// is normal tree link
    {
        if( shared_ptr<SpecialBase> sbs = dynamic_pointer_cast<SpecialBase>((TreePtr<Node>)*ptr) )   // is to a special node
        {            
            if( typeid( *ptr ) != typeid( *(sbs->GetPreRestrictionArchetype()) ) )    // pre-restrictor is nontrivial
            {
                return true;
            }
        }
    }
    return false;
}