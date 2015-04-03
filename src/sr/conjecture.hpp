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
    struct Choice
    {
        ContainerInterface::iterator it;
        ContainerInterface::iterator end;   
        int choice_num;
        Agent *agent; // Only for first decision registered by the agent; others NULL
        int num_decisions; // Only for first decision registered by the agent
    };

public:
    Conjecture();
    ~Conjecture();
	void PrepareForDecidedCompare();    			                                     
    bool Increment(bool trace=true); // returns true for try again, false for give up				 
    				   
    // General note: decided compare functions should call one of HandleDecision() or 
    // GetDecision() before calling other functions like ReportDecision() or
    // ForceDecision().
    				   
    // Standard interface for decided compare functions
    void BeginAgent( Agent *agent );
    void EndAgent();
    int GetCount( Agent *agent );
    bool GetChoice( ContainerInterface::iterator &it );
    void RegisterDecision( ContainerInterface::iterator begin,
                           ContainerInterface::iterator end );    

    string ChoiceAsString(const Choice &c);
				   
private:
    int get_choice_index;
    int register_decision_index;
	vector<Choice> choices;
	Agent *current_agent;
    int agent_first_index;
	
	// Tracing stuff
	void ResizeCounts();
	vector<int> inc_counts;
	vector<int> start_counts;
	vector<string> it_names;
	bool failed;
};

};

#endif
