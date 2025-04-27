#include "and_rule_engine.hpp"

#include "csp/symbolic_constraint.hpp"
#include "csp/reference_solver.hpp"
#include "csp/backjumping_solver.hpp"
#include "csp/solver_holder.hpp"
#include "csp/solver_factory.hpp"
#include "scr_engine.hpp"
#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "agents/embedded_scr_agent.hpp"
#include "agents/standard_agent.hpp"
#include "agents/delta_agent.hpp"
#include "agents/depth_agent.hpp"
#include "agents/star_agent.hpp"
#include "common/common.hpp"
#include "agents/disjunction_agent.hpp"
#include "link.hpp"
#include "tree/cpptree.hpp"
#include "render/graph.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/rewriters.hpp"
#include "sym/expression_analysis.hpp"

#include <list>
 
//#define CHECK_EVERYTHING_IS_IN_DOMAIN

//#define NLQ_TEST

#define CHECK_FOR_SURROUNDING_KEYERS

// Enabling this is making the Karnaugh map finder slow down #586
//#define EXTRA_CONDENSED_CONSTRAINTS

using namespace SR;

AndRuleEngine::AndRuleEngine( PatternLink base_plink, 
                              const set<PatternLink> &surrounding_plinks,
                              const set<PatternLink> &surrounding_keyer_plinks ) :
    plan( this, base_plink, surrounding_plinks, surrounding_keyer_plinks )
{
}    


AndRuleEngine::~AndRuleEngine() 
{ 
}

 
AndRuleEngine::Plan::Plan( AndRuleEngine *algo_, 
                           PatternLink base_plink_, 
                           const set<PatternLink> &surrounding_plinks_,
                           const set<PatternLink> &surrounding_keyer_plinks_ ) :
    algo( algo_ ),
    base_plink( base_plink_ ),
    base_pattern( base_plink.GetPattern() ),
    base_agent( base_plink.GetChildAgent() ),
    surrounding_plinks( surrounding_plinks_ ),
    surrounding_keyer_plinks( surrounding_keyer_plinks_ )
{    
    INDENT("P");
    TRACE(algo->GetTrace())(" planning\n");
    
    // ------------------ Fill in the plan ---------------------
    surrounding_agents.clear();
    for( PatternLink plink : surrounding_plinks )
        surrounding_agents.insert( plink.GetChildAgent() );

    set<Agent *> normal_agents;
    set<PatternLink> normal_links;
    PopulateNormalAgents( &normal_agents, &normal_links, base_plink );    
    
    for( PatternLink plink : normal_links )
        if( surrounding_agents.count( plink.GetChildAgent() ) == 0 )
            my_normal_links.insert( plink );
            
    my_normal_agents = DifferenceOf( normal_agents, surrounding_agents );       
    reached_agents.clear();
    reached_links.clear();    
    PopulateBoundaryAgents( base_plink, 
                           surrounding_agents );

    // Collect together the parent links to agents
    for( PatternLink plink : my_normal_links )
        parent_links_to_my_normal_agents[plink.GetChildAgent()].insert(plink);
    // my_normal_agents should be same set of agents as those reached by my_normal_links (uniquified)
    ASSERT( parent_links_to_my_normal_agents.size() == my_normal_agents.size() );
    for( PatternLink plink : my_boundary_links )
        parent_residual_links_to_boundary_agents[plink.GetChildAgent()].insert(plink);
    // boundary_agents should be same set of agents as those reached by my_boundary_links (uniquified)
    ASSERT( parent_residual_links_to_boundary_agents.size() == boundary_agents.size() );
        
    DetermineKeyers( base_plink, surrounding_agents );
    DetermineResiduals( base_agent, surrounding_agents );
    
    // Turns out these two are the same
    my_normal_links_unique_by_agent = coupling_keyer_links_all;

    boundary_keyer_links.clear();
    for( PatternLink plink : surrounding_keyer_plinks )
        if( boundary_agents.count(plink.GetChildAgent()) == 1 )
            boundary_keyer_links.insert( plink );
        
    my_fixed_keyer_links = { base_plink };
    
    // ------------------ Log the plan ---------------------
    Dump();
    
    // ------------------ Check the plan ---------------------
    // Well, obviously...
    ASSERT( my_normal_links_unique_by_agent.size()==my_normal_agents.size() );
    
    // Trivial problem checks   
    if( my_normal_links.empty() ) 
    {        
        ASSERT( my_normal_agents.empty() );
        // Root link obviously isn't in my_normal_links because that's empty, 
        // so it needs to be found in my_boundary_links
        ASSERT( my_boundary_links.count(base_plink) == 1 )
              ("\nmbrl:\n")(my_boundary_links);
    }
    else
    {
        ASSERT( !my_normal_agents.empty() );
    }

    // ------------------ Set up Agents ---------------------
    ConfigureAgents();       

    // ------------------ Set up Symbolics ---------------------
    DeduceCSPVariables(); 
    CreateMyFullSymbolics();
    CreateBoundarySymbolics();    
    SymbolicRewrites();
    
    // ------------------ Configure subordinates ---------------------
    // Do this last to keep all the rest of the planning trace/dumps
    // all together in the same pre-order sequence instead of mixed pre 
    // and post order
    set<PatternLink> subordinate_surrounding_plinks = UnionOf( my_normal_links, surrounding_plinks );         
    set<PatternLink> subordinate_surrounding_keyer_plinks = UnionOf( coupling_keyer_links_all, surrounding_keyer_plinks );         
    CreateSubordniateEngines( my_normal_agents, subordinate_surrounding_plinks, subordinate_surrounding_keyer_plinks );          
}


