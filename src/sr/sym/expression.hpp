#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "../link.hpp"
#include "../the_knowledge.hpp"

#include <exception>

namespace SYM
{ 

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
        UNKNOWN,
        TRUE,
        FALSE
    };    
    BooleanResult( BooleanValue value );
    BooleanValue value;
};

// ------------------------- SymbolResult --------------------------

class SymbolResult : public Result
{
public:
    SymbolResult();
    SymbolResult( const SR::XLink &xlink );
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
        COMPARE_GTLT, // <, >, <=, >=
        COMPARE_EQNE, // ==, !=
        AND,
        XOR,
        OR,
        COMMA 
    };    
        
    virtual list<shared_ptr<Expression>> GetOperands() const;

    virtual set<SR::PatternLink> GetRequiredVariables() const;
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
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const = 0;
};

// ------------------------- SymbolExpression --------------------------

// Kept in operator.hpp because of wider inclusion than the impl classes
class SymbolExpression : public Expression
{    
public:
    virtual unique_ptr<SymbolResult> Evaluate( const EvalKit &kit ) const = 0;
};

// ------------------------- BooleanToBooleanExpression --------------------------

// Kept in operator.hpp because of wider inclusion than the impl classes
class BooleanToBooleanExpression : public BooleanExpression
{    
public:
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const;
    virtual list<shared_ptr<Expression>> GetOperands() const final override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit, 
                                                const list<unique_ptr<BooleanResult>> &op_results ) const;
};

// ------------------------- SymbolToBooleanExpression --------------------------

// Kept in operator.hpp because of wider inclusion than the impl classes
class SymbolToBooleanExpression : public BooleanExpression
{    
public:
    // If you want 0 operands and a boolean result, use BooleanExpression as the base
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const = 0;
    virtual list<shared_ptr<Expression>> GetOperands() const override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit, 
                                                const list<unique_ptr<SymbolResult>> &op_results ) const;
};

// ------------------------- SymbolToSymbolExpression --------------------------

// Kept in operator.hpp because of wider inclusion than the impl classes
class SymbolToSymbolExpression : public SymbolExpression
{    
public:
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const;
    virtual list<shared_ptr<Expression>> GetOperands() const override;
    virtual unique_ptr<SymbolResult> Evaluate( const EvalKit &kit ) const;
    virtual unique_ptr<SymbolResult> Evaluate( const EvalKit &kit, 
                                               const list<unique_ptr<SymbolResult>> &op_results ) const;
};

};

#endif // include guard
