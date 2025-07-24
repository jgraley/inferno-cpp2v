#include "scr_engine.hpp"
#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "agents/standard_agent.hpp"
#include "agents/delta_agent.hpp"
#include "agents/depth_agent.hpp"
#include "common/common.hpp"
#include "agents/agent.hpp"
#include "and_rule_engine.hpp"
#include "link.hpp"
#include "vn_sequence.hpp"

#include <list>

//#define TRACE_KEEP_ALIVES

using namespace SR;
using namespace std;

int SCREngine::repetitions;
bool SCREngine::rep_error;


// The enclosing_plinks argument is a set of plinks to agents that we should not
// configure because they were already configured, and enclosing engines take 
// higher priority for configuration (so when an agent is reached from multiple
// engines, it's the one closest to root that "owns" it).
SCREngine::SCREngine( VNSequence *vn_sequence,
                      const CompareReplace *root_engine,
                      CompareReplace::AgentPhases &in_progress_agent_phases,
                      TreePtr<Node> cp,
                      TreePtr<Node> rp,
                      const set<PatternLink> &enclosing_plinks,
                      const SCREngine *enclosing_engine ) :
    plan(this, vn_sequence, root_engine, in_progress_agent_phases, cp, rp, enclosing_plinks, enclosing_engine),
    depth( 0 )
{
}

    
SCREngine::Plan::Plan( SCREngine *algo_,
                       VNSequence *vn_sequence_,
                       const CompareReplace *root_engine_,
                       CompareReplace::AgentPhases &in_progress_agent_phases,
                       TreePtr<Node> cp,
                       TreePtr<Node> rp,
                       const set<PatternLink> &enclosing_plinks_,
                       const SCREngine *enclosing_engine_ ) : // Note: Is planning stage one
    algo( algo_ ),
    vn_sequence( vn_sequence_ ),
    enclosing_engine( nullptr ),
    enclosing_plinks( enclosing_plinks_ )
{
    INDENT("}");
    TRACE("Planning stage one\n");
    ASSERT(!enclosing_engine)("Calling configure on already-configured");
    root_engine = root_engine_;
    enclosing_engine = enclosing_engine_;
    
    ASSERT( cp )("Compare pattern must always be provided\n");
    ASSERT( cp==rp ); // Should have managed to reduce to a single pattern by now
    pattern_origin = cp; 
    origin_agent = Agent::AsAgent(pattern_origin);
    // For closure under full arrowhead model, we need a link to root
    origin_plink = PatternLink::CreateDistinct( pattern_origin );   
            
    CategoriseAgents( enclosing_plinks, in_progress_agent_phases );    

    // This recurses SCR engine planning stage 1
    CreateMyEngines( in_progress_agent_phases );    
}

    
void SCREngine::Plan::CategoriseAgents( const set<PatternLink> &enclosing_plinks, 
                                        CompareReplace::AgentPhases &in_progress_agent_phases )
{
    // Walkers for compare and replace patterns that do not recurse beyond embeddeds (except via "through")
    // So that the compare and replace subtrees of embeddeds are "obsucured" and not visible. Determine 
    // compare and replace sets separately.
    set<PatternLink> visible_compare_plinks, visible_replace_plinks;
    list<PatternLink> visible_replace_plinks_postorder;
    
    // Common stem and compare path of deltas
    WalkVisible( visible_compare_plinks, nullptr, origin_plink, Agent::COMPARE_PATH );

    // Common stem and replace path of deltas
    WalkVisible( visible_replace_plinks, &visible_replace_plinks_postorder, origin_plink, Agent::REPLACE_PATH );
    
    // Determine all the agents we can see (can only see though embedded "through", 
    // not into the embeddeds's pattern)
    set<PatternLink> visible_plinks = UnionOf( visible_compare_plinks, visible_replace_plinks );
    
    for( PatternLink plink : visible_plinks )
    {
        unsigned int phase = (unsigned int)in_progress_agent_phases[plink.GetChildAgent()];
        if( visible_compare_plinks.count(plink) == 0 )
            phase |= Agent::IN_REPLACE_ONLY;
        else if( visible_replace_plinks.count(plink) == 0 )
            phase |= Agent::IN_COMPARE_ONLY;
        else
            phase |= Agent::IN_COMPARE_AND_REPLACE;
        in_progress_agent_phases[plink.GetChildAgent()] = (Agent::Phase)phase;
    }
        
    enclosing_agents.clear();
    for( PatternLink plink : enclosing_plinks )
        enclosing_agents.insert( plink.GetChildAgent() );

    // Determine which ones really belong to us (some might be visible in 
    // an enclosing pattern, in which case it should be in enclosing_plinks.)      
    my_plinks.clear();
    //my_plinks = DifferenceOf( visible_plinks, enclosing_plinks );
    for( PatternLink plink : visible_plinks )
        if( enclosing_agents.count( plink.GetChildAgent() ) == 0 ) // exclude by agent
            InsertSolo( my_plinks, plink );

    // Need the replace plinks in the same order that GenReplaceLayout() walks the tree
    for( PatternLink plink : visible_replace_plinks_postorder )
    {		
        if( enclosing_plinks.count(plink) == 0 )
        {          
            Agent *agent = plink.GetChildAgent();
            if( agent->GetEmbeddedSearchPattern() ||
                agent->GetEmbeddedReplacePattern() )
                my_embedded_plinks_postorder.push_back(plink);        
                
            if( visible_compare_plinks.count(plink) == 0 )
				my_replace_only_plinks_postorder.push_back( plink ); // be exclusive of shared contexts      
		}
	}
	
    my_agents.clear();
    for( PatternLink plink : my_plinks )
        my_agents.insert( plink.GetChildAgent() );

    // Determine who our embeddeds are
    my_overlay_starter_engines.clear();
    for( Agent *a : my_agents )
    {
        if( auto ao = dynamic_cast<StartsOverlay *>(a) )
            InsertSolo(my_overlay_starter_engines, ao);
    }
}


