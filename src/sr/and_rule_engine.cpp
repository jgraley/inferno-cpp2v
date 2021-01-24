#include "and_rule_engine.hpp"

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
#include "agents/star_agent.hpp"
#include "common/common.hpp"
#include "agents/disjunction_agent.hpp"
#include "link.hpp"
#include "tree/cpptree.hpp"
#include "equivalence.hpp"

#include <list>

//#define TEST_PATTERN_QUERY

// This now works!
//#define USE_SOLVER
 
//#define CHECK_EVERYTHING_IS_IN_DOMAIN

//#define NLQ_TEST


using namespace SR;

AndRuleEngine::AndRuleEngine( PatternLink root_plink, 
                              const unordered_set<Agent *> &master_agents_ ) :
    plan( this, root_plink, master_agents_ )
{
}    
 
AndRuleEngine::Plan::Plan( AndRuleEngine *algo_, 
                           PatternLink root_plink_, 
                           const unordered_set<Agent *> &master_agents_) :
    algo( algo_ ),
    root_plink( root_plink_ ),
    root_pattern( root_plink.GetPattern() ),
    root_agent( root_plink.GetChildAgent() ),
    master_agents( master_agents_ )
{
    TRACE(GetName());
    INDENT("P");
    
    unordered_set<Agent *> normal_agents;
    unordered_set<PatternLink> normal_links;
    PopulateNormalAgents( &normal_agents, &normal_links, root_plink );    
    for( PatternLink plink : normal_links )
        if( master_agents.count( plink.GetChildAgent() ) == 0 )
            my_normal_links.insert( plink );
            
    my_normal_agents = DifferenceOf( normal_agents, master_agents );       
    if( my_normal_agents.empty() ) 
        return;  // Early-out on trivial problems

    unordered_set<Agent *> surrounding_agents = UnionOf( my_normal_agents, master_agents );         
    CreateSubordniateEngines( normal_agents, surrounding_agents );    
        
    reached_agents.clear();
    reached_links.clear();
    PopulateSomeThings( root_plink, 
                        master_agents );

    DetermineKeyers( root_plink, master_agents );
    DetermineResiduals( root_agent, master_agents );
    DetermineNontrivialKeyers();
        
#ifdef USE_SOLVER   
    {
        list< shared_ptr<CSP::Constraint> > constraints_list;
        CreateMyConstraints(constraints_list);
        CreateMasterCouplingConstraints(constraints_list);
        CreateCSPSolver(constraints_list);
        // Note: constraints_list drops out of scope and discards its 
        // references; only constraints held onto by solver will remain.
    }
#else
    conj = make_shared<Conjecture>(my_normal_agents, root_agent);
#endif                      
}


void AndRuleEngine::Plan::CreateMyConstraints( list< shared_ptr<CSP::Constraint> > &constraints_list )
{
    unordered_set<Agent *> check_we_got_the_right_agents;
    for( PatternLink keyer_plink : coupling_keyer_links )
    {        
        // Only one constraint per agent
        ASSERT( check_we_got_the_right_agents.count( keyer_plink.GetChildAgent() ) == 0 );
        check_we_got_the_right_agents.insert( keyer_plink.GetChildAgent() );            
            
        CSP::SystemicConstraint::VariableQueryLambda vql = [&](PatternLink plink) -> CSP::SystemicConstraint::VariableFlags
        {
            CSP::SystemicConstraint::VariableFlags flags;
                                  
            if( plink == root_plink ) // Root variable will be forced
                flags.freedom = CSP::SystemicConstraint::Freedom::FORCED;
            else 
                flags.freedom = CSP::SystemicConstraint::Freedom::FREE;
            
            return flags;            
        };
                
        // Determine the coupling residuals for this agent
        set<PatternLink> residual_plinks;
        for( PatternLink residual_plink : coupling_residual_links )
            if( residual_plink.GetChildAgent() == keyer_plink.GetChildAgent() )
                residual_plinks.insert( residual_plink );
                
        shared_ptr<CSP::Constraint> c = make_shared<CSP::SystemicConstraint>( keyer_plink, 
                                                                              residual_plinks, 
                                                                              CSP::SystemicConstraint::Action::FULL,
                                                                              vql );
        constraints_list.push_back(c);    
    }

    ASSERT( check_we_got_the_right_agents == my_normal_agents );
}


