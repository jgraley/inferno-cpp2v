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
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/symbol_operators.hpp"
#include "sym/clutch.hpp"

#include <stdexcept>

using namespace SR;
using namespace SYM;

//---------------------------------- DefaultMMAXAgent ------------------------------------    

SYM::Lazy<SYM::BooleanExpression> DefaultMMAXAgent::SymbolicNormalLinkedQuery(PatternLink keyer_plink) const
{    
    auto mmax_expr = MakeLazy<SymbolConstant>(XLink::MMAX);
    ClutchRewriter mmax_rewriter( mmax_expr );
    shared_ptr<BooleanExpression> original_expr = SymbolicNormalLinkedQueryMMed(keyer_plink);
    return mmax_rewriter.ApplyDistributed( original_expr );
}

//---------------------------------- PreRestrictedAgent ------------------------------------    

SYM::Lazy<SYM::BooleanExpression> PreRestrictedAgent::SymbolicNormalLinkedQueryMMed(PatternLink keyer_plink) const
{
    return SymbolicPreRestriction() & SymbolicNormalLinkedQueryPRed(keyer_plink);
}                  

//---------------------------------- NonlocatingAgent ------------------------------------    

shared_ptr<PatternQuery> NonlocatingAgent::GetPatternQuery() const
{ 
    return make_shared<PatternQuery>();
}

