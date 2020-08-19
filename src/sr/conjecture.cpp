
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
        record.pq = make_shared<PatternQuery>();
        *(record.pq) = agent->GetPatternQuery();
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

    if( reached.IsExist(record) )
        return; // already reached: probably a coupling
    reached.insert(record);

    auto pq = record->pq;
    
    // Makes sense, but we won't match the old algo with this
    if( pq->GetEvaluator() )
        return; // we don't process evaluators or their children
        
    if( !pq->GetDecisions()->empty() )
    {           
        record->previous_agent = last_agent;
        last_agent = agent;
    }
    
    for( const PatternQuery::Link &l : *(pq->GetNormalLinks()) )
        ConfigRecordWalk( l.agent );
}


void Conjecture::Start()
{
    for( auto &p : agent_records )
    {
        Agent *agent = p.first;
        p.second.query = agent->CreateDecidedQuery();
        FillChoicesWithHardBegin( agent );
    }
}


void Conjecture::FillChoicesWithHardBegin( Agent *agent )
{
    AgentRecord *record = &agent_records.at(agent);
    auto query = record->query;

    for( int i=0; i<query->GetChoices()->size(); i++ )
    {
        DecidedQueryCommon::Choice new_choice;
        new_choice.mode = DecidedQueryCommon::Choice::BEGIN;
        query->SetChoice( i, new_choice );
    }
}


void Conjecture::EnsureChoicesHaveIterators()
{
    for( auto &p : agent_records )
        p.second.query->EnsureChoicesHaveIterators();
}


bool Conjecture::IncrementAgent( Agent *agent, int bc )
{    
    auto query = agent_records.at(agent).query;
    ASSERT( query );
    
    DecidedQueryCommon::Choice back_choice = (*query->GetChoices())[bc];
    const auto &back_decision = (*query->GetDecisions())[bc];
    
    // Inclusive case - we let the choice go to end but we won't go any further
    if( back_choice.mode==DecidedQueryCommon::Choice::ITER && back_decision.inclusive && back_choice.iter == back_decision.end )
    {
        query->Invalidate(bc);
        if( bc==0 )
            return false;
        else
            return IncrementAgent( agent, bc - 1 );
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
        query->SetChoice( bc, back_choice );
    }
		
    // Exclusive case - we don't let the choice be end
    if( back_choice.mode==DecidedQueryCommon::Choice::ITER && !back_decision.inclusive && back_choice.iter == back_decision.end )
    {
        query->Invalidate(bc);
        if( bc==0 )
            return false;
        else
            return IncrementAgent( agent, bc - 1 );
	}
		
    return true;
}


bool Conjecture::IncrementConjecture(Agent *agent)
{       
    AgentRecord *record = &agent_records.at(agent);
    auto query = record->query;
    auto pq = record->pq;
    
    bool ok = false;

    if( !pq->GetDecisions()->empty() )
    {
        switch( query->last_activity )
        {
            case DecidedQueryCommon::NEW:
                break;
                
            case DecidedQueryCommon::QUERY:
                ok = IncrementAgent( agent, pq->GetDecisions()->size() - 1 );
                query->last_activity = DecidedQueryCommon::CONJECTURE;
                break;
                
            case DecidedQueryCommon::CONJECTURE:
                break;
        }
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

};
