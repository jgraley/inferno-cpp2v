
#include "search_replace.hpp"
#include "conjecture.hpp"

namespace SR 
{

Conjecture::Conjecture(Set<Agent *> my_agents)
{
    last_block = NULL;
    FOREACH( Agent *a, my_agents )
    {
		AgentBlock block;
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
    
	typedef pair<Agent * const, AgentBlock> BlockPair;
	FOREACH( BlockPair &p, agent_blocks )
	{
		AgentBlock &block = p.second;
		block.seen = false;
	}          
	prepared = true;
}


bool Conjecture::IncrementBlock( AgentBlock *block )
{
	if( block->choices.empty() )
	{
		block->decisions.clear(); // make it defunct
	    return false;
	}
	
	if( block->choices.back() != block->decisions[block->choices.size()-1].end ) 
	{
        ++(last_block->choices.back()); 
        // note this can push us onto "end" which is not valid, so more must be done
    }
		
    if( block->choices.back() == block->decisions[block->choices.size()-1].end )
    {
        block->choices.pop_back();
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


void Conjecture::RegisterDecisions( Agent *agent, bool local_match, deque<Range> decisions )
{                
	ASSERT( prepared );
	
	ASSERT( agent_blocks.IsExist(agent) )(*agent);
 	AgentBlock &block = agent_blocks[agent];
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


deque<ContainerInterface::iterator> Conjecture::GetChoices(Agent *agent)
{            
    if( agent_blocks.IsExist(agent) )
    {
        return agent_blocks[agent].choices;
	}
	else
	{
		return deque<ContainerInterface::iterator>(); // no choices
	}    
}

};
