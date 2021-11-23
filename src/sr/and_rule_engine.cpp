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
#include "agents/delta_agent.hpp"
#include "agents/search_container_agent.hpp"
#include "agents/star_agent.hpp"
#include "common/common.hpp"
#include "agents/disjunction_agent.hpp"
#include "link.hpp"
#include "tree/cpptree.hpp"
#include "equivalence.hpp"
#include "render/graph.hpp"

#include <list>

//#define TEST_PATTERN_QUERY
 
//#define CHECK_EVERYTHING_IS_IN_DOMAIN

//#define NLQ_TEST

using namespace SR;

AndRuleEngine::AndRuleEngine( PatternLink root_plink, 
                              const unordered_set<PatternLink> &master_plinks,
                              const unordered_set<PatternLink> &master_keyer_plinks ) :
    plan( this, root_plink, master_plinks, master_keyer_plinks )
{
}    


AndRuleEngine::~AndRuleEngine() 
{ 
    //ASSERT( used )( plan.root_plink );
}

 
AndRuleEngine::Plan::Plan( AndRuleEngine *algo_, 
                           PatternLink root_plink_, 
                           const unordered_set<PatternLink> &master_plinks_,
                           const unordered_set<PatternLink> &master_keyer_plinks_ ) :
    algo( algo_ ),
    root_plink( root_plink_ ),
    root_pattern( root_plink.GetPattern() ),
    root_agent( root_plink.GetChildAgent() ),
    master_plinks( master_plinks_ )
{    
    INDENT("P");
    TRACE(algo->GetTrace())(" planning\n");

    TRACE("Master plinks: ")(master_plinks)("\n");
    
    master_agents.clear();
    for( PatternLink plink : master_plinks )
        master_agents.insert( plink.GetChildAgent() );

    unordered_set<Agent *> normal_agents;
    unordered_set<PatternLink> normal_links;
    PopulateNormalAgents( &normal_agents, &normal_links, root_plink );    
    for( PatternLink plink : normal_links )
        if( master_agents.count( plink.GetChildAgent() ) == 0 )
            my_normal_links.insert( plink );
            
    my_normal_agents = DifferenceOf( normal_agents, master_agents );       
    reached_agents.clear();
    reached_links.clear();    
    PopulateMasterBoundaryStuff( root_plink, 
                        master_agents );
    TRACE("Normal agents ")(normal_agents)("\nMaster boundary agents ")(master_boundary_agents)("\n");

    // Collect together the parent links to agents
    for( PatternLink plink : my_normal_links )
        parent_links_to_my_normal_agents[plink.GetChildAgent()].insert(plink);
    // my_normal_agents should be same set of agents as those reached by my_normal_links (uniquified)
    ASSERT( parent_links_to_my_normal_agents.size() == my_normal_agents.size() );
    for( PatternLink plink : my_master_boundary_links )
        parent_residual_links_to_master_boundary_agents[plink.GetChildAgent()].insert(plink);
    // master_boundary_agents should be same set of agents as those reached by my_master_boundary_links (uniquified)
    ASSERT( parent_residual_links_to_master_boundary_agents.size() == master_boundary_agents.size() );
        
    DetermineKeyers( root_plink, master_agents );
    DetermineResiduals( root_agent, master_agents );
    DetermineNontrivialKeyers();
    
    // Turns out these two are the same
    my_normal_links_unique_by_agent = coupling_keyer_links_all;

    // Well, obviously...
    ASSERT( my_normal_links_unique_by_agent.size()==my_normal_agents.size() );
    
    ConfigureAgents();       
    unordered_set<PatternLink> surrounding_plinks = UnionOf( my_normal_links, master_plinks );         
    unordered_set<PatternLink> surrounding_keyer_plinks = UnionOf( coupling_keyer_links_nontrivial, master_keyer_plinks );         
    CreateSubordniateEngines( my_normal_agents, surrounding_plinks, surrounding_keyer_plinks );   
    
    master_boundary_keyer_links.clear();
    for( PatternLink plink : master_keyer_plinks_ )
        if( master_boundary_agents.count(plink.GetChildAgent()) == 1 )
            master_boundary_keyer_links.insert( plink );
    TRACE("master_boundary_keyer_links ")(master_boundary_keyer_links)("\n");
    
    // Trivial problem checks   
    if( my_normal_links.empty() ) 
    {        
        ASSERT( my_normal_agents.empty() );
        // Root link obviously isn't in my_normal_links because that's empty, 
        // so it needs to be found in my_master_boundary_links
        ASSERT( my_master_boundary_links.count(root_plink) == 1 )
              ("\nmbrl:\n")(my_master_boundary_links);
    }
    else
    {
        ASSERT( !my_normal_agents.empty() );
    }

    // For CSP solver only...
    list< shared_ptr<CSP::Constraint> > constraints_list;
    CreateMyFullConstraints(constraints_list);
    CreateMasterCouplingConstraints(constraints_list);
    CreateCSPSolver(constraints_list);
    // Note: constraints_list drops out of scope and discards its 
    // references; only constraints held onto by solver will remain.

    // For old solver only...
    conj = make_shared<Conjecture>(my_normal_agents, root_agent);   
    
    Dump();
}


