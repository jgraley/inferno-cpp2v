#include "normality_agent_wrapper.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 

using namespace SR;

// Terminology: abnormal context, as normal, means the subtree under an abnormal block
// evaluator context means an evaluator node and all the abnormal contexts it blocks into
// in both cases, contexts end at couplings.
// The region contains all the agents in a context.

NormalityAgentWrapper::NormalityAgentWrapper( Agent *agent ) :
    wrapped_agent( agent )
{
}
    

void NormalityAgentWrapper::Configure( const Set<Agent *> &engine_agents, 
                                  const Set<Agent *> &master_agents, 
                                  const Engine *engine )
{
    PatternQueryResult plinks = wrapped_agent->PatternQuery();

    abnormal_links.clear();    
    FOREACH( PatternQueryResult::Block b, plinks.blocks )
    {
		if( b.abnormal )
		{
			shared_ptr<AbnormalLink> al( new AbnormalLink() );
			
			// Find the terminal agents
			TerminalFinder tf( b.agent, engine_agents, master_agents, al->terminal_agents );
			FOREACH( TreePtr<Node> n, tf )
			{ // Don't do anything: the TerminalFinder itself fills in al->terminal_agents
			}
			
			// Configure the engine for this abnormal block
			Set< Agent * > surrounding_agents = SetUnion( master_agents, al->terminal_agents );
			al.engine.Configure( b.agent, TreePtr<Node>(), surrounding_agents, engine );
			
			abnormal_links.push_back( al );
		}
	}
	evaluator = blocks.evaluator;
}


void SetMasterKeys( const CouplingMap &keys )
{
	master_keys = keys;
}


PatternQueryResult NormalityAgentWrapper::PatternQuery() const
{
    PatternQueryResult wrapped_result = wrapped_agent->PatternQuery();
    PatternQueryResult wrapper_result;
    wrapper_result.evaluator = wrapped_result.evaluator;
	
    deque< shared_ptr<AbnormalLink> >::iterator alit = abnormal_links.begin();    
    FOREACH( PatternQueryResult::Block b, plinks.blocks ) // JSG2020 wrapped_result.blocks?
    {
		if( b.abnormal )
		{
			al = *alit;
			++alit;
			FOREACH( Agent *ta, al->terminal_agents )
			{
				PatternQueryResult::Block nb;
				nb.abnormal = false;
				nb.agent = ta;
				wrapped_result.push_back( nb ); // JSG2020 wrapper_result.push_back?
			}
		}
		else
		{
			wrapped_result.push_back( b ); // JSG2020 wrapper_result.push_back?
		}
	}
	return wrapper_result;
}


