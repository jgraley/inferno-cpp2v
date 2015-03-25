#ifndef CONJECTURE_HPP
#define CONJECTURE_HPP

#include "common/common.hpp"
#include <vector>

namespace SR 
{

class CompareReplace;

/// Utility to keep track of the current set of choices that have been made at decision points, and to advance through them until a full match is found
class Conjecture
{
public:
    struct Choice
    {
        ContainerInterface::iterator it;
        ContainerInterface::iterator end;   
        int choice_num;
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
	ContainerInterface::iterator HandleDecision( ContainerInterface::iterator begin,
			                                     ContainerInterface::iterator end );

    Choice *GetChoicePtr() { return decision_index < choices.size() ? &choices[decision_index] : NULL; } // TODO should be const ptr
    Choice *GetPrevChoicePtr() { return (decision_index>0 && decision_index-1 < choices.size()) ? &choices[decision_index-1] : NULL; } // TODO should be const ptr
				   
    string ChoiceAsString(const Choice &c);
				   
private:
	int decision_index;
	vector<Choice> choices;
	
	// Tracing stuff
	void ResizeCounts();
	vector<int> inc_counts;
	vector<int> start_counts;
	vector<string> it_names;
	bool failed;
};

};

#endif
