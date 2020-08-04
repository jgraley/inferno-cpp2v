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
    typedef QueryCommonInterface::Range Range;
    typedef QueryCommonInterface::Ranges Ranges;
    typedef QueryCommonInterface::Choices Choices;
    
    struct AgentRecord
    {
		Agent *agent;
        shared_ptr<AgentQuery> query;
 		Agent *previous_agent;
        bool linked;
        int story;
	};

public:
    Conjecture();
    ~Conjecture();

    void Configure(Set<Agent *> my_agents, Agent *root_agent);
    void RecordWalk( Agent *agent );
	bool IncrementAgent( shared_ptr<AgentQuery> query );			                                     
    bool Increment(); // returns true for try again, false for give up				 
    bool IncrementConjecture( Agent *agent ); 
        				       
    // Standard interface for decided compare functions
    AgentQuery::Choices GetChoices(Agent *agent);
    shared_ptr<AgentQuery> GetQuery(Agent *agent);
    void FillMissingChoicesWithBegin( shared_ptr<AgentQuery> query );      
				   
private:
	Map<Agent *, AgentRecord> agent_records;
	Agent *last_agent;		
    bool configured;
};

};

#endif