void SCREngine::Plan::WalkVisible( set<PatternLink> &visible,
                                   list<PatternLink> *visible_postorder, // optional
                                   PatternLink plink, 
                                   Agent::Path path ) const
{
	// Can insert more than once TODO better to terminate the walk on repeat visits 
	// and use InsertSlol()
    visible.insert( plink );    
    
    // Gee, I sure hope recovers children in the same order as GenReplaceLayoutImpl()    
    // We will not recurse into the "wrong" branch of a Delta (wrt path) or
    // the compare or replace branches of an embedded engine.
    list<PatternLink> visible_child_plinks = plink.GetChildAgent()->GetVisibleChildren(path); 
    
    for( PatternLink visible_child_plink : visible_child_plinks )
        WalkVisible( visible, visible_postorder, visible_child_plink, path ); // recurse here

    if( visible_postorder )
        visible_postorder->push_back( plink ); // Push to list here, hence post-order
}


void SCREngine::Plan::CreateMyEngines( CompareReplace::AgentPhases &in_progress_agent_phases )
{
    // Determine which agents our embeddeds should not configure
    set<PatternLink> surrounding_plinks = UnionOf( enclosing_plinks, my_plinks ); 
            
    for( PatternLink plink : my_embedded_plinks_postorder )
    {
		Agent *agent = plink.GetChildAgent();   
        my_engines[agent] = make_shared<SCREngine>( vn_sequence,
                                                    root_engine, 
                                                    in_progress_agent_phases,
                                                    agent->GetEmbeddedSearchPattern(),
                                                    agent->GetEmbeddedReplacePattern(),
                                                    surrounding_plinks, 
                                                    algo );       
    }        
}


void SCREngine::Plan::PlanningStageTwo(const CompareReplace::AgentPhases &final_agent_phases_)
{
    INDENT("}");
    TRACE("Planning stage two\n");
    final_agent_phases = final_agent_phases_;
    
    // Recurse into subordinate SCREngines
    for( pair< Agent *, shared_ptr<SCREngine> > p : my_engines )
        p.second->PlanningStageTwo(final_agent_phases);                                      

    // Configure agents on the way out
    ConfigureAgents();
}


void SCREngine::Plan::ConfigureAgents()
{
    // Give agents pointers to here and our coupling keys
    for( Agent *agent : my_agents )
    {  
		TRACE("Call SCRConfigure() on agent %p: ", agent)(agent)("\n");  
        agent->SCRConfigure( algo,
                             final_agent_phases.at(agent) );                                                 
    }
}