void AndRuleEngine::Plan::PopulateMasterBoundaryStuff( PatternLink link,
                                                       const unordered_set<Agent *> &master_agents )
{
    // Definition: it's a master boundary link/agent if:
    // 1. It's a master agent
    // 2. It's not the child of a master agent (we don't recurse on them)
    // See #125
    
    if( reached_links.count(link) > 0 )    
        return; 
    reached_links.insert(link);
    // ------------ Now unique by plink (weaker) -------------

    Agent *agent = link.GetChildAgent();
    
    // Note: here, we WILL see root if root is a master agent (i.e. trivial pattern)
    if( master_agents.count( agent ) )
        my_master_boundary_links.insert( link );

    normal_links_ordered.push_back( link );    

    if( reached_agents.count(agent) > 0 )    
        return; 
    reached_agents.insert(agent);
    // ------------ Now unique by agent (stronger) -------------

    if( master_agents.count( agent ) > 0 )
    {
        // At master boundary so don't recurse
        master_boundary_agents.insert( agent );
    } 
    else
    {
        // Not yet at master boundary so recurse
        shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
        for( PatternLink link : pq->GetNormalLinks() )
            PopulateMasterBoundaryStuff( link, master_agents );        
    }
}

        
void AndRuleEngine::Plan::DetermineKeyersModuloDisjunction( PatternLink plink,
                                                            unordered_set<Agent *> *senior_agents,
                                                            unordered_set<Agent *> *disjunction_agents )
{
    if( senior_agents->count( plink.GetChildAgent() ) > 0 )
        return; // will be fixed values for our solver
    senior_agents->insert( plink.GetChildAgent() );

    // See #129, can fail on legal patterns - will also fail on illegal Disjunction couplings
    for( PatternLink l : coupling_keyer_links_all )        
        ASSERT( l.GetChildAgent() != plink.GetChildAgent() )
              ("Conflicting coupling in and-rule pattern: check Disjunction nodes\n");

    coupling_keyer_links_all.insert(plink);

    if( dynamic_cast<DisjunctionAgent *>(plink.GetChildAgent()) )
    {
        disjunction_agents->insert( plink.GetChildAgent() );
        return;
    }

    shared_ptr<PatternQuery> pq = plink.GetChildAgent()->GetPatternQuery();
    FOREACH( PatternLink plink, pq->GetNormalLinks() )
    {
        DetermineKeyersModuloDisjunction( plink, senior_agents, disjunction_agents );        
    }
}

        
void AndRuleEngine::Plan::DetermineKeyers( PatternLink plink,
                                           unordered_set<Agent *> senior_agents ) 
{
    // See rule #384
    // Scan the senior region. We wish to break off at Disjunction nodes. Senior is the
    // region up to and including a Disjunction; junior is the region under each of its
    // links.
    unordered_set<Agent *> my_disjunction_agents;
    DetermineKeyersModuloDisjunction( plink, &senior_agents, &my_disjunction_agents );
    // After this:
    // - my_master_agents has union of master_agents and all the identified keyed agents
    // - my_match_any_agents has the Disjunction agents that we saw, BUT SKIPPED
    
    // Now do all the links under the Disjunction nodes' links. Keying is allowed in each
    // of these junior regions individually, but no cross-keying is allowed if not keyed already.
    // Where that happens, there will be a conflict writing to coupling_keyer_links_nontrivial and the
    // ASSERT will fail.
    for( Agent *ma_agent : my_disjunction_agents )
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
        for( PatternLink l : coupling_keyer_links_all )
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
    coupling_keyer_links_nontrivial.clear();
    for( PatternLink keyer_plink : coupling_keyer_links_all )
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
            coupling_keyer_links_nontrivial.insert( keyer_plink );
        }
    }
}