void AndRuleEngine::Plan::PlanningStageFive( shared_ptr<const Lacing> lacing )
{   
    // ------------------ Set up CSP solver ---------------------   
    list< shared_ptr<CSP::Constraint> > constraints_list;
    CreateMyConstraints(constraints_list, lacing);
    
    // Boundary links may not be in our domain if eg they got 
    // deleted by the enclosing replace. So base_plink is the only one that
    // we can guarantee will be in the domain.
    solver_holder = CreateSolverAndHolder( constraints_list, 
                                           ToVector(free_normal_links_ordered), 
                                           my_fixed_keyer_links,
                                           boundary_keyer_links );    
                                    
    // Note: constraints_list drops out of scope and discards its 
    // references; only constraints held onto by solver will remain.
    solver_holder->Dump();    
    solver_holder->CheckPlan();   
    
    // ------------------ Stage five on subordinates ---------------------
    for( auto p : my_free_abnormal_engines )
        p.second->PlanningStageFive(lacing);
    for( auto p : my_evaluator_abnormal_engines )
        p.second->PlanningStageFive(lacing);
    for( auto p : my_multiplicity_engines )
        p.second->PlanningStageFive(lacing);    
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
    for( PatternLink link : pq->GetNormalLinks() )
    {
        PopulateNormalAgents( normal_agents, normal_links, link );        
    }
}


