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

// ------------------------- ItemiseToSymbolOperator --------------------------

class ItemiseToSymbolOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit ItemiseToSymbolOperator( const SR::Agent *ref_agent,
                             int item, 
                             shared_ptr<SymbolExpression> a );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual shared_ptr<SymbolResult> Evaluate( const EvalKit &kit,
                                               const list<shared_ptr<SymbolResult>> &op_results ) const override final;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    virtual SR::XLink XLinkFromItem( SR::XLink parent_xlink, 
                                     Itemiser::Element *item ) const = 0;
    virtual string GetItemTypeName() const = 0;
    
private:
    const SR::Agent *ref_agent;
    const int item_index;
    const shared_ptr<SymbolExpression> a;
};

// ------------------------- ChildSequenceFrontOperator --------------------------

class ChildSequenceFrontOperator : public ItemiseToSymbolOperator
{
public:    
    using ItemiseToSymbolOperator::ItemiseToSymbolOperator;
    virtual SR::XLink XLinkFromItem( SR::XLink parent_xlink, 
                                     Itemiser::Element *item ) const override;
    virtual string GetItemTypeName() const override;    
};

// ------------------------- ChildSequenceBackOperator --------------------------

class ChildSequenceBackOperator : public ItemiseToSymbolOperator
{
public:    
    using ItemiseToSymbolOperator::ItemiseToSymbolOperator;
    virtual SR::XLink XLinkFromItem( SR::XLink parent_xlink, 
                                     Itemiser::Element *item ) const override;
    virtual string GetItemTypeName() const override;    
};

// ------------------------- ChildCollectionFrontOperator --------------------------

class ChildCollectionFrontOperator : public ItemiseToSymbolOperator
{
public:    
    using ItemiseToSymbolOperator::ItemiseToSymbolOperator;
    virtual SR::XLink XLinkFromItem( SR::XLink parent_xlink, 
                                     Itemiser::Element *item ) const override;
    virtual string GetItemTypeName() const override;    
};

// ------------------------- ChildSingularOperator --------------------------

class ChildSingularOperator : public ItemiseToSymbolOperator
{
public:    
    using ItemiseToSymbolOperator::ItemiseToSymbolOperator;
    virtual SR::XLink XLinkFromItem( SR::XLink parent_xlink, 
                                     Itemiser::Element *item ) const override;
    virtual string GetItemTypeName() const override;    
};

// ------------------------- KnowledgeToSymbolOperator --------------------------

class KnowledgeToSymbolOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit KnowledgeToSymbolOperator( shared_ptr<SymbolExpression> a );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual shared_ptr<SymbolResult> Evaluate( const EvalKit &kit,
                                               const list<shared_ptr<SymbolResult>> &op_results ) const override final;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    virtual SR::XLink XLinkFromNugget( SR::XLink parent_xlink, 
                                       const SR::TheKnowledge::Nugget &nugget ) const = 0;
    virtual string GetKnowledgeName() const = 0;
    
protected:
    const shared_ptr<SymbolExpression> a;
};

// ------------------------- ParentOperator --------------------------

class ParentOperator : public KnowledgeToSymbolOperator
{
public:    
    using KnowledgeToSymbolOperator::KnowledgeToSymbolOperator;
    virtual SR::XLink XLinkFromNugget( SR::XLink parent_xlink, 
                                       const SR::TheKnowledge::Nugget &nugget ) const override;
    virtual string GetKnowledgeName() const override;
};

// ------------------------- LastDescendantOperator --------------------------

class LastDescendantOperator : public KnowledgeToSymbolOperator
{
public:    
    using KnowledgeToSymbolOperator::KnowledgeToSymbolOperator;
    virtual SR::XLink XLinkFromNugget( SR::XLink parent_xlink, 
                                       const SR::TheKnowledge::Nugget &nugget ) const override;
    virtual string GetKnowledgeName() const override;
};

// ------------------------- MyContainerFrontOperator --------------------------

class MyContainerFrontOperator : public KnowledgeToSymbolOperator
{
public:    
    using KnowledgeToSymbolOperator::KnowledgeToSymbolOperator;
    virtual SR::XLink XLinkFromNugget( SR::XLink parent_xlink, 
                                       const SR::TheKnowledge::Nugget &nugget ) const override;
    virtual string GetKnowledgeName() const override;
};

// ------------------------- MyContainerBackOperator --------------------------

class MyContainerBackOperator : public KnowledgeToSymbolOperator
{
public:    
    using KnowledgeToSymbolOperator::KnowledgeToSymbolOperator;
    virtual SR::XLink XLinkFromNugget( SR::XLink parent_xlink, 
                                       const SR::TheKnowledge::Nugget &nugget ) const override;
    virtual string GetKnowledgeName() const override;
};

// ------------------------- MySequenceSuccessorOperator --------------------------

class MySequenceSuccessorOperator : public KnowledgeToSymbolOperator
{
public:    
    using KnowledgeToSymbolOperator::KnowledgeToSymbolOperator;
    virtual SR::XLink XLinkFromNugget( SR::XLink parent_xlink, 
                                       const SR::TheKnowledge::Nugget &nugget ) const override;
    virtual string GetKnowledgeName() const override;
};

};

#endif // include guard
