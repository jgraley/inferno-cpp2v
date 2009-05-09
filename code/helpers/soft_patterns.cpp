#include "soft_patterns.hpp"

bool SoftExpressonOfType::IsMatchPattern( SearchReplace *sr, shared_ptr<Node> x )
{
	if( shared_ptr<Expression> xe = dynamic_pointer_cast<Expression>(x) )
	{
	    // Find out the type of the candidate expression	
	    shared_ptr<Type> xt = TypeOf().Get( sr->GetProgram(), xe );
	    TRACE("TypeOf(%s) is %s\n", TypeInfo(xe).name().c_str(), TypeInfo(xt).name().c_str() );
	    ASSERT(xt);
	    
	    // Punt it back into the search/replace engine
	    return sr->IsMatchPattern( xt, type_pattern );
	}
	else
	{
	    // not even an expression lol that aint going to match
	    return false;
	}        
}     
