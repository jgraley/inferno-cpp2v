
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
    auto &decision = (*record->query->GetDecisions())[record->query->GetChoices()->size()-1];

    // Inclusive case - we let the choice go to end() but we won't go any further
    if( decision.inclusive && record->query->GetChoices()->back() == decision.end )
    {
        record->query->GetChoices()->pop_back();
        record->query->GetDecisions()->pop_back();
        return IncrementAgent( record );
	}

	if( record->query->GetChoices()->back() != decision.end ) 
	{
        ++(last_record->query->GetChoices()->back()); 
    }
		
    // Exclusive case - we don't let the choice be end
    if( !decision.inclusive && record->query->GetChoices()->back() == decision.end )
    {
        record->query->GetChoices()->pop_back();
        record->query->GetDecisions()->pop_back();
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

	if( record.seen_in_current_pass )
	{
	    ASSERT( record.query->GetDecisionCount() == record.query->GetDecisions()->size() )(*agent)
              (" %d!=%d %d", record.query->GetDecisionCount(), record.query->GetDecisions()->size(), record.query->GetChoices()->size());
        return;
	}

	if( record.query->GetDecisionCount()==0 )
	    return;	// TODO ideally, we'd determine this from a PatternQuery(), and not even have an agent record for it
 
    // Feed the decisions info in the blocks structure back to the conjecture
    record.query->GetDecisions()->clear();
    FOREACH( const DecidedQueryResult::Block &b, record.query->GetBlocks() )
        if( b.is_decision ) 
            record.query->GetDecisions()->push_back( b.decision );
    record.local_match = record.query->IsLocalMatch(); // always overwrite this field - if the local match fails it will be the last call here before Increment()
    
    if( !record.active ) // new block or defunct
    {
        record.previous_record = last_record;	
        record.active = true;
        last_record = &record;
    }
    record.seen_in_current_pass = true;
    while( record.query->GetChoices()->size() < record.query->GetDecisions()->size() )
    {
        int index = record.query->GetChoices()->size();
        record.query->GetChoices()->push_back( (*record.query->GetDecisions())[index].begin );
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
