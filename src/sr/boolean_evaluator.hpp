#ifndef BOOLEAN_EVALUATOR_HPP
#define BOOLEAN_EVALUATOR_HPP

#include <list>

namespace SR
{

// Interface class for a functor object that can produce the result of 
// some N to 1 operator on bools.
class BooleanEvaluator
{
public:
	virtual bool operator()( list<bool> &inputs ) const = 0;
	virtual ~BooleanEvaluator() {}
};
 
};

#endif
