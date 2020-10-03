#include "csp/systemic_constraint.hpp"
#include "csp/simple_solver.hpp"
#include "csp/solver_holder.hpp"
#include "scr_engine.hpp"
#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "helpers/simple_compare.hpp"
#include "agents/slave_agent.hpp"
#include "agents/standard_agent.hpp"
#include "agents/overlay_agent.hpp"
#include "agents/search_container_agent.hpp"
#include "common/common.hpp"
#include "agents/match_any_agent.hpp"
#include "link.hpp"
#include "tree/cpptree.hpp"

#include "and_rule_engine.hpp"

#include <list>

//#define TEST_PATTERN_QUERY

//#define USE_SOLVER

// After-pass restricts based on evaluators, free abnormals and
// multiplicities. REGENERATE is the new algo that figures out where
// they are based on normals, using RunNormalLocatedQuery().
#define REGENERATE_AFTER_PASS_KEYS

// EXTRACT is the old algo that gets them from CompareLinks() (in 
// DC solver) or diversions (CSP solver). Latter probably broken.
// Enable both and ASSERTs will confirm they match.
#define EXTRACT_AFTER_PASS_KEYS

using namespace SR;

AndRuleEngine::AndRuleEngine( Agent *root_agent_, const set<Agent *> &master_agents_ ) :
    plan( this, root_agent_, master_agents_ )
{
}    
 
AndRuleEngine::Plan::Plan( AndRuleEngine *algo_, Agent *root_agent_, const set<Agent *> &master_agents_) :
    algo( algo_ )
{
    TRACE(GetName());
    INDENT(" ");
    root_agent = root_agent_;
    master_agents = master_agents_;
    
    set<Agent *> normal_agents;
    PopulateNormalAgents( &normal_agents, root_agent );    
    my_normal_agents = SetDifference( normal_agents, master_agents );       
    if( my_normal_agents.empty() ) 
        return;  // Early-out on trivial problems: TODO do for conjecture mode too; see #126

    set<Agent *> surrounding_agents = SetUnion( master_agents, my_normal_agents );         
    CreateVariousThings( normal_agents, surrounding_agents );    
        
    list<Agent *> normal_agents_ordered;
    master_boundary_agents.clear();    
    master_boundary_links.clear();
    reached.clear();
    PopulateForSolver( &normal_agents_ordered, 
                             root_agent, 
                             master_agents );

    set<PatternLink> possible_keyer_links; // maps from child to parent
    DeterminePossibleKeyers( &possible_keyer_links, root_agent, master_agents );
    coupling_residual_links.clear();
    DetermineResiduals( &possible_keyer_links, root_agent, master_agents );
    FilterKeyers(&possible_keyer_links);
#ifdef USE_SOLVER    
    list< shared_ptr<CSP::Constraint> > constraints;
    for( Agent *constraint_agent : my_normal_agents )
    {        
        CSP::VariableQueryLambda vql = [&](Agent *link_agent) -> pair<CSP::VariableFlags, Agent *>
        {
            CSP::VariableFlags flags;
            
            shared_ptr<PatternQuery> pq = constraint_agent->GetPatternQuery();
            PatternLink link;
            for( PatternLink cur_link : pq->GetAllLinks() )
            {
                if( cur_link.GetChildAgent() == link_agent )
                {
                    ASSERT(!link);
                    ASSERT(cur_link);
                    link = cur_link; // Found a link the the agent the constaint mentioned: not safe! TODO could be parallel links!
                }
            }
                    
            if( link_agent==constraint_agent ) // Self-variable must be by location
                flags.compare_by = CSP::CompareBy::LOCATION;   
            else if( coupling_residual_links.count(link) > 0 ) // Coupling residuals are by value
                flags.compare_by = CSP::CompareBy::VALUE;
            else if( master_boundary_agents.count(link_agent) > 0) // Couplings to master are by value
                flags.compare_by = CSP::CompareBy::VALUE;
            else
                flags.compare_by = CSP::CompareBy::LOCATION;   
                                 
            if( link_agent==root_agent ) // Root variable will be forced
                flags.freedom = CSP::Freedom::FORCED;
            else if( master_boundary_agents.count(link_agent) > 0) // Couplings to master are forced
                flags.freedom = CSP::Freedom::FORCED;
            else
                flags.freedom = CSP::Freedom::FREE;
            
            if( diversion_agents.count(link) > 0 )
                flags.correspondance = CSP::Correspondance::DIVERTED;
            else
                flags.correspondance = CSP::Correspondance::DIRECT;
            
            if( flags.correspondance == CSP::Correspondance::DIVERTED )
                return make_pair(flags, diversion_agents.at(link).get());
            else
                return make_pair(flags, nullptr);            
        };
                
        shared_ptr<CSP::Constraint> c = make_shared<CSP::SystemicConstraint>( constraint_agent, vql );
        my_constraints[constraint_agent] = c;    
        constraints.push_back(c);
    }
    // Passing in normal_agents_ordered will force SimpleSolver's backtracker to
    // take the same route we do with DecidedCompare(). Need to remove FORCED agents
    // though.
    list<Agent *> free_normal_agents_ordered;
    for( Agent *agent : normal_agents_ordered )
    {
        if( agent != root_agent &&
            master_boundary_agents.count(agent) == 0 )
            free_normal_agents_ordered.push_back( agent );
    }
    auto salg = make_shared<CSP::SimpleSolver>(constraints, &free_normal_agents_ordered);
    solver = make_shared<CSP::SolverHolder>(salg);
#else
    conj = make_shared<Conjecture>(my_normal_agents, root_agent);
#endif
    compare_by_value_links = SetUnion( coupling_residual_links, 
                                       master_boundary_links );                              
}


