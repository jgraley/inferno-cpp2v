#ifndef SYMBOL_OPERATORS_HPP
#define SYMBOL_OPERATORS_HPP

#include "expression.hpp"
#include "lazy.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SR
{
    class Agent;
};

namespace SYM
{ 

// ------------------------- ChildOperator --------------------------

class ChildOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    ChildOperator( const SR::Agent *ref_agent,
                   int item, 
                   shared_ptr<SymbolExpression> a );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual unique_ptr<SymbolResult> Evaluate( const EvalKit &kit,
                                               const list<unique_ptr<SymbolResult>> &op_results ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    virtual SR::XLink XLinkFromItem( SR::XLink parent_xlink, 
                                     Itemiser::Element *item ) const = 0;
    virtual string GetItemTypeName() const = 0;
    
private:
    const SR::Agent *ref_agent;
    const int item_index;
    shared_ptr<SymbolExpression> a;
};

// ------------------------- ChildSequenceFrontOperator --------------------------

class ChildSequenceFrontOperator : public ChildOperator
{
public:    
    using ChildOperator::ChildOperator;
    virtual SR::XLink XLinkFromItem( SR::XLink parent_xlink, 
                                     Itemiser::Element *item ) const override;
    virtual string GetItemTypeName() const override;    
};

// ------------------------- ChildSequenceBackOperator --------------------------

class ChildSequenceBackOperator : public ChildOperator
{
public:    
    using ChildOperator::ChildOperator;
    virtual SR::XLink XLinkFromItem( SR::XLink parent_xlink, 
                                     Itemiser::Element *item ) const override;
    virtual string GetItemTypeName() const override;    
};

// ------------------------- ChildCollectionFrontOperator --------------------------

class ChildCollectionFrontOperator : public ChildOperator
{
public:    
    using ChildOperator::ChildOperator;
    virtual SR::XLink XLinkFromItem( SR::XLink parent_xlink, 
                                     Itemiser::Element *item ) const override;
    virtual string GetItemTypeName() const override;    
};

// ------------------------- ChildSingularOperator --------------------------

class ChildSingularOperator : public ChildOperator
{
public:    
    using ChildOperator::ChildOperator;
    virtual SR::XLink XLinkFromItem( SR::XLink parent_xlink, 
                                     Itemiser::Element *item ) const override;
    virtual string GetItemTypeName() const override;    
};

// ------------------------- MyContainerOperator --------------------------

class MyContainerOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    MyContainerOperator( shared_ptr<SymbolExpression> a );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual unique_ptr<SymbolResult> Evaluate( const EvalKit &kit,
                                               const list<unique_ptr<SymbolResult>> &op_results ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    virtual SR::XLink XLinkFromNugget( SR::XLink parent_xlink, 
                                       const SR::TheKnowledge::Nugget &nugget ) const = 0;
    virtual string GetKnowledgeName() const = 0;
    
protected:
    shared_ptr<SymbolExpression> a;
};

// ------------------------- MyContainerFrontOperator --------------------------

class MyContainerFrontOperator : public MyContainerOperator
{
public:    
    using MyContainerOperator::MyContainerOperator;
    virtual SR::XLink XLinkFromNugget( SR::XLink parent_xlink, 
                                       const SR::TheKnowledge::Nugget &nugget ) const override;
    virtual string GetKnowledgeName() const override;
};

// ------------------------- MyContainerBackOperator --------------------------

class MyContainerBackOperator : public MyContainerOperator
{
public:    
    using MyContainerOperator::MyContainerOperator;
    virtual SR::XLink XLinkFromNugget( SR::XLink parent_xlink, 
                                       const SR::TheKnowledge::Nugget &nugget ) const override;
    virtual string GetKnowledgeName() const override;
};

// ------------------------- MySequenceSuccessorOperator --------------------------

class MySequenceSuccessorOperator : public MyContainerOperator
{
public:    
    using MyContainerOperator::MyContainerOperator;
    virtual SR::XLink XLinkFromNugget( SR::XLink parent_xlink, 
                                       const SR::TheKnowledge::Nugget &nugget ) const override;
    virtual string GetKnowledgeName() const override;
};

};

#endif // include guard
