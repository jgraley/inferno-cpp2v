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
	void PrepareForDecidedCompare();    			                                     
    bool Increment(); // returns true for try again, false for give up				 
    				   
    // General note: decided compare functions should call one of HandleDecision() or 
    // GetDecision() before calling other functions like ReportDecision() or
    // ForceDecision().
    				   
    // Standard interface for decided compare functions
	ContainerInterface::iterator HandleDecision( ContainerInterface::iterator begin,
			                                     ContainerInterface::iterator end,
			                                     int en=1 );
			                                     
	// Standard interface for decided compare functions, supports multiple end choices (numbered 1, 2, 3 etc)
	// Client must use ReportDecision(false) to stop endless ends
	Choice GetDecision( ContainerInterface::iterator begin,
			            ContainerInterface::iterator end,
			            int en=1 );
    void ReportDecision( bool ok );

    Choice ForceDecision( ContainerInterface::iterator tohere );
    
    Choice *GetChoicePtr() { return &choices[decision_index]; } // TODO should be const ptr
				   
private:
	int decision_index;
	vector<Choice> choices;
};

#endif
