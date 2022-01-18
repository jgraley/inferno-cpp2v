#ifndef BOOLEAN_OPERATORS_HPP
#define BOOLEAN_OPERATORS_HPP

#include "expression.hpp"
#include "overloads.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SYM
{ 

// ------------------------- BooleanOperator --------------------------

class BooleanOperator : public BooleanToBooleanExpression
{
protected:
    virtual shared_ptr<Expression> TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                         shared_ptr<BooleanExpression> to_equal ) const override;
    
    // Helper function for various bool nodes that generate implications. Tries to 
    // add them to psol if they meet the required conditions, solves for value and
    // normalises nots.
    void TryAddPartialSolutionFor( shared_ptr<Expression> target,
                                   PartialSolution &psol, 
                                   bool key_sense, 
                                   shared_ptr<BooleanExpression> key, 
                                   shared_ptr<BooleanExpression> val_unsolved ) const;
};

// ------------------------- NotOperator --------------------------

class NotOperator : public BooleanToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit NotOperator( shared_ptr<BooleanExpression> a );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<BooleanResult>> &op_results ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<BooleanExpression> a;
};

Over<BooleanExpression> operator~( Over<BooleanExpression> a );

// ------------------------- AndOperator --------------------------

class AndOperator : public BooleanOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit AndOperator( list< shared_ptr<BooleanExpression> > sa_ );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<BooleanResult>> &op_results ) const override;

    virtual shared_ptr<Expression> TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                         shared_ptr<BooleanExpression> to_equal ) const override;
    virtual PartialSolution PartialSolveFor( shared_ptr<Expression> target ) const;    

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const list< shared_ptr<BooleanExpression> > sa;
};

Over<BooleanExpression> operator&( Over<BooleanExpression> a, Over<BooleanExpression> b );

// ------------------------- OrOperator --------------------------

class OrOperator : public BooleanOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit OrOperator( list< shared_ptr<BooleanExpression> > sa_ );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<BooleanResult>> &op_results ) const override;

    virtual PartialSolution PartialSolveFor( shared_ptr<Expression> target ) const;    

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const list< shared_ptr<BooleanExpression> > sa;
};

Over<BooleanExpression> operator|( Over<BooleanExpression> a, Over<BooleanExpression> b );

// ------------------------- BoolEqualOperator --------------------------

class BoolEqualOperator : public BooleanOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit BoolEqualOperator( shared_ptr<BooleanExpression> a, 
                                shared_ptr<BooleanExpression> b );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<BooleanResult>> &op_results ) const override;

    virtual shared_ptr<Expression> TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                         shared_ptr<BooleanExpression> to_equal ) const override;
    virtual PartialSolution PartialSolveFor( shared_ptr<Expression> target ) const;    

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<BooleanExpression> a;
    const shared_ptr<BooleanExpression> b;
};

Over<BooleanExpression> operator==( Over<BooleanExpression> a, Over<BooleanExpression> b );

// ------------------------- ImplicationOperator --------------------------

class ImplicationOperator : public BooleanOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit ImplicationOperator( shared_ptr<BooleanExpression> a_,
                                  shared_ptr<BooleanExpression> b_ );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<BooleanResult>> &op_results ) const override;

    virtual PartialSolution PartialSolveFor( shared_ptr<Expression> target ) const;    

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<BooleanExpression> a;
    const shared_ptr<BooleanExpression> b;
};

// ------------------------- BooleanConditionalOperator --------------------------

class BooleanConditionalOperator : public BooleanOperator
{
public:    
    typedef BooleanExpression NominalType;
    explicit BooleanConditionalOperator( shared_ptr<BooleanExpression> a_,
                                         shared_ptr<BooleanExpression> b_,
                                         shared_ptr<BooleanExpression> c_ );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const override;
    virtual shared_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const override;

    virtual PartialSolution PartialSolveFor( shared_ptr<Expression> target ) const;    

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<BooleanExpression> a;
    const shared_ptr<BooleanExpression> b;
    const shared_ptr<BooleanExpression> c;
};

};

#endif // include guard