void AndRuleEngine::Plan::ConfigureAgents()
{
    TRACE("Configuring these ")(coupling_keyer_links_all)("\n");
    for( PatternLink keyer_plink : coupling_keyer_links_all )
    {
        ASSERT( keyer_plink );
        Agent *agent = keyer_plink.GetChildAgent();
                
        set< PatternLink > residual_plinks;
        for( PatternLink residual_plink : coupling_residual_links )
            if( residual_plink.GetChildAgent() == agent )
                residual_plinks.insert( residual_plink );

        agent->ConfigureCoupling( algo, keyer_plink, residual_plinks );
    }

    // New coupling planning
    // Where we have a link to a master agent (= master boundary link)
    // it will be residual because master always keys. Agent needs to 
    // know because we'll query it, so tell it.
    for( PatternLink master_boundary_plink : my_master_boundary_links )
    {
        ASSERT( master_boundary_plink );
        Agent *agent = master_boundary_plink.GetChildAgent();
        agent->AddResiduals( {master_boundary_plink} );
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
                                                    const unordered_set<PatternLink> &surrounding_plinks, 
                                                    const unordered_set<PatternLink> &surrounding_keyer_plinks )
{
    for( PatternLink plink : my_normal_links_unique_by_agent )
    {
        Agent *agent = plink.GetChildAgent();
        
        shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
            
        if( pq->GetEvaluator() )
            my_evaluators.insert(agent);
        
        FOREACH( PatternLink link, pq->GetAbnormalLinks() )
        {        
            if( pq->GetEvaluator() )
            {
                my_evaluator_abnormal_engines[link] = make_shared<AndRuleEngine>( link, surrounding_plinks, surrounding_keyer_plinks );  
            }
            else
            {
                my_free_abnormal_engines[link] = make_shared<AndRuleEngine>( link, surrounding_plinks, surrounding_keyer_plinks );  
            }
        }
        
        FOREACH( PatternLink link, pq->GetMultiplicityLinks() )
        {
            my_multiplicity_engines[link] = make_shared<AndRuleEngine>( link, surrounding_plinks, surrounding_keyer_plinks );  
        }
    }
}


void AndRuleEngine::Plan::CreateMyFullConstraints( list< shared_ptr<CSP::Constraint> > &constraints_list )
{
    for( PatternLink keyer_plink : my_normal_links_unique_by_agent ) // Only one constraint per agent
    {                        
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
}


void AndRuleEngine::Plan::CreateMasterCouplingConstraints( list< shared_ptr<CSP::Constraint> > &constraints_list )
{
    for( PatternLink keyer_plink : master_boundary_keyer_links )
    {                                    
        CSP::SystemicConstraint::VariableQueryLambda vql = [&](PatternLink plink) -> CSP::SystemicConstraint::VariableFlags
        {
            CSP::SystemicConstraint::VariableFlags flags;
                                  
            if( plink == root_plink || plink == keyer_plink ) // keyer will be forced
                flags.freedom = CSP::SystemicConstraint::Freedom::FORCED;
            else // residual
                flags.freedom = CSP::SystemicConstraint::Freedom::FREE;
            
            return flags;            
        };
                
        // Determine the coupling residuals for this agent
        set<PatternLink> residual_plinks;
        for( PatternLink residual_plink : my_master_boundary_links )
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


void AndRuleEngine::Plan::Dump()
{
    FTRACE( make_tuple(
        parent_residual_links_to_master_boundary_agents,
        normal_links_ordered 
    ) )("\n");
}


string AndRuleEngine::Plan::GetTrace() const
{
    return algo->GetName() + ".plan" + algo->GetSerialString();
}


void AndRuleEngine::StartCSPSolver(XLink root_xlink)
{    
    // Determine the full set of forces 
    // TODO presumably doesn't need to be the ordered one
    SolutionMap master_and_root_links;
    for( PatternLink link : plan.master_boundary_keyer_links )
    {
        // distinct OK because this only runs once per solve
        ASSERT( master_solution->count(link) == 1 )
              ("Master proxy link ")(link)(" not in:\n")(master_solution)("\n");
        TreePtr<Node> keynode = master_solution->at(link).GetChildX();
        master_and_root_links[link] = XLink::CreateDistinct(keynode);
    }
    master_and_root_links[plan.root_plink] = root_xlink;

    plan.solver->Start( master_and_root_links, knowledge );
}


void AndRuleEngine::GetNextCSPSolution( LocatedLink root_link )
{
    TRACE("GetNextCSPSolution()\n");
    SolutionMap csp_solution;
    bool match = plan.solver->GetNextSolution( &csp_solution );        
    if( !match )
        throw NoSolution();

    // Add the root variable/value, which is FORCED. Not sure why we have
    // to add this and not any other FIXED variable.
    csp_solution.insert( root_link );

    // Recreate my_coupling_keys
    for( pair< PatternLink, XLink > pxp : csp_solution )
        InsertSolo( basic_solution, pxp );                        
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
             plan.coupling_keyer_links_nontrivial.count( (PatternLink)link ), 
             plan.coupling_residual_links.count( (PatternLink)link ), 
             plan.my_master_boundary_links.count( (PatternLink)link ) );
        ASSERT( link.GetChildX() );
                             
        DecidedCompare(link);      
    }
}


void AndRuleEngine::DecidedCompare( LocatedLink link )  
{
    INDENT("D");
    ASSERT( link.GetChildAgent() ); // Pattern must not be nullptr
    ASSERT( link.GetChildX() ); // Target must not be nullptr
     
    InsertSolo( basic_solution, link );        
    SolutionMap combined_solution = UnionOfSolo( *master_solution,
                                                 basic_solution );
    Agent * const agent = link.GetChildAgent();
    
    // NOTE: Probable bug in couplings algo, see #315
    if( plan.coupling_residual_links.count( (PatternLink)link ) > 0 ||
        plan.my_master_boundary_links.count( (PatternLink)link ) > 0 )
    {
        agent->RunCouplingQuery( &combined_solution );
    }
    else
    {                                 
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
}


void AndRuleEngine::CompareEvaluatorLinks( Agent *agent, 
                                           const SolutionMap *solution_for_subordinates, 
                                           const SolutionMap *solution_for_evaluators ) 
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
        XLink xlink = solution_for_evaluators->at(link);
                                
        try 
        {
            shared_ptr<AndRuleEngine> e = plan.my_evaluator_abnormal_engines.at(link);
            e->Compare( xlink, solution_for_subordinates, knowledge );
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
                                              const SolutionMap *solution_for_subordinates ) 
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
            e->Compare( xe_link, solution_for_subordinates, knowledge );
        }
    }
    else if( auto xssl = dynamic_cast<SubSequence *>(xsc) )
    {
        for( XLink xe_link : xssl->elts )
        {
            TRACE("Comparing ")(xe_link)("\n");
            e->Compare( xe_link, solution_for_subordinates, knowledge );
        }
    }    
    else
    {
        ASSERTFAIL("unrecognised SubContainer\n");
    }
}