void AndRuleEngine::Plan::CreateMasterCouplingConstraints( list< shared_ptr<CSP::Constraint> > &constraints_list )
{
    for( PatternLink keyer_plink : master_boundary_keyer_links )
    {                                    
        CSP::SystemicConstraint::VariableQueryLambda vql = [&](PatternLink plink) -> CSP::SystemicConstraint::VariableFlags
        {
            CSP::SystemicConstraint::VariableFlags flags;
                                  
            if( plink == keyer_plink ) // keyer will be forced
                flags.freedom = CSP::SystemicConstraint::Freedom::FORCED;
            else // residual
                flags.freedom = CSP::SystemicConstraint::Freedom::FREE;
            
            return flags;            
        };
                
        // Determine the coupling residuals for this agent
        set<PatternLink> residual_plinks;
        for( PatternLink residual_plink : master_boundary_residual_links )
            if( residual_plink.GetChildAgent() == keyer_plink.GetChildAgent() )
                residual_plinks.insert( residual_plink );
                
        shared_ptr<CSP::Constraint> c = make_shared<CSP::SystemicConstraint>( keyer_plink, 
                                                                              residual_plinks, 
                                                                              CSP::SystemicConstraint::Action::COUPLING,
                                                                              vql );
        constraints_list.push_back(c);    
    }
}


void AndRuleEngine::Plan::CreateCSPSolver( const list< shared_ptr<CSP::Constraint> > &constraints_list )
{       
    // Passing in normal_agents_ordered will force SimpleSolver's backtracker to
    // take the same route we do with DecidedCompare(). Need to remove FORCED agents
    // though.
    list<PatternLink> free_normal_links_ordered;
    for( PatternLink link : normal_links_ordered )
    {
        if( link != root_plink )
            free_normal_links_ordered.push_back( link );
    }
    auto salg = make_shared<CSP::SimpleSolver>(constraints_list, &free_normal_links_ordered);
    solver = make_shared<CSP::SolverHolder>(salg);
}