void AndRuleEngine::Plan::PopulateForSolver( list<Agent *> *normal_agents_ordered, 
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
    FOREACH( PatternLink link, pq->GetNormalLinks() )
    {
        PopulateForSolver( normal_agents_ordered, link.GetChildAgent(), master_agents );        
        
        // Note: here, we won't see root if root is a master agent (i.e. trivial pattern)
        if( master_boundary_agents.count( link.GetChildAgent() ) )
            master_boundary_links.insert( link );
    }
}


void AndRuleEngine::Plan::DetermineKeyersModuloMatchAny( set<PatternLink> *possible_keyer_links,
                                                         Agent *agent,
                                                         set<Agent *> *senior_agents,
                                                         set<Agent *> *matchany_agents ) const
{
    shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
    FOREACH( PatternLink link, pq->GetNormalLinks() )
    {
        if( senior_agents->count( link.GetChildAgent() ) > 0 )
            continue; // will be fixed values for our solver
            
        if( dynamic_cast<MatchAnyAgent *>(link.GetChildAgent()) )
        {
            matchany_agents->insert( link.GetChildAgent() );
            continue;
        }

        // See #129, can fail on legal patterns - will also fail on illegal MatchAny couplings
        for( PatternLink l : *possible_keyer_links )        
            ASSERT( l.GetChildAgent() != link.GetChildAgent() )
                  ("Conflicting coupling in and-rule pattern: check MatchAny nodes\n");

        possible_keyer_links->insert(link);
        senior_agents->insert( link.GetChildAgent() );
    
        DetermineKeyersModuloMatchAny( possible_keyer_links, link.GetChildAgent(), senior_agents, matchany_agents );        
    }
}
        
        
void AndRuleEngine::Plan::DeterminePossibleKeyers( set<PatternLink> *possible_keyer_links,
                                                   Agent *agent,
                                                   set<Agent *> senior_agents ) const
{
    // Scan the senior region. We wish to break off at MatchAny nodes. Senior is the
    // region up to and including a MatchAny; junior is the region under each of its
    // links.
    set<Agent *> my_matchany_agents;
    set<Agent *> my_senior_agents = senior_agents;
    DetermineKeyersModuloMatchAny( possible_keyer_links, agent, &my_senior_agents, &my_matchany_agents );
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
        FOREACH( PatternLink link, pq->GetNormalLinks() )
        {
            DeterminePossibleKeyers( possible_keyer_links, link.GetChildAgent(), my_senior_agents );        
        }
    }
}
        
        
void AndRuleEngine::Plan::DetermineResiduals( set<PatternLink> *possible_keyer_links,
                                              Agent *agent,
                                              set<Agent *> master_agents ) 
{
    shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
    FOREACH( PatternLink link, pq->GetNormalLinks() )
    {            
        PatternLink keyer;
        for( PatternLink l : *possible_keyer_links )
        {
            if( l.GetChildAgent() == link.GetChildAgent() )
                keyer = l; // keyer keys the same child node that we're looking at
        }
        
        if( keyer && keyer != link )
        {
            ASSERT( coupling_residual_links.count(link) == 0 );
            coupling_residual_links.insert(link);
            continue; // Coupling residuals do not recurse (keyer does that and it only needs to be done once)
        }
        
        DetermineResiduals( possible_keyer_links, link.GetChildAgent(), master_agents );        
    }
}


