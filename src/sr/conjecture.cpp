
#include "search_replace.hpp"
#include "conjecture.hpp"

namespace SR 
{

Conjecture::Conjecture(Set<Agent *> my_agents)
{
    last_block = NULL;
    FOREACH( Agent *a, my_agents )
    {
		AgentRecord block;
		block.agent = a;
		block.seen = false;
		agent_blocks[a] = block;
	}        
	prepared = false;
}


Conjecture::~Conjecture()
{
}


void Conjecture::PrepareForDecidedCompare(int pass)
{
	ASSERT( this );

	TRACE("Decision prepare\n");
    
	typedef pair<Agent * const, AgentRecord> BlockPair;
	FOREACH( BlockPair &p, agent_blocks )
	{
		AgentRecord &block = p.second;
		block.seen = false;
	}          
	prepared = true;
}


bool Conjecture::IncrementBlock( AgentRecord *block )
{    
    ASSERT( block->choices.size() == block->decisions.size() );
	if( block->decisions.empty() )
	{
	    return false;  // this block is defunct
	}
    auto &decision = block->decisions[block->choices.size()-1];

    // Inclusive case - we let the choice go to end() but we won't go any further
    if( decision.inclusive && block->choices.back() == decision.end )
    {
        block->choices.pop_back();
        block->decisions.pop_back();
        return IncrementBlock( block );
	}

	if( block->choices.back() != decision.end ) 
	{
        ++(last_block->choices.back()); 
    }
		
    // Exclusive case - we don't let the choice be end
    if( !decision.inclusive && block->choices.back() == decision.end )
    {
        block->choices.pop_back();
        block->decisions.pop_back();
        return IncrementBlock( block );
	}
		
    return true;
}


bool Conjecture::Increment()
{   
    prepared = false;

	// If we've run out of choices, we're done.
	if( last_block==NULL )
	    return false;
	
    bool ok = IncrementBlock( last_block );
    if( !ok )
    {		
		last_block = last_block->previous_block;
		return Increment();
	}
 
    return true;
}


void Conjecture::RegisterDecisions( Agent *agent, bool local_match, Ranges decisions )
{                
	ASSERT( prepared );
	
	ASSERT( agent_blocks.IsExist(agent) )(*agent);
 	AgentRecord &block = agent_blocks[agent];
    block.local_match = local_match; // always overwrite this field - if the local match fails it will be the last call here before Increment()

	if( decisions.empty() )
	    return;
	
	if( block.seen )
	{
	    ASSERT( block.decisions == decisions )(*agent)(" %d!=%d %d", block.decisions.size(), decisions.size(), block.choices.size());
	}
	else
	{
		if( block.decisions.empty() ) // new block or defunct
		{
			block.previous_block = last_block;	
			last_block = &block;
		}
		block.seen = true;
		block.decisions = decisions;
		while( block.choices.size() < block.decisions.size() )
		{
			int index = block.choices.size();
			block.choices.push_back( block.decisions[index].begin );
		}
		ASSERT( block.choices.size()==block.decisions.size() )("%d != %d", block.choices.size(), block.decisions.size() );
	}
}


Conjecture::Choices Conjecture::GetChoices(Agent *agent)
{            
    if( agent_blocks.IsExist(agent) )
    {
        return agent_blocks[agent].choices;
	}
	else
	{
		return Choices(); // no choices
	}    
}

Conjecture::Ranges Conjecture::GetDecisions(Agent *agent)
{            
    if( agent_blocks.IsExist(agent) )
    {
        return agent_blocks[agent].decisions;
	}
	else
	{
		return Ranges(); // no decisions
	}    
}

};
