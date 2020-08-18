
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
    FOREACH( Agent *agent, my_agents )
    {
		AgentRecord record;
		record.agent = agent;
        record.previous_agent = (Agent *)0xFEEDF00D;
        record.linked = false;
        record.pq = make_shared<PatternQuery>();
        *(record.pq) = agent->GetPatternQuery();
        record.query = agent->CreateDecidedQuery();        
		agent_records[agent] = record;
	}        
    last_agent = nullptr;
    ConfigRecordWalk( root_agent );
    configured = true;
}


void Conjecture::ConfigRecordWalk( Agent *agent )
{
    if( agent_records.count(agent)==0 ) // Probably belongs to master
        return;

    AgentRecord *record = &agent_records.at(agent);

    if( record->linked )
        return; // already reached: probably a coupling
        
    record->previous_agent = last_agent;
    last_agent = agent;
    record->linked = true;
    auto pq = record->pq;
    
    // Makes sense, but we won't match the old algo with this
    if( pq->GetEvaluator() )
        return; // we don't process evaluators
    
    for( const PatternQuery::Link &l : *(pq->GetNormalLinks()) )
        ConfigRecordWalk( l.agent );
}


bool Conjecture::IncrementAgent( Agent *agent )
{    
    auto query = agent_records.at(agent).query;
    ASSERT( query );
    
	if( query->GetChoices()->empty() )
	{
        // this query is defunct
	    return false;  
	}

    DecidedQueryCommon::Choice back_choice = query->GetChoices()->back();
    const auto &back_decision = (*query->GetDecisions())[query->GetChoices()->size()-1];
    
    // Inclusive case - we let the choice go to end but we won't go any further
    if( back_choice.mode==DecidedQueryCommon::Choice::ITER && back_decision.inclusive && back_choice.iter == back_decision.end )
    {
        query->InvalidateBack();
        return IncrementAgent( agent );
	}

	if( back_choice.mode==DecidedQueryCommon::Choice::BEGIN || back_choice.iter != back_decision.end ) 
	{
        switch( back_choice.mode )
        {
            case DecidedQueryCommon::Choice::ITER:
                ++back_choice.iter; 
                break;
            
            case DecidedQueryCommon::Choice::BEGIN:
                back_choice.iter = back_decision.begin;
                back_choice.mode = DecidedQueryCommon::Choice::ITER;
                break;
        }        
        query->SetBackChoice( back_choice );
    }
		
    // Exclusive case - we don't let the choice be end
    if( back_choice.mode==DecidedQueryCommon::Choice::ITER && !back_decision.inclusive && back_choice.iter == back_decision.end )
    {
        query->InvalidateBack();
        return IncrementAgent( agent );
	}
		
    return true;
}


bool Conjecture::IncrementConjecture(Agent *agent)
{       
    AgentRecord *record = &agent_records.at(agent);
    auto query = record->query;
    auto pq = record->pq;
    
    bool ok = false;
    switch( query->last_activity )
    {
        case DecidedQueryCommon::NEW:
            ASSERT( query->GetDecisions()->size() == pq->GetDecisions()->size() );
            break;
            
        case DecidedQueryCommon::QUERY:
            ASSERT( query->GetDecisions()->size() == pq->GetDecisions()->size() );
            FillMissingChoicesWithBegin(agent);
            ASSERT( query->GetDecisions()->size() == query->GetChoices()->size() );
            ok = IncrementAgent( agent );
            FillMissingChoicesWithBegin(agent);
            query->last_activity = DecidedQueryCommon::CONJECTURE;
            break;
            
        case DecidedQueryCommon::CONJECTURE:
            break;
    }

    if( !ok )
    {
        if( record->previous_agent )
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


DecidedQuery::Choices Conjecture::GetChoices(Agent *agent) const 
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


shared_ptr<DecidedQuery> Conjecture::GetQuery(Agent *agent)
{
    ASSERT(configured);
    return agent_records.at(agent).query;
}


void Conjecture::FillMissingChoicesWithBegin( Agent *agent )
{
    AgentRecord *record = &agent_records.at(agent);
    auto query = record->query;
    auto pq = record->pq;
    //ASSERT( query->GetDecisions()->size() == pq->GetDecisions()->size() );
           
    while( query->GetChoices()->size() < pq->GetDecisions()->size() )
    {
        DecidedQueryCommon::Choice new_choice;
        new_choice.mode = DecidedQueryCommon::Choice::BEGIN;
        query->PushBackChoice( new_choice );
    }
    
    ASSERT( query->GetChoices()->size()==pq->GetDecisions()->size() )
            ("%d != %d", query->GetChoices()->size(), pq->GetDecisions()->size() );
}

};
