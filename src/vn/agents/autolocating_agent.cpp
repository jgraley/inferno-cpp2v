#include "autolocating_agent.hpp"
#include "conjecture.hpp"
#include "link.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/symbol_operators.hpp"
#include "up/patches.hpp"

using namespace VN;
using namespace SYM;

//---------------------------------- AutolocatingAgent ------------------------------------    

Lazy<BooleanExpression> AutolocatingAgent::SymbolicNormalLinkedQuery(PatternLink keyer_plink) const
{
    Lazy<BooleanExpression> my_expr = SYM::MakeLazy<SYM::BooleanConstant>(true);
	shared_ptr<PatternQuery> pq = GetPatternQuery();

    for( PatternLink plink : pq->GetNormalLinks() )
        my_expr &= MakeLazy<SymbolVariable>(keyer_plink) == MakeLazy<SymbolVariable>(plink);
    
    my_expr &= SymbolicAutolocatingQuery(keyer_plink);
    my_expr &= SymbolicPreRestriction(keyer_plink) |
                 (MakeLazy<SymbolVariable>(keyer_plink) == MakeLazy<SymbolConstant>(XLink::MMAX));

    return my_expr;
}


void AutolocatingAgent::RunAutolocatingQuery( XLink common_xlink ) const
{
	(void)common_xlink;
    // No restriction by default
}


SYM::Lazy<SYM::BooleanExpression> AutolocatingAgent::SymbolicAutolocatingQuery(PatternLink keyer_plink) const
{
	(void)keyer_plink;
    return SYM::MakeLazy<SYM::BooleanConstant>(true); // TODO don't like this, belongs in ConjuctionAgent
}


Agent::ReplacePatchPtr AutolocatingAgent::GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                                                PatternLink me_plink, 
                                                                XLink key_xlink,
                                                                const SCREngine *acting_engine )
{
	shared_ptr<PatternQuery> pq = GetPatternQuery();
    auto plinks = pq->GetNormalLinks();
    if( plinks.size() == 1 )
    {
        // Unambiguous path through replace pattern so we can continue to overlay
        PatternLink replace_plink = SoloElementOf(plinks);
        ASSERT( replace_plink );          
        return replace_plink.GetChildAgent()->GenReplaceLayout(kit, replace_plink, acting_engine);    
    }
    else
    {
        return AgentCommon::GenReplaceLayoutImpl(kit, me_plink, key_xlink, acting_engine);
    }
}                                         


bool AutolocatingAgent::IsNonTrivialPreRestriction(const TreePtrInterface *tpi) const
{
    if( !AgentCommon::IsNonTrivialPreRestriction(tpi) )
		return false;
				
	// This is autolocating agents, so normal children are autolocating which means
	// they must match the same XLink. If one of them restricts this XLink then we don't have
	// to due to global and-rule.
	//FTRACE(GetPatternQuery()->GetNormalLinks())("\n")
	//      (AreChildrenRestricting())("\n")
	//      (!Any( AreChildrenRestricting() ))("\n");
	return !Any( AreChildrenRestricting() );
} 


bool AutolocatingAgent::IsFixedType() const 
{
	return Any( AreChildrenFixedType() );
}


vector<bool> AutolocatingAgent::AreChildrenRestricting() const
{
	vector<bool> vb;
	shared_ptr<PatternQuery> pq = GetPatternQuery();   
    for( PatternLink child_link : pq->GetNormalLinks() )
    {
		Agent *child_agent = child_link.GetChildAgent();
		vb.push_back(
		    child_agent->IsFixedType() ||
		    child_agent->IsNonTrivialPreRestriction(child_link.GetPatternTreePtrInterface()) );
	}
	return vb;
}


vector<bool> AutolocatingAgent::AreChildrenFixedType() const
{
	vector<bool> vb;
	shared_ptr<PatternQuery> pq = GetPatternQuery();   
    for( PatternLink child_link : pq->GetNormalLinks() )
    {
		Agent *child_agent = child_link.GetChildAgent();
		vb.push_back( child_agent->IsFixedType() );
	}
	return vb;
}


