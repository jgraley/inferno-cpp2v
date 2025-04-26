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
    WalkVisible( visible_compare_plinks, nullptr, origin_plink, Agent::COMPARE_PATH );
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
            my_plinks.insert( plink );

    // Need the replace plinks in the same order that GenReplaceLayout() walks the tree
    for( PatternLink plink : visible_replace_plinks_postorder )
        if( enclosing_plinks.count(plink) == 0 ) // exclude by plink
        {
            my_replace_plinks_postorder.push_back( plink );
            if( dynamic_cast<RequiresSubordinateSCREngine *>(plink.GetChildAgent()) )
                my_subordinate_plinks_postorder.push_back(plink);
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
    visible.insert( plink );    
    
    // Gee, I sure hope recovers children in the same order as GenReplaceLayoutImpl()    
    list<PatternLink> visible_child_plinks = plink.GetChildAgent()->GetVisibleChildren(path); 
    
    for( PatternLink visible_child_plink : visible_child_plinks )
        WalkVisible( visible, visible_postorder, visible_child_plink, path );    

    if( visible_postorder )
        visible_postorder->push_back( plink );
}


void SCREngine::Plan::CreateMyEngines( CompareReplace::AgentPhases &in_progress_agent_phases )
{
    // Determine which agents our embeddeds should not configure
    set<PatternLink> surrounding_plinks = UnionOf( enclosing_plinks, my_plinks ); 
            
    for( PatternLink plink : my_subordinate_plinks_postorder )
    {
        auto ae = dynamic_cast<RequiresSubordinateSCREngine *>(plink.GetChildAgent());
        ASSERT( ae );    
        my_engines[ae] = make_shared<SCREngine>( vn_sequence,
                                                 root_engine, 
                                                 in_progress_agent_phases,
                                                 ae->GetSearchPattern(),
                                                 ae->GetReplacePattern(),
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
    for( pair< RequiresSubordinateSCREngine *, shared_ptr<SCREngine> > p : my_engines )
        p.second->PlanningStageTwo(final_agent_phases);                                      

    // Configure agents on the way out
    ConfigureAgents();
}


void SCREngine::Plan::ConfigureAgents()
{
    // Give agents pointers to here and our coupling keys
    for( Agent *agent : my_agents )
    {        
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
    for( pair< RequiresSubordinateSCREngine *, shared_ptr<SCREngine> > p : my_engines )
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

    for( PatternLink plink : my_replace_plinks_postorder )
    {
        Agent *agent = plink.GetChildAgent();
        if( agent->ReplaceKeyerQuery(plink, all_keyer_plinks) )
        {
            InsertSolo( all_keyer_plinks, plink );
            agent->ConfigureCoupling( algo, plink, {} );
        }
    }
}


void SCREngine::Plan::PlanningStageFive( shared_ptr<const Lacing> lacing )
{    
    TRACE("Planning stage five\n");

    and_rule_engine->PlanningStageFive(lacing);
    
    for( pair< RequiresSubordinateSCREngine *, shared_ptr<SCREngine> > p : my_engines )
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
        { "my_replace_plinks_postorder", 
          Trace(my_replace_plinks_postorder) },
        { "my_subordinate_plinks_postorder", 
          Trace(my_subordinate_plinks_postorder) }
    };
    TRACE("=============================================== ")
         (*this)(":\n")(plan_as_strings)("\n");
}


string SCREngine::Plan::GetTrace() const 
{
    return algo->GetName() + ".plan" + algo->GetSerialString();
}

    
void SCREngine::UpdateEmbeddedActionRequests( TreePtr<Node> through_subtree, TreePtr<Node> new_subtree ) const 
{
    INDENT("F");
    
    // We need to fix up any remaining action requests at the same level as the one
    // that just ran if they have the same through node as the one we just changed.
    for( auto &p : origins_for_embedded ) // ref important - we're modifying!
    {
        if( p.second == through_subtree )
        {
            TRACE("Fixup for ")(*(Agent *)p.first)(": ")(through_subtree)(" becomes ")(new_subtree)("\n");
            p.second = new_subtree;
        }
    }
    
    // Master SCREngines may also have pending action requests with matching through node
    if( plan.enclosing_engine ) 
        plan.enclosing_engine->UpdateEmbeddedActionRequests( through_subtree, new_subtree );
}


void SCREngine::RunEmbedded( PatternLink plink_to_embedded, XLink origin_xlink )
{         
    INDENT("E");
    auto embedded_agent = dynamic_cast<RequiresSubordinateSCREngine *>(plink_to_embedded.GetChildAgent());
    ASSERT( embedded_agent );    
    shared_ptr<SCREngine> embedded_engine = plan.my_engines.at(embedded_agent);
    ASSERT( embedded_engine );
   
    TRACE("Going to run embedded on ")(*embedded_engine)
         (" agent ")(embedded_agent)
         (" and origins_for_embedded are\n")(origins_for_embedded)("\n");
   
    // Recall the origin of the subtree under through (after replace)
    ASSERT(origins_for_embedded.count(embedded_agent) == 1)
          ("No call to SCREngine::MarkOriginForEmbedded() for ")(embedded_agent);
    TreePtr<Node> embedded_origin = origins_for_embedded.at(embedded_agent);
    EraseSolo( origins_for_embedded, embedded_agent); // not needed any more
    ASSERT( embedded_origin );
    
    // Obtain a pointer to the though link that will be origin for the 
    // embedded engine. 
    NodeTable::Row nn = x_tree_db->GetNodeRow(embedded_origin);
    XLink embedded_origin_xlink = SoloElementOf(nn.incoming_xlinks);

    // Run the embedded's engine on this subtree and overwrite through ptr via p_through_x
    int hits = embedded_engine->RepeatingCompareReplace( embedded_origin_xlink, &replace_solution );
    (void)hits;
    
    UpdateEmbeddedActionRequests( embedded_origin, embedded_origin_xlink.GetChildTreePtr() );
}


void SCREngine::Replace( XLink origin_xlink )
{
    INDENT("R");

    // Get an expression that evaluates to the new X tree
    Agent::ReplaceKit replace_kit { x_tree_db.get() };
    Agent::ReplacePatchPtr source_expr = plan.origin_agent->GenReplaceLayout(replace_kit, plan.origin_plink);
        
    // Request to update the tree
    plan.vn_sequence->UpdateUsingLayout( origin_xlink, move(source_expr) );  
    
    TRACE("Replace done\n");
}


void SCREngine::SingleCompareReplace( XLink origin_xlink,
                                      const SolutionMap *enclosing_solution ) 
{
    INDENT(">");

    TRACE("Begin search\n");
    // Note: comparing doesn't require double pointer any more, but
    // replace does so it can change the origin node. Throws on mismatch.
    const SolutionMap &cs = plan.and_rule_engine->Compare( origin_xlink, 
                                                           enclosing_solution );
    TRACE("Search got a match (otherwise throws)\n");
           
    // Replace will need the compare keys unioned with the enclosing keys
    SolutionMap rs = UnionOfSolo( *enclosing_solution, cs );    
    origins_for_embedded.clear();
    replace_solution = move(rs);
    replace_solution_available = true;

    // Now replace according to the couplings
    Replace(origin_xlink);

    // Now run the embedded SCR engines (LATER model)
    for( PatternLink plink_to_embedded : plan.my_subordinate_plinks_postorder )
    {
        TRACE("Running embedded ")(plink_to_embedded)(" base xlink=")(origin_xlink)("\n");
        RunEmbedded(plink_to_embedded, origin_xlink);       
    }
    TRACE("Embedded SCRs done\n");
    
    replace_solution_available = false;
    replace_solution.clear();
          
    // Clear out anything cached in agents and update the x_tree_db 
    // now that replace is done
    for( Agent *a : plan.my_agents )
        a->Reset();
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
            for( const pair< RequiresSubordinateSCREngine * const, shared_ptr<SCREngine> > &p : plan.my_engines )
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
    
    for( pair< RequiresSubordinateSCREngine *, shared_ptr<SCREngine> > p : plan.my_engines )
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


void SCREngine::MarkOriginForEmbedded( const RequiresSubordinateSCREngine *embedded_agent,
                                       TreePtr<Node> embedded_origin ) const
{

    InsertSolo( origins_for_embedded, make_pair( embedded_agent, embedded_origin ) );
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
        return replace_solution.at(plink);
    else
        return XLink();
}


bool SCREngine::IsKeyedByAndRuleEngine( Agent *agent ) const
{
    ASSERT( plan.and_rule_engine );
    return plan.and_rule_engine->GetKeyedAgents().count( agent );
}


bool SCREngine::IsKeyed( PatternLink plink ) const
{
    return plan.all_keyer_plinks.count(plink)==1;
}


bool SCREngine::IsKeyed( Agent *agent ) const
{
    for( PatternLink keyer_plink : plan.all_keyer_plinks )
    {
        Agent *keyer_agent = keyer_plink.GetChildAgent();
        if( keyer_agent == agent )
            return true; 
    }
    return false;
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
