#ifndef CONJECTURE_HPP
#define CONJECTURE_HPP

#include "common/common.hpp"
#include "coupling.hpp"
#include <vector>

class CompareReplace;


class Conjecture
{
private:
    typedef ContainerInterface::iterator Choice;
public:
	void PrepareForDecidedCompare();
	ContainerInterface::iterator HandleDecision( ContainerInterface::iterator begin,
			                                     ContainerInterface::iterator end );
	Result Search( TreePtr<Node> x,
				   TreePtr<Node> pattern,
				   bool can_key,
				   const CompareReplace *sr );
private:
	bool ShouldTryMore( Result r, int threshold );
	int decision_index;
	vector<Choice> choices;
};

#endif