void AndRuleEngine::RegenerationPassAgent( Agent *agent,
                                           const SolutionMap &solution_for_subordinates )
{
    // Get a list of the links we must supply to the agent for regeneration
    auto pq = agent->GetPatternQuery();
    TRACE("Trying to regenerate ")(*agent)("\n");    
    TRACEC("Pattern links ")(pq->GetNormalLinks())("\n");    

#ifdef CHECK_EVERYTHING_IS_IN_DOMAIN      
    for( XLink xlink : pq->GetNormalLinks() )    
        ASSERT( knowledge->domain.count(xlink) > 0 )(xlink)(" not found in ")(knowledge->domain)(" (see issue #202)\n"); // #202 expected to cause this to fail
#endif
    
#ifdef NLQ_TEST
    auto nlq_lambda = agent->TestStartRegenerationQuery( &basic_solution, knowledge );
#else    
    auto nlq_lambda = agent->StartRegenerationQuery( &basic_solution, knowledge );
#endif
    
    int i=0;
    while(1)
    {
        shared_ptr<SR::DecidedQuery> query = nlq_lambda();
        i++;
                
        TRACE("Try out query, attempt %d (1-based)\n", i);    

        try // in here, only have the stuff that could throw a mismatch
        {
            Tracer::RAIIDisable silencer();   // Shush, I'm trying to debug the NLQs
            SolutionMap solution_for_evaluators;
            
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
                    e->Compare( link, &solution_for_subordinates, knowledge );
                }
            }                    
            
            // Try matching the multiplicity links.
            FOREACH( const LocatedLink &link, query->GetMultiplicityLinks() )
            {
                if( plan.my_evaluator_abnormal_engines.count( (PatternLink)link ) )
                    InsertSolo( solution_for_evaluators, link );                

                if( plan.my_multiplicity_engines.count( (PatternLink)link ) )
                    CompareMultiplicityLinks( link, &solution_for_subordinates );  
            }

            // Try matching the evaluator agents.
            if( plan.my_evaluators.count( agent ) )
                CompareEvaluatorLinks( agent, &solution_for_subordinates, &solution_for_evaluators );                    
        }
        catch( const ::Mismatch& mismatch )
        {
            TRACE("Caught Mismatch exception, retrying the lambda\n", i);    
            continue; // deal with exception by iterating the loop 
        }     
                                        
        // Replace needs these keys 
        FOREACH( const LocatedLink &link, query->GetAbnormalLinks() )
            InsertSolo( basic_solution, link );                
                
        // If we got here, we're done!
        TRACE("Success after %d tries\n", i);  
        break;
    } 
    agent->ResetNLQConjecture(); // save memory
}      


