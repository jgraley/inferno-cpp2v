#include "region_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 

using namespace SR;

// Terminology: abnormal context, as normal, means the subtree under an abnormal link
// evaluator context means an evaluator node and all the abnormal contexts it links into
// in both cases, contexts end at couplings.
// The region contains all the agents in a context.

void RegionAgent::PatternQueryImpl() const
{
    FOREACH( Agent *a, external_couplings )
        RememberLink( false, a ); // these should always turn out to be couplings - somehow check this?
}


bool RegionAgent::DecidedQueryImpl( const TreePtrInterface &x ) const
{
    INDENT("'");    
    
    FOREACH( Agent *a, external_couplings )
    {
		// Get the root of everything
		TreePtr<Node> c = *(engine->GetOverallMaster()->pcontext);
		// Make a walker of everything
		shared_ptr<ContainerInterface> pwc = shared_ptr<ContainerInterface>( new Walk( c, NULL, NULL ) );
		// Give that walker to the conjecture as a decision and make a link
		// Note: this permits coupling pushing, which would remove the complexity order from the search
		ContainerInterface::iterator thistime = RememberDecisionLink( false, a, pwc->begin(), pwc->end() );
        
        // Note: the problems with this are that
        // (1) If the coupling is to a locally generated tree node, we'll never propose it here
        // because we're only doing input tree nodes and
        // (2) We need not repeat choices that are equal (in the SimpleCompare sense) since 
        // SimpleCompare is always used to check couplings.
        // Only (1) causes malfunciton - (2) is just inefficient. 
        // Under the orthodox map, regions could not couple to locally generated tree nodes, and
        // this is an excessive restriction.
        // Under the closed orthodox map (i.e. the closure over all achievable locally generated 
        // nodes) we could do it and would need that closure as the domain of our decision.
        // A closed equivalence orthodox map would fix 2.
        // However, regardless of equivalence, closure may make the domain too big.
        // Another possibility is to request the domain from the coupled-to Agent. This information
        // should be choice-and position-independent, so it will be the context walk for absolute nodes
        // and the transformation applied to the parent's domain for locally generated nodes. I call
        // this the agent-domain map. But it's not *that* much better than equivalence orthodox map, 
        // since in the latter, local-nodes do NOT cross, but just add, so you total 
        // O(root walk * count of locals). What is bad is if choices modify a local map, because 
        // then the domain looks like O( root walk * count of choices ^ count of decisions ) where
        // count of decisions could ACCUMULATE in nested local generator agents. However, I don't
        // think this happens. 
	}
        
    // I think this is right regardless whether the region is an abnormal context or an evaluator 
    // context.
    return Engine::Compare( root_agent, x, master_keys );
}

/* What might happen:
 * A pass reaches a RegionAgent for an evaluator context. The region agent registers whole-domain 
 * decisions for all couplings OUT of the evaluator context and gets choices. It links out back
 * into the surrounding context with these choices, and they are therefore evaluated using the 
 * global AND-rule. But this is CORRECT! Since a coupling must include at least one normal 
 * context, that normal context will restrict the entire search to successful matches of the
 * subtree at the coupling point. We then additionally restrict according to the output of
 * the evaluator.
 */

/* Efficiency issues:
 * Choice of domain:
 * - probably use common domain eg closed orthodox to begin with since much easier
 *     - from master root, so truly fixed
 * - moving the domains to equivalence classes (modulo SimpleCompare) is a seperate optimisation
 * - need coupling pushing to avoid the extra decision search resulting from the new decision
 *     - this can only work of regions come last, but now this is an efficiency concern
 *       rather than correctness.
 *     - coupling pushing needs links and decisions to be more closely bound, and that is
 *       the next Agent tweak I would say.
 */

TreePtr<Node> RegionAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    ASSERT("search only");
    return TreePtr<Node>();
}


void RegionAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The Region agent should not be used
	*bold = true;
	*shape = "diamond";
	*text = string("[]");
}


shared_ptr<ContainerInterface> RegionAgent::GetVisibleChildren() const
{
	// Normally, when an engine hits another engine while walking its subtree
	// during configure, any children (presumably in a co-inherited Agent class)
	// are hidden. Unless the agent overrides (eg Slave, which exposes "through")
	return shared_ptr< Sequence<Node> >();
}


