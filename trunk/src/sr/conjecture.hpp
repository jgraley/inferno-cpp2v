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
    };

    struct AgentBlock
    {
		deque<Range> decisions;
		deque<ContainerInterface::iterator> choices;
		AgentBlock *previous_block;
	};

public:
    Conjecture();
    ~Conjecture();
	void PrepareForDecidedCompare();    
	bool IncrementBlock( AgentBlock *block );			                                     
    bool Increment(); // returns true for try again, false for give up				 
    				   
    // General note: decided compare functions should call one of HandleDecision() or 
    // GetDecision() before calling other functions like ReportDecision() or
    // ForceDecision().
    				       
    // Standard interface for decided compare functions
    void RegisterDecisions( Agent *agent, deque<Range> decisions );      
    deque<ContainerInterface::iterator> GetChoices(Agent *agent);
				   
private:
    int register_decision_index;
	Map<Agent *, AgentBlock> agent_blocks;
	AgentBlock *last_block;	
};

};

#endif
