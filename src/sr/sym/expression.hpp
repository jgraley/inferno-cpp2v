#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include "common/common.hpp"
#include "common/orderable.hpp"
#include "../link.hpp"

#include <exception>

namespace SR
{
    class TheKnowledge;
}

namespace SYM
{ 

class SymbolExpression;
class SymbolResultInterface;
class BooleanResult;

// ------------------------- Expression --------------------------

class Expression : public Traceable, public Orderable
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
    
    bool IsIndependentOf( shared_ptr<Expression> target ) const;
    using Orderable::OrderCompare;
    using Orderable::OrderCompareEqual;
    // Add versions that take shared_ptr
    static Orderable::Result OrderCompare( shared_ptr<const Expression> l, 
                                           shared_ptr<const Expression> r, 
                                           OrderProperty order_property = STRICT );    
    static bool OrderCompareEqual( shared_ptr<const Expression> l, 
                                   shared_ptr<const Expression> r, 
                                   OrderProperty order_property = STRICT );    
    virtual Orderable::Result OrderCompareChildren( const Orderable *candidate, 
                                                    OrderProperty order_property ) const override;
    struct OrderComparer : public Traceable 
    {
        bool operator()( const shared_ptr<const Expression> &a, 
                         const shared_ptr<const Expression> &b ) const;
    };
    
    // Fore more than 2 operands, this would mean "any permutation of operands
    // gives same result". Should not affect unary/primary. Default is false.
    virtual bool IsCommutative() const;

    virtual string Render() const = 0;    
    string RenderWithParentheses() const;
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
    // Value must be independent of target it is part of a solution. 
    // Key does not have this requirement, but it must not be NotOperator.
    typedef map< shared_ptr<BooleanExpression>, 
                 shared_ptr<Expression> > PartialSolutionForSense;
    
    // key is the sense of value.key: false if there would have been an odd 
    // number of NotOperator, otherwise true. Note: use [] to access this - 
    // doing so will make sure an empty PartialSolutionForSense is created.
    typedef map<bool, PartialSolutionForSense> PartialSolution;

    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const = 0;
    
    shared_ptr<Expression> TrySolveForToEqual( shared_ptr<Expression> target, 
                                               shared_ptr<BooleanExpression> to_equal ) const;
    virtual shared_ptr<Expression> TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                         shared_ptr<BooleanExpression> to_equal ) const;
};

// ------------------------- SymbolExpression --------------------------

// Kept in operator.hpp because of wider inclusion than the impl classes
class SymbolExpression : public Expression
{    
public:
    virtual shared_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit ) const = 0;
    
    shared_ptr<Expression> TrySolveForToEqual( shared_ptr<Expression> target, 
                                               shared_ptr<SymbolExpression> to_equal ) const;
    virtual shared_ptr<Expression> TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                         shared_ptr<SymbolExpression> to_equal ) const;
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
                                                const list<shared_ptr<SymbolResultInterface>> &op_results ) const;
};

// ------------------------- SymbolToSymbolExpression --------------------------

// Kept in operator.hpp because of wider inclusion than the impl classes
class SymbolToSymbolExpression : public SymbolExpression
{    
public:
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const;
    virtual list<shared_ptr<Expression>> GetOperands() const override;
    virtual shared_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit ) const override;
    virtual shared_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit, 
                                               const list<shared_ptr<SymbolResultInterface>> &op_results ) const;
};

};

#endif // include guard
