#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "helpers/simple_compare.hpp"
#include "slave_agent.hpp"
#include <list>

using namespace SR;


CompareReplace::CompareReplace( TreePtr<Node> cp,
                                TreePtr<Node> rp,
                                bool search ) :
    scr_engine( search )
{
    // If cp and rp are provided, do an instant configuration
    if( cp )
        scr_engine.Configure( this, cp, rp );
}    


void CompareReplace::Configure( TreePtr<Node> cp,
                                TreePtr<Node> rp )
{
    scr_engine.Configure( this, cp, rp );
}


void CompareReplace::GetGraphInfo( vector<string> *labels, 
                                   vector< TreePtr<Node> > *blocks ) const
{
    // Disambiguate conflict between Transformation and SCREngine
    scr_engine.GetGraphInfo( labels, blocks );
}

    
bool CompareReplace::IsMatch( TreePtr<Node> context,       
                              TreePtr<Node> root )
{
    pcontext = &context;
    try
    {
        scr_engine.Compare(&root); // Using &root here in principle allows a PointerIs to operate on the root (yeah, I know right?)
        pcontext = NULL;
        return true;
    }
    catch( ::Mismatch & )
    {
        pcontext = NULL;
        return false;
    }
}


// Do a search and replace based on patterns stored in our members
void CompareReplace::operator()( TreePtr<Node> c, TreePtr<Node> *proot )
{
    INDENT(")");
    TRACE("Enter S&R instance ")(*this);
    
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
    
    Map< Agent *, TreePtr<Node> > empty;
    
    (void)scr_engine.RepeatingCompareReplace( proot, &empty );   

    pcontext = NULL; // just to avoid us relying on the context outside of a search+replace pass
}


SearchReplace::SearchReplace( TreePtr<Node> sp,
                              TreePtr<Node> rp ) :
    CompareReplace( sp, rp, true )                              
{
}

