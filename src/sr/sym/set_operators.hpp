#ifndef SET_OPERATORS_HPP
#define SET_OPERATORS_HPP

#include "expression.hpp"
#include "overloads.hpp"

#include "../equivalence.hpp"
#include "../the_knowledge.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SYM
{ 

// ------------------------- ComplementOperator --------------------------

class ComplementOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit ComplementOperator( shared_ptr<SymbolExpression> a );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual shared_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit,
                                                        const list<shared_ptr<SymbolResultInterface>> &op_results ) const override final;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const shared_ptr<SymbolExpression> a;
};

// ------------------------- UnionOperator --------------------------

class UnionOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit UnionOperator( list< shared_ptr<SymbolExpression> > sa_ );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual shared_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit,
                                                        const list<shared_ptr<SymbolResultInterface>> &op_results ) const override;

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const list< shared_ptr<SymbolExpression> > sa;
};

// ------------------------- IntersectionOperator --------------------------

class IntersectionOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit IntersectionOperator( list< shared_ptr<SymbolExpression> > sa_ );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual shared_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit,
                                                        const list<shared_ptr<SymbolResultInterface>> &op_results ) const override;

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const list< shared_ptr<SymbolExpression> > sa;
};

};

#endif // include guard