void SCREngine::Plan::PlanningStageThree(set<PatternLink> enclosing_keyers)
{    
    INDENT("}");
    // Stage three mirrors the sequence of events taken at run time i.e.
    // COMPARE, REPLACE, RECURSE, RECURSE (this is LATER embedded S/R)
    TRACE("Planning stage three\n");
 
    all_keyer_plinks = enclosing_keyers;
    
    // COMPARE
    PlanCompare();
    
    // REPLACE
    PlanReplace();
    
    // RECURSE RECURSE
    // Recurse into subordinate SCREngines
    for( pair< Agent *, shared_ptr<SCREngine> > p : my_engines )
        p.second->PlanningStageThree(all_keyer_plinks);    
} 


void SCREngine::Plan::PlanCompare()
{
    // All agents this AndRuleEngine see must have been configured 
    and_rule_engine = make_shared<AndRuleEngine>(origin_plink, enclosing_plinks, all_keyer_plinks);
    
    all_keyer_plinks = UnionOfSolo( all_keyer_plinks, 
                                    and_rule_engine->GetKeyerPatternLinks() );
}


void SCREngine::Plan::PlanReplace()
{
    // Plan the keyers for couplings 
    for( StartsOverlay *ao : my_overlay_starter_engines )
        ao->StartPlanOverlay();        

    for( PatternLink plink : my_replace_only_plinks_postorder ) // common stem and Delta->replace
    {
        Agent *agent = plink.GetChildAgent();
		
		// Only want to be here when not already keyed, i.e. exclusively replace context.
		ASSERT( all_keyer_plinks.count(plink)==0 )(plink);
			
		bool need_replace_key = !IsAgentKeyed(agent); // not keyed by any incoming plink
		
		if( need_replace_key )
		{
            InsertSolo( all_keyer_plinks, plink );
            TRACE("Call ConfigureCoupling() on agent %p: ", agent)(agent)(" plink: ")(plink)("\n");  
            agent->ConfigureCoupling( algo, plink, {} );
        }
    }
}


bool SCREngine::Plan::IsAgentKeyed( Agent *agent ) const
{
    for( PatternLink keyer_plink : all_keyer_plinks )
    {
        Agent *keyer_agent = keyer_plink.GetChildAgent();
        if( keyer_agent == agent )
            return true; 
    }
    return false;
}


void SCREngine::Plan::PlanningStageFive( shared_ptr<const Lacing> lacing )
{    
    TRACE("Planning stage five\n");

    and_rule_engine->PlanningStageFive(lacing);
    
    for( pair< Agent *, shared_ptr<SCREngine> > p : my_engines )
        p.second->PlanningStageFive(lacing);    
            
    Dump();
} 


void SCREngine::Plan::Dump()
{
    list<KeyValuePair> plan_as_strings = 
    {
        { "root_engine", 
          Trace(root_engine) },
        { "pattern_origin", 
          Trace(pattern_origin) },
        { "origin_plink", 
          Trace(origin_plink) },
        { "origin_agent", 
          Trace(origin_agent) },
        { "enclosing_engine", 
          Trace(enclosing_engine) },
        { "enclosing_plinks", 
          Trace(enclosing_plinks) },
        { "enclosing_agents", 
          Trace(enclosing_agents) },
        { "my_plinks", 
          Trace(my_plinks) },
        { "my_agents", 
          Trace(my_agents) },
        { "all_keyer_plinks", 
          Trace(all_keyer_plinks) },
        { "my_overlay_starter_engines", 
          Trace(my_overlay_starter_engines) },
        { "my_engines", 
          Trace(my_engines) },
        { "and_rule_engine", 
          Trace(and_rule_engine) },
        { "final_agent_phases", 
          Trace(final_agent_phases) },
        { "my_replace_only_plinks_postorder", 
          Trace(my_replace_only_plinks_postorder) },
        { "my_embedded_plinks_postorder", 
          Trace(my_embedded_plinks_postorder) }
    };
    TRACE("=============================================== ")
         (*this)(":\n")(plan_as_strings)("\n");
}


string SCREngine::Plan::GetTrace() const 
{
    return algo->GetName() + ".plan" + algo->GetSerialString();
}


void SCREngine::RunEmbedded( PatternLink plink_to_embedded )
{         
    INDENT("E");
    auto embedded_agent = plink_to_embedded.GetChildAgent();
    shared_ptr<SCREngine> embedded_engine = plan.my_engines.at(embedded_agent);
    ASSERT( embedded_engine );
    TRACE("Going to run embedded on ")(*embedded_engine)
         (" agent ")(embedded_agent)("\n");
              
	// Discover new origin for embedded by consulting the replace assignments
	XLink embedded_origin_xlink = replace_solution.at(plink_to_embedded);
    TRACE("Running embedded ")(plink_to_embedded)(" with origin=")(embedded_origin_xlink)("\n");
   
    // Run the embedded's engine on this subtree and overwrite through ptr via p_through_x    
    int hits = embedded_engine->RepeatingCompareReplace( embedded_origin_xlink, &replace_solution );
    (void)hits;    
}


