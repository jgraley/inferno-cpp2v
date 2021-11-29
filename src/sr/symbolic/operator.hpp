#ifndef OPERATOR_HPP
#define OPERATOR_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "../link.hpp"
#include "../the_knowledge.hpp"

namespace SYM
{ 

class Operator
{    
public:
    /**
     * The information we need to evaluate a symbolic expression (but
     * not to manipulate it).
     */
    struct EvalKit
    {
        const SR::SolutionMap *required_links;
        const SR::TheKnowledge *knowledge;
    };

    virtual set<SR::PatternLink> GetInputPatternLinks() const = 0;
    virtual string Render() const = 0;
};


// Kept in operator.hpp because of wider inclusion thatn the impl classes
class BooleanOperator : public Operator
{    
public:
    virtual void Evaluate( const EvalKit &kit ) const = 0; // throws on mismatch
};


};

#endif // include guard
