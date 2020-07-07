#include "pointer_is_agent.hpp"
#include "search_replace.hpp"
#include "agent.hpp"

using namespace SR;


PatternQueryResult PointerIsAgent::PatternQuery() const
{
    PatternQueryResult r;
	r.AddLink( false, AsAgent(GetPointer()) );
    return r;
}


DecidedQueryResult PointerIsAgent::DecidedQuery( const TreePtrInterface &x, 
                                                 const deque<ContainerInterface::iterator> &choices ) const
{
	INDENT("@");
    DecidedQueryResult r;
	
	// Note: using MakeValueArchitype() means we need to be using the 
	// TreePtr<Blah> from the original node, not converted to TreePtr<Node>.
	// Thus, it must be passed around via const TreePtrInterface &        
	TreePtr<Node> ptr_arch = x.MakeValueArchitype();
	
	r.AddLocalLink( false, AsAgent(GetPointer()), ptr_arch );
	return r;
}


void PointerIsAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The PointerIs node appears as a slightly flattened pentagon.
	*bold = true;
	*shape = "pentagon";
	*text = string("pointer is"); 
}

