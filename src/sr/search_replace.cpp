#include "scr_engine.hpp"

#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "node/graphable.hpp"

#include <list>

using namespace SR;


CompareReplace::CompareReplace( TreePtr<Node> cp,
                                TreePtr<Node> rp,
                                bool is_search ) :
    plan( this, is_search )
{
    // If cp and rp are provided, do an instant configuration
    if( cp )
    {
        plan.Configure( cp, rp );
    }
}    


CompareReplace::Plan::Plan( CompareReplace *algo_, bool is_search_ ) :
    algo( algo_ ),
    is_search(is_search_)
{
}


void CompareReplace::Plan::Configure( TreePtr<Node> cp,
                                      TreePtr<Node> rp )
{
    TRACE(algo->GetName());
    // Two-part init for SCREngine: 
    // First, add extra root nodes, categorise, create subordinate 
    // SCREngines and recurse into them
    // This allows the phases of the agents to be determined correctly
    scr_engine = make_shared<SCREngine>(is_search, algo, agent_phases, cp, rp);
    FTRACE(*algo)(" agent phases\n")(agent_phases)("\n");
    // Second, configure the agents and create subordinate AndRuleEngines
    scr_engine->InitPartTwo(agent_phases);
}                                      


void CompareReplace::Configure( TreePtr<Node> cp,
                                TreePtr<Node> rp )
{
    INDENT("P");
    plan.Configure( cp, rp );
}


void CompareReplace::SetMaxReps( int n, bool e ) 
{ 
    SCREngine::SetMaxReps(n, e); 
}


Graphable::Block CompareReplace::GetGraphBlockInfo() const
{
    // We want our name (via GetName()) but SCREngine's layout
    Graphable::Block block = plan.scr_engine->GetGraphBlockInfo();
    block.title = GetName();
    return block;
}


void CompareReplace::SetStopAfter( vector<int> ssa, int d )
{
    plan.scr_engine->SetStopAfter( ssa, d );
}


SCREngine *CompareReplace::GetRootEngine() 
{ 
    return plan.scr_engine.get(); 
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
}


SearchReplace::SearchReplace( TreePtr<Node> sp,
                              TreePtr<Node> rp ) :
    CompareReplace( sp, rp, true )                              
{
}

