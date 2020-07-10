#ifndef CONJECTURE_HPP
#define CONJECTURE_HPP

#include "common/common.hpp"
#include <vector>

namespace SR 
{

class CompareReplace;
class Agent;

/// Utility to keep track of the current set of choices that have been made at decision points, and to advance through them until a full match is found
class Conjecture
{
public:
    struct Range
    {
        ContainerInterface::iterator begin;
        ContainerInterface::iterator end;    
        bool inclusive; // If true, include "end" as a possible choice
        bool operator==(const Range &o) const // Only required for an ASSERT
        {
			return begin==o.begin && end==o.end;
		}
    };

    struct AgentBlock
    {
		Agent *agent;
		deque<Range> decisions;
		deque<ContainerInterface::iterator> choices;
		AgentBlock *previous_block;
		bool seen;
		bool local_match;
	};
    typedef pair<Agent * const, AgentBlock> BlockPair;


public:
    Conjecture(Set<Agent *> my_agents);
    ~Conjecture();
	void PrepareForDecidedCompare(int pass);    
	bool IncrementBlock( AgentBlock *block );			                                     
    bool Increment(); // returns true for try again, false for give up				 
    				       
    // Standard interface for decided compare functions
    void RegisterDecisions( Agent *agent, bool local_match, deque<Range> decisions=deque<Range>() );      
    deque<ContainerInterface::iterator> GetChoices(Agent *agent);
				   
private:
	Map<Agent *, AgentBlock> agent_blocks;
	AgentBlock *last_block;	
	bool prepared;
};

};

#endif
