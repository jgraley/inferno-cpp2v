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
        shared_ptr<AgentQuery> query;
		AgentRecord *previous_record;
        bool active;
	};

public:
    Conjecture(Set<Agent *> my_agents);
    ~Conjecture();
	bool IncrementAgent( AgentRecord *record );			                                     
    bool Increment(); // returns true for try again, false for give up				 
    				       
    // Standard interface for decided compare functions
    AgentQuery::Choices GetChoices(Agent *agent);
    shared_ptr<AgentQuery> GetQuery(Agent *agent);
				   
private:
    void FillMissingChoicesWithBegin( AgentRecord *record );      
	Map<Agent *, AgentRecord> agent_records;
	AgentRecord *last_record;	
};

};

#endif
