
#include "search_replace.hpp"
#include "conjecture.hpp"

Conjecture::Conjecture()
{
    failed = false;
}

Conjecture::~Conjecture()
{
    TRACE("Conjecture @%p %d entries\nstart counts: ", this, it_names.size());
    for( int i=0; i<it_names.size(); i++ )
        TRACE("%d ", start_counts[i]);
    TRACE("\nIncrement counts: ", this);
    for( int i=0; i<it_names.size(); i++ )
        TRACE("%d ", inc_counts[i]);
    TRACE("\nIterators: ");
    for( int i=0; i<it_names.size(); i++ )
        TRACE("%s ", it_names[i].c_str());
    TRACE("\nresult: %s\n", failed?"FAILED":"SUCCEEDED");
}

void Conjecture::PrepareForDecidedCompare()
{
	ASSERT( this );

	TRACE("Decision prepare\n");
	decision_index = 0;
}


bool Conjecture::Increment()
{   
    // tracing stuff
    while( choices.size() > it_names.size() )
    {
        start_counts.resize( start_counts.size()+1 );
        inc_counts.resize( inc_counts.size()+1 );
        it_names.resize( it_names.size()+1 );         
        it_names[it_names.size()-1] = (string)(choices[it_names.size()-1].it);
        start_counts[start_counts.size()-1] = 0;
        inc_counts[inc_counts.size()-1] = 0;
    }

	// If we've run out of choices, we're done.
	if( choices.empty() )
	{
	    failed = true;
        TRACE("Giving up\n");
	    return false;
	}
	else if( choices.back().it != choices.back().end )
	{
        inc_counts[choices.size()-1]++;

 		TRACE("Incrementing choice FROM ")(**choices.back().it)("\n");
		++(choices.back().it); // There are potentially more choices so increment the last decision
    }
		
    if( choices.back().it == choices.back().end )
    {
        start_counts[choices.size()-1]++;

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

