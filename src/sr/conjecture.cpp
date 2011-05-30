
#include "search_replace.hpp"
#include "conjecture.hpp"

void Conjecture::PrepareForDecidedCompare()
{
	ASSERT( this );

	TRACE("Decision prepare\n");
	decision_index = 0;
}

bool Conjecture::ShouldTryMore( Result r, int threshold )
{
	ASSERT( this );

	if( r == FOUND )
    	return false; // stop trying if we found a match

    if( choices.size() <= threshold ) // we've made all the decisions we can OR
        return false;         // our last decision went out of bounds

    return true;
}


Result Conjecture::Search( TreePtr<Node> x,
					       TreePtr<Node> pattern,
						   bool can_key,
					  	   const CompareReplace *sr )
{
	// Loop through candidate conjectures
	while(1)
	{
		// Try out the current conjecture. This will call HandlDecision() once for each decision;
		// HandleDecision() will return the current choice for that decision, if absent it will
		// add the decision and choose the first choice, if the decision reaches the end it
		// will remove the decision.
		Result r = sr->MatchingDecidedCompare( x, pattern, can_key, *this );

		// If we got a match, we're done. If we didn't, and we've run out of choices, we're done.
		if( r || choices.empty() )
		    return r;
		else
			++(choices.back()); // There are potentially more choices so increment the last decision
	}
}


ContainerInterface::iterator Conjecture::HandleDecision( ContainerInterface::iterator begin,
		                                                 ContainerInterface::iterator end )
{
	ASSERT( this );
	ASSERT( choices.size() >= decision_index ); // consistency check; as we see more decisions, we should be adding them to the conjecture
	Choice c;

	// See if this decision needs to be added to the present Conjecture
	if( choices.size() == decision_index ) // this decision missing from conjecture?
	{
		c = begin; // Choose the first option supplied
		choices.push_back( c ); // append this decision so we will iterate it later
		TRACE("Decision %d appending begin\n", decision_index );
	}
	else // already know about this decision
	{
		// Adopt the current decision based on Conjecture
	    c = choices[decision_index]; // Get present decision
	}

	// Check the decision obeys bounds
	if( c == end ) // gone off the end?
	{
		// throw away the bad iterator; will force initialisation to begin() next time
		// NOTE: we will still return end in this case, i.e. an invalid iterator. This tells
		// the caller to please not try to do any matching with this decision, but fall out
		// with NOT_FOUND.
		TRACE("Decision %d hit end\n", decision_index );
		choices.resize( decision_index );
	}
	else
	{
		// That decision is OK, so move to the next one
		TRACE("Decision %d OK\n", decision_index );

/* JSG this is slow!
 		bool seen_c=false;
		for( Choice i = begin; i != end; ++i )
		{
			TRACE("%p == %p?\n", (*i).get(), (*c).get() );
			seen_c |= (i==c);
		}
		ASSERT( seen_c )("Decision #%d: c not in x or x.end(), seems to have overshot!!!!", decision_index);
*/
		decision_index++;
	}

	// Return whatever choice we made
    return c;
}


