#include "pointer_is_agent.hpp"
#include "search_replace.hpp"
#include "agent.hpp"

using namespace SR;


void PointerIsAgent::PatternQueryImpl() const
{
	RememberLink( false, AsAgent(GetPointer()) );
}


bool PointerIsAgent::DecidedQueryImpl( const TreePtrInterface &x ) const
{
	INDENT("@");
	
	// Note: using MakeValueArchitype() means we need to be using the 
	// TreePtr<Blah> from the original node, not converted to TreePtr<Node>.
	// Thus, it must be passed around via const TreePtrInterface &        
	TreePtr<Node> ptr_arch = x.MakeValueArchitype();
	
	RememberLocalLink( false, AsAgent(GetPointer()), ptr_arch );
	return true;
}


void PointerIsAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The PointerIs node appears as a slightly flattened pentagon.
	*bold = true;
	*shape = "pentagon";
	*text = string("pointer is"); 
}