void AndRuleEngine::RegenerationPass()
{
    INDENT("R");
    const SolutionMap solution_for_subordinates = UnionOfSolo( *master_solution, basic_solution );   
    TRACE("---------------- Regeneration ----------------\n");      
    //TRACEC("Subordinate keys ")(keys_for_subordinates)("\n");       
    TRACEC("Basic solution ")(basic_solution)("\n");    

    for( Agent *agent : plan.my_normal_agents )
    {
        RegenerationPassAgent( agent, 
                               solution_for_subordinates );
    }

    TRACE("Regeneration complete\n");
}


// This one if you want the resulting couplings and conj (ie doing a replace imminently)
void AndRuleEngine::Compare( XLink root_xlink,
                             const SolutionMap *master_solution_,
                             const TheKnowledge *knowledge_ )
{
    INDENT("C");
    ASSERT( root_xlink );
    used = true;
         
    master_solution = master_solution_;    
    knowledge = knowledge_;
    
    LocatedLink root_link( plan.root_plink, root_xlink );

    TRACE("Compare root ")(root_link)("\n");

#ifdef CHECK_EVERYTHING_IS_IN_DOMAIN
    if( !dynamic_cast<StarAgent*>(root_link.GetChildAgent()) ) // Stars are based at SubContainers which don't go into domain    
        ASSERT( knowledge->domain.count(root_xlink) > 0 )(root_xlink)(" not found in ")(knowledge->domain)(" (see issue #202)\n");
#endif
                     
    if( ReadArgs::use_csp_solver )
        StartCSPSolver( root_xlink );
    else
        plan.conj->Start();
           
    // Create the conjecture object we will use for this compare, and keep iterating
    // though different conjectures trying to find one that allows a match.
    //int i=0;
    while(1)
    {
        basic_solution.clear();
        // Get a solution from the solver
        if( ReadArgs::use_csp_solver )        
            GetNextCSPSolution(root_link);        

        try
        {
            if( !ReadArgs::use_csp_solver )
            {
                // Try out the current conjecture. This will call RegisterDecision() once for each decision;
                // RegisterDecision() will return the current choice for that decision, if absent it will
                // add the decision and choose the first choice, if the decision reaches the end it
                // will remove the decision.    
                DecidedCompare( root_link );       
            }

            // Is the solution complete? 
            for( auto plink : plan.my_normal_links )
            {            
                ASSERT( basic_solution.count(plink) > 0 )("Cannot find normal link ")(plink)("\nIn ")(basic_solution)("\n");
            }
            RegenerationPass();
        }
        catch( const ::Mismatch& e )
        {                
            if( ReadArgs::use_csp_solver )
            {
                TRACE(e)(" after recursion, trying next solution\n");
                continue; // Get another solution from the solver
            }
            else
            {
                TRACE(e)(" after recursion, trying increment conjecture\n");
                if( plan.conj->Increment() )
                    continue; // Conjecture would like us to try again with new choices
                    
                plan.conj->Reset();
                // We didn't match and we've run out of choices, so we're done.              
                throw NoSolution();
            }
        }
        // We got a match so we're done. 
        TRACE("AndRuleEngine hit\n");
        break; // Success
    }
    
    if( !ReadArgs::use_csp_solver )
        plan.conj->Reset();
    
    // By now, we succeeded and slave_keys is the right set of keys
}