ReplaceAssignments SCREngine::Replace( XLink origin_xlink )
{
    INDENT("R");

    // Get an expression that evaluates to the new X tree
    Agent::ReplaceKit replace_kit { x_tree_db.get() };
    Agent::ReplacePatchPtr source_expr = plan.origin_agent->GenReplaceLayout(replace_kit, plan.origin_plink);
        
    // Request to update the tree
    ReplaceAssignments assignments = plan.vn_sequence->UpdateUsingLayout( origin_xlink, move(source_expr) );  
    
    TRACE("Replace done\n");
    
    return assignments;
}


void SCREngine::SingleCompareReplace( XLink origin_xlink,
                                      const SolutionMap *enclosing_solution ) 
{
    INDENT(">");
    
	// XLink memory safety: we need to keep some nodes alive - for example
	// nodes from regeneration which are not in any tree but are created
	// during search/matching. They are needed by replace and embeddeds
	// and have the same lifetime as the replace solution.
    set<TreePtr<Node>> *keep_alive_nodes = new set<TreePtr<Node>>(); // TODO won't need new once XLinks sorted

    TRACE("Begin search\n");
    // Note: comparing doesn't require double pointer any more, but
    // replace does so it can change the origin node. Throws on mismatch.
    SolutionMap cs = plan.and_rule_engine->Compare( origin_xlink, 
                                                    enclosing_solution,
                                                    keep_alive_nodes );
    TRACE("Search got a match (otherwise throws)\n");
           
    // Replace will need the compare keys unioned with the enclosing keys
    replace_solution = UnionOfSolo( *enclosing_solution, cs );    
    replace_solution_available = true;

    // Now replace according to the couplings
    ReplaceAssignments replace_assignments = Replace(origin_xlink);

    // replace_assignments overrides
	//replace_solution = UnionOf( replace_solution, replace_assignments );    	
	for( auto p : replace_assignments )
		replace_solution[p.first] = p.second;

	//FTRACE("replace_assignments: ")(replace_assignments)("\n");
	//FTRACE("enclosing_solution: ")(*enclosing_solution)("\n");

    // Now run the embedded SCR engines (LATER model)
    for( PatternLink plink_to_embedded : plan.my_embedded_plinks_postorder )	    
        RunEmbedded(plink_to_embedded);       
		
    TRACE("Embedded SCRs done\n");
    
    replace_solution_available = false;
    replace_solution.clear();
    replace_assignments.clear();
    cs.clear();
          
    // Clear out anything cached in agents and update the x_tree_db 
    // now that replace is done
    for( Agent *a : plan.my_agents )
        a->Reset();
        
#ifdef TRACE_KEEP_ALIVES
    set<pair<string, string>> ss;
    for( TreePtr<Node> x : *keep_alive_nodes )
    {
		pair<string, string> s;
		s.first = Trace(x);
		if( auto xscr = dynamic_cast<SubContainerRange *>(x.get()) )
		{
			ContainerInterface *xci = dynamic_cast<ContainerInterface *>(xscr);
			ASSERT(xci)("Multiplicity x must implement ContainerInterface");    
			
			for( const TreePtrInterface &xe_node : *xci )
			{
				s.second += ":" + Trace(xe_node);
			}
		}
		ss.insert(s);
	}
	FTRACE("delete keep_alive_nodes: ")(ss)("\n");
#endif
    delete keep_alive_nodes;
#ifdef TRACE_KEEP_ALIVES
	FTRACE("done deleting keep_alive_nodes\n");
#endif
}