void AndRuleEngine::Plan::FilterKeyers(set<PatternLink> *possible_keyer_links)
{
    coupling_keyer_links.clear();
    for( PatternLink keyer_l : *possible_keyer_links )
    {
        for( PatternLink residual_l : coupling_residual_links )
        {
            if( residual_l.GetChildAgent() == keyer_l.GetChildAgent() )
            {
                coupling_keyer_links.insert( keyer_l );
                break;
            }
        }
    }
}


void AndRuleEngine::Plan::PopulateNormalAgents( set<Agent *> *normal_agents, 
                                                Agent *agent )
{
    if( normal_agents->count(agent) != 0 )
        return; // Only act on couplings the first time they are reached
    normal_agents->insert(agent);

    shared_ptr<PatternQuery> pq = agent->GetPatternQuery();   
    FOREACH( PatternLink link, pq->GetNormalLinks() )
        PopulateNormalAgents( normal_agents, link.GetChildAgent() );        
}


void AndRuleEngine::Plan::CreateVariousThings( const set<Agent *> &normal_agents, 
                                               const set<Agent *> &surrounding_agents )
{
    for( auto agent : normal_agents )
    {
        shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
            
        if( pq->GetEvaluator() )
            my_evaluators.insert(agent);
        
        FOREACH( PatternLink link, pq->GetAbnormalLinks() )
        {        
            if( pq->GetEvaluator() )
            {
                my_evaluator_abnormal_engines[link] = make_shared<AndRuleEngine>( link.GetChildAgent(), 
                                                                                  surrounding_agents );  
            }
            else
            {
                my_free_abnormal_engines[link] = make_shared<AndRuleEngine>( link.GetChildAgent(), 
                                                                             surrounding_agents );  
            }
            diversion_agents[link] = make_shared<PlaceholderAgent>(); 
        }
        
        FOREACH( PatternLink link, pq->GetMultiplicityLinks() )
        {
            my_multiplicity_engines[link] = make_shared<AndRuleEngine>( link.GetChildAgent(), 
                                                                        surrounding_agents );  
            diversion_agents[link] = make_shared<PlaceholderAgent>(); 
        }
    }
}


void AndRuleEngine::ExpandDomain( Agent *agent, set< TreePtr<Node> > &domain )
{
    // It's important that this walk hits parents first because local node 
    // transformations occur in parent-then-child order. That's why this 
    // part is here and not in the CSP stuff: it exploits knowlege of 
    // the directedenss of the pattern trees.
    plan.my_constraints.at(agent)->ExpandDomain( domain );  
    
    shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
    FOREACH( PatternLink link, pq->GetNormalLinks() )
        ExpandDomain( link.GetChildAgent(), domain );
}


