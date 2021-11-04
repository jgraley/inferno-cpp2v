#include "scr_engine.hpp"

#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "node/graphable.hpp"
#include "and_rule_engine.hpp"

#include <list>

using namespace SR;

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


void CompareReplace::Plan::PlanningStageOne()
{
    // Two-part init for SCREngine: 
    // First, add extra root nodes, categorise, create subordinate 
    // SCREngines and recurse into them
    // This allows the phases of the agents to be determined correctly
    ASSERT( compare_pattern );
    ASSERT( !is_search );
    scr_engine = make_shared<SCREngine>(algo, agent_phases, compare_pattern, replace_pattern);

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
    scr_engine->PlanningStageThree( {} );
    
    list<const AndRuleEngine *> ares = scr_engine->GetAndRuleEngines();
	TRACE("And-rule engines for this step: ")(ares)("\n");
}                                      


void CompareReplace::Configure( TreePtr<Node> cp,
                                TreePtr<Node> rp )
{
    plan.Configure( cp, rp );
}


void CompareReplace::PlanningStageOne()
{
    plan.PlanningStageOne();
}


void CompareReplace::PlanningStageTwo()
{
    plan.PlanningStageTwo();
}


void CompareReplace::PlanningStageThree()
{
    plan.PlanningStageThree();
}


// Do a search and replace based on patterns stored in our members
void CompareReplace::operator()( TreePtr<Node> c, TreePtr<Node> *proot )
{
    INDENT(")");
    TRACE("Enter S&R instance ")(*this);
    ASSERT(c==*proot);
    
    // If the initial root and context are the same node, then arrange for the context
    // to follow the root node as we modify it (in SingleSearchReplace()). This ensures
    // new declarations can be found in slave searches. 
    //
    // TODO but does not work for sub-slaves, because the first level slave's proot
    // is not the same as pcontext. When slave finishes a singe CR, only the locally-created
    // *proot is updated, not the top level *proot or *pcontext, so the updates do not appear 
    // in the context until the first level slave completes, the local *proot is copied over
    // the TL *proot (and hence *pcontext) and the mechanism described here kicks in
    //  
    // We could get the
    // same effect by taking the context as a reference, but leave it like this for now.
    // If *proot is under context, then we're OK as long as proot points to the actual
    // tree node - then the walk at context will follow the new *proot pointer and get
    // into the new subtree.
    if( c == *proot )
	    pcontext = proot;
    else
	    pcontext = &c;
    
    CouplingKeysMap empty;
    
    (void)plan.scr_engine->RepeatingCompareReplace( proot, &empty );   

    pcontext = nullptr; // just to avoid us relying on the context outside of a search+replace pass
    dirty_grass.clear(); // save memory
}


Graphable::Block CompareReplace::GetGraphBlockInfo() const
{ 
    list<SubBlock> sub_blocks;
    auto compare_link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(plan.compare_pattern.get()),
                                                      list<string>{},
                                                      list<string>{""},
                                                      IN_COMPARE_AND_REPLACE,
                                                      &plan.compare_pattern );                                  
    sub_blocks.push_back( { "search/compare", 
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
    
        sub_blocks.push_back( { "replace", 
                                "",
                                true,
                                { replace_link } } );
    }
    return { false, GetName(), "", "", CONTROL, nullptr, sub_blocks };
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


TreePtr<Node> CompareReplace::GetReplacePattern()
{
    return plan.replace_pattern;
}


SearchReplace::SearchReplace() :
    CompareReplace( true )                              
{
}


