
#include "search_replace.hpp"
#include "conjecture.hpp"

void Conjecture::PrepareForDecidedCompare()
{
	ASSERT( this );

	TRACE("Decision prepare\n");
	decision_index = 0;
}


bool Conjecture::Increment()
{
	// If we've run out of choices, we're done.
	TRACE();
	if( choices.empty() )
	    return false;
	else if( choices.back().it != choices.back().end )
	{
		TRACE("Incrementing choice FROM ")(**choices.back().it)("\n");
		++(choices.back().it); // There are potentially more choices so increment the last decision
    }
		
    if( choices.back().it == choices.back().end )
    {
        TRACE("Incrementing end count FROM %d\n", choices.back().end_count);
        ++choices.back().end_count;
    }
        
    return true;
}


ContainerInterface::iterator Conjecture::HandleDecision( ContainerInterface::iterator begin,
		                                                 ContainerInterface::iterator end,
		                                                 int en )
{
    Choice c = GetDecision( begin, end, en );
    ReportDecision( c.end_count != c.end_num ); // the first end is assumed to mean this decision has failed
    return c.it;
}


Conjecture::Choice Conjecture::GetDecision( ContainerInterface::iterator begin,
		                                    ContainerInterface::iterator end,
		                                    int en )
{
	ASSERT( this );
	ASSERT( choices.size() >= decision_index ); // consistency check; as we see more decisions, we should be adding them to the conjecture
	Choice c;

	// See if this decision needs to be added to the present Conjecture
	if( choices.size() == decision_index ) // this decision missing from conjecture?
	{
		c.it = begin; // Choose the first option supplied
		c.end = end; // Choose the first option supplied
		c.end_count = 0;
		c.end_num = en;
		c.forced = false;
		choices.push_back( c ); // append this decision so we will iterate it later
		TRACE("Decision %d appending begin at %p\n", decision_index, GetChoicePtr() );
	}
	else // already know about this decision
	{
		// Adopt the current decision based on Conjecture
	    c = choices[decision_index]; // Get present decision
	}

	// Return whatever choice we made
    return c;
}


void Conjecture::ReportDecision( bool ok )
{
	// Check the decision obeys bounds
	if( ok ) 
	{
		// That decision is OK, so move on to the next one
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
	else
	{
		// Throw away the current decision since we ran out of valid choices.
		// The next Increment() will increment the *previous* decision
		TRACE("Decision %d hit end\n", decision_index );
		choices.resize( decision_index );
	}
}

Conjecture::Choice Conjecture::ForceDecision( ContainerInterface::iterator tohere )
{
    // TODO don't keep indexing choices, use a temp lol
	ASSERT( this );
	ASSERT( choices.size() > decision_index ); // consistency check; as we see more decisions, we should be adding them to the conjecture
	if( !choices[decision_index].forced )
	{
	    choices[decision_index].it = tohere;
	    ++tohere;
	    choices[decision_index].end = tohere;
	    choices[decision_index].end_num = 1;
        choices[decision_index].forced = true;
        TRACE("Forcing decision to ")(**(choices[decision_index].it))("\n");
    }
    else
        TRACE("not forcing decision again\n");
        
    return choices[decision_index];
}

