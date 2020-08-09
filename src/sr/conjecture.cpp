
#include "search_replace.hpp"
#include "conjecture.hpp"

namespace SR 
{

Conjecture::Conjecture() : 
    configured(false)
{
}


Conjecture::~Conjecture()
{
}


void Conjecture::Configure(Set<Agent *> my_agents, Agent *root_agent)
{
    FOREACH( Agent *a, my_agents )
    {
		AgentRecord record;
		record.agent = a;
        record.previous_agent = (Agent *)0xFEEDF00D;
        record.linked = false;
        record.query = make_shared<AgentQuery>();        
		agent_records[a] = record;
	}        
    last_agent = nullptr;
    RecordWalk( root_agent );
    configured = true;
}


void Conjecture::RecordWalk( Agent *agent )
{
    if( agent_records.count(agent)==0 ) // Probably belongs to master
        return;

    AgentRecord *record = &agent_records.at(agent);

    if( record->linked )
        return; // already reached: probably a coupling
        
    record->previous_agent = last_agent;
    last_agent = agent;
    record->linked = true;

    PatternQueryResult r = agent->PatternQuery();
    
    // Makes sense, but we won't match the old algo with this
    if( r.GetEvaluator() )
        return; // we don't process evaluators
    
    for( const PatternQueryResult::Link &l : *r.GetNormalLinks() )
        RecordWalk( l.agent );
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
    QueryCommonInterface::Choice back_choice = query->GetChoices()->back();
    
    // Inclusive case - we let the choice go to end but we won't go any further
    if( back_choice.mode==QueryCommonInterface::Choice::ITER && back_decision.inclusive && back_choice.iter == back_decision.end )
    {
        query->InvalidateBack();
        return IncrementAgent( query );
	}

	if( back_choice.mode==QueryCommonInterface::Choice::BEGIN || back_choice.iter != back_decision.end ) 
	{
        switch( back_choice.mode )
        {
            case QueryCommonInterface::Choice::ITER:
                ++back_choice.iter; 
                break;
            
            case QueryCommonInterface::Choice::BEGIN:
                back_choice.iter = back_decision.begin;
                back_choice.mode = QueryCommonInterface::Choice::ITER;
                break;
        }        
        query->SetBackChoice( back_choice );
    }
		
    // Exclusive case - we don't let the choice be end
    if( back_choice.mode==QueryCommonInterface::Choice::ITER && !back_decision.inclusive && back_choice.iter == back_decision.end )
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
    ASSERT(configured);
    if( last_agent )
        return IncrementConjecture(last_agent);
    else
        return false;
}


AgentQuery::Choices Conjecture::GetChoices(Agent *agent) const 
{            
    ASSERT(configured);
    if( agent_records.IsExist(agent) )
    {
        return *agent_records.at(agent).query->GetChoices();
	}
	else
	{
		return Choices(); // no choices
	}    
}


shared_ptr<AgentQuery> Conjecture::GetQuery(Agent *agent)
{
    ASSERT(configured);
    return agent_records.at(agent).query;
}


void Conjecture::FillMissingChoicesWithBegin( shared_ptr<AgentQuery> query )
{
    ASSERT( query );
       
    while( query->GetChoices()->size() < query->GetDecisions()->size() )
    {
        QueryCommonInterface::Choice new_choice;
        new_choice.mode = QueryCommonInterface::Choice::BEGIN;
        query->PushBackChoice( new_choice );
        
        //int index = query->GetChoices()->size();
        //query->PushBackChoice( (*query->GetDecisions())[index].begin );
    }
    
    ASSERT( query->GetChoices()->size()==query->GetDecisions()->size() )
            ("%d != %d", query->GetChoices()->size(), query->GetDecisions()->size() );
}

};
