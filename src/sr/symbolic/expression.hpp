#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "../link.hpp"
#include "../the_knowledge.hpp"

#include <exception>

namespace SYM
{ 

class Expression
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
    
    // Bigger number is LOWER precedence.
    enum Precedence
    {
        LITERAL,
        SCOPE,
        POSTFIX,
        PREFIX,
        MULTIPLY,
        ADD,
        SHIFT,
        COMPARE_GTLT, // <, >, <=, >=
        COMPARE_EQNE, // ==, !=
        AND,
        XOR,
        OR,
        COMMA 
    };    
        
    virtual set<SR::PatternLink> GetInputPatternLinks() const = 0;
    
    virtual string Render() const = 0;    
    
protected:    
    virtual Precedence GetPrecedence() const = 0;
    string RenderForMe( shared_ptr<const Expression> inner ) const;
};


struct BooleanResult
{
    bool matched;
    exception_ptr reason; // Actually an exception object
};


// Kept in operator.hpp because of wider inclusion than the impl classes
class BooleanExpression : public Expression
{    
public:
    virtual BooleanResult Evaluate( const EvalKit &kit ) const = 0; // throws on mismatch
};

};

#endif // include guard
