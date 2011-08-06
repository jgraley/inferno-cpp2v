
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
        if( choices.back().end_count > choices.back().end_num )
        {
            choices.resize( choices.size()-1 );
            return Increment();  // TODO use a loop?
        }
    }
       	    
    return true;
}


ContainerInterface::iterator Conjecture::HandleDecision( ContainerInterface::iterator begin,
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
    
    decision_index++;

	// Return whatever choice we made
    return c.it;
}


ContainerInterface::iterator Conjecture::HandleDecision( ContainerInterface::iterator only )
{
    ContainerInterface::iterator end = only;
    ++end;
    return HandleDecision( only, end );
}

