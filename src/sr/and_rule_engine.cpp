#include "systemic_constraint.hpp"
#include "simple_solver.hpp"
#include "solver_holder.hpp"
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
#include "match_any_agent.hpp"
#include <list>

#define TEST_PATTERN_QUERY

//#define USE_SOLVER

using namespace SR;

void AndRuleEngine::Configure( Agent *root_agent_, const set<Agent *> &master_agents )
{
    root_agent = root_agent_;

    set<Agent *> normal_agents;
    ConfigPopulateNormalAgents( &normal_agents, root_agent );    
    my_agents = SetDifference( normal_agents, master_agents );       
    if( my_agents.empty() ) 
        return;  // Early-out on trivial problems: TODO do for conjecture mode too; see #126

    set<Agent *> surrounding_agents = SetUnion( master_agents, my_agents ); 
        
    for( std::pair<Agent * const, AndRuleEngine> &pae : my_abnormal_engines )
        pae.second.Configure( pae.first, surrounding_agents );
        
    for( std::pair<Agent * const, AndRuleEngine> &pae : my_multiplicity_engines )
        pae.second.Configure( pae.first, surrounding_agents );

    list<Agent *> normal_agents_ordered;
    master_boundary_agents.clear();    
    reached.clear();
    ConfigPopulateForSolver( &normal_agents_ordered, 
                             root_agent, 
                             master_agents );

    map< Agent *, Agent * > possible_keyer_links; // maps from child to parent
    ConfigDeterminePossibleKeyers( &possible_keyer_links, root_agent, nullptr, master_agents );
    coupling_residual_links.clear();
    ConfigDetermineResiduals( &possible_keyer_links, root_agent, nullptr, master_agents );
    ConfigFilterKeyers(&possible_keyer_links);
#ifdef USE_SOLVER
    list< shared_ptr<CSP::Constraint> > constraints;
    for( Agent *a : my_agents )
    {
        // This bit needs to be improved, see #131
        set<Agent *> cr;
        for( auto p : coupling_residual_links )
            if( p.first == a )
                cr.insert( p.second );
                
        shared_ptr<CSP::Constraint> c = make_shared<CSP::SystemicConstraint>( a, cr );
        my_constraints[a] = c;    
        constraints.push_back(c);
    }
    // Passing in normal_agents_ordered will force SimpleSolver's backtracker to
    // take the same route we do with DecidedCompare(). Note that master agents
    // have not been removed from this list - it's easier if solver knows about 
    // them and we constrain their values explicitly.
    auto salg = make_shared<CSP::SimpleSolver>(constraints, &normal_agents_ordered);
    solver = make_shared<CSP::SolverHolder>(salg);
#else
    conj = make_shared<Conjecture>();
    conj->Configure(my_agents, root_agent);
#endif
}


void AndRuleEngine::ConfigPopulateForSolver( list<Agent *> *normal_agents_ordered, 
                                             Agent *agent,
                                             const set<Agent *> &master_agents )
{
    // Ignore repeated hits
    if( reached.count(agent) > 0 )    
        return; 

    if( master_agents.count( agent ) > 0 )
    {
        normal_agents_ordered->push_back( agent );
        // It's a master boundary variable/agent because:
        // 1. It's a master agent
        // 2. It's not the child of a master agent (we don't recurse on them)
        // See #125
        master_boundary_agents.insert( agent );
        reached.insert(agent);
        return;
    }
    
    reached.insert(agent);
     
    normal_agents_ordered->push_back( agent );
    shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
    FOREACH( shared_ptr<const PatternQuery::Link> b, *pq->GetNormalLinks() )
        ConfigPopulateForSolver( normal_agents_ordered, b->agent, master_agents );        
}


