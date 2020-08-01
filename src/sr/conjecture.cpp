
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


bool Conjecture::Increment()
{   
	// If we've run out of choices, we're done.
	if( last_record==NULL )
	    return false;
	
    bool ok = IncrementAgent( last_record->query );
    if( !ok )
    {		
        AgentRecord *record_to_remove = last_record;
        record_to_remove->query = nullptr;
		last_record = record_to_remove->previous_record;
        record_to_remove->previous_record = nullptr;
		return Increment();
	}
 
    return true;
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
        
    if( !record.query )
    {
        record.query = make_shared<AgentQuery>();
        record.previous_record = last_record;	
        last_record = &record;
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
