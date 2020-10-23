#include "csp/systemic_constraint.hpp"
#include "csp/simple_solver.hpp"
#include "csp/solver_holder.hpp"
#include "scr_engine.hpp"
#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "agents/slave_agent.hpp"
#include "agents/standard_agent.hpp"
#include "agents/overlay_agent.hpp"
#include "agents/search_container_agent.hpp"
#include "common/common.hpp"
#include "agents/match_any_agent.hpp"
#include "link.hpp"
#include "tree/cpptree.hpp"
#include "equivalence.hpp"

#include "and_rule_engine.hpp"

#include <list>

//#define TEST_PATTERN_QUERY

//#define USE_SOLVER

using namespace SR;

AndRuleEngine::AndRuleEngine( TreePtr<Node> root_pattern_, const set<Agent *> &master_agents_ ) :
    plan( this, root_pattern_, master_agents_ )
{
}    
 
AndRuleEngine::Plan::Plan( AndRuleEngine *algo_, TreePtr<Node> root_pattern_, const set<Agent *> &master_agents_) :
    algo( algo_ )
{
    TRACE(GetName());
    INDENT("P");
    root_pattern = root_pattern_;
    root_agent = Agent::AsAgent(root_pattern);
    master_agents = master_agents_;
    
    // For closure under semi-link model, we need a link to root, and therefore a parent of root
    closure_pattern = MakePatternPtr< MatchAll<Node> >();
    closure_pattern->patterns = (root_pattern);
    root_pattern_link = PatternLink( Agent::AsAgent(closure_pattern), &closure_pattern->patterns.front() );
    
    set<Agent *> normal_agents;
    set<PatternLink> normal_links;
    PopulateNormalAgents( &normal_agents, &normal_links, root_pattern_link );    
    for( PatternLink plink : normal_links )
        if( master_agents.count( plink.GetChildAgent() ) == 0 )
            my_normal_links.insert( plink );
            
    my_normal_agents = SetDifference( normal_agents, master_agents );       
    if( my_normal_agents.empty() ) 
        return;  // Early-out on trivial problems: TODO do for conjecture mode too; see #126

    set<Agent *> surrounding_agents = SetUnion( master_agents, my_normal_agents );         
    CreateVariousThings( normal_agents, surrounding_agents );    
        
    master_boundary_agents.clear();    
    master_boundary_links.clear();
    reached.clear();
    PopulateForSolver( root_agent, 
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
        CSP::VariableQueryLambda vql = [&](Agent *link_agent) -> CSP::VariableFlags
        {
            CSP::VariableFlags flags;
            
            shared_ptr<PatternQuery> pq = constraint_agent->GetPatternQuery();
            PatternLink link;
            for( PatternLink cur_link : pq->GetNormalLinks() )
            {
                if( cur_link.GetChildAgent() == link_agent )
                {
                    ASSERT(!link);
                    ASSERT(cur_link);
                    link = cur_link; // Found a link the the agent the constaint mentioned: not safe! TODO could be parallel links!
                }
            }
            // Link will be nullptr if this is the self-variable
            ASSERT( link || link_agent==constraint_agent )
                  ("Engine being queried via lambda about a non-normal linked agent\n")
                  ("Constraint agent ")(constraint_agent)("\n")
                  ("Linked agent ")(link_agent)("\n");
            
                    
            if( !link ) // Self-variable must be by location
                flags.compare_by = CSP::CompareBy::LOCATION;   
            else if( coupling_residual_links.count(link) > 0 ) // Coupling residuals are by value
                flags.compare_by = CSP::CompareBy::EQUIVALENCE;
            else if( master_boundary_agents.count(link_agent) > 0) // Couplings to master are by value
                flags.compare_by = CSP::CompareBy::EQUIVALENCE;
            else
                flags.compare_by = CSP::CompareBy::LOCATION;   
                                 
            if( link_agent==root_agent ) // Root variable will be forced
                flags.freedom = CSP::Freedom::FORCED;
            else if( master_boundary_agents.count(link_agent) > 0) // Couplings to master are forced
                flags.freedom = CSP::Freedom::FORCED;
            else
                flags.freedom = CSP::Freedom::FREE;
            
            return flags;            
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
    // Aside from CompareCoupling(), this is the other place where
    // we establish the couplings criterion as the  EquivalenceRelation, 
    // whatever that might be...? (it's SimpleCompare)
    by_equivalence_links = SetUnion( coupling_residual_links, 
                                     master_boundary_links );                           
}


void AndRuleEngine::Plan::PopulateForSolver( Agent *agent,
                                             const set<Agent *> &master_agents )
{
    // Ignore repeated hits
    if( reached.count(agent) > 0 )    
        return; 
    reached.insert(agent);

    normal_agents_ordered.push_back( agent );

    if( master_agents.count( agent ) > 0 )
    {
        // It's a master boundary variable/agent because:
        // 1. It's a master agent
        // 2. It's not the child of a master agent (we don't recurse on them)
        // See #125
        master_boundary_agents.insert( agent );
        return;
    } 
         
    shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
    FOREACH( PatternLink link, pq->GetNormalLinks() )
    {
        PopulateForSolver( link.GetChildAgent(), master_agents );        
        
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
                                                set<PatternLink> *normal_links,
                                                PatternLink link )
{
    // Note that different links can point to the same agent, so 
    // unique agents is the stronger condition
    
    if( normal_links->count(link) != 0 )
        return; // Links must be uniquified (weaker condition)
    normal_links->insert(link);
    
    Agent *agent = link.GetChildAgent();    
    if( normal_agents->count(agent) != 0 )
        return; // Agents must be uniquified (stronger condition)
    normal_agents->insert(agent);

    shared_ptr<PatternQuery> pq = agent->GetPatternQuery();   
    FOREACH( PatternLink link, pq->GetNormalLinks() )
    {
        PopulateNormalAgents( normal_agents, normal_links, link );        
    }
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
                my_evaluator_abnormal_engines[link] = make_shared<AndRuleEngine>( link.GetPattern(), 
                                                                                  surrounding_agents );  
            }
            else
            {
                my_free_abnormal_engines[link] = make_shared<AndRuleEngine>( link.GetPattern(), 
                                                                             surrounding_agents );  
            }
        }
        
        FOREACH( PatternLink link, pq->GetMultiplicityLinks() )
        {
            my_multiplicity_engines[link] = make_shared<AndRuleEngine>( link.GetPattern(), 
                                                                        surrounding_agents );  
        }
    }
}


