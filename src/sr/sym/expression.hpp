#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "../link.hpp"
#include "../the_knowledge.hpp"

#include <exception>

namespace SYM
{ 

class SymbolVariable;
class SymbolExpression;

// ------------------------- Result --------------------------

class Result
{
};

// ------------------------- BooleanResult --------------------------

class BooleanResult : public Result
{
public:
    enum BooleanValue
    {
        FALSE,
        UNDEFINED,
        TRUE
    };    
    BooleanResult( BooleanValue value );
    BooleanValue value;
};

// ------------------------- SymbolResult --------------------------

class SymbolResult : public Result
{
public:
    enum BooleanCategory
    {
        UNDEFINED,
        XLINK
    };    
    SymbolResult();
    SymbolResult( BooleanCategory cat, SR::XLink xlink=SR::XLink() );
    BooleanCategory cat; 
    SR::XLink xlink;
};

// ------------------------- Expression --------------------------

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
        COMPARE,
        AND,
        XOR,
        OR,
        IMPLICATION,
        CONDITIONAL,
        COMMA 
    };    
        
    virtual list<shared_ptr<Expression>> GetOperands() const;
    virtual set<SR::PatternLink> GetRequiredVariables() const;
    virtual shared_ptr<SymbolExpression> TrySolveFor( shared_ptr<SymbolVariable> target ) const;
    virtual bool IsIndependentOf( shared_ptr<SymbolVariable> target ) const;
    
    virtual string Render() const = 0;    
    string GetTrace() const; // used for debug
    
protected:    
    virtual Precedence GetPrecedence() const = 0;
    string RenderForMe( shared_ptr<const Expression> inner ) const;
};

// ------------------------- BooleanExpression --------------------------

// Kept in operator.hpp because of wider inclusion than the impl classes
class BooleanExpression : public Expression
{    
public:
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const = 0;
};

// ------------------------- SymbolExpression --------------------------

// Kept in operator.hpp because of wider inclusion than the impl classes
class SymbolExpression : public Expression
{    
public:
    virtual shared_ptr<SymbolResult> Evaluate( const EvalKit &kit ) const = 0;
};

// ------------------------- BooleanToBooleanExpression --------------------------

// Kept in operator.hpp because of wider inclusion than the impl classes
class BooleanToBooleanExpression : public BooleanExpression
{    
public:
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const;
    virtual list<shared_ptr<Expression>> GetOperands() const final override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit, 
                                                const list<shared_ptr<BooleanResult>> &op_results ) const;
};

// ------------------------- SymbolToBooleanExpression --------------------------

// Kept in operator.hpp because of wider inclusion than the impl classes
class SymbolToBooleanExpression : public BooleanExpression
{    
public:
    // If you want 0 operands and a boolean result, use BooleanExpression as the base
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const = 0;
    virtual list<shared_ptr<Expression>> GetOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit, 
                                                const list<shared_ptr<SymbolResult>> &op_results ) const;
};

// ------------------------- SymbolToSymbolExpression --------------------------

// Kept in operator.hpp because of wider inclusion than the impl classes
class SymbolToSymbolExpression : public SymbolExpression
{    
public:
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const;
    virtual list<shared_ptr<Expression>> GetOperands() const override;
    virtual shared_ptr<SymbolResult> Evaluate( const EvalKit &kit ) const override;
    virtual shared_ptr<SymbolResult> Evaluate( const EvalKit &kit, 
                                               const list<shared_ptr<SymbolResult>> &op_results ) const;
};

};

#endif // include guard
