
#include "search_replace.hpp"
#include "conjecture.hpp"

//#define STIFF_LINKS

namespace SR 
{

Conjecture::Conjecture(Set<Agent *> my_agents, Agent *root_agent)
{
    FOREACH( Agent *a, my_agents )
    {
		AgentRecord record;
		record.agent = a;
        record.previous_agent = nullptr;
        record.linked = false;
        record.query = make_shared<AgentQuery>();        
		agent_records[a] = record;
	}        
    last_agent = nullptr;
    RecordWalk( root_agent );
#ifndef STIFF_LINKS
    // Undo the linking
    FOREACH( Agent *a, my_agents )
    {
        AgentRecord *record = &agent_records.at(a);
        record->linked = false;
    }
    last_agent = nullptr;
#endif
}


void Conjecture::RecordWalk( Agent *agent )
{
    if( agent_records.count(agent)==0 ) // Probably belongs to master
        return;

    AgentRecord *record = &agent_records.at(agent);

    if( record->linked );
        return; // already reached: probably a coupling
        
    if( last_agent )
        record->previous_agent = last_agent;
    else
        record->previous_agent = nullptr;
    last_agent = agent;
    record->linked = true;

    PatternQueryResult r = agent->PatternQuery();
    
    if( r.GetEvaluator() )
        return; // we don't process evaluators
    
    for( const PatternQueryResult::Link &l : *r.GetNormalLinks() )
        RecordWalk( l.agent );
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


bool Conjecture::IncrementConjecture(Agent *agent)
{   
    AgentRecord *record = &agent_records.at(agent);
    bool ok = IncrementAgent( record->query );
    if( !ok )
    {	if( record->previous_agent )
            return IncrementConjecture( record->previous_agent );
        else
            return false;
	}
 
    return true;
}


bool Conjecture::Increment()
{
    if( last_agent )
        return IncrementConjecture(last_agent);
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
        
#ifndef STIFF_LINKS
    if( !record.linked )
    {
        if( last_agent )
            record.previous_agent = last_agent;
        else
            record.previous_agent = nullptr;
        last_agent = agent;
        record.linked = true;
    }
#endif
    
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