void AndRuleEngine::Plan::PopulateBoundaryAgents( PatternLink link,
                                                 const set<Agent *> &surrounding_agents )
{
    // Definition: it's a boundary link/agent if:
    // 1. It's a surrounding agent
    // 2. It's not the child of a surrounding agent (we don't recurse on them)
    // See #125
    
    if( reached_links.count(link) > 0 )    
        return; 
    reached_links.insert(link);
    // ------------ Now unique by plink (weaker) -------------

    Agent *agent = link.GetChildAgent();
    
    // Note: here, we WILL see root if root is a surrounding agent (i.e. trivial pattern)
    if( surrounding_agents.count( agent ) )
        my_boundary_links.insert( link );

    normal_and_boundary_links_preorder.push_back( link );    

    if( reached_agents.count(agent) > 0 )    
        return; 
    reached_agents.insert(agent);
    // ------------ Now unique by agent (stronger) -------------

    if( surrounding_agents.count( agent ) > 0 )
    {
        // At boundary so don't recurse
        boundary_agents.insert( agent );
    } 
    else
    {
        // Not yet at boundary so recurse
        shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
        for( PatternLink link : pq->GetNormalLinks() )
            PopulateBoundaryAgents( link, surrounding_agents );        
    }
}

        
void AndRuleEngine::Plan::DetermineKeyersModuloDisjunction( PatternLink plink,
                                                            set<Agent *> *senior_agents,
                                                            set<Agent *> *disjunction_agents )
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
    for( PatternLink plink : pq->GetNormalLinks() )
    {
        DetermineKeyersModuloDisjunction( plink, senior_agents, disjunction_agents );        
    }
}

        
void AndRuleEngine::Plan::DetermineKeyers( PatternLink plink,
                                           set<Agent *> senior_agents ) 
{
    // See rule #384
    // Scan the senior region. We wish to break off at Disjunction nodes. Senior is the
    // region up to and including a Disjunction; junior is the region under each of its
    // links.
    set<Agent *> my_disjunction_agents;
    DetermineKeyersModuloDisjunction( plink, &senior_agents, &my_disjunction_agents );
    // After this:
    // - senior_agents has union of surrounding_agents and all the identified keyed agents
    // - my_disjunction_agents has the Disjunction agents that we saw, BUT SKIPPED
    
    // Now do all the links under the Disjunction nodes' links. Keying is allowed in each
    // of these junior regions individually, but no cross-keying is allowed if not keyed already.
    // Where that happens, there will be a conflict writing to coupling_keyer_links_nontrivial and the
    // ASSERT will fail.
    for( Agent *ma_agent : my_disjunction_agents )
    {
        shared_ptr<PatternQuery> pq = ma_agent->GetPatternQuery();
        for( PatternLink link : pq->GetNormalLinks() )
        {
            DetermineKeyers( link, senior_agents );        
        }
    }
}
        
        
void AndRuleEngine::Plan::DetermineResiduals( Agent *agent,
                                              set<Agent *> surrounding_agents ) 
{
    shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
    for( PatternLink link : pq->GetNormalLinks() )
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
        
        DetermineResiduals( link.GetChildAgent(), surrounding_agents );        
    }
}


void AndRuleEngine::Plan::ConfigureAgents()
{
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
    // Where we have a link to a surrounding agent (= boundary link)
    // it will be residual because surrounding engine always keys. Agent needs to 
    // know because we'll query it, so tell it.
    for( PatternLink boundary_plink : my_boundary_links )
    {
        ASSERT( boundary_plink );
        Agent *agent = boundary_plink.GetChildAgent();
        agent->AddResiduals( {boundary_plink} );
    }
}


void AndRuleEngine::Plan::DeduceCSPVariables()
{
    for( PatternLink link : normal_and_boundary_links_preorder )
    {
        if( my_fixed_keyer_links.count(link)==0 )
            free_normal_links_ordered.push_back( link );
        current_solve_plinks.insert( link );
    }
    
    for( PatternLink link : boundary_keyer_links )
        current_solve_plinks.insert( link );    
}


void AndRuleEngine::Plan::CreateMyFullSymbolics()
{
    for( PatternLink keyer_plink : my_normal_links_unique_by_agent ) // Only one constraint per agent
    {
        Agent *agent = keyer_plink.GetChildAgent();
        SYM::Lazy<SYM::BooleanExpression> op = agent->SymbolicQuery(false);
        expressions_from_agents.insert( op );
    }
}


void AndRuleEngine::Plan::CreateBoundarySymbolics()
{
#ifdef CHECK_FOR_SURROUNDING_KEYERS
    // First do some checking
    for( PatternLink residual_plink : my_boundary_links )
    {
        Agent *agent = residual_plink.GetChildAgent();
        bool found_keyer = false;
        for( PatternLink keyer_plink : boundary_keyer_links )
            if( keyer_plink.GetChildAgent()==agent )
                found_keyer = true;
        ASSERT( found_keyer )("Boundary residual plink ")(residual_plink)(" has no keyer\n");
    }
#endif
    
    for( PatternLink keyer_plink : boundary_keyer_links )
    {                                    
        Agent *agent = keyer_plink.GetChildAgent();
        SYM::Lazy<SYM::BooleanExpression> op = agent->SymbolicQuery(true);
        expressions_from_agents.insert( op );
    }
}