void AndRuleEngine::ExpandDomain( set< TreePtr<Node> > &domain )
{
    INDENT("X");
    // It's important that this walk hits parents first because local node 
    // transformations occur in parent-then-child order. That's why this 
    // part is here and not in the CSP stuff: it exploits knowlege of 
    // the directedenss of the pattern trees.
    for( Agent *agent : plan.normal_agents_ordered )
        if( plan.master_boundary_agents.count(agent) == 0 )
            plan.my_constraints.at(agent)->ExpandDomain( domain );  
}


void AndRuleEngine::StartCSPSolver( TreePtr<Node> start_x )
{
    // Put all the nodes in the X tree into the domain
    set< TreePtr<Node> > domain;
	Walk wx( start_x ); 
	for( Walk::iterator wx_it=wx.begin(); wx_it!=wx.end(); ++wx_it )
        domain.insert(*wx_it);
    
    // Determine the full set of forces 
    CouplingKeysMap forces = *master_keys;
    forces[plan.root_agent] = start_x;
    
    // Tell all the constraints about them
    for( pair< Agent *, shared_ptr<CSP::Constraint> > p : plan.my_constraints )
        p.second->SetForces( forces );
    
    // Expand the domain to include generated child y nodes.
    ExpandDomain( domain );
    
    plan.solver->Start( domain );
}


void AndRuleEngine::GetNextCSPSolution( TreePtr<Node> start_x )
{
    map< shared_ptr<CSP::Constraint>, list< TreePtr<Node> > > values;
    bool match = plan.solver->GetNextSolution( &values );        
    if( !match )
        throw NoSolution();

    // Recreate my_coupling_keys
    for( pair< Agent *, shared_ptr<CSP::Constraint> > p : plan.my_constraints )
    {
        if( p.first == plan.root_agent ) 
        {
            // Constraint's self-variable is fixed because we're at root
            // TODO it would be nice to be able to do p.second->GetAllVariables()
            // and and obtain the force back from the constraint
            InsertSolo(my_solution, make_pair(plan.root_pattern_link, start_x));
            KeyCoupling(external_keys, make_pair(plan.root_pattern_link, start_x));
            // TODO not sure if external_keys is right any more
        }
        else
        {
            // Constraint's self-variable is free so grab it
            list< TreePtr<Node> > &vals = values.at(p.second);
            list< Agent * > vars = p.second->GetFreeVariables();
            ASSERT( vars.front() == p.first )
                  (vars)("\n")
                  (p)("\n")
                  (plan.root_agent)("\n");
            if( plan.my_normal_agents.count(vars.front()) )
            {
                ASSERT(false); 
                //InsertSolo(my_solution, make_pair(vars.front(), vals.front()));       // TODO!! #171
                //KeyCoupling(external_keys, make_pair(vars.front(), vals.front()));    // TODO!! #171
                external_keys[vars.front()] = vals.front();    
                // TODO not sure if external_keys is right any more
            }
        }            
    }
}