void AndRuleEngine::ConfigDetermineKeyersModuloMatchAny( map< Agent *, Agent * > *possible_keyer_links,
                                                         Agent *agent,
                                                         Agent *parent_agent,
                                                         set<Agent *> *senior_agents,
                                                         set<Agent *> *matchany_agents ) const
{
    if( senior_agents->count( agent ) > 0 )
        return; // will be fixed values for our solver
        
    if( dynamic_cast<MatchAnyAgent *>(agent) )
    {
        matchany_agents->insert( agent );
    }
    else
    {
        ASSERT( possible_keyer_links->count(agent) == 0 );  // See #129, can fail on legal patterns - will also fail on illegal MatchAny couplings
        (*possible_keyer_links)[agent] = parent_agent;
        senior_agents->insert( agent );
             
        shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
        FOREACH( shared_ptr<const PatternQuery::Link> b, *pq->GetNormalLinks() )
            ConfigDetermineKeyersModuloMatchAny( possible_keyer_links, b->agent, agent, senior_agents, matchany_agents );        
    }
}
        
        
void AndRuleEngine::ConfigDeterminePossibleKeyers( map< Agent *, Agent * > *possible_keyer_links,
                                                   Agent *agent,
                                                   Agent *parent_agent,
                                                   set<Agent *> senior_agents ) const
{
    // Scan the senior region. We wish to break off at MatchAny nodes. Senior is the
    // region up to and including a MatchAny; junior is the region under each of its
    // links.
    set<Agent *> my_matchany_agents;
    set<Agent *> my_senior_agents = senior_agents;
    ConfigDetermineKeyersModuloMatchAny( possible_keyer_links, agent, parent_agent, &my_senior_agents, &my_matchany_agents );
    // After this:
    // - my_master_agents has union of master_agents and all the identified keyed agents
    // - my_match_any_agents has the MatchAny agents that we saw, BUT SKIPPED
    
    // Now do all the links under the MatchAny nodes' links. Keying is allowed in each
    // of these junior regions individually, but no cross-keying is allowed if not keyed already.
    // Where that happens, there will be a conflict writing to coupling_keyer_links and the
    // ASSERT will fail.
    for( Agent *ma_agent : my_matchany_agents )
    {
        shared_ptr<PatternQuery> pq = ma_agent->GetPatternQuery();
        FOREACH( shared_ptr<const PatternQuery::Link> b, *pq->GetNormalLinks() )
            ConfigDeterminePossibleKeyers( possible_keyer_links, b->agent, ma_agent, my_senior_agents );        
    }
}
        
        
void AndRuleEngine::ConfigDetermineResiduals( map< Agent *, Agent * > *possible_keyer_links,
                                              Agent *agent,
                                              Agent *parent_agent,
                                              set<Agent *> master_agents ) 
{
    if( (possible_keyer_links->count(agent) > 0 && 
         possible_keyer_links->at(agent) != parent_agent) ||
         master_agents.count(agent) > 0 )
    {
        auto link = make_pair(agent, parent_agent);
        ASSERT( coupling_residual_links.count(link) == 0 ); // See #129, can fail on legal patterns
        coupling_residual_links.insert(link);
        return; 
    }
        
    shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
    FOREACH( shared_ptr<const PatternQuery::Link> b, *pq->GetNormalLinks() )
        ConfigDetermineResiduals( possible_keyer_links, b->agent, agent, master_agents );        
}


void AndRuleEngine::ConfigFilterKeyers(map< Agent *, Agent * > *possible_keyer_links)
{
    coupling_keyer_links.clear();
    for( pair<Agent *, Agent *> keyer_p : *possible_keyer_links )
    {
        for( pair<Agent *, Agent *> residual_p : coupling_residual_links )
        {
            if( residual_p.first == keyer_p.first )
            {
                coupling_keyer_links.insert( keyer_p );
                break;
            }
        }
    }
}


void AndRuleEngine::ConfigPopulateNormalAgents( set<Agent *> *normal_agents, 
                                                Agent *current_agent )
{
    if( normal_agents->count(current_agent) != 0 )
        return; // Only act on couplings the first time they are reached
    
    normal_agents->insert(current_agent);
    
    shared_ptr<PatternQuery> pq = current_agent->GetPatternQuery();
    FOREACH( shared_ptr<const PatternQuery::Link> b, *pq->GetNormalLinks() )
        ConfigPopulateNormalAgents( normal_agents, b->agent );
        
    // Can be nicer in C++17, apparently.
    FOREACH( shared_ptr<const PatternQuery::Link> b, *pq->GetAbnormalLinks() )
        my_abnormal_engines.emplace(std::piecewise_construct,
                                    std::forward_as_tuple(b->agent),
                                    std::forward_as_tuple());    

    FOREACH( shared_ptr<const PatternQuery::Link> b, *pq->GetMultiplicityLinks() )
        my_multiplicity_engines.emplace(std::piecewise_construct,
                                        std::forward_as_tuple(b->agent),
                                        std::forward_as_tuple());
}


