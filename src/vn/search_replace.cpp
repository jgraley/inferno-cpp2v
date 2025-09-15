#include "scr_engine.hpp"

#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "node/graphable.hpp"
#include "and_rule_engine.hpp"
#include "vn_sequence.hpp"

#include <list>

using namespace VN;

CompareReplace::CompareReplace( bool is_search ) :
    plan( this, is_search )
{
}    


CompareReplace::Plan::Plan( CompareReplace *algo_, bool is_search_ ) :
    algo( algo_ ),
    is_search(is_search_)
{
}


void CompareReplace::Plan::Configure( TreePtr<Node> cp,
                                      TreePtr<Node> rp )
{
    //TRACE(algo->GetName())("\n");
    ASSERT( cp );
    compare_pattern = cp;
    replace_pattern = rp;
}


void CompareReplace::Plan::PlanningStageOne(VNSequence *vn_sequence)
{
    // Two-part init for SCREngine: 
    // First, add extra root nodes, categorise, create subordinate 
    // SCREngines and recurse into them
    // This allows the phases of the agents to be determined correctly
    ASSERT( compare_pattern );
    ASSERT( !is_search );
    scr_engine = make_shared<SCREngine>(vn_sequence, algo, agent_phases, compare_pattern, replace_pattern);

    list<const SCREngine *> scrs = scr_engine->GetSCREngines();
    TRACE("SCR engines for this step: ")(scrs)("\n");
}

    
void CompareReplace::Plan::PlanningStageTwo()
{
    //FTRACE(*algo)(" agent phases\n")(agent_phases)("\n");
    // Second, configure the agents
    scr_engine->PlanningStageTwo(agent_phases);
}                                      


void CompareReplace::Plan::PlanningStageThree()
{
    // Third, create subordinate AndRuleEngines
    scr_engine->PlanningStageThree( {}, {}, {} );
        
    map<const Agent *, StringNoQuotes> agent_plans;
    for( auto p : agent_phases )
    {
        const Agent *agent = p.first;
        agent_plans[agent] = agent->GetPlanAsString();
    }    

    TRACE("=============================================== AGENTS:")
         (agent_plans)("\n");
}                                      


void CompareReplace::Plan::PlanningStageFive( shared_ptr<const Lacing> lacing )
{
    // Third, create subordinate AndRuleEngines
    scr_engine->PlanningStageFive(lacing);
}                                      


string CompareReplace::Plan::GetTrace() const 
{
    return algo->GetName() + ".plan" + algo->GetSerialString();
}

    
void CompareReplace::Configure( TreePtr<Node> cp,
                                TreePtr<Node> rp )
{
    plan.Configure( cp, rp );
}


void CompareReplace::PlanningStageOne(VNSequence *vn_sequence_)
{
    vn_sequence = vn_sequence_;
    plan.PlanningStageOne(vn_sequence);
}


void CompareReplace::PlanningStageTwo()
{
    plan.PlanningStageTwo();
}


void CompareReplace::PlanningStageThree()
{
    plan.PlanningStageThree();
}


void CompareReplace::PlanningStageFive( shared_ptr<const Lacing> lacing )
{
    plan.PlanningStageFive(lacing);
}


void CompareReplace::SetXTreeDb( shared_ptr<XTreeDatabase> x_tree_db_ )
{
    x_tree_db = x_tree_db_;
    plan.scr_engine->SetXTreeDb( x_tree_db );
}


void CompareReplace::Transform()
{
    INDENT(")");
    TRACE("Enter S&R instance ")(*this);

    SolutionMap universal_assignments;    
    (void)plan.scr_engine->RepeatingCompareReplace( x_tree_db->GetMainRootXLink(), 
                                                    &universal_assignments );       
}


Graphable::NodeBlock CompareReplace::GetGraphBlockInfo() const
{ 
    list<ItemBlock> item_blocks;
    auto compare_link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(plan.compare_pattern.get()),
                                                      list<string>{},
                                                      list<string>{""},
                                                      IN_COMPARE_AND_REPLACE,
                                                      &plan.compare_pattern );                                  
    item_blocks.push_back( { "search/compare", 
                            "",
                            true,
                            { compare_link } } );

    if( plan.replace_pattern && plan.replace_pattern != plan.compare_pattern )
    {
        auto replace_link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(plan.replace_pattern.get()),
                                                     list<string>{},
                                                     list<string>{""},
                                                     IN_REPLACE_ONLY,
                                                     &plan.replace_pattern );                                  
    
        item_blocks.push_back( { "replace", 
                                "",
                                true,
                                { replace_link } } );
    }
    return { false, GetName(), "", "", CONTROL, nullptr, item_blocks };
}


string CompareReplace::GetGraphId() const
{
    return "CR"+GetSerialString();
}


void CompareReplace::GenerateGraphRegions( Graph &graph ) const
{
    plan.scr_engine->GenerateGraphRegions(graph);
}


void CompareReplace::SetStopAfter( vector<int> ssa, int d )
{
    plan.scr_engine->SetStopAfter( ssa, d );
}


SCREngine *CompareReplace::GetRootEngine() 
{ 
    return plan.scr_engine.get(); 
}


TreePtr<Node> CompareReplace::GetSearchComparePattern()
{
    return plan.compare_pattern;
}


TreePtr<Node> CompareReplace::GetEmbeddedReplacePattern()
{
    return plan.replace_pattern;
}


string CompareReplace::GetTrace() const
{
    string s = Traceable::GetName();
    return s;
}


SearchReplace::SearchReplace() :
    CompareReplace( true )                              
{
}




