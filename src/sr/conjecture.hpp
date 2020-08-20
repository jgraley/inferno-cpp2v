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
    
    struct AgentRecord;
	typedef Map<Agent *, AgentRecord> AgentRecords;
    struct AgentRecord
    {
        shared_ptr<PatternQuery> pq;
        shared_ptr<DecidedQuery> query;
 		AgentRecords::iterator previous_agent;
	};

public:
    Conjecture();
    ~Conjecture();

    void Configure(Set<Agent *> my_agents, Agent *root_agent);
    void ConfigRecordWalk( AgentRecords::iterator rit );
    
    void Start();
    void FillChoicesWithHardBegin( AgentRecords::iterator rit );      
    void EnsureChoicesHaveIterators();
    
	bool IncrementAgent( AgentRecords::iterator rit, int bc );			                                     
    bool Increment(); // returns true for try again, false for give up				 
    bool IncrementConjecture( AgentRecords::iterator rit );     
        				       
    // Standard interface for decided compare functions
    DecidedQuery::Choices GetChoices( Agent *agent ) const;
    shared_ptr<DecidedQuery> GetQuery( Agent *agent );
				   
private:
    AgentRecords agent_records;
	AgentRecords::iterator last_agent;		
    bool configured;
    Set<AgentRecord *> reached;
};

};

#endif
