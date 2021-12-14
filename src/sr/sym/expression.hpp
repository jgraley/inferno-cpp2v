#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "../link.hpp"
#include "../the_knowledge.hpp"

#include <exception>

namespace SYM
{ 

class Expression : public Traceable
{    
public:
    /**
     * The information we need to evaluate a symbolic expression (but
     * not to manipulate it).
     */
    struct EvalKit
    {
        const SR::SolutionMap *hypothesis_links;
        const SR::TheKnowledge *knowledge;
    };
    
    // Bigger number is LOWER precedence.
    enum class Precedence
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
        
    virtual set<shared_ptr<Expression>> GetOperands() const;

    virtual set<SR::PatternLink> GetRequiredVariables() const;
    virtual string Render() const = 0;    
    string GetTrace() const; // used for debug
    
protected:    
    virtual Precedence GetPrecedence() const = 0;
    string RenderForMe( shared_ptr<const Expression> inner ) const;
};


struct BooleanResult
{
    enum Matched
    {
        UNKNOWN,
        TRUE,
        FALSE
    } matched;
};


// Kept in operator.hpp because of wider inclusion than the impl classes
class BooleanExpression : public Expression
{    
public:
    virtual BooleanResult Evaluate( const EvalKit &kit ) const = 0; // throws on mismatch
};


struct SymbolResult
{
    SR::XLink xlink;
};


// Kept in operator.hpp because of wider inclusion than the impl classes
class SymbolExpression : public Expression
{    
public:
    virtual SymbolResult Evaluate( const EvalKit &kit ) const = 0; // throws on mismatch
};

};

#endif // include guard
