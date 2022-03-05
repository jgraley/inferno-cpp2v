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
#include "sym/predicate_operators.hpp"
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


SYM::Over<SYM::BooleanExpression> PreRestrictedAgent::SymbolicNormalLinkedQueryMMed() const
{
    return SymbolicPreRestriction() & SymbolicNormalLinkedQueryPRed();
}

                               
SYM::Over<SYM::BooleanExpression> PreRestrictedAgent::SymbolicNormalLinkedQueryPRed() const                                      
{                      
	ASSERTFAIL("Override me\n");
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