void AndRuleEngine::CompareLinks( Agent *agent,
                                  shared_ptr<const DecidedQuery> query ) 
{    
    FOREACH( const LocatedLink &link, query->GetNormalLinks() )
    {
        TRACE("Comparing normal link ")(link)(" keyer? %d residual? %d master? %d\n", plan.coupling_keyer_links.count( link ), plan.coupling_residual_links.count( link ), plan.master_boundary_links.count(link) );
        // Recurse normally 
        // Get x for linked node
        ASSERT( link.GetChildX() );
        
        // This is needed for decisionised MatchAny #75. Other schemes for
        // RegisterAlwaysMatchingLink() could be deployed.
        //if( x == DecidedQueryCommon::MMAX_Node )
        //    continue; // Pattern nodes immediately match themselves
        
        // Are we at a residual coupling?
        if( plan.coupling_residual_links.count( link ) > 0 ) // See #129
        {
            CompareCoupling( my_coupling_keys, link );
            TRACE("Accepted working coupling for ")(link)(" key=")(my_coupling_keys.at(link.GetChildAgent()))("\n");
            InsertSolo( my_solution, link );       
            continue;
        }
        
        // Master couplings are now checked in a post-pass
        if( plan.master_boundary_links.count(link) > 0 )
        {
            InsertSolo( master_solution, link);
            continue;
        }

        // Remember the coupling before recursing, as we can hit the same node 
        // (eg identifier) and we need to have coupled it. The "if" statement
        // tests coupling_keyer_links as well as providing a small optimisation.
        if( plan.coupling_keyer_links.count( link ) )
        {
            ASSERT( link.GetChildX() != DecidedQueryCommon::MMAX_Node )("Can't key with MMAX because would leak");
            KeyCoupling( my_coupling_keys, link );
        }

        DecidedCompare(link);   
    }
}


void AndRuleEngine::DecidedCompare( LocatedLink link )  
{
    INDENT("D");
    ASSERT( link.GetChildAgent() ); // Pattern must not be nullptr
    ASSERT( link.GetChildX() ); // Target must not be nullptr
    Agent * const agent = link.GetChildAgent();
    TreePtr<Node> const x = link.GetChildX();

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
    InsertSolo( my_solution, link );                
    if( link.GetChildX() != DecidedQueryCommon::MMAX_Node )
        KeyCoupling( external_keys, link );
}


void AndRuleEngine::CompareEvaluatorLinks( Agent *agent, 
                                           const CouplingKeysMap *subordinate_keys, 
                                           const SolutionMap *solution ) 
{
    INDENT("E");
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
        TreePtr<Node> x = solution->at(link);
                                
        try 
        {
            plan.my_evaluator_abnormal_engines.at(link)->Compare( x, subordinate_keys );
            compare_results.push_back( true );
        }
        catch( ::Mismatch & )
        {
            compare_results.push_back( false );
        }

        i++;
    }
    
	TRACE("Evaluating ");
	FOREACH(bool b, compare_results)
	    TRACEC(b)(" ");
	if( !(*evaluator)( compare_results ) )
        throw EvaluatorFalse();
}


void AndRuleEngine::CompareMultiplicityLinks( LocatedLink link, 
                                              const CouplingKeysMap *combined_keys ) 
{
    INDENT("M");

    shared_ptr<AndRuleEngine> e = plan.my_multiplicity_engines.at(link);
    TRACE("Checking multiplicity ")(link)("\n");
    TreePtr<Node> x = link.GetChildX();
        
    ASSERT( x );
    ContainerInterface *xc = dynamic_cast<ContainerInterface *>(x.get());
    ASSERT(xc)("Multiplicity x must implement ContainerInterface");
    
    FOREACH( TreePtr<Node> xe, *xc )
    {
        TRACE("Comparing ")(xe)("\n");
        e->Compare( xe, combined_keys );
    }
}