void AndRuleEngine::Plan::SymbolicRewrites()
{
    for( shared_ptr<SYM::BooleanExpression> expr : expressions_from_agents )
        SYM::PredicateAnalysis::CheckRegularPredicateForm( expr );
        
    //TRACE("expressions_from_agents:\n")(expressions_from_agents)("\n");
    expressions_split = SYM::PreprocessForEngine()(expressions_from_agents);
    //TRACE("expressions_split:\n")(expressions_split)("\n");

    for( auto bexpr : expressions_split )
    {
        // Constraint will require these variables
        set<PatternLink> required_plinks = bexpr->GetRequiredVariables();
        
        // If required plinks are not a subset of the current solve, the
        // constraint's requirements will not be met. Hopefully another
        // AndRuleEngine will (TODO check this).
        if( IsIncludes( current_solve_plinks, required_plinks ) )
            expressions_for_current_solve.insert(bexpr);    
    }        
}


void AndRuleEngine::Plan::CreateSubordniateEngines( const set<Agent *> &normal_agents, 
                                                    const set<PatternLink> &subordinate_surrounding_plinks, 
                                                    const set<PatternLink> &subordinate_surrounding_keyer_plinks )
{
    for( PatternLink plink : my_normal_links_unique_by_agent )
    {
        Agent *agent = plink.GetChildAgent();
        
        shared_ptr<PatternQuery> pq = agent->GetPatternQuery();
            
        if( pq->GetEvaluator() )
            my_evaluators.insert(agent);
        
        for( PatternLink link : pq->GetAbnormalLinks() )
        {        
            if( pq->GetEvaluator() )
            {
                my_evaluator_abnormal_engines[link] = make_shared<AndRuleEngine>( link, 
                                                                                  subordinate_surrounding_plinks, 
                                                                                  subordinate_surrounding_keyer_plinks );  
            }
            else
            {
                my_free_abnormal_engines[link] = make_shared<AndRuleEngine>( link, 
                                                                             subordinate_surrounding_plinks, 
                                                                             subordinate_surrounding_keyer_plinks );  
            }
        }
        
        for( PatternLink link : pq->GetMultiplicityLinks() )
        {
            my_multiplicity_engines[link] = make_shared<AndRuleEngine>( link, 
                                                                        subordinate_surrounding_plinks, 
                                                                        subordinate_surrounding_keyer_plinks );  
        }
    }
}


void AndRuleEngine::Plan::CreateMyConstraints( list< shared_ptr<CSP::Constraint> > &constraints_list,
                                               shared_ptr<const Lacing> lacing )
{
    for( auto bexpr : expressions_for_current_solve )
    {        
        // Constraint will require these variables
        set<PatternLink> required_plinks = bexpr->GetRequiredVariables();
        
        // Uniquify sets of expressions wrt required_plinks
        expressions_condensed[required_plinks].insert( bexpr );
    }
    
#ifdef EXTRA_CONDENSED_CONSTRAINTS
    bool changed;
    do
    {
        changed = false;
        for( auto &p1 : expressions_condensed )
        {
            for( auto &p2 : expressions_condensed )
            {
                if( p1.first != p2.first && IsIncludes(p1.first, p2.first) )
                {
                    p1.second = UnionOf( p1.second, p2.second );
                    EraseSolo( expressions_condensed, p2.first );
                    changed = true;
                    goto DONE;
                }
            }
        }
        DONE:;
    } while( changed );
#endif    
    
    for( auto p : expressions_condensed )
    {
        // Tidily AND-together p.second, which is the set of expressions for the constraint
        list<shared_ptr<SYM::BooleanExpression>> bexpr_list = ToList(p.second);        
        shared_ptr<SYM::BooleanExpression> bexpr = SYM::CreateTidiedOperator<SYM::AndOperator>(true)(bexpr_list);

        SYM::PredicateAnalysis::CheckRegularPredicateForm( bexpr );

        auto c = make_shared<CSP::SymbolicConstraint>(bexpr, lacing);
        constraints_list.push_back(c);    
    }        
}


