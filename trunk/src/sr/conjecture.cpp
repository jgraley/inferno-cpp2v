
#include "search_replace.hpp"
#include "conjecture.hpp"

namespace SR 
{

Conjecture::Conjecture()
{
    last_block = NULL;
}


Conjecture::~Conjecture()
{
}


void Conjecture::PrepareForDecidedCompare()
{
	ASSERT( this );

	TRACE("Decision prepare\n");
    register_decision_index = 0;
}


bool Conjecture::IncrementBlock( AgentBlock *block )
{
	if( block->choices.empty() )
	    return false;
	
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
	// If we've run out of choices, we're done.
	if( last_block==NULL )
	    return false;
	
    bool ok = IncrementBlock( last_block );
    if( !ok )
    {
		last_block->decisions.clear(); // make it defunct
		last_block = last_block->previous_block;
		return Increment();
	}
 
    return true;
}


void Conjecture::RegisterDecisions( Agent *agent, deque<Range> decisions )
{                
	if( decisions.empty() )
	    return;
	
 	AgentBlock &block = agent_blocks[agent];
	if( block.decisions.empty() ) // new block or defunct
	{
	    block.previous_block = last_block;	
	    last_block = &block;
	}
    block.decisions = decisions;
    int choices_already_initialised = block.choices.size();
    FOREACH( Range r, decisions )
    {
		if( choices_already_initialised == 0 )
			block.choices.push_back( r.begin );
		else
			choices_already_initialised--;
    }
    ASSERT( block.choices.size()==block.decisions.size() )("%d != %d", block.choices.size(), block.decisions.size() );
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
