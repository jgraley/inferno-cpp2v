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
#include "coupling.hpp"
#include "tree/cpptree.hpp"
#include "equivalence.hpp"
#include "render/graph.hpp"

#include <list>

//#define TEST_PATTERN_QUERY

// This now works!
//#define USE_SOLVER
 
//#define CHECK_EVERYTHING_IS_IN_DOMAIN

//#define NLQ_TEST

using namespace SR;

AndRuleEngine::AndRuleEngine( PatternLink root_plink, 
                              const unordered_set<PatternLink> &master_plinks,
                              const SerialNumber *serial_to_use ) :
    SerialNumber( false, serial_to_use ),
    plan( this, root_plink, master_plinks )
{
}    


AndRuleEngine::~AndRuleEngine() 
{ 
    //ASSERT( used )( plan.root_plink );
}

 
AndRuleEngine::Plan::Plan( AndRuleEngine *algo_, 
                           PatternLink root_plink_, 
                           const unordered_set<PatternLink> &master_plinks_ ) :
    algo( algo_ ),
    root_plink( root_plink_ ),
    root_pattern( root_plink.GetPattern() ),
    root_agent( root_plink.GetChildAgent() ),
    master_plinks( master_plinks_ )
{
    TRACE(algo->GetTrace())(" planning\n");
    INDENT("P");
    
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
    for( PatternLink plink : master_boundary_links )
        parent_residual_links_to_master_boundary_agents[plink.GetChildAgent()].insert(plink);
    // master_boundary_agents should be same set of agents as those reached by master_boundary_links (uniquified)
    ASSERT( parent_residual_links_to_master_boundary_agents.size() == master_boundary_agents.size() );
        
    DetermineKeyers( root_plink, master_agents );
    DetermineResiduals( root_agent, master_agents );
    DetermineNontrivialKeyers();
    
    // Well, obviously...
    ASSERT( my_normal_links_unique_by_agent.size()==my_normal_agents.size() );
    
    ConfigureAgents();       
    unordered_set<PatternLink> surrounding_plinks = UnionOf( my_normal_links, master_plinks );         
    CreateSubordniateEngines( my_normal_agents, surrounding_plinks );   
    
    // Trivial problem checks   
    if( my_normal_links.empty() ) 
    {        
        ASSERT( my_normal_agents.empty() );
        // Root link obviously isn't in my_normal_links because that's empty, 
        // so it needs to be found in master_boundary_links
        ASSERT( master_boundary_links.count(root_plink) == 1 )
              ("\nmbrl:\n")(master_boundary_links);
    }
    else
    {
        ASSERT( !my_normal_agents.empty() );
    }

#ifdef USE_SOLVER   
    {
        list< shared_ptr<CSP::Constraint> > constraints_list;
        CreateMyFullConstraints(constraints_list);
        CreateMasterCouplingConstraints(constraints_list);
        CreateCSPSolver(constraints_list);
        // Note: constraints_list drops out of scope and discards its 
        // references; only constraints held onto by solver will remain.
    }
#else
    conj = make_shared<Conjecture>(my_normal_agents, root_agent);
#endif                      
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
        master_boundary_links.insert( link );

    normal_links_ordered.push_back( link );    

    if( reached_agents.count(agent) > 0 )    
        return; 
    reached_agents.insert(agent);
    // ------------ Now unique by agent (stronger) -------------

    if( master_agents.count( agent ) > 0 )
    {
        // At master boundary so don't recurse
        master_boundary_agents.insert( agent );

        // We don't need the original keyer link for this agent (it belongs
        // to master) so just create a new one. These will be the FORCED
        // variables that permit us to inject master keys into CSP. I'm hoping
        // we only need one per agent. TODO do this some other way, even
        // if that de-abstracts the CSP solver interface a bit.
        TreePtr<Node> agent_pattern = link.GetPattern();
        master_proxy_keyer_links.insert( PatternLink::CreateDistinct(agent_pattern) ); 
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
    for( PatternLink l : my_normal_links_unique_by_agent )        
        ASSERT( l.GetChildAgent() != plink.GetChildAgent() )
              ("Conflicting coupling in and-rule pattern: check Disjunction nodes\n");

    my_normal_links_unique_by_agent.insert(plink);

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
    // Where that happens, there will be a conflict writing to coupling_keyer_links and the
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
        for( PatternLink l : my_normal_links_unique_by_agent )
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
    coupling_keyer_links.clear();
    for( PatternLink keyer_plink : my_normal_links_unique_by_agent )
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
            coupling_keyer_links.insert( keyer_plink );
        }
    }
}