void AndRuleEngine::Plan::Dump()
{
    list<KeyValuePair> plan_as_strings = 
    {
        { "base_plink", 
          Trace(base_plink) },
        { "base_pattern", 
          Trace(base_pattern) },
        { "base_agent", 
          Trace(base_agent) },
        { "surrounding_plinks", 
          Trace(surrounding_plinks) },
        { "surrounding_keyer_plinks", 
          Trace(surrounding_keyer_plinks) },
        { "surrounding_agents", 
          Trace(surrounding_agents) },
        { "my_normal_agents", 
          Trace(my_normal_agents) },
        { "my_normal_links", 
          Trace(my_normal_links) },
        { "my_normal_links_unique_by_agent", 
          Trace(my_normal_links_unique_by_agent) },
        { "my_evaluators", 
          Trace(my_evaluators) },
        { "my_free_abnormal_engines", 
          Trace(my_free_abnormal_engines) },
        { "my_evaluator_abnormal_engines", 
          Trace(my_evaluator_abnormal_engines) },
        { "my_multiplicity_engines", 
          Trace(my_multiplicity_engines) },
        { "boundary_agents", 
          Trace(boundary_agents) },
        { "coupling_residual_links", 
          Trace(coupling_residual_links) },
        { "coupling_keyer_links_all", 
          Trace(coupling_keyer_links_all) },
        { "my_boundary_links", 
          Trace(my_boundary_links) },
        { "boundary_keyer_links", 
          Trace(boundary_keyer_links) },
        { "my_fixed_keyer_links",
          Trace(my_fixed_keyer_links) },  
        { "parent_links_to_my_normal_agents", 
          Trace(parent_links_to_my_normal_agents) },
        { "parent_residual_links_to_boundary_agents",
          Trace(parent_residual_links_to_boundary_agents) },
        { "normal_and_boundary_links_preorder", 
          Trace(normal_and_boundary_links_preorder) },
        { "free_normal_links_ordered",
          Trace(free_normal_links_ordered) },
        { "current_solve_plinks",
          Trace(current_solve_plinks) }          
    };
    TRACE("=============================================== ")
         (*this)(":\n")(plan_as_strings)("\n");
}


string AndRuleEngine::Plan::GetTrace() const
{
    return algo->GetName() + ".plan" + algo->GetSerialString();
}


void AndRuleEngine::PlanningStageFive( shared_ptr<const Lacing> lacing )
{
    plan.PlanningStageFive(lacing);
}


void AndRuleEngine::StartCSPSolver(const SolutionMap &fixes, const SolutionMap *surrounding_solution)
{    
    // Determine the full set of forces 
    // TODO presumably doesn't need to be the ordered one
    SolutionMap surrounding_and_base_links;
    for( PatternLink link : plan.boundary_keyer_links )
        surrounding_and_base_links[link] = surrounding_solution->at(link);
    for( PatternLink link : plan.my_fixed_keyer_links )
        surrounding_and_base_links[link] = fixes.at(link);
    
    TRACE("Starting solver\n");
    ASSERT( plan.solver_holder );
    plan.solver_holder->Start( surrounding_and_base_links, x_tree_db.get() );
}