void AndRuleEngine::StartCSPSolver( TreePtr<Node> start_x )
{
    set< TreePtr<Node> > domain;
	Walk wx( start_x ); 
	for( Walk::iterator wx_it=wx.begin(); wx_it!=wx.end(); ++wx_it )
        domain.insert(*wx_it);
    
    CouplingMap forces = *master_keys;
    forces[plan.root_agent] = start_x;
    for( pair< Agent *, shared_ptr<CSP::Constraint> > p : plan.my_constraints )
        p.second->SetForces( forces );
    
    // Expand the domain to include generated child y nodes.
    ExpandDomain( plan.root_agent, domain );
    
    plan.solver->Start( domain );
}


void AndRuleEngine::GetNextCSPSolution()
{
    map< shared_ptr<CSP::Constraint>, list< TreePtr<Node> > > values;
    bool match = plan.solver->GetNextSolution( &values );        
    if( !match )
        throw NoSolution();

    // Recreate working_keys
    for( pair< Agent *, shared_ptr<CSP::Constraint> > p : plan.my_constraints )
    {
        list< TreePtr<Node> > &vals = values.at(p.second);
        list< Agent * > vars = p.second->GetFreeVariables();
        ASSERT( vars.front() == p.first );
        if( plan.my_normal_agents.count(vars.front()) )
            solution_keys[vars.front()] = vals.front(); 
#ifdef EXTRACT_AFTER_PASS_KEYS
        else
            extracted_after_pass_keys[vars.front()] = vals.front();
        // Note: suspect this doesn't work, and that you need to loop
        // over the constraint's variables and check through them to
        // find the abnormals, because diversion links/variables don't 
        // have their own constraints.
#endif                
    }
}


void AndRuleEngine::CompareCoupling( Agent *agent,
                                     TreePtr<Node> x,
                                     const CouplingMap *keys )
{
    ASSERT( keys->count(agent) > 0 );

    // Enforce rule #149
    ASSERT( !TreePtr<SubContainer>::DynamicCast( keys->at(agent) ) ); 

    // Allow Magic Match Anything 
    if( x == DecidedQueryCommon::MMAX_Node )
        return;

    // This function establishes the policy for couplings in one place.
    // Today, it's SimpleCompare. 
    // And it always will be: see #121; para starting at "No!!"
    static SimpleCompare couplings_comparer;
    if( !couplings_comparer( x, keys->at(agent) ) )
        throw Mismatch();    
}                                     


void AndRuleEngine::KeyCoupling( Agent *agent,
                                 TreePtr<Node> x,
                                 CouplingMap *keys )
{
    ASSERT( keys->count(agent) == 0 )("Coupling conflict!\n");    
    (*keys)[agent] = x;
}                                     


