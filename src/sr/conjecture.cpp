
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


void Conjecture::RegisterDecisions( Agent *agent, deque<Range> decisions )
{                
	if( decisions.empty() )
	    return;
	
    bool first_one = true;
    FOREACH( Range r, decisions )
    {
		ASSERT( register_decision_index <= choices.size() ); // consistency check; as we see more decisions, we should be adding them to the conjecture

		TRACE("Register decision index #%d size=#%d\n", register_decision_index, choices.size());

		// See if this decision needs to be added to the present Conjecture
		if( register_decision_index == choices.size() ) // this decision missing from conjecture?
		{
    		Choice c;
			if( first_one )
			{    
				c.agent = agent;                
                c.num_decisions = decisions.size();
			}
			else
			{
				c.agent = NULL;
                c.num_decisions = 0;
			}
			c.it = r.begin; // Choose the first option supplied
			c.choice_num = 0;
			c.end = r.end; 
			choices.push_back( c ); // append this decision so we will iterate it later
			
			ResizeCounts();
			start_counts[choices.size()-1]++;
			
			first_one = false;
		}
		register_decision_index++;
	}
}


deque<ContainerInterface::iterator> Conjecture::GetChoices(Agent *agent)
{            
    deque<ContainerInterface::iterator> ac;
    bool ours = false;
    FOREACH( const Choice &c, choices )
    {
        if( c.agent )
            ours = (c.agent == agent);
        if( ours )
            ac.push_back(c.it);
    }
    return ac;
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
