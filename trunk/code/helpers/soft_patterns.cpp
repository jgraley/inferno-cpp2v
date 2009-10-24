#include "soft_patterns.hpp"

SearchReplace::Result SoftExpressonOfType::DecidedCompare( const SearchReplace *sr,
		                                                   shared_ptr<Node> x,
		                                                   SearchReplace::MatchKeys *match_keys,
		                                                   SearchReplace::Conjecture &conj,
		                                                   unsigned context_flags ) const
{
	if( shared_ptr<Expression> xe = dynamic_pointer_cast<Expression>(x) )
	{
	    // Find out the type of the candidate expression	
	    shared_ptr<Type> xt = TypeOf( sr->GetProgram() ).Get( xe );
	    TRACE("TypeOf(%s) is %s\n", TypeInfo(xe).name().c_str(), TypeInfo(xt).name().c_str() );
	    ASSERT(xt);
	    
	    // Punt it back into the search/replace engine
	    return sr->DecidedCompare( xt, type_pattern, match_keys, conj, context_flags );
	}
	else
	{
	    // not even an expression lol that aint going to match (means this node must be in a wider
		// context eg Node or Statement, and the tree contained something other than Expression - so
		// we're restricting to Expressions in addition to checking the type)
	    return SearchReplace::NOT_FOUND;
	}        
}     
