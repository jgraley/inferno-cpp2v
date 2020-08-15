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
    typedef DecidedQueryCommon::Range Range;
    typedef DecidedQueryCommon::Ranges Ranges;
    typedef DecidedQueryCommon::Choices Choices;
    
    struct AgentRecord
    {
		Agent *agent;
        shared_ptr<DecidedQuery> query;
 		Agent *previous_agent;
        bool linked;
	};

public:
    Conjecture();
    ~Conjecture();

    void Configure(Set<Agent *> my_agents, Agent *root_agent);
    void RecordWalk( Agent *agent );
	bool IncrementAgent( Agent *agent );			                                     
    bool Increment(); // returns true for try again, false for give up				 
    bool IncrementConjecture( Agent *agent ); 
        				       
    // Standard interface for decided compare functions
    DecidedQuery::Choices GetChoices( Agent *agent ) const;
    shared_ptr<DecidedQuery> GetQuery( Agent *agent );
    void FillMissingChoicesWithBegin( Agent *agent );      
				   
private:
	Map<Agent *, AgentRecord> agent_records;
	Agent *last_agent;		
    bool configured;
};

};

#endif
