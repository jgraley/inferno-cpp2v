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
    ASSERTFAIL("");
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
    ASSERTFAIL("");

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
	ASSERTFAIL("Override me or turn off ImplHasNLQ()\n");
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


SYM::Over<SYM::BooleanExpression> SearchLeafAgent::SymbolicNormalLinkedQueryPRed() const
{
    return MakeOver<SYM::BooleanConstant>(true);
}                                      


//---------------------------------- SpecialBase ------------------------------------    

bool SpecialBase::IsNonTrivialPreRestriction(const TreePtrInterface *ptr)
{
    if( ptr )		// is normal tree link
    {
        if( shared_ptr<SpecialBase> sbs = dynamic_pointer_cast<SpecialBase>((TreePtr<Node>)*ptr) )   // is to a special node
        {            
            if( typeid( *ptr ) != typeid( *(sbs->GetArchetypeTreePtr()) ) )    // pre-restrictor is nontrivial
            {
                return true;
            }
        }
    }
    return false;
}
