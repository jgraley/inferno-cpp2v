
#include "search_replace.hpp"
#include "agents/agent.hpp"
#include "link.hpp"

#include "conjecture.hpp"

namespace SR 
{
Conjecture::Plan::Plan( set<Agent *> my_agents, const Agent *root_agent )
{
    last_agent = agent_records.end();

    if( my_agents.empty() )
        return; // no decisions at all this time!

    FOREACH( const Agent *agent, my_agents )
    {
		AgentRecord record;
        record.pq = agent->GetPatternQuery();
        record.query = agent->CreateDecidedQuery();
		agent_records[agent] = record;
	}        
    AgentRecords::iterator root_rit = agent_records.find(root_agent);
    ASSERT( root_rit != agent_records.end() );
    RecordWalk( root_rit );
}


Conjecture::Plan::Plan( const Agent *agent, shared_ptr<DecidedQuery> query )
{
    last_agent = agent_records.end();

    AgentRecord record;
    record.pq = agent->GetPatternQuery();
    record.query = query ? query : agent->CreateDecidedQuery();
    agent_records[agent] = record;

    AgentRecords::iterator root_rit = agent_records.find(agent);
    ASSERT( root_rit != agent_records.end() );
    RecordWalk( root_rit );
}


void Conjecture::Plan::RecordWalk( AgentRecords::iterator rit )
{
    AgentRecord &record = rit->second;

    if( reached.count(&record) > 0 ) // TODO put the rits in there?
        return; // already reached: probably a coupling
    reached.insert(&record);

    auto pq = record.pq;
    
    // Makes sense, but we won't match the old algo with this
    if( pq->GetEvaluator() )
        return; // we don't process evaluators or their children
        
    if( !pq->GetDecisions().empty() )
    {           
        record.previous_agent = last_agent;
        last_agent = rit;
    }
    
    for( PatternLink link : pq->GetNormalLinks() )
    {
        AgentRecords::iterator child_rit = agent_records.find(link.GetChildAgent());
        if( child_rit != agent_records.end() ) // If fails, probably belongs to master
            RecordWalk( child_rit );
    }
}


Conjecture::Conjecture( set<Agent *> my_agents, const Agent *root_agent ) :
    plan( my_agents, root_agent )
{
}


Conjecture::Conjecture( const Agent *agent, shared_ptr<DecidedQuery> query ) :
    plan( agent, query )
{
}


Conjecture::~Conjecture()
{
}


void Conjecture::Start()
{
    for( AgentRecords::const_iterator rit=plan.agent_records.begin(); 
         rit != plan.agent_records.end();
         ++rit )
    {
        rit->second.query->Start();
        FillChoicesWithHardBegin( rit );
    }
}


void Conjecture::FillChoicesWithHardBegin( AgentRecords::const_iterator rit )
{
    AgentRecord record = rit->second;
    auto query = record.query;

    for( int i=0; i<query->GetChoices().size(); i++ )
    {
        DecidedQueryCommon::Choice new_choice;
        new_choice.mode = DecidedQueryCommon::Choice::BEGIN;
        query->SetChoice( i, new_choice );
    }
}


void Conjecture::EnsureChoicesHaveIterators()
{
    for( auto &p : plan.agent_records )
        p.second.query->EnsureChoicesHaveIterators();
}


bool Conjecture::IncrementAgent( AgentRecords::const_iterator rit, int bc )
{    
    AgentRecord record = rit->second;
    auto query = record.query;
    
    DecidedQueryCommon::Choice back_choice = query->GetChoices()[bc];
    const auto &back_decision = query->GetDecisions()[bc];
    
    // Inclusive case - we let the choice go to end but we won't go any further
    if( back_choice.mode==DecidedQueryCommon::Choice::ITER && back_decision.inclusive && back_choice.iter == back_decision.end )
    {
        query->Invalidate(bc);
        if( bc==0 )
            return false;
        else
            return IncrementAgent( rit, bc - 1 );
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
            return IncrementAgent( rit, bc - 1 );
	}
		
    return true;
}


bool Conjecture::IncrementConjecture(AgentRecords::const_iterator rit)
{       
    AgentRecord record = rit->second;
    auto query = record.query;
    auto pq = record.pq;
    
    bool ok = false;

    if( !pq->GetDecisions().empty() )
    {
        switch( query->last_activity )
        {
            case DecidedQueryCommon::NEW:
                break;
                
            case DecidedQueryCommon::QUERY:
                ok = IncrementAgent( rit, pq->GetDecisions().size() - 1 );
                query->last_activity = DecidedQueryCommon::CONJECTURE;
                break;
                
            case DecidedQueryCommon::CONJECTURE:
                break;
        }
    }

    if( !ok )
    {
        if( record.previous_agent != plan.agent_records.end() )
            return IncrementConjecture( record.previous_agent );
        else
            return false;
	}
 
    return true;
}


bool Conjecture::Increment()
{
    if( plan.last_agent != plan.agent_records.end() )
        return IncrementConjecture(plan.last_agent);
    else
        return false; // no decisions at all this time!
}


shared_ptr<DecidedQuery> Conjecture::GetQuery(const Agent *agent)
{
    return plan.agent_records.at(agent).query;
}

};
