#include "scr_engine.hpp"
#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "helpers/simple_compare.hpp"
#include "slave_agent.hpp"
#include "standard_agent.hpp"
#include "overlay_agent.hpp"
#include "search_container_agent.hpp"
#include "common/common.hpp"
#include <list>

//#define TEST_PATTERN_QUERY

using namespace SR;

void AndRuleEngine::CompareLinks( shared_ptr<const AgentQuery> query,
                                  CompareState &state ) const
{
	ASSERT( !query->GetEvaluator() );
    // Follow up on any blocks that were noted by the agent impl

    int i=0;        
    FOREACH( const AgentQuery::Link &b, *query->GetAbnormalLinks() )
    {
        state.abnormal_links.insert( make_pair(query, &b) ); 
    }    
    FOREACH( const AgentQuery::Link &b, *query->GetNormalLinks() )
    {
        TRACE("Comparing normal link %d\n", i);
        // Recurse normally
        // Get x for linked node
        const TreePtrInterface *px = b.GetPX();
        ASSERT( *px );
        
        DecidedCompare(b.agent, px, state);   
        i++;             
    }
}


// Only to be called in the restricting pass
void AndRuleEngine::CompareEvaluatorLinks( shared_ptr<const AgentQuery> query,
					   				       const CouplingMap *slave_keys ) const
{
	ASSERT( query->GetEvaluator() );
    ASSERT( query->GetNormalLinks()->empty() )("When an evaluator is used, all links must be into abnormal contexts");

    // Follow up on any blocks that were noted by the agent impl    
    int i=0;
    list<bool> compare_results;
    FOREACH( const AgentQuery::Link &b, *query->GetAbnormalLinks() )
    {
        TRACE("Comparing block %d\n", i);
 
        // Get x for linked node
        const TreePtrInterface *px = b.GetPX();
                                 
        try 
        {
            Compare( b.agent, px, slave_keys );
            compare_results.push_back( true );
        }
        catch( ::Mismatch & )
        {
            compare_results.push_back( false );
        }

        i++;
    }
    
    shared_ptr<BooleanEvaluator> evaluator = query->GetEvaluator();
	TRACE(" Evaluating ");
	FOREACH(bool b, compare_results)
	    TRACE(b)(" ");
	if( !(*evaluator)( compare_results ) )
        throw EvaluatorFalse();
}


void AndRuleEngine::DecidedCompare( Agent *agent,
                                    const TreePtrInterface *px,
                                    CompareState &state ) const 
{
    INDENT(" ");
    ASSERT( px ); // Ref to target must not be NULL (i.e. corrupted ref)
    ASSERT( *px ); // Target must not be NULL

    // Check for a coupling match to a master engine's agent. 
    SimpleCompare sc;
    if( state.master_keys->IsExist(agent) )
    {               
        sc( *px, state.master_keys->At(agent) );
        return;
    }
    // Check for a coupling match to one of our agents we reached earlier in this pass.
    else if( state.reached.IsExist(agent) )
    {
        sc( *px, state.slave_keys->At(agent) );
        return;
    }

    // Remember we reached this agent 
    state.reached.insert( agent );

    // Obtain the query state from the conjecture
    shared_ptr<AgentQuery> query = state.conj->GetQuery(agent);

    // Run the compare implementation to get the links based on the choices
    TRACE(*agent)("?=")(**px)(" DecidedQuery()\n");    
    agent->DecidedQuery( *query, px );

#ifdef TEST_PATTERN_QUERY
    PatternQueryResult r = agent->PatternQuery();
    ASSERT( r.GetNormalLinks()->size() == query->GetNormalLinks()->size() )
          ("PatternQuery disagrees with DecidedQuery!!!!\n")
          ("GetNormalLinks()->size() : %d != %d!!\n", r.GetNormalLinks()->size(), query->GetNormalLinks()->size() )
          (*agent);
    // Note: number of abnormal links can depend on x, for example
    // in the case of pattern restriction on Star, Stuff. See #60 
#endif
                        
    (void)state.conj->FillMissingChoicesWithBegin(query);

    // Remember the coupling before recursing, as we can hit the same node 
    // (eg identifier) and we need to have coupled it. 
    if( !state.slave_keys->IsExist(agent) )
        (*state.slave_keys)[agent] = *px;
          
    // Use worker functions to go through the links, special case if there is evaluator
    if( query->GetEvaluator() )
    {
        state.evaluator_queries.insert(query);
	}
    else
    {
		TRACE(*agent)("?=")(**px)(" Comparing links\n");
        CompareLinks( query, state );
	}
}