void AndRuleEngine::CompareLinks( Agent *agent,
                                  shared_ptr<const DecidedQuery> query ) 
{    
    FOREACH( const LocatedLink &link, query->GetNormalLinks() )
    {
        TRACE("Comparing normal link ")(link)(" keyer? %d residual? %d master? %d\n", plan.coupling_keyer_links.count( link ), plan.coupling_residual_links.count( link ), plan.master_boundary_links.count(link) );
        // Recurse normally 
        // Get x for linked node
        TreePtr<Node> x = link.GetChildX();
        ASSERT( x );
        
        // This is needed for decisionised MatchAny #75. Other schemes for
        // RegisterAlwaysMatchingLink() could be deployed.
        //if( x == DecidedQueryCommon::MMAX_Node )
        //    continue; // Pattern nodes immediately match themselves
        
        // Are we at a residual coupling?
        if( plan.coupling_residual_links.count( link ) > 0 ) // See #129
        {
            CompareCoupling( link.GetChildAgent(), x, &working_keys );
            TRACE("Accepted normal coupling for ")(link.GetChildAgent())(" x=")(x)(" key=")(working_keys.at(link.GetChildAgent()))("\n");
            continue;
        }
        
        // Master couplings are now checked in a post-pass
        if( plan.master_boundary_links.count(link) > 0 )
        {
            master_coupling_candidates[link.GetChildAgent()] = x;
            continue;
        }

        // Remember the coupling before recursing, as we can hit the same node 
        // (eg identifier) and we need to have coupled it. The "if" statement
        // tests coupling_keyer_links as well as providing a small optimisation.
        if( plan.coupling_keyer_links.count( link ) )
        {
            ASSERT( x != DecidedQueryCommon::MMAX_Node )("Can't key with MMAX because would leak");
            KeyCoupling( link.GetChildAgent(), x, &working_keys );
        }

        DecidedCompare(link.GetChildAgent(), x);   
    }
    
#ifdef EXTRACT_AFTER_PASS_KEYS
    // Fill this on the way out- by now I think we've succeeded in matching the current conjecture.
    FOREACH( const LocatedLink &link, query->GetAbnormalLinks() )
    {
        ASSERT( plan.diversion_agents.count(link) );
        Agent *diversion_agent = plan.diversion_agents.at(link).get();
        KeyCoupling( diversion_agent, link.GetChildX(), &extracted_after_pass_keys );
    }
        
    FOREACH( const LocatedLink &link, query->GetMultiplicityLinks() )
    {
        ASSERT( plan.diversion_agents.count(link) );
        Agent *diversion_agent = plan.diversion_agents.at(link).get();
        KeyCoupling( diversion_agent, link.GetChildX(), &extracted_after_pass_keys );
    }
#endif    
}


void AndRuleEngine::DecidedCompare( Agent *agent,
                                    TreePtr<Node> x )  
{
    INDENT(" ");
    ASSERT( x ); // Target must not be NULL

    // Obtain the query state from the conjecture
    shared_ptr<DecidedQuery> query = plan.conj->GetQuery(agent);

    // Run the compare implementation to get the links based on the choices
    TRACE(*agent)(" ?= ")(*x)(" RunDecidedQuery()\n");     
    agent->RunDecidedQuery( *query, x );
    TRACEC("Normal ")(query->GetNormalLinks())("\n")
          ("Abormal ")(query->GetAbnormalLinks())("\n")
          ("Multiplicity ")(query->GetMultiplicityLinks())("\n");  
#ifdef TEST_PATTERN_QUERY
    shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
    ASSERT( pq->GetNormalLinks() == query->GetNormalLinks() &&
            pq->GetAbnormalLinks() == query->GetAbnormalLinks() &&
            pq->GetMultiplicityLinks() == query->GetMultiplicityLinks() &&
            pq->GetDecisions().size() == query->GetDecisions().size() )
          ("PatternQuery disagrees with DecidedQuery!!!!\n")
          ("GetNormalLinks().size() : %d vs %d\n", pq->GetNormalLinks().size(), query->GetNormalLinks().size() )
          ("GetAbnormalLinks().size() : %d vs %d\n", pq->GetAbnormalLinks().size(), query->GetAbnormalLinks().size() )
          ("GetMultiplicityLinks().size() : %d vs %d\n", pq->GetMultiplicityLinks().size(), query->GetMultiplicityLinks().size() )
          ("GetDecisions().size() : %d vs %d\n", pq->GetDecisions().size(), query->GetDecisions().size() )
          (*agent);
#endif
                                          
    CompareLinks( agent, query );

    // Fill this on the way out- by now I think we've succeeded in matching the current conjecture.
    ASSERT( solution_keys.count(agent) == 0 )("Coupling conflict!\n");
    KeyCoupling( agent, x, &solution_keys );

    TRACE("OK\n");
}