void AndRuleEngine::RegenerationPassAgent( Agent *agent, 
                                           TreePtr<Node> x,
                                           const CouplingKeysMap &subordinate_keys,
                                           const SolutionMap &combined_solution )
{
    // Get a list of the links we must supply to the agent for regeneration
    auto pq = agent->GetPatternQuery();
    TRACE("In after-pass, trying to regenerate ")(*agent)(" at ")(*x)("\n");    
    TRACEC("Pattern links ")(pq->GetNormalLinks())("\n");    
    list<LocatedLink> ll = LocateLinksFromMap( pq->GetNormalLinks(), combined_solution );
    TRACEC("Relocated links ")(ll)("\n");    
    
    // We will need a conjecture, so that we can iterate through multiple 
    // potentially valid values for the abnormals and multiplicities.
    auto query = make_shared<DecidedQuery>(pq);
    Conjecture conj(agent, query);            
    conj.Start();
    
    int i=0;
    while(1)
    {
        // Query the agent: our conj will be used for the iteration and
        // therefore our query will hold the result 
        agent->ResumeNormalLinkedQuery( conj, x, ll );
        i++;

        try
        {
            TRACE("Try out query, attempt %d (1-based)\n", i);    
            SolutionMap solution_for_evaluators;
            CouplingKeysMap provisional_external_keys;
            
            // Try matching the abnormal links (free and evaluator).
            FOREACH( const LocatedLink &link, query->GetAbnormalLinks() )
            {
                // Actions if evaluator link
                if( plan.my_evaluator_abnormal_engines.count(link) )                
                    InsertSolo( solution_for_evaluators, link );                
                
                // Actions if free link
                if( plan.my_free_abnormal_engines.count(link) )
                {
                    shared_ptr<AndRuleEngine> e = plan.my_free_abnormal_engines.at(link);
                    e->Compare( link.GetChildX(), &subordinate_keys );
                    
                    // Replace needs these keys
                    KeyCoupling( provisional_external_keys, link );
                }
            }                    
            
            // Try matching the multiplicity links.
            FOREACH( const LocatedLink &link, query->GetMultiplicityLinks() )
            {
                if( plan.my_evaluator_abnormal_engines.count(link) )
                    InsertSolo( solution_for_evaluators, link );                

                if( plan.my_multiplicity_engines.count(link) )
                    CompareMultiplicityLinks( link, &subordinate_keys );  
            }

            // Try matching the evaluator agents.
            if( plan.my_evaluators.count( agent ) )
                CompareEvaluatorLinks( agent, &subordinate_keys, &solution_for_evaluators );            
            
            // if we got here, we're done!
            external_keys = MapUnion( provisional_external_keys, external_keys );      
            
            TRACE("Leaving while loop after %d tries\n", i);    
            break;
        }
        catch( const ::Mismatch& mismatch )
        {
        }
        if( !conj.Increment() )
            throw Agent::NormalLinksMismatch(); // Conjecture has run out of choices to try.            
    } 
}      


void AndRuleEngine::RegenerationPass()
{
    INDENT("R");
    const CouplingKeysMap subordinate_keys = MapUnion( *master_keys, external_keys );          
    TRACEC("External combined keys ")(subordinate_keys)("\n");    

    const SolutionMap combined_solution = MapUnion( master_solution, my_solution );      
    TRACEC("Internal combined solution ")(combined_solution)("\n");    

    for( auto plink : plan.my_normal_links )
    {
        TreePtr<Node> x = combined_solution.at(plink);
        RegenerationPassAgent( plink.GetChildAgent(), x, subordinate_keys, combined_solution );
    }
}


void AndRuleEngine::CompareTrivialProblem( LocatedLink root_link )
{
    // Trivial case: we have no agents, so there won't be any decisions
    // and so no problem to solve. Spare all algorithms the hassle of 
    // dealing with this. Root agent should have been keyed by master,
    // otherwise it'd be in my_normal_agents.
    ASSERT( master_keys->count(plan.root_agent) );
    try
    {            
        CompareCoupling( *master_keys, root_link );
    }
    catch( const ::Mismatch& mismatch ) 
    {
        throw NoSolution();
    }
    return;
}


void AndRuleEngine::CompareMasterKeys()
{
    for( auto plink : plan.master_boundary_links )
    {
        LocatedLink link(plink, master_solution.at(plink));
        CompareCoupling( *master_keys, link );
        TRACE("Accepted master coupling for link=")(link)(" key=")(master_keys->at(link.GetChildAgent()))("\n");
    }
}