// This one if you want the resulting couplings and conj (ie doing a replace imminently)
void AndRuleEngine::Compare( Agent *start_agent,
                             const TreePtrInterface *p_start_x,
                             Conjecture *conj,
                             CouplingMap *slave_keys,
                             const CouplingMap *master_keys ) const
{
    INDENT("C");
    ASSERT( p_start_x );
    ASSERT( *p_start_x );
    TRACE("Compare x=")(**p_start_x);
    TRACE(" pattern=")(*start_agent);
    ASSERT( &slave_keys != &master_keys );
    //TRACE(**pcontext)(" @%p\n", pcontext);
           
    CompareState state;
    state.master_keys = master_keys;    
    state.conj = conj;
           
    // Create the conjecture object we will use for this compare, and keep iterating
    // though different conjectures trying to find one that allows a match.
    //int i=0;
    while(1)
    {
        try
        {
            // Try out the current conjecture. This will call RememberDecision() once for each decision;
            // RememberDecision() will return the current choice for that decision, if absent it will
            // add the decision and choose the first choice, if the decision reaches the end it
            // will remove the decision.

            // Only key if the keys are already set to KEYING (which is 
            // the initial value). Keys could be RESTRICTING if we're under
            // a SoftNot node, in which case we only want to restrict.
            
            // Initialise keys to the ones inherited from master, keeping 
            // none of our own from any previous unsuccessful attempt.
            slave_keys->clear();

            // Do a two-pass matching process: first get the keys...
            {
                state.slave_keys = slave_keys;
                
                state.reached.clear();
                state.abnormal_links.clear();
                state.evaluator_queries.clear();
                DecidedCompare( start_agent, p_start_x, state );
            }
            
            CouplingMap combined_keys = MapUnion( *master_keys, *(state.slave_keys) );     
                        
            // Process the free abnormal links. These may be more than one with the same linked pattern
            // node and the number can vary depending on x. We wouldn't know which one to key to, so we 
            // process them in a post-pass which ensures all the couplings have been keyed already.
            // Examples are the pattern restrictions on Star and Stuff.
            for( std::pair< shared_ptr<const AgentQuery>, const AgentQuery::Link * > lp : state.abnormal_links )
            {            
                Compare( lp.second->agent, lp.second->GetPX(), &combined_keys );
            }

            // Process the evaluator queries. These can match when their children have not matched and
            // we wouldn't be able to reliably key those children so we process them in a post-pass 
            // which ensures all the couplings have been keyed already.
            // Examples are MatchAny and NotMatch (but not MatchAll, because MatchAll conforms with
            // the global and-rule and so its children can key couplings.
            for( shared_ptr<const AgentQuery> query : state.evaluator_queries )
            {
                //TRACE(*query)(" Comparing evaluator query\n"); TODO get useful trace off queries
                CompareEvaluatorLinks( query, &combined_keys );
            }
        }
        catch( const ::Mismatch& mismatch )
        {                
            TRACE("SCREngine miss, trying increment conjecture\n");
            if( conj->Increment() )
                continue; // Conjecture would like us to try again with new choices
                
            // We didn't match and we've run out of choices, so we're done.              
            throw NoSolution();
        }
        // We got a match so we're done. 
        TRACE("SCREngine hit\n");
        break; // Success
    }
    
    // By now, we succeeded and slave_keys is the right set of keys
}


// This one if you don't want the resulting keys and conj (ie not doing a replace)
void AndRuleEngine::Compare( Agent *start_agent,
                             const TreePtrInterface *p_start_x,
                             const CouplingMap *master_keys ) const
{
    Conjecture conj;
    conj.Configure(*my_agents, start_agent);
    CouplingMap slave_keys; 
    Compare( start_agent, p_start_x, &conj, &slave_keys, master_keys );
}



