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

#define TEST_PATTERN_QUERY

//#define USE_SOLVER

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

#ifdef USE_SOLVER
    list<Constraint *> constraints;
    for( Agent *a : my_agents )
    {
        shared_ptr<Constraint> c = make_shared<CSP::SystemicConstraint>( a );
        my_constraints[a] = c;    
        constraints.push_back(c);
    }
    solver = make_shared<SimpleSolver>(constraints);
#else
    conj.Configure(my_agents, root_agent);
#endif
}


void AndRuleEngine::ConfigPopulateNormalAgents( Set<Agent *> *normal_agents, Agent *current_agent )
{
    normal_agents->insert(current_agent);
    
    shared_ptr<PatternQuery> pq = current_agent->GetPatternQuery();
    FOREACH( const PatternQuery::Link &b, *pq->GetNormalLinks() )
        ConfigPopulateNormalAgents( normal_agents, b.agent );
        
    // Can be nicer in C++17, apparently.
    FOREACH( const PatternQuery::Link &b, *pq->GetAbnormalLinks() )
        my_abnormal_engines.emplace(std::piecewise_construct,
                                    std::forward_as_tuple(b.agent),
                                    std::forward_as_tuple());    

    FOREACH( const PatternQuery::Link &b, *pq->GetMultiplicityLinks() )
        my_multiplicity_engines.emplace(std::piecewise_construct,
                                        std::forward_as_tuple(b.agent),
                                        std::forward_as_tuple());
}


void AndRuleEngine::CompareLinks( shared_ptr<const DecidedQuery> query ) 
{

    FOREACH( const DecidedQuery::Link &b, *query->GetAbnormalLinks() )
        abnormal_links.insert( make_pair(query, &b) ); 
        
    FOREACH( const DecidedQuery::Link &b, *query->GetMultiplicityLinks() )
        multiplicity_links.insert( make_pair(query, &b) ); 
    
    int i=0;        
    FOREACH( const DecidedQuery::Link &b, *query->GetNormalLinks() )
    {
        TRACE("Comparing normal link %d\n", i++);
        // Recurse normally 
        // Get x for linked node
        TreePtr<Node> x = b.x;
        ASSERT( x );
        
        // This is needed for decisionised MatchAny #75. Other schemes for
        // RegisterAlwaysMatchingLink() could be deployed.
        if( x.get() == (Node *)(b.agent) )
            continue; // Pattern nodes immediately match themselves
        
        DecidedCompare(b.agent, x);   
    }
}


// New rules for evaluators:
// - Abnormal links will be collected up and submitted to the evaluator functor.
// - Normal and multiplicity links will be handled as usual. 