void AndRuleEngine::Plan::PopulateSomeThings( PatternLink link,
                                              const unordered_set<Agent *> &master_agents )
{
    if( reached_links.count(link) > 0 )    
        return; 
    reached_links.insert(link);

    normal_links_ordered.push_back( link );
    Agent *agent = link.GetChildAgent();
    TreePtr<Node> agent_pattern = link.GetPattern();
    
    if( reached_agents.count(agent) > 0 )    
        return; 
    reached_agents.insert(agent);

    if( master_agents.count( agent ) > 0 )
    {
        // It's a master boundary variable/agent because:
        // 1. It's a master agent
        // 2. It's not the child of a master agent (we don't recurse on them)
        // See #125
        master_boundary_agents.insert( agent );

        // We don't need the original keyer link for this agent (it belongs
        // to master) so just create a new one. These will be the FORCED
        // variables that permit us to inject master keys into CSP.
        master_boundary_keyer_links.insert( PatternLink::CreateDistinct(agent_pattern) );
        return;
    } 
         
    shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
    FOREACH( PatternLink link, pq->GetNormalLinks() )
    {
        PopulateSomeThings( link, master_agents );        
        
        // Note: here, we won't see root if root is a master agent (i.e. trivial pattern)
        if( master_boundary_agents.count( link.GetChildAgent() ) )
            master_boundary_residual_links.insert( link );
    }
}

        
void AndRuleEngine::Plan::DetermineKeyersModuloMatchAny( PatternLink plink,
                                                         unordered_set<Agent *> *senior_agents,
                                                         unordered_set<Agent *> *matchany_agents )
{
    if( senior_agents->count( plink.GetChildAgent() ) > 0 )
        return; // will be fixed values for our solver
    senior_agents->insert( plink.GetChildAgent() );

    // See #129, can fail on legal patterns - will also fail on illegal Disjunction couplings
    for( PatternLink l : coupling_keyer_links )        
        ASSERT( l.GetChildAgent() != plink.GetChildAgent() )
              ("Conflicting coupling in and-rule pattern: check Disjunction nodes\n");

    coupling_keyer_links.insert(plink);
    agent_to_keyer[plink.GetChildAgent()] = plink;

    if( dynamic_cast<DisjunctionAgent *>(plink.GetChildAgent()) )
    {
        matchany_agents->insert( plink.GetChildAgent() );
        return;
    }

    shared_ptr<PatternQuery> pq = plink.GetChildAgent()->GetPatternQuery();
    FOREACH( PatternLink plink, pq->GetNormalLinks() )
    {
        DetermineKeyersModuloMatchAny( plink, senior_agents, matchany_agents );        
    }
}
        
        
void AndRuleEngine::Plan::DetermineKeyers( PatternLink plink,
                                           unordered_set<Agent *> senior_agents ) 
{
    // Scan the senior region. We wish to break off at Disjunction nodes. Senior is the
    // region up to and including a Disjunction; junior is the region under each of its
    // links.
    unordered_set<Agent *> my_matchany_agents;
    DetermineKeyersModuloMatchAny( plink, &senior_agents, &my_matchany_agents );
    // After this:
    // - my_master_agents has union of master_agents and all the identified keyed agents
    // - my_match_any_agents has the Disjunction agents that we saw, BUT SKIPPED
    
    // Now do all the links under the Disjunction nodes' links. Keying is allowed in each
    // of these junior regions individually, but no cross-keying is allowed if not keyed already.
    // Where that happens, there will be a conflict writing to coupling_nontrivial_keyer_links and the
    // ASSERT will fail.
    for( Agent *ma_agent : my_matchany_agents )
    {
        shared_ptr<PatternQuery> pq = ma_agent->GetPatternQuery();
        FOREACH( PatternLink link, pq->GetNormalLinks() )
        {
            DetermineKeyers( link, senior_agents );        
        }
    }
}
        
        
void AndRuleEngine::Plan::DetermineResiduals( Agent *agent,
                                              unordered_set<Agent *> master_agents ) 
{
    shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
    FOREACH( PatternLink link, pq->GetNormalLinks() )
    {            
        PatternLink keyer;
        for( PatternLink l : coupling_keyer_links )
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
        
        DetermineResiduals( link.GetChildAgent(), master_agents );        
    }
}


void AndRuleEngine::Plan::DetermineNontrivialKeyers()
{
    coupling_nontrivial_keyer_links.clear();
    for( PatternLink keyer_plink : coupling_keyer_links )
    {
        bool found_residual_on_same_child_node = false;
        for( PatternLink residual_plink : coupling_residual_links )
        {
            if( residual_plink.GetChildAgent() == keyer_plink.GetChildAgent() )
            {
                found_residual_on_same_child_node = true;
                break;
            }
        }
        if( found_residual_on_same_child_node )
        {
            coupling_nontrivial_keyer_links.insert( keyer_plink );
        }
    }
}


void AndRuleEngine::Plan::PopulateNormalAgents( unordered_set<Agent *> *normal_agents, 
                                                unordered_set<PatternLink> *normal_links,
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


void AndRuleEngine::Plan::CreateSubordniateEngines( const unordered_set<Agent *> &normal_agents, 
                                                    const unordered_set<Agent *> &surrounding_agents )
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
                my_evaluator_abnormal_engines[link] = make_shared<AndRuleEngine>( link, surrounding_agents );  
            }
            else
            {
                my_free_abnormal_engines[link] = make_shared<AndRuleEngine>( link, surrounding_agents );  
            }
        }
        
        FOREACH( PatternLink link, pq->GetMultiplicityLinks() )
        {
            my_multiplicity_engines[link] = make_shared<AndRuleEngine>( link, surrounding_agents );  
        }
    }
}


void AndRuleEngine::StartCSPSolver(XLink root_xlink)
{    
    // Determine the full set of forces 
    // TODO presumably doesn't need to be the ordered one
    SolutionMap master_and_root_links;
    for( PatternLink link : plan.master_boundary_keyer_links )
    {
        // distinct OK because this only runs once per solve
        TreePtr<Node> node = master_keys->at(link.GetChildAgent()).GetChildX();
        master_and_root_links[link] = XLink::CreateDistinct(node);
    }
    master_and_root_links[plan.root_plink] = root_xlink;

    plan.solver->Start( knowledge->domain, master_and_root_links, knowledge );
}


