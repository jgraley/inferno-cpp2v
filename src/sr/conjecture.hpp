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
	typedef map<const Agent *, AgentRecord> AgentRecords;
    struct AgentRecord
    {
        shared_ptr<PatternQuery> pq;
        shared_ptr<DecidedQuery> query;
 		AgentRecords::const_iterator previous_agent;
	};

public:
    Conjecture( set<Agent *> my_agents, const Agent *base_agent );
    Conjecture( const Agent *agent, shared_ptr<DecidedQuery> query = nullptr );
    
    ~Conjecture();

    const struct Plan
    {
        Plan( set<Agent *> my_agents, const Agent *base_agent );
        Plan( const Agent *agent, shared_ptr<DecidedQuery> query = nullptr );
        void RecordWalk( AgentRecords::iterator rit );
        
        AgentRecords agent_records;
        AgentRecords::iterator last_agent;		
        set<AgentRecord *> reached;
    } plan;

    void RecordWalk( AgentRecords::iterator rit );
    
    void Start();
    void FillChoicesWithHardBegin( AgentRecords::const_iterator rit );      
    
	bool IncrementAgent( AgentRecords::const_iterator rit, int bc );			                                     
    bool Increment(); // returns true for try again, false for give up		
    void Reset();		 
    bool IncrementConjecture( AgentRecords::const_iterator rit );     
        				       
    // Standard interface for decided compare functions
    shared_ptr<DecidedQuery> GetQuery( const Agent *agent );
};

};

#endif
