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

void AndRuleEngine::Configure( Agent *root_agent_, const Set<Agent *> &master_agents )
{
    root_agent = root_agent_;

    Set<Agent *> normal_agents;
    ConfigPopulateNormalAgents( &normal_agents, root_agent );    
    my_agents = SetDifference( normal_agents, master_agents );       
    Set<Agent *> surrounding_agents = SetUnion( master_agents, my_agents ); 
        
    for( std::pair<Agent * const, AndRuleEngine> &pae : my_abnormal_engines )
        pae.second.Configure( pae.first, surrounding_agents );
        
    for( std::pair<Agent * const, AndRuleEngine> &pae : my_multiplicity_engines )
        pae.second.Configure( pae.first, surrounding_agents );
}


void AndRuleEngine::ConfigPopulateNormalAgents( Set<Agent *> *normal_agents, Agent *current_agent )
{
    normal_agents->insert(current_agent);
    
    PatternQueryResult query = current_agent->PatternQuery();
    FOREACH( const PatternQueryResult::Link &b, *query.GetNormalLinks() )
        ConfigPopulateNormalAgents( normal_agents, b.agent );
        
    FOREACH( const PatternQueryResult::Link &b, *query.GetAbnormalLinks() )
        my_abnormal_engines.emplace(std::piecewise_construct,
                                 std::forward_as_tuple(b.agent),
                                 std::forward_as_tuple());    

    FOREACH( const PatternQueryResult::Link &b, *query.GetMultiplicityLinks() )
        my_multiplicity_engines.emplace(std::piecewise_construct,
                                     std::forward_as_tuple(b.agent),
                                     std::forward_as_tuple());
}


void AndRuleEngine::CompareLinks( shared_ptr<const AgentQuery> query ) 
{
	ASSERT( !query->GetEvaluator() );
    // Follow up on any blocks that were noted by the agent impl

    FOREACH( const AgentQuery::Link &b, *query->GetAbnormalLinks() )
        abnormal_links.insert( make_pair(query, &b) ); 
        
    FOREACH( const AgentQuery::Link &b, *query->GetMultiplicityLinks() )
        multiplicity_links.insert( make_pair(query, &b) ); 
    
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
            my_abnormal_engines.at(b.agent).Compare( px, coupling_keys );
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
    TRACE("OK\n");
}


// This one if you want the resulting couplings and conj (ie doing a replace imminently)
void AndRuleEngine::Compare( const TreePtrInterface *p_start_x,
                             const CouplingMap *master_keys_ )
{
    INDENT("C");
    ASSERT( p_start_x );
    ASSERT( *p_start_x );
    TRACE("Compare x=")(**p_start_x);
    TRACE(" pattern=")(*root_agent);
           
    master_keys = master_keys_;    
    conj.Configure(my_agents, root_agent);
           
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
                multiplicity_links.clear();
                evaluator_queries.clear();
                DecidedCompare( root_agent, p_start_x );
            }
            
            CouplingMap combined_keys = MapUnion( *master_keys, slave_keys );     
                        
            // Process the free abnormal links.
            for( std::pair< shared_ptr<const AgentQuery>, const AgentQuery::Link * > lp : abnormal_links )
            {            
                my_abnormal_engines.at(lp.second->agent).Compare( lp.second->GetPX(), &combined_keys );
            }

            // Process the free multiplicity links.
            int i=0;
            for( std::pair< shared_ptr<const AgentQuery>, const AgentQuery::Link * > lp : multiplicity_links )
            {            
                AndRuleEngine &e = my_multiplicity_engines.at(lp.second->agent);

                const TreePtrInterface *px = lp.second->GetPX();
                ASSERT( *px );
                ContainerInterface *xc = dynamic_cast<ContainerInterface *>(px->get());
                ASSERT(xc)("Multiplicity x must implement ContainerInterface");
                TRACE("Comparing multiplicity link %d size %d\n", i, xc->size());
                
                FOREACH( const TreePtrInterface &xe, *xc )
                {
                    e.Compare( &xe, &combined_keys );
                }

                i++;
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
    TRACE("OK\n");
}


// This one operates from root for a stand-alone compare operation and
// no master keys.
void AndRuleEngine::Compare( const TreePtrInterface *p_start_x )
{
    CouplingMap master_keys;
    Compare( p_start_x, &master_keys );
}


const Conjecture &AndRuleEngine::GetConjecture()
{
    return conj;
}


const CouplingMap &AndRuleEngine::GetCouplingKeys()
{
    return slave_keys;
}