void AndRuleEngine::CompareEvaluatorLinks( Agent *agent, 
                                           const CouplingMap *combined_keys, 
                                           const CouplingMap *after_pass_keys ) 
{
    auto pq = agent->GetPatternQuery();
    shared_ptr<BooleanEvaluator> evaluator = pq->GetEvaluator();
	ASSERT( evaluator );

    // Follow up on any blocks that were noted by the agent impl    
    int i=0;
    list<bool> compare_results;
    FOREACH( PatternLink link, pq->GetAbnormalLinks() )
    {
        TRACE("Comparing block %d\n", i);
 
        // Get x for linked node
        Agent *diversion_agent = plan.diversion_agents.at(link).get();
        TreePtr<Node> x = after_pass_keys->at(diversion_agent);
                                
        try 
        {
            plan.my_evaluator_abnormal_engines.at(link)->Compare( x, combined_keys );
            compare_results.push_back( true );
        }
        catch( ::Mismatch & )
        {
            compare_results.push_back( false );
        }

        i++;
    }
    
	TRACE(" Evaluating ");
	FOREACH(bool b, compare_results)
	    TRACE(b)(" ");
	if( !(*evaluator)( compare_results ) )
        throw EvaluatorFalse();
}


void AndRuleEngine::CompareFreeAbnormalLinks( PatternLink link, 
                                              const CouplingMap *combined_keys, 
                                              const CouplingMap *after_pass_keys ) 
{
    shared_ptr<AndRuleEngine> e = plan.my_free_abnormal_engines.at(link);
    Agent *diversion_agent = plan.diversion_agents.at(link).get();
    TRACE("Checking free abnormal ")(link)(" diversion=")(diversion_agent)("\n");
    TRACEC("HSK ")(after_pass_keys)("\n");
    
    if( after_pass_keys->count(diversion_agent) ) // agent can be missing from after_pass_keys, not sure why
    {
        TreePtr<Node> x = after_pass_keys->at(diversion_agent);  
               
        e->Compare( x, combined_keys );
        
        // Free abnormal links are AND-rule and singular so they can key
        KeyCoupling( link.GetChildAgent(), x, &solution_keys );
    }
}


void AndRuleEngine::CompareMultiplicityLinks( PatternLink link, 
                                              const CouplingMap *combined_keys, 
                                              const CouplingMap *after_pass_keys ) 
{
    shared_ptr<AndRuleEngine> e = plan.my_multiplicity_engines.at(link);
    Agent *diversion_agent = plan.diversion_agents.at(link).get();
    TRACE("Checking multiplicity ")(link)(" diversion=")(diversion_agent)("\n");
    TRACEC("HSK ")(after_pass_keys)("\n");
    TreePtr<Node> x = after_pass_keys->at(diversion_agent);
        
    ASSERT( x );
    ContainerInterface *xc = dynamic_cast<ContainerInterface *>(x.get());
    ASSERT(xc)("Multiplicity x must implement ContainerInterface");
    
    FOREACH( TreePtr<Node> xe, *xc )
    {
        e->Compare( xe, combined_keys );
    }
}