void AndRuleEngine::GetNextCSPSolution()
{
    TRACE("GetNextCSPSolution()\n");
    SolutionMap csp_solution;
    bool match = plan.solver->GetNextSolution( &csp_solution );        
    if( !match )
        throw NoSolution();

    // Recreate my_coupling_keys
    for( pair< PatternLink, XLink > pxp : csp_solution )
    {
        RecordLink( LocatedLink(pxp) );                        
    }
}


void AndRuleEngine::CompareLinks( Agent *agent,
                                  shared_ptr<const DecidedQuery> query ) 
{    
    // Couplings require links in the same order as during planning (i.e. 
    // pattern query order) so that keyers act before restricters (I think...)
    shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
    for( PatternLink plink : pq->GetNormalLinks() )    
    {
        LocatedLink link( plink, query->GetNormalLinks().at(plink) );

#ifdef CHECK_EVERYTHING_IS_IN_DOMAIN    
        ASSERT( knowledge->domain.count(link) > 0 )(link)(" not found in ")(knowledge->domain)(" (see issue #202)\n");
#endif

        TRACE("Comparing normal link ")(link)
             (" keyer? %d residual? %d master? %d\n", 
             plan.coupling_nontrivial_keyer_links.count( (PatternLink)link ), 
             plan.coupling_residual_links.count( (PatternLink)link ), 
             plan.master_boundary_residual_links.count( (PatternLink)link ) );
        ASSERT( link.GetChildX() );
        
        // Check the link: we will either compare a coupling
        // or recurse to DecidedCompare(). We never DC() after a
        // coupling compare, because couplings are only keyed
        // after a successful DC().  
        if( plan.coupling_residual_links.count( (PatternLink)link ) > 0 )
        {
            CompareCoupling( my_coupling_keys, link );
        }
        else if( plan.master_boundary_residual_links.count( (PatternLink)link ) > 0 )
        {
            CompareCoupling( *master_keys, link );
        }
        else
        {
            DecidedCompare(link);   
            if( plan.coupling_nontrivial_keyer_links.count( (PatternLink)link ) > 0 )
                KeyCoupling( my_coupling_keys, link );
        }

        RecordLink( link );        
    }
}


void AndRuleEngine::DecidedCompare( LocatedLink link )  
{
    INDENT("D");
    ASSERT( link.GetChildAgent() ); // Pattern must not be nullptr
    ASSERT( link.GetChildX() ); // Target must not be nullptr
    Agent * const agent = link.GetChildAgent();

    // Obtain the query state from the conjecture
    shared_ptr<DecidedQuery> query = plan.conj->GetQuery(agent);

    // Run the compare implementation to get the links based on the choices
    TRACE("RunDecidedQuery() with ")(link)("\n");     
    agent->RunDecidedQuery( *query, link );
    TRACE("Normal ")(query->GetNormalLinks())("\n")
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
        XLink xlink = solution->at(link);
                                
        try 
        {
            shared_ptr<AndRuleEngine> e = plan.my_evaluator_abnormal_engines.at(link);
            e->Compare( xlink, subordinate_keys, knowledge );
            compare_results.push_back( true );
        }
        catch( ::Mismatch & )
        {
            compare_results.push_back( false );
        }

        i++;
    }
    
	TRACE("Evaluating: ");
	FOREACH(bool b, compare_results)
	    TRACEC(b)(" ");
    TRACEC("\n");
	if( !(*evaluator)( compare_results ) )
        throw EvaluatorFalse();
}


