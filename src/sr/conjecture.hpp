#ifndef CONJECTURE_HPP
#define CONJECTURE_HPP

#include "query.hpp"
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
    typedef DecidedQueryResult::Range Range;
    typedef DecidedQueryResult::Ranges Ranges;
    typedef DecidedQueryResult::Choices Choices;
    
    struct AgentRecord
    {
		Agent *agent;
		Ranges decisions;
		Choices choices;
		AgentRecord *previous_block;
		bool seen;
		bool local_match;
	};
    typedef pair<Agent * const, AgentRecord> BlockPair;

public:
    Conjecture(Set<Agent *> my_agents);
    ~Conjecture();
	void PrepareForDecidedCompare(int pass);    
	bool IncrementBlock( AgentRecord *block );			                                     
    bool Increment(); // returns true for try again, false for give up				 
    				       
    // Standard interface for decided compare functions
    void RegisterDecisions( Agent *agent, bool local_match, Ranges decisions=Ranges() );      
    AgentQuery::Choices GetChoices(Agent *agent);
    AgentQuery::Ranges GetDecisions(Agent *agent);
				   
private:
	Map<Agent *, AgentRecord> agent_blocks;
	AgentRecord *last_block;	
	bool prepared;
};

};

#endif