void AndRuleEngine::CompareCoupling( Agent *agent,
                                     TreePtr<Node> x,
                                     const CouplingMap *keys )
{
    ASSERT( keys->count(agent) > 0 );

    // This function establishes the policy for couplings in one place.
    // Today, it's SimpleCompare. 
    // And it always will be: see #121; para starting at "No!!"
    static SimpleCompare couplings_comparer;
    if( !couplings_comparer( x, keys->at(agent) ) )
        throw Mismatch();    
}                                     


void AndRuleEngine::CompareLinks( Agent *agent,
                                  shared_ptr<const DecidedQuery> query ) 
{
    FOREACH( shared_ptr<const DecidedQuery::Link> l, *query->GetAbnormalLinks() )
        abnormal_links.insert( l ); 
        
    FOREACH( shared_ptr<const DecidedQuery::Link> l, *query->GetMultiplicityLinks() )
        multiplicity_links.insert( l ); 
    
    int i=0;        
    FOREACH( shared_ptr<const DecidedQuery::Link> l, *query->GetNormalLinks() )
    {
        TRACE("Comparing normal link %d\n", i++);
        // Recurse normally 
        // Get x for linked node
        TreePtr<Node> x = l->x;
        ASSERT( x );
        
        // This is needed for decisionised MatchAny #75. Other schemes for
        // RegisterAlwaysMatchingLink() could be deployed.
        if( x.get() == (Node *)(l->agent) )
            continue; // Pattern nodes immediately match themselves
        
        DecidedCompare(l->agent, agent, x);   
    }
}


void AndRuleEngine::CompareEvaluatorLinks( pair< shared_ptr<BooleanEvaluator>, DecidedQuery::Links > record,
							               const CouplingMap *coupling_keys ) 
{
	ASSERT( record.first );

    // Follow up on any blocks that were noted by the agent impl    
    int i=0;
    list<bool> compare_results;
    FOREACH( shared_ptr<const DecidedQuery::Link> l, record.second )
    {
        // Don't let this link go into the general AND-rule
        abnormal_links.erase( l ); 
        
        TRACE("Comparing block %d\n", i);
 
        // Get x for linked node
        TreePtr<Node> x = l->x;
                                 
        try 
        {
            my_abnormal_engines.at(l->agent).Compare( x, coupling_keys );
            compare_results.push_back( true );
        }
        catch( ::Mismatch & )
        {
            compare_results.push_back( false );
        }

        i++;
    }
    
    shared_ptr<BooleanEvaluator> evaluator = record.first;
	TRACE(" Evaluating ");
	FOREACH(bool b, compare_results)
	    TRACE(b)(" ");
	if( !(*evaluator)( compare_results ) )
        throw EvaluatorFalse();
}


void AndRuleEngine::DecidedCompare( Agent *agent,
                                    Agent *parent_agent,
                                    TreePtr<Node> x )  
{
    INDENT(" ");
    ASSERT( x ); // Target must not be NULL

    // Are we at a residual coupling?
    if( coupling_residual_links.count( make_pair(agent, parent_agent) ) > 0 ) // See #129
    {
        const CouplingMap *keys;
        if( master_boundary_agents.count(agent) > 0 )
            keys = master_keys;
        else
            keys = &my_keys;
        CompareCoupling( agent, x, keys );
        return;
    }

    // Obtain the query state from the conjecture
    shared_ptr<DecidedQuery> query = conj->GetQuery(agent);

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
    // (eg identifier) and we need to have coupled it. The "if" statement
    // tests coupling_keyer_links as well as providing a small optimisation.
    if( coupling_keyer_links.count( make_pair(agent, parent_agent) ) > 0 )
        my_keys[agent] = x;
        
    // Remember we reached this agent 
    reached.insert( agent );

    // Use worker functions to go through the links, special case if there is evaluator
    if( query->GetEvaluator() )
    {        
        evaluator_records.insert( make_pair( query->GetEvaluator(), *query->GetAbnormalLinks() ) );
	}

    TRACE(*agent)("?=")(*x)(" Comparing links\n");
    CompareLinks( agent, query );

    // Fill this on the way out- by now I think we've succeeded in matching the current conjecture.
    solution_keys[agent] = x;

    TRACE("OK\n");
}


