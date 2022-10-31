#ifndef BOOLEAN_OPERATORS_HPP
#define BOOLEAN_OPERATORS_HPP

#include "expression.hpp"
#include "lazy_eval.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SYM
{ 

// ------------------------- BooleanConstant --------------------------

class BooleanConstant : public BooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit BooleanConstant( bool value );
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const override;
    bool GetAsBool() const;    

    Orderable::Diff OrderCompare3WayLocal( const Orderable &right, 
                                           OrderProperty order_property ) const override;

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const bool value;
};

// ------------------------- NotOperator --------------------------

class NotOperator : public BooleanToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit NotOperator( shared_ptr<BooleanExpression> a );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<BooleanResult>> &&op_results ) const;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<BooleanExpression> a;
};

Lazy<BooleanExpression> operator~( Lazy<BooleanExpression> a );

// ------------------------- AndOperator --------------------------

class AndOperator : public BooleanToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit AndOperator( list< shared_ptr<BooleanExpression> > sa_ );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<BooleanResult>> &&op_results ) const;
    bool IsCommutative() const override;

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const list< shared_ptr<BooleanExpression> > sa;
};

Lazy<BooleanExpression> operator&( Lazy<BooleanExpression> a, Lazy<BooleanExpression> b );

// ------------------------- OrOperator --------------------------

class OrOperator : public BooleanToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit OrOperator( list< shared_ptr<BooleanExpression> > sa_ );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<BooleanResult>> &&op_results ) const;
    bool IsCommutative() const override;

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const list< shared_ptr<BooleanExpression> > sa;
};

Lazy<BooleanExpression> operator|( Lazy<BooleanExpression> a, Lazy<BooleanExpression> b );

// ------------------------- BoolEqualOperator --------------------------

class BoolEqualOperator : public BooleanToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit BoolEqualOperator( shared_ptr<BooleanExpression> a, 
                                shared_ptr<BooleanExpression> b );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<BooleanResult>> &&op_results ) const;
    bool IsCommutative() const override;

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<BooleanExpression> a;
    const shared_ptr<BooleanExpression> b;
};

Lazy<BooleanExpression> operator==( Lazy<BooleanExpression> a, Lazy<BooleanExpression> b );

// ------------------------- ImplicationOperator --------------------------

class ImplicationOperator : public BooleanToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit ImplicationOperator( shared_ptr<BooleanExpression> a_,
                                  shared_ptr<BooleanExpression> b_ );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const override;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit,
                                                list<unique_ptr<BooleanResult>> &&op_results ) const;

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<BooleanExpression> a;
    const shared_ptr<BooleanExpression> b;
};

// ------------------------- Uniplexor --------------------------

class Uniplexor : public BooleanToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    explicit Uniplexor( shared_ptr<BooleanExpression> a_,
                                         shared_ptr<BooleanExpression> b_,
                                         shared_ptr<BooleanExpression> c_ );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const;

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<BooleanExpression> a;
    const shared_ptr<BooleanExpression> b;
    const shared_ptr<BooleanExpression> c;
};

// ------------------------- Multiplexor --------------------------

class Multiplexor : public BooleanToBooleanExpression
{
public:    
    typedef BooleanExpression NominalType;
    // Controls evaluated and then taken as a little-endian binary nuber which
    // indexes into options - this option is then evaluated and returned. No smart
    // handling of undefined controls yet.
    explicit Multiplexor( vector<shared_ptr<BooleanExpression>> controls,
                          vector<shared_ptr<BooleanExpression>> options );
    virtual list<shared_ptr<BooleanExpression>> GetBooleanOperands() const;
    virtual unique_ptr<BooleanResult> Evaluate( const EvalKit &kit ) const;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const vector<shared_ptr<BooleanExpression>> controls;
    const vector<shared_ptr<BooleanExpression>> options;
};

};

#endif // include guard