DecidedQueryResult NormalityAgentWrapper::DecidedQuery( const TreePtrInterface &x,
                                                        const deque<ContainerInterface::iterator> &choices ) const;
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const{
    INDENT("'");    
    deque<ContainerInterface::iterator> wrapped_choices, wrapper_choices; // JSG2020 wrapper_choices either is choices or should be init'd from it?

    PatternQueryResult wrapped_result = wrapped_agent->PatternQuery();
    wrapper_result.evaluator = wrapped_result.evaluator;
	
    deque< shared_ptr<AbnormalLink> >::iterator alit = abnormal_links.begin();    
    int i=0;
    FOREACH( PatternQueryResult::Block b, plinks.blocks ) // JSG2020 wrapped_result.blocks?
    {
		if( b.abnormal )
		{
			al = *alit;
			++alit;
			FOREACH( Agent *ta, al->terminal_agents )
			{
				wrapped_choices.push_back( *(wrapper_choices[i]) ); 
				i++;
			}
		}
		else
		{
			wrapped_choices.push_back( *(wrapper_choices[i]) ); 
			i++;
		}
	}
    
    PatternQueryResult wrapper_result;
    wrapper_result.evaluator = false; // we will hide evaluators
    wrapper_result.local_match = true; // ready for and-rule 
    
    // Query the wrapped node for blocks and decisions
    DecidedQueryResult wrapped_result = wrapped_agent->DecidedQuery( x, wrapped_choices );
    
    // Early-out on local mismatch of wrapped node.
    if( !wrapped_result.local_match )
    {
        wrapper_result.local_match = false;
		return wrapper_result;
	}           

    // Loop over the wrapped node's returned blocks. Abnormal entries are co-looped with our
    // own abnormal_links container.
    deque<bool> compare_results;
    deque< AbnormalLink >::iterator alit = abnormal_links.begin();    
    FOREACH( DecidedQueryResult::Block b, wrapped_result.blocks )
    {
		if( b.is_link && b.abnormal )
		{
			ASSERT( !b.is_decision ); // Don't know what to do with decision joined to abnormal link TODO decide
			al = *alit;
			++alit;
			
			// TODO should do this before the wrapped_agent->DecidedQuery() since it *might* help with
			// choice pushing into the wrapped node.
			// Create whole-domain decisions for the terminal agents and key them with the resultant choices 
			CouplingMap terminal_keys;			
			FOREACH( Agent *ta, terminal_agents )
			{
				// Get the root of everything
				TreePtr<Node> c = *(engine->GetOverallMaster()->pcontext);
				// Make a walker of everything
				shared_ptr<ContainerInterface> pwc = shared_ptr<ContainerInterface>( new Walk( c, NULL, NULL ) );
				// TODO see "Problems with context walk" below
				
				// Give that walker to the conjecture as a decision and make a block
				ContainerInterface::iterator cit;
				if( wrapper_choices.empty() )
				{
					cit = pwc->begin(); // No choice was given to us so assume first one
				}
				else
				{
					cit = wrapper_choices.front();  // Use and consume the choice that was given to us
					ASSERT( cit != end );
					wrapper_choices.pop_front();
				}
				DecidedQueryResult::Block nb;
				nb.is_link = true;
				nb.abnormal = false;
				nb.agent = ta;
				nb.px = &(*cit); // do not simplify! we want a simple pointer, not an iterator TODO or do we
				nb.local_x = TreePtr<Node>();					
				Conjecture::Range r; // JSG2020 these 3 lines can be removed; see nb.decision =...
				r.begin = pwc->begin();
				r.end = pwc->end();
				nb.is_decision = true;
				nb.decision = Conjecture::Range( false, pwc->begin(), pwc->end() );
				wrapped_result.push_back( nb );

				// Consider the chosen (or pushed) node to be a key for the benefit of the local engine
				terminal_keys[ta] = *cit;				
			}
			
			// Get the keys that the sub-engine will need to use, and invoke it on the block
			// Theory is that, of the enclosing engine's agents, the terminal ones are the only ones the sub-engine will see.
			CouplingMap coupling_keys = MapUnion( master_keys, terminal_keys ); 
			bool result = al.engine.Compare( l.GetX(), coupling_keys );
			
			// Deal with result - store for evaluator otherwise do AND-rule with early-out
			if( evaluator )
				compare_results.push_back(result);
			else if( !result )
			{
				wrapper_result.local_match = false;
				return wrapper_result;
			}
    	}
	    else
	    {
		    wrapped_result.push_back( b ); // JSG2020 wrapper_result?
		}
    }
	
	// Run the evaluator if one was supplied.
	if( wrapped_result.evaluator )
	    wrapper_result.local_match = (*evaluator)( compare_results );
    return wrapper_result;
}

// Note: Problems with context walk
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

/* What might happen:
 * A pass reaches a NormalityAgentWrapper for an evaluator context. The wrapper registers whole-domain 
 * decisions for all couplings OUT of the evaluator's abnormals and gets choices. It blocks out back
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
 *     - coupling pushing needs blocks and decisions to be more closely bound, and that is
 *       the next Agent tweak I would say.
 */


TreePtr<Node> NormalityAgentWrapper::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    ASSERT("search only");
    return TreePtr<Node>();
}


void NormalityAgentWrapper::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The Region agent should not be used
	*bold = true;
	*shape = "diamond";
	*text = string("[]");
}


shared_ptr<ContainerInterface> NormalityAgentWrapper::GetVisibleChildren() const
{
    PatternQueryResult plinks = wrapped_agent->PatternQuery();
	shared_ptr< Sequence<Node> > seq( new Sequence<Node> );

    // Hide the abnormal children. I need different names for things.
    FOREACH( PatternQueryResult::Block b, plinks.blocks )
		if( !b.abnormal )
			seq->push_back( b.agent );
			
	return seq;
}


shared_ptr<ContainerInterface> NormalityAgentWrapper::TerminalFinder_iterator::GetChildContainer( TreePtr<Node> n ) const
{
	// We are walking agents under the wrapped agent, which is hidden from the 
	// surrounding engine, so we will only see an engine agent if it is coupled to the
	// subtree under our wrapped agent (the abnormal context) - these are the
	// agents we want to use as the terminator layer. 
	if( master_agents.IsExist( AsAgent(n) ) // Surrounding engine's master - ignore (will always be already coupled)
	{
		return shared_ptr< Sequence<Node> >(); // Do not recurse into its children
	}    		    
	else if( engine_agents.IsExist( AsAgent(n) )
	{
		terminal_agents.insert( AsAgent(n) ); // Add this agent as a terminator 
		return shared_ptr< Sequence<Node> >(); // Do not recurse into its children
	}    		    
	else
	{
		return UniqueWalk::iterator::GetChildContainer( n ); // Recurse into children
	}
}


// Standard virtual cloner
shared_ptr<ContainerInterface::iterator_interface> NormalityAgentWrapper::TerminalFinder_iterator::Clone() const
{
	return shared_ptr<VisibleWalk_iterator>( new VisibleWalk_iterator(*this) );
}      