void AndRuleEngine::CompareAfterPassRegenerate()
{
    const CouplingMap combined_keys = MapUnion( *master_keys, solution_keys );    

    CouplingMap regenerated_after_pass_keys;
    for( auto agent : plan.my_normal_agents )
    {
        TreePtr<Node> x = solution_keys.at(agent);
        auto pq = agent->GetPatternQuery();
        TRACE("In after-pass, trying to regenerate ")(*agent)(" at ")(*x)("\n");    
        TRACEC("Pattern links ")(pq->GetNormalLinks())("\n");    
        TRACEC("Based on ")(combined_keys)("\n");    
        list<LocatedLink> ll = LocateLinksFromMap( pq->GetNormalLinks(), combined_keys );
        TRACEC("Relocated links ")(ll)("\n");    
        auto query = make_shared<DecidedQuery>(pq);
        Conjecture conj(agent, query);            
        conj.Start();
        try
        {
            agent->IncrementNormalLinkedQuery( conj, x, ll, plan.compare_by_value_links );
        }
        catch( const ::Mismatch& mismatch )
        {
            ASSERT(false)("Unexpected mismatch thrown from RunNormalLinkedQuery(): ")(mismatch)("\n");                    
        }
        
        combined_keys.clear();
        // Fill this on the way out- by now I think we've succeeded in matching the current conjecture.
        FOREACH( const LocatedLink &link, query->GetAbnormalLinks() )
        {
            ASSERT( plan.diversion_agents.count(link) );
            Agent *diversion_agent = plan.diversion_agents.at(link).get();
#ifdef EXTRACT_AFTER_PASS_KEYS // Need both methods to double-check            
            AssertNewCoupling( extracted_after_pass_keys, diversion_agent, link.GetChildX(), agent ); 
#endif
            KeyCoupling( diversion_agent, link.GetChildX(), &regenerated_after_pass_keys );            
        }                    
        FOREACH( const LocatedLink &link, query->GetMultiplicityLinks() )
        {
            ASSERT( plan.diversion_agents.count(link) );
            Agent *diversion_agent = plan.diversion_agents.at(link).get();
#ifdef EXTRACT_AFTER_PASS_KEYS // Need both methods to double-check            
            AssertNewCoupling( extracted_after_pass_keys, diversion_agent, link.GetChildX(), agent ); 
#endif
            KeyCoupling( diversion_agent, link.GetChildX(), &regenerated_after_pass_keys );
        }
        // Process if an evaluator agent.
        if( plan.my_evaluators.count( agent ) > 0 )
            CompareEvaluatorLinks( agent, &combined_keys, &regenerated_after_pass_keys );
    }      
    ASSERT( regenerated_after_pass_keys.size() == extracted_after_pass_keys.size() )
          ("regenerated keys ")(regenerated_after_pass_keys)("\n")
          ("extracted keys   ")(extracted_after_pass_keys)("\n");
          
    

    // Process the free abnormal links.
    for( const std::pair< const PatternLink, shared_ptr<AndRuleEngine> > &pae : plan.my_free_abnormal_engines )
        CompareFreeAbnormalLinks( pae.first, &combined_keys, &regenerated_after_pass_keys );

    // Process the free multiplicity links.
    for( const std::pair< const PatternLink, shared_ptr<AndRuleEngine> > &pae : plan.my_multiplicity_engines )
        CompareMultiplicityLinks( pae.first, &combined_keys, &regenerated_after_pass_keys );        
}


void AndRuleEngine::CompareAfterPassExtractOnly()
{
    const CouplingMap combined_keys = MapUnion( *master_keys, solution_keys );    

    // Process the evaluator agents.
    for( Agent *agent : plan.my_evaluators )
        CompareEvaluatorLinks( agent, &combined_keys, &extracted_after_pass_keys );

    // Process the free abnormal links.
    for( const std::pair< const PatternLink, shared_ptr<AndRuleEngine> > &pae : plan.my_free_abnormal_engines )
        CompareFreeAbnormalLinks( pae.first, &combined_keys, &extracted_after_pass_keys );

    // Process the free multiplicity links.
    for( const std::pair< const PatternLink, shared_ptr<AndRuleEngine> > &pae : plan.my_multiplicity_engines )
        CompareMultiplicityLinks( pae.first, &combined_keys, &extracted_after_pass_keys );        
}


void AndRuleEngine::CompareTrivialProblem( TreePtr<Node> start_x )
{
    // Trivial case: we have no agents, so there won't be any decisions
    // and so no problem to solve. Spare all algorithms the hassle of 
    // dealing with this. Root agent should have been keyed by master,
    // otherwise it'd be in my_normal_agents.
    ASSERT( master_keys->count(plan.root_agent) );
    try
    {            
        CompareCoupling( plan.root_agent, start_x, master_keys );
    }
    catch( const ::Mismatch& mismatch ) 
    {
        throw NoSolution();
    }
    return;
}


