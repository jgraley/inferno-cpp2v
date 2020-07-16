
#include "search_replace.hpp"
#include "conjecture.hpp"

namespace SR 
{

Conjecture::Conjecture(Set<Agent *> my_agents)
{
    last_record = NULL;
    FOREACH( Agent *a, my_agents )
    {
		AgentRecord record;
		record.agent = a;
		record.seen = false;
		agent_records[a] = record;
	}        
	prepared = false;
}


Conjecture::~Conjecture()
{
}


void Conjecture::PrepareForDecidedCompare(int pass)
{
	ASSERT( this );

	TRACE("Decision prepare\n");
    
	FOREACH( auto &p, agent_records )
	{
		AgentRecord &record = p.second;
		record.seen = false;
	}          
	prepared = true;
}


bool Conjecture::IncrementAgent( AgentRecord *record )
{    
    ASSERT( record->choices.size() == record->decisions.size() );
	if( record->decisions.empty() )
	{
	    return false;  // this query is defunct
	}
    auto &decision = record->decisions[record->choices.size()-1];

    // Inclusive case - we let the choice go to end() but we won't go any further
    if( decision.inclusive && record->choices.back() == decision.end )
    {
        record->choices.pop_back();
        record->decisions.pop_back();
        return IncrementAgent( record );
	}

	if( record->choices.back() != decision.end ) 
	{
        ++(last_record->choices.back()); 
    }
		
    // Exclusive case - we don't let the choice be end
    if( !decision.inclusive && record->choices.back() == decision.end )
    {
        record->choices.pop_back();
        record->decisions.pop_back();
        return IncrementAgent( record );
	}
		
    return true;
}


bool Conjecture::Increment()
{   
    prepared = false;

	// If we've run out of choices, we're done.
	if( last_record==NULL )
	    return false;
	
    bool ok = IncrementAgent( last_record );
    if( !ok )
    {		
		last_record = last_record->previous_record;
		return Increment();
	}
 
    return true;
}


void Conjecture::RegisterDecisions( Agent *agent, bool local_match, Ranges decisions )
{                
	ASSERT( prepared );
	
	ASSERT( agent_records.IsExist(agent) )(*agent);
 	AgentRecord &record = agent_records[agent];
    record.local_match = local_match; // always overwrite this field - if the local match fails it will be the last call here before Increment()

	if( decisions.empty() )
	    return;
	
	if( record.seen )
	{
	    ASSERT( record.decisions == decisions )(*agent)(" %d!=%d %d", record.decisions.size(), decisions.size(), record.choices.size());
	}
	else
	{
		if( record.decisions.empty() ) // new block or defunct
		{
			record.previous_record = last_record;	
			last_record = &record;
            
		}
		record.seen = true;
		record.decisions = decisions;
		while( record.choices.size() < record.decisions.size() )
		{
			int index = record.choices.size();
			record.choices.push_back( record.decisions[index].begin );
		}
		ASSERT( record.choices.size()==record.decisions.size() )("%d != %d", record.choices.size(), record.decisions.size() );
	}
}


AgentQuery::Choices Conjecture::GetChoices(Agent *agent)
{            
    if( agent_records.IsExist(agent) )
    {
        return agent_records[agent].choices;
	}
	else
	{
		return Choices(); // no choices
	}    
}

AgentQuery::Ranges Conjecture::GetDecisions(Agent *agent)
{            
    if( agent_records.IsExist(agent) )
    {
        return agent_records[agent].decisions;
	}
	else
	{
		return Ranges(); // no decisions
	}    
}

};
