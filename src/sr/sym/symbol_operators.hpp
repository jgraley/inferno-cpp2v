#ifndef SYMBOL_OPERATORS_HPP
#define SYMBOL_OPERATORS_HPP

#include "expression.hpp"
#include "overloads.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "../the_knowledge.hpp"

namespace SYM
{ 

// ------------------------- SymbolConstant --------------------------

class SymbolConstant : public SymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit SymbolConstant( SR::XLink xlink );
    virtual unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit ) const override;    
    unique_ptr<SymbolResultInterface> GetValue() const;
    SR::XLink GetOnlyXLink() const;

    virtual Orderable::Result OrderCompareLocal( const Orderable *candidate, 
                                                 OrderProperty order_property ) const override;

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const SR::XLink xlink;
};

// ------------------------- SymbolVariable --------------------------

class SymbolVariable : public SymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit SymbolVariable( const SR::PatternLink &plink );
    virtual set<SR::PatternLink> GetRequiredVariables() const override;
    virtual unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit ) const override;
    SR::PatternLink GetPatternLink() const;

    // Note: no TrySolveForToEqualNT() because trivial solver is sufficient
    Orderable::Result OrderCompareLocal( const Orderable *candidate, 
                                         OrderProperty order_property ) const override;

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;

private:
    const SR::PatternLink plink;
};

// ------------------------- ItemiseToSymbolOperator --------------------------

class ItemiseToSymbolOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit ItemiseToSymbolOperator( TreePtr<Node> archetype_node,
                                      int item, 
                                      shared_ptr<SymbolExpression> a );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit,
                                               list<unique_ptr<SymbolResultInterface>> &&op_results ) const override final;
    virtual unique_ptr<SymbolResultInterface> EvalFromItem( SR::XLink parent_xlink, 
                                                   Itemiser::Element *item ) const = 0;

    virtual Orderable::Result OrderCompareLocal( const Orderable *candidate, 
                                                 OrderProperty order_property ) const override;                                                

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    virtual string GetItemTypeName() const = 0;
    
protected:
    const TreePtr<Node> archetype_node;
    const int item_index;
    const shared_ptr<SymbolExpression> a;
};

// ------------------------- ChildOperator --------------------------

class ChildOperator : public ItemiseToSymbolOperator
{
    using ItemiseToSymbolOperator::ItemiseToSymbolOperator;
    virtual shared_ptr<Expression> TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                         shared_ptr<SymbolExpression> to_equal ) const;
};

// ------------------------- ChildSequenceFrontOperator --------------------------

class ChildSequenceFrontOperator : public ChildOperator
{
public:    
    using ChildOperator::ChildOperator;
    virtual unique_ptr<SymbolResultInterface> EvalFromItem( SR::XLink parent_xlink, 
                                                   Itemiser::Element *item ) const override;
    virtual string GetItemTypeName() const override;    
};

// ------------------------- ChildSequenceBackOperator --------------------------

class ChildSequenceBackOperator : public ChildOperator
{
public:    
    using ChildOperator::ChildOperator;
    virtual unique_ptr<SymbolResultInterface> EvalFromItem( SR::XLink parent_xlink, 
                                                   Itemiser::Element *item ) const override;
    virtual string GetItemTypeName() const override;    
};

// ------------------------- ChildCollectionFrontOperator --------------------------

class ChildCollectionFrontOperator : public ChildOperator
{
public:    
    using ChildOperator::ChildOperator;
    virtual unique_ptr<SymbolResultInterface> EvalFromItem( SR::XLink parent_xlink, 
                                                   Itemiser::Element *item ) const override;
    virtual string GetItemTypeName() const override;    
};

// ------------------------- SingularChildOperator --------------------------

class SingularChildOperator : public ChildOperator
{
public:    
    using ChildOperator::ChildOperator;
    virtual unique_ptr<SymbolResultInterface> EvalFromItem( SR::XLink parent_xlink, 
                                                   Itemiser::Element *item ) const override;
    virtual string GetItemTypeName() const override;    
};

// ------------------------- KnowledgeToSymbolOperator --------------------------

class KnowledgeToSymbolOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit KnowledgeToSymbolOperator( shared_ptr<SymbolExpression> a );
    VariablesRequiringNuggets GetVariablesRequiringNuggets() const override;
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit,
                                               list<unique_ptr<SymbolResultInterface>> &&op_results ) const override final;
    virtual SR::XLink EvalXLinkFromNugget( SR::XLink parent_xlink, 
                                           const SR::TheKnowledge::Nugget &nugget ) const = 0;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    virtual string GetKnowledgeName() const = 0;
    
protected:
    const shared_ptr<SymbolExpression> a;
};

// ------------------------- ParentOperator --------------------------

class ParentOperator : public KnowledgeToSymbolOperator
{
public:    
    using KnowledgeToSymbolOperator::KnowledgeToSymbolOperator;
    virtual SR::XLink EvalXLinkFromNugget( SR::XLink parent_xlink, 
                                           const SR::TheKnowledge::Nugget &nugget ) const override;

    virtual shared_ptr<Expression> TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                         shared_ptr<SymbolExpression> to_equal ) const override;

    virtual string GetKnowledgeName() const override;
};

// ------------------------- LastDescendantOperator --------------------------

class LastDescendantOperator : public KnowledgeToSymbolOperator
{
public:    
    using KnowledgeToSymbolOperator::KnowledgeToSymbolOperator;
    virtual SR::XLink EvalXLinkFromNugget( SR::XLink parent_xlink, 
                                           const SR::TheKnowledge::Nugget &nugget ) const override;
    virtual string GetKnowledgeName() const override;
};

// ------------------------- MyContainerFrontOperator --------------------------

class MyContainerFrontOperator : public KnowledgeToSymbolOperator
{
public:    
    using KnowledgeToSymbolOperator::KnowledgeToSymbolOperator;
    virtual SR::XLink EvalXLinkFromNugget( SR::XLink parent_xlink, 
                                           const SR::TheKnowledge::Nugget &nugget ) const override;
    virtual string GetKnowledgeName() const override;
};

// ------------------------- MyContainerBackOperator --------------------------

class MyContainerBackOperator : public KnowledgeToSymbolOperator
{
public:    
    using KnowledgeToSymbolOperator::KnowledgeToSymbolOperator;
    virtual SR::XLink EvalXLinkFromNugget( SR::XLink parent_xlink, 
                                           const SR::TheKnowledge::Nugget &nugget ) const override;
    virtual string GetKnowledgeName() const override;
};

// ------------------------- MySequenceSuccessorOperator --------------------------

class MySequenceSuccessorOperator : public KnowledgeToSymbolOperator
{
public:    
    using KnowledgeToSymbolOperator::KnowledgeToSymbolOperator;
    virtual SR::XLink EvalXLinkFromNugget( SR::XLink parent_xlink, 
                                           const SR::TheKnowledge::Nugget &nugget ) const override;

    virtual shared_ptr<Expression> TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                         shared_ptr<SymbolExpression> to_equal ) const override;

    virtual string GetKnowledgeName() const override;
};

// ------------------------- MySequencePredecessorOperator --------------------------

class MySequencePredecessorOperator : public KnowledgeToSymbolOperator
{
public:    
    using KnowledgeToSymbolOperator::KnowledgeToSymbolOperator;
    virtual SR::XLink EvalXLinkFromNugget( SR::XLink parent_xlink, 
                                           const SR::TheKnowledge::Nugget &nugget ) const override;

    virtual shared_ptr<Expression> TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                         shared_ptr<SymbolExpression> to_equal ) const override;

    virtual string GetKnowledgeName() const override;
};

// ------------------------- AllChildrenOperator --------------------------

class AllChildrenOperator : public SymbolToSymbolExpression
{
public:    
    typedef SymbolExpression NominalType;
    explicit AllChildrenOperator( shared_ptr<SymbolExpression> a );
    virtual list<shared_ptr<SymbolExpression>> GetSymbolOperands() const override;
    virtual unique_ptr<SymbolResultInterface> Evaluate( const EvalKit &kit,
                                               list<unique_ptr<SymbolResultInterface>> &&op_results ) const override final;

    virtual shared_ptr<Expression> TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                         shared_ptr<SymbolExpression> to_equal ) const override;
                                             
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
protected:
    const shared_ptr<SymbolExpression> a;
};
};

#endif // include guard
