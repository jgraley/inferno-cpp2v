
#include "search_replace.hpp"
#include "conjecture.hpp"

//#define STIFF_LINKS
#define STIFF_CHECK
//#define ROOT_CHECK

#if defined(STIFF_CHECK) && defined(STIFF_LINKS)
#error STIFF_LINKS needs STIFF_CHECK undefined
#endif

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
#ifdef ROOT_CHECK
    // Not a fair check: eg in CleanupDuplicateLabels there is
    // an Evaluator agent (the |) which goes directly to coupled
    // identifiers (the slave S&R is supposed to hit on both 
    // identifiers).
    ASSERT( my_agents.IsExist( root_agent ) );
#endif
    FOREACH( Agent *a, my_agents )
    {
		AgentRecord record;
		record.agent = a;
        record.previous_agent = (Agent *)0xFEEDF00D;
        record.linked = false;
        record.query = make_shared<AgentQuery>();        
        record.story = 0;
		agent_records[a] = record;
	}        
#ifdef ROOT_CHECK
    ASSERT(agent_records.count(root_agent) > 0);
#endif
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
    configured = true;
}


void Conjecture::RecordWalk( Agent *agent )
{
    if( agent_records.count(agent)==0 ) // Probably belongs to master
        return;

    AgentRecord *record = &agent_records.at(agent);

    record->story += 0x1;
    if( record->linked )
        return; // already reached: probably a coupling
        
    record->previous_agent = last_agent;
    last_agent = agent;
    record->linked = true;
    record->story += 0x100;

    PatternQueryResult r = agent->PatternQuery();
    
    // Makes sense, but we won't match the old algo with this
    //if( r.GetEvaluator() )
    //    return; // we don't process evaluators
    record->story += 0x10000;
    
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
    ASSERT( agent_records.IsExist(agent) );
 	AgentRecord &record = agent_records[agent];
        
#ifndef STIFF_LINKS
    if( !record.linked )
    {
#ifdef STIFF_CHECK
        ASSERT(record.previous_agent == last_agent);
#endif
        record.previous_agent = last_agent;
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