void AndRuleEngine::Plan::ConfigureAgents()
{
    for( PatternLink keyer_plink : my_normal_links_unique_by_agent )
    {
        ASSERT( keyer_plink );
        Agent *agent = keyer_plink.GetChildAgent();
                
        set< PatternLink > residual_plinks;
        for( PatternLink residual_plink : coupling_residual_links )
            if( residual_plink.GetChildAgent() == agent )
                residual_plinks.insert( residual_plink );
        /*
         * This gets no hits: my_normal_links_unique_by_agent only contains
         * links to my agents, whereas master_boundary_links only
         * contains links to master agents. There's no easy way around this
         * since we should only configure my agents.
         
        for( PatternLink residual_plink : master_boundary_links )
            if( residual_plink.GetChildAgent() == agent )
                residual_plinks.insert( residual_plink );
          */  
        agent->AndRuleConfigure( algo, keyer_plink, residual_plinks );
    }

    if( ReadArgs::new_feature )
    {
        for( PatternLink residual_plink : master_boundary_links )
        {
            ASSERT( residual_plink );
            Agent *agent = residual_plink.GetChildAgent();
            agent->AddResiduals( {residual_plink} );
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
                                                    const unordered_set<PatternLink> &surrounding_plinks )
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
                my_evaluator_abnormal_engines[link] = make_shared<AndRuleEngine>( link, surrounding_plinks );  
            }
            else
            {
                my_free_abnormal_engines[link] = make_shared<AndRuleEngine>( link, surrounding_plinks );  
            }
        }
        
        FOREACH( PatternLink link, pq->GetMultiplicityLinks() )
        {
            my_multiplicity_engines[link] = make_shared<AndRuleEngine>( link, surrounding_plinks );  
        }
    }
}


void AndRuleEngine::Plan::CreateMyFullConstraints( list< shared_ptr<CSP::Constraint> > &constraints_list )
{
    unordered_set<Agent *> check_we_got_the_right_agents;
    for( PatternLink keyer_plink : my_normal_links_unique_by_agent )
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
    for( PatternLink keyer_plink : master_proxy_keyer_links )
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
        for( PatternLink residual_plink : master_boundary_links )
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


string AndRuleEngine::Plan::GetTrace() const
{
    return algo->GetName() + "::Plan" + algo->GetSerialString();
}


void AndRuleEngine::StartCSPSolver(XLink root_xlink)
{    
    // Determine the full set of forces 
    // TODO presumably doesn't need to be the ordered one
    SolutionMap master_and_root_links;
    for( PatternLink link : plan.master_proxy_keyer_links )
    {
        // distinct OK because this only runs once per solve
        TreePtr<Node> keynode = master_keys->at(link.GetChildAgent()).GetKeyXNode(KEY_CONSUMER_4);
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
        RecordLink( LocatedLink(pxp), KEY_PRODUCER_4 );                        
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
             plan.coupling_keyer_links.count( (PatternLink)link ), 
             plan.coupling_residual_links.count( (PatternLink)link ), 
             plan.master_boundary_links.count( (PatternLink)link ) );
        ASSERT( link.GetChildX() );
                             
        DecidedCompare(link);      
    }
}


void AndRuleEngine::DecidedCompare( LocatedLink link )  
{
    INDENT("D");
    ASSERT( link.GetChildAgent() ); // Pattern must not be nullptr
    ASSERT( link.GetChildX() ); // Target must not be nullptr
     
    // NOTE: Probable bug in couplings algo, see #315
    if( plan.coupling_residual_links.count( (PatternLink)link ) > 0 )
    {
        CompareCoupling( external_keys, link, KEY_CONSUMER_1 );
    }
    else if( plan.master_boundary_links.count( (PatternLink)link ) > 0 )
    {
        CompareCoupling( *master_keys, link, KEY_CONSUMER_6 );
    }
    else
    {                         
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
    RecordLink( link, KEY_PRODUCER_1 );        
}


void AndRuleEngine::CompareEvaluatorLinks( Agent *agent, 
                                           const CouplingKeysMap *keys_for_subordinates, 
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
            e->Compare( xlink, keys_for_subordinates, knowledge );
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
                                              const CouplingKeysMap *keys_for_subordinates ) 
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
            e->Compare( xe_link, keys_for_subordinates, knowledge );
        }
    }
    else if( auto xssl = dynamic_cast<SubSequence *>(xsc) )
    {
        for( XLink xe_link : xssl->elts )
        {
            TRACE("Comparing ")(xe_link)("\n");
            e->Compare( xe_link, keys_for_subordinates, knowledge );
        }
    }    
    else
    {
        ASSERTFAIL("unrecognised SubContainer\n");
    }
}


