
#include "search_replace.hpp"
#include "conjecture.hpp"

namespace SR 
{

Conjecture::Conjecture()
{
    failed = false;
}


Conjecture::~Conjecture()
{
    if( !it_names.empty() )
    {
        TRACE("Conjecture dump step %d; start counts: ", HitCount::instance.GetStep());
        for( int i=0; i<it_names.size(); i++ )
            TRACE("%d ", start_counts[i]);
        TRACE("; inc counts: ", this);
        for( int i=0; i<it_names.size(); i++ )
            TRACE("%d ", inc_counts[i]);
        TRACE("; iterators: ");
        for( int i=0; i<it_names.size(); i++ )
        {
            string s = it_names[i];
            for( int i=0; i<s.size(); i++ )
                if( s[i] == '<' )
                    break;
                else 
                    TRACE("%c", s[i]);
            TRACE(" ");
        }
        TRACE("; %s\n", failed?"FAILED":"SUCCEEDED");
    }
}


void Conjecture::PrepareForDecidedCompare()
{
	ASSERT( this );

	TRACE("Decision prepare\n");
	get_choice_index = 0;
    register_decision_index = 0;
    current_agent = NULL;
    agent_first_index = 0;
}


bool Conjecture::Increment(bool trace)
{   
    if(trace)
    {
        string s;
        int i=0;
        FOREACH( const Choice &c, choices )
        {
            s += SSPrintf("%d:", i) + ChoiceAsString(c) + " ";
            i++;
        }
    }
    
	// If we've run out of choices, we're done.
	if( choices.empty() )
	{
	    failed = true;
        TRACE("ConjSpin Giving up\n");
	    return false;
	}
	
	if( choices.back().it != choices.back().end )
	{
    	ResizeCounts();
        inc_counts[choices.size()-1]++;

 		TRACE("Incrementing decision #%d FROM ", choices.size()-1)(ChoiceAsString(choices.back()))("\n");
		++(choices.back().it); // There are potentially more choices so increment the last decision
        ++(choices.back().choice_num);
    }
		
    if( choices.back().it == choices.back().end )
    {
        choices.resize( choices.size()-1 );
        bool r = Increment(false);  
        if( !r )
            return false;
    }       	    
       	
    if(trace)
    {
        string s;
        int i=0;
        FOREACH( const Choice &c, choices )
        {            
            s += SSPrintf("%d:", i) + ChoiceAsString(c) + " ";
            i++;
        }
        TRACE("ConjSpin ")(s)("...\n");
    }

    return true;
}


void Conjecture::BeginAgent( Agent *agent )
{            
    ASSERT( !current_agent );
    
    // Start tracking the next agent if not already done
    if( register_decision_index >= choices.size() )
    {
        current_agent = agent;
        agent_first_index = register_decision_index;
    }
}


void Conjecture::EndAgent()
{            
    // Store info for current agent if there was one
    if( current_agent )
    {
        int n = register_decision_index - agent_first_index;
        if( n >= 1 )
        {
            choices[agent_first_index].agent = current_agent;                
            choices[agent_first_index].num_decisions = n;
        }
        current_agent = NULL;
        agent_first_index = 0;
    }
}


int Conjecture::GetCount(Agent *agent)
{            
    if( register_decision_index < choices.size() && choices[register_decision_index].agent == agent) 
    {
        // There is at least one choice and it is marked for us (we might make
        // no decisions, in which case we are not recorded in the choice)
        TRACE("Retrieved count %d\n", choices[register_decision_index].num_decisions);
        return choices[register_decision_index].num_decisions;
    }
    else 
    {
        return 0;
    }
}


bool Conjecture::GetChoice(ContainerInterface::iterator &it)
{
    ASSERT( this );
    Choice c;
    bool known;

    // See if this decision needs to be added to the present Conjecture
    known = ( get_choice_index < choices.size() ); // this decision missing from conjecture?

    if( known ) // already know about this decision
    {
        // Adopt the current decision based on Conjecture
        c = choices[get_choice_index]; // Get present decision
        it = c.it;
        TRACE("Existing decision choosing ")(ChoiceAsString(c))("\n");
    }
    
    get_choice_index++;
    
    // Return whatever choice we made
    return known;
}


void Conjecture::RegisterDecision( ContainerInterface::iterator begin,
	                               ContainerInterface::iterator end )
{
	ASSERT( this );
	ASSERT( register_decision_index <= choices.size() ); // consistency check; as we see more decisions, we should be adding them to the conjecture
	Choice c;

    TRACE("Register decision index #%d size=#%d\n", register_decision_index, choices.size());

    // See if this decision needs to be added to the present Conjecture
	if( register_decision_index >= choices.size() ) // this decision missing from conjecture?
	{
		c.it = begin; // Choose the first option supplied
		c.choice_num = 0;
		c.end = end; // Choose the first option supplied
		c.agent = NULL;
		choices.push_back( c ); // append this decision so we will iterate it later
        
        ResizeCounts();
        start_counts[choices.size()-1]++;
	}
    
    register_decision_index++;
}


void Conjecture::ResizeCounts()
{
    while( choices.size() > it_names.size() )
    {
        start_counts.resize( start_counts.size()+1 );
        inc_counts.resize( inc_counts.size()+1 );
        it_names.resize( it_names.size()+1 );         
        it_names[it_names.size()-1] = ChoiceAsString(choices[it_names.size()-1]);
        start_counts[start_counts.size()-1] = 0;
        inc_counts[inc_counts.size()-1] = 0;
    }
}


string Conjecture::ChoiceAsString(const Choice &c)
{
    string s;
    if( c.it==c.end )
        s=SSPrintf("end");
    else
    {
        TreePtr<Node> n = *c.it;
        if( !n )
            s="NULL";
        else
            s=(*n).GetTrace();
        s+=SSPrintf("[%d]", c.choice_num);
    }
    return s;        
}


};
/*
 **/