void AndRuleEngine::ExpandDomain( Agent *agent, set< TreePtr<Node> > &domain )
{
    // It's important that this walk hits parents first because local node 
    // transformations occur in parent-then-child order. That's why this 
    // part is here and not in the CSP stuff: it exploits knowlege of 
    // the directedenss of the pattern trees.
    my_constraints.at(agent)->ExpandDomain( domain );  
    
    shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
    FOREACH( shared_ptr<const PatternQuery::Link> b, *pq->GetNormalLinks() )
        ExpandDomain( b->agent, domain );
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

    if( my_agents.empty() )
    {
        // Trivial case: we have no agents, so there won't be any decisions
        // and so no problem to solve. Spare all algorithms the hassle of 
        // dealing with this. Root agent should have been keyed by master,
        // otherwise it'd be in my_agents.
        ASSERT( master_keys->count(root_agent) );
        try
        {            
            CompareCoupling( root_agent, start_x, master_keys );
        }
        catch( const ::Mismatch& mismatch ) 
        {
            throw NoSolution();
        }
        return;
    }
           
#ifdef USE_SOLVER
    set< TreePtr<Node> > domain;
	Walk wx( start_x ); 
	for( Walk::iterator wx_it=wx.begin(); wx_it!=wx.end(); ++wx_it )
        domain.insert(*wx_it);
    
    CouplingMap forces = *master_keys;
    forces[root_agent] = start_x;
    // TODO use the forces #125
    
    // Expand the domain to include generated child y nodes.
    ExpandDomain( root_agent, domain );
    
    solver->Start( domain );
#else
    conj->Start();
#endif
           
    // Create the conjecture object we will use for this compare, and keep iterating
    // though different conjectures trying to find one that allows a match.
    //int i=0;
    while(1)
    {
        solution_keys.clear();
        my_keys.clear();
#ifdef USE_SOLVER        
        // Get a solution from the solver
        CSP::SideInfo side_info;
        map< shared_ptr<CSP::Constraint>, list< TreePtr<Node> > > values;
        bool match = solver->GetNextSolution( &values, &side_info );        
        if( !match )
            throw NoSolution();

        // Recreate my_keys
        for( pair< Agent *, shared_ptr<CSP::Constraint> > p : my_constraints )
        {
            list< TreePtr<Node> > &vals = values.at(p.second);
            list< Agent * > vars = p.second->GetVariables();
            ASSERT( vars.front() == p.first );
            solution_keys[vars.front()] = vals.front(); // For now only do the first one, which is 
        }
        
        // Grab the side info into our containers
        abnormal_links = side_info.abnormal_links;
        multiplicity_links = side_info.multiplicity_links;
        evaluator_records = side_info.evaluator_records;
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
            evaluator_records.clear();
            DecidedCompare( root_agent, nullptr, start_x );
#endif
            CouplingMap combined_keys = MapUnion( *master_keys, solution_keys );     
                                                
            // Process the evaluator queries. These can match when their children have not matched and
            // we wouldn't be able to reliably key those children so we process them in a post-pass 
            // which ensures all the couplings have been keyed already.
            // Examples are MatchAny and NotMatch (but not MatchAll, because MatchAll conforms with
            // the global and-rule and so its children can key couplings.
            for( pair< shared_ptr<BooleanEvaluator>, DecidedQuery::Links > record : evaluator_records )
            {
                //TRACE(*query)(" Comparing evaluator query\n"); TODO get useful trace off queries
                CompareEvaluatorLinks( record, &combined_keys );
            }

            // Process the free abnormal links.
            for( shared_ptr<const DecidedQuery::Link> lp : abnormal_links )
            {            
                AndRuleEngine &e = my_abnormal_engines.at(lp->agent);
                TreePtr<Node> xe = lp->x;
                e.Compare( xe, &combined_keys );
            }

            // Process the free multiplicity links.
            int i=0;
            for( shared_ptr<const DecidedQuery::Link> lp : multiplicity_links )
            {            
                AndRuleEngine &e = my_multiplicity_engines.at(lp->agent);

                TreePtr<Node> x = lp->x;
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
            if( conj->Increment() )
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
#ifndef USE_SOLVER
    conj->EnsureChoicesHaveIterators();
#endif
}


const CouplingMap &AndRuleEngine::GetCouplingKeys()
{
    return solution_keys;
}