SolutionMap AndRuleEngine::GetNextCSPSolution()
{
    TRACE("GetNextCSPSolution()\n");
    SolutionMap csp_solution;
    bool match = plan.solver_holder->GetNextSolution( &csp_solution );        
    if( !match )
        throw NoSolution();

    return csp_solution;
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
    for( PatternLink link : pq->GetAbnormalLinks() )
    {
        TRACE("Comparing block %d\n", i);
 
        // Get x for linked node
        XLink xlink = solution_for_evaluators->at(link);
                                
        try 
        {
            shared_ptr<AndRuleEngine> e = plan.my_evaluator_abnormal_engines.at(link);
            (void)e->Compare( xlink, solution_for_subordinates );
            compare_results.push_back( true );
        }
        catch( const ::Mismatch &e )
        {
            TRACE("Caught ")(e)("\n");
            compare_results.push_back( false );
        }

        i++;
    }
    
    TRACE("Evaluating: ");
    for(bool b : compare_results)
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
        
    auto xsc = dynamic_cast<SubContainer *>( link.GetChildTreePtr().get() );
    
    if( auto xscr = dynamic_cast<SubContainerRange *>(xsc) )
    {
        ASSERT( link );
        ContainerInterface *xci = dynamic_cast<ContainerInterface *>(xscr);
        ASSERT(xci)("Multiplicity x must implement ContainerInterface");    
        
        for( const TreePtrInterface &xe_node : *xci )
        {
            TRACE("Comparing ")(xe_node)("\n");
            XLink xe_link = XLink(xscr->GetParentX(), &xe_node);
            (void)e->Compare( xe_link, solution_for_subordinates );
        }
    }
    else if( auto xssl = dynamic_cast<SubSequence *>(xsc) )
    {
        for( XLink xe_link : xssl->elts )
        {
            TRACE("Comparing ")(xe_link)("\n");
            (void)e->Compare( xe_link, solution_for_subordinates );
        }
    }    
    else
    {
        ASSERTFAIL("unrecognised SubContainer\n");
    }
}


