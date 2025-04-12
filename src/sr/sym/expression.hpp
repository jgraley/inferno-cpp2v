#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include "common/common.hpp"
#include "common/orderable.hpp"
#include "../link.hpp"

#include <exception>

namespace SR
{
    class XTreeDatabase;
    class Lacing;
}

namespace SYM
{ 

class SymbolExpression;
class SymbolVariable;
class SymbolicResult;
class BooleanResult;

// ------------------------- Expression --------------------------

class Expression : public virtual Traceable, public Orderable
{    
public:
    /**
     * The information we need to solve a symbolic expression (and
     * maybe other manipulations) at planning time.
     */
    struct SolveKit
    {
        const SR::Lacing *lacing;
    };

    /**
     * The information we need to evaluate a symbolic expression.
     */
    struct EvalKit
    {
        const SR::SolutionMap *hypothesis_links;
        const SR::XTreeDatabase *x_tree_db;
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
        MEMBER,
        COMPARE,
        AND,
        XOR,
        OR,
        IMPLICATION,
        CONDITIONAL,
        COMMA 
    };    
                
    typedef set<SR::PatternLink> VariablesRequiringRows;
                
    virtual list<shared_ptr<Expression>> GetOperands() const;
    virtual set<SR::PatternLink> GetRequiredVariables() const;
    virtual VariablesRequiringRows GetVariablesRequiringRows() const;

    void ForTreeDepthFirstWalk( function<void(const Expression *)> f ) const;
    
    bool IsIndependentOf( shared_ptr<Expression> target ) const;
    using Orderable::OrderCompare3Way;     
    virtual Orderable::Diff OrderCompare3WayChildren( const Orderable &right, 
                                                      OrderProperty order_property ) const override;
    struct Relation : public Traceable 
    {
        Orderable::Diff Compare3Way( const shared_ptr<const Expression> &l, 
                                     const shared_ptr<const Expression> &r ) const;
        bool operator()( const shared_ptr<const Expression> &l, 
                         const shared_ptr<const Expression> &r ) const;
    };
    
    // For more than 2 operands, this would mean "any permutation of operands
    // gives same result". Should not affect unary/primary. Default is false.
    virtual bool IsCommutative() const;

    virtual string Render() const = 0;    
    string RenderWithParentheses() const;
    string GetTrace() const final; // used for debug
    
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

    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const = 0;
    
    // Solve methods are named per rule #527
    virtual shared_ptr<SymbolExpression> TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const;
};

// ------------------------- SymbolExpression --------------------------

// Kept in operator.hpp because of wider inclusion than the impl classes
class SymbolExpression : public Expression
{    
public:
    virtual unique_ptr<SymbolicResult> Evaluate( const EvalKit &kit ) const = 0;
    
    // Solve methods are named per rule #527
    virtual shared_ptr<SymbolExpression> TrySolveForToEqual( const SolveKit &kit, shared_ptr<SymbolVariable> target, 
                                                         shared_ptr<SymbolExpression> to_equal ) const;
};

// ------------------------- BooleanToBooleanExpression --------------------------

// Kept in operator.hpp because of wider inclusion than the impl classes
class BooleanToBooleanExpression : public BooleanExpression
{    
public:
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const;
    virtual list<shared_ptr<Expression>> GetOperands() const final override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit, 
                                                list<unique_ptr<BooleanResult>> &&op_results ) const;
};

// ------------------------- SymbolToBooleanExpression --------------------------

// Kept in operator.hpp because of wider inclusion than the impl classes
class SymbolToBooleanExpression : public BooleanExpression
{    
public:
    // If you want 0 operands and a boolean result, use BooleanExpression as the base
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const = 0;
    virtual list<shared_ptr<Expression>> GetOperands() const override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit, 
                                                list<unique_ptr<SymbolicResult>> &&op_results ) const;
};

// ------------------------- SymbolToSymbolExpression --------------------------

// Kept in operator.hpp because of wider inclusion than the impl classes
class SymbolToSymbolExpression : public SymbolExpression
{    
public:
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const;
    virtual list<shared_ptr<Expression>> GetOperands() const override;
    virtual unique_ptr<SymbolicResult> Evaluate( const EvalKit &kit ) const override;
    virtual unique_ptr<SymbolicResult> Evaluate( const EvalKit &kit, 
                                               list<unique_ptr<SymbolicResult>> &&op_results ) const;
};

};

#endif // include guard