void AndRuleEngine::RegenerationPassAgent( Agent *agent,
                                           const CouplingKeysMap &keys_for_subordinates )
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
            Tracer::RAIIEnable silencer( false );   // Shush, I'm trying to debug the NLQs
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
                    e->Compare( link, &keys_for_subordinates, knowledge );
                }
            }                    
            
            // Try matching the multiplicity links.
            FOREACH( const LocatedLink &link, query->GetMultiplicityLinks() )
            {
                if( plan.my_evaluator_abnormal_engines.count( (PatternLink)link ) )
                    InsertSolo( solution_for_evaluators, link );                

                if( plan.my_multiplicity_engines.count( (PatternLink)link ) )
                    CompareMultiplicityLinks( link, &keys_for_subordinates );  
            }

            // Try matching the evaluator agents.
            if( plan.my_evaluators.count( agent ) )
                CompareEvaluatorLinks( agent, &keys_for_subordinates, &solution_for_evaluators );                    
        }
        catch( const ::Mismatch& mismatch )
        {
            TRACE("Caught Mismatch exception, retrying the lambda\n", i);    
            continue; // deal with exception by iterating the loop 
        }     
                                        
        // Replace needs these keys 
        FOREACH( const LocatedLink &link, query->GetAbnormalLinks() )
            if( plan.my_free_abnormal_engines.count( (PatternLink)link ) )                        
                KeyCoupling( external_keys, link, KEY_PRODUCER_2 );
                
        // If we got here, we're done!
        TRACE("Success after %d tries\n", i);  
        break;
    } 
    agent->ResetNLQConjecture(); // save memory
}      


void AndRuleEngine::RegenerationPass()
{
    INDENT("R");
    const CouplingKeysMap keys_for_subordinates = UnionOfSolo( *master_keys, external_keys );   
    TRACE("---------------- Regeneration ----------------\n");      
    //TRACEC("Subordinate keys ")(keys_for_subordinates)("\n");       
    TRACEC("Basic solution ")(basic_solution)("\n");    

    for( Agent *agent : plan.my_normal_agents )
    {
        RegenerationPassAgent( agent, 
                               keys_for_subordinates );
    }

    TRACE("Regeneration complete\n");
}


