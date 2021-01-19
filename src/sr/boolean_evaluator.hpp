#ifndef BOOLEAN_EVALUATOR_HPP
#define BOOLEAN_EVALUATOR_HPP

#include <list>
#include <functional>

namespace SR
{
    
// Now just a functor that evaluates down the list
typedef function<bool(list<bool>)> BooleanEvaluator;
 
};

#endif