// This one operates from root for a stand-alone compare operation and
// no master keys.
void AndRuleEngine::Compare( TreePtr<Node> root_xnode )
{
    SolutionMap empty_solution;
    TheKnowledge empty_knowledge;
    XLink root_xlink = XLink::CreateDistinct(root_xnode);
    Compare( root_xlink, &empty_solution, &empty_knowledge );
}


const SolutionMap &AndRuleEngine::GetSolution()
{
    return basic_solution;
}


void AndRuleEngine::ClearSolution()
{
    basic_solution.clear();
}

    
const unordered_set<Agent *> &AndRuleEngine::GetKeyedAgents() const
{
   // We will key all our normal agents
   return plan.my_normal_agents;
}


const unordered_set<PatternLink> AndRuleEngine::GetKeyerPatternLinks() const
{
    unordered_set<PatternLink> keyer_plinks_incl_subs;
    //keyer_plinks_incl_subs = plan.coupling_keyer_links_all; 
    
    list<const AndRuleEngine *> subs = GetAndRuleEngines();
    for( const AndRuleEngine *e : subs )
        keyer_plinks_incl_subs = UnionOfSolo( keyer_plinks_incl_subs, e->plan.coupling_keyer_links_all );
        
    return keyer_plinks_incl_subs;
}


string AndRuleEngine::GetTrace() const
{
    string s = Traceable::GetName() + GetSerialString();
    return s;
}


list<const AndRuleEngine *> AndRuleEngine::GetAndRuleEngines() const
{
	list<const AndRuleEngine *> engines;
	engines.push_back( this );
	for( auto p : plan.my_free_abnormal_engines )
		engines = engines + p.second->GetAndRuleEngines();
	for( auto p : plan.my_evaluator_abnormal_engines )
		engines = engines + p.second->GetAndRuleEngines();
	for( auto p : plan.my_multiplicity_engines )
		engines = engines + p.second->GetAndRuleEngines();
	return engines;
}


string AndRuleEngine::GetGraphId() const
{
	return "And"+GetSerialString();
}


void AndRuleEngine::GenerateGraphRegions( Graph &graph, string scr_engine_id ) const
{
	GenerateMyGraphRegion(graph, scr_engine_id); // this engine
	for( auto p : plan.my_free_abnormal_engines )
		p.second->GenerateGraphRegions(graph, "");
	for( auto p : plan.my_evaluator_abnormal_engines )
		p.second->GenerateGraphRegions(graph, "");
	for( auto p : plan.my_multiplicity_engines )
		p.second->GenerateGraphRegions(graph, "");
}


