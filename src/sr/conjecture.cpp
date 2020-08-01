
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
        record.previous_agent = nullptr;
        record.linked = false;
        record.query = make_shared<AgentQuery>();        
		agent_records[a] = record;
	}        
}


Conjecture::~Conjecture()
{
}


bool Conjecture::IncrementAgent( shared_ptr<AgentQuery> query )
{    
    ASSERT( query );
    
	if( query->GetDecisions()->empty() )
	{
        // this query is defunct
	    return false;  
	}
    FillMissingChoicesWithBegin(query);
    
    const auto &back_decision = query->GetDecisions()->back();
    ContainerInterface::iterator back_choice = query->GetChoices()->back();
    
    // Inclusive case - we let the choice go to end but we won't go any further
    if( back_decision.inclusive && back_choice == back_decision.end )
    {
        query->InvalidateBack();
        return IncrementAgent( query );
	}

	if( back_choice != back_decision.end ) 
	{
        ++back_choice; 
        query->SetBackChoice( back_choice );
    }
		
    // Exclusive case - we don't let the choice be end
    if( !back_decision.inclusive && back_choice == back_decision.end )
    {
        query->InvalidateBack();
        return IncrementAgent( query );
	}
		
    return true;
}


bool Conjecture::IncrementConjecture(AgentRecord *record)
{   
    bool ok = IncrementAgent( record->query );
    if( !ok )
    {	if( record->previous_agent )
            return IncrementConjecture( &agent_records.at(record->previous_agent) );
        else
            return false;
	}
 
    return true;
}


bool Conjecture::Increment()
{
    if( last_record )
        return IncrementConjecture(last_record);
    else
        return false;
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


shared_ptr<AgentQuery> Conjecture::GetQuery(Agent *agent)
{
    ASSERT( agent_records.IsExist(agent) );
 	AgentRecord &record = agent_records[agent];
        
    if( !record.linked )
    {
        if( last_record )
            record.previous_agent = last_record->agent;
        else
            record.previous_agent = nullptr;
        last_record = &record;
        record.linked = true;
    }
    
    shared_ptr<AgentQuery> query = agent_records[agent].query;
    //FillMissingChoicesWithBegin(query);
    return query;
}


void Conjecture::FillMissingChoicesWithBegin( shared_ptr<AgentQuery> query )
{
    ASSERT( query );
       
    while( query->GetChoices()->size() < query->GetDecisions()->size() )
    {
        int index = query->GetChoices()->size();
        query->PushBackChoice( (*query->GetDecisions())[index].begin );
    }
    
    ASSERT( query->GetChoices()->size()==query->GetDecisions()->size() )
            ("%d != %d", query->GetChoices()->size(), query->GetDecisions()->size() );
}

};
