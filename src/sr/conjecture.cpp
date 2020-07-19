
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
        record.previous_record = nullptr;
        record.active = false;
		agent_records[a] = record;
	}        
	prepared = false;
    current_pass = -1;
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
		record.seen_in_current_pass = false;    
	}          
	prepared = true;
    current_pass = pass;
}


bool Conjecture::IncrementAgent( AgentRecord *record )
{    
    ASSERT( record->query );
    ASSERT( record->query->GetChoices()->size() == record->query->GetDecisions()->size() );
	if( record->query->GetDecisions()->empty() )
	{
        // this query is defunct
        record->query = nullptr;
	    return false;  
	}
    const auto &back_decision = record->query->GetDecisions()->back();
    ContainerInterface::iterator back_choice = record->query->GetChoices()->back();
    
    // Inclusive case - we let the choice go to end but we won't go any further
    if( back_decision.inclusive && back_choice == back_decision.end )
    {
        record->query->InvalidateBack();
        return IncrementAgent( record );
	}

	if( back_choice != back_decision.end ) 
	{
        ++back_choice; 
        record->query->SetBackChoice( back_choice );
    }
		
    // Exclusive case - we don't let the choice be end
    if( !back_decision.inclusive && back_choice == back_decision.end )
    {
        record->query->InvalidateBack();
        return IncrementAgent( record );
	}
		
    return true;
}


bool Conjecture::Increment()
{   
    prepared = false;
    current_pass = 1000;
    
	// If we've run out of choices, we're done.
	if( last_record==NULL )
	    return false;
	
    bool ok = IncrementAgent( last_record );
    if( !ok )
    {		
        AgentRecord *record_to_remove = last_record;
		last_record = record_to_remove->previous_record;
        record_to_remove->previous_record = nullptr;
        record_to_remove->active = false;
		return Increment();
	}
 
    return true;
}


void Conjecture::RegisterQuery( Agent *agent )
{
	ASSERT( prepared );
                	
	ASSERT( agent_records.IsExist(agent) )(*agent);
 	AgentRecord &record = agent_records[agent];

    if( !record.active ) // new block or defunct
    {
        record.previous_record = last_record;	
        last_record = &record;
        record.active = true;
    }
    while( record.query->GetChoices()->size() < record.query->GetDecisions()->size() )
    {
        int index = record.query->GetChoices()->size();
        record.query->PushBackChoice( (*record.query->GetDecisions())[index].begin );
    }
    
    ASSERT( record.query->GetChoices()->size()==record.query->GetDecisions()->size() )
          ("%d != %d", record.query->GetChoices()->size(), record.query->GetDecisions()->size() );
}


AgentQuery::Choices Conjecture::GetChoices(Agent *agent)
{            
    if( agent_records.IsExist(agent) )
    {
        return *agent_records[agent].query->GetChoices();
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
        return *agent_records[agent].query->GetDecisions();
	}
	else
	{
		return Ranges(); // no decisions
	}    
}

shared_ptr<AgentQuery> Conjecture::GetQuery(Agent *agent)
{
    ASSERT( agent_records.IsExist(agent) );
 	AgentRecord &record = agent_records[agent];
    
    if( !record.query )
    {
        ASSERT( !record.active );
        record.query = make_shared<AgentQuery>();
    }
    
    return agent_records[agent].query;
}

};