void AndRuleEngine::CompareEvaluatorLinks( shared_ptr<const DecidedQuery> query,
							               const CouplingMap *coupling_keys ) 
{
	ASSERT( query->GetEvaluator() );
    ASSERT( query->GetNormalLinks()->empty() )("When an evaluator is used, all links must be into abnormal contexts");

    // Follow up on any blocks that were noted by the agent impl    
    int i=0;
    list<bool> compare_results;
    FOREACH( const DecidedQuery::Link &b, *query->GetAbnormalLinks() )
    {
        // Don't let this link go into the general AND-rule
        abnormal_links.erase( make_pair(query, &b) ); 
        
        TRACE("Comparing block %d\n", i);
 
        // Get x for linked node
        TreePtr<Node> x = b.x;
                                 
        try 
        {
            my_abnormal_engines.at(b.agent).Compare( x, coupling_keys );
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
                                    TreePtr<Node> x )  
{
    INDENT(" ");
    ASSERT( x ); // Target must not be NULL

    // Check for a coupling match to a master engine's agent. 
    SimpleCompare sc;
    if( master_keys->IsExist(agent) )
    {               
        sc( x, master_keys->At(agent) );
        return;
    }
    // Check for a coupling match to one of our agents we reached earlier in this pass.
    else if( reached.IsExist(agent) )
    {
        sc( x, my_keys.At(agent) );
        return;
    }

    // Obtain the query state from the conjecture
    shared_ptr<DecidedQuery> query = conj.GetQuery(agent);

    // Run the compare implementation to get the links based on the choices
    TRACE(*agent)("?=")(*x)(" RunDecidedQueryImpl()\n");    
    agent->RunDecidedQuery( *query, x );

#ifdef TEST_PATTERN_QUERY
    shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
    ASSERT( pq->GetNormalLinks()->size() == query->GetNormalLinks()->size() &&
            pq->GetAbnormalLinks()->size() == query->GetAbnormalLinks()->size() &&
            pq->GetMultiplicityLinks()->size() == query->GetMultiplicityLinks()->size() &&
            pq->GetDecisions()->size() == query->GetDecisions()->size() )
          ("PatternQuery disagrees with DecidedQuery!!!!\n")
          ("GetNormalLinks()->size() : %d vs %d\n", pq->GetNormalLinks()->size(), query->GetNormalLinks()->size() )
          ("GetAbnormalLinks()->size() : %d vs %d\n", pq->GetAbnormalLinks()->size(), query->GetAbnormalLinks()->size() )
          ("GetMultiplicityLinks()->size() : %d vs %d\n", pq->GetMultiplicityLinks()->size(), query->GetMultiplicityLinks()->size() )
          ("GetDecisions()->size() : %d vs %d\n", pq->GetDecisions()->size(), query->GetDecisions()->size() )
          (*agent);
    // Note: number of abnormal links does NOT now depend on x; #60 completed
#endif
                                  
    // Remember the coupling before recursing, as we can hit the same node 
    // (eg identifier) and we need to have coupled it. 
    my_keys[agent] = x;

    // Remember we reached this agent 
    reached.insert( agent );

    // Use worker functions to go through the links, special case if there is evaluator
    if( query->GetEvaluator() )
    {
        evaluator_queries.insert(query);
	}
    {
		TRACE(*agent)("?=")(*x)(" Comparing links\n");
        CompareLinks( query );
	}
    TRACE("OK\n");
}


// This one if you want the resulting couplings and conj (ie doing a replace imminently)
void AndRuleEngine::Compare( TreePtr<Node> start_x,
                             const CouplingMap *master_keys_ )
{
    INDENT("C");
    ASSERT( start_x );
    TRACE("Compare x=")(*start_x);
    TRACE(" pattern=")(*root_agent);
           
    master_keys = master_keys_;    
#ifdef USE_SOLVER
    list< TreePtr<Node> > x_nodes;
	Walk w( start_x ); 
	for( Walk::iterator wit=w.begin(); wit!=w.end(); ++wit )
        x_nodes.push_back(*wit);
    solver.Start( x_nodes );
#else
    conj.Start();
#endif
           
    // Create the conjecture object we will use for this compare, and keep iterating
    // though different conjectures trying to find one that allows a match.
    //int i=0;
    while(1)
    {
#ifdef USE_SOLVER        
        SideInfo side_info;
        map< shared_ptr<CSP::Constraint>, list< TreePtr<Node> > > values;
        bool match = GetNextSolution( values, &side_info );        
        if( !match )
            throw NoSolution();

        my_keys.clear();
        for( pair< Agent *, shared_ptr<Constraint> > p : my_constraints )
        {
            list< TreePtr<Node> > &vals = values.at(p->second);
            list< Agent * > vars = p.second->GetVariables();
            ASSERT( vars.front() == p.first );
            my_keys[vars.front()] = vals.front(); // For now only do the first one, which is 
        }
        
        abnormal_links = side_info.abnormal_links;
        multiplicity_links = side_info.multiplicity_links;
        evaluator_queries = side_info.evaluator_queries;
#endif
        try
        {
#ifndef USE_SOLVER
            // Try out the current conjecture. This will call RegisterDecision() once for each decision;
            // RegisterDecision() will return the current choice for that decision, if absent it will
            // add the decision and choose the first choice, if the decision reaches the end it
            // will remove the decision.
            
            // Initialise keys to the ones inherited from master, keeping 
            // none of our own from any previous unsuccessful attempt.
            reached.clear();
            abnormal_links.clear();
            multiplicity_links.clear();
            evaluator_queries.clear();
            DecidedCompare( root_agent, start_x );
#endif
            CouplingMap combined_keys = MapUnion( *master_keys, my_keys );     
                                                
            // Process the evaluator queries. These can match when their children have not matched and
            // we wouldn't be able to reliably key those children so we process them in a post-pass 
            // which ensures all the couplings have been keyed already.
            // Examples are MatchAny and NotMatch (but not MatchAll, because MatchAll conforms with
            // the global and-rule and so its children can key couplings.
            for( shared_ptr<const DecidedQuery> query : evaluator_queries )
            {
                //TRACE(*query)(" Comparing evaluator query\n"); TODO get useful trace off queries
                CompareEvaluatorLinks( query, &combined_keys );
            }

            // Process the free abnormal links.
            for( std::pair< shared_ptr<const DecidedQuery>, const DecidedQuery::Link * > lp : abnormal_links )
            {            
                AndRuleEngine &e = my_abnormal_engines.at(lp.second->agent);
                TreePtr<Node> xe = lp.second->x;
                e.Compare( xe, &combined_keys );
            }

            // Process the free multiplicity links.
            int i=0;
            for( std::pair< shared_ptr<const DecidedQuery>, const DecidedQuery::Link * > lp : multiplicity_links )
            {            
                AndRuleEngine &e = my_multiplicity_engines.at(lp.second->agent);

                TreePtr<Node> x = lp.second->x;
                ASSERT( x );
                ContainerInterface *xc = dynamic_cast<ContainerInterface *>(x.get());
                ASSERT(xc)("Multiplicity x must implement ContainerInterface");
                TRACE("Comparing multiplicity link %d size %d\n", i, xc->size());
                
                FOREACH( TreePtr<Node> xe, *xc )
                {
                    e.Compare( xe, &combined_keys );
                }

                i++;
            }

        }
        catch( const ::Mismatch& mismatch )
        {                
#ifdef USE_SOLVER
            TRACE("Miss after recursion, trying next solution\n");
            continue; // Get another solution from the solver
#else
            TRACE("AndRuleEngine miss, trying increment conjecture\n");
            if( conj.Increment() )
                continue; // Conjecture would like us to try again with new choices
                
            // We didn't match and we've run out of choices, so we're done.              
            throw NoSolution();
#endif            
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
void AndRuleEngine::Compare( TreePtr<Node> start_x )
{
    CouplingMap master_keys;
    Compare( start_x, &master_keys );
}


void AndRuleEngine::EnsureChoicesHaveIterators()
{
    conj.EnsureChoicesHaveIterators();
}


const Conjecture &AndRuleEngine::GetConjecture()
{
    return conj;
}


const CouplingMap &AndRuleEngine::GetCouplingKeys()
{
    return my_keys;
}