// This one if you want the resulting couplings and conj (ie doing a replace imminently)
void AndRuleEngine::Compare( XLink root_xlink,
                             const CouplingKeysMap *master_keys_,
                             const TheKnowledge *knowledge_ )
{
    INDENT("C");
    ASSERT( root_xlink );
    used = true;
         
    master_keys = master_keys_;    
    knowledge = knowledge_;
    
    LocatedLink root_link( plan.root_plink, root_xlink );

    TRACE("Compare root ")(root_link)("\n");

#ifdef CHECK_EVERYTHING_IS_IN_DOMAIN
    if( !dynamic_cast<StarAgent*>(root_link.GetChildAgent()) ) // Stars are based at SubContainers which don't go into domain    
        ASSERT( knowledge->domain.count(root_xlink) > 0 )(root_xlink)(" not found in ")(knowledge->domain)(" (see issue #202)\n");
#endif
                     
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
#ifdef USE_SOLVER        
        // Get a solution from the solver
        GetNextCSPSolution(root_link);        
#endif
        try
        {
#ifndef USE_SOLVER
            // Try out the current conjecture. This will call RegisterDecision() once for each decision;
            // RegisterDecision() will return the current choice for that decision, if absent it will
            // add the decision and choose the first choice, if the decision reaches the end it
            // will remove the decision.    
            DecidedCompare( root_link );       
#endif
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


const CouplingKeysMap &AndRuleEngine::GetCouplingKeys()
{
    return external_keys;
}


const void AndRuleEngine::ClearCouplingKeys()
{
    external_keys.clear();
}


void AndRuleEngine::RecordLink( LocatedLink link, KeyProducer place )
{
    // All go into the basic solution which is enough to
    // regenerate a full solution.
    InsertSolo( basic_solution, link );                
    
    // Keying for external use (subordinates, slaves and replace)
    // We don't want residuals (which are unreliable) or MMAX
    if( plan.master_boundary_links.count( (PatternLink)link ) == 0 &&
        plan.coupling_residual_links.count( (PatternLink)link ) == 0 && 
        (XLink)link != XLink::MMAX_Link )
        KeyCoupling( external_keys, link, place );        
}


void AndRuleEngine::CompareCoupling( const CouplingKeysMap &keys, const LocatedLink &residual_link, KeyConsumer consumer )
{
    Agent *agent = residual_link.GetChildAgent();
    ASSERT( keys.count(agent) > 0 );
    XLink keyer_xlink = keys.at(agent).GetKeyXLink(consumer);

    //FTRACE(keys.at(agent))("\n");

    // Enforce rule #149
    ASSERT( !TreePtr<SubContainer>::DynamicCast( keyer_xlink.GetChildX() ) ); 

    multiset<XLink> candidate_links { keyer_xlink, residual_link };
    
    // Only filling in two required liks: keyer and one residual - this may
    // lead to a partial query if there are really more residuals.
    // Note: it would be great to split the constraint into binaries (keyer 
    // plus one residual) and then this old solver could use those and 
    // not need to make partial queries)
    SolutionMap required_links;
    if( ReadArgs::new_feature )
    {
        PatternLink keyer_plink = agent->GetKeyerPatternLink();
        required_links[keyer_plink] = keyer_xlink;
        required_links[(PatternLink)residual_link] = (XLink)residual_link;
    }
    
    agent->RunCouplingQuery( &required_links, candidate_links );
}                                     


void AndRuleEngine::KeyCoupling( CouplingKeysMap &keys, const LocatedLink &keyer_link, KeyProducer place )
{
    // A coupling keyed to Magic-Match-Anything-X would not be able to 
    // restrict the residuals wrt to each other. 
    ASSERT( (XLink)keyer_link != XLink::MMAX_Link );
    
    ASSERT( plan.coupling_residual_links.count((PatternLink)keyer_link) == 0 );
    
    // A coupling relates the coupled agent to an X node, not the
    // link into the agent.
    CouplingKey key( keyer_link, place, this, nullptr );
    InsertSolo( keys, make_pair( keyer_link.GetChildAgent(), key ) ); 
}                                                       
 

void AndRuleEngine::AssertNewCoupling( const CouplingKeysMap &extracted, Agent *new_agent, XLink new_xlink, Agent *parent_agent )
{
    TreePtr<Node> new_xnode = new_xlink.GetChildX();
    ASSERT( extracted.count(new_agent) == 1 );
    CouplingKey extracted_key = extracted.at(new_agent);
    XLink extracted_xlink = extracted_key.GetKeyXLink(KEY_CONSUMER_2);
    TreePtr<Node> extracted_xnode = extracted_xlink.GetChildX();
    
    if( TreePtr<SubContainer>::DynamicCast(new_xnode) )
    {                    
        EquivalenceRelation equivalence_relation;
        CompareResult cr = equivalence_relation.Compare( extracted_xlink, new_xlink );
        if( cr != EQUAL )
        {
            FTRACE("New x node ")(new_xnode)(" mismatches extracted x ")(extracted_key)
                  (" for agent ")(new_agent)(" with parent ")(parent_agent)("\n");
            if( TreePtr<SubSequence>::DynamicCast(new_xnode) && TreePtr<SubSequence>::DynamicCast(extracted_xnode))
                FTRACEC("SubSequence\n");
            else if( TreePtr<SubSequenceRange>::DynamicCast(new_xnode) && TreePtr<SubSequenceRange>::DynamicCast(extracted_xnode))
                FTRACEC("SubSequenceRange\n");
            else if( TreePtr<SubCollection>::DynamicCast(new_xnode) && TreePtr<SubCollection>::DynamicCast(extracted_xnode))
                FTRACEC("SubCollections\n");
            else
                FTRACEC("Container types don't match\n");
            ContainerInterface *xc = dynamic_cast<ContainerInterface *>(extracted_xnode.get());
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
        ASSERT( extracted_xnode == new_xnode );
    }
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
    TRACE(*this)(" parent_residual_links_to_master_boundary_agents ")( plan.parent_residual_links_to_master_boundary_agents )("\n");
    TRACE(*this)(" master_boundary_agents ")( plan.master_boundary_agents )("\n");
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
                                            plan.coupling_keyer_links,
                                            plan.coupling_residual_links );
	TRACE("   Exterior (master boundary agents/links):\n");    
    figure.exterior_agents = agents_lambda( plan.parent_residual_links_to_master_boundary_agents,
                                            plan.master_proxy_keyer_links, // Won't show up as not in p_r_l_t_m_b_a, but could generate invisible nodes and links?
                                            plan.master_boundary_links );       
        
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
            figure.subordinate_engines_and_root_agents[p.second.get()] = root_agent;
        }
	};
	TRACE("   Subordinates (my free abnormals):\n");    
    subordinates_lambda( plan.my_free_abnormal_engines, Graph::LINK_ABNORMAL );
	TRACE("   Subordinates (my evaluator abnormals):\n");    
    subordinates_lambda( plan.my_evaluator_abnormal_engines, Graph::LINK_EVALUATOR );
	TRACE("   Subordinates (my multiplicity engines):\n");    
    subordinates_lambda( plan.my_multiplicity_engines, Graph::LINK_MULTIPLICITY );
	TRACE("Ready to render ")(*this)("\n");
	graph(figure);
}

