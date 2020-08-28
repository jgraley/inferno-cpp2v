#include "pointer_is_agent.hpp"
#include "search_replace.hpp"
#include "agent.hpp"

using namespace SR;
#define POINTER_IS_VIA_PARENT
//#define SPIKE
//#define SPIKE_MISMATCH

#ifdef SPIKE
// to get CPPTree::Integer for the spike only
#include "../tree/cpptree.hpp" 
#endif

shared_ptr<PatternQuery> PointerIsAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
	pq->RegisterNormalLink( GetPointer() );
    return pq;
}


void PointerIsAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                          const TreePtrInterface *px ) const
{
	INDENT("@");
    query.Reset();
#ifdef SPIKE
#ifdef SPIKE_MISMATCH
	TreePtr<CPPTree::Integer> wrong_ptr_arch; // won't match in the use-case
    TreePtr<Node> ptr_arch = wrong_ptr_arch.MakeValueArchitype();
	query.RegisterLocalNormalLink( GetPointer(), ptr_arch );	
#endif
#else
#ifdef POINTER_IS_VIA_PARENT    
    TreePtr<Node> x = *px; // only use x - that's the whole point!
    
    // Do a walk over context (the whole x tree)
	Walk e( engine->GetOverallMaster()->GetContext() ); 
	for( Walk::iterator wit=e.begin(); wit!=e.end(); ++wit )
	{
		if( *wit == x ) // found ourself
        {
            // Get the pointer that points to us
            const TreePtrInterface *px = wit.GetCurrentParentPointer();         
            // Make an architypical node of the type of the pointer   
            TreePtr<Node> ptr_arch = px->MakeValueArchitype();
            // Stick that in your pipe + smoke it
            query.RegisterLocalNormalLink( GetPointer(), ptr_arch );
            break; // ignore any more TODO could do better if multiple parents, for example identifier?
        }
    }
#else
	// Note: using MakeValueArchitype() means we need to be using the 
	// TreePtr<Blah> from the original node, not converted to TreePtr<Node>.
	// Thus, it must be passed around via const TreePtrInterface *       
	TreePtr<Node> ptr_arch = px->MakeValueArchitype();
	
	query.RegisterLocalNormalLink( GetPointer(), ptr_arch );
#endif
#endif
}


void PointerIsAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The PointerIs node appears as a slightly flattened pentagon.
	*bold = true;
	*shape = "pentagon";
	*text = string("pointer is"); 
}