void AndRuleEngine::RegenerationPassAgent( Agent *agent,
                                           SolutionMap &basic_solution,
                                           const SolutionMap &solution_for_subordinates )
{
    // Get a list of the links we must supply to the agent for regeneration
    auto pq = agent->GetPatternQuery();
    TRACE("Trying to regenerate ")(*agent)("\n");    
    TRACEC("Pattern links ")(pq->GetNormalLinks())("\n");    

#ifdef CHECK_EVERYTHING_IS_IN_DOMAIN      
    for( XLink xlink : pq->GetNormalLinks() )    
        ASSERT( x_tree_db->domain.count(xlink) > 0 )(xlink)(" not found in ")(x_tree_db->domain)(" (see issue #202)\n"); // #202 expected to cause this to fail
#endif
    
#ifdef NLQ_TEST
    auto nlq_lambda = agent->TestStartRegenerationQuery( &basic_solution, x_tree_db.get() );
#else    
    auto nlq_lambda = agent->StartRegenerationQuery( &basic_solution, x_tree_db.get() );
#endif
    
    int i=0;
    while(1)
    {
        shared_ptr<SR::DecidedQuery> query = nlq_lambda();
        i++;
                
        TRACE("Try out query, attempt %d (1-based)\n", i);    

        try // in here, only have the stuff that could throw a mismatch
        {
            Tracer::RAIIDisable silencer;   // Shush, I'm trying to debug the NLQs
            SolutionMap solution_for_evaluators;
            
            // Try matching the abnormal links (free and evaluator).
            for( const LocatedLink &link : query->GetAbnormalLinks() )
            {
                ASSERT( link );
                // Actions if evaluator link
                if( plan.my_evaluator_abnormal_engines.count( (PatternLink)link ) )                
                    InsertSolo( solution_for_evaluators, link );                
                
                // Actions if free link
                if( plan.my_free_abnormal_engines.count( (PatternLink)link ) )
                {
                    shared_ptr<AndRuleEngine> e = plan.my_free_abnormal_engines.at( (PatternLink)link );
                    (void)e->Compare( link, &solution_for_subordinates );
                }
            }                    
            
            // Try matching the multiplicity links.
            for( const LocatedLink &link : query->GetMultiplicityLinks() )
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
        catch( const ::Mismatch &e )
        {
            TRACE("Caught ")(e)(", retrying the lambda\n");    
            continue; // deal with exception by iterating the loop 
        }     
                                        
        // Replace needs these keys 
        for( const LocatedLink &link : query->GetAbnormalLinks() )
            InsertSolo( basic_solution, link );                
                
        // If we got here, we're done!
        TRACE("Success after %d tries\n", i);  
        break;
    } 
    agent->ResetNLQConjecture(); // save memory
}      


void AndRuleEngine::RegenerationPass( SolutionMap &basic_solution, const SolutionMap *surrounding_solution )
{
    INDENT("R");
    const SolutionMap solution_for_subordinates = UnionOfSolo( *surrounding_solution, basic_solution );   
    TRACE("---------------- Regeneration ----------------\n");      
    //TRACEC("Subordinate keys ")(keys_for_subordinates)("\n");       
    TRACEC("Basic solution ")(basic_solution)("\n");    

    for( Agent *agent : plan.my_normal_agents )
    {
        RegenerationPassAgent( agent, 
                               basic_solution,
                               solution_for_subordinates );
    }

    TRACE("Regeneration complete\n");
}


void AndRuleEngine::SetXTreeDb( shared_ptr<const XTreeDatabase> x_tree_db_ )
{
    x_tree_db = x_tree_db_;

    // ------------------ Stage five on subordinates ---------------------
    for( auto p : plan.my_free_abnormal_engines )
        p.second->SetXTreeDb(x_tree_db);
    for( auto p : plan.my_evaluator_abnormal_engines )
        p.second->SetXTreeDb(x_tree_db);
    for( auto p : plan.my_multiplicity_engines )
        p.second->SetXTreeDb(x_tree_db);    
}


SolutionMap AndRuleEngine::Compare( XLink base_xlink,
                                    const SolutionMap *surrounding_solution )
{
    INDENT("C");
    ASSERT( base_xlink );            
    TRACE("Compare base ")(base_xlink)("\n");

#ifdef CHECK_EVERYTHING_IS_IN_DOMAIN
    if( !dynamic_cast<StarAgent*>(plan.base_plink.GetChildAgent()) ) // Stars are based at SubContainers which don't go into domain    
        ASSERT( x_tree_db->domain.count(base_xlink) > 0 )(base_xlink)(" not found in ")(x_tree_db->GetOrderings()->domain)(" (see issue #202)\n");
#endif
    
    // Determine my fixed (just root pattern link to base x link)
    SolutionMap my_fixed_assignments = {{plan.base_plink, base_xlink}};
    
    // Start the CSP solver
    StartCSPSolver( my_fixed_assignments, surrounding_solution );
           
    // These are partial solutions, and are mapped against the links
    // into the agents (half-link model). Note: solutions can specify
    // the MMAX node.
    SolutionMap basic_solution; 

    // Create the conjecture object we will use for this compare, and keep iterating
    // though different conjectures trying to find one that allows a match.
    //int i=0;
    while(1)
    {
        // Get a solution from the solver
        basic_solution = GetNextCSPSolution();        

        // Merge my fixes into the solution (but we're not expected to merge
        // in surrounding solution: caller can do that, if required).
        basic_solution = UnionOfSolo( basic_solution, my_fixed_assignments );

        try
        {
            // Is the solution complete? 
            for( auto plink : plan.my_normal_links )
            {            
                ASSERT( basic_solution.count(plink) > 0 )("Cannot find normal link ")(plink)("\nIn ")(basic_solution)("\n");
            }
            RegenerationPass( basic_solution, surrounding_solution );
        }
        catch( const ::Mismatch &e )
        {                
            TRACE("Caught ")(e)(" after recursion, trying next solution\n");
            continue; // Get another solution from the solver
        }
        // We got a match so we're done. 
        TRACE("AndRuleEngine hit\n");
        break; // Success
    }
    
    // By now, we succeeded and basic_solution is the right set of keys
    return basic_solution;
}


const set<Agent *> &AndRuleEngine::GetKeyedAgents() const
{
   // We will key all our normal agents
   return plan.my_normal_agents;
}


const set<PatternLink> AndRuleEngine::GetKeyerPatternLinks() const
{
    set<PatternLink> keyer_plinks_incl_subs;
    //keyer_plinks_incl_subs = plan.coupling_keyer_links_all; 
    
    list<const AndRuleEngine *> subs = GetAndRuleEngines();
    for( const AndRuleEngine *e : subs )
        keyer_plinks_incl_subs = UnionOfSolo( keyer_plinks_incl_subs, e->plan.coupling_keyer_links_all );
        
    return keyer_plinks_incl_subs;
}


set< shared_ptr<SYM::BooleanExpression> > AndRuleEngine::GetExpressions() const
{
    return plan.expressions_for_current_solve;
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


string AndRuleEngine::GetTrace() const
{
    return GetName() + GetSerialString();
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
    TRACE(" parent_residual_links_to_boundary_agents ")( plan.parent_residual_links_to_boundary_agents )("\n");
    TRACE(" boundary_agents ")( plan.boundary_agents )("\n");
    TRACE("Specifying figure nodes for ")(*this)("\n");
    Graph::Figure figure;
    figure.id = GetGraphId();
    figure.title = scr_engine_id.empty() ? GetGraphId() : scr_engine_id+" / "+GetGraphId();
    
    auto agents_lambda = [&](const map< Agent *, set<PatternLink> > &parent_links_to_agents,
                             const set<PatternLink> &keyers,
                             const set<PatternLink> &residuals ) -> list<Graph::Figure::Agent>
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
                    link.details.planned_as = Graph::LINK_DEFAULT;
                gf_agent.incoming_links.push_back( link );
            }            
            TRACEC(*p.first)("\n");            
            figure_agents.push_back(gf_agent);
        }
        return figure_agents;
    };
    
    TRACE("   Interior (my agents/links):\n");    
    figure.interior_agents = agents_lambda( plan.parent_links_to_my_normal_agents,
                                            plan.coupling_keyer_links_all,
                                            plan.coupling_residual_links );
    TRACE("   Exterior (boundary agents/links):\n");    
    figure.exterior_agents = agents_lambda( plan.parent_residual_links_to_boundary_agents,
                                            plan.boundary_keyer_links, // Won't show up as not in p_r_l_t_b_a, but could generate invisible nodes and links?
                                            plan.my_boundary_links );       
        
    auto subordinates_lambda = [&](const map< PatternLink, shared_ptr<AndRuleEngine> > &engines, Graph::LinkPlannedAs incoming_link_planned_as )
    {
        set< shared_ptr<AndRuleEngine> > reached;
        for( auto p : engines )
        {
            ASSERT( reached.count(p.second) == 0 );
            reached.insert( p.second );
            
            Graph::Figure::Agent base_agent;
            base_agent.g = p.second->plan.base_agent;
            Graph::Figure::Link incoming_link;
            incoming_link.details.planned_as = incoming_link_planned_as;
            incoming_link.short_name = p.first.GetShortName();
            base_agent.incoming_links.push_back( incoming_link );
            TRACEC(p.second.get())(" : ( ")(incoming_link.short_name)("->")(base_agent.g->GetGraphId())(" )\n");
            figure.subordinate_engines_and_base_agents.push_back( make_pair(p.second.get(), base_agent) );
        }
    };
    TRACE("   Subordinates (my free abnormals):\n");    
    subordinates_lambda( plan.my_free_abnormal_engines, Graph::LINK_ABDEFAULT );
    TRACE("   Subordinates (my evaluator abnormals):\n");    
    subordinates_lambda( plan.my_evaluator_abnormal_engines, Graph::LINK_EVALUATOR );
    TRACE("   Subordinates (my multiplicity engines):\n");    
    subordinates_lambda( plan.my_multiplicity_engines, Graph::LINK_MULTIPLICITY );
    TRACE("Ready to render\n");
    graph.GenerateGraph(figure);
}

