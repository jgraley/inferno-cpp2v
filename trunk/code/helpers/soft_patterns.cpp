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
/*
 *     // Key for a match set, viewed as an STL-type range.
    struct Key
    {
    	bool keyed; // begin and end only valid if this is true
    	Choice begin;
    	Choice end;
    }
    // Match set - if required, construct a set of these, fill in the set
    // of shared pointers but don't worry about key, pass to SearchReplace constructor.
    struct MatchSet : public set< shared_ptr<Node> >
    {
        mutable Key key_x;    // This is filled in by the search and replace engine
    };
 *
 */
