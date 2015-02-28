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
    Engine( search )
{
    // If cp and rp are provided, do an instant configuration
    if( cp )
        Configure( cp, rp );
}    


void CompareReplace::GetGraphInfo( vector<string> *labels, 
                                   vector< TreePtr<Node> > *links ) const
{
    // Disambiguate conflict between Transformation and Engine
    Engine::GetGraphInfo( labels, links );
}

    
bool CompareReplace::IsMatch( TreePtr<Node> context,       
                              TreePtr<Node> root )
{
    pcontext = &context;
    ASSERT( pattern );
    bool r = Compare(root, pattern);
    pcontext = NULL;
    return r == true;
}


// Do a search and replace based on patterns stored in our members
void CompareReplace::operator()( TreePtr<Node> c, TreePtr<Node> *proot )
{
    INDENT("");
    TRACE("Enter S&R instance ")(*this);
    ASSERT( pattern )("CompareReplace (or SearchReplace) object was not configured before invocation.\n"
                      "Either call Configure() or supply pattern arguments to constructor.\n"
                      "Thank you for taking the time to read this message.\n");
    
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
        
    (void)RepeatingCompareReplace( proot );   

    pcontext = NULL; // just to avoid us relying on the context outside of a search+replace pass
}


SearchReplace::SearchReplace( TreePtr<Node> sp,
                              TreePtr<Node> rp ) :
    CompareReplace( sp, rp, true )                              
{
}