// This one if you want the resulting couplings and conj (ie doing a replace imminently)
void AndRuleEngine::Compare( TreePtr<Node> start_x,
                             const CouplingKeysMap *master_keys_ )
{
    INDENT("C");
    ASSERT( start_x );
    TRACE("Compare x=")(start_x)(" pattern=")(plan.root_pattern_link)("\n");
           
    master_keys = master_keys_;    
    
    LocatedLink root_link( plan.root_pattern_link, start_x );

    if( plan.my_normal_agents.empty() )
    {
        CompareTrivialProblem( root_link );
        return;
    }
           
#ifdef USE_SOLVER
    StartCSPSolver( start_x );
#else
    plan.conj->Start();
#endif
           
    // Create the conjecture object we will use for this compare, and keep iterating
    // though different conjectures trying to find one that allows a match.
    //int i=0;
    while(1)
    {
        my_solution.clear();
        external_keys.clear();
        my_coupling_keys.clear();
#ifdef USE_SOLVER        
        // Get a solution from the solver
        GetNextCSPSolution( start_x );
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
            master_solution.clear();            
            DecidedCompare( root_link );            
            CompareMasterKeys();
#endif

            RegenerationPass();
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
}


// This one operates from root for a stand-alone compare operation and
// no master keys.
void AndRuleEngine::Compare( TreePtr<Node> start_x )
{
    CouplingKeysMap master_keys;
    Compare( start_x, &master_keys );
}


void AndRuleEngine::EnsureChoicesHaveIterators()
{
#ifndef USE_SOLVER
    plan.conj->EnsureChoicesHaveIterators();
#endif
}


const CouplingKeysMap &AndRuleEngine::GetCouplingKeys()
{
    return external_keys;
}


void AndRuleEngine::CompareCoupling( const CouplingKeysMap &keys, const LocatedLink &residual_link )
{
    // Allow Magic Match Anything 
    if( residual_link.GetChildX() == DecidedQueryCommon::MMAX_Node )
        return;

    Agent *agent = residual_link.GetChildAgent();
    ASSERT( keys.count(agent) > 0 );
    //TreePtr<Node> key_x = keys.at(agent);
    //TreePtr<Node> residual_x = keys.at(agent);

    // Enforce rule #149
    ASSERT( !TreePtr<SubContainer>::DynamicCast( keys.at(agent) ) ); 

    // This function establishes the policy for couplings in one place,
    // apart from the other place which is plan.by_equivalence_links.
    // Today, it's SimpleCompare, via EquivalenceRelation. 
    // And it always will be: see #121; para starting at "No!!"
    static EquivalenceRelation equivalence_relation;
    if( !equivalence_relation( residual_link.GetChildX(), keys.at(agent) ) )
        throw Mismatch();    
}                                     


void AndRuleEngine::KeyCoupling( CouplingKeysMap &keys, const LocatedLink &keyer_link )
{
    // A coupling keyed to Magic-Match-Anything-X would not be able to 
    // restrict the residuals wrt to each other. 
    ASSERT( keyer_link.GetChildX() != DecidedQueryCommon::MMAX_Node );
    
    // A coupling relates the coupled agent to an X node, not the
    // link into the agent.
    InsertSolo( keys, make_pair( keyer_link.GetChildAgent(), 
                                 keyer_link.GetChildX() ) ); 
}                                                       


void AndRuleEngine::AssertNewCoupling( const CouplingKeysMap &extracted, Agent *new_agent, TreePtr<Node> new_x, Agent *parent_agent )
{
    ASSERT( extracted.count(new_agent) == 1 );
    if( TreePtr<SubContainer>::DynamicCast(new_x) )
    {                    
        EquivalenceRelation equivalence_relation;
        bool same  = equivalence_relation( extracted.at(new_agent), new_x );
        if( !same )
        {
            FTRACE("New x ")(new_x)(" mismatches extracted x ")(extracted.at(new_agent))
                  (" for agent ")(new_agent)(" with parent ")(parent_agent)("\n");
            if( TreePtr<SubSequence>::DynamicCast(new_x) && TreePtr<SubSequence>::DynamicCast(extracted.at(new_agent)))
                FTRACEC("SubSequence\n");
            else if( TreePtr<SubSequenceRange>::DynamicCast(new_x) && TreePtr<SubSequenceRange>::DynamicCast(extracted.at(new_agent)))
                FTRACEC("SubSequenceRange\n");
            else if( TreePtr<SubCollection>::DynamicCast(new_x) && TreePtr<SubCollection>::DynamicCast(extracted.at(new_agent)))
                FTRACEC("SubCollections\n");
            else
                FTRACEC("Container types don't match\n");
            ContainerInterface *xc = dynamic_cast<ContainerInterface *>(extracted.at(new_agent).get());
            FOREACH( TreePtr<Node> n, *xc )
                FTRACEC("ext: ")( n )("\n");
            xc = dynamic_cast<ContainerInterface *>(new_x.get());
            FOREACH( TreePtr<Node> n, *xc )
                FTRACEC("new: ")( n )("\n");
            ASSERTFAIL("AssertNewCoupling() failure");                                                
        }
    }
    else
    {
        ASSERT( extracted.at(new_agent) == new_x );
    }
}