// Perform search and replace on supplied program based
// on supplied patterns and couplings. Does search and replace
// operations repeatedly until there are no more matches. Returns how
// many hits we got.
int SCREngine::RepeatingCompareReplace( XLink origin_xlink,
                                        const SolutionMap *enclosing_solution )
{
    INDENT("}");
    TRACE("Begin RCR\n");
        
    ASSERT( plan.pattern_origin )("SCREngine object was not configured before invocation.\n"
                                "Either call Configure() or supply pattern arguments to constructor.\n"
                                "Thank you for taking the time to read this message.\n");
    
    
    if( depth < stop_after.size() && stop_after[depth]==0 )
    {
        TRACE("Stopping as requested before trying\n");
        return 0;
    }    
    
    for(int i=0; i<repetitions; i++) 
    {
        bool stop = depth < stop_after.size() && stop_after[depth]==i+1;
        if( stop )
            for( const pair< Agent * const, shared_ptr<SCREngine> > &p : plan.my_engines )
                p.second->SetStopAfter(stop_after, depth+1); // and propagate the remaining ones
        try
        {
            // Cannonicalise could change origin
            SingleCompareReplace( origin_xlink, enclosing_solution );
        }
        catch( const ::Mismatch &e )
        {
            TRACE("Caught ")(e)("; stopping\n");
            if( depth < stop_after.size() )
                ASSERT(stop_after[depth]<i)("Stop requested after hit that doesn't happen, there are only %d", i);            
            return i+1; // when the compare fails, we're done
        }
        if( stop )
        {
            TRACE("Stopping as requested after hit %d\n", stop_after[depth]);
            return i+1;
        }           
    }
    
    TRACE("Stop after ")(stop_after)(" depth=")(depth)("\n");
    TRACE("Lazy the limit of %d reps\n", repetitions); 
    ASSERT(!rep_error)
          ("Still getting matches after %d repetitions, may be repeating forever.\n"
           "Try using -rn%d to suppress this error\n", repetitions, repetitions);
    
    return repetitions;
}                                


void SCREngine::SetStopAfter( vector<int> ssa, int d )
{
    stop_after = ssa;
    depth = d;
}


void SCREngine::SetMaxReps( int n, bool e ) 
{ 
    repetitions = n; 
    rep_error = e; 
}


void SCREngine::SetXTreeDb( shared_ptr<XTreeDatabase> x_tree_db_ )
{
    x_tree_db = x_tree_db_;

    plan.and_rule_engine->SetXTreeDb(x_tree_db);
    
    for( pair< Agent *, shared_ptr<SCREngine> > p : plan.my_engines )
        p.second->SetXTreeDb(x_tree_db);            
}


set< shared_ptr<SYM::BooleanExpression> > SCREngine::GetExpressions() const
{
    set< shared_ptr<SYM::BooleanExpression> > expressions;
    for( const AndRuleEngine *are : GetAndRuleEngines() )
    {
        expressions = UnionOfSolo( expressions, are->GetExpressions() );
    }
    return expressions;
}


list<const AndRuleEngine *> SCREngine::GetAndRuleEngines() const
{
    list<const AndRuleEngine *> engines;
    engines = plan.and_rule_engine->GetAndRuleEngines();
    for( auto p : plan.my_engines )
    {
        //engines.push_back(nullptr);
        engines = engines + p.second->GetAndRuleEngines();
    }
    return engines;
}


list<const SCREngine *> SCREngine::GetSCREngines() const
{
    list<const SCREngine *> engines;
    engines.push_back( this );
    for( auto p : plan.my_engines )
        engines = engines + p.second->GetSCREngines();
    return engines;
}


void SCREngine::GenerateGraphRegions( Graph &graph ) const
{
    plan.and_rule_engine->GenerateGraphRegions(graph, GetGraphId());
    for( auto p : plan.my_engines )
        p.second->GenerateGraphRegions(graph);    
}



void SCREngine::SetReplaceKey( LocatedLink keyer_link ) const
{
    ASSERT( replace_solution_available );
    InsertSolo( replace_solution, keyer_link );
}


XLink SCREngine::GetReplaceKey( PatternLink plink ) const
{
    ASSERT( plink );
    ASSERT( replace_solution_available );
    if( replace_solution.count(plink) == 1 )
    {
		XLink xlink = replace_solution.at(plink);
		//FTRACE("Extracted xlink: ")(xlink)("\n");
        return xlink;
	}
    else
        return XLink();
}


bool SCREngine::IsKeyedByAndRuleEngine( Agent *agent ) const
{
    ASSERT( plan.and_rule_engine );
    return plan.and_rule_engine->GetKeyedAgents().count( agent );
}


const CompareReplace * SCREngine::GetOverallMaster() const
{
    return plan.root_engine;
}


string SCREngine::GetTrace() const
{
    string s = Traceable::GetName() + GetSerialString();
    return s;
}


string SCREngine::GetGraphId() const
{
    return "SCR"+GetSerialString();
}
