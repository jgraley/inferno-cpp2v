#ifndef CONJECTURE_HPP
#define CONJECTURE_HPP

#include "common/common.hpp"
//#include "coupling.hpp"
#include <vector>

class CompareReplace;


class Conjecture
{
public:
    struct Choice
    {
        ContainerInterface::iterator it;
        ContainerInterface::iterator end;        
        int end_count;
        int end_num;
        bool forced;
    };
public:
    Conjecture();
    ~Conjecture();
	void PrepareForDecidedCompare();    			                                     
    bool Increment(); // returns true for try again, false for give up				 
    				   
    // General note: decided compare functions should call one of HandleDecision() or 
    // GetDecision() before calling other functions like ReportDecision() or
    // ForceDecision().
    				   
    // Standard interface for decided compare functions
	ContainerInterface::iterator HandleDecision( ContainerInterface::iterator begin,
			                                     ContainerInterface::iterator end,
			                                     int en=0 );
    // Version that forces the decision down to a single choice			                                     
	ContainerInterface::iterator HandleDecision( ContainerInterface::iterator only );			                                    
    
    Choice *GetChoicePtr() { return decision_index < choices.size() ? &choices[decision_index] : NULL; } // TODO should be const ptr
    Choice *GetPrevChoicePtr() { return (decision_index>0 && decision_index-1 < choices.size()) ? &choices[decision_index-1] : NULL; } // TODO should be const ptr
				   
private:
	int decision_index;
	vector<Choice> choices;
	vector<int> counts;
	vector<string> it_names;
	bool failed;
};

#endif