void AndRuleEngine::CompareMultiplicityLinks( LocatedLink link, 
                                              const CouplingKeysMap *subordinate_keys ) 
{
    INDENT("M");

    shared_ptr<AndRuleEngine> e = plan.my_multiplicity_engines.at( (PatternLink)link );
    TRACE("Checking multiplicity ")(link)("\n");
        
    auto xsc = dynamic_cast<SubContainer *>( link.GetChildX().get() );
    
    if( auto xscr = dynamic_cast<SubContainerRange *>(xsc) )
    {
        ASSERT( link );
        ContainerInterface *xci = dynamic_cast<ContainerInterface *>(xscr);
        ASSERT(xci)("Multiplicity x must implement ContainerInterface");    
        
        FOREACH( const TreePtrInterface &xe_node, *xci )
        {
            TRACE("Comparing ")(xe_node)("\n");
            XLink xe_link = XLink(xscr->GetParentX(), &xe_node);
            e->Compare( xe_link, subordinate_keys, knowledge );
        }
    }
    else if( auto xssl = dynamic_cast<SubSequence *>(xsc) )
    {
        for( XLink xe_link : xssl->elts )
        {
            TRACE("Comparing ")(xe_link)("\n");
            e->Compare( xe_link, subordinate_keys, knowledge );
        }
    }    
    else
    {
        ASSERTFAIL("unrecognised SubContainer\n");
    }
}


void AndRuleEngine::RegenerationPassAgent( Agent *agent,
                                           XLink base_xlink,
                                           const CouplingKeysMap &subordinate_keys )
{
    // Get a list of the links we must supply to the agent for regeneration
    auto pq = agent->GetPatternQuery();
    TRACE("Trying to regenerate ")(*agent)(" at ")(base_xlink)("\n");    
    TRACEC("Pattern links ")(pq->GetNormalLinks())("\n");    

#ifdef CHECK_EVERYTHING_IS_IN_DOMAIN      
    if( !dynamic_cast<StarAgent*>(agent) ) // Stars are based at SubContainers which don't go into domain    
        ASSERT( knowledge->domain.count(base_xlink) > 0 )(base_xlink)(" not found in ")(knowledge->domain)(" (see issue #202)\n"); // #202 expected to cause this to fail
    for( XLink xlink : pq->GetNormalLinks() )    
        ASSERT( knowledge->domain.count(xlink) > 0 )(xlink)(" not found in ")(knowledge->domain)(" (see issue #202)\n"); // #202 expected to cause this to fail
#endif
    
#ifdef NLQ_TEST
    auto nlq_lambda = agent->TestStartRegenerationQuery( base_xlink, &basic_solution, knowledge );
#else    
    auto nlq_lambda = agent->StartRegenerationQuery( base_xlink, &basic_solution, knowledge );
#endif
    
    int i=0;
    while(1)
    {
        shared_ptr<SR::DecidedQuery> query = nlq_lambda();
        i++;
                
        TRACE("Try out query, attempt %d (1-based)\n", i);    

        try
        {
            {
                Tracer::RAIIEnable silencer( false );   // Shush, I'm trying to debug the NLQs
                SolutionMap solution_for_evaluators;
                CouplingKeysMap provisional_external_keys;
                
                // Try matching the abnormal links (free and evaluator).
                FOREACH( const LocatedLink &link, query->GetAbnormalLinks() )
                {
                    ASSERT( link );
                    // Actions if evaluator link
                    if( plan.my_evaluator_abnormal_engines.count( (PatternLink)link ) )                
                        InsertSolo( solution_for_evaluators, link );                
                    
                    // Actions if free link
                    if( plan.my_free_abnormal_engines.count( (PatternLink)link ) )
                    {
                        shared_ptr<AndRuleEngine> e = plan.my_free_abnormal_engines.at( (PatternLink)link );
                        e->Compare( link, &subordinate_keys, knowledge );
                        
                        // Replace needs these keys
                        KeyCoupling( provisional_external_keys, link );
                    }
                }                    
                
                // Try matching the multiplicity links.
                FOREACH( const LocatedLink &link, query->GetMultiplicityLinks() )
                {
                    if( plan.my_evaluator_abnormal_engines.count( (PatternLink)link ) )
                        InsertSolo( solution_for_evaluators, link );                

                    if( plan.my_multiplicity_engines.count( (PatternLink)link ) )
                        CompareMultiplicityLinks( link, &subordinate_keys );  
                }

                // Try matching the evaluator agents.
                if( plan.my_evaluators.count( agent ) )
                    CompareEvaluatorLinks( agent, &subordinate_keys, &solution_for_evaluators );            
            
                // If we got here, we're done!
                external_keys = UnionOfSolo( provisional_external_keys, external_keys );                  
            }
            TRACE("Success after %d tries\n", i);    
            break;
        }
        catch( const ::Mismatch& mismatch )
        {
            TRACE("Caught Mismatch exception, retrying the lambda\n", i);    
        }                             
    } 
    agent->ResetNLQConjecture(); // save memory
}      