void AndRuleEngine::CompareMasterKeys()
{
    for( auto link : plan.master_boundary_links )
    {
        auto x = master_coupling_candidates.at(link.GetChildAgent());
        CompareCoupling( link.GetChildAgent(), x, master_keys );
        TRACE("Accepted master coupling for ")(link.GetChildAgent())(" x=")(x)(" key=")(master_keys->at(link.GetChildAgent()))("\n");
    }
}


// This one if you want the resulting couplings and conj (ie doing a replace imminently)
void AndRuleEngine::Compare( TreePtr<Node> start_x,
                             const CouplingMap *master_keys_ )
{
    INDENT("C");
    ASSERT( start_x );
    TRACE("Compare x=")(*start_x)(" pattern=")(*plan.root_agent)("\n");
           
    master_keys = master_keys_;    

    if( plan.my_normal_agents.empty() )
    {
        CompareTrivialProblem( start_x );
        return;
    }
           
#ifdef USE_SOLVER
    StartCSPSolver( start_x, master_keys );
#else
    plan.conj->Start();
#endif
           
    // Create the conjecture object we will use for this compare, and keep iterating
    // though different conjectures trying to find one that allows a match.
    //int i=0;
    while(1)
    {
#ifdef EXTRACT_AFTER_PASS_KEYS 
        extracted_after_pass_keys.clear();
#endif
        solution_keys.clear();
        working_keys.clear();
#ifdef USE_SOLVER        
        // Get a solution from the solver
        GetNextCSPSolution();
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
            master_coupling_candidates.clear();            
            DecidedCompare( plan.root_agent, start_x );            
            CompareMasterKeys();
#endif
#ifdef REGENERATE_AFTER_PASS_KEYS
            CompareAfterPassRegenerate();
#else
            CompareAfterPassExtractOnly();
#endif
        }
        catch( const ::Mismatch& mismatch )
        {                
#ifdef USE_SOLVER
            TRACE("Miss after recursion, trying next solution\n");
            continue; // Get another solution from the solver
#else
            TRACE("AndRuleEngine miss, trying increment conjecture\n");
            if( plan.conj->Increment() )
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
    plan.conj->EnsureChoicesHaveIterators();
#endif
}


const CouplingMap &AndRuleEngine::GetCouplingKeys()
{
    return solution_keys;
}


void AndRuleEngine::AssertNewCoupling( const CouplingMap &old, Agent *new_agent, TreePtr<Node> new_x, Agent *parent_agent )
{
    ASSERT( old.count(new_agent) == 1 );
    if( TreePtr<SubContainer>::DynamicCast(new_x) )
    {                    
        SimpleCompare sc;
        bool same  = sc( old.at(new_agent), new_x );
        if( !same )
        {
            FTRACE("New x ")(new_x)(" mismatches old x ")(old.at(new_agent))
                  (" for agent ")(new_agent)(" with parent ")(parent_agent)("\n");
            if( TreePtr<SubSequence>::DynamicCast(new_x) && TreePtr<SubSequence>::DynamicCast(old.at(new_agent)))
                FTRACEC("SubSequence\n");
            else if( TreePtr<SubSequenceRange>::DynamicCast(new_x) && TreePtr<SubSequenceRange>::DynamicCast(old.at(new_agent)))
                FTRACEC("SubSequenceRange\n");
            else if( TreePtr<SubCollection>::DynamicCast(new_x) && TreePtr<SubCollection>::DynamicCast(old.at(new_agent)))
                FTRACEC("SubCollections\n");
            else
                FTRACEC("Container types don't match\n");
            ContainerInterface *xc = dynamic_cast<ContainerInterface *>(old.at(new_agent).get());
            FOREACH( TreePtr<Node> n, *xc )
                FTRACEC("old: ")( n )("\n");
            xc = dynamic_cast<ContainerInterface *>(new_x.get());
            FOREACH( TreePtr<Node> n, *xc )
                FTRACEC("new: ")( n )("\n");
            ASSERTFAIL("AssertNewCoupling() failure");                                                
        }
    }
    else
    {
        ASSERT( old.at(new_agent) == new_x );
    }
}
