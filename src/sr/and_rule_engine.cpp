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

void AndRuleEngine::Configure( std::shared_ptr< Set<Agent *> > _my_agents )
{
    my_agents = _my_agents;
}


void AndRuleEngine::CompareLinks( shared_ptr<const AgentQuery> query ) 
{
	ASSERT( !query->GetEvaluator() );
    // Follow up on any blocks that were noted by the agent impl

    FOREACH( const AgentQuery::Link &b, *query->GetAbnormalLinks() )
    {
        abnormal_links.insert( make_pair(query, &b) ); 
    }    
    int i=0;        
    FOREACH( const AgentQuery::Link &b, *query->GetNormalLinks() )
    {
        TRACE("Comparing normal link %d\n", i);
        // Recurse normally
        // Get x for linked node
        const TreePtrInterface *px = b.GetPX();
        ASSERT( *px );
        
        DecidedCompare(b.agent, px);   
        i++;             
    }
    i=0;
    FOREACH( const AgentQuery::Link &b, *query->GetMultiplicityLinks() )
    {
        TRACE("Comparing multiplicity link %d\n", i);
        // Recurse normally
        // Get x for linked node
        const TreePtrInterface *px = b.GetPX();
        ASSERT( *px );
        
        if( const TreePtr<SubSequence> px_subsequence = TreePtr<SubSequence>::DynamicCast(*px) )
        {
            //FOREACH( const TreePtrInterface &x_element, *px_subcontainer )
            for( SubSequence::iterator it = px_subsequence->begin(); it != px_subsequence->end(); ++it )
            {
                auto & x_element = *it;
                DecidedCompare(b.agent, &x_element);   
            }            
        }
        else if( const TreePtr<SubSequenceRange> px_subsequence = TreePtr<SubSequenceRange>::DynamicCast(*px) )
        {
            //FOREACH( const TreePtrInterface &x_element, *px_subcontainer )
            for( SubSequenceRange::iterator it = px_subsequence->begin(); it != px_subsequence->end(); ++it )
            {
                auto & x_element = *it;
                DecidedCompare(b.agent, &x_element);   
            }            
        }
        else if( const TreePtr<SubCollection> px_subcollection = TreePtr<SubCollection>::DynamicCast(*px) )
        {
            //FOREACH( const TreePtrInterface &x_element, *px_subcontainer )
            for( SubCollection::iterator it = px_subcollection->begin(); it != px_subcollection->end(); ++it )
            {
                auto & x_element = *it;
                DecidedCompare(b.agent, &x_element);   
            }            
        }
        else
        {
            ASSERT(false)("Unrecognised multiplicity x type");
        }
        i++;             
    }
}


// Only to be called in the restricting pass
void AndRuleEngine::CompareEvaluatorLinks( shared_ptr<const AgentQuery> query,
							               const CouplingMap *coupling_keys ) 
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
            AndRuleEngine e;
            e.Configure(my_agents);
            e.Compare( b.agent, px, coupling_keys );
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
                                    const TreePtrInterface *px )  
{
    INDENT(" ");
    ASSERT( px ); // Ref to target must not be NULL (i.e. corrupted ref)
    ASSERT( *px ); // Target must not be NULL

    // Check for a coupling match to a master engine's agent. 
    SimpleCompare sc;
    if( master_keys->IsExist(agent) )
    {               
        sc( *px, master_keys->At(agent) );
        return;
    }
    // Check for a coupling match to one of our agents we reached earlier in this pass.
    else if( reached.IsExist(agent) )
    {
        sc( *px, slave_keys.At(agent) );
        return;
    }

    // Remember we reached this agent 
    reached.insert( agent );

    // Obtain the query state from the conjecture
    shared_ptr<AgentQuery> query = conj.GetQuery(agent);

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
                        
    (void)conj.FillMissingChoicesWithBegin(query);

    // Remember the coupling before recursing, as we can hit the same node 
    // (eg identifier) and we need to have coupled it. 
    if( !slave_keys.IsExist(agent) )
        slave_keys[agent] = *px;
          
    // Use worker functions to go through the links, special case if there is evaluator
    if( query->GetEvaluator() )
    {
        evaluator_queries.insert(query);
	}
    else
    {
		TRACE(*agent)("?=")(**px)(" Comparing links\n");
        CompareLinks( query );
	}
}


// This one if you want the resulting couplings and conj (ie doing a replace imminently)
void AndRuleEngine::Compare( Agent *start_agent,
                             const TreePtrInterface *p_start_x,
                             const CouplingMap *master_keys_ ) 
{
    INDENT("C");
    ASSERT( p_start_x );
    ASSERT( *p_start_x );
    TRACE("Compare x=")(**p_start_x);
    TRACE(" pattern=")(*start_agent);
           
    master_keys = master_keys_;    
    conj.Configure(*my_agents, start_agent);
           
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
            slave_keys.clear();

            // Do a two-pass matching process: first get the keys...
            {
                reached.clear();
                abnormal_links.clear();
                evaluator_queries.clear();
                DecidedCompare( start_agent, p_start_x );
            }
            
            CouplingMap combined_keys = MapUnion( *master_keys, slave_keys );     
                        
            // Process the free abnormal links. These may be more than one with the same linked pattern
            // node and the number can vary depending on x. We wouldn't know which one to key to, so we 
            // process them in a post-pass which ensures all the couplings have been keyed already.
            // Examples are the pattern restrictions on Star and Stuff.
            for( std::pair< shared_ptr<const AgentQuery>, const AgentQuery::Link * > lp : abnormal_links )
            {            
                AndRuleEngine e;
                e.Configure(my_agents);
                e.Compare( lp.second->agent, lp.second->GetPX(), &combined_keys );
            }

            // Process the evaluator queries. These can match when their children have not matched and
            // we wouldn't be able to reliably key those children so we process them in a post-pass 
            // which ensures all the couplings have been keyed already.
            // Examples are MatchAny and NotMatch (but not MatchAll, because MatchAll conforms with
            // the global and-rule and so its children can key couplings.
            for( shared_ptr<const AgentQuery> query : evaluator_queries )
            {
                //TRACE(*query)(" Comparing evaluator query\n"); TODO get useful trace off queries
                CompareEvaluatorLinks( query, &combined_keys );
            }
        }
        catch( const ::Mismatch& mismatch )
        {                
            TRACE("SCREngine miss, trying increment conjecture\n");
            if( conj.Increment() )
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


const Conjecture &AndRuleEngine::GetConjecture()
{
    return conj;
}


const CouplingMap &AndRuleEngine::GetCouplingKeys()
{
    return slave_keys;
}