void AndRuleEngine::RegenerationPass()
{
    INDENT("R");
    const CouplingKeysMap subordinate_keys = UnionOfSolo( *master_keys, external_keys );   
    TRACE("---------------- Regeneration ----------------\n");      
    TRACEC("Subordinate keys ")(subordinate_keys)("\n");       
    TRACEC("Basic solution ")(basic_solution)("\n");    

    for( auto plink : plan.coupling_keyer_links )
    {
        RegenerationPassAgent( plink.GetChildAgent(), 
                               basic_solution.at(plink), 
                               subordinate_keys );
    }

    TRACE("Regeneration complete\n");
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


// This one if you want the resulting couplings and conj (ie doing a replace imminently)
void AndRuleEngine::Compare( XLink root_xlink,
                             const CouplingKeysMap *master_keys_,
                             const TheKnowledge *knowledge_ )
{
    INDENT("C");
    ASSERT( root_xlink );
           
    master_keys = master_keys_;    
    knowledge = knowledge_;
    
    // distinct OK because this only runs once per solve
    LocatedLink root_link( plan.root_plink, root_xlink );

    TRACE("Compare root ")(root_link)("\n");

#ifdef CHECK_EVERYTHING_IS_IN_DOMAIN    
    if( !dynamic_cast<StarAgent*>(root_link.GetChildAgent()) ) // Stars are based at SubContainers which don't go into domain    
        ASSERT( knowledge->domain.count(root_xlink) > 0 )(root_xlink)(" not found in ")(knowledge->domain)(" (see issue #202)\n");
#endif

    if( plan.my_normal_agents.empty() )
    {
        CompareTrivialProblem( root_link );
        return;
    }
                     
#ifdef USE_SOLVER
    StartCSPSolver( root_xlink );
#else
    plan.conj->Start();
#endif
           
    // Create the conjecture object we will use for this compare, and keep iterating
    // though different conjectures trying to find one that allows a match.
    //int i=0;
    while(1)
    {
        basic_solution.clear();
        external_keys.clear();
        my_coupling_keys.clear();
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
            DecidedCompare( root_link );       
            my_coupling_keys.clear(); // save memory     
#endif
            basic_solution[plan.root_plink] = root_xlink;
            // Fill this on the way out- by now I think we've succeeded in matching the current conjecture.
            if( root_xlink != XLink::MMAX_Link )
                KeyCoupling( external_keys, root_link );            

            // Is the solution complete? 
            for( auto plink : plan.my_normal_links )
            {            
                ASSERT( basic_solution.count(plink) > 0 )("Cannot find normal link ")(plink)("\nIn ")(basic_solution)("\n");
            }
            RegenerationPass();
            basic_solution.clear(); // save memory
        }
        catch( const ::Mismatch& e )
        {                
#ifdef USE_SOLVER
            TRACE(e)(" after recursion, trying next solution\n");
            continue; // Get another solution from the solver
#else
            TRACE(e)(" after recursion, trying increment conjecture\n");
            if( plan.conj->Increment() )
                continue; // Conjecture would like us to try again with new choices
                
            plan.conj->Reset();
            // We didn't match and we've run out of choices, so we're done.              
            throw NoSolution();
#endif            
        }
        // We got a match so we're done. 
        TRACE("AndRuleEngine hit\n");
        break; // Success
    }
#ifndef USE_SOLVER
    plan.conj->Reset();
#endif
    
    // By now, we succeeded and slave_keys is the right set of keys
}


