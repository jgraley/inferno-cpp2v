#include "scr_engine.hpp"

#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"

#include <list>

using namespace SR;


CompareReplace::CompareReplace( TreePtr<Node> cp,
                                TreePtr<Node> rp,
                                bool is_search_ ) :
    is_search(is_search_)
{
    // If cp and rp are provided, do an instant configuration
    if( cp )
    {
        scr_engine = make_shared<SCREngine>(is_search, this, cp, rp);
    }
}    


void CompareReplace::Configure( TreePtr<Node> cp,
                                TreePtr<Node> rp )
{
    TRACE(GetName());
    INDENT("P");
    scr_engine = make_shared<SCREngine>(is_search, this, cp, rp);
}


void CompareReplace::SetMaxReps( int n, bool e ) 
{ 
    SCREngine::SetMaxReps(n, e); 
}


void CompareReplace::GetGraphInfo( vector<string> *labels, 
                                   vector< TreePtr<Node> > *blocks ) const
{
    // Disambiguate conflict between Transformation and SCREngine
    scr_engine->GetGraphInfo( labels, blocks );
}


void CompareReplace::SetStopAfter( vector<int> ssa, int d )
{
    scr_engine->SetStopAfter( ssa, d );
}


SCREngine *CompareReplace::GetRootEngine() 
{ 
    return scr_engine.get(); 
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
    
    (void)scr_engine->RepeatingCompareReplace( proot, &empty );   

    pcontext = nullptr; // just to avoid us relying on the context outside of a search+replace pass
}


SearchReplace::SearchReplace( TreePtr<Node> sp,
                              TreePtr<Node> rp ) :
    CompareReplace( sp, rp, true )                              
{
}

