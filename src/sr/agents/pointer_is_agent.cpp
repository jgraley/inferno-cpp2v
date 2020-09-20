#include "pointer_is_agent.hpp"
#include "../search_replace.hpp"
#include "agent.hpp"
#include "../scr_engine.hpp"

using namespace SR;
//#define SPIKE
// SPIKE with neiter SPIKE_MATCH nor SPIKE_MATCH will fail to register the link
//#define SPIKE_MATCH
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
                                          TreePtr<Node> x ) const
{
	INDENT("@");
    query.Reset();
#ifdef SPIKE
#ifdef SPIKE_MATCH
    TreePtr<Node> spike( new Node );
	query.RegisterNormalLink( GetPointer(), spike );	// Singular Link
#endif
#ifdef SPIKE_MISMATCH
	TreePtr<CPPTree::Integer> spikex; // won't match in the use-case
    TreePtr<Node> spike = spikex.MakeValueArchitype();
	query.RegisterNormalLink( GetPointer(), spike );	// Singular Link
#endif
#else   
    // Do a walk over context (the whole x tree)
    bool found_one_already = false;
	Walk e( master_scr_engine->GetOverallMaster()->GetContext() ); 
	for( Walk::iterator wit=e.begin(); wit!=e.end(); ++wit )
	{
		if( *wit == x ) // found ourself TODO use find()
        {            
            if(found_one_already)
                throw Mismatch(); // X has multiple parents - ambiguous, so don't match
            found_one_already = true;
                
            // Get the pointer that points to us
            const TreePtrInterface *px = wit.GetCurrentParentPointer();         
            // Make an architypical node matching the pointer's type
            TreePtr<Node> ptr_arch = px->MakeValueArchitype();
            // Stick that in your pipe + smoke it
            query.RegisterNormalLink( GetPointer(), ptr_arch ); // Generated Link (one-to-one with X)  
        }
    }
    if(!found_one_already)
    {
        // If there's no parent we must be at the root of the x tree,
        // so simulate a link that allows anything (because in fact
        // you can replace the root node with anything).
        TreePtr<Node> node( new Node );
        query.RegisterNormalLink( GetPointer(), node );	// Singular Link
    }
#endif
}


void PointerIsAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The PointerIs node appears as a slightly flattened pentagon.
	*bold = true;
	*shape = "pentagon";
	*text = string("pointer is"); 
}