// This one operates from root for a stand-alone compare operation and
// no master keys.
void AndRuleEngine::Compare( TreePtr<Node> root_xnode )
{
    CouplingKeysMap empty_master_keys;
    TheKnowledge empty_knowledge;
    XLink root_xlink = XLink::CreateDistinct(root_xnode);
    Compare( root_xlink, &empty_master_keys, &empty_knowledge );
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


const void AndRuleEngine::ClearCouplingKeys()
{
    external_keys.clear();
}


void AndRuleEngine::RecordLink( LocatedLink link )
{
    // All go into the basic solution which is enough to
    // regenerate a full solution.
    InsertSolo( basic_solution, link );                
    
    // Keying for external use (subordinates, slaves and replace)
    // We don't want residuals (which are unreliable) or MMAX
    if( (PatternLink)link != plan.root_plink &&
        plan.master_boundary_residual_links.count( (PatternLink)link ) == 0 &&
        plan.coupling_residual_links.count( (PatternLink)link ) == 0 && 
        (XLink)link != XLink::MMAX_Link )
        KeyCoupling( external_keys, link );        
}


void AndRuleEngine::CompareCoupling( const CouplingKeysMap &keys, const LocatedLink &residual_link )
{
    Agent *agent = residual_link.GetChildAgent();
    ASSERT( keys.count(agent) > 0 );
    XLink keyer_link = keys.at(agent);

    // Enforce rule #149
    ASSERT( !TreePtr<SubContainer>::DynamicCast( keyer_link.GetChildX() ) ); 

    multiset<XLink> candidate_links { keyer_link, residual_link };
    agent->RunCouplingQuery( candidate_links );
}                                     


void AndRuleEngine::KeyCoupling( CouplingKeysMap &keys, const LocatedLink &keyer_link )
{
    // A coupling keyed to Magic-Match-Anything-X would not be able to 
    // restrict the residuals wrt to each other. 
    ASSERT( (XLink)keyer_link != XLink::MMAX_Link );
    
    // A coupling relates the coupled agent to an X node, not the
    // link into the agent.
    InsertSolo( keys, make_pair( keyer_link.GetChildAgent(), keyer_link ) ); 
}                                                       


void AndRuleEngine::AssertNewCoupling( const CouplingKeysMap &extracted, Agent *new_agent, XLink new_xlink, Agent *parent_agent )
{
    TreePtr<Node> new_xnode = new_xlink.GetChildX();
    ASSERT( extracted.count(new_agent) == 1 );
    if( TreePtr<SubContainer>::DynamicCast(new_xnode) )
    {                    
        EquivalenceRelation equivalence_relation;
        CompareResult cr  = equivalence_relation.Compare( extracted.at(new_agent), new_xlink );
        if( cr != EQUAL )
        {
            FTRACE("New x node ")(new_xnode)(" mismatches extracted x ")(extracted.at(new_agent))
                  (" for agent ")(new_agent)(" with parent ")(parent_agent)("\n");
            if( TreePtr<SubSequence>::DynamicCast(new_xnode) && TreePtr<SubSequence>::DynamicCast(extracted.at(new_agent).GetChildX()))
                FTRACEC("SubSequence\n");
            else if( TreePtr<SubSequenceRange>::DynamicCast(new_xnode) && TreePtr<SubSequenceRange>::DynamicCast(extracted.at(new_agent).GetChildX()))
                FTRACEC("SubSequenceRange\n");
            else if( TreePtr<SubCollection>::DynamicCast(new_xnode) && TreePtr<SubCollection>::DynamicCast(extracted.at(new_agent).GetChildX()))
                FTRACEC("SubCollections\n");
            else
                FTRACEC("Container types don't match\n");
            ContainerInterface *xc = dynamic_cast<ContainerInterface *>(extracted.at(new_agent).GetChildX().get());
            FOREACH( const TreePtrInterface &node, *xc )
                FTRACEC("ext: ")( node )("\n");
            xc = dynamic_cast<ContainerInterface *>(new_xnode.get());
            FOREACH( const TreePtrInterface &node, *xc )
                FTRACEC("new: ")( node )("\n");
            ASSERTFAIL("AssertNewCoupling() failure");                                                
        }
    }
    else
    {
        ASSERT( extracted.at(new_agent).GetChildX() == new_xnode );
    }
}