void AndRuleEngine::GenerateMyGraphRegion( Graph &graph, string scr_engine_id ) const
{
    TRACE(" parent_residual_links_to_master_boundary_agents ")( plan.parent_residual_links_to_master_boundary_agents )("\n");
    TRACE(" master_boundary_agents ")( plan.master_boundary_agents )("\n");
	TRACE("Specifying figure nodes for ")(*this)("\n");
	Graph::Figure figure;
	figure.id = GetGraphId();
	figure.title = scr_engine_id.empty() ? GetGraphId() : scr_engine_id+" / "+GetGraphId();
    
	auto agents_lambda = [&](const unordered_map< Agent *, unordered_set<PatternLink> > &parent_links_to_agents,
                             const unordered_set<PatternLink> &keyers,
                             const unordered_set<PatternLink> &residuals ) -> list<Graph::Figure::Agent>
    {
        list<Graph::Figure::Agent> figure_agents;
        for( auto p : parent_links_to_agents )
        {
            Graph::Figure::Agent gf_agent;
            gf_agent.g = p.first;
            for( PatternLink plink : p.second )
            {
                Graph::Figure::Link link;
                link.short_name = plink.GetShortName();
                if( residuals.count(plink) > 0 )
                    link.details.planned_as = Graph::LINK_RESIDUAL;
                else if( keyers.count(plink) > 0 )
                    link.details.planned_as = Graph::LINK_KEYER;
                else
                    link.details.planned_as = Graph::LINK_NORMAL;
                gf_agent.incoming_links.push_back( link );
            }            
            TRACEC(*p.first)("\n");            
            figure_agents.push_back(gf_agent);
        }
        return figure_agents;
	};
    
	TRACE("   Interior (my agents/links):\n");    
    figure.interior_agents = agents_lambda( plan.parent_links_to_my_normal_agents,
                                            plan.coupling_keyer_links_nontrivial,
                                            plan.coupling_residual_links );
	TRACE("   Exterior (master boundary agents/links):\n");    
    figure.exterior_agents = agents_lambda( plan.parent_residual_links_to_master_boundary_agents,
                                            plan.master_boundary_keyer_links, // Won't show up as not in p_r_l_t_m_b_a, but could generate invisible nodes and links?
                                            plan.my_master_boundary_links );       
        
	auto subordinates_lambda = [&](const unordered_map< PatternLink, shared_ptr<AndRuleEngine> > &engines, Graph::LinkPlannedAs incoming_link_planned_as )
    {
        set< shared_ptr<AndRuleEngine> > reached;
        for( auto p : engines )
        {
            ASSERT( reached.count(p.second) == 0 );
            reached.insert( p.second );
            
            Graph::Figure::Agent root_agent;
            root_agent.g = p.second->plan.root_agent;
            Graph::Figure::Link incoming_link;
            incoming_link.details.planned_as = incoming_link_planned_as;
            incoming_link.short_name = p.first.GetShortName();
            root_agent.incoming_links.push_back( incoming_link );
            TRACEC(p.second.get())(" : ( ")(incoming_link.short_name)("->")(root_agent.g->GetGraphId())(" )\n");
            figure.subordinate_engines_and_root_agents.push_back( make_pair(p.second.get(), root_agent) );
        }
	};
	TRACE("   Subordinates (my free abnormals):\n");    
    subordinates_lambda( plan.my_free_abnormal_engines, Graph::LINK_ABNORMAL );
	TRACE("   Subordinates (my evaluator abnormals):\n");    
    subordinates_lambda( plan.my_evaluator_abnormal_engines, Graph::LINK_EVALUATOR );
	TRACE("   Subordinates (my multiplicity engines):\n");    
    subordinates_lambda( plan.my_multiplicity_engines, Graph::LINK_MULTIPLICITY );
	TRACE("Ready to render\n");
	graph(figure);
}

