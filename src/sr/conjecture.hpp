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
	typedef map<Agent *, AgentRecord> AgentRecords;
    struct AgentRecord
    {
        shared_ptr<PatternQuery> pq;
        shared_ptr<DecidedQuery> query;
 		AgentRecords::const_iterator previous_agent;
	};

public:
    Conjecture( set<Agent *> my_agents, Agent *root_agent );
    ~Conjecture();

    const struct Plan
    {
        Plan( set<Agent *> my_agents, Agent *root_agent );
        void RecordWalk( AgentRecords::iterator rit );
        
        AgentRecords agent_records;
        AgentRecords::iterator last_agent;		
        set<AgentRecord *> reached;
    } plan;

    void RecordWalk( AgentRecords::iterator rit );
    
    void Start();
    void FillChoicesWithHardBegin( AgentRecords::const_iterator rit );      
    void EnsureChoicesHaveIterators();
    
	bool IncrementAgent( AgentRecords::const_iterator rit, int bc );			                                     
    bool Increment(); // returns true for try again, false for give up				 
    bool IncrementConjecture( AgentRecords::const_iterator rit );     
        				       
    // Standard interface for decided compare functions
    shared_ptr<DecidedQuery> GetQuery( Agent *agent );
};

};

#endif